#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
int main()
{
	int numbytes=101;
	char buf[numbytes];
	for(;;) {
		numbytes=read(0,buf, 100);
		buf[numbytes++]=0;
		write(1, buf, numbytes); 
	} 
}
