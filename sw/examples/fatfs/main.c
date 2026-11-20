// Read a FAT-formatted SD card with FatFs: list every file, then print hello.txt
#include <stdio.h>
#include <string.h>
#include <ff.h>

static FATFS fs;

// list the directory at path and everything below it
static void list(char *path, int depth) {
    DIR dir;
    FILINFO fi;
    if(f_opendir(&dir, path) != FR_OK)
        return;
    while(f_readdir(&dir, &fi) == FR_OK && fi.fname[0]) {
        for(int i = 0; i < depth; i++)
            puts("  ");
        if(fi.fattrib & AM_DIR) {
            printf("%s/\n", fi.fname);
            size_t n = strlen(path);
            path[n] = '/';
            strcpy(path + n + 1, fi.fname);
            list(path, depth + 1);
            path[n] = '\0';
        } else {
            printf("%s  (%u bytes)\n", fi.fname, (unsigned)fi.fsize);
        }
    }
    f_closedir(&dir);
}

int main() {
    FRESULT fr = f_mount(&fs, "", 1);
    if(fr != FR_OK) {
        printf("mount failed (FatFs error %d): no card, or not FAT\n", fr);
        return 1;
    }

    puts("Files on the card:\n");
    char path[128] = "";
    list(path, 1);

    puts("\nhello.txt:\n");
    FIL f;
    if((fr = f_open(&f, "hello.txt", FA_READ)) != FR_OK) {
        printf("can't open hello.txt (FatFs error %d)\n", fr);
        return 1;
    }
    char buf[65];
    UINT n;
    while(f_read(&f, buf, sizeof buf - 1, &n) == FR_OK && n) {
        buf[n] = '\0';
        puts(buf);
    }
    f_close(&f);
    return 0;
}
