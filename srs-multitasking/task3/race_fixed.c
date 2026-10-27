
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

#define THREADS 4
#define ITERATIONS 1000000

long long counter = 0;
atomic_int atomic_counter = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

long long local_counters[THREADS] = {0};

void *mutex_worker(void *arg)
{
    (void)arg;

    for (int i = 0; i < ITERATIONS; i++)
    {
        pthread_mutex_lock(&mutex);
        counter++;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void *atomic_worker(void *arg)
{
    (void)arg;

    for (int i = 0; i < ITERATIONS; i++)
    {
        atomic_fetch_add(&atomic_counter, 1);
    }

    return NULL;
}

void *local_worker(void *arg)
{
    int id = *(int *)arg;
    long long local = 0;

    for (int i = 0; i < ITERATIONS; i++)
    {
        local++;
    }

    local_counters[id] = local;

    return NULL;
}

double get_time(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);

    return t.tv_sec + t.tv_nsec / 1e9;
}

int main(void)
{
    pthread_t threads[THREADS];
    int ids[THREADS];

    for (int i = 0; i < THREADS; i++)
        ids[i] = i;

    for (int mode = 0; mode < 3; mode++)
    {
        counter = 0;
        atomic_store(&atomic_counter, 0);

        for (int i = 0; i < THREADS; i++)
            local_counters[i] = 0;

        double start = get_time();

        for (int i = 0; i < THREADS; i++)
        {
            void *(*worker)(void *);

            if (mode == 0)
                worker = mutex_worker;
            else if (mode == 1)
                worker = atomic_worker;
            else
                worker = local_worker;

            int rc = pthread_create(
                &threads[i], NULL, worker, &ids[i]);

            if (rc != 0)
            {
                fprintf(stderr, "pthread_create failed: %d\n", rc);
                return 1;
            }
        }

        for (int i = 0; i < THREADS; i++)
        {
            int rc = pthread_join(threads[i], NULL);

            if (rc != 0)
            {
                fprintf(stderr, "pthread_join failed: %d\n", rc);
                return 1;
            }
        }

        long long result = 0;

        if (mode == 0)
            result = counter;
        else if (mode == 1)
            result = atomic_load(&atomic_counter);
        else
        {
            for (int i = 0; i < THREADS; i++)
                result += local_counters[i];
        }

        double elapsed = get_time() - start;

        const char *names[] = {
            "Mutex",
            "Atomic",
            "Local counters"
        };

        printf("%s: Counter = %lld, Time = %.6f sec\n",
               names[mode], result, elapsed);
    }

    pthread_mutex_destroy(&mutex);

    printf("Expected counter = %d\n",
           THREADS * ITERATIONS);

    return 0;
}
