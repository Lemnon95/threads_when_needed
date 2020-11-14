#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

/*Write a Pthreads program that implements a “task queue.” The main thread
begins by starting a user-specified number of threads that immediately go to
sleep in a condition wait. The main thread generates blocks of tasks to be carried out by the other threads; each time it generates a new block of tasks, it awakens a thread with a condition signal. When a thread finishes executing its block of tasks, it should return to a condition wait. When the main thread completes generating tasks, it sets a global variable indicating that there will be no more tasks, and awakens all the threads with a condition broadcast. For the sake of explicitness, make your tasks linked list operations.*/

/* shared variables */
pthread_mutex_t mutex;
pthread_cond_t cond_var;
//int wake_up = 0; // variable for sleeping
bool wake_up_all = false; //global variable indicating when there will be no more tasks
int sum = 0;
int *a;

void* Sleep (void* rank);


int main(int argc, char* argv[]) {
	/* variables declaration */
	int flag = 1;
	long thread_count = strtol(argv[1], NULL, 10);
	pthread_t* thread_handles = malloc(thread_count*sizeof(pthread_t));
	
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond_var, NULL);
	
	for (long thread = 0; thread < thread_count; thread++)
		pthread_create(&thread_handles[thread], NULL, Sleep, (void*) thread);
		
	/* main thread generates tasks */
	do {
		/*srand(time(NULL));
		int num = rand() % 100;
		a = malloc(num*sizeof(int));
		for (int i = 0; i < num; i++) {
			*(a+i) = rand() % 400;
		}*/
		printf("Enter three numbers\n");
		a = malloc(3*sizeof(int));
		for (int i = 0; i < 3; i++) {
			scanf("%d", a+i);
		}
		pthread_mutex_lock(&mutex);
		//wake_up = 1;
		pthread_cond_signal(&cond_var);
		pthread_mutex_unlock(&mutex);
		printf("Do you have other numbers?[1/0]\n");
		scanf("%d", &flag);
		
	} while (flag == 1);
	
	
	/* main thread wakes up other threads */
	pthread_mutex_lock(&mutex);
	//wake_up = 1;
	wake_up_all = true;
	pthread_cond_broadcast(&cond_var);
	pthread_mutex_unlock(&mutex);
	
	/* main thread joins the other threads */
	for (long i = 0; i < thread_count; i++) {
		pthread_join(thread_handles[i], NULL);
	}
	
	printf("The global sum is: %d ", sum);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond_var);
	free(thread_handles);
	return 0;
}

void* Sleep (void* rank) {
	long my_rank = (long) rank;
	
	while (!wake_up_all){
		pthread_mutex_lock(&mutex);
		//if (!wake_up)
			while(pthread_cond_wait(&cond_var, &mutex) != 0); 
		if (!wake_up_all) { // I'm awaken because I've work to do
			for (int i = 0; i < 3; i++)
				sum += a[i];
			//wake_up = 0;
			free(a);
			}
		pthread_mutex_unlock(&mutex);		
	}
	
	return NULL;
}


