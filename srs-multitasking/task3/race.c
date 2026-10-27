
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define THREADS 4
#define ITERATIONS 1000000

long long counter = 0;

void *worker(void *arg)
{
    (void)arg;

    for (int i = 0; i < ITERATIONS; i++)
    {
        counter++;
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[THREADS];

    for (int run = 1; run <= 5; run++)
    {
        counter = 0;

        struct timespec start, end;

        clock_gettime(CLOCK_MONOTONIC, &start);

        for (int i = 0; i < THREADS; i++)
        {
            if (pthread_create(&threads[i], NULL,
                               worker, NULL) != 0)
            {
                fprintf(stderr, "Thread creation failed\n");
                return 1;
            }
        }

        for (int i = 0; i < THREADS; i++)
        {
            if (pthread_join(threads[i], NULL) != 0)
            {
                fprintf(stderr, "Thread join failed\n");
                return 1;
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        double elapsed =
            (end.tv_sec - start.tv_sec) +
            (end.tv_nsec - start.tv_nsec) / 1e9;

        printf("Run %d: Counter = %lld, Time = %.6f sec\n",
               run, counter, elapsed);
    }

    printf("Expected counter = %d\n",
           THREADS * ITERATIONS);

    return 0;
}
