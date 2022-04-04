#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<signal.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<pthread.h>

int N;
char **stringhe;
char buffer[1024];
int sem, semt;
struct sembuf oper;

void printer(int signo){
	char command[1024];
	printf("Stampando il contenuto del file\n");
	sprintf(command, "cat %s", stringhe[1]);
	system(command);
	printf("-----------------------------------------------------------------\n");
	for(int i=2;i<N;i++){
		semctl(semt, i, SETVAL, 0);
	}
	return;
}

void *funct(void* dummy){
	long i= (long) dummy;
	while(1){
		oper.sem_num=i;
		oper.sem_op=-1;
		oper.sem_flg=0;
		semop(semt, &oper, 1);
		if(strcmp(buffer, stringhe[i])!=0 && (i)<N){
			oper.sem_num=i+1;
			oper.sem_op=1;
			oper.sem_flg=0;
			semop(semt, &oper, 1);
			continue;
		}
		if(strcmp(buffer, stringhe[i])==0){
			for(int j=0; j<strlen(buffer);j++){
				buffer[j]='*';
			}
		}

		oper.sem_num=0;
		oper.sem_op=1;
		oper.sem_flg=0;
		semop(sem, &oper, 1);
	}
}

int main(int argc, char* argv[]){
	if(argc<3){
		printf("Inserire almeno il nome del file e una stringa\n");
		return 1;
	}

	N=argc;
	long i;
	pthread_t tid;
	FILE* file;

	stringhe=argv;
	signal(SIGINT, printer);

	file=fopen(argv[1], "w");

	sem=semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
	semt=semget(IPC_PRIVATE, N, IPC_CREAT|0666);

	semctl(sem, 0, SETVAL, 1);

	for(i=2;i<N;i++){
		semctl(semt, i, SETVAL, 0);
	}


	for(i=2;i<N;i++){
		pthread_create(&tid, NULL, funct, (void*)i);
	}

	while(1){
		oper.sem_num=0;
		oper.sem_op=-1;
		oper.sem_flg=0;
		semop(sem, &oper, 1);
		fprintf(file, "%s\n", buffer);
		fflush(file);
		scanf("%s", buffer);
		oper.sem_num=2;
		oper.sem_op=1;
		oper.sem_flg=0;
		semop(semt, &oper, 1);
	}

}