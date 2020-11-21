#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

/*Write a Pthreads program that implements a “task queue.” The main thread
begins by starting a user-specified number of threads that immediately go to
sleep in a condition wait. The main thread generates blocks of tasks to be carried out by the other threads; each time it generates a new block of tasks, it awakens a thread with a condition signal. When a thread finishes executing its block of tasks, it should return to a condition wait. When the main thread completes generating tasks, it sets a global variable indicating that there will be no more tasks, and awakens all the threads with a condition broadcast. For the sake of explicitness, make your tasks linked list operations.*/

/* shared variables */
pthread_mutex_t mutex;
pthread_cond_t cond_var;
pthread_rwlock_t lock;
bool wake_up_all = false; //global variable indicating when there will be no more tasks

struct list_node_s {
	int data;
	struct list_node_s* next;
}*head;

typedef struct queue {
	int type, value;
	struct queue *link;
} Queue;

Queue *front;
Queue *rear;

int size = 0; //queue size

void* Sleep (void* rank);
void Enqueue(int type, int value, struct queue **front, struct queue **rear);
int Dequeue(int *type, int *value, struct queue **front, struct queue **rear);
int Delete(int value, struct list_node_s** head_p);
int Insert(int value, struct list_node_s** head_p);
int Member(int value, struct list_node_s* head_p);


int main(int argc, char* argv[]) {
	/* variables declaration */
	long thread_count = strtol(argv[1], NULL, 10);
	pthread_t* thread_handles = malloc(thread_count*sizeof(pthread_t));
	

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond_var, NULL);
	pthread_rwlock_init(&lock, NULL);

	for (long thread = 0; thread < thread_count; thread++)
		pthread_create(&thread_handles[thread], NULL, Sleep, (void*) thread);
	

	/* main thread generates tasks */
	int flag;
	int type, value;
	int count = 1;
	int how_many;
	do {
		
		printf("You're about to enter your %d round of tasks, how many this time? Remember, you can choose how many tasks you want!\n", count);
		scanf("%d", &how_many);
		printf("Great! Now remember, when asked for the type, you should use the number corresponding to your operation: Member [1], Insert [2], Delete[3]\n");
		for (int i = 0; i < how_many; i++){
			printf("Type: ");
			scanf("%d", &type);
			printf("Value: ");
			scanf("%d", &value);
			Enqueue(type, value, &front, &rear);
		}
		
		while (how_many > thread_count) {
			pthread_mutex_lock(&mutex);
			pthread_cond_broadcast(&cond_var);
			pthread_mutex_unlock(&mutex);
			how_many -= thread_count;
		}
		
		for (int i = 0; i < how_many; i++) {
			pthread_mutex_lock(&mutex);
			pthread_cond_signal(&cond_var);
			pthread_mutex_unlock(&mutex);
		}
		
		printf("Do you want to continue? [1/0]\n");
		scanf("%d", &flag);
		count++;
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

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond_var);
	pthread_rwlock_destroy(&lock);
	free(thread_handles);
	return 0;
}

void* Sleep (void* rank) {
	long my_rank = (long) rank;
	int type, value;

	while (!wake_up_all){
		pthread_mutex_lock(&mutex);
		while(pthread_cond_wait(&cond_var, &mutex) != 0); 
		if (!wake_up_all) {  // I'm awaken because I've work to do
			Dequeue(&type, &value, &front, &rear);
		}
		pthread_mutex_unlock(&mutex);
		if (!wake_up_all) { // I'm awaken because I've work to do	
			switch(type) {
				case 1 :
					pthread_rwlock_rdlock(&lock);
					int is_member = Member(value, head);
					pthread_rwlock_unlock(&lock);
					if (is_member) {
						printf("%d is member\n", value);
					}
						
					break;
				case 2 :
					pthread_rwlock_wrlock(&lock);
					Insert(value, &head);
					pthread_rwlock_unlock(&lock);
					break;
				case 3 :
					pthread_rwlock_wrlock(&lock);
					Delete(value, &head);
					pthread_rwlock_unlock(&lock);
					break;
				default : 
					printf("I shouldn't be here! Type = %d\n", type);
			}
		}
	}

	return NULL;
}

void Enqueue(int type, int value, struct queue **front, struct queue **rear) {
	Queue *task = NULL;
	
	task = (struct queue*)malloc(sizeof(struct queue));
	task->type = type;
	task->value = value;
	task->link = NULL;
	if ((*rear)) {
		(*rear)->link = task;
	}
	
	*rear = task;
	
	if (!(*front)) {
		*front = *rear;
	}
	
	size++;
}

int Dequeue(int *type, int *value, struct queue **front, struct queue **rear){
	Queue *temp = NULL;
	if (size == 0){
		return -1;
	}
	temp = *front;
	*type = temp->type;
	*value = temp->value;
	
	*front = (*front)->link;
	
	size--;
	free(temp);
	return 0;
}


int Member(int value, struct list_node_s* head_p){
	struct list_node_s* curr_p = head_p;

	while (curr_p != NULL && curr_p->data < value)
		curr_p = curr_p->next;

	if (curr_p == NULL || curr_p->data > value) {
		return 0;
	}
	else {
		return 1;
	}
}


int Insert(int value, struct list_node_s** head_p){
	struct list_node_s* curr_p = *head_p;
	struct list_node_s* pred_p = NULL;
	struct list_node_s* temp_p;

	while (curr_p != NULL && curr_p-> data < value) {
		pred_p = curr_p;
		curr_p = curr_p->next;
	}

	if (curr_p == NULL || curr_p->data > value){
		temp_p = malloc(sizeof(struct list_node_s));
		temp_p-> data = value;
		temp_p-> next = curr_p;
		if (pred_p == NULL)
			*head_p = temp_p;
		else
			pred_p->next = temp_p;
		return 1;
	}
	else {
		return 0;
	}
}

int Delete(int value, struct list_node_s** head_p) {
	struct list_node_s* curr_p = *head_p;
	struct list_node_s* pred_p = NULL;

	while (curr_p != NULL && curr_p->data < value) {
		pred_p = curr_p;
		curr_p = curr_p-> next;
	}

	if (curr_p != NULL && curr_p->data == value) {
		if (pred_p == NULL) {
			*head_p = curr_p->next;
			free(curr_p);
		} 
		else {
			pred_p->next = curr_p->next;
			free(curr_p);
		}
		return 1;
	}
	else {
		return 0;
	}
}
