#include <kernel/task.h>
#include <inc/x86.h>

#define ctx_switch(ts) \
  do { env_pop_tf(&((ts)->tf)); } while(0)

/* TODO: Lab5
* Implement a simple round-robin scheduler (Start with the next one)
*
* 1. You have to remember the task you picked last time.
*
* 2. If the next task is in TASK_RUNNABLE state, choose
*    it.
*
* 3. After your choice set cur_task to the picked task
*    and set its state, remind_ticks, and change page
*    directory to its pgdir.
*
* 4. CONTEXT SWITCH, leverage the macro ctx_switch(ts)
*    Please make sure you understand the mechanism.
*/
void sched_yield(void)
{
	extern Task tasks[];
	extern Task *cur_task;
	int i;
	int j=0;
	i = (cur_task->task_id+1)%NR_TASKS;
	for(j=0;j<NR_TASKS;j++){
		if(tasks[i].state == TASK_RUNNABLE)
		{
			if(cur_task->state == TASK_RUNNING  )
			{
				cur_task->state = TASK_RUNNABLE;
				cur_task->remind_ticks = TIME_QUANT;
				cur_task = &tasks[i];
				lcr3( PADDR( tasks[i].pgdir ) );
				tasks[i].state = TASK_RUNNING;
				break;
			}
			
			if(cur_task->state == TASK_SLEEP)
			{
				cur_task = &tasks[i];		
				lcr3( PADDR( tasks[i].pgdir ) );
				tasks[i].state = TASK_RUNNING;
				break;
			}
			
			if(cur_task->state == TASK_STOP)
			{
				cur_task = &tasks[i];
				lcr3( PADDR( tasks[i].pgdir ) );
                                tasks[i].state = TASK_RUNNING;
                                break;
			}
			if(cur_task->state == TASK_FREE)
			{
				cur_task = &tasks[i];
				lcr3( PADDR( tasks[i].pgdir ) );
                                tasks[i].state = TASK_RUNNING;
                                break;
				
			}
		}
		else if(tasks[i].state == TASK_RUNNING)
		{
			cur_task = &tasks[i];
				lcr3( PADDR( tasks[i].pgdir ) );
			tasks[i].remind_ticks = TIME_QUANT;	
		}
		i = (i+1)%NR_TASKS;


	}
	env_pop_tf(&cur_task->tf);
}
