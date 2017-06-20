## musicxml2hum GNU makefile 
##
## Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
## Creation Date: Sat Aug  6 10:57:54 CEST 2016
## Last Modified: Tue Jun 20 11:54:25 CEST 2017
## Filename:      musicxml2hum/Makefile
##
## Description: This Makefile compiles the musicxml2hum program.
##
## To run this makefile, type (without quotes) "make" (or
## "gmake library" on FreeBSD computers).
##

# targets which don't actually refer to files:
.PHONY: tests 

.SUFFIXES:

SRCDIR    = src
INCDIR    = include
BINDIR    = bin
COMPILER  = g++
TARGET    = musicxml2hum
PREFLAGS  = -Wall -O3 -I$(INCDIR)
POSTFLAGS = 

# Humlib needs C++11:
PREFLAGS += -std=c++11

FILES    = src/humlib.cpp src/musicxml2hum.cpp src/pugixml.cpp

all: bindir update compile


bindir:
	@mkdir -p $(BINDIR)


download: update
update:
	@(cd src; ./.download);
	@(cd include; ./.download);


install:
	sudo cp bin/musicxml2hum /usr/local/bin/


compile: bindir
	@echo [CC] $(BINDIR)/$(TARGET)
	@$(COMPILER) $(PREFLAGS) $(FILES) -o $(BINDIR)/$(TARGET) && \
		strip $(BINDIR)/$(TARGET)
	@echo Executable created in $(BINDIR)/$(TARGET)
	@echo Type "[32mmake install[0m" to copy to /usr/local/bin.


tests:
	for i in tests/*.xml; do ./$(TARGET) $$i > tests/`basename $$i .xml`.krn; done


