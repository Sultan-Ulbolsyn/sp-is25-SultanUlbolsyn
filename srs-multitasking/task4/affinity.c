
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>

#define LIMIT 1000000
#define THREADS 4

typedef struct {
    int id;
    int mode;
    int cpu_count;
    int count;
} ThreadData;

int is_prime(int n)
{
    if (n < 2)
        return 0;

    for (int i = 2; i <= n / i; i++)
    {
        if (n % i == 0)
            return 0;
    }

    return 1;
}

void *worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;

    if (data->mode != 0)
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);

        int cpu;

        if (data->mode == 1)
            cpu = 0;
        else
            cpu = data->id % data->cpu_count;

        CPU_SET(cpu, &cpuset);

        int rc = pthread_setaffinity_np(
            pthread_self(),
            sizeof(cpu_set_t),
            &cpuset
        );

        if (rc != 0)
        {
            fprintf(stderr,
                    "CPU affinity error: %d\n", rc);
            data->count = -1;
            return NULL;
        }
    }

    int start = 2 + data->id * (LIMIT / THREADS);
    int end = start + (LIMIT / THREADS);

    if (data->id == THREADS - 1)
        end = LIMIT + 1;

    int count = 0;

    for (int n = start; n < end; n++)
    {
        if (is_prime(n))
            count++;
    }

    data->count = count;

    return NULL;
}

double get_time(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);

    return t.tv_sec + t.tv_nsec / 1e9;
}

void run_test(int mode, int cpu_count)
{
    pthread_t threads[THREADS];
    ThreadData data[THREADS];

    double start = get_time();

    for (int i = 0; i < THREADS; i++)
    {
        data[i].id = i;
        data[i].mode = mode;
        data[i].cpu_count = cpu_count;
        data[i].count = 0;

        int rc = pthread_create(
            &threads[i], NULL, worker, &data[i]);

        if (rc != 0)
        {
            fprintf(stderr,
                    "Thread creation error: %d\n", rc);
            exit(1);
        }
    }

    int total = 0;
    int failed = 0;

    for (int i = 0; i < THREADS; i++)
    {
        pthread_join(threads[i], NULL);

        if (data[i].count < 0)
            failed = 1;
        else
            total += data[i].count;
    }

    double elapsed = get_time() - start;

    if (failed)
    {
        printf("Mode %d: FAILED\n", mode);
        return;
    }

    printf("Mode %d: Primes = %d, Time = %.6f sec\n",
           mode, total, elapsed);
}

int main(void)
{
    int cpu_count = (int)sysconf(_SC_NPROCESSORS_ONLN);

    printf("Available CPUs: %d\n", cpu_count);
    printf("Threads: %d\n", THREADS);

    run_test(0, cpu_count);
    run_test(1, cpu_count);
    run_test(2, cpu_count);

    return 0;
}
