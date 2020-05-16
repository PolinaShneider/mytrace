#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "defs.h"

struct_sysent syscall_entries[] = {
#include "syscallent.h"
};

char *syscallname = NULL;

int do_child(int argc, char **argv);
int do_trace(pid_t child);

int main(int argc, char **argv)
{
    int push = 1;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s [-n syscal_name] prog args\n", argv[0]);
        exit(1);
    }

    if (strcmp(argv[1], "-n") == 0)
    {
        syscallname = argv[2];
        push = 3;
    }

    pid_t child = fork();
    if (child == 0)
    {
        return do_child(argc - push, argv + push);
    }
    else
    {
        return do_trace(child);
    }
}

int do_child(int argc, char **argv)
{
    char *args[argc + 1];
    memcpy(args, argv, argc * sizeof(char *));
    args[argc] = NULL;

    ptrace(PTRACE_TRACEME);
    kill(getpid(), SIGSTOP);
    return execvp(args[0], args);
}

int wait_for_syscall(pid_t child);

int do_trace(pid_t child)
{
    int status, syscall, retval;
    waitpid(child, &status, 0);
    ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD);

    while (1)
    {
        if (wait_for_syscall(child) != 0)
            break;

        syscall = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * ORIG_RAX);

        if (!syscallname || strcmp(syscallname, syscall_entries[syscall].sys_name) == 0)
        {
            fprintf(stderr, "syscall(%s) = ", syscall_entries[syscall].sys_name);
        }

        if (wait_for_syscall(child) != 0)
            break;

        retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * RAX);
        if (!syscallname || strcmp(syscallname, syscall_entries[syscall].sys_name) == 0)
        {
            fprintf(stderr, "%d\n", retval);
        }
    }
    return 0;
}

int wait_for_syscall(pid_t child)
{
    int status;
    while (1)
    {
        ptrace(PTRACE_SYSCALL, child, 0, 0);
        waitpid(child, &status, 0);
        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80)
            return 0;
        if (WIFEXITED(status))
            return 1;
    }
}