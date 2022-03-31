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

#define MAILBOX_SIZE 32

sem_t sem_thread1, sem_thread2;
pthread_mutex_t lock;

struct msg 
{
	int event; 
	int val;
	int id;
};

 
struct mailbox 
{
	struct msg buffer[MAILBOX_SIZE]; // Mailbox message buffer
	int total; // total messages sent
	int count;
	int head;
	int tail;
};

void get_time()
{
	time_t c_time;
	char* ctimestr;
	c_time = time(NULL);
	ctimestr = ctime(&c_time);
	printf("Time: %s \n", ctimestr);

}

void msg_send(struct mailbox *mbx_s, struct msg *data_send)
{	

	struct mailbox *mailbox_s = (struct mailbox *)mbx_s;
	struct msg *data_sendx = (struct msg *) data_send;
	
	//while(pthread_mutex_trylock(&lock) != 0)
	{
		pthread_mutex_lock(&lock);
		mailbox_s->buffer[mailbox_s->tail] = *data_sendx;
		mailbox_s->total = mailbox_s->total + 1;
		printf("Message Sent! -> Event:%d Val:%d ID:%d Total:%d\n", data_sendx->event,data_sendx->val,data_sendx->id, mailbox_s->total);
		mailbox_s->count = mailbox_s->count + 1;
		mailbox_s->tail = mailbox_s->tail + 1;
		if(mailbox_s->tail == MAILBOX_SIZE)
			mailbox_s->tail = 0;
		
		pthread_mutex_unlock(&lock);
	}
	
}


void *thread1(void *th)
{
	struct mailbox *mailbox_t = (struct mailbox *)th;
	
	while(mailbox_t->count < MAILBOX_SIZE)
	{	
		usleep(100000);
		struct msg data_send;

		data_send.event = 1;
		data_send.val = rand() % 100 + 1;
		data_send.id = mailbox_t->total + 1;
		msg_send(mailbox_t, &data_send);
		sem_wait(&sem_thread1);
		
	}	
}


void *thread2(void *th)
{
	struct mailbox *mailbox_t = (struct mailbox *)th;

	while(mailbox_t->count < MAILBOX_SIZE)
	{	
		usleep(110000);
		struct msg data_send;

		data_send.event = 2;
		data_send.val = rand() % 100 + 1;
		data_send.id = mailbox_t->total + 1;
		msg_send(mailbox_t, &data_send);
		sem_wait(&sem_thread2);
	}	
}

void rec_event(struct msg data_r, struct mailbox *mbx)
{	
	printf("Recieved Event! Event:%d Val:%d ID:%d Total:%d \n", data_r.event, data_r.val, data_r.id, mbx->total);
	get_time();
}


void *thread3(void *th)
{
	struct mailbox *mailbox_r = (struct mailbox *)th;
	struct msg data_rec;

	int rcount = 0;
	
	while(mailbox_r->count < MAILBOX_SIZE)
	{	
		while(rcount < mailbox_r->total)
		{	
			pthread_mutex_lock(&lock);	
			data_rec = mailbox_r->buffer[mailbox_r->head];
			rcount = rcount + 1;
			rec_event(data_rec, mailbox_r);

			if(data_rec.event == 1)
				sem_post(&sem_thread1);
			else if(data_rec.event ==2)
				sem_post(&sem_thread2);

			mailbox_r->head = mailbox_r->head + 1;
			mailbox_r->count = mailbox_r->count  - 1;
			if(mailbox_r->head == 32)
				mailbox_r->head = 0;
			pthread_mutex_unlock(&lock);									
		}
	}	
	
}

void main()
{
	pthread_t s_thread1, s_thread2;; // send thread 1, thread 2
	pthread_t r_thread3;
	int rc;
	int ret;
	srand(time(NULL));
	pthread_mutex_init(&lock, NULL);
	ret = sem_init(&sem_thread1, 0, 1);
	ret = sem_init(&sem_thread2, 0, 1);

	struct mailbox *mailbox_m = (struct mailbox *)malloc(sizeof(struct mailbox));
	mailbox_m->tail = 0;
	mailbox_m->head = 0;
	mailbox_m->total = 0;
	mailbox_m->count = 0;
	
	rc = pthread_create(&s_thread1, NULL, thread1, mailbox_m);
	rc = pthread_create(&s_thread2, NULL, thread2, mailbox_m);
	rc = pthread_create(&r_thread3,NULL,thread3,mailbox_m);
	rc = pthread_join(s_thread1,NULL);
	rc = pthread_join(s_thread2,NULL);	
	rc = pthread_join(r_thread3, NULL);

	free(mailbox_m);

}