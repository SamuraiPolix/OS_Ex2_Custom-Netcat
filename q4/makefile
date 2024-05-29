all : mync mync2 talker listener

clean : 
	rm mync mync2 talker listener

.PHONY : all clean

mync : 
	gcc -g mync.c -o mync

mync2 : 
	gcc -g mync2.c -o mync2

talker : 
	gcc -g talker.c -o talker

listener : 
	gcc -g listener.c -o listener

