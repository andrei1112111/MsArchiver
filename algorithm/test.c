/*
// Read file
uint64 fSize; char *filename = "test.txt";
uchar *fData = fLoad(filename, &fSize);
if (fData == NULL) {return 1;}
 --------
Save file
filename = "arch.mlz";
fSave(filename, fData, fSize);
 -------

    uint64 size;
    uint64 len_encoded, len_decoded;
 --------------------------
    uint64 *encoded = (uint64 *) lzwEncode(input, size, &len_encoded);
    uchar *decoded = lzwDecode(encoded, len_encoded, &len_decoded);
 --------------------------
    free(encoded);
    free(decoded);

int test(int argc, char *argv[]) {
    for (int i = 0; i < 11; i++) {
        printf("|");
        for (int j = 0; j < i; j++) {
            printf("#");
        }
        for (int j = i; j < 11; j++) {
            printf(" ");
        }
        printf("|\r");
        fflush(stdout);
        sleep(1);
    }

} // печать loading bar (работает в консоли)
*/
