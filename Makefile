CC = gcc
CFLAGS = -o2
# -Wall -Wextra -std=c99
OUTNAME = MsArchiver
main:
	$(CC) $(CFLAGS) -o $(OUTNAME) main.c algorithm/file.c algorithm/LZW.c -pthread
