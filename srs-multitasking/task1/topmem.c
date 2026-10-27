
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>

typedef struct {
    int pid;
    long rss;
    char name[256];
} Process;

int compare(const void *a, const void *b)
{
    const Process *p1 = a;
    const Process *p2 = b;

    if (p1->rss < p2->rss) return 1;
    if (p1->rss > p2->rss) return -1;
    return 0;
}

int main(void)
{
    while (1) {
        DIR *dir = opendir("/proc");

        if (dir == NULL) {
            perror("opendir");
            return 1;
        }

        static Process processes[65536];
        int count = 0;
        struct dirent *entry;

        while ((entry = readdir(dir)) != NULL) {
            if (!isdigit((unsigned char)entry->d_name[0]))
                continue;

            char path[512];

            snprintf(path, sizeof(path),
                     "/proc/%s/status", entry->d_name);

            FILE *file = fopen(path, "r");

            if (file == NULL)
                continue;

            Process p = {0};
            p.pid = atoi(entry->d_name);

            char line[512];

            while (fgets(line, sizeof(line), file)) {
                if (strncmp(line, "Name:", 5) == 0)
                    sscanf(line + 5, "%255s", p.name);

                if (strncmp(line, "VmRSS:", 6) == 0)
                    sscanf(line + 6, "%ld", &p.rss);
            }

            fclose(file);

            if (count < 65536)
                processes[count++] = p;
        }

        closedir(dir);

        qsort(processes, count, sizeof(Process), compare);

        printf("\033[2J\033[H");
        printf("TOP 10 MEMORY PROCESSES\n\n");

        printf("%-10s %-25s %12s\n",
               "PID", "NAME", "VmRSS(KB)");

        for (int i = 0; i < count && i < 10; i++) {
            printf("%-10d %-25s %12ld\n",
                   processes[i].pid,
                   processes[i].name,
                   processes[i].rss);
        }

        fflush(stdout);
        sleep(2);
    }

    return 0;
}
