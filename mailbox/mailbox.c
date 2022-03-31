/*

1. Thread 1 and Thread 2 Send Messages to Mailbox (Buffer)
2. Thread 1 Send Message containing "Event1" every 100ms, Thread 2, "Event2", every 110ms
3. Thread 3 receives Messages from Mailbox. If Event 1 is received, it is printed with its timestamp. 
4, Thread 3 acknowledges Thread1 / Thread2 Msg Processed so next event can be sent. 


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
pthread_mutex_t lock; // add lock to the mailbox. 



void delay(int milli)
{
	long pause;
	clock_t now, then;
	pause = milli * (CLOCKS_PER_SEC/1000);
	now = then = clock ();
	while((now-then)<pause)
		now=clock();
}
//usleep better choice

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
	int val;
};

struct mailbox 
{
	struct msg buffer[32]; // Mailbox message buffer
	int ack1;  // to handle two threads simultaneously
	int ack2;
	int count; // number of messages in mailbox
	int total;

};

struct mail_data
{	
	struct msg msg_d;
	struct mailbox mailbox_d;
};


void *msg_send(void *md)
{


	pthread_mutex_lock(&lock);
	struct mail_data *args = (struct mail_data *)md;

	if(args->mailbox_d.count > 32) //
		delay(10);
	else
	{
		args->mailbox_d.count = args->mailbox_d.count + 1;
		args->mailbox_d.total = args->mailbox_d.total + 1;
		args->mailbox_d.ack1 = 0;
		args->mailbox_d.ack2 = 0;
		args->mailbox_d.buffer[addr] = args->msg_d;
		addr = addr + 1;
		printf("Sent Msg!     Event:%d Value:%d  (Mailbx Size: %d | Msgs Sent: %d)", args->msg_d.event,args->msg_d.val,args->mailbox_d.count, args->mailbox_d.total);
		get_time(2);
	}
	
	pthread_mutex_unlock(&lock);
}


void *msg_rec(void *md)
{	

	pthread_mutex_lock(&lock);
	struct mail_data *args = (struct mail_data *)md;
	
	
	while(args->mailbox_d.count != 0) // check mailbox is not empty
	{		
			args->mailbox_d.count = args->mailbox_d.count - 1;
			args->msg_d = args->mailbox_d.buffer[addr-1];
			addr = addr - 1;

			printf("Recieved Msg! Event:%d Value:%d  (Mailbx Size: %d | Msgs Sent: %d)", args->msg_d.event,args->msg_d.val,args->mailbox_d.count, args->mailbox_d.total);
			get_time(2);


			if(args->msg_d.event == 1)
				args->mailbox_d.ack1 == 1;
			else
				args->mailbox_d.ack2 == 1;
	}
	pthread_mutex_unlock(&lock);
}

void rec_event(struct mail_data *maildata)
{
	int rc;
	struct msg data;
	pthread_t thread3;


	if(pthread_mutex_init(&lock, NULL)!=0) // Initializing lock
		printf(" \n Mutex Init Failed! \n");

	else
		rc = pthread_create(&thread3, NULL, msg_rec, maildata); 
		
		if(pthread_join(thread3, NULL) != 0)
			printf("Pthread join fail \n");

}



void create_event(struct mail_data *maildata, int event_type, int ms_delay) //msg_send must be ptr
{
	int rc;
	struct msg data;
	int ack;

	if(maildata->mailbox_d.total != 0)
	{
		if(event_type == 1)
			ack = maildata->mailbox_d.ack1;
		else
			ack = maildata->mailbox_d.ack2;		
	}
	else
		ack = 1; // Initializing. 
		maildata->mailbox_d.ack1 = 1;
		maildata->mailbox_d.ack2 = 1;


	if(ack)
	{
		
		data.event = event_type;
		data.val = rand() % 100 + 1;
		
		delay(ms_delay);
		maildata->msg_d = data;

		if(pthread_mutex_init(&lock, NULL)!=0)
			printf(" \n Mutex Init Failed! \n");
		else
			rc = pthread_create(&maildata->msg_d.thread, NULL, msg_send, maildata);

	}


}


void main()
{		
	start_time = clock();
	
	struct mail_data *maildata = (struct mail_data *)malloc(sizeof(struct mail_data));
	
	maildata->mailbox_d.count = 0;
	maildata->mailbox_d.total = 0;
	srand(time(NULL));

	get_time(1); //Start
	for(int i=0; i<40; i++)
	{	

		create_event(maildata, 1, 100);
		pthread_join(maildata->msg_d.thread, NULL);
		rec_event(maildata);
		create_event(maildata, 2, 10);
		rec_event(maildata);
		pthread_join(maildata->msg_d.thread, NULL);
		
	}
	get_time(1); // End
	
	//printf("Final Address of addr: %d\n", addr);
	
	// printf("Buffer check! \n");
	// for(int i=0; i<20; i++)
	// {
	// 	printf("Buffer[%d] = Event%d\n",i,maildata->mailbox_d.buffer[i].event);
	// }
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

