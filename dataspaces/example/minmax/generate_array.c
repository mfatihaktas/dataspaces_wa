#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	
	if(argc!=2){
		printf("Usage: %s <size>\n",argv[0]);
		return 0;
	}


	int size = atoi(argv[argc-1]);	

	for(int i = 0;i<size;i++){
		printf("%d\n", rand()%65536);
	}

	return 0;
}
