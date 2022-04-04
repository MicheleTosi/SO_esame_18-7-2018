/**
SPECIFICATION TO BE IMPLEMENTED: 
Implementare un'applicazione che riceva in input tramite argv[] il 
nome di un file F ed una stringa indicante un valore numerico N maggiore
o uguale ad 1.
L'applicazione, una volta lanciata dovra' creare il file F ed attivare 
N thread. Inoltre, l'applicazione dovra' anche attivare un processo 
figlio, in cui vengano attivati altri N thread. 
I due processi che risulteranno attivi verranno per comodita' identificati
come A (il padre) e B (il figlio) nella successiva descrizione.

Ciascun thread del processo A leggera' stringhe da standard input. 
Ogni stringa letta dovra' essere comunicata al corrispettivo thread 
del processo B tramite memoria condivisa, e questo la scrivera' su una 
nuova linea del file F. Per semplicita' si assuma che ogni stringa non
ecceda la taglia di 4KB. 

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo A venga colpito esso dovra' 
inviare la stessa segnalazione verso il processo B. Se invece ad essere 
colpito e' il processo B, questo dovra' riversare su standard output il 
contenuto corrente del file F.

Qalora non vi sia immissione di input, l'applicazione dovra' utilizzare 
non piu' del 5% della capacita' di lavoro della CPU. 
**/

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
