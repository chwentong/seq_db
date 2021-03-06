#
# Makefile Template
#
# @COPYLEFT
# ALL WRONGS RESERVED
#
# Chen Wentong <chwentong@gmail.com>
#

# Source list, 
SOURCES = mpool.c mpool_t.c
# Header list, for install library only
HEADERS	= mpool.h
# Object list, one or more items, 
OBJECTS	= mpool.o
# Program list, one or more items, 
PROGRAM = 
# Distribute library, one item only, 
LIBRARY = libmpool.a
# Programs for test use, one or more items, 
TEST	= mpool_t
# Compile flags, 
CFLAGS	= -I./ -Wall -O2 $(DEFINES)
# Library options, 
LDFLAGS	= 
# Sub directory list
SUBDIRS = 
# Install directory, 
DESTDIR	= /idv

# Default target, could be one or more of 'program/library/tests', plus recursive
ALL:	recursive library

# All targets in this makefile
ALLS:	recursive program library tests

# Try to make depends, indent your code and clean your workspace
CLEAN:	depend indent clean


##################################################################
# You should not modify below lines unless you really need to. #
##################################################################

CC	= gcc
RM  = rm -rf
MAKEDEP	= makedepend
RANLIB	= ranlib
INDENT	= indent

recursive: $(SUBDIRS)
	for sd in $(SUBDIRS); do \
		make -C $$sd; \
	done

clean:
	$(RM) a.out core *.o *.t *.bak tt.* *~
	$(RM) $(PROGRAM) $(TEST) $(LIBRARY)

install:
	if [ -n "$(PROGRAM)" ]; then \
		install -m 0755 $(PROGRAM) $(DESTDIR)/bin; \
	fi
	
	if [ -n "$(LIBRARY)" ]; then \
		install -m 0644 $(LIBRARY) $(DESTDIR)/lib; \
	fi
	
	fi [ -n "$(HEADERS)" ]; then \
		install -m 0644 $(HEADERS) $(DESTDIR)/include; \
	fi

library: $(OBJECTS)
	$(AR) cr $(LIBRARY) $?
	$(RANLIB) $(LIBRARY)

program: $(foreach t, $(PROGRAM), $(t).o) $(OBJECTS)
	for p in $(PROGRAM); do \
		$(CC) -o $$p $$p.o $(OBJECTS) $(LDFLAGS); \
	done

tests: $(foreach t, $(TEST), $(t).o) $(OBJECTS)
	for t in $(TEST); do \
		$(CC) -o $$t $$t.o $(OBJECTS) $(LDFLAGS); \
	done

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

depend:
	$(MAKEDEP) -Y -- $(CFLAGS) -- $(SOURCES) 2>/dev/null

indent:
	$(INDENT) -linux *.c

.PHONY: depend indent clean recursive

# Caution: ~make depend depends on the following lines!~
# DO NOT DELETE

mpool.o: mpool.h mpool_loc.h
mpool_t.o: mpool.h mpool_loc.h
