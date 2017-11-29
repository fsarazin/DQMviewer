#
# General Makefile for ADST analysis examples
#
USER_SRCS = $(wildcard *.cc)
#
# Executable names come from the .cc sources in this directory.
# Replace the wildcard expression with .cc file list if you do
# not want to compile all .cc files in this directory
#
EXE = $(patsubst %.cc,%, $(wildcard *.cc))
OBJS = $(USER_SRCS:.cc=.o)
#
#############################################################

## You should not need to change anything below this line ###

.PHONY: all depend clean

#ifdef AUGEROFFLINEROOT
#ADSTROOT = $(AUGEROFFLINEROOT)
#endif

#ifndef ADSTROOT
#LEVELUP=/..
#ADSTROOT:=$(shell auger-offline-config --path)$(LEVELUP)
#ifeq ("$(ADSTROOT)", "$(LEVELUP)")
#$(error "Neither ADSTROOT, AUGEROFFLINEROOT are defined nor auger-offline-config is available")
#endif
#endif

CXXFLAGS    = $(shell root-config --cflags)
#CXXFLAGS   += -I$(ADSTROOT)/include/adst
#CXXFLAGS   += -I$(ADSTROOT)/include/
CXXFLAGS   += -std=c++0x
LDFLAGS     = $(shell root-config --ldflags)
LDFLAGS    += $(shell root-config --glibs)
#LDFLAGS    += -L$(ADSTROOT)/lib -lRecEventKG -lAnalysisKG
CXXFLAGS += -O3
#CXX = gcc-4.8
#CXX = g++

all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) -lMinuit 

#############################################################
# gcc can generate the dependency list

depend: Make-depend

Make-depend: $(USER_SRCS)
	$(CPP) $(CXXFLAGS) -MM $^ > $@

clean:
	- rm -f *.o  *.so *.ps core Make-depend $(EXE)

-include Make-depend
