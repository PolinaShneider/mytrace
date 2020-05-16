#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>

#define MAX_RESULTS 15

int find_nums(int do_yield, int skip_syscall, long int *results)
{
    long int a, b, c;
    int count = 0;
    struct timespec t;

    for (a = 100; a <= 999; a++)
    {
        for (b = 0; b <= 999; b++)
        {
            for (c = 0; c <= 999; c++)
            {
                if (a * a * a + b * b * b + c * c * c == a * 1000 * 1000 + b * 1000 + c)
                {
                    if (count < MAX_RESULTS)
                    {
                        results[count] = a * 1000 * 1000 + b * 1000 + c;
                    }
                    count++;
                }
                if (!skip_syscall & c % 200 == 0)
                {
                    if (do_yield)
                        sched_yield();
                    else
                        sched_rr_get_interval(0, &t);
                }
            }
        }
    }

    return count;
}

int main(int argc, char **argv)
{
    int do_yield;
    int skip_syscall;

    long int results[MAX_RESULTS];
    int count_results;

    skip_syscall = (argc < 2) || (strcmp(argv[1], "--skip-syscall") == 0);
    do_yield = (argc > 1) && strcmp(argv[1], "--do-yield") == 0;

    sleep(1); // allow run second proc

    count_results = find_nums(do_yield, skip_syscall, results);
    return 0;

    for (int i = 0; i < count_results; i++)
    {
        printf("%ld\n", results[i]);
    }

    return 0;
}
