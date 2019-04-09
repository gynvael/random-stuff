CC   = gcc.exe
BIN  = ..\..\gosu\plugins\projmgr.dll
CFLAGS = -fexpensive-optimizations -O3
DLLWRAP= dllwrap.exe
DEFFILE= dll.def
API    = ..\..\plugin_api\gosuapi.a

all:
	$(CC) -c dllmain.c -o dllmain.o $(CFLAGS)
	$(DLLWRAP) --def $(DEFFILE) dllmain.o $(API) -o $(BIN) 
	
