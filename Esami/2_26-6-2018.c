#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>

typedef struct _data{
	int val;
	struct _data* next;
}data;

pthread_mutex_t next, lock;
int N;
data *lists;
int val;

void *funct(void* i){
	data *aux;
	int ret;

	while(1){
		aux = malloc(sizeof(data));//pre-malloc for critical path speedup
		if (aux == NULL){
			printf("malloc error\n");
			exit(-1);
		}
		ret = pthread_mutex_lock(&lock);
		if (ret != 0){
			printf("locking error\n");
			exit(-1);
		}
		printf("thread %ld - found value is %d\n",(long)i,val);
		aux->val = val;
		ret = pthread_mutex_unlock(&next);
		if (ret != 0){
			printf("unlocking error\n");
			exit(-1);
		}
		/*aux->next = lists[(long)i].next;
		lists[(long)i].next = aux;*/
	}
	return 0;

}

void printer(int signo, siginfo_t *a, void *b){
	data aux;
	int i;

	for(i=0; i<N; i++){
		aux=lists[i];
		printf("Printing list %d\n", i);
		while(aux.next){
			printf("%d\n", aux.next->val);
			aux=*(aux.next);
		}
		printf("\n");
	}
}


int main(int argc, char *argv[]){
	int ret; 
	if(argc<2){
		printf("Devi passare in input il numero di thread da generare\n");
		exit(1);
	}

	N=atoi(argv[1]);

	if(N<1){
		printf("Il numero passato in input deve essere maggiore uguale a 1\n");
		exit(1);
	}

	int i=0;

	sigset_t set;
	struct sigaction act;


	lists=malloc(N*sizeof(data));

	for(i=0;i<N;i++){
		lists[i].val=-1;
		lists[i].next=NULL;
	}

	pthread_mutex_init(&lock,NULL);
	pthread_mutex_init(&next, NULL);

	pthread_mutex_lock(&lock);

	sigfillset(&set);
	act.sa_sigaction=printer;
	act.sa_mask=set;
	act.sa_flags=0;
	sigaction(SIGINT, &act, NULL);


	pthread_t tid;

	for(i=0;i<N;i++){
		pthread_create(&tid, NULL, funct, (void*)&i);

	}

	while(1){

		
step1:
		ret = pthread_mutex_lock(&next);
		if(ret !=0 && errno == EINTR) goto step1;
step2:
		ret = scanf("%d",&val);
		if(ret == EOF) goto step2;
		if(ret == 0){
			printf("non compliant input\n");
			exit(-1);
		}
step3:		
		ret = pthread_mutex_unlock(&lock);
		if(ret != 0 && errno == EINTR) goto step3;

	}

	return 0;
}