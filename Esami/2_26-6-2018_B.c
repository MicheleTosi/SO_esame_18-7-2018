/**
SPECIFICATION TO BE IMPLEMENTED:
Implementare un programma che riceva in input tramite argv[2] un numero
intero N maggiore o uguale ad 1 (espresso come una stringa di cifre 
decimali), e generi N nuovi processi. Ciascuno di questi leggera' in modo 
continuativo un valore intero da standard input, e lo comunichera' al
processo padre tramite memoria condivisa. Il processo padre scrivera' ogni
nuovo valore intero ricevuto su di un file, come sequenza di cifre decimali. 
I valori scritti su file devono essere separati dal carattere ' ' (blank).
Il pathname del file di output deve essere comunicato all'applicazione 
tramite argv[1].
L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che se il processo padre venga colpito il contenuto
del file di output venga interamente riversato su standard-output.
Nel caso in cui non vi sia immissione in input, l'applicazione non deve 
consumare piu' del 5% della capacita' di lavoro della CPU.
**/

#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/mman.h>
#include<fcntl.h>

#define PAGE_SIZE 4096

int N, sem, *values;
char command[1024];

void funct(){
	int val;
	struct sembuf oper;

	signal(SIGINT, SIG_IGN);

	while(1){
		scanf("%d", &val);
		oper.sem_num=1;
		oper.sem_op=-1;
		oper.sem_flg=SEM_UNDO;
		semop(sem, &oper, 1);

		*values=val;

		oper.sem_num=0;
		oper.sem_op=1;
		oper.sem_flg=SEM_UNDO;
		semop(sem, &oper, 1);
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

	N=atoi(argv[2]);
	FILE *file;
	key_t key=1234;
	int fd;
	struct sembuf oper;



	values=mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0);

	sem=semget(key, 2, IPC_CREAT|0666);
	semctl(sem, 2, IPC_RMID, NULL);
	sem=semget(key, 2, IPC_CREAT|0666);

	semctl(sem, 0, SETVAL, 0);
	semctl(sem, 1, SETVAL, 1);

	signal(SIGINT, printer);

	for(int i=0; i<N; i++){
		if(fork())continue;
		else funct();
	}

	sprintf(command, "cat %s", argv[1]);

	fd=open(argv[1], O_CREAT|O_RDWR, 0666);
	file=fdopen(fd, "r+");

	while(1){
		oper.sem_num=0;
		oper.sem_op=-1;
		oper.sem_flg=SEM_UNDO;
		semop(sem, &oper, 1);
		fprintf(file, "%d ", *values);
		fflush(file);
		oper.sem_num=1;
		oper.sem_op=1;
		oper.sem_flg=SEM_UNDO;
		semop(sem, &oper, 1);
	}
	return 0;

}

