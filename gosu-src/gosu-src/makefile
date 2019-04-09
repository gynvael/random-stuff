OBJS	= string_ext.o linked_list.o varlst.o net.o http.o main.o config.o plugins.o script_parser.o gosucmd.o dsdb.o
GOUT	= ../gosu/gosu.exe
CFLAGS	= -Wall -O3 -fno-strict-aliasing -mwindows
NONUSED = -D_WIN32 --pedantic -ansi  -DDEBUG  

%.o : %.c
	gcc -c $< -o $@ $(CFLAGS)

$(GOUT): $(OBJS)
	gcc $(OBJS) -o $(GOUT) -lwsock32 $(CFLAGS) 
	
