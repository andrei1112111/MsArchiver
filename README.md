# The .mlz simple archiver
#
#### If you want to use archiver as a terminal app, just open it without some parameters.
#### To use it as utility just write some parameters you want.
### utility mode usage:

```
PARAMETER         DESCRIPTION                                                           
-h, --help        This help. (only this one will be printed)                            
-f, --file        Input file. Multiple files separated by a space are allowed              
-e, --encode      Input files will be compressed (Files with the .mlz extension will be  
                                             compressed. The rest will remain untouched)
-d, --decode      Input files will be decompressed                                       
-c, --check       Returns the contents of the archives. (works only for .mlz files)      
```
#
#### Compression algorithm based on [THIS ARTICLE](https://en.wikipedia.org/wiki/Lempel–Ziv–Welch).
