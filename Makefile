## hum2ly GNU makefile 
##
## Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
## Creation Date: Sat Aug  6 10:57:54 CEST 2016
## Last Modified: Sun Sep 18 12:31:52 PDT 2016
## Filename:      musicxml2hum/Makefile
##
## Description: This Makefile compiles the musicxml2hum program.
##
## To run this makefile, type (without quotes) "make" (or
## "gmake library" on FreeBSD computers).
##

# targets which don't actually refer to files:
.PHONY: external tests humlib pugixml
.SUFFIXES:

OBJDIR    = obj
SRCDIR    = src
INCDIR    = include
BINDIR    = bin
TARGET    = musicxml2hum
INCDIRS   = -I$(INCDIR)
INCDIRS  += -Iexternal/humlib/include 
INCDIRS  += -Iexternal/pugixml/src 
LIBDIRS   = -Lexternal/humlib/lib
LIBDIRS  += -Lexternal/pugixml
HUMLIB    = humlib
PUGIXML   = pugixml
COMPILER  = g++
#PREFLAGS  = -O3 -Wall $(INCDIRS)
PREFLAGS  = -Wall $(INCDIRS)
POSTFLAGS = $(LIBDIRS) -l$(HUMLIB) -l$(PUGIXML)

# Humlib needs C++11:
PREFLAGS += -std=c++11

# setting up the directory paths to search for dependency files
vpath %.h   $(INCDIR)
vpath %.cpp $(SRCDIR)
vpath %.o   $(OBJDIR)

# generating a list of the object files
OBJS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/*.cpp)))


all: objdir bindir external $(OBJS)
	@echo [CC] $(BINDIR)/$(TARGET)
	@$(COMPILER) $(PREFLAGS) -o $(BINDIR)/$(TARGET) $(OBJS) $(POSTFLAGS) 
#		&& strip $(BINDIR)/$(TARGET)

objdir:
	mkdir -p $(OBJDIR)


bindir:
	mkdir -p $(BINDIR)

update:
	git pull

install:
	sudo cp bin/musicxml2hum /usr/local/bin/

external:
ifeq ($(wildcard external/humlib/lib/libhumlib.a),)
	(cd external && $(MAKE) humlib)
endif
ifeq ($(wildcard libpugixml.a),)
	(cd external && $(MAKE) pugixml)
endif


tests:
	for i in tests/*.xml; do ./$(TARGET) $$i > tests/`basename $$i .xml`.krn; done


clean:
	-rm $(OBJDIR)/*.o
	-rmdir $(OBJDIR)


superclean: clean
	(cd external && $(MAKE) clean)
	-rm $(BINDIR)/$(TARGET)
	-rmdir $(BINDIR)


###########################################################################
#                                                                         #
# defining an explicit rule for object file dependencies                  #
#                                                                         #

$(OBJDIR)/%.o : %.cpp
	@echo [CC] $@
	@$(COMPILER) $(PREFLAGS) -g -c -o $(OBJDIR)/$(notdir $@) $<


#                                                                         #
###########################################################################


