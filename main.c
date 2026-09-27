#include "ScisSos.h"
#include <stdio.h>

int main(void)
{
    printf("Starting simulator...\n");
    scissos_initialise();
    printf("Creating process creator...\n");
    ScisSosProcess *creator = ScisSosPCreator(20);
    if (creator == NULL)
    {
        printf("Failed to create process creator.\n");
        return 1;
    }
    FILE *fp = fopen("processes.txt", "w");
    if (fp == NULL)
    {
        printf("Could not open processes.txt\n");
        return 1;
    }
    //scissos_print_pcb(creator, fp);
    scissos_proc_save(creator, fp);
    fclose(fp);
    printf("Starting scheduler...\n");
    scissos_call_scheduler();
    printf("Scheduler finished.\n");
    scissos_print_timings();
    return 0;
}