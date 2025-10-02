ROOT = /usr/local/arm_linux_4.2/
LIB = $(ROOT)/lib/gcc/arm-linux/4.2.1
LIB1 =$(ROOT)/arm-linux/lib
INC :=$(ROOT)/arm-linux/include
INCMY = ./include
OBJ = ./obj

CC=arm-linux-gcc
CCFLAGS=-O2 -c -I$(INCMY) -I-$(INC) -D_GNU_SOURCE
LDFLAGS=-static -L$(LIB) -L$(LIB1)
STRIP=arm-linux-strip

TARGET = uvtest
OBJS := $(OBJ)/test.o $(OBJ)/rtctime.o \
	$(OBJ)/async.o $(OBJ)/core.o $(OBJ)/fs.o $(OBJ)/fs-poll.o \
	$(OBJ)/getaddrinfo.o $(OBJ)/getnameinfo.o $(OBJ)/inet.o \
	$(OBJ)/linux-core.o $(OBJ)/linux-inotify.o $(OBJ)/linux-syscalls.o \
	$(OBJ)/loop.o $(OBJ)/loop-watcher.o $(OBJ)/pipe.o $(OBJ)/poll.o \
	$(OBJ)/process.o $(OBJ)/procfs-exepath.o $(OBJ)/proctitle.o \
	$(OBJ)/pthread-fixes.o $(OBJ)/signal.o $(OBJ)/stream.o $(OBJ)/sysinfo-loadavg.o \
	$(OBJ)/sysinfo-memory.o $(OBJ)/tcp.o $(OBJ)/thread.o $(OBJ)/threadpool.o $(OBJ)/timer.o \
	$(OBJ)/tty.o $(OBJ)/udp.o $(OBJ)/uv-common.o $(OBJ)/uv-data-getter-setters.o \
	$(OBJ)/version.o \

LIBS= -lpthread -lrt

all: $(TARGET)

clean:
	rm -f $(OBJ)/*.o 
	rm -f *.x 
	rm -f *.flat
	rm -f *.map
	rm -f temp
	rm -f *.img
	rm -f $(TARGET)	
	rm -f *.gdb
	rm -f *.bak

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(TARGET) $(LIBS)
	$(STRIP) $(TARGET) 
	cp $(TARGET) /home/secu/tftpboot

$(OBJ)/test.o:  test.c
	$(CC) $(CCFLAGS) test.c -o $(OBJ)/test.o

$(OBJ)/rtctime.o:  ../mylib/rtctime.c
	$(CC) $(CCFLAGS) ../mylib/rtctime.c -o $(OBJ)/rtctime.o

$(OBJ)/async.o:  async.c
	$(CC) $(CCFLAGS) async.c -o $(OBJ)/async.o

$(OBJ)/core.o:  core.c
	$(CC) $(CCFLAGS) core.c -o $(OBJ)/core.o

$(OBJ)/dl.o:  dl.c
	$(CC) $(CCFLAGS) dl.c -o $(OBJ)/dl.o

$(OBJ)/fs.o:  fs.c
	$(CC) $(CCFLAGS) fs.c -o $(OBJ)/fs.o

$(OBJ)/fs-poll.o:  fs-poll.c
	$(CC) $(CCFLAGS) fs-poll.c -o $(OBJ)/fs-poll.o

$(OBJ)/getaddrinfo.o:  getaddrinfo.c
	$(CC) $(CCFLAGS) getaddrinfo.c -o $(OBJ)/getaddrinfo.o

$(OBJ)/getnameinfo.o:  getnameinfo.c
	$(CC) $(CCFLAGS) getnameinfo.c -o $(OBJ)/getnameinfo.o

$(OBJ)/inet.o:  inet.c
	$(CC) $(CCFLAGS) inet.c -o $(OBJ)/inet.o

$(OBJ)/linux-core.o:  linux-core.c
	$(CC) $(CCFLAGS) linux-core.c -o $(OBJ)/linux-core.o

$(OBJ)/linux-inotify.o:  linux-inotify.c
	$(CC) $(CCFLAGS) linux-inotify.c -o $(OBJ)/linux-inotify.o

$(OBJ)/linux-syscalls.o:  linux-syscalls.c
	$(CC) $(CCFLAGS) linux-syscalls.c -o $(OBJ)/linux-syscalls.o

$(OBJ)/loop.o:  loop.c
	$(CC) $(CCFLAGS) loop.c -o $(OBJ)/loop.o

$(OBJ)/loop-watcher.o:  loop-watcher.c
	$(CC) $(CCFLAGS) loop-watcher.c -o $(OBJ)/loop-watcher.o

$(OBJ)/pipe.o:  pipe.c
	$(CC) $(CCFLAGS) pipe.c -o $(OBJ)/pipe.o

$(OBJ)/poll.o:  poll.c
	$(CC) $(CCFLAGS) poll.c -o $(OBJ)/poll.o

$(OBJ)/process.o:  process.c
	$(CC) $(CCFLAGS) process.c -o $(OBJ)/process.o

$(OBJ)/procfs-exepath.o:  procfs-exepath.c
	$(CC) $(CCFLAGS) procfs-exepath.c -o $(OBJ)/procfs-exepath.o

$(OBJ)/proctitle.o:  proctitle.c
	$(CC) $(CCFLAGS) proctitle.c -o $(OBJ)/proctitle.o

$(OBJ)/pthread-fixes.o:  pthread-fixes.c
	$(CC) $(CCFLAGS) pthread-fixes.c -o $(OBJ)/pthread-fixes.o

$(OBJ)/signal.o:  signal.c
	$(CC) $(CCFLAGS) signal.c -o $(OBJ)/signal.o

$(OBJ)/stream.o:  stream.c
	$(CC) $(CCFLAGS) stream.c -o $(OBJ)/stream.o

$(OBJ)/sysinfo-loadavg.o:  sysinfo-loadavg.c
	$(CC) $(CCFLAGS) sysinfo-loadavg.c -o $(OBJ)/sysinfo-loadavg.o

$(OBJ)/sysinfo-memory.o:  sysinfo-memory.c
	$(CC) $(CCFLAGS) sysinfo-memory.c -o $(OBJ)/sysinfo-memory.o

$(OBJ)/tcp.o:  tcp.c
	$(CC) $(CCFLAGS) tcp.c -o $(OBJ)/tcp.o

$(OBJ)/thread.o:  thread.c
	$(CC) $(CCFLAGS) thread.c -o $(OBJ)/thread.o

$(OBJ)/threadpool.o:  threadpool.c
	$(CC) $(CCFLAGS) threadpool.c -o $(OBJ)/threadpool.o

$(OBJ)/timer.o:  timer.c
	$(CC) $(CCFLAGS) timer.c -o $(OBJ)/timer.o

$(OBJ)/tty.o:  tty.c
	$(CC) $(CCFLAGS) tty.c -o $(OBJ)/tty.o

$(OBJ)/udp.o:  udp.c
	$(CC) $(CCFLAGS) udp.c -o $(OBJ)/udp.o

$(OBJ)/uv-common.o:  uv-common.c
	$(CC) $(CCFLAGS) uv-common.c -o $(OBJ)/uv-common.o

$(OBJ)/uv-data-getter-setters.o:  uv-data-getter-setters.c
	$(CC) $(CCFLAGS) uv-data-getter-setters.c -o $(OBJ)/uv-data-getter-setters.o

$(OBJ)/version.o:  version.c
	$(CC) $(CCFLAGS) version.c -o $(OBJ)/version.o

