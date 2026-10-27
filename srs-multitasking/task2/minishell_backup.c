
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_ARGS 64
#define MAX_LINE 1024

static volatile sig_atomic_t child_finished = 0;

void sigchld_handler(int sig)
{
    (void)sig;
    child_finished = 1;
}

void collect_children(void)
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (WIFEXITED(status))
        {
            printf("[Background] PID=%ld, exit=%d\n",
                   (long)pid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("[Background] PID=%ld, signal=%d\n",
                   (long)pid, WTERMSIG(status));
        }
    }

    child_finished = 0;
}

int main(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    char line[MAX_LINE];
    char *args[MAX_ARGS];

    while (1)
    {
        if (child_finished)
            collect_children();

        printf("minishell> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            if (feof(stdin))
                break;

            if (errno == EINTR)
            {
                clearerr(stdin);
                continue;
            }

            perror("fgets");
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        int count = 0;
        char *token = strtok(line, " \t");

        while (token != NULL && count < MAX_ARGS - 1)
        {
            args[count++] = token;
            token = strtok(NULL, " \t");
        }

        args[count] = NULL;

        if (count == 0)
            continue;

        if (strcmp(args[0], "exit") == 0)
            break;

        if (strcmp(args[0], "cd") == 0)
        {
            const char *path = args[1];

            if (path == NULL)
                path = getenv("HOME");

            if (path == NULL || chdir(path) == -1)
                perror("cd");

            continue;
        }

        int background = 0;

        if (strcmp(args[count - 1], "&") == 0)
        {
            background = 1;
            args[--count] = NULL;
        }

        if (count == 0)
            continue;

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            continue;
        }

        if (pid == 0)
        {
            execvp(args[0], args);
            perror("execvp");
            _exit(127);
        }

        printf("Child PID: %ld\n", (long)pid);

        if (background)
        {
            printf("Background process started\n");
        }
        else
        {
            int status;
            pid_t result;

            do
            {
                result = waitpid(pid, &status, 0);
            }
            while (result == -1 && errno == EINTR);

            if (result == -1)
            {
                perror("waitpid");
            }
            else if (WIFEXITED(status))
            {
                printf("PID=%ld, exit=%d\n",
                       (long)pid, WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                printf("PID=%ld, signal=%d\n",
                       (long)pid, WTERMSIG(status));
            }
        }
    }

    collect_children();
    printf("Mini Shell finished\n");

    return 0;
}
