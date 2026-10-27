
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define N 5
#define MEALS 3

pthread_mutex_t forks[N];
sem_t waiter;

int mode = 0;

void *philosopher(void *arg)
{
    int id = *(int *)arg;

    int left = id;
    int right = (id + 1) % N;

    for (int i = 0; i < MEALS; i++)
    {
        printf("Philosopher %d is thinking\n", id);
        struct timespec delay = {0, 100000000};
        nanosleep(&delay, NULL);

        if (mode == 1)
        {
            if (left < right)
            {
                pthread_mutex_lock(&forks[left]);
                pthread_mutex_lock(&forks[right]);
            }
            else
            {
                pthread_mutex_lock(&forks[right]);
                pthread_mutex_lock(&forks[left]);
            }
        }
        else
        {
            sem_wait(&waiter);

            pthread_mutex_lock(&forks[left]);
            pthread_mutex_lock(&forks[right]);
        }

        printf("Philosopher %d is eating\n", id);
        nanosleep(&delay, NULL);

        pthread_mutex_unlock(&forks[left]);
        pthread_mutex_unlock(&forks[right]);

        if (mode == 2)
            sem_post(&waiter);
    }

    printf("Philosopher %d finished\n", id);

    return NULL;
}

int main(void)
{
    pthread_t threads[N];
    int ids[N];

    for (int i = 0; i < N; i++)
    {
        pthread_mutex_init(&forks[i], NULL);
        ids[i] = i;
    }

    for (mode = 1; mode <= 2; mode++)
    {
        printf("\nMETHOD %d\n", mode);

        if (mode == 2)
            sem_init(&waiter, 0, N - 1);

        for (int i = 0; i < N; i++)
        {
            pthread_create(
                &threads[i], NULL,
                philosopher, &ids[i]);
        }

        for (int i = 0; i < N; i++)
            pthread_join(threads[i], NULL);

        if (mode == 2)
            sem_destroy(&waiter);
    }

    for (int i = 0; i < N; i++)
        pthread_mutex_destroy(&forks[i]);

    return 0;
}
