#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>


void delay(int milli)
{
	long pause;
	clock_t now, then;
	pause = milli * (CLOCKS_PER_SEC/1000);
	now = then = clock ();
	while((now-then)<pause)
		now=clock();
}


void get_time(clock_t start) 
{
	clock_t now;
	double diff;
	start = clock();

	now = clock();
	diff = (now-start/CLOCKS_PER_SEC)/1000;
	printf("Time: %.2f ms\n", diff);
	
}

struct msg {
	pthread_t thread;
	char* str;
};

struct msg1 {
	pthread_t thread;
	int value; 
};

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




void main()
{		
	clock_t start;
	start = clock();
	int rc;
	struct msg event;
	struct msg1 event2;

	event.str = "Event1";

	event2.value = 10;



	for(int i =0; i<5; i++) 
	{
		get_time(start);
		delay(200);
		rc = pthread_create(&event.thread, NULL, display, (void *)event.str);
		
		rc = pthread_create(&event2.thread, NULL, display1, (void *)&event2.value); 

	}

	pthread_exit(NULL);




}
 
