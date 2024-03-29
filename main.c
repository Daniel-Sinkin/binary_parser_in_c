#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

const char* filepath = "data/binary0.npy";

const int MAX_JSON_KEY_SIZE = 64;

// BYTES PER WORD
const int WORD_SIZE = 8;
// BITS PER CACHE LINE
const int CACHE_LINE_SIZE = 8 * WORD_SIZE;

// .NUMPY..v.
const unsigned char numpy_header[8] = {0x93, 0x4e, 0x55, 0x4d, 0x50, 0x59, 0x01, 0x00};
const unsigned char second_word_start[3] = {0x76, 0x00, 0x7b};


enum PARSE_STATE {
    PARSE_HEADER,
    PARSE_JSON,
    PARSE_JSON_KEY,
    PARSE_JSON_VALUE,
    PARSE_DATA
};

enum SELECTED_KEY {
    NONE,
    DESCR,
    FORTRAN_ORDER,
    SHAPE
};


/*
Want to read the file in CACHE_LINE_SIZE chunks, the header will in general
be 2 cache lines long, where the second one is mostly padding, but I want to
support the general case where the header is very long.

Should read more into the actual binary encoding of .npy files, but for now it
looks like the start with
```
0x93, 0x4e, 0x55, 0x4d, 0x50, 0x59, 0x01, 0x00
```
which written as chars looks like .NUMPY..v.

Then a hashmap in the form of a JSON object is written, i.e. we start with
{ and end with }. Then the cacheline gets filled with 0x20 until we get to a
newline character, i.e. 0x0a.

The keys I've seen so far are {"descr", "fortran_order", "shape"}.

"descr" contains the datatype of the array, i.e. "<f8" for a float64, or "<f4"
for float32, "<i8" for int64, "<i4" for int32, "<i2" for int16, "<i1" for int8
and so on. Probably can just support everything that numpy supports, maybe only
restricting myself to the 4 bit datatypes for now.

"fortran_order" is a boolean, 0x00 for false and 0x01 for true, I don't want to
support fortran order so if this is not set to false I'll just exit the program.

"shape" is the tensor dimension of the numpy array, it depends on how I want to
handle it, basically this only makes a difference when we work with the numpy
array and this project only cares about decoding the binary, so I'll prolly just
parse it and give the option to pass it along.
*/
int main() {
    // Read the file, "rb" means read + binary
    FILE *file = fopen(filepath, "rb");
    // If we couldn't open the file then fopen returns NULL
    if (file == NULL) {
        printf("File not found. Exiting the program.\n");
        exit(1);
    }

    enum PARSE_STATE read_mode = PARSE_HEADER;
    enum SELECTED_KEY selected_key = NONE;

    // Allocating memory on the stack for the cache line chunk we read in, we
    // only want to read in CACHE_LINE_SIZE bits at a time to make this a bit
    // more challenging
    unsigned char data[CACHE_LINE_SIZE];
    data[CACHE_LINE_SIZE - 1] = 0x00;

    int idx = -1;

    /*
    READ IN THE HEADER
    */
    while(data[CACHE_LINE_SIZE - 1] != 0x0a) {
        // Reads in CACHE_LINE_SIZE bits from the file and stores it in the data,
        // this moves the internal file pointer to the next CACHE_LINE_SIZE bits
        // so we don't have to keep track of how many bits we've read so far.
        size_t itemsRead = fread(&data, 1, sizeof(data), file);

        // There are not enough bits in the file to read in a CACHE_LINE_SIZE chunk
        // which would mean that the file is not aligned properly, don't want to
        // support that case so we just exit.
        if (itemsRead != sizeof(data)) {
            printf("Failed to read full cache line from the file, it's encoded improperly.\n");
            fclose(file);
            exit(1);
        } 

        // Hexdumps the cache line with relative numbering
        printf("HEXDUMP of Cache Line:\n");
        for(int i = 0; i < CACHE_LINE_SIZE; i++) {
            if(i % 8 == 0) {
                printf("\t%04x -> ", i + 1);
            }
            printf("%02x ", data[i]);
            if(i % 8 == 7) {
                printf("\n");
            }
        }
        
        printf("\n");

        // Will need to have different reading modes for the cache line, at the
        // start we're trying to parse the first word of the header to check if
        // it's a .npy file. Afterwards we need to write a json parser.
        if(read_mode == PARSE_HEADER) {
            printf("Checking if the header starts with '.NUMPY..v.' \n");
            for(int j = 0; j < 8; j++) {
                if (data[j] != numpy_header[j]) {
                    printf("Header mismatch at position %d.\n", j);
                    fclose(file);
                    exit(1);
                }
            }
            printf("This file is a .npy file, reading the header params.\n");
        }
        
        if(read_mode == PARSE_HEADER) {
            idx = 11;
            read_mode = PARSE_JSON;
        } else if(idx == -1) {
            printf("Failed to find the start of the JSON object.\n");
            fclose(file);
            exit(1);
        }
        
        if(data[idx - 1] != '{'){
            printf("%c", data[idx]);
            printf("Invalid JSON object. Exiting the program.\n");
            break;
        }

        while(idx < CACHE_LINE_SIZE) {
            char curr = data[idx];
            printf("curr = %c\n", curr);
            switch (read_mode) {
                case PARSE_JSON:
                    printf("Parsing JSON object...\n");
                    if((curr != '\'') && (curr != '\"')){
                        printf("<MISFORMED_KEY>Invalid JSON object. Exiting the program.\n");
                        fclose(file);
                        exit(-1);
                    }
                    read_mode = PARSE_JSON_KEY;
                    char key[MAX_JSON_KEY_SIZE];
                    size_t key_idx = 0;
                    break;
                case PARSE_JSON_KEY:
                    if ((curr != '\'') && (curr != '\"')) {
                        key[key_idx] = curr;
                    } else {
                        printf("Hit the end!\n");
                        key[key_idx] = '\0';
                        printf("The final key is %s!\n", key);
                        read_mode = PARSE_JSON_VALUE;

                        if(strcmp(key, "descr") == 0) {
                            printf("We have correctly parse the 'descr' key!\n");
                            selected_key = DESCR;
                        } else if(strcmp(key, "fortran_order")) {
                            printf("We have correctly parse the 'fortran_order' key!\n");
                            selected_key = FORTRAN_ORDER;
                        } else if(strcmp(key, "shape") == 0) {
                            printf("We have correctly parse the 'shape' key!\n");
                            selected_key = SHAPE;
                        } else {
                            printf("Invalid JSON key '%s'. Exiting the program.\n", key);
                            fclose(file);
                            exit(-1);
                        }
                        char key[MAX_JSON_KEY_SIZE];
                        size_t key_idx = 0;
                    } 
                    key_idx += 1;
                    if(key_idx >= MAX_JSON_KEY_SIZE) {
                        printf("<KEY_TOO_LONG>Invalid JSON object. Exiting the program.\n");
                        fclose(file);
                        exit(-1);
                    }
                    printf("key = ");
                    for (int i = 0; i < key_idx; i++) {
                        printf("%c", key[i]);
                    }
                    printf("\n");
                    break;
                case PARSE_JSON_VALUE:
                    printf("Parsing JSON value...\n");

                    return 0;
                    // TODO: Implement JSON value parsing logic
                    break;
                default:
                    printf("Invalid read mode. Exiting the program.\n");
                    fclose(file);
                    exit(-1);
            }
            idx += 1;
        }

        printf("\n");
        printf("Finished reading in the header.\n");

        /*
        We know from the "descr" value what kind of bit alignment the contents have
        and we know how to interpret them, so this should be relatively easy.

        Read in 1, 2, 4, 8 bytes depending on how large the datatype is and then
        parse the binary data to the correct type, I think a switch statement would
        be great here, I think we'll just parse and print it for now.

        Supporting different shapes also shouldn't be a problem, but we'll deal with
        that once we get to it.
        */
    }

    fclose(file);
    return 0;
}