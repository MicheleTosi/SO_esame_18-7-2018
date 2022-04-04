#include<stdio.h>
#include<unistd.h>
#include<semaphore.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<signal.h>
#include<pthread.h>

typedef struct _data{
	int val;
	struct _data *next;
}data;

data *lists;
sem_t *sem,*sem1;
int val, N;

void* funct(void *dummy){
	long i=(long)dummy;
	data* aux;

	while(1){
		aux=malloc(sizeof(data));
		sem_wait(sem1);
		aux->val=val;
		sem_post(sem);
		aux->next=lists[i].next;
		lists[i].next=aux;
	}
}

void printer(){
	int i;
	data aux;
	for(i=0;i<N;i++){
		aux=lists[i];
		printf("Printing list %d\n", i);
		while(aux.next){
			printf("%d ", aux.next->val);
			aux=*(aux.next);
		}
		printf("\n");
	}
}

int main(int argc, char *argv[]){
	if(argc<2){
		printf("Inserire il numero di threads da creare\n");
		exit(1);
	}

	long i;
	pthread_t tid;

	N=atoi(argv[1]);

	lists=malloc(N*sizeof(data));

	signal(SIGINT, printer);

	sem=sem_open("/rea", O_CREAT, 0666, 1);
	sem1=sem_open("/wri", O_CREAT, 0666, 0);

	for(i = 0; i< N; i++){
		lists[i].val = -1;
		lists[i].next = NULL;
	}

	for(i=0; i<N; i++){
		printf("%ld\n", i);
		pthread_create(&tid, NULL, funct, (void*)i);
	}

	while(1){
		sem_wait(sem);
		scanf("%d", &val);
		sem_post(sem1);
	}

	return 0;
}