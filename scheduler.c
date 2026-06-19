/*
 * =============================================================================
 * SLeOS - Process Scheduler (scheduler.c)
 *
 * FCFS:  Processes run to completion in arrival order.
 * Round-Robin: Each process gets RR_QUANTUM time units per turn.
 * =============================================================================
 */

#include "scheduler.h"
#include "../drivers/screen.h"
#include "types.h"

/* ── Private State ───────────────────────────────────────────────────────── */
static PCB   process_table[MAX_PROCESSES];
static int   proc_count  = 0;
static uint32_t next_pid = 1;

/* Simple string copy — no libc in kernel */
static void kstrcpy(char *dst, const char *src, int max)
{
    int i;
    for (i = 0; i < max - 1 && src[i]; i++)
        dst[i] = src[i];
    dst[i] = '\0';
}

/* Simple delay loop (visual pacing only) */
static void delay(uint32_t count)
{
    volatile uint32_t i;
    for (i = 0; i < count * 100000; i++);
}

/* ── Init ────────────────────────────────────────────────────────────────── */
void scheduler_init(void)
{
    int i;
    for (i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid   = 0;
        process_table[i].state = PROC_EMPTY;
    }
    proc_count = 0;
    next_pid   = 1;

    /* Pre-load demo processes representative of a low-resource computing env */
    scheduler_add_process("SysInit",    4, 0);
    scheduler_add_process("MemMgr",     3, 1);
    scheduler_add_process("EduLoader",  5, 2);
    scheduler_add_process("FileServ",   2, 3);
    scheduler_add_process("NetDaemon",  6, 4);
}

/* ── Add Process ─────────────────────────────────────────────────────────── */
int scheduler_add_process(const char *name, uint32_t burst, uint32_t arrival)
{
    if (proc_count >= MAX_PROCESSES) return -1;

    PCB *p = &process_table[proc_count];
    p->pid            = next_pid++;
    kstrcpy(p->name, name, MAX_PROC_NAME);
    p->state          = PROC_READY;
    p->burst_time     = burst;
    p->remaining_time = burst;
    p->arrival_time   = arrival;
    p->finish_time    = 0;
    p->waiting_time   = 0;
    p->turnaround_time= 0;
    p->priority       = proc_count;

    proc_count++;
    return (int)p->pid;
}

/* ── Print Table ─────────────────────────────────────────────────────────── */
void scheduler_print_table(void)
{
    println_color("", COLOR_DEFAULT);
    println_color("  PROCESS TABLE (ps)", COLOR_HEADER);
    print_line('-', COLOR_BORDER);

    print_color("  PID  NAME            STATE      BURST  REMAIN  PRIO\n", COLOR_INFO);
    print_line('-', COLOR_BORDER);

    int i;
    const char *state_names[] = {"EMPTY  ", "READY  ", "RUNNING", "WAITING", "DONE   "};

    for (i = 0; i < MAX_PROCESSES; i++) {
        PCB *p = &process_table[i];
        if (p->state == PROC_EMPTY) continue;

        print_color("  [", COLOR_DEFAULT);
        print_int(p->pid);
        print("]  ");

        /* Name padded to 14 chars */
        print_color(p->name, COLOR_TITLE);
        int len = 0;
        const char *nm = p->name;
        while (*nm++) len++;
        int j;
        for (j = len; j < 14; j++) put_char(' ');

        print_color(state_names[p->state], COLOR_WARNING);
        print("  ");
        print_int(p->burst_time);
        print("      ");
        print_int(p->remaining_time);
        print("       ");
        print_int(p->priority);
        print_newline();
    }
    print_line('-', COLOR_BORDER);
    print_color("  Total processes: ", COLOR_INFO);
    print_int(proc_count);
    print_newline();
}

/* ── Helper: find shortest remaining for FCFS (just uses arrival order) ─── */
static int find_next_fcfs(void)
{
    /* Find first READY process by arrival time */
    int best = -1;
    uint32_t min_arrival = 0xFFFFFFFF;
    int i;
    for (i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROC_READY &&
            process_table[i].arrival_time < min_arrival) {
            min_arrival = process_table[i].arrival_time;
            best = i;
        }
    }
    return best;
}

/* ── FCFS Scheduler Simulation ───────────────────────────────────────────── */
void scheduler_run_fcfs(void)
{
    /* Work on a local copy so we don't destroy the main table */
    PCB local[MAX_PROCESSES];
    int i;
    for (i = 0; i < MAX_PROCESSES; i++) local[i] = process_table[i];

    println_color("", COLOR_DEFAULT);
    println_color("  FCFS SCHEDULING SIMULATION", COLOR_HEADER);
    print_line('=', COLOR_BORDER);
    println_color("  Algorithm: First-Come, First-Served", COLOR_INFO);
    println_color("  (Non-preemptive — process runs until completion)", COLOR_DEFAULT);
    print_line('-', COLOR_BORDER);

    uint32_t time = 0;
    int n = 0;

    /* Count ready processes */
    for (i = 0; i < MAX_PROCESSES; i++)
        if (local[i].state == PROC_READY) n++;

    int done = 0;
    while (done < n) {
        /* Find next by arrival time */
        int best = -1;
        uint32_t min_arr = 0xFFFFFFFF;
        for (i = 0; i < MAX_PROCESSES; i++) {
            if (local[i].state == PROC_READY && local[i].arrival_time < min_arr) {
                min_arr = local[i].arrival_time;
                best    = i;
            }
        }
        if (best == -1) break;

        PCB *p = &local[best];
        if (time < p->arrival_time) time = p->arrival_time;

        p->state          = PROC_RUNNING;
        p->waiting_time   = time - p->arrival_time;
        p->finish_time    = time + p->burst_time;
        p->turnaround_time= p->finish_time - p->arrival_time;

        /* Visual trace */
        print_color("  [t=", COLOR_DEFAULT);
        print_int((int)time);
        print("] CPU -> ");
        print_color(p->name, COLOR_SUCCESS);
        print(" (PID ");
        print_int((int)p->pid);
        print(") burst=");
        print_int((int)p->burst_time);
        print(" | finish=");
        print_int((int)p->finish_time);
        print_newline();

        delay(1);

        time    = p->finish_time;
        p->state = PROC_DONE;
        done++;
    }

    print_line('-', COLOR_BORDER);
    println_color("  FCFS simulation complete.", COLOR_SUCCESS);

    /* Print statistics */
    uint32_t total_wt = 0, total_tat = 0;
    int count = 0;
    for (i = 0; i < MAX_PROCESSES; i++) {
        if (local[i].state == PROC_DONE) {
            total_wt  += local[i].waiting_time;
            total_tat += local[i].turnaround_time;
            count++;
        }
    }
    if (count > 0) {
        print_color("  Avg Waiting Time:    ", COLOR_INFO);
        print_int((int)(total_wt / count));
        println(" units");
        print_color("  Avg Turnaround Time: ", COLOR_INFO);
        print_int((int)(total_tat / count));
        println(" units");
    }
}

/* ── Round-Robin Scheduler Simulation ───────────────────────────────────── */
void scheduler_run_rr(void)
{
    /* Work on a local copy */
    PCB local[MAX_PROCESSES];
    int i;
    for (i = 0; i < MAX_PROCESSES; i++) local[i] = process_table[i];

    println_color("", COLOR_DEFAULT);
    println_color("  ROUND-ROBIN SCHEDULING SIMULATION", COLOR_HEADER);
    print_line('=', COLOR_BORDER);
    print_color("  Algorithm: Round-Robin | Quantum = ", COLOR_INFO);
    print_int(RR_QUANTUM);
    println(" time units");
    println_color("  (Preemptive — each process gets a fixed time slice)", COLOR_DEFAULT);
    print_line('-', COLOR_BORDER);

    /* Simple circular queue — just iterate processes repeatedly */
    uint32_t time  = 0;
    int      done  = 0;
    int      n     = 0;

    for (i = 0; i < MAX_PROCESSES; i++)
        if (local[i].state == PROC_READY) n++;

    int iterations = 0;
    int max_iter   = n * 50; /* safety cap */

    while (done < n && iterations < max_iter) {
        iterations++;
        int any_ran = 0;

        for (i = 0; i < MAX_PROCESSES; i++) {
            PCB *p = &local[i];
            if (p->state != PROC_READY && p->state != PROC_RUNNING) continue;
            if (p->arrival_time > time) continue;
            if (p->remaining_time == 0)  continue;

            p->state = PROC_RUNNING;

            uint32_t slice = (p->remaining_time < RR_QUANTUM)
                             ? p->remaining_time : RR_QUANTUM;

            /* Visual trace */
            print_color("  [t=", COLOR_DEFAULT);
            print_int((int)time);
            print("] CPU -> ");
            print_color(p->name, COLOR_WARNING);
            print(" slice=");
            print_int((int)slice);
            print(" remain=");
            print_int((int)(p->remaining_time - slice));
            print_newline();

            delay(1);

            time               += slice;
            p->remaining_time  -= slice;

            if (p->remaining_time == 0) {
                p->state           = PROC_DONE;
                p->finish_time     = time;
                p->turnaround_time = time - p->arrival_time;
                p->waiting_time    = p->turnaround_time - p->burst_time;
                done++;

                print_color("    -> ", COLOR_SUCCESS);
                print_color(p->name, COLOR_SUCCESS);
                println(" COMPLETED");
            } else {
                p->state = PROC_READY;
            }
            any_ran = 1;
        }

        if (!any_ran) time++; /* advance time if nothing ready */
    }

    print_line('-', COLOR_BORDER);
    println_color("  Round-Robin simulation complete.", COLOR_SUCCESS);

    uint32_t total_wt = 0, total_tat = 0;
    int count = 0;
    for (i = 0; i < MAX_PROCESSES; i++) {
        if (local[i].state == PROC_DONE) {
            total_wt  += local[i].waiting_time;
            total_tat += local[i].turnaround_time;
            count++;
        }
    }
    if (count > 0) {
        print_color("  Avg Waiting Time:    ", COLOR_INFO);
        print_int((int)(total_wt / count));
        println(" units");
        print_color("  Avg Turnaround Time: ", COLOR_INFO);
        print_int((int)(total_tat / count));
        println(" units");
    }
}

void scheduler_reset(void)
{
    int i;
    for (i = 0; i < MAX_PROCESSES; i++)
        process_table[i].state = PROC_EMPTY;
    proc_count = 0;
    next_pid   = 1;
    scheduler_init();
}

int scheduler_count(void) { return proc_count; }

void scheduler_print_stats(void) {
    println_color("  Use 'sched fcfs' and 'sched rr' to run simulations.", COLOR_INFO);
}
