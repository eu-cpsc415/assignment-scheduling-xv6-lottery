# An xv6 Lottery Scheduler

In this project, you'll be putting a new scheduler into xv6. It is called a **lottery scheduler** and is described in [this chapter](https://pages.cs.wisc.edu/~remzi/OSFEP/cpu-sched-lottery.pdf) of the textbook book.

The idea is simple: assign each running process a slice of the processor based in proportion to the number of tickets it has; the more tickets a process has, the more it runs. Each time slice, a randomized lottery determines the winner of the lottery; that winning process is the one that runs for that time slice.

The objectives for this project:

- To gain further knowledge of a real kernel, xv6.
- To familiarize yourself with a scheduler.
- To change that scheduler to a new algorithm.

## Details

### System Calls

You will need to implement two new system calls for this project.

The first is `int settickets(int number)`. This sets the number of tickets of the calling process. By default, each process should get one ticket. This routine allows a process to raise the number of tickets it receives, and thus receive a higher proportion of CPU cycles. This routine should return 0 if successful, and -1 otherwise (if, for example, the caller passes in a number less than one).

The second is `int getpinfo(struct pstat *)`. This routine returns information about all running processes, including how many times each has been chosen to run and the process ID of each. This routine should return 0 if successful, and -1 otherwise (if, for example, a bad or NULL pointer is passed into the kernel).

The structure `pstat` is defined below; note, you cannot change this structure, and must use it exactly as is. Store this in a file named `pstat.h` in the `src/kernel` directory.

```c
#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

struct pstat {
  int inuse[NPROC];   // whether this slot of the process table is in use (1 or 0)
  int tickets[NPROC]; // the number of tickets this process has
  int pid[NPROC];     // the PID of each process 
  int ticks[NPROC];   // the number of ticks each process has accumulated
  char names[NPROC][PROCNAMESZ]; // process names
};

#endif // _PSTAT_H_
```

You'll need to understand how to fill in the structure `pstat` in the kernel and pass the results to user space. Good examples of how to pass arguments into the kernel are found in existing system calls. In particular, check out `sys_fstat()` and `filestat()`, which will show you how to use `argaddr()` (and related calls) to obtain a pointer that has been passed into the kernel. Note how careful the kernel is with pointers passed from user space -- they are a security threat(!), and thus must be checked very carefully before usage.

If you wish, you can use the new `getpinfo()` system call to build a variant of the command line program `ps`, which can then be called to observe the state of running processes. 

### Scheduler

Most of the code for the scheduler is quite localized and can be found in `proc.c`; the associated header file, `proc.h` is also quite useful to examine. To change the scheduler, not much needs to be done; study its control flow and then try some small changes. 

You'll need to assign tickets to a process when it is created. Specfically, you'll need to make sure a child process *inherits* the same number of tickets as its parents. Thus, if the parent has 10 tickets, and calls `fork()` to create a child process, the child should also get 10 tickets.

You'll also need to adjust the total number of tickets in the lottery whenever a process sleeps or wakes up (pay attention to all the places where the state of a process can change in `proc.c`). If a process sleeps, it's tickets should no longer be taken into consideration for the lottery (and vice versa).

You'll also need to figure out how to generate random numbers in the kernel; some searching should lead you to a simple pseudo-random number generator, which you can then include in the kernel and use as appropriate.

### Testing

There is a test program that is used to verify the behavior of your scheduler. It is a user program named `testlotto`. This program is only included when using the `Makefile.test` makefile.

You can trigger the automated test script from the root of the repository using this command:

```
./test-lottery.sh
```

You can manually make and run the test build. This can be more convenient for development. You must be inside the `src` directory to do this:

```
make -f Makefile.test CPUS=1 qemu
```

When the shell starts, run the test program:

```
testlotto
```
