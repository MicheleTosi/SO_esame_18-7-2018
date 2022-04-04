#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<pthread.h>
#include<signal.h>
#include<sys/sem.h>
#include<stdlib.h>

int N;
int sem, semt;
char **str;
char stringa[1024];
struct sembuf oper;
FILE* file;

void printer(int signo){
	system("cat output.txt");
}


void *funct(void* dummy){
	long i=(long) dummy;
	while(1){
		oper.sem_num=i;
		oper.sem_op=-1;
		oper.sem_flg=0;
		semop(semt, &oper,1);
		if(i==(N-1)){
			fprintf(file, "%s\n", stringa);
			fflush(file);
			oper.sem_num=0;
			oper.sem_op=1;
			oper.sem_flg=0;
			semop(sem, &oper, 1);
		}else{
			for(int j=0; j<strlen(stringa);j++){
				for(int k=0;k<strlen(str[i]);k++){
					if(stringa[j]==str[i][k]){
						stringa[j]=' ';
					}
				}
			}
			oper.sem_num=i+1;
			oper.sem_op=1;
			oper.sem_flg=0;
			semop(semt, &oper, 1);
		}
	}
	exit(0);
}

int main(int argc, char *argv[]){
	if(argc<2){
		printf("Inserire almeno una stringa\n");
		return 1;
	}

	N=argc+1;
	long i;
	pthread_t tid;
	file=fopen("output.txt", "w+");

	str=argv;

	signal(SIGINT, printer);

	sem=semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
	semt=semget(IPC_PRIVATE, N, IPC_CREAT|0666);

	semctl(sem, 0, SETVAL, 1);

	for(i=1;i<N;i++){
		semctl(semt, i, SETVAL, 0);
	}

	for(i=1;i<N;i++){
		pthread_create(&tid, NULL, funct, (void*)i);
	}

	while(1){
		oper.sem_num=0;
		oper.sem_op=-1;
		oper.sem_flg=0;
		semop(sem, &oper, 1);
		scanf("%s", stringa);
		oper.sem_num=1;
		oper.sem_op=1;
		oper.sem_flg=0;
		semop(semt, &oper, 1);
	}
	return 0;
}