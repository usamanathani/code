#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <semaphore.h>


int mailbox_size = 1024;
sem_t sem_thread1, sem_thread2;

struct msg 
{
	int event; 
	int val;
	int id;
	sem_t sem_thread;
};

 
struct mailbox 
{
	struct msg buffer[1024]; // Mailbox message buffer
	int total; // total messages sent
	int count;
	struct msg data;
	int head;
	int tail;
	pthread_mutex_t lock;

};


void msg_send(struct mailbox *mbx_s)
{	

	pthread_mutex_lock(&lock);
	struct mailbox *mailbox_s = (struct mailbox *)mbx_s;
	mailbox_s->buffer[mailbox_s->tail] = mailbox_s->data;
	mailbox_s->total = mailbox_s->total + 1;
	mailbox_s->count = mailbox_s->count + 1;
	printf("Message Sent! Event:%d Val:%d ID:%d Total:%d\n", mailbox_s->data.event,mailbox_s->data.val,mailbox_s->data.id, mailbox_s->total);
	mailbox_s->tail = mailbox_s->tail + 1;
	if(mailbox_s->tail == 32)
		mailbox_s->tail = 0;
	pthread_mutex_unlock(&lock);



}	

void *thread1(void *th)
{
	struct mailbox *mailbox_t = (struct mailbox *)th;
	while(mailbox_t->count < mailbox_size)
	{	
		if(mailbox_t->ack1 == 1)
		{
			usleep(100000);
			struct msg data_send;

			data_send.event = 1;
			data_send.ack = 0;
			data_send.val = rand() % 100 + 1;
			data_send.id = mailbox_t->total + 1;
			mailbox_t->data = data_send;
			msg_send(mailbox_t);
			mailbox_t->ack1 = 0;
		}		
	
	}	

}

void *thread2(void *th)
{

	struct mailbox *mailbox_t = (struct mailbox *)th;
	while(mailbox_t->count < mailbox_size)
	{
		if(mailbox_t->ack2 == 1)
		{	
			usleep(110000);
			struct msg data_send;

			data_send.event = 2;
			data_send.ack = 0;
			data_send.val = rand() % 100 + 1;
			data_send.id = mailbox_t->total + 1;
			mailbox_t->data = data_send;
			msg_send(mailbox_t);
			mailbox_t->ack2 = 0;
		}	
	}
}	




void rec_event(struct msg data_r)
{
	printf("Recieved Event! Event:%d Val:%d ID:%d", data_r.event, data_r.val, data_r.id);
}

void *thread3(void *th)
{
	struct mailbox *mailbox_r = (struct mailbox *)th;
	int rcount = 0;
	int ack;

	while(mailbox_r->count < mailbox_size)
	{	
		while(rcount < mailbox_r->total)
		{
			struct msg data_rec;
			data_rec = mailbox_r->buffer[mailbox_r->head];
				
			mailbox_r->head = mailbox_r->head + 1;
			mailbox_r->count = mailbox_r->count - 1;

			if(mailbox_r->head == 32)
				mailbox_r->head = 0;

			if(data_rec.event == 1)
				mailbox_r->ack1 = 1;
			else
				mailbox_r->ack2 = 1;

			rec_event(data_rec);
			printf(" Total:%d \n", mailbox_r->total);
			rcount = rcount + 1;
		}
	}
	
}


void main()
{
	pthread_t s_thread1, s_thread2;; // send thread 1, thread 2
	pthread_t r_thread3;
	int rc;
	srand(time(NULL));
	pthread_mutex_init(&lock, NULL);


	struct mailbox *mailbox_m = (struct mailbox *)malloc(sizeof(struct mailbox));
	mailbox_m->tail = 0;
	mailbox_m->head = 0;
	mailbox_m->total = 0;
	mailbox_m->count = 0;
	mailbox_m->ack1 = 1;
	mailbox_m->ack2 = 1;
	
	
	rc = pthread_create(&s_thread1, NULL, thread1, mailbox_m);
	rc = pthread_create(&s_thread2, NULL, thread2, mailbox_m);
	rc = pthread_create(&r_thread3,NULL,thread3,mailbox_m);
	rc = pthread_join(s_thread1,NULL);
	rc = pthread_join(s_thread2,NULL);	
	rc = pthread_join(r_thread3, NULL);

	free(mailbox_m);

	// if(mailbox_m->head == mailbox_size)
	// 	mailbox_m->head = 0;
	// if(mailbox_m->tail == mailbox_size)
	// 	mailbox_m->tail = 0;


}