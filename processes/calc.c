#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define ERR(source) (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                     perror(source), kill(0, SIGKILL),               \
                     exit(EXIT_FAILURE))

volatile sig_atomic_t last_signal;

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s n m\n", name);
    exit(EXIT_FAILURE);
}

void sig_handler(int sig)
{
    last_signal = sig;
}

void sethandler(void (*f)(int), int sig)
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = f;
    if (sigaction(sig, &act, NULL) == -1)
        ERR("sigaction()");
}

void child_work(int i, sigset_t oldmask)
{
    int sig_count = 0;
    for (;;)
    {
        printf("i=%d sig_count=%d \n", i, sig_count);

        last_signal = 0;
        while (last_signal != SIGUSR1 || last_signal != SIGUSR2)
            sigsuspend(&oldmask);

        if (last_signal == SIGUSR1)
            ++sig_count;
        if (last_signal == SIGUSR2)
        {
            pid_t pid = getpid();
            srand(time(NULL) * pid);
            int r = rand() % (2 - (-2) + 1) - 2;
            int new_sig_count = sig_count * r;
            printf("%d %d\n", pid, new_sig_count);
            sig_count += new_sig_count;
        }
    }
}

void create_children(int n)
{
    for (int i = 1; i <= n; ++i)
    {
        switch (fork())
        {
        case -1:
            ERR("fork()");
        case 0:
            sethandler(sig_handler, SIGUSR1);
            sigset_t mask, oldmask;
            sigemptyset(&mask);
            sigaddset(&mask, SIGUSR1);
            sigaddset(&mask, SIGUSR2);
            sigprocmask(SIG_BLOCK, &mask, &oldmask);
            child_work(i, oldmask);
            sigprocmask(SIG_UNBLOCK, &mask, NULL);
            exit(EXIT_SUCCESS);
        }
    }
}

int parent_work(int m, char *line)
{
    if (strcmp(line, "exit") == 0)
    {
        sethandler(SIG_IGN, SIGTERM);
        if (kill(0, SIGTERM))
            ERR("kill()");
        return 1;
    }
    int k = atoi(line);
    if (k > 2 && k < 20)
    {
        for (int i = 0; i < k; ++i)
        {
            sethandler(SIG_IGN, SIGUSR1);
            if (kill(0, SIGUSR1))
                ERR("kill()");
            struct timespec t = {0, m * 1000000};
            nanosleep(&t, NULL);
            sethandler(SIG_IGN, SIGUSR2);
            if (kill(0, SIGUSR2))
                ERR("kill()");
            return 0;
        }
    }
    return -1;
}

int main(int argc, char **argv)
{
    if (argc != 3)
        usage(argv[0]);
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    if (n < 3 || n > 10 || m < 1 || m > 500)
        usage(argv[0]);

    create_children(n);
    char line[5];
    while (scanf("%s", line))
    {
        int status = parent_work(m, line);
        if (status == 1)
            break;
        if (status == -1)
            fprintf(stderr, "fgets()");
    }

    while (wait(NULL) > 0)
        ;
    return EXIT_SUCCESS;
}