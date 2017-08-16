TARGET:=main
OBJECTS:=file.o server.o protocol.o list.o smain.o
CC:=gcc
OPTIONS:=-g -pthread

	
$(TARGET):$(OBJECTS)
	$(CC) $^ -o $@ $(OPTIONS)
%.o:%.c
	$(CC) -c $< -o $@ $(OPTIONS)

	
clean:
	rm -rvf *.o $(TARGET)
#main:a.o b.o
#	gcc a.o b.o -o main
#a.o:a.c
#	gcc -c a.c -o a.o
#b.o:b.c
#	gcc -c b.c -o b.o
