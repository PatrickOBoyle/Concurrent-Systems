#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <sys/time.h>

#define NUM_PHILOSOPHERS 5
#define MAX_RUNS 75

pthread_mutex_t forks[NUM_PHILOSOPHERS];

int runs = 0;
double total_wait_time_seconds = 0;
double peak_wait = 0;

void think_about_life()
{
    usleep(random() % 500);
}

double get_average_wait(int runs, int wait_time_seconds)
{
    return ((double)wait_time_seconds / (double)runs);
}

void *philosopher(void *thread_id)
{
    // setup left & right fork indexes
    int id = (int)thread_id;
    
    int left, right;
    left = id;
    
    if(id + 1 <= NUM_PHILOSOPHERS - 1)
    {
        right = id + 1;
    }
    else
    {
        right = 0;
    }
    
    if(id % 2 == 0)
    {
        think_about_life();
    }
    
    int locked;

    while(runs < MAX_RUNS)
    {
        locked = 0;
        
        struct timeval before, after, result;
        gettimeofday(&before, NULL);
        printf("Before: %lf\n", &before);
        
        while(!locked)
        { 
            // acquire both locks or fuck off
            if(pthread_mutex_trylock(&forks[left]) == 0 )
            {
                if(pthread_mutex_trylock(&forks[right]) == 0)
                {
                    printf("ID: %d - Locking\n", id);

                    printf("After: %lf\n", gettimeofday(&after, NULL));

                    timersub(&after, &before, &result);

                    double time_useconds = (long int) result.tv_usec;
                    double time_seconds = time_useconds / 1000;
                    
                    total_wait_time_seconds += time_seconds;
                    
                    if(time_seconds > peak_wait)
                        peak_wait = time_seconds;

                    locked = 1;       

                    // eating...please wait....
                    usleep(random() % 500);

                    printf("ID: %d - Unlocking\n", id);
                    pthread_mutex_unlock (&forks[left]);
                    pthread_mutex_unlock (&forks[right]);

                    runs++;
                }else
                {
                    pthread_mutex_unlock(&forks[left]);
                    think_about_life();
                }
            }else
            {
                think_about_life();
            }
        }

        think_about_life();
    }
}

int main()
{ 
	pthread_t philosophers[NUM_PHILOSOPHERS]; 
    
    for(int i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        pthread_mutex_init(&forks[i], NULL);   
    }
    
    int rc, p;
    
	for (p = 0; p < NUM_PHILOSOPHERS; p++)
    { 
		printf("Creating philosopher %d\n", p);
        
		rc = pthread_create(&philosophers[p], NULL, philosopher, (void *) p); 
		
        if (rc)
        { 
			printf("ERROR return code from pthread_create(): %d\n", rc); 
			exit(-1); 
		} 
	} 
    
	// wait for threads to exit 
	for(p = 0; p < NUM_PHILOSOPHERS; p++) 
    { 
		pthread_join( philosophers[p], NULL); 
	} 
    
    printf("Average time waited: %lf\n", get_average_wait(runs, total_wait_time_seconds) );
    printf("Total time waited: %lf\n", total_wait_time_seconds);
    printf("Peak time waited: %lf\n", peak_wait);
    
	return 0;
}