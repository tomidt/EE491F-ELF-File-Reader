###############################################################################
#   University of Hawaii, College of Engineering
#   readelf - SRE - Spring 2024
#
### Build and test an ELF parser
###
### @see     https://www.gnu.org/software/make/manual/make.html
###
### @file    Makefile
### @author  Dustin Tomi <tomid@hawaii.edu>
###############################################################################


TARGET_H = readelf_h
TARGET_S = readelf_s

all: $(TARGET_H) $(TARGET_S)

CC     = gcc
CFLAGS = -Wall $(DEBUG_FLAGS)

debug: DEBUG_FLAGS = -g -DDEBUG
debug: clean $(TARGET_H) $(TARGET_S)

readelf_h: readelf.c
	$(CC) $(CFLAGS) -DHEADER -o $(TARGET_H) readelf.c

readelf_s: readelf.c
	$(CC) $(CFLAGS) -DSYMBOL -o $(TARGET_S) readelf.c

test: $(TARGET_H) $(TARGET_S)
	./$(TARGET_H) wc
	./$(TARGET_S) wc

clean:
	rm -f $(TARGET) *.o

LINT      = clang-tidy
LINTFLAGS = --extra-arg=-DPAST

lint: $(TARGET_H) $(TARGET_S)
	$(LINT) $(LINTFLAGS) *.c -- 2> /dev/null