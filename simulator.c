#include "ScisSos.h"
//timing functions
struct timeval *_diff_times(struct timeval end, struct timeval start)
{
    struct timeval *result = malloc(sizeof(struct timeval));
    result->tv_sec = end.tv_sec - start.tv_sec;
    result->tv_usec = end.tv_usec - start.tv_usec;
    if (result->tv_usec < 0)
    {
        result->tv_sec--;
        result->tv_usec += 1000000;
    }
    return result;
}

struct timeval *_add_times(struct timeval t1, struct timeval t2)
{
    struct timeval *result = malloc(sizeof(struct timeval));
    result->tv_sec = t1.tv_sec + t2.tv_sec;
    result->tv_usec = t1.tv_usec + t2.tv_usec;
    if (result->tv_usec >= 1000000)
    {
        result->tv_sec++;
        result->tv_usec -= 1000000;
    }
    return result;
}

void scissos_print_timings(void)
{
    long long total_run = 0;
    long long total_wait = 0;
    long long total_response = 0;
    int process_count = 0;

    printf("\n");
    printf("PID\tCRT(us)\tRSP(us)\tRUN(us)\tWAIT(us)\tTOTAL(us)\n");
    printf("------------------------------------------------------------\n");

    for (int pid = 2; pid < MAXPROC; pid++)
    {
        if (pr_times[pid].crt_time.tv_sec != 0 ||
            pr_times[pid].crt_time.tv_usec != 0)
        {
            struct timeval *crt =
                _diff_times(pr_times[pid].crt_time, _basetime);

            struct timeval *rsp =
                _diff_times(pr_times[pid].rsp_time, _basetime);

            struct timeval *total =
                _diff_times(pr_times[pid].end_time,
                            pr_times[pid].crt_time);

            long long crt_us =
                (long long)crt->tv_sec * 1000000 +
                crt->tv_usec;

            long long rsp_us =
                (long long)rsp->tv_sec * 1000000 +
                rsp->tv_usec;

            long long run_us =
                (long long)pr_times[pid].run_time.tv_sec * 1000000 +
                pr_times[pid].run_time.tv_usec;

            long long wait_us =
                (long long)pr_times[pid].wt_time.tv_sec * 1000000 +
                pr_times[pid].wt_time.tv_usec;

            long long total_us =
                (long long)total->tv_sec * 1000000 +
                total->tv_usec;

            printf("%d\t%lld\t%lld\t%lld\t%lld\t%lld\n",
                   pid,
                   crt_us,
                   rsp_us,
                   run_us,
                   wait_us,
                   total_us);

            total_run += run_us;
            total_wait += wait_us;
            total_response += rsp_us;

            process_count++;

            free(crt);
            free(rsp);
            free(total);
        }
    }

    printf("------------------------------------------------------------\n");

    printf("Total Run Time      : %lld us\n", total_run);
    printf("Total Response Time : %lld us\n", total_response);
    printf("Total Wait Time     : %lld us\n", total_wait);

    if (process_count > 0)
    {
        printf("Average Run Time      : %.2f us\n",
               (double)total_run / process_count);

        printf("Average Response Time : %.2f us\n",
               (double)total_response / process_count);

        printf("Average Wait Time     : %.2f us\n",
               (double)total_wait / process_count);
    }
}

//initialize 

ScisSosPCB *_proctable[MAXPROC];
int _readyQ[MAXPROC];
int _blockQ[MAXPROC];

struct timeval _basetime;
TIMING pr_times[MAXPROC];
void scissos_initialise(void)
{
    for (int i = 0; i < MAXPROC; i++)
    {
        _proctable[i] = NULL;
    }

    for (int i = 0; i < MAXPROC; i++)
    {
        _readyQ[i] = EMPTY;
    }
    memset(pr_times, 0, sizeof(pr_times));
    gettimeofday(&_basetime, NULL);
    srand(_basetime.tv_usec);
}

//schdeuler part
int is_empty(int *queue)
{
    return queue[0] == EMPTY;
}
int is_present(int *queue, int pid)
{
    for (int i = 0; i < MAXPROC; i++)
    {
        if (queue[i] == EMPTY)
            return 0;
        if (queue[i] == pid)
            return 1;
    }
    return 0;
}
void push(int *queue, int pid)
{
    if (is_present(queue, pid))
        return;
    for (int i = 0; i < MAXPROC; i++)
    {
        if (queue[i] == EMPTY)
        {
            queue[i] = pid;
            return;
        }
    }
}
void remove_pid(int *queue, int pid)
{
    int position = -1;
    for (int i = 0; i < MAXPROC; i++)
    {
        if (queue[i] == pid)
        {
            position = i;
            break;
        }
    }
    if (position == -1)
        return;
    for (int i = position; i < MAXPROC - 1; i++)
    {
        queue[i] = queue[i + 1];
    }
    queue[MAXPROC - 1] = EMPTY;
}

void scissos_call_scheduler(void)
{
    while (1)
    {
        for (int i = 0; i < MAXPROC; i++)
        {
            ScisSosPCB *pcb = _proctable[i];
            if (pcb != NULL && pcb->ps_state == PS_RUN)
            {
                pcb->ps_state = PS_RDY;
            }
            if (pcb != NULL && pcb->ps_state == PS_RDY)
            {
                push(_readyQ, i);
            }
        }
        if (is_empty(_readyQ))
        {
            break;
        }
        int pid = scheduling_algorithm(_readyQ);
        if (pid == EMPTY)
        {
            break;
        }
        remove_pid(_readyQ, pid);
        _proctable[pid]->ps_state = PS_RUN;
        struct timeval start, end;
        gettimeofday(&start, NULL);
        scissos_proc_run(pid);
        gettimeofday(&end, NULL);
        struct timeval *elapsed = _diff_times(end, start);
        struct timeval *new_run_time = _add_times(pr_times[pid].run_time, *elapsed);
        pr_times[pid].run_time = *new_run_time;
        free(elapsed);
        free(new_run_time);

        //if process deleted then we have to take run-create for wait
        if (_proctable[pid] == NULL)
        {
            pr_times[pid].end_time = end;

            struct timeval *total_time =
                _diff_times(pr_times[pid].end_time,
                            pr_times[pid].crt_time);

            struct timeval *wait_time =
                _diff_times(*total_time,
                            pr_times[pid].run_time);

            pr_times[pid].wt_time = *wait_time;

            free(total_time);
            free(wait_time);
        }
    }
}