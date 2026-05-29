/*
Test notes:

Needs to be run with CPUS=1. We are testing the scheduler; we don't want
to spread processes out among multiple CPUs. Timing of the test and test
verification is done with the assumption of a single CPU.

Child processes perform CPU-bound work (no IO, sleep, etc.). They should
not exit on their own. The parent test process should kill them.

Getting process stats is only viable when the child processes are still
alive. If you sample process stats after a child process is killed,
you won't see those stats (they will be cleared when the process is freed).

Floating point arithmetic is not supported.
*/

#include "kernel/pstat.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Checks if a value is outside of lower and upper bounds
#define IS_OUTSIDE_BOUNDS(val, lower, upper) ( ((val) < (lower)) || ((val) > (upper)) )

// Number of ticks to run test for (may be +/- a few in reality)
#define TEST_TICKS 500

static volatile unsigned sink;

static void run_child()
{
  // Infinite loop, don't return (parent will kill when finished)

  unsigned x = 1;
  while (1) {
    x = x * 1664525 + 1013904223;
    sink = x;
  }
}

// returns child PID
static int spawn_child(int tickets)
{
  int pid = fork();
  if (pid < 0) {
    printf("Fork failed.\n");
    exit(1);
  }
  else if (pid == 0) {
    // Child
    settickets(tickets);
    run_child();

    // should never return
  }
  else {
    // Parent
  }

  return pid;
}

int
main(int argc, char *argv[])
{
  printf("Test initializing, spawning child processes.\n");

  settickets(50);
  int start_ticks = uptime();

  // Start child processes
  int c1 = spawn_child(100);
  int c2 = spawn_child(200);
  int c3 = spawn_child(300);

  // Let children work for a bit (pause parent process for some ticks)
  printf("Test running, this can take a minute or two.\n");
  pause(TEST_TICKS);

  // Sample process stats
  struct pstat stats;
  if (getpinfo(&stats)) {
    printf("Failed to get process stats.\n");
    exit(1);
  }

  int end_ticks = uptime();
  int elapsed_ticks = end_ticks - start_ticks;

  // Done with child processes
  kill(c1);
  kill(c2);
  kill(c3);

  // Wait for children to exit
  wait(0);
  wait(0);
  wait(0);

  // Compute expected values
  // Allow some margin of error (lottery scheduler is probabilistic not deterministic)
  // Using 15% since we don't want to run a long time (maybe 15% is too wide a margin?)

  // c1 had 100 / 600 tickets, so ~33% (1/3)
  // 15% margin: +/- 15
  int c1lower = (85 * elapsed_ticks) / 600;
  int c1upper = (115 * elapsed_ticks) / 600;

  // c2 had 200 / 600 tickets, so ~33% (1/3)
  // 15% margin: +/- 30
  int c2lower = (170 * elapsed_ticks) / 600;
  int c2upper = (230 * elapsed_ticks) / 600;

  // c3 had 300 / 600 tickets, so 50% (1/2)
  // 15% margin: +/- 45
  int c3lower = (255 * elapsed_ticks) / 600;
  int c3upper = (345 * elapsed_ticks) / 600;

  printf("Ticks: start (%d), end (%d), elapsed (%d)\n", start_ticks, end_ticks, elapsed_ticks);
  int pass = 1;

  for (int i = 0; i < NPROC; i++)
  {
    if (stats.pid[i] == c1) {
      printf("child 1: ticks (%d), lower (%d), upper (%d)\n", stats.ticks[i], c1lower, c1upper);

      if (IS_OUTSIDE_BOUNDS(stats.ticks[i], c1lower, c1upper)) {
        printf("XV6_TEST_OUTPUT: c1 not within margin.\n");
        pass = 0;
      }
    }
    else if (stats.pid[i] == c2) {
      printf("child 2: ticks (%d), lower (%d), upper (%d)\n", stats.ticks[i], c2lower, c2upper);

      if (IS_OUTSIDE_BOUNDS(stats.ticks[i], c2lower, c2upper)) {
        printf("XV6_TEST_OUTPUT: c2 not within margin.\n");
        pass = 0;
      }
    }
    else if (stats.pid[i] == c3) {
      printf("child 3: ticks (%d), lower (%d), upper (%d)\n", stats.ticks[i], c3lower, c3upper);

      if (IS_OUTSIDE_BOUNDS(stats.ticks[i], c3lower, c3upper)) {
        printf("XV6_TEST_OUTPUT: c3 not within margin.\n");
        pass = 0;
      }
    }
  }

  if (pass)
    printf("XV6_TEST_OUTPUT: passed\n");
  else
    printf("XV6_TEST_OUTPUT: failed\n");

  exit(0);
}
