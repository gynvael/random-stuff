CC   	= gcc
BDIR 	= ..
BIN  	= $(BDIR)/gosuapi.a
CFLAGS 	=  -ansi -fexpensive-optimizations -O3
DLLWRAP = dllwrap.exe
DEFFILE = libcmp.def
HEADER 	= gosuapi.h
CP	= copy
ODIR	= ../../gosu-src
OBJS	= gosuapi.o $(ODIR)/config.o $(ODIR)/string_ext.o $(ODIR)/linked_list.o $(ODIR)/varlst.o
AR	= ar rcs

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	$(AR) $@ $(OBJS)
	$(CP) $(HEADER) $(BDIR) > nul
	
