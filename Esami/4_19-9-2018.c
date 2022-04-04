#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<fcntl.h>
#include<signal.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<pthread.h>

int N;
struct sembuf oper;
int sem, semt;
char buff[1024];
int last;
char **file_name;

void printer(int signo){
	char command[1024];
	for(int i=1;i<N;i++){
		sprintf(command, "cat %s", file_name[i]);
		system(command);
	}
	return;
}

void *funct(void *dummy){
	long i=(long)dummy;
	int fd=open(file_name[i+1], O_CREAT|O_RDWR|O_TRUNC, 0660);

	
	while(1){
		oper.sem_num=i;
		oper.sem_op=-1;
		oper.sem_flg=0;
		semop(semt, &oper, 1);
		last++;
		write(fd, buff, 6);
		oper.sem_num=0;
		oper.sem_op=1;
		oper.sem_flg=0;
		semop(sem, &oper, 1);
	}
	exit(0);
}

int main(int argc, char *argv[]){
	if(argc<2){
		printf("Inserire il nome di almeno un file");
		return 1;
	}

	N=argc;
	file_name=argv;
	long i;
	char c;
	last=0;
	pthread_t tid;
	for(i=0;i<N;i++){
		pthread_create(&tid, NULL, funct, (void*)i);
	}

	signal(SIGINT, printer);

	sem=semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
	semt=semget(IPC_PRIVATE, N, IPC_CREAT|0666);
	semctl(sem, 0, SETVAL, 1);
	for(i=0;i<N;i++){
		semctl(IPC_PRIVATE, i, SETVAL, 0);
	}

	while(1){
		oper.sem_num=0;
		oper.sem_op=-1;
		oper.sem_flg=0;
		semop(sem, &oper, 1);
		for(int j=0;j<5;j++){ 
			if((c=getchar())!='\n'){
				buff[j]=c;
			}
		}
		buff[5]='\0';
		oper.sem_num=(last%(N-1));
		oper.sem_op=1;
		oper.sem_flg=0;
		semop(semt, &oper, 1);
	}
	return 0;

}