#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_LINE 80

int parser(char *src, char *args[])
{
    int i, k = 0;
    int arg_num = 0;
    while (src[k] != '\n')
    {
        i = 0;
        while (src[k] != ' ' && src[k] != '\n')
        {
            args[arg_num][i++] = src[k];
            ++k;
        }
        if (src[k] != '\n')
            ++k;
        if (i > 0)
        {
            args[arg_num][i] = '\0';
            ++arg_num;
        }
    }
    return arg_num;
}

int get_input(char *args[])
{
    char c;
    int i;
    int arg_num = 0;
    c = getchar();
    while (c != '\n')
    {
        i = 0;
        while (c != ' ' && c != '\n')
        {
            args[arg_num][i++] = c;
            c = getchar();
        }
        if (c != '\n')
            c = getchar();
        if (i > 0)
        {
            args[arg_num][i] = '\0';
            ++arg_num;
        }
    }
    return arg_num;
}

int main(void)
{
    char input[200];
    char *args[MAX_LINE / 2 + 1];
    char *para[MAX_LINE / 2 + 1];
    char *history[MAX_LINE / 2 + 1], history_len = 0;
    int should_run = 1, bg;
    int arg_num;
    int status, savefd, fd;
    pid_t pid;

    savefd = dup(STDOUT_FILENO);

    for (int i = 0; i < MAX_LINE / 2 + 1; ++i)
    {
        args[i] = (char *)malloc(sizeof(char) * 100);
        history[i] = (char *)malloc(sizeof(char) * 100);
    }

    while (should_run)
    {
        dup2(savefd, STDOUT_FILENO);
        printf("osh>");
        fflush(stdout);
        gets(input);
        arg_num = parser(input, args);
        if (arg_num == 0)
            continue;
        if (strcmp(args[0], "exit") == 0)
        {
            should_run = 0;
            continue;
        }
        if (strcmp(args[0], "!!") == 0)
        {
            if (history_len == 0)
            {
                printf("No commands in history.\n");
                continue;
            }
            for (int i = 0; i < history_len; ++i)
                strcpy(args[i], history[i]);
            arg_num = history_len;
        }
        history_len = arg_num;
        for (int i = 0; i < arg_num; ++i)
            strcpy(history[i], args[i]);
        if (strcmp(args[arg_num - 1], "&") == 0)
        {
            bg = 1;
            arg_num--;
        }
        else
            bg = 0;

        if(arg_num>2 && strcmp(args[arg_num-2], ">")==0)
        {

            fd = open(args[arg_num-1], O_RDWR|O_CREAT);
            if(fd<0)
            {
                perror("Open Fail");
                continue;
            }
            dup2(fd, STDOUT_FILENO);
            arg_num -= 2;
        }

        for (int i = 0; i < arg_num; ++i)
            para[i] = args[i];
        para[arg_num] = NULL;

        pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "Fork Failed\n");
            return 1;
        }
        else if (pid == 0)
        {
            //child process
            execvp(args[0], para);
            exit(0);
        }
        else
        {
            if (!bg)
                while (wait(&status) != pid)
                {
                }
        }
    }

    for (int i = 0; i < MAX_LINE / 2 + 1; ++i)
    {
        free(args[i]);
        free(history[i]);
    }
    return 0;
}