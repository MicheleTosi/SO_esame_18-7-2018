/**
SPECIFICATION TO BE IMPLEMENTED:
Implementare una programma che riceva in input, tramite argv[], un insieme di
stringhe S_1 ..... S_n con n maggiore o uguale ad 1. 
Per ogni stringa S_i dovra' essere attivato un thread T_i.
Il main thread dovra' leggere indefinitamente stringhe dallo standard-input.
Ogni stringa letta dovra' essere resa disponibile al thread T_1 che dovra' 
eliminare dalla stringa ogni carattere presente in S_1, sostituendolo con il 
carattere 'spazio'.
Successivamente T_1 rendera' la stringa modificata disponibile a T_2 che dovra' 
eseguire la stessa operazione considerando i caratteri in S_2, e poi la passera' 
a T_3 (che fara' la stessa operazione considerando i caratteri in S_3) e cosi' 
via fino a T_n. 
T_n, una volta completata la sua operazione sulla stringa ricevuta da T_n-1, dovra'
passare la stringa ad un ulteriore thread che chiameremo OUTPUT il quale dovra' 
stampare la stringa ricevuta su un file di output dal nome output.txt.
Si noti che i thread lavorano secondo uno schema pipeline, sono ammesse quindi 
operazioni concorrenti su differenti stringhe lette dal main thread dallo 
standard-input.

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito esso dovra' 
stampare il contenuto corrente del file output.txt su standard-output.

In caso non vi sia immissione di dati sullo standard-input, l'applicazione 
dovra' utilizzare non piu' del 5% della capacita' di lavoro della CPU.
**/

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
