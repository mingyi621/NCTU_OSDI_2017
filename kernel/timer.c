/* Reference: http://www.osdever.net/bkerndev/Docs/pit.htm */
#include <kernel/trap.h>
#include <kernel/picirq.h>
#include <kernel/task.h>
#include <kernel/cpu.h>
#include <inc/mmu.h>
#include <inc/x86.h>

#define TIME_HZ 100

static unsigned long jiffies = 0;

void set_timer(int hz)
{
  int divisor = 1193180 / hz;       /* Calculate our divisor */
  outb(0x43, 0x36);             /* Set our command byte 0x36 */
  outb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
  outb(0x40, divisor >> 8);     /* Set high byte of divisor */
}

/* It is timer interrupt handler */
//
// TODO: Lab6
// Modify your timer_handler to support Multi processor
// Don't forget to acknowledge the interrupt using lapic_eoi()
//
void timer_handler(struct Trapframe *tf)
{
  	extern void sched_yield();
  	int i;
//lab6
	extern struct CpuInfo cpus[NCPU];
	int f = cpunum();
	
  	jiffies++;

	lapic_eoi();
  	extern Task tasks[];

//  	extern Task *cur_task;

//  	if (cur_task != NULL)
//  	{
  	/* TODO: Lab 5
   	 * 1. Maintain the status of slept tasks
   	 * 
   	 * 2. Change the state of the task if needed
   	 *
   	 * 3. Maintain the time quantum of the current task
   	 *
   	 * 4. sched_yield() if the time is up for current task
   	 *
   	 */
	/*	for(i = 0; i<NR_TASKS; i++){
			if(tasks[i].state == TASK_SLEEP)
				tasks[i].remind_ticks--;
			if(cur_task->remind_ticks-- <= 0)
				sched_yield();
		}
	*/
/*lab5	for(i=0;i<NR_TASKS;i++){
		switch(tasks[i].state){
			case TASK_SLEEP:
				tasks[i].remind_ticks--;
				if(tasks[i].remind_ticks <= 0){
					tasks[i].state = TASK_RUNNABLE;
					tasks[i].remind_ticks = TIME_QUANT;
				}
				break;
			case TASK_RUNNING:
				tasks[i].remind_ticks--;
				break;
		}
	}
	if(cur_task->remind_ticks<=0)
		sched_yield();
  	}
*/
//lab6
	for(i=0;i<NR_TASKS;i++)
	{
		if(cpus[f].cpu_rq.task_rq[i]!=NULL)
		{
		switch( cpus[f].cpu_rq.task_rq[i]->state)
			{
			case TASK_SLEEP:
				cpus[f].cpu_rq.task_rq[i]->remind_ticks--;
				if(cpus[f].cpu_rq.task_rq[i]->remind_ticks<=0)
				{
					cpus[f].cpu_rq.task_rq[i]->state = TASK_RUNNABLE;
					cpus[f].cpu_rq.task_rq[i]->remind_ticks = TIME_QUANT;
				}
				break;
			case TASK_RUNNING:
				cpus[f].cpu_rq.task_rq[i]->remind_ticks --;
				break;
			}
		}
	}
	if(cpus[f].cpu_task->remind_ticks<=0)
	{
		sched_yield();
	}
}

unsigned long sys_get_ticks()
{
  return jiffies;
}
void timer_init()
{
  set_timer(TIME_HZ);

  /* Enable interrupt */
  irq_setmask_8259A(irq_mask_8259A & ~(1<<IRQ_TIMER));

  /* Register trap handler */
  extern void TIM_ISR();
  register_handler( IRQ_OFFSET + IRQ_TIMER, &timer_handler, &TIM_ISR, 0, 0);
}

