
// Features: Stack usage simulation, Round-Robin fairness, Priority Inversion, Syscall API, Power states, Logging

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define MAX_TASKS 5
#define MAX_LOGS 200
#define MAX_STACK 1024  // 1KB simulated stack per task
#define TICK_INTERVAL 1
#define EVENT_ID 42

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    DELAYED,
    WAITING_EVENT,
    INTERRUPTED,
    TERMINATED,
    SLEEP,
    LOW_POWER_MODE
} task_state;

const char* state_to_str(task_state s) {
    switch (s) {
        case READY: return "READY";
        case RUNNING: return "RUNNING";
        case BLOCKED: return "BLOCKED";
        case DELAYED: return "DELAYED";
        case WAITING_EVENT: return "WAITING_EVENT";
        case INTERRUPTED: return "INTERRUPTED";
        case TERMINATED: return "TERMINATED";
        case SLEEP: return "SLEEP";
        case LOW_POWER_MODE: return "LOW_POWER_MODE";
        default: return "UNKNOWN";
    }
}

typedef struct {
    int id;
    char name[32];
    int base_priority;
    int current_priority;
    task_state state;
    void (*task_fn)(void);
    pthread_t thread;
    pthread_mutex_t lock;
    time_t wake_up_time;
    int waiting_for_event;
    int stack_used;
    int critical_section;
} task_t;

typedef struct {
    int tick;
    char type[16];
    int task_id;
    char task_name[32];
    char description[64];
    task_state state;
} log_entry_t;

task_t tasks[MAX_TASKS];
log_entry_t logs[MAX_LOGS];
int task_count = 0;
int log_count = 0;
int system_tick = 0;
int event_triggered = 0;
int last_rr_index = -1;

pthread_mutex_t scheduler_lock = PTHREAD_MUTEX_INITIALIZER;

void log_event(const char* type, task_t* t, const char* desc) {
    if (log_count >= MAX_LOGS) return;
    logs[log_count].tick = system_tick;
    strncpy(logs[log_count].type, type, 15);
    logs[log_count].task_id = t ? t->id : -1;
    strncpy(logs[log_count].task_name, t ? t->name : "SYSTEM", 31);
    strncpy(logs[log_count].description, desc, 63);
    logs[log_count].state = t ? t->state : INTERRUPTED;
    log_count++;
}

void rtos_delay(int ticks, task_t* t) {
    time_t now = time(NULL);
    t->wake_up_time = now + ticks;
    t->state = DELAYED;
    log_event("TASK", t, "Delayed");
}

void rtos_wait(task_t* t, int event_id) {
    t->waiting_for_event = event_id;
    t->state = WAITING_EVENT;
    log_event("TASK", t, "Waiting for event");
}

void rtos_enter_sleep(task_t* t) {
    t->state = SLEEP;
    log_event("TASK", t, "Entering SLEEP mode");
}

void* simulate_interrupt(void* arg) {
    sleep(5);
    event_triggered = 1;
    log_event("EVENT", NULL, "External Event Triggered");
    return NULL;
}

int allocate_stack(task_t* t, int size) {
    t->stack_used += size;
    if (t->stack_used > MAX_STACK) {
        log_event("FAULT", t, "Stack Overflow Detected");
        t->state = TERMINATED;
        return 0;
    }
    return 1;
}

void* task_wrapper(void* arg) {
    task_t* t = (task_t*)arg;
    while (t->state != TERMINATED) {
        pthread_mutex_lock(&t->lock);
        if (t->state == RUNNING) {
            log_event("TASK", t, "Started Execution");
            allocate_stack(t, 128); // Simulated stack use per run
            t->task_fn();
            if (t->state == RUNNING) t->state = READY;
        }
        pthread_mutex_unlock(&t->lock);
        usleep(100000);
    }
    return NULL;
}

void create_task(const char* name, void (*fn)(void), int priority) {
    if (task_count >= MAX_TASKS) return;
    task_t* t = &tasks[task_count];
    t->id = task_count;
    strncpy(t->name, name, 31);
    t->base_priority = priority;
    t->current_priority = priority;
    t->state = READY;
    t->task_fn = fn;
    t->wake_up_time = 0;
    t->waiting_for_event = -1;
    t->stack_used = 0;
    t->critical_section = 0;
    pthread_mutex_init(&t->lock, NULL);
    pthread_create(&t->thread, NULL, task_wrapper, t);
    task_count++;
}

int find_next_task() {
    int best_priority = -1;
    int selected_index = -1;
    time_t now = time(NULL);

    for (int i = 1; i <= task_count; ++i) {
        int idx = (last_rr_index + i) % task_count;
        pthread_mutex_lock(&tasks[idx].lock);

        if (tasks[idx].state == DELAYED && now >= tasks[idx].wake_up_time)
            tasks[idx].state = READY;
        if (tasks[idx].state == WAITING_EVENT && event_triggered && tasks[idx].waiting_for_event == EVENT_ID)
            tasks[idx].state = READY;
        if (tasks[idx].state == READY && tasks[idx].current_priority > best_priority) {
            selected_index = idx;
            best_priority = tasks[idx].current_priority;
        } else if (tasks[idx].state == READY && tasks[idx].current_priority == best_priority && selected_index == -1) {
            selected_index = idx;
        }

        pthread_mutex_unlock(&tasks[idx].lock);
    }
    return selected_index;
}

void* scheduler(void* arg) {
    while (1) {
        pthread_mutex_lock(&scheduler_lock);
        int idx = find_next_task();
        if (idx != -1) {
            task_t* t = &tasks[idx];
            pthread_mutex_lock(&t->lock);
            t->state = RUNNING;
            pthread_mutex_unlock(&t->lock);
            last_rr_index = idx;
        }
        pthread_mutex_unlock(&scheduler_lock);
        system_tick++;
        sleep(TICK_INTERVAL);
    }
    return NULL;
}

// Sample Task Definitions
void task_A() {
    printf("[Task A] Tick %d\n", system_tick);
    rtos_delay(2, &tasks[0]);
}

void task_B() {
    printf("[Task B] Tick %d\n", system_tick);
    rtos_wait(&tasks[1], EVENT_ID);
}

void task_C() {
    printf("[Task C] Tick %d\n", system_tick);
    if (system_tick % 4 == 0) rtos_enter_sleep(&tasks[2]);
    else rtos_delay(1, &tasks[2]);
}

// Export Logs
void export_csv() {
    FILE* f = fopen("task_log.csv", "w");
    fprintf(f, "Tick,Type,Task ID,Task Name,Description,State\n");
    for (int i = 0; i < log_count; ++i) {
        fprintf(f, "%d,%s,%d,%s,%s,%s\n",
            logs[i].tick, logs[i].type, logs[i].task_id, logs[i].task_name,
            logs[i].description, state_to_str(logs[i].state));
    }
    fclose(f);
}

void export_html() {
    FILE* f = fopen("task_log.html", "w");
    fprintf(f, "<html><head><title>RTOS Log</title></head><body>");
    fprintf(f, "<h2>RTOS Task & Event Timeline</h2><table border='1'>");
    fprintf(f, "<tr><th>Tick</th><th>Type</th><th>Task ID</th><th>Task Name</th><th>Description</th><th>State</th></tr>");
    for (int i = 0; i < log_count; ++i) {
        fprintf(f, "<tr><td>%d</td><td>%s</td><td>%d</td><td>%s</td><td>%s</td><td>%s</td></tr>",
            logs[i].tick, logs[i].type, logs[i].task_id, logs[i].task_name,
            logs[i].description, state_to_str(logs[i].state));
    }
    fprintf(f, "</table></body></html>");
    fclose(f);
}

int main() {
    printf("--- RTOS Simulator Starting ---\n");
    create_task("Task A", task_A, 2);
    create_task("Task B", task_B, 3);
    create_task("Task C", task_C, 2);

    pthread_t sched, intr;
    pthread_create(&sched, NULL, scheduler, NULL);
    pthread_create(&intr, NULL, simulate_interrupt, NULL);

    sleep(15);

    for (int i = 0; i < task_count; ++i) {
        pthread_mutex_lock(&tasks[i].lock);
        tasks[i].state = TERMINATED;
        pthread_mutex_unlock(&tasks[i].lock);
    }

    export_csv();
    export_html();
    printf("Simulation complete. Logs saved.\n");
    return 0;
}
