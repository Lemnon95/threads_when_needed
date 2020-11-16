#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

/*Write a Pthreads program that implements a “task queue.” The main thread
begins by starting a user-specified number of threads that immediately go to
sleep in a condition wait. The main thread generates blocks of tasks to be carried out by the other threads; each time it generates a new block of tasks, it awakens a thread with a condition signal. When a thread finishes executing its block of tasks, it should return to a condition wait. When the main thread completes generating tasks, it sets a global variable indicating that there will be no more tasks, and awakens all the threads with a condition broadcast.*/

/* shared variables */
pthread_mutex_t mutex;
pthread_mutex_t sums;
pthread_cond_t cond_var;
bool wake_up_all = false; //global variable indicating when there will be no more tasks
int global_sum = 0;
int **a;

void* Sleep (void* rank);


int main(int argc, char* argv[]) {
	/* variables declaration */
	int flag;
	int blocks;
	long thread_count = strtol(argv[1], NULL, 10);
	pthread_t* thread_handles = malloc(thread_count*sizeof(pthread_t));
	a = malloc(thread_count*sizeof(int *));

	/* dinamically allocating each thread portion of a */
	for (int i = 0; i < thread_count; i++){
		a[i] = malloc(4*sizeof(int));
		a[i][0] = 0; //used like a flag, 0 for empty 1 for full.
	}
	
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&sums, NULL);
	pthread_cond_init(&cond_var, NULL);
	
	for (long thread = 0; thread < thread_count; thread++)
		pthread_create(&thread_handles[thread], NULL, Sleep, (void*) thread);
		
	/* main thread generates tasks */
	do {
		do {
			printf("How many blocks of 3 numbers do you want to enter?\n");
			scanf("%d", &blocks);
			if (blocks > thread_count)
				printf("Too many tasks, try again.\n\n");
		} while (blocks > thread_count);
		
		for(int j = 0; j < blocks; j++) {
			a[j][0] = 1;
			printf("Enter three numbers\n");
			for (int i = 1; i < 4; i++) {
				scanf("%d", &a[j][i]);
			}
		}
		pthread_mutex_lock(&mutex);
		pthread_cond_broadcast(&cond_var);
		pthread_mutex_unlock(&mutex);
		printf("Do you have other numbers?[1/0]\n");
		scanf("%d", &flag);
		
	} while (flag == 1);
	
	
	/* main thread wakes up other threads */
	pthread_mutex_lock(&mutex);
	wake_up_all = true;
	pthread_cond_broadcast(&cond_var);
	pthread_mutex_unlock(&mutex);
	
	/* main thread joins the other threads */
	for (long i = 0; i < thread_count; i++) {
		pthread_join(thread_handles[i], NULL);
	}
	
	printf("The global sum is: %d ", global_sum);
	
	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&sums);
	pthread_cond_destroy(&cond_var);
	free(thread_handles);
	
	for (int i = 0; i < thread_count; i++){
		free(a[i]);
	}
	free(a);
	return 0;
}

void* Sleep (void* rank) {
	long my_rank = (long) rank;
	int my_sum = 0;
	
	while (!wake_up_all){
		pthread_mutex_lock(&mutex);
		while(pthread_cond_wait(&cond_var, &mutex) != 0); 
		pthread_mutex_unlock(&mutex);	
		if (!wake_up_all) { // I'm awaken because I've work to do
			if (a[my_rank][0] == 1) { // my array is full
				for (int i = 1; i < 4; i++)
					my_sum += a[my_rank][i];
				
				pthread_mutex_lock(&sums);
				global_sum += my_sum;
				pthread_mutex_unlock(&sums);
				my_sum = 0;
			}
		}			
	}	
	return NULL;
}




