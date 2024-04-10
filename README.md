
# MsArchiver

A simple cross-platform archiver with support for the native .mlz format. It can be used as a console utility with a simple user interface or without it.

⚠️ Tested only on linux and macOS.

## Features

* Work in the user interaction mode using the console.
* Run from the command line with startup parameters (No interface)
* Compressing files into archive
* Extracting files from archive
* View archive contents
### Supported formats:
* Files to compress:
    *  any formats.
* Archives:
    * only native .mlz.


## Deployment

To deploy this project run

```sh
gcc -o MsArchiver main.c algorithm/file.c algorithm/LZW.c -O2 -pthread
```

## Usage/Examples


### Compression

Terminal Input:
```sh
./MsArchiver -e -f 
"test_files/harry potter/Philosophers Stone.doc"
"test_files/harry potter/Philosophers Stone.epub"
"test_files/harry potter/Philosophers Stone.fb2"
```
output ->
```sh
|##########| 3/3
Complete
```


### Get archive content
Terminal Input:
```sh
./MsArchiver -c -f archive.mlz
```
output ->
```sh
IN archive.mlz
.________________________________.
COMPRESSED      SIZE      FILENAME
+               599 KB    Philosophers Stone.doc
-               619 KB    Philosophers Stone.epub
+               420 KB    Philosophers Stone.fb2
.________________________________.

Complete
```


### Decompression
Terminal Input:
```sh
./MsArchiver -d -f archive.mlz
```
output ->
```sh
|##########| 1/1
Complete
```


### Get some help
Terminal Input:
```sh
./MsArchiver -h 
```
output ->
```sh
Usage:
PARAMETER         DESCRIPTION                                                           
-h, --help        This help. (only this one will be printed)                            
-f, --file        Input file. Multiple files separated by a space are allowed             
-d, --decode      Input files will be decompressed (Files with the .mlz extension
                     will be decompressed. The rest will remain untouched)
-e, --encode      Input files will be compressed                                         
-c, --check       Returns the contents of the archives. (works only for .mlz files)   
```
## Made with
Compression algorithm based on [THIS ARTICLE](https://en.wikipedia.org/wiki/Lempel–Ziv–Welch).