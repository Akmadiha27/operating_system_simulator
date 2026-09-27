/*****  Constants defining OS parameters****/
 #define MAXPROC     1000                  /* Max number of processes */
 #define MAXUSRS     10                    /* Max number of users */
 #define DEFPRIO     20                    /* Default priority for process */
 #define EMPTY       -100                  /* Unfilled entries */
 #define MAXPGES     10                    /* Max number of pages/process */
 #define DEFTS       6000                  /* Default time slice */
 #define REG_THR     0.02                  /* Normal process: 2% long calls */
 #define CMP_THR     0.001                 /* Compute Intensive: 0.1% */
 #define IOE_THR     0.2                   /* IO Intensive: 20% long calls */
 #define MAXPROCNAME 80                    /* Max Process Name Length */
 #define SCHEDFUNC   sched_rnd             /* Scheduling algorithm - random */
 
 /**** Constants for Process States ************************************/
 #define PS_NEW  0
 #define PS_RDY  1
 #define PS_RUN  2
 #define PS_BLK  3                         /* Self-explanatory, I guess! */
 #define PS_SRDY 4
 #define PS_SBLK 5
 #define PS_DEAD 6
 
 /**** Constants for Process Types *************************************/
 #define PT_REG  0                         /* Regular Process */
 #define PT_CMP  1                         /* Compute Intensive Process */
 #define PT_IOE  2                         /* IO Intensive Process */
 #define MT_GOOD 0                         /* Structured Memory Usage */
 #define MT_BAD  1                         /* Unstructured Memory Usage */
 #define MT_UGLY 2                         /* Spaghetti Code! */
 #define INS_LNG 10                        /* Long instruction */
 #define INS_SHR 20                        /* Short instruction */
 
 #define MAX(A, B)   (((A) > (B)) ? (A) : (B))
 #define MIN(A, B)   (((A) < (B)) ? (A) : (B))

//our added things

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
  #include <time.h>
  #ifndef _WINSOCKAPI_
    #include <winsock2.h>
  #endif
#else
  #include <sys/time.h>
#endif
int is_empty(int *);
void push(int *, int);
void remove_pid(int *, int );
int scheduling_algorithm(int *);
int is_present(int *, int);
//our added ends

 typedef int ScisSosPGTable[2];
 
 /** Instruction in a process; process is a sequence of instructions **/
 typedef struct {
      int _inum;                           /* Instruction Number */
      int _syscall;                        /* System call type: long/short */
      int _addref;                         /* Memory address reference */
 } ScisSosInst;
 
 /** Process Control Block structure **/
 typedef struct {
      int pid;                             /* Process ID 1 to MAXPROC */
      int uid;                             /* User ID 1 to MAXUSRS */
      int size;                            /* Size specified by users */
      int priority_value;                  /* Priority value */
      int ps_state;                        /* Process State */
      int p_type;                          /* Process Type (See above) */
      int m_type;                          /* Memory behaviour */
      int pc;                              /* Program Counter */
      ScisSosInst **p_code;                /* Pointer to exectable code */
      ScisSosPGTable pg_table[MAXPGES];    /* Page Table Information */
      int p_timeslice;                     /* Current Time-Slice */
 } ScisSosPCB;
 
 /** Process Structure **/
 typedef struct {
      char _pname[MAXPROCNAME];            /* Name of the process */
      int _PID;                            /* PID */
      int _psize;                          /* Size of the process */
      ScisSosPCB *_pcb;                    /* Pointer to its PCB */
      ScisSosInst **_CODE;                 /* Pointer to its code */
 } ScisSosProcess;
 
 /** Data structures used by the OS to do its management actions **/
 /** Defined in the scissos_os.c file                      **/
 extern ScisSosPCB *_proctable[MAXPROC];   /* Process Table */
 extern int _readyQ[MAXPROC];              /* Ready Queue */
 extern int _blockQ[MAXPROC];              /* Wait Queue */
 
 /** These are used only in Memory Management assignment - not now!  **/
 #define TBEG      0.02                    /* Set up phase of Process: 2% */
 #define TEND      0.02                    /* End phase of Process: 2% */
 
 /** Time maintenance structure **/
 /** Use this structure to maintain process creation time, response time, 
     waiting time and running time for comparing different scheduling 
     algorithms **/ 
 typedef struct {
      struct timeval crt_time, rsp_time, wt_time, run_time,end_time;
 } TIMING;
 
 extern struct timeval _basetime;
 extern TIMING pr_times[MAXPROC];
 
 struct timeval *_diff_times(struct timeval, struct timeval);
 struct timeval *_add_times(struct timeval, struct timeval);
 struct timeval _max_times(struct timeval, struct timeval);
 
 /** Process-related functions found in scissos_processes.c file **/
 /** Some functions not meant for users are found in the file    **/
 ScisSosProcess *scissos_proc_create(char *, int, int, int);
 ScisSosProcess *ScisSosPCreator(int); /* Special function to create a process */
 int scissos_proc_save(ScisSosProcess *, FILE *);
 void scissos_print_pcb(ScisSosProcess *, FILE *);
 int scissos_proc_run(int);
 
 /** OS-related functions found in scissos_os.c file **/
 void scissos_initialise(void);
 void scissos_call_scheduler(void);
 void scissos_print_timings(void);
 
 /** Memory management functions found in scissos_memory.c file **/
 /** THIS IS NOT FOR THE FIRST ASSIGNMENT **/
 int *memory_gen_addrefstrings(int, int);