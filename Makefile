## hum2ly GNU makefile 
##
## Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
## Creation Date: Sat Aug  6 10:57:54 CEST 2016
## Last Modified: Sun Sep 18 12:31:52 PDT 2016
## Filename:      xml2hum2/Makefile
##
## Description: This Makefile compiles the xml2hum2 program.
##
## To run this makefile, type (without quotes) "make" (or
## "gmake library" on FreeBSD computers).
##

# targets which don't actually refer to files:
.PHONY: external tests
.SUFFIXES:

OBJDIR    = obj
SRCDIR    = src
INCDIR    = include
TARGDIR   = bin
TARGET    = xml2hum2
INCDIRS   = -I$(INCDIR)
INCDIRS  += -Iexternal/humlib/include 
INCDIRS  += -Isrc 
LIBDIRS   = -Lexternal/humlib/lib
LIBDIRS  += -Lexternal/pugixml
HUMLIB    = humlib
PUGIXML   = pugixml
COMPILER  = g++
PREFLAGS  = -O3 -Wall $(INCDIRS)
POSTFLAGS = $(LIBDIRS) -l$(HUMLIB) -l$(PUGIXML)

# Humlib needs C++11:
PREFLAGS += -std=c++11

# setting up the directory paths to search for dependency files
vpath %.h   $(INCDIR)
vpath %.cpp $(SRCDIR)
vpath %.o   $(OBJDIR)

# generating a list of the object files
#OBJS = $(notdir $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/*.cpp)))
SRCS = $(wildcard $(SRCDIR)/*.cpp)


all: objdir external targetdir
	$(COMPILER) $(PREFLAGS) -o $(TARGDIR)/$(TARGET) $(SRCS) $(POSTFLAGS) \
		&& strip $(TARGDIR)/$(TARGET)

objdir:
	mkdir -p $(OBJDIR)


targetdir:
	mkdir -p $(TARGDIR)


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
	(cd external && $(MAKE) clean)
	-rm -f $(TARGET)


###########################################################################
#                                                                         #
# defining an explicit rule for object file dependencies                  #
#                                                                         #

%.o : %.cpp
	@echo [CC] $@
	@$(COMPILER) $(PREFLAGS) -o $(OBJDIR)/$(notdir $@) $(POSTFLAGS) $<

#                                                                         #
###########################################################################



###########################################################################
#                                                                         #
# Dependencies -- generated with the following command in                 #
#      the src directory (in bash shell):                                 #
#                                                                         #
#   for i in *.cpp                                                        #
#   do                                                                    #
#      cc -I../include -MM $i | sed 's/\.\.\/include\///g'                #
#      echo ""                                                            #
#   done                                                                  #
#                                                                         #
# Or in a csh-type shell (such as tcsh):                                  #
#                                                                         #
#   foreach i (*.cpp)                                                     #
#      cc -I../include -MM $i | sed 's/\.\.\/include\///g'                #
#      echo ""                                                            #
#   end                                                                   #

 
MxmlEvent.o: MxmlEvent.cpp humlib.h MxmlEvent.h pugiconfig.hpp \
  pugixml.hpp MxmlMeasure.h

MxmlMeasure.o: MxmlMeasure.cpp humlib.h MxmlEvent.h pugiconfig.hpp \
  pugixml.hpp MxmlMeasure.h MxmlPart.h

MxmlPart.o: MxmlPart.cpp humlib.h MxmlMeasure.h pugiconfig.hpp \
  pugixml.hpp MxmlPart.h

main.o: main.cpp xml2hum.h humlib.h pugiconfig.hpp pugixml.hpp \
  MxmlPart.h MxmlMeasure.h MxmlEvent.h

xml2hum.o: xml2hum.cpp xml2hum.h humlib.h pugiconfig.hpp \
  pugixml.hpp MxmlPart.h MxmlMeasure.h MxmlEvent.h


