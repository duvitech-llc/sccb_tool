##### Change the following for your environment:
#-include ../Rules.make
#CROSS_COMPILE?=		arm-linux-gnueabihf-
COMPILE_OPTS =		$(INCLUDES) -I. -O2 -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64  -fPIC -D ALLOW_RTSP_SERVER_PORT_REUSE=1 
# C =			c
C_COMPILER =		$(CROSS_COMPILE)gcc
#C_COMPILER =		$(CC)
C_FLAGS =		$(COMPILE_OPTS)
# CPP =			cpp
LDFLAGS= -pthread -lpthread 
# CPLUSPLUS_COMPILER =	$(CROSS_COMPILE)g++
# CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -Wall -DBSD=1
# OBJ =			o
# LINK =			$(CROSS_COMPILE)g++ -o
# LINK_OPTS =
# CONSOLE_LINK_OPTS =	$(LINK_OPTS)
# LIBRARY_LINK =		$(CROSS_COMPILE)ar cr
# LIBRARY_LINK_OPTS =	$(LINK_OPTS)
# LIB_SUFFIX =			a
# LIBS_FOR_CONSOLE_APPLICATION =
# LIBS_FOR_GUI_APPLICATION =
# EXE =

##### End of variables to change


# define any directories containing header files other than /usr/include
#
INCLUDES = -I.\

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS = -L.\

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname 
#   option, something like (this will link in libmylib.so and libm.so:
LIBS = 

# define the C source files
SRCS = sccb_tool.c

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = sccb

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN)
	@echo  sccb has been compiled

$(MAIN): $(OBJS) 
	$(C_COMPILER) $(C_FLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS) $(LDFLAGS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.c.o:
	$(C_COMPILER) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
