#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

/*Write a Pthreads program that implements a “task queue.” The main thread
begins by starting a user-specified number of threads that immediately go to
sleep in a condition wait. The main thread generates blocks of tasks to be carried out by the other threads; each time it generates a new block of tasks, it awakens a thread with a condition signal. When a thread finishes executing its block of tasks, it should return to a condition wait. When the main thread completes generating tasks, it sets a global variable indicating that there will be no more tasks, and awakens all the threads with a condition broadcast. For the sake of explicitness, make your tasks linked list operations.*/

/* shared variables */
pthread_mutex_t mutex;
pthread_mutex_t mutex2;
pthread_cond_t cond_var;
pthread_rwlock_t lock;
bool wake_up_all = false; //global variable indicating when there will be no more tasks
long is_done;

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

typedef struct queue_str {
	char str[100];
	struct queue_str *link;
} Queue_Str;

Queue_Str *front_str;
Queue_Str *rear_str;

int size_QS = 0;

void* Sleep (void* rank);
void Enqueue(int type, int value, struct queue **front, struct queue **rear);
int Dequeue(int *type, int *value, struct queue **front, struct queue **rear);
int Delete(int value, struct list_node_s** head_p);
int Insert(int value, struct list_node_s** head_p);
int Member(int value, struct list_node_s* head_p);
void Enqueue_Str(char *src, struct queue_str **front, struct queue_str **rear);
int Dequeue_Str(char *str, struct queue_str **front, struct queue_str **rear);


int main(int argc, char* argv[]) {
	/* variables declaration */
	long thread_count = strtol(argv[1], NULL, 10);
	pthread_t* thread_handles = malloc(thread_count*sizeof(pthread_t));
	

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mutex2, NULL);
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
		
		is_done = 0;
		int when_done = how_many;
		int i = 0;
		while (how_many >= thread_count) {
			pthread_mutex_lock(&mutex);
			pthread_cond_broadcast(&cond_var);
			pthread_mutex_unlock(&mutex);
			how_many -= thread_count;
			i++;
			while (is_done < thread_count*i);
			
		}
		
		for (int i = 0; i < how_many; i++) {
			pthread_mutex_lock(&mutex);
			pthread_cond_signal(&cond_var);		
			pthread_mutex_unlock(&mutex);
		}
		
		while(is_done < when_done); //barrier, wait for completion of children
		printf("Tasks acquired, now processing...\n");
		char string[100];
		while (Dequeue_Str(string, &front_str, &rear_str) == 0) { //no need for critical section, every thread is sleeping
			printf("%s", string);
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
	pthread_mutex_destroy(&mutex2);
	pthread_cond_destroy(&cond_var);
	pthread_rwlock_destroy(&lock);
	free(thread_handles);
	return 0;
}

void* Sleep (void* rank) {
	long my_rank = (long) rank;
	int type, value;
	int i = 0;

	while (!wake_up_all){
		pthread_mutex_lock(&mutex);
		if (i > 0) //I've actually completed a task
			is_done++;
		while(pthread_cond_wait(&cond_var, &mutex) != 0); 
		if (!wake_up_all) {  // I'm awaken because I've work to do
			Dequeue(&type, &value, &front, &rear);
		}
		pthread_mutex_unlock(&mutex);
		if (!wake_up_all) { // I'm awaken because I've work to do	
			char src[100];
			switch(type) {
				case 1 :
					pthread_rwlock_rdlock(&lock);
					int is_member = Member(value, head);
					pthread_rwlock_unlock(&lock);
					pthread_mutex_lock(&mutex2); //critical section for Queue_Str
					if (is_member) {
						sprintf(src, "%d is a list member\n", value);
					}
					else {
						sprintf(src, "%d is NOT a list member\n", value);
					}
					Enqueue_Str(src, &front_str, &rear_str);
					pthread_mutex_unlock(&mutex2);
					
					break;
				case 2 :
					pthread_rwlock_wrlock(&lock);
					Insert(value, &head);
					pthread_rwlock_unlock(&lock);
					pthread_mutex_lock(&mutex2); //critical section for Queue_Str
					sprintf(src, "%d has been added to the list\n", value);
					Enqueue_Str(src, &front_str, &rear_str);
					pthread_mutex_unlock(&mutex2);
					break;
				case 3 :
					pthread_rwlock_wrlock(&lock);
					int success = Delete(value, &head);
					pthread_rwlock_unlock(&lock);
					pthread_mutex_lock(&mutex2); //critical section for Queue_Str
					if (success) {
						sprintf(src, "%d has been deleted from the list\n", value);
					}
					else {
						sprintf(src, "%d has NOT been deleted from the list, because it wasn't in it!\n", value);
					}
					Enqueue_Str(src, &front_str, &rear_str);
					pthread_mutex_unlock(&mutex2);
					break;
				default : 
					printf("I shouldn't be here! Type = %d\n", type);
			}
			i++;
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

void Enqueue_Str(char *src, struct queue_str **front, struct queue_str **rear) {
	Queue_Str *task = NULL;
	
	task = (struct queue_str*)malloc(sizeof(struct queue_str));
	memset(task->str, '\0', sizeof(task->str));
	strcpy(task->str, src);
	task->link = NULL;
	if ((*rear)) {
		(*rear)->link = task;
	}
	
	*rear = task;
	
	if (!(*front)) {
		*front = *rear;
	}
	
	size_QS++;
}


int Dequeue_Str(char *str, struct queue_str **front, struct queue_str **rear){
	Queue_Str *temp = NULL;
	if (size_QS == 0){
		return -1;
	}
	temp = *front;
	strcpy(str, (temp->str));

	
	*front = (*front)->link;
	
	size_QS--;
	free(temp);
	return 0;
}

