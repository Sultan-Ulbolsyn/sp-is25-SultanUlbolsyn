
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <string.h>

int main(void)
{
    printf("PID: %ld\n", (long)getpid());
    printf("PPID: %ld\n", (long)getppid());
    printf("UID: %ld\n", (long)getuid());

    printf("Process Group: %ld\n", (long)getpgrp());
    printf("Session ID: %ld\n", (long)getsid(0));

    int nice_value = getpriority(PRIO_PROCESS, 0);
    printf("Nice: %d\n", nice_value);

    FILE *file = fopen("/proc/self/status", "r");

    if (file == NULL)
    {
        perror("fopen");
        return 1;
    }

    char line[256];

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "State:", 6) == 0 ||
            strncmp(line, "voluntary_ctxt_switches:", 24) == 0 ||
            strncmp(line, "nonvoluntary_ctxt_switches:", 27) == 0)
        {
            printf("%s", line);
        }
    }

    fclose(file);

    return 0;
}

