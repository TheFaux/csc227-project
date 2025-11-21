#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "queue.c"


#define MAX_PROCS 30
#define MAX_MEM 2048
#define CONTEXT_SWITCH_TIME 0
#define QUANTUM 6

typedef struct{
	int id;
	int burst;
	int priority;
	int memory;
	
	enum {waiting, ready, running} state;
	int turnaround_time;
	int wait_time;
} PCB;

PCB procs[MAX_PROCS];
int count = 0;
int bursts_i[MAX_PROCS];


void *read_file(void*);
void add_burst(int, int);
void *SJF(void*);
void *RR(void*);


int main() {
	pthread_t reader_id;
	pthread_create(&reader_id, NULL, read_file, "jobs");
	pthread_join(reader_id, NULL);
	//read_file("jobs");
    //for(int i = 0; i < count; i++) printf("%d\n", procs[i].turnaround_time);
    //for(int i = 0; i < count; i++) printf("%d\n", bursts_i[i]);
    //SJF();
	/*
    pthread_t SJF_id;
    pthread_create(&SJF_id, NULL, SJF, NULL);
    pthread_join(SJF_id, NULL);
    */
    
	
    pthread_t RR_id;
    pthread_create(&RR_id, NULL, RR, NULL);
    pthread_join(RR_id, NULL);
	
    
    return 0;
}

void *RR(void* a){
	
	for(int p=0; p < count; p++){
		if(procs[p].memory > MAX_MEM){
			fprintf(stderr, "Not enough memory for running process (%d)", procs[p].id);
			return NULL;
		}
	}
	
	
	int n_finish = 0;//number of finished processes (useful for simple queue implementation)
	CQueue job_q = cq_new();//indices of the processes waiting to be loaded into memory
	for(int i = 0; i < count; i++) cq_enqueue(&job_q, i);
	int og_bursts[count];
	for(int i = 0; i< count; i++) og_bursts[procs[i].id] = procs[i].burst;
	
	int time = 0;
	
	while(1){
		PCB* proc = &procs[cq_peek(&job_q)];
		
		//run process for  = cq_create(count)quantum
		printf("Process (%d) started at: %d\n", proc->id, time);
		printf("Running");
		for(int t=1; t<=QUANTUM && proc->burst!=0; t++){
			printf(".");
			proc->burst--;
			time++;
		}
		if(proc->burst==0){
			cq_dequeue(&job_q);
			n_finish++;
			puts("");
			printf("Process (%d) done at: %d\n", proc->id, time);
			proc->turnaround_time = time - 0;//arrive time is always 0
			proc->wait_time = proc->turnaround_time - og_bursts[proc->id];
	
			printf("Turnaround Time: %d\n", proc->turnaround_time);
			printf("Waiting Time: %d\n", proc->wait_time);
			puts("---");	
		}
		else{
			//requeue process
			puts("");
			printf("Process (%d) swapped out at: %d\n", proc->id, time);
			//printf("Remaining: %ds\n", proc->burst);
			puts("---");

			cq_enqueue(&job_q, cq_dequeue(&job_q));
		}
		
		if(cq_is_empty(&job_q)) break;
	}
	
	pthread_exit(0);
}

void *SJF(void* a){
	
	for(int p=0; p < count; p++){
		if(procs[p].memory > MAX_MEM){
			fprintf(stderr, "Not enough memory for running process (%d)", procs[p].id);
			return NULL;
		}
	}
	
	int mem = MAX_MEM;
	int job_q[count];//indices of the processes waiting to be loaded into memory
	int ready_q[count];//indices of the processes in the ready queue in memory
	
	memcpy(job_q, bursts_i, count*sizeof(int));//put processes into the job queue
	
	int time = 0;
	//TODO: bounds checking
	int i = 0;//will determine start of job_q
	int r = 0;//# of items in ready_q
	int r_i = 0;//start of ready_q
	long_term:
	for(; i < count; i++){
		PCB proc = procs[job_q[i]];
		if(mem - proc.memory < 0) goto short_term;
		else{
			mem -= proc.memory;
			ready_q[r++] = job_q[i];
		}
	}
	short_term:
	for(; r_i < r; r_i++){
		PCB proc = procs[ready_q[r_i]];//get process burst time
		int burst = proc.burst;
		
		printf("Process (%d) started at: %d\n", proc.id, time);
		printf("Running");
		while(burst != 0){//run process
			printf(".");
			burst--;
			time++;
		}
		puts("");
		printf("Process (%d) done at: %d\n", proc.id, time);
		
		proc.turnaround_time = time - 0;//arrive time is always 0
		proc.wait_time = proc.turnaround_time - proc.burst;
		
		printf("Turnaround Time: %d\n", proc.turnaround_time);
		printf("Waiting Time: %d\n", proc.wait_time);
		puts("---");
		
		mem+= proc.memory;//free memory
	}
	if (i!=count) goto long_term;//go back to job queue if not done
	
	pthread_exit(0);
}

void *read_file(void* filename) {
    FILE *file = fopen((char*) filename, "r");
    if (!file) {
        perror("Error opening file");
    }

    int id, burst, priority, memory;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d:%d:%d;%d", &id, &burst, &priority, &memory) == 4) {
            //printf("%d	%d	%d	%d\n", id, burst, priority, memory);
            procs[count] = (PCB) {id, burst, priority, memory, waiting, -1, -1};
            add_burst(count, burst);
            count++;
        }
    }

    fclose(file);
    pthread_exit(0);
}

void add_burst(int index, int burst) {
    
    int i = 0;
    while (i <= index && procs[bursts_i[i]].burst < burst) {
        i++;
    }

    for (int j = index+1; j > i; j--) {
        bursts_i[j] = bursts_i[j - 1];
    }

    bursts_i[i] = index;
}

