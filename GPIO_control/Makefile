PROGRAM_NAME=character-device-control
obj = $(PROGRAM_NAME).c

all:
	$(CC) $(obj) -o $(PROGRAM_NAME)
clean:
	rm -f *.o $(PROGRAM_NAME)
install:
	scp $(PROGRAM_NAME) root@192.168.8.8:/home/root
