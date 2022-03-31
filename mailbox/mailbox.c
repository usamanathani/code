/*

1. Thread 1 and Thread 2 Send Messages to Mailbox (Buffer)
2. Thread 1 Send Message containing "Event1" every 100ms, Thread 2, "Event2", every 110ms
3. Thread 3 receives Messages from Mailbox. If Event 1 is received, it is printed with its timestamp. 


*/


#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

clock_t start_time;
int addr  = 0;




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
	struct msg buffer[32]; // Mailbox message buffer
	int ack1;  // to handle two threads simultaneously
	int ack2;
	int m_count; // number of messages in mailbox

};

struct mail_data
{	
	struct msg msg_d;
	struct mailbox mailbox_d;
};


void *msg_send(void *md)
{
	struct mail_data *args = (struct mail_data *)md;


	if(args->mailbox_d.m_count > 32)
		delay(10);
	else
	{
		args->mailbox_d.m_count = args->mailbox_d.m_count + 1;
		args->mailbox_d.buffer[addr] = args->msg_d;
		addr = addr + 1;
		printf("Sent Message! (Event%d) (Mailbox Count: %d)", args->msg_d.event,args->mailbox_d.m_count);
		get_time(2);
	}
}

void create_event(struct mail_data *maildata, int event_type, int ms_delay) //msg_send must be ptr
{
	int rc;

	struct msg data;

	data.event = event_type;
	data.ack = 0;
	delay(ms_delay);
	maildata->msg_d = data;
	 
	rc = pthread_create(&maildata->msg_d.thread, NULL, msg_send, maildata);

}



void main()
{		
	start_time = clock();
	
	struct mail_data *maildata = (struct mail_data *)malloc(sizeof(struct mail_data));



	
	maildata->mailbox_d.m_count = 0;

	
	
	get_time(1); //Start
	for(int i=0; i<10; i++)
	{	
		
		create_event(maildata, 1, 100);
		pthread_join(maildata->msg_d.thread, NULL);
		create_event(maildata, 2, 10);
		pthread_join(maildata->msg_d.thread, NULL);
		
	}
	get_time(1); // End
	printf("Final Address of addr: %d\n", addr);
	for(int i=0; i<20; i++)
	{
		printf("Buffer[%d] = Event%d\n",i,maildata->mailbox_d.buffer[i].event);
	}
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