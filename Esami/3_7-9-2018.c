#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<pthread.h>

#define PAGE_SIZE 4096

int N, fd;
int sem1, sem2;
void** mem;
char *file_name;
struct sembuf oper;
int p;

void resignal(int signo){
	kill(p, SIGINT);
}

void printer(int signo){
	char command[1024];
	sprintf(command, "cat %s", file_name);
	system(command);
	return;
}

void *A(void* dummy){
	long i=(long)dummy;
	while(1){
		oper.sem_num=i;
		oper.sem_op=-1;
		oper.sem_flg=0;
		semop(sem1, &oper, 1);
		scanf("%s ", (char*)mem[i]);
		oper.sem_num=i;
		oper.sem_op=1;
		oper.sem_flg=0;
		semop(sem2, &oper, 1);
	}
	exit(0);

}

void* B(void* dummy){
	long i=(long)dummy;
	FILE *file;
	file=fdopen(fd, "w");
	while(1){
		oper.sem_num=i;
		oper.sem_op=-1;
		oper.sem_flg=0;
		semop(sem2, &oper, 1);
		write(fd,(char*)mem[i], strlen((char*)mem[i])+1);
		fflush(file);
		oper.sem_num=i;
		oper.sem_op=1;
		oper.sem_flg=0;
		semop(sem1, &oper, 1);
	}
	exit(0);

}

int main(int argc, char* argv[]){
	if(argc<3){
		printf("Inserire 3 argomenti\n");
		exit(1);
	}

	N=atoi(argv[2]);
	if(N<1){
		printf("N deve essere almeno 1\n");
		exit(1);
	}
	long i;
	int pid;
	pthread_t tid;

	mem=malloc(N*sizeof(char*));

	for(i=0;i<N;i++){
		mem[i]=(char*)mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
	}
	

	fd=open(argv[1], O_CREAT|O_TRUNC|O_RDWR, 0660);

	file_name=argv[1];

	sem1=semget(IPC_PRIVATE, N, IPC_CREAT|0666);
	sem2=semget(IPC_PRIVATE, N, IPC_CREAT|0666);

	for(i=0;i<N;i++){
		semctl(sem1, i, SETVAL, 1);
		semctl(sem2, i , SETVAL, 0);
	}

	if(pid=fork()){
		p=pid;
		signal(SIGINT, resignal);

		for(i=0;i<N;i++){
			pthread_create(&tid, NULL, A, (void*) i);
		}


	}else{
		signal(SIGINT, printer);
		for(i=0; i<N;i++){
			pthread_create(&tid, NULL, B, (void*) i);
		}
	}
	while(1) pause();

}