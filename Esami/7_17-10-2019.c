/**
SPECIFICATION TO BE IMPLEMENTED:
Implementare una programma che riceva in input, tramite argv[], il nomi
di N file (con N maggiore o uguale a 1).
Per ogni nome di file F_i ricevuto input dovra' essere attivato un nuovo thread T_i.
Il main thread dovra' leggere indefinitamente stringhe dallo standard-input 
e dovra' rendere ogni stringa letta disponibile ad uno solo degli altri N thread
secondo uno schema circolare.
Ciascun thread T_i a sua volta, per ogni stringa letta dal main thread e resa a lui disponibile, 
dovra' scriverla su una nuova linea del file F_i. 

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito esso dovra' 
riversare su standard-output e su un apposito file chiamato "output-file" il 
contenuto di tutti i file F_i gestiti dall'applicazione 
ricostruendo esattamente la stessa sequenza di stringhe (ciascuna riportata su 
una linea diversa) che era stata immessa tramite lo standard-input.

In caso non vi sia immissione di dati sullo standard-input, l'applicazione dovra' utilizzare 
non piu' del 5% della capacita' di lavoro della CPU.
**/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/sem.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<pthread.h>
#include<signal.h>

int N;
int sem, semt;
int last;
char **file_names;
char stringa[1024];
struct sembuf oper;

void printer(int signo){
	FILE *out, *file;
	int i, ret;
	char cmd[1024];
	char *p;
	ret=0;
	out=fopen("output-file", "w");
	for(i=1;i<N;i++){
		printf("File %s\n", file_names[i]);
		fflush(stdout);
		sprintf(cmd, "cat %s", file_names[i]);
		system(cmd);
		printf("\n");
		file=fopen(file_names[i],"r");
		while(1){
			ret=fscanf(file, "%ms", &p);
			if(ret==EOF) break;
			fprintf(out, "%s", p);
			fflush(out);
			free(p);
		}
	}
}

void* funct(void *dummy){
	long i=(long) dummy;
	FILE *file;
	file=fopen(file_names[i], "w");

	while(1){
		oper.sem_num=i;
		oper.sem_op=-1;
		oper.sem_flg=0;
		semop(semt, &oper, 1);
		fprintf(file, "%s\n", stringa);
		fflush(file);
		last++;
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
		exit(1);
	}

	long i;
	N=argc;
	pthread_t tid;
	last=0;
	file_names=argv;

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
		printf("Tornato\n");
		scanf("%s", stringa);
		oper.sem_num=(last)%(N-1)+1;
		oper.sem_op=1;
		oper.sem_flg=0;
		semop(semt, &oper, 1);
	}
	return 0;

}
