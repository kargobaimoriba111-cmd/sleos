/*
 * =============================================================================
 * SLeOS - Process Scheduler (scheduler.h)
 *
 * Implements two scheduling algorithms for demonstration:
 *   1. First-Come First-Served (FCFS)
 *   2. Round-Robin (RR) with configurable time quantum
 *
 * Processes are simple descriptors — no real context switching to ring-3
 * (this is an educational prototype running in ring-0).  The scheduler
 * simulates CPU-time allocation and prints the execution trace to screen.
 * =============================================================================
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../kernel/types.h"

/* ── Constants ───────────────────────────────────────────────────────────── */
#define MAX_PROCESSES       8
#define RR_QUANTUM          2       /* time units per RR slice                */
#define MAX_PROC_NAME       16

/* ── Process States ──────────────────────────────────────────────────────── */
typedef enum {
    PROC_EMPTY   = 0,
    PROC_READY   = 1,
    PROC_RUNNING = 2,
    PROC_WAITING = 3,
    PROC_DONE    = 4
} ProcessState;

/* ── Process Control Block (PCB) ─────────────────────────────────────────── */
typedef struct {
    uint32_t      pid;
    char          name[MAX_PROC_NAME];
    ProcessState  state;
    uint32_t      burst_time;       /* total CPU time needed (time units)     */
    uint32_t      remaining_time;   /* time left (used by RR)                 */
    uint32_t      arrival_time;     /* time unit when process entered queue   */
    uint32_t      finish_time;      /* time unit when process completed       */
    uint32_t      waiting_time;     /* calculated after scheduling            */
    uint32_t      turnaround_time;  /* finish - arrival                       */
    uint32_t      priority;         /* lower value = higher priority (unused) */
} PCB;

/* ── Public API ──────────────────────────────────────────────────────────── */
void scheduler_init(void);

/* Add a process to the scheduler's process table */
int  scheduler_add_process(const char *name, uint32_t burst, uint32_t arrival);

/* Run FCFS scheduling simulation and print trace */
void scheduler_run_fcfs(void);

/* Run Round-Robin scheduling simulation and print trace */
void scheduler_run_rr(void);

/* Print the current process table (ps command) */
void scheduler_print_table(void);

/* Print FCFS + RR statistics comparison */
void scheduler_print_stats(void);

/* Reset — clear all processes */
void scheduler_reset(void);

/* Returns number of active (non-EMPTY) processes */
int  scheduler_count(void);

#endif /* SCHEDULER_H */
