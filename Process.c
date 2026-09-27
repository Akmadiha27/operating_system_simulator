#include "ScisSos.h"
void scissos_proc_delete(int);
static int creator_pid = 1;
static int creator_target = 0;
static int creator_created = 0;
static int next_pid = 2;

ScisSosProcess *scissos_proc_create(char *name, int size,
                                     int priority, int pid)
{
    ScisSosProcess *p = malloc(sizeof(ScisSosProcess));
    if (p == NULL)
        return NULL;
    p->_PID = pid;
    p->_psize = size;
    strcpy(p->_pname, name);
    ScisSosPCB *pcb = malloc(sizeof(ScisSosPCB));
    if (pcb == NULL)
    {
        free(p);
        return NULL;
    }
    p->_pcb = pcb;
    pcb->pid = pid;
    pcb->uid = 1;
    pcb->size = size;
    pcb->priority_value = priority;
    pcb->ps_state = PS_RDY;
    pcb->pc = 0;
    pcb->p_timeslice = DEFTS;

    int type = rand() % 3;
    double long_probability;
    if (type == PT_REG)
    {
        pcb->p_type = PT_REG;
        long_probability = REG_THR;
    }
    else if (type == PT_CMP)
    {
        pcb->p_type = PT_CMP;
        long_probability = CMP_THR;
    }
    else
    {
        pcb->p_type = PT_IOE;
        long_probability = IOE_THR;
    }
    pcb->m_type = MT_GOOD;
    p->_CODE = malloc(sizeof(ScisSosInst *) * size);
    if (p->_CODE == NULL)
    {
        free(pcb);
        free(p);
        return NULL;
    }
    for (int i = 0; i < size; i++)
    {
        p->_CODE[i] = malloc(sizeof(ScisSosInst));

        if (p->_CODE[i] == NULL)
        {
            for (int j = 0; j < i; j++)
                free(p->_CODE[j]);
            free(p->_CODE);
            free(pcb);
            free(p);
            return NULL;
        }
        p->_CODE[i]->_inum = i;
        p->_CODE[i]->_addref = 100;
        if ((double)rand() / RAND_MAX < long_probability)
        {
            p->_CODE[i]->_syscall = INS_LNG;
        }
        else
        {
            p->_CODE[i]->_syscall = INS_SHR;
        }
    }
    pcb->p_code = p->_CODE;
    _proctable[pid] = pcb;
    gettimeofday(&pr_times[pid].crt_time, NULL);
    FILE *fp = fopen("processes.txt", "a");
    if (fp != NULL)
    {   
        //scissos_print_pcb(p, fp);
        scissos_proc_save(p, fp);
        fclose(fp);
    }
    return p;
}

ScisSosProcess *ScisSosPCreator(int num)
{
    ScisSosProcess *creator;
    creator_target = num;
    creator_created = 0;
    next_pid = 2;
    creator = malloc(sizeof(ScisSosProcess));
    if (creator == NULL)
        return NULL;
    strcpy(creator->_pname, "Process_Creator");
    creator->_PID = creator_pid;
    creator->_psize = 0;
    ScisSosPCB *pcb = malloc(sizeof(ScisSosPCB));
    if (pcb == NULL)
    {
        free(creator);
        return NULL;
    }
    creator->_pcb = pcb;
    creator->_CODE = NULL;

    pcb->pid = creator_pid;
    pcb->uid = 1;
    pcb->size = 0;
    pcb->priority_value = DEFPRIO;
    pcb->ps_state = PS_RDY;
    pcb->p_type = PT_REG;
    pcb->m_type = MT_GOOD;
    pcb->pc = 0;
    pcb->p_timeslice = DEFTS;
    _proctable[creator_pid] = pcb;
    gettimeofday(&pr_times[creator_pid].crt_time, NULL);
    return creator;
}
int scissos_proc_run(int pid)
{
    if (pid < 1 || pid >= MAXPROC)
        return -1;
    ScisSosPCB *pcb = _proctable[pid];
    if (pcb == NULL)
        return -1;
    if (pid == creator_pid)
    {
        if (creator_created >= creator_target)
        {
            pcb->ps_state = PS_DEAD;
            scissos_proc_delete(pid);
            return 0;
        }
        if (next_pid >= MAXPROC)
        {
            pcb->ps_state = PS_DEAD;
            scissos_proc_delete(pid);
            return 0;
        }
        char name[MAXPROCNAME];
        sprintf(name, "Process_%d", next_pid);
        int size = 24000 + rand() % 1000;
        int priority = DEFPRIO;
        ScisSosProcess *new_process =
            scissos_proc_create(name, size, priority, next_pid);
        if (new_process == NULL)
            return -1;
        printf("Creator created PID: %d\n", next_pid);
        creator_created++;
        next_pid++;
        if (creator_created < creator_target)
        {
            pcb->ps_state = PS_RDY;
        }
        else
        {
            pcb->ps_state = PS_DEAD;
            scissos_proc_delete(pid);
        }
        return 0;
    }
    int executed = 0;
    if (pcb->pc == 0)
    {
        gettimeofday(&pr_times[pid].rsp_time, NULL);
    }
    while (pcb->pc < pcb->size &&
           executed < pcb->p_timeslice)
    {
        ScisSosInst *inst = pcb->p_code[pcb->pc];
        pcb->pc++;
        executed++;
        if (inst->_syscall == INS_LNG)
        {
            pcb->ps_state = PS_RDY;
            return 0;
        }
    }
    if (pcb->pc >= pcb->size)
    {
        pcb->ps_state = PS_DEAD;

        /*gettimeofday(&pr_times[pid].end_time, NULL);
        struct timeval *total_time =_diff_times(pr_times[pid].end_time,pr_times[pid].crt_time);
        struct timeval *wait_time =_diff_times(*total_time,pr_times[pid].run_time);
        pr_times[pid].wt_time = *wait_time;
        free(total_time);
        free(wait_time);*/

        scissos_proc_delete(pid);
        return 0;
    }
    if (executed >= pcb->p_timeslice)
    {
        pcb->ps_state = PS_RDY;
        return 0;
    }
    return 0;
}
void scissos_proc_delete(int pid)
{
    if (pid < 1 || pid >= MAXPROC)
        return;
    ScisSosPCB *pcb = _proctable[pid];
    if (pcb == NULL)
        return;
    if (pcb->p_code != NULL)
    {
        for (int i = 0; i < pcb->size; i++)
        {
            free(pcb->p_code[i]);
        }
        free(pcb->p_code);
    }
    free(pcb);
    _proctable[pid] = NULL;
}
void scissos_print_pcb(ScisSosProcess *process, FILE *fp)
{
    if (process == NULL || process->_pcb == NULL)
        return;
    ScisSosPCB *pcb = process->_pcb;

    fprintf(fp, "Process Name : %s\n", process->_pname);
    fprintf(fp, "PID          : %d\n", pcb->pid);
    fprintf(fp, "UID          : %d\n", pcb->uid);
    fprintf(fp, "Size         : %d\n", pcb->size);
    fprintf(fp, "Priority     : %d\n", pcb->priority_value);
    fprintf(fp, "State        : %d\n", pcb->ps_state);
    fprintf(fp, "Process Type : %d\n", pcb->p_type);
    fprintf(fp, "Memory Type  : %d\n", pcb->m_type);
    fprintf(fp, "PC           : %d\n", pcb->pc);
    fprintf(fp, "Time Slice   : %d\n", pcb->p_timeslice);
    fprintf(fp, "_____________________\n");
}

int scissos_proc_save(ScisSosProcess *process, FILE *fp)
{
    if (process == NULL || process->_pcb == NULL || fp == NULL)
        return -1;

    fprintf(fp, "Process Name : %s\n", process->_pname);
    fprintf(fp, "PID          : %d\n", process->_PID);
    fprintf(fp, "Size         : %d\n", process->_psize);
    fprintf(fp, "_____________________\n");
    scissos_print_pcb(process, fp);

    return 0;
}