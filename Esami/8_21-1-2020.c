/**
SPECIFICATION TO BE IMPLEMENTED:
Implementare una programma che ricevento in input tramite argv[] una stringa S
esegua le seguenti attivita'.
Il main thread dovra' attivare due nuovi thread, che indichiamo con T1 e T2.
Successivamente il main thread dovra' leggere indefinitamente caratteri dallo 
standard input, a blocchi di 5 per volta, e dovra' rendere disponibili i byte 
letti a T1 e T2. 
Il thread T1 dovra' inserire di volta in volta i byte ricevuti dal main thread 
in coda ad un file di nome S_diretto, che dovra' essere creato. 
Il thread T2 dovra' inserirli invece nel file S_inverso, che dovra' anche esso 
essere creato, scrivendoli ogni volta come byte iniziali del file (ovvero in testa al 
file secondo uno schema a pila).

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito esso dovra' 
calcolare il numero dei byte che nei due file hanno la stessa posizione ma sono
tra loro diversi in termini di valore. Questa attivita' dovra' essere svolta attivando 
per ogni ricezione di segnale un apposito thread.

In caso non vi sia immissione di dati sullo standard input, l'applicazione dovra' 
utilizzare non piu' del 5% della capacita' di lavoro della CPU.
**/

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<string.h>
#include<sys/types.h>
#include<signal.h>
#include<pthread.h>

int sem, semt;
struct sembuf oper;
char buffer[1024];
int lend, leni;

void printer(int signo){
	FILE *diretto, *inverso;
	char dir, inv;
	int count=0;
	diretto=fopen("S_diretto","r");
	inverso=fopen("S_inverso","r");
	while(1){
		dir=fgetc(diretto);
		inv=fgetc(inverso);
		if(dir==EOF || inv==EOF) break;
		if(dir!=inv) count++;
	}
	printf("%d caratteri stessa posizione diverso valore\n", count);


}

void *inverso(void* dummy){
	FILE* file;
	int i;
	file=fopen("S_inverso", "w");

	while(1){
		oper.sem_num=1;
		oper.sem_op=-1;
		oper.sem_flg=0;
		semop(semt, &oper, 1);
		for(i=0;i<strlen(buffer);i++){
			printf("Partito buffer: %c, %ld\n", buffer[4-i], strlen(buffer));
			fprintf(file, "%c", buffer[4-i]);
			rewind(file);
			
		}
		fflush(file);
		oper.sem_num=0;
		oper.sem_op=1;
		oper.sem_flg=0;
		semop(sem, &oper, 1);
	}
}

void *diretto(void* dummy){
	FILE *file;
	char *temp;
	int i=0;
	file=fopen("S_diretto", "w+");
	lend=0;
	char cmd[1024];


	while(1){
		//rewind(file);
		oper.sem_num=0;
		oper.sem_op=-1;
		oper.sem_flg=0;
		semop(semt, &oper, 1);
		/*temp=(char*)malloc(lend);
		while(1){
			if(lend==0) break;
			if((c=fgetc(file))==EOF) break;
			temp[i]=c;
			i++;
		}*/
		//rewind(file);
		/*printf("Partito diretto: %s\n", temp);
		fprintf(file, "%s", buffer);
		fprintf(file, "%s", temp);
		fflush(file);
		rewind(file);
		free(temp);
		lend+=strlen(buffer);*/
		sprintf(cmd, "sed -i \"1i\\ %s\" ./S_inverso", buffer);
		printf("comando: %s\n", cmd);
		system(cmd);
		oper.sem_num=0;
		oper.sem_op=1;
		oper.sem_flg=0;
		semop(sem, &oper, 1);
	}
}

int main(int argc, char *argv[]){
	if(argc<2){
		printf("Inserire una stringa da passare ai thread\n");
		exit(1);
	}

	int i, ret;
	pthread_t tid;
	signal(SIGINT, printer);

	pthread_create(&tid, NULL, diretto, NULL);
	pthread_create(&tid, NULL, inverso, NULL);

	sem=semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
	semt=semget(IPC_PRIVATE, 2, IPC_CREAT|0666);
	semctl(sem, 0, SETVAL, 2);
	semctl(semt, 0, SETVAL, 0);
	semctl(semt, 1, SETVAL, 0);

	while(1){
		oper.sem_num=0;
		oper.sem_op=-2;
		oper.sem_flg=0;
		semop(sem, &oper, 1);
		/*for(i=0;i<5;i++){
			buffer[i]=getchar();
		}
		buffer[5]='\0';*/
		ret=scanf("%5c", buffer);
		buffer[ret*8]='\0';
		printf("stampa:%s\n", buffer);
		oper.sem_num=0;
		oper.sem_op=1;
		oper.sem_flg=0;
		semop(semt, &oper, 1);
		oper.sem_num=1;
		semop(semt, &oper, 1);
	}

	return 0;

}
