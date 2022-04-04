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

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include<string.h>
#include<sys/mman.h>


int N;
int sd, c, fd;
struct sembuf oper;
void **mem;
char *segment;

void funct(){
	FILE* f;
	f=fdopen(fd, "r");
	while(fscanf(f, "%s", segment)!=EOF){
		printf("read %s\n", segment);
		segment+=strlen(segment);
	}

	oper.sem_num=0;
	oper.sem_op=1;
	oper.sem_flg=0;
	semop(sd, &oper, 1);

	while(1) pause();
}

void printer(){
	int i;
	for(i=1;i<N;i++){
		segment=mem[i];
		while(strcmp(segment, "\0")!=0){
			printf("%s\n", segment);
			segment+=strlen(segment)+1;
		}
	}

}

int main(int argc, char *argv[]){
	if(argc<2){
		printf("Inserire almeno il path di un file\n");
		exit(1);
	}

	int i;
	N=argc;

	for(i=1;i<N;i++){
		fd=open(argv[i], O_RDONLY);
	}

	sd=semget(IPC_PRIVATE, 1, 0666);
	semctl(sd, 0, SETVAL, 0);

	mem= malloc((argc)*sizeof(void*));
	signal(SIGINT, printer);

	for(i=1;i<N;i++){
		mem[i]=mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0);
		segment=mem[i];
		fd=open(argv[i], O_RDONLY);
		if(fork()) continue;
		else funct();
	}

	oper.sem_num=0;
	oper.sem_op=1-N;
	oper.sem_flg=0;
	semop(sd, &oper, 1);

	while(1) pause();
	return 0;

}
