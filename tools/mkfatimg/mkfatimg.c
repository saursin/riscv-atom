// mkfatimg: make a FAT32 disk image from a directory, using FatFs (sw/lib/fatfs) on the host.
// The image has no partition table (the volume starts at sector 0). Names must be 8.3, as the
// FatFs build has no long file names.
//
//     mkfatimg <dir> <image> [size]        size: e.g. 64M (default), 512K, or bytes
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define DIR FF_DIR      // FatFs and <dirent.h> both name a type DIR
#include "ff.h"
#include "diskio.h"
#undef DIR

#define SECTOR 512

static const char *img_path;
static int img_fd = -1;
static LBA_t img_sectors;


// ---- FatFs disk interface: drive 0 is the image file ----

DSTATUS disk_status(BYTE pdrv) {
    return (pdrv || img_fd < 0) ? STA_NOINIT : 0;
}

DSTATUS disk_initialize(BYTE pdrv) {
    return disk_status(pdrv);
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
    ssize_t n = (ssize_t)count * SECTOR;
    return pread(img_fd, buff, n, (off_t)sector * SECTOR) == n ? RES_OK : RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
    ssize_t n = (ssize_t)count * SECTOR;
    return pwrite(img_fd, buff, n, (off_t)sector * SECTOR) == n ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    switch(cmd) {
        case CTRL_SYNC:        return RES_OK;
        case GET_SECTOR_COUNT: *(LBA_t *)buff = img_sectors; return RES_OK;
        case GET_BLOCK_SIZE:   *(DWORD *)buff = 1; return RES_OK;
    }
    return RES_PARERR;
}


// ---- copying ----

// a failed run leaves no image behind
static void fail(void) {
    if(img_path)
        unlink(img_path);
    exit(1);
}

static void die(const char *what, const char *path, FRESULT fr) {
    if(fr == FR_INVALID_NAME)
        fprintf(stderr, "mkfatimg: %s: name is not 8.3 (at most 8 characters, a dot, 3 characters)\n", path);
    else
        fprintf(stderr, "mkfatimg: %s %s: FatFs error %d\n", what, path, fr);
    fail();
}

static void copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if(!in) {
        fprintf(stderr, "mkfatimg: %s: %s\n", src, strerror(errno));
        fail();
    }
    FIL f;
    FRESULT fr = f_open(&f, dst, FA_WRITE | FA_CREATE_NEW);
    if(fr != FR_OK)
        die("create", dst, fr);

    char buf[4096];
    size_t n, total = 0;
    while((n = fread(buf, 1, sizeof buf, in)) > 0) {
        UINT wr;
        if((fr = f_write(&f, buf, n, &wr)) != FR_OK || wr != n)
            die("write", dst, fr != FR_OK ? fr : FR_DENIED);    // FR_DENIED: the volume is full
        total += n;
    }
    fclose(in);
    if((fr = f_close(&f)) != FR_OK)
        die("close", dst, fr);
    printf("  %-30s %zu bytes\n", dst, total);
}

// copy the contents of host directory src into image directory dst ("" is the root)
static void copy_dir(const char *src, const char *dst) {
    struct dirent **ents;
    int n = scandir(src, &ents, NULL, alphasort);   // sorted: the same image every time
    if(n < 0) {
        fprintf(stderr, "mkfatimg: %s: %s\n", src, strerror(errno));
        fail();
    }
    for(int i = 0; i < n; i++) {
        const char *name = ents[i]->d_name;
        if(strcmp(name, ".") && strcmp(name, "..")) {
            char s[4096], d[4096];
            struct stat st;
            snprintf(s, sizeof s, "%s/%s", src, name);
            snprintf(d, sizeof d, "%s/%s", dst, name);
            if(stat(s, &st) != 0) {
                fprintf(stderr, "mkfatimg: %s: %s\n", s, strerror(errno));
                fail();
            }
            if(S_ISDIR(st.st_mode)) {
                FRESULT fr = f_mkdir(d);
                if(fr != FR_OK)
                    die("mkdir", d, fr);
                printf("  %s/\n", d);
                copy_dir(s, d);
            } else if(S_ISREG(st.st_mode)) {
                copy_file(s, d);
            }
        }
        free(ents[i]);
    }
    free(ents);
}


static unsigned long long parse_size(const char *s) {
    char *end;
    unsigned long long v = strtoull(s, &end, 0);
    switch(*end) {
        case 'K': case 'k': v <<= 10; end++; break;
        case 'M': case 'm': v <<= 20; end++; break;
        case 'G': case 'g': v <<= 30; end++; break;
    }
    if(*end || v == 0) {
        fprintf(stderr, "mkfatimg: bad size '%s'\n", s);
        exit(1);
    }
    return v;
}


int main(int argc, char **argv) {
    if(argc < 3 || argc > 4) {
        fprintf(stderr, "usage: mkfatimg <dir> <image> [size (default 64M; FAT32 needs at least ~33M)]\n");
        return 1;
    }
    unsigned long long size = argc == 4 ? parse_size(argv[3]) : 64ull << 20;
    img_sectors = size / SECTOR;

    img_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(img_fd < 0) {
        fprintf(stderr, "mkfatimg: %s: %s\n", argv[2], strerror(errno));
        return 1;
    }
    img_path = argv[2];
    if(ftruncate(img_fd, (off_t)img_sectors * SECTOR) != 0) {
        fprintf(stderr, "mkfatimg: %s: %s\n", argv[2], strerror(errno));
        fail();
    }

    static BYTE work[FF_MAX_SS * 8];
    MKFS_PARM opt = {.fmt = FM_FAT32 | FM_SFD};
    FRESULT fr = f_mkfs("", &opt, work, sizeof work);
    if(fr == FR_MKFS_ABORTED) {
        fprintf(stderr, "mkfatimg: %llu bytes is too small for FAT32 (needs at least ~33M)\n", size);
        fail();
    }
    if(fr != FR_OK)
        die("format", argv[2], fr);

    FATFS fs;
    if((fr = f_mount(&fs, "", 1)) != FR_OK)
        die("mount", argv[2], fr);
    printf("%s: FAT32, %llu bytes, from %s\n", argv[2], size, argv[1]);
    copy_dir(argv[1], "");
    f_unmount("");
    close(img_fd);
    return 0;
}
