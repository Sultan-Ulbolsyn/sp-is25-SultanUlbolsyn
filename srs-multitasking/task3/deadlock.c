
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define N 5

pthread_mutex_t forks[N];
pthread_barrier_t barrier;

void *philosopher(void *arg)
{
    int id = *(int *)arg;
    int left = id;
    int right = (id + 1) % N;

    pthread_mutex_lock(&forks[left]);

    printf("Philosopher %d took fork %d\n",
           id, left);
    fflush(stdout);

    pthread_barrier_wait(&barrier);

    printf("Philosopher %d waits for fork %d\n",
           id, right);
    fflush(stdout);

    pthread_mutex_lock(&forks[right]);

    pthread_mutex_unlock(&forks[right]);
    pthread_mutex_unlock(&forks[left]);

    return NULL;
}

int main(void)
{
    pthread_t threads[N];
    int ids[N];

    pthread_barrier_init(&barrier, NULL, N);

    for (int i = 0; i < N; i++)
    {
        pthread_mutex_init(&forks[i], NULL);
        ids[i] = i;
    }

    for (int i = 0; i < N; i++)
    {
        pthread_create(&threads[i], NULL,
                       philosopher, &ids[i]);
    }

    for (int i = 0; i < N; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
