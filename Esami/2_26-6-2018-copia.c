/**
SPECIFICATION TO BE IMPLEMENTED:
Implementare un programma che riceva in input tramite argv[1] un numero
intero N maggiore o uguale ad 1 (espresso come una stringa di cifre 
decimali), e generi N nuovi thread. Ciascuno di questi, a turno, dovra'
inserire in una propria lista basata su memoria dinamica un record
strutturato come segue:

typedef struct _data{
	int val;
	struct _data* next;
} data; 

I record vengono generati e popolati dal main thread, il quale rimane
in attesa indefinita di valori interi da standard input. Ad ogni nuovo
valore letto avverra' la generazione di un nuovo record, che verra'
inserito da uno degli N thread nella sua lista. 
L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito esso dovra' 
stampare a terminale il contenuto corrente di tutte le liste (ovvero 
i valori interi presenti nei record correntemente registrati nelle liste
di tutti gli N thread). 
**/

#include<stdio.h>
#include<unistd.h>
#include<fctrl.h>
#include<signal.h>
#include<stdlib.h>
#include<pthread.h>

typedef struct _data{
	int val;
	struct _data* next;
}data;

int N, val;
pthread_mutext_t lock, next;
data *lists;

void *funct((void*)i){
	data *aux;

	while(1){
		aux=malloc(sizeof(data));
		pthread_mutex_lock(&lock);
		aux->val=val;
		pthread_mutex_unlock(&next);
		aux->next=lists[(long)i].next;
		lists[(long)i].next=aux;
	}
	return 0;
}

void printer(int signo, siginfo_t *a, void* b){
	data *aux;
	int i;

	for(i=0;i++;i<N){
		aux=list[i];
		printf("Thread list %d\n", i);
		while(aux.next){
			printf("%d ", aux.next->val);
			aux=*(aux.next);	
		}
		printf("\n");
	}
}

int main(int argc, char *argv[]){
	if(argc<1){
		printf("Passa in input il numero di thread da creare\n");
		exit(1);
	}
	
	N=atoi(argv[1]);
	sigset_t set;
	struct sigaction *act;


	if(N<1){
		printf("N deve essere maggiore di 1\n");
		exit(1);
	}

	int i;

	lists=malloc(N*sizeof(data));

	pthread_mutex_init(&lock, NULL);

	pthread_mutex_init(&next, NULL);

	pthread_mutex_lock(&lock);

	for(i=0;i<N;i++){
		lists[i].val=-1;
		lists[i].next=NULL;
	}

	sigfillset(&set);
	act.sa_sigaction=printer;
	act.sa_mask=set;
	sa_flags=0;
	sigaction(SIGINT, act, NULL);

	pthread_t tid;

	for(i=0;i<N;i++){
		pthread_crate(&tid, NULL, funct, (void*)&i);
	}

	while(1){
		pthread_mutex_lock(&next);
		scanf("%d", &val);
		pthread_mutex_unlock(&lock);
	}
	return 0;
}
