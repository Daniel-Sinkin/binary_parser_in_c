#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

const char* filepath = "data/binary0.npy";

// BYTES PER WORD
const int WORD_SIZE = 8;
// WORDS PER CACHE LINE
const int CACHE_LINE_LENGTH = 8;
// BITS PER CACHE LINE
const int CACHE_LINE_SIZE = CACHE_LINE_LENGTH * WORD_SIZE;

// .NUMPY..v.
const unsigned char numpy_header[8] = {0x93, 0x4e, 0x55, 0x4d, 0x50, 0x59, 0x01, 0x00};

int main() {
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        printf("File not found. Exiting the program.\n");
        exit(1);
    }

    unsigned char data[CACHE_LINE_SIZE];

    /*
    READ IN THE HEADER
    */
    size_t itemsRead = fread(&data, 1, sizeof(data), file);

    printf("0000 -> ");
    for(int i = 0; i < CACHE_LINE_SIZE; i++) {
        printf("%02x ", data[i]);
        if(i % 8 == 7) {
            printf("\n");
            printf("%04x -> ", i + 1);
        }
    }

    if (itemsRead != sizeof(data)) {
        printf("Failed to read CACHE_LINE_SIZE bits from the file.\n");
        fclose(file);
        exit(1);
    } 
    


    if(data[CACHE_LINE_SIZE - 1] != 0x0a) {
        printf("The last byte of the header is not a newline character.\n");
        fclose(file);
        exit(1);
    }

    printf("Checking if the header starts with '.NUMPY..v.' \n");
    for(int j = 0; j < 8; j++) {
        if (data[j] != numpy_header[j]) {
            printf("Header mismatch at position %d.\n", j);
            fclose(file);
            exit(1);
        }
    }
    printf("This file is a .npy file, reading the header params.\n");

    for (int j = 0; j < CACHE_LINE_SIZE; j++) {
        printf("%02x ", data[j]);
        if(j % 8 == 7) {
            printf("\n");
        }
    }
    printf("\n");
    printf("Finished reading in the header.");
    
    fclose(file);
    return 0;
}