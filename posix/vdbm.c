#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define LEN 32

#define ERR(source) (perror(source),                                 \
                     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                     exit(EXIT_FAILURE))

void usage(char *pname)
{
    fprintf(stderr, "USAGE: %s create file.vdb\n", pname);
    exit(EXIT_FAILURE);
}

typedef struct
{
    char option[LEN];
    int votes;
} data;
data vdb[10];

int main(int argc, char **argv)
{
    if (argc < 3)
        usage("argv[0]");
    if (strcmp(argv[1], "create") == 0)
    {
        struct stat filestat;
        if (!stat(argv[2], &filestat))
            ERR("stat");
        FILE *f = fopen(argv[2], "w");
        if (!f)
            ERR("fopen");
        for (int i = 3, j = 0; argv[i]; ++i, ++j)
        {
            strcpy(vdb[j].option, argv[i]);
            vdb[j].votes = 0;
            fprintf(f, "%-32s%-32d\n", vdb[j].option, vdb[j].votes);
        }
        if (fclose(f))
            ERR("fclose");
        return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "show") == 0)
    {
        FILE *f = fopen(argv[2], "r");
        if (!f)
            ERR("fopen");
        printf("Database filename: %s\n", argv[2]);
        char line[LEN];
        while (fgets(line, LEN, f) != NULL)
            printf("%s", line);
        printf("Database is locked/unlocked\n");
        if (fclose(f))
            ERR("fclose");
    }
}