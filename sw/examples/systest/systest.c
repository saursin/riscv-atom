#include <stdio.h>
#include <string.h>
#include <time.h>
#include <csr.h>

#define CORE "RISC-V Atom"
#define ARCH "RV32I - little endian"

void print_arr(int arr[], int n){
    putchar('{');
    for(int i=0; i<n; i++){
        printf("%d%s", arr[i], ((i==n-1) ? "" : ", "));
    }
    putchar('}');
}

#include "tests/fibonacci.c"
#include "tests/factorial.c"
#include "tests/crypto.c"

void print_info() {
    printf("core(s):    %s @ %d Hz\n", CORE, CLK_FREQ);
    printf("arch:       %s\n", ARCH);
    printf("misa:       0x%x\n", CSR_read(CSR_MISA));
    printf("vend id:    0x%x\n", CSR_read(CSR_MVENDORID));
    printf("arch id:    0x%x\n", CSR_read(CSR_MARCHID));
    printf("vend id:    0x%x\n", CSR_read(CSR_MVENDORID));
    printf("hart id:    0x%x\n", CSR_read(CSR_MHARTID));
    printf("memory:\n");
    printf("\tcode ram: 0x%08x (%d bytes)\n", (uint32_t)CODERAM_BASE, (uint32_t)CODERAM_SIZE);
    printf("\tdata ram: 0x%08x (%d bytes)\n", (uint32_t)DATARAM_BASE, (uint32_t)DATARAM_SIZE);
}

int main(){
    serial_init(UART_BAUD_115200);

    print_info();

    // Prepare test list
    struct Test_t{
        char name[40];
        int (*fptr)(void);
    };
    
    struct Test_t testlist[] = {
        {.name="Fibonacci", .fptr=test_fibonacci},
        {.name="Factorial", .fptr=test_factorial},
        {.name="Crypto", .fptr=test_crypto}
    };
    int ntests = sizeof(testlist)/sizeof(struct Test_t);
    
    // Run tests
    printf("Running tests...\n");
    int nfailed = 0;
    char res[2][15] = {"*** FAIL ***", "*** PASS ***"};
    
    for(int i=0; i<ntests; i++){
        struct Test_t *test = &testlist[i];
        printf("==[ %d: %s ]====================\n", i, test->name);
        int ret = test->fptr();
        if(ret !=0 ){
            nfailed++;
        }
        printf("retcode: %d, %s\n\n", ret, res[ret==0]);
    }
    puts("========================================\n");
    puts(nfailed>0 ? "Some tests failed\n": "All passed!!\n");

    // calc CPI
    uint64_t cycles = cycle();
    uint64_t instret = CSR_read(CSR_INSTRETH);
    instret = (instret << 32) | CSR_read(CSR_INSTRET);

    printf("N Cycles: %lld\n", cycles);
    printf("N Instr:  %lld\n", instret);
    printf("CPI:      %lld\n", cycles/instret);
    return 0;
}
