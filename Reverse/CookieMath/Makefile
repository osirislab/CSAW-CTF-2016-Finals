CC=gcc
CFLAGS=-I. -std=c99 -m32 -Wall -O1

all: packer

debug: CFLAGS += -DDEBUG -g
debug: packer

release: packer
	cp packer packer-release
	strip packer-release

packer: packer.c
	$(CC) $(CFLAGS) -o packer packer.c
