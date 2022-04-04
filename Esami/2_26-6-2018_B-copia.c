#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<semaphore.h>

#define PAGE_SIZE 4096

int N, i, *values;
sem_t *sem, *sem1;
char command[1024];

void funct(){
	int val;

	signal(SIGINT, SIG_IGN);

	while(1){
		scanf("%d", &val);
		sem_wait(sem1);
		*values=val;
		sem_post(sem);
	}
}

void printer(){
	system(command);
	return;
}

int main(int argc, char *argv[]){
	if(argc<3){
		printf("Errore\n");
		exit(1);
	}

	if(N<0){
		printf("Il numero di processi deve essere positivo\n");
		exit(1);
	}

	N=atoi(argv[2]);
	int fd;
	FILE *file;

	values=mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0);

	signal(SIGINT, printer);

	sem=sem_open("/s", O_CREAT, 0666, 0);
	sem1=sem_open("/l", O_CREAT, 0666, 1);

	for(i=0;i<N;i++){
		if(fork()) continue;
		else funct();
	}

	sprintf(command, "cat %s\n", argv[1]);

	fd=open(argv[1], O_CREAT|O_RDWR, 0666);
	file=fdopen(fd, "r+");

	
	while(1){
		sem_wait(sem);
		fprintf(file, "%d ", *values);
		fflush(file);
		sem_post(sem1);
	}
	return 0;

}