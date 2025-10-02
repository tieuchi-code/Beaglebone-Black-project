PROGRAM_NAME=LED_control
obj = simple_control.c
all:
	$(CC) $(obj) -o $(PROGRAM_NAME)

clear:
	rm -f *.o $(PROGRAM_NAME)
install:
	scp $(PROGRAM_NAME) root@192.168.8.8:/home/root
