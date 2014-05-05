CC = gcc
LD = gcc
SRCS = $(wildcard *.c)
OBJS = $(patsubst %c, %o, $(SRCS))
TARGET = simfs
CFLAGS = -g -ggdb3

#all:
#	@echo $(SRCS)
#	@echo $(OBJS)
#	@echo $(TARGET)
#	@echo %.o
#	@echo $^ $@
#	@echo "end"
	
	
.PHONY: all clean
all: $(TARGET)
$(TARGET):$(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^
%.o:%.c
	$(CC) $(CFLAGS) -c  $^ -o $@
clean:
	rm -rf *.o $(TARGET) _VIRTUAL_DISK_IMG
test:
	./simfs
release:

