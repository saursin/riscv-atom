#include <stdbool.h>
#include <stdlib.h>

#define isLower(c) ((c >= 'a') && (c <= 'z'))
#define isUpper(c) ((c >= 'A') && (c <= 'Z'))
#define isDigit(c) ((c >= '0') && (c <= '9'))

void caesar(char * input, char * output, int n, int key, bool encrypt) {
    for (int i = 0; i<n; ++i) {
        char ch = input[i];
        
        if (isLower(ch)) {          // Lowercase characters.
            ch = (ch - 'a' + (encrypt ? key: 26-key)) % 26 + 'a';
        }
        else if (isUpper(ch)) {     // Uppercase characters.
            ch = (ch - 'A' + (encrypt ? key: 26-key)) % 26 + 'A';
        }
        else if (isDigit(ch)) {     // Numbers.
            ch = (ch - '0' + (encrypt ? key: 26-key)) % 10 + '0';
        }
        output[i] = ch;
    }
    output[n] = '\0';
}

int test_crypto(){
    char plaintext[40] =  "aQUICKbrownFOXjumpedOVERtheLAZYdog";
    char ciphertext[40];
    char decrypted[40];

    int len = strlen(plaintext);

    int key = rand() % 26;
    caesar(plaintext, ciphertext, len, key, true);
    caesar(ciphertext, decrypted, len, key, false);

    printf("Key: %d\n", key);
    printf("plaintext : %s\n", plaintext);
    printf("ciphertext: %s\n", ciphertext);
    printf("decrypted : %s\n", decrypted);
    if(strcmp(plaintext, decrypted)){
        return 1;
    }
    return 0;
}