#include <stdio.h>
#include <stdlib.h>

#include "algorithm/LZW.h"
#include "algorithm/file.h"

#define uint64 unsigned long long int
#define uchar unsigned char


int main(void) {

    // Read file
    uint64 fSize; char *filename = "test.txt";
    uchar *fData = fLoad(filename, &fSize);
    if (fData == NULL) {return 1;}
    // --------
    //Save file
     filename = "arch.mlz";
    fSave(filename, fData, fSize);
    // -------

    return 0;
}
//    uint64 size;
//    uint64 len_encoded, len_decoded;
//// --------------------------
//    uint64 *encoded = (uint64 *) lzwEncode(input, size, &len_encoded);
//    uchar *decoded = lzwDecode(encoded, len_encoded, &len_decoded);
//// --------------------------
//    free(encoded);
//    free(decoded);
