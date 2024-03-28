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

    // Allocating memory on the stack for the cache line chunk we read in, we
    // only want to read in CACHE_LINE_SIZE bits at a time to make this a bit
    // more challenging
    unsigned char data[CACHE_LINE_SIZE];

    /*
    READ IN THE HEADER
    */
    // Reads in CACHE_LINE_SIZE bits from the file and stores it in the data,
    // this moves the internal file pointer to the next CACHE_LINE_SIZE bits
    // so we don't have to keep track of how many bits we've read so far.
    size_t itemsRead = fread(&data, 1, sizeof(data), file);

    // Hexdumps the cache line with relative numbering
    printf("0000 -> ");
    for(int i = 0; i < CACHE_LINE_SIZE; i++) {
        printf("%02x ", data[i]);
        if(i % 8 == 7) {
            printf("\n");
            printf("%04x -> ", i + 1);
        }
    }

    // There are not enough bits in the file to read in a CACHE_LINE_SIZE chunk
    // which would mean that the file is not aligned properly, don't want to
    // support that case so we just exit.
    if (itemsRead != sizeof(data)) {
        printf("Failed to read CACHE_LINE_SIZE bits from the file.\n");
        fclose(file);
        exit(1);
    } 
    
    // Checks if this cacheline is the last one, will need to be able to handle
    // having multiple cache lines back to back which don't end in 0x0a, 
    // in the worst case it'll just read through the entire file.
    if(data[CACHE_LINE_SIZE - 1] != 0x0a) {
        printf("The last byte of the header is not a newline character.\n");
        fclose(file);
        exit(1);
    }

    // Will need to have different reading modes for the cache line, at the
    // start we're trying to parse the first word of the header to check if
    // it's a .npy file. Afterwards we need to write a json parser.
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

    /*
    We know from the "descr" value what kind of bit alignment the contents have
    and we know how to interpret them, so this should be relatively easy.

    Read in 1, 2, 4, 8 bytes depending on how large the datatype is and then
    parse the binary data to the correct type, I think a switch statement would
    be great here, I think we'll just parse and print it for now.

    Supporting different shapes also shouldn't be a problem, but we'll deal with
    that once we get to it.
    */

    fclose(file);
    return 0;
}