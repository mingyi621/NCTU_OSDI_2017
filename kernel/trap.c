#include <kernel/trap.h>
#include <inc/mmu.h>
#include <inc/x86.h>

#include <inc/stdio.h>

//#include <kernel/printf.c>
//#include <kernel/kbd.c>
//#include <kernel/trap_entry.S>

/* For debugging, so print_trapframe can distinguish between printing
 * a saved trapframe and printing the current trapframe and print some
 * additional information in the latter case.
 */
static struct Trapframe *last_tf;

/* TODO: You should declare an interrupt descriptor table.
 *       In x86, there are at most 256 it.
 *
 * Note: You can leverage the Gatedesc data structure inside mmu.h
 *       Interrupt descriptor table must be built at run time because shifted
 *       function addresses can't be represented in relocation records.
 */

//My Add
struct Gatedesc idt[256] = { { 0 } };
struct Pseudodesc idt_pd = {
	sizeof(idt) - 1, (uint32_t) idt
};


/* For debugging */
static const char *trapname(int trapno)
{
	static const char * const excnames[] = {
		"Divide error",
		"Debug",
		"Non-Maskable Interrupt",
		"Breakpoint",
		"Overflow",
		"BOUND Range Exceeded",
		"Invalid Opcode",
		"Device Not Available",
		"Double Fault",
		"Coprocessor Segment Overrun",
		"Invalid TSS",
		"Segment Not Present",
		"Stack Fault",
		"General Protection",
		"Page Fault",
		"(unknown trap)",
		"x87 FPU Floating-Point Error",
		"Alignment Check",
		"Machine-Check",
		"SIMD Floating-Point Exception"
	};

	if (trapno < sizeof(excnames)/sizeof(excnames[0]))
		return excnames[trapno];
	if (trapno == T_SYSCALL)
		return "System call";
	if (trapno >= IRQ_OFFSET && trapno < IRQ_OFFSET + 16)
		return "Hardware Interrupt";
	return "(unknown trap)";
}

/* For debugging */
void
print_trapframe(struct Trapframe *tf)
{
	cprintf("TRAP frame at %p \n");
	print_regs(&tf->tf_regs);
	cprintf("  es   0x----%04x\n", tf->tf_es);
	cprintf("  ds   0x----%04x\n", tf->tf_ds);
	cprintf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
	// If this trap was a page fault that just happened
	// (so %cr2 is meaningful), print the faulting linear address.
	if (tf == last_tf && tf->tf_trapno == T_PGFLT)
		cprintf("  cr2  0x%08x\n", rcr2());
	cprintf("  err  0x%08x", tf->tf_err);
	// For page faults, print decoded fault error code:
	// U/K=fault occurred in user/kernel mode
	// W/R=a write/read caused the fault
	// PR=a protection violation caused the fault (NP=page not present).
	if (tf->tf_trapno == T_PGFLT)
		cprintf(" [%s, %s, %s]\n",
			tf->tf_err & 4 ? "user" : "kernel",
			tf->tf_err & 2 ? "write" : "read",
			tf->tf_err & 1 ? "protection" : "not-present");
	else
		cprintf("\n");
	cprintf("  eip  0x%08x\n", tf->tf_eip);
	cprintf("  cs   0x----%04x\n", tf->tf_cs);
	cprintf("  flag 0x%08x\n", tf->tf_eflags);
	if ((tf->tf_cs & 3) != 0) {
		cprintf("  esp  0x%08x\n", tf->tf_esp);
		cprintf("  ss   0x----%04x\n", tf->tf_ss);
	}
}

/* For debugging */
void
print_regs(struct PushRegs *regs)
{
	cprintf("  edi  0x%08x\n", regs->reg_edi);
	cprintf("  esi  0x%08x\n", regs->reg_esi);
	cprintf("  ebp  0x%08x\n", regs->reg_ebp);
	cprintf("  oesp 0x%08x\n", regs->reg_oesp);
	cprintf("  ebx  0x%08x\n", regs->reg_ebx);
	cprintf("  edx  0x%08x\n", regs->reg_edx);
	cprintf("  ecx  0x%08x\n", regs->reg_ecx);
	cprintf("  eax  0x%08x\n", regs->reg_eax);
}

static void
trap_dispatch(struct Trapframe *tf)
{
  /* TODO: Handle specific interrupts.
   *       You need to check the interrupt number in order to tell
   *       which interrupt is currently happening since every interrupt
   *       comes to this function called by default_trap_handler.
   *
   * NOTE: Checkout the Trapframe data structure for interrupt number,
   *       which we had pushed into the stack when going through the
   *       declared interface in trap_entry.S
   *
   *       The interrupt number is defined in inc/trap.h
   *
   *       We prepared the keyboard handler and timer handler for you
   *       already. Please reference in kernel/kbd.c and kernel/timer.c
   */

	if (tf->tf_trapno == IRQ_OFFSET + IRQ_SPURIOUS) {
		cprintf("Spurious interrupt on irq 7\n");
		print_trapframe(tf);
		return;
	}

//My add 
//	extern void page_fault_handler();
	extern void timer_handler();
	extern void kbd_intr();
	switch(tf->tf_trapno){
/*		case T_BRKPT: {
			monitor(tf);
			break;
		}
		case T_PGFLT: {
			page_fault_handler(tf);
			break;
		}
*/		case T_PGFLT: {
			cprintf("[0556175] Page Fault @ 0x%8x\n",rcr2());
			while(1);
			break;
		}
	
		case IRQ_OFFSET + IRQ_TIMER: {
			timer_handler();
			break;
		}
		case IRQ_OFFSET + IRQ_KBD:{
			kbd_intr();
			break;
		}

/*		case IRQ_OFFSET + IRQ_SERIAL: {
			serial_intr();
			break;
		}
		case T_SYSCALL: {
			int32_t r = syscall(
				tf->tf_regs.reg_eax, 
				tf->tf_regs.reg_edx,
				tf->tf_regs.reg_ecx,
				tf->tf_regs.reg_ebx,
				tf->tf_regs.reg_edi,
				tf->tf_regs.reg_esi);
			if(tf->tf_regs.reg_eax == SYS_ipc_recv) {
				cprintf("return value is %d, %e\n", r, r);
				while(1);
			}
			tf->tf_regs.reg_eax = r;
			break;
		}*/
		default:{
			print_trapframe(tf);
//			if(tf->tf_cs == GD_KT)
//				panic("unhandled trap in kernel");
//			else {
//				env_destroy(curenv);
//				return;
//			}
		}
	}
//My add end	
	
	

	// Unexpected trap: The user process or the kernel has a bug.
//	print_trapframe(tf);
}

/* 
 * Note: This is the called for every interrupt.
 */
void default_trap_handler(struct Trapframe *tf)
{
	// Record that tf is the last real trapframe so
	// print_trapframe can print some additional information.
	last_tf = tf;

	// Dispatch based on what type of trap occurred
	trap_dispatch(tf);
}


void trap_init()
{
  /* TODO: You should initialize the interrupt descriptor table.
   *       You should setup at least keyboard interrupt and timer interrupt as
   *       the lab's requirement.
   *
   *       Noted that there is another file kernel/trap_entry.S, in which places
   *       all the entry of interrupt handler.
   *       Thus, you can declare an interface there by macro providing there and
   *       use that function pointer when setting up the corresponding IDT entry.
   *
   *       By doing so, we can have more flexibility in adding new IDT entry and 
   *       reuse the routine when interrupt occurs.
   *
   *       Remember to load you IDT with x86 assembly instruction lidt.
   *
   * Note:
   *       You might be benefitted from the macro SETGATE inside mmu.h      
   *       There are defined macros for Segment Selectors in mmu.h
   *       Also, check out inc/x86.h for easy-to-use x86 assembly instruction
   *       There is a data structure called Pseudodesc in mmu.h which might
   *       come in handy for you when filling up the argument of "lidt"
   */

	/* Keyboard interrupt setup */
	extern void kbd_entry();
	/* Timer Trap setup */
	extern void timer_entry();
	/* Page Fault Setup */
	extern void page_fault_entry();
  /* Load IDT */
	SETGATE(idt[14], 0, GD_KT, page_fault_entry, 0);
	SETGATE(idt[32], 0, GD_KT, timer_entry, 0);
	SETGATE(idt[33], 0, GD_KT, kbd_entry, 0);

	lidt(&idt_pd);
}
