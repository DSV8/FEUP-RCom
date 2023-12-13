CC = gcc
CFLAGS = -Wall

SRC = src/

download: $(SRC)download.c
	$(CC) $(CFLAGS) -o download $(SRC)download.c

testing:  $(SRC)testing.c
	$(CC) $(CFLAGS) -o testing $(SRC)testing.c

clean:
	rm -rf download
	rm -rf testing