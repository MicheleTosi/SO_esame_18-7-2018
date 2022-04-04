/**
SPECIFICATION TO BE IMPLEMENTED:
Implementare un programma che riceva in input tramite argv[] i pathname 
associati ad N file, con N maggiore o uguale ad 1. Per ognuno di questi
file generi un processo che legga tutte le stringhe contenute in quel file
e le scriva in un'area di memoria condivisa con il processo padre. Si 
supponga per semplicita' che lo spazio necessario a memorizzare le stringhe
di ognuno di tali file non ecceda 4KB. 
Il processo padre dovra' attendere che tutti i figli abbiano scritto in 
memoria il file a loro associato, e successivamente dovra' entrare in pausa
indefinita.
D'altro canto, ogni figlio dopo aver scritto il contenuto del file nell'area 
di memoria condivisa con il padre entrera' in pausa indefinita.
L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo padre venga colpito da esso dovra' 
stampare a terminale il contenuto corrente di tutte le aree di memoria 
condivisa anche se queste non sono state completamente popolate dai processi 
figli. **/


#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<fcntl.h>
#include<pthread.h>

char stringa[1024], **file_names;
__thread char* file_name;
int sem, semt;
long i,N;

void printer(int signo){
	char command[1024];
	for(i=1;i<N;i++){
		sprintf(command, "cat %s", file_names[i]);
		system(command);
	}
	return;
}

void *funct(void* dummy){
	long i= (long)dummy;
	struct sembuf oper;

	file_name=file_names[i];
	int fd;
	fd=open(file_name, O_CREAT|O_TRUNC|O_RDWR, 0666);


	while(1){
		oper.sem_num=i;
		oper.sem_op=-1;
		oper.sem_flg=0;
		semop(semt, &oper, 1);
		write(fd, stringa, strlen(stringa)+1);
		oper.sem_num=0;
		oper.sem_op=1;
		oper.sem_flg=0;
		semop(sem, &oper, 1);
	}
	exit(0);

}

int main(int argc, char *argv[]){
	if(argc<2){
		printf("Devi passare almeno il nome di un file\n");
		exit(1);
	}

	N=argc;
	
	pthread_t tid;
	struct sembuf oper;

	signal(SIGINT, printer);

	file_names=argv;
	
	sem=semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
	semt=semget(IPC_PRIVATE, N, IPC_CREAT|0666);
	semctl(sem, 0, SETVAL, N-1);

	for(i=1;i<N;i++){
		semctl(semt, i, SETVAL, 0);
	}

	for(i=1;i<N;i++){
		pthread_create(&tid, NULL, funct, (void*) i);
	}


	while(1){
		oper.sem_num=0;
		oper.sem_op=-N+1;
		oper.sem_flg=0;
		semop(sem, &oper, 1); 
		scanf("%s", stringa);
		oper.sem_op=1;
		oper.sem_flg=0;
		for(i=1;i<N;i++){
			oper.sem_num=i;
			semop(semt, &oper, 1);
		}

	}
	return 0;
}
