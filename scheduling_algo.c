#include "ScisSos.h"
// FCFS

int scheduling_algorithm(int *_readyQ)
{
    if (is_empty(_readyQ))
    {
        return EMPTY;
    }
    for (int i = 0; i < MAXPROC; i++)
    {
        if (_readyQ[i] != EMPTY)
        {
            return _readyQ[i];
        }
    }
    return EMPTY;
}