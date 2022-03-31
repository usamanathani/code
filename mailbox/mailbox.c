/*

1. Thread 1 and Thread 2 Send Messages to Mailbox (Buffer)
2. Thread 1 Send Message containing "Event1" every 100ms, Thread 2, "Event2", every 110ms
3. Thread 3 receives Messages from Mailbox. If Event 1 is received, it is printed with its timestamp. 
4. If Thread 1 executes, it will not send another message until ack, which is when the thread Event is printed with timestamp. 

*/


#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

clock_t start_time; // Start time of Program to baseline get_time()
int addr = 0;  // Global buffer counter


void delay(int milli)
{
	long pause;
	clock_t now, then;
	pause = milli * (CLOCKS_PER_SEC/1000);
	now = then = clock ();
	while((now-then)<pause)
		now=clock();
}


void get_time(int choice) 
{
	clock_t now;
	double diff;

	now = clock();
	diff = (now-start_time/CLOCKS_PER_SEC)/1000;
	if(choice==1)
	printf("[Time: %.2f ms]\n", diff);
	else
	printf("\t[Time: %.2f ms]\n", diff);
}	


struct msg 
{
	int event;
	pthread_t thread;
	int ack;
};

struct mailbox 
{
	struct msg buffer[32]; // 0 --> 31
	int ack1;  // to handle two threads simultaneously
	int ack2;
	int busy;
	int m_count; // mailbox count 
};

struct mail_data
{	
	struct msg *msg_d;
	struct mailbox *mailbox_d;
};


void *msg_send(void *md)
{
	struct mail_data *args = (struct mail_data *)md;

	struct msg msg_s = args->msg_d;
	struct mailbox mb_s = args-> mailbox_d;
	mb_s.buffer[addr] = args->msg_d;
	addr = addr + 1;

	printf("Sent Message! (Event%d)", msg_s.event);
	get_time(2);
	printf("Mailbox count: %d \n", mb_s.m_count);

}

void event1(struct mail_data maildata)
{
	printf("Event 1 has been processed");
	get_time(2);
	maildata.mailbox_d.ack1 = 1;
}

void event2(struct mail_data maildata)
{
	printf("Event 2 has been processed");
	get_time(2);
	maildata.mailbox_d.ack2 = 1;
}

void *msg_rec(void *md)
{
	struct mail_data *args = (struct mail_data *)md;
	struct msg msg_s = args->msg_d;
	struct mailbox mb_s = args-> mailbox_d;

	int output_event;

	output_event = mb_s.buffer[addr].event;
	addr = addr - 1;

	if(mb_s.busy != 1)
	{
		if(output_event == 1)
		{
			event1(*args);
			mb_s.busy = 1;
			printf("Mailbox count: %d \n", args->mailbox_d.m_count);
		}
		else if(output_event == 2)
		{
			event2(*args);
			mb_s.busy = 1;
			printf("Mailbox count: %d \n", mb_s.m_count);
		}
	}
}

void create_event(struct mail_data *maildata, int event_type) 
{
	int rc;
	maildata.msg_d.event = event_type;
	maildata.msg_d.ack = 0;
	maildata.mailbox_d.buffer[addr] = maildata.msg_d; // Need global variable buffer counter
	

	rc = pthread_create(maildata.msg_d.thread, NULL, msg_send, (void *) &maildata);

}



void rec_event(struct mail_data maildata)
{	
	if(maildata.mailbox_d.m_count != 0 )
	{
		int rc;

		rc = pthread_create(&maildata.msg_d.thread, NULL, msg_rec, (void *) &maildata);
	}
}



void main()
{		
	
	struct mail_data *mdx = malloc(sizeof struct maildata);
	mdx->mailbox_d.m_count = 0; // Initializing number of messages in mailbox.
	
	
	get_time(1); //Start
	for(int i=0; i<10; i++)
	{	
		delay(100);
		create_event(mdx, 1);
		pthread_join(mdx->msg_d.thread, NULL);
		delay(10);
		create_event(mdx, 2);
		pthread_join(mdx->msg_d.thread, NULL);
			
		rec_event(*mdx);
	}

	get_time(1); // End
	
	
	
	free(mdx);
}





/*

void *display1 (void *args)
{
	int *val = (int *)args;
	printf("Value is %d \n", *val);

}


void *display (void *args)
{
	char *val = (char *)args;
	printf("Value is %s \n", val);

}

*/