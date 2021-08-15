#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER 100
#define ERR(source) (perror(source),                                 \
                     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                     exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE: %s n m k\n", pname);
    exit(EXIT_FAILURE);
}

void read_args(int argc, char **argv, int *n, int *m, int *k)
{
    if (argc > 3)
    {
        *n = atoi(argv[1]);
        *m = atoi(argv[2]);
        *k = atoi(argv[3]);
        if (1 <= *n && *n <= 100 && 1 <= *m && *m <= 100 && 3 <= *k && *k <= 100)
            return;
    }
    usage(argv[0]);
}

void *do_work(void *unused)
{
    printf("%ld\n", pthread_self());
    sleep(1);
    printf("%ld\n", pthread_self());
    return NULL;
}

int parse(char *line)
{
    if (strcmp(line, "print\n") == 0)
        return 0;
    if (strcmp(line, "exit\n") == 0)
        return -1;
    char *cmd = strtok(line, " ");
    char *count = strtok(NULL, " ");
    if (!(cmd && count))
        ERR("strtok()");
    if (strcmp(cmd, "replace") == 0)
    {
        int c = atoi(count);
        if (c > 0)
            return c;
    }
    return -2;
}

int main(int argc, char **argv)
{
    int n, m, k;
    read_args(argc, argv, &n, &m, &k);

    int size = n * sizeof(int *) + n * m * sizeof(int);
    int **arr = malloc(size);
    pthread_mutex_t *locks = malloc(n * sizeof(*locks));
    pthread_t *threads = malloc(k * sizeof(*threads));
    if (!(arr && locks && threads))
        ERR("malloc()");
    int *ptr = (int *)(arr + n);
    for (int i = 0; i < n; ++i)
        arr[i] = (ptr + m * i);
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < m; ++j)
            arr[i][j] = rand() % ('z' - 'a' + 1) + 'a';
        if (pthread_mutex_init(&locks[i], NULL))
            ERR("pthread_mutex_init()");
    }
    for (int i = 0; i < k; ++i)
    {
        if (pthread_create(&threads[i], NULL, do_work, NULL))
            ERR("pthread_create()");
    }

    char line[BUFFER];
    while (fgets(line, BUFFER, stdin) != NULL)
    {
        int c = parse(line);
        if (c == 0)
            for (int i = 0; i < n; ++i)
            {
                for (int j = 0; j < m; ++j)
                    putchar(arr[i][j]);
                putchar('\n');
            }
        else if (c == -1)
        {
            for (int i = 0; i < k; ++i)
                pthread_kill(threads[i], SIGINT);
            break;
        }
        else if (c != -2)
            for (int i = 0; i < c; ++i)
                kill(0, SIGUSR1);
    }

    for (int i = 0; i < k; ++i)
        if (pthread_join(threads[i], NULL))
            ERR("pthread_join()");
    free(threads);
    return EXIT_SUCCESS;
}
