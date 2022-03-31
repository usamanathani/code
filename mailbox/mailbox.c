#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <semaphore.h>




// Remove message from mailbox
// Change function send message, and have recieve two values, mailbox + message being sent. 

#define MAILBOX_SIZE 32 // Defining the initial mailbox_size

sem_t sem_thread1, sem_thread2; // Used as an ack. Thread 1 send messages and sem_wait. Thread 3 receieves messages and sem_post. 
pthread_mutex_t lock; // Mutex is used to lock send function so threads dont clash

struct msg // Msg struct. Contains event (1 or 2 depending on thread), random val, id = total + 1 to track results between sent and rec
{
	int event; 
	int val;
	int id;
};

 
struct mailbox // Mailbox has buffer, total for msgs sent, count for current msgs in mailbox
{
	struct msg buffer[MAILBOX_SIZE]; // Mailbox message buffer
	int total; // total messages sent
	int count;
	int head; // head and tail used to represent buffer as FIFO - First In First Out
	int tail;
};

void get_time() // Used to get the current timestamp
{
	time_t c_time;
	char* ctimestr;
	c_time = time(NULL);
	ctimestr = ctime(&c_time);
	printf("Time: %s \n", ctimestr);

}

void msg_send(struct mailbox *mbx_s, struct msg *data_send) // Msg Send takes Mailbox and Msg that is sent.
{	

	struct mailbox *mailbox_s = (struct mailbox *)mbx_s; 
	struct msg *data_sendx = (struct msg *) data_send;


	pthread_mutex_lock(&lock); // Locking the thread so that two threads dont clash
	int waslocked=0;

	mailbox_s->buffer[mailbox_s->tail] = *data_sendx; // Message is put inside buffer
	mailbox_s->total = mailbox_s->total + 1; 
	printf("Message Sent! -> Event:%d Val:%d ID:%d Total:%d\n", data_sendx->event,data_sendx->val,data_sendx->id, mailbox_s->total);
	mailbox_s->count = mailbox_s->count + 1;
	mailbox_s->tail = mailbox_s->tail + 1;

	if(mailbox_s->tail == MAILBOX_SIZE) // To make sure buffer rolls back
		mailbox_s->tail = 0;
	
	pthread_mutex_unlock(&lock); // Unlocking thread so another message can be sent
}


void *thread1(void *th)
{
	struct mailbox *mailbox_t = (struct mailbox *)th;
	
	while(mailbox_t->count < MAILBOX_SIZE) // while mailbox is not full
	{	
		usleep(100000); // sleep for 100ms
		struct msg data_send;

		data_send.event = 1; // Creates Event 1
		data_send.val = rand() % 100 + 1; // Random value from 1 --> 100
		data_send.id = mailbox_t->total + 1; // Id assigned to track values between send and rec
		msg_send(mailbox_t, &data_send); // Send message and mailbox
		sem_wait(&sem_thread1); // Used as ack. Sem_waits from thread 1 and is released in thread 3 when Message is rec
		
	}	
}


void *thread2(void *th) // Thread 2 behaves similarly like Thread 1
{
	struct mailbox *mailbox_t = (struct mailbox *)th;

	while(mailbox_t->count < MAILBOX_SIZE)
	{	
		usleep(110000); // sleep for 110ms
		struct msg data_send;

		data_send.event = 2;
		data_send.val = rand() % 100 + 1;
		data_send.id = mailbox_t->total + 1;
		msg_send(mailbox_t, &data_send);
		sem_wait(&sem_thread2);
	}	
}

void rec_event(struct mailbox *mbx) // Recieves Event from Mailbox
{	
	struct msg data_rec = mbx->buffer[mbx->head]; // Takes message from buffer and places in variable to use
	printf("Recieved Event! Event:%d Val:%d ID:%d Total:%d \n", data_rec.event, data_rec.val, data_rec.id, mbx->total);
	get_time(); // Gets Time
	mbx->head = mbx->head + 1; // Increases Head whenever message recieved
	mbx->count = mbx->count - 1; // Once head reaches mailbox_size, goes back to 0
	if(mbx->head == MAILBOX_SIZE)
		mbx->head = 0;
}


void *thread3(void *th) // Thread 3 recieves messages
{
	struct mailbox *mailbox_r = (struct mailbox *)th;
	struct msg data_rec;

	int rcount = 0; // Rcount used to determine when there is msg in mailbox
	// Initially 0, when total count of mailbox goes up, e.g rcount = 0, total = 0. Msg Sent, rcount = 0 < total = 1. Goes inside loop
	// Once it completes rcount = 1, total = 1. Waits for total to go up again before taking next message. 
	while(1)
	{	
		while(rcount < mailbox_r->total)
		{	
			pthread_mutex_lock(&lock);	
			data_rec = mailbox_r->buffer[mailbox_r->head]; 
			rcount = rcount + 1; // rcount is increased so loop doesnt rec empty events
			rec_event(mailbox_r); // Calls Rec Event

			if(data_rec.event == 1)  // If Event Rec is Event Type 1, thread 1 gets released. Thread 2 behaves similarly.
				sem_post(&sem_thread1);
			else if(data_rec.event ==2)
				sem_post(&sem_thread2);

			pthread_mutex_unlock(&lock);									
		}
	}	
	
}

void main()
{
	pthread_t s_thread1, s_thread2;; // Threads for Sending Messages, thread 1 and thread 2
	pthread_t r_thread3; // Thread for Rec Messages
	int rc;
	int ret;
	srand(time(NULL)); // Randomizing Seed
	pthread_mutex_init(&lock, NULL); // Init Lock
	ret = sem_init(&sem_thread1, 0, 1); // Init Semaphores
	ret = sem_init(&sem_thread2, 0, 1);

	struct mailbox *mailbox_m = (struct mailbox *)malloc(sizeof(struct mailbox)); // Allocating space for mailbox
	// Init values for mailbox
	mailbox_m->tail = 0;
	mailbox_m->head = 0;
	mailbox_m->total = 0;
	mailbox_m->count = 0;
	// Starting Threads
	rc = pthread_create(&s_thread1, NULL, thread1, mailbox_m);
	rc = pthread_create(&s_thread2, NULL, thread2, mailbox_m);
	rc = pthread_create(&r_thread3,NULL,thread3,mailbox_m);
	rc = pthread_join(s_thread1,NULL);
	rc = pthread_join(s_thread2,NULL);	
	rc = pthread_join(r_thread3, NULL);

	free(mailbox_m); // De-allocating space

}