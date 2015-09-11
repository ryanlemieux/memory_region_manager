# Regions Makefile

CC = clang
CFLAGS = -Wall -DNDEBUG

PROG = regions
HDRS = region_list.h block_list.h globals.h
SRCS = regions.c region_list.c block_list.c main.c

OBJDIR = object
OBJS = $(OBJDIR)/regions.o $(OBJDIR)/region_list.o $(OBJDIR)/block_list.o $(OBJDIR)/main.o

# compiling rules

# WARNING: *must* have a tab before each definition
$(PROG): $(OBJS) $(OBJDIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROG)

$(OBJDIR)/regions.o: regions.c $(HDRS) $(OBJDIR)
	$(CC) $(CFLAGS) -c regions.c -o $(OBJDIR)/regions.o

$(OBJDIR)/region_list.o: region_list.c $(HDRS) $(OBJDIR)
	$(CC) $(CFLAGS) -c region_list.c -o $(OBJDIR)/region_list.o

$(OBJDIR)/block_list.o: block_list.c $(HDRS) $(OBJDIR)
	$(CC) $(CFLAGS) -c block_list.c -o $(OBJDIR)/block_list.o

$(OBJDIR)/main.o: main.c $(HDRS) $(OBJDIR)
	$(CC) $(CFLAGS) -c main.c -o $(OBJDIR)/main.o

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm -f $(PROG) $(OBJS)

