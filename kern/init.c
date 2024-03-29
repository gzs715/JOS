/* See COPYRIGHT for copyright information. */

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/monitor.h>
#include <kern/console.h>
#include <kern/pmap.h>
#include <kern/kclock.h>
#include <kern/env.h>
#include <kern/trap.h>
#include <kern/sched.h>
#include <kern/picirq.h>


void
i386_init(uint32_t memsize)
{
	extern char etext[],edata[], end[];

	// Before doing anything else, complete the ELF loading process.
	// Clear the uninitialized global data (BSS) section of our program.
	// This ensures that all static/global variables start out zero.
	
	/* 
	 * From http://en.wikipedia.org/wiki/.bss
	 * In computer programming
	 * .bss or bss (Block Started by Symbol) is used by many compilers and linkers 
	 * as the name of the data segment containing static variables 
	 * that are filled solely with zero-valued data initially 
	 * (i. e., when execution begins). 
	 * It is often referred to as the "bss section" or "bss segment". 
	 * The program loader initializes the memory allocated for the bss section 
	 * when it loads the program.
	 */
	
	memset(edata, 0, end - edata);

	// Initialize the console.
	// Can't call cprintf until after we do this!
	cons_init();


	cprintf("6828 decimal is %o octal!\n", 6828);
	cprintf("etext:%08x,edata:%08x,end:%08x\n",etext,edata,end);

	// Lab 2 memory management initialization functions
	i386_detect_memory(memsize);
	i386_vm_init();

	// Lab 3 user environment initialization functions
	env_init();
	idt_init();

	// Lab 4 multitasking initialization functions
	pic_init();
	kclock_init();
	// Should always have an idle process as first one.
	ENV_CREATE(user_idle);

	// Start fs.
	ENV_CREATE(fs_fs);
	// Start init
#if defined(TEST)
	// Don't touch -- used by grading script!
	ENV_CREATE2(TEST, TESTSIZE);
#else
	// Touch all you want.
	ENV_CREATE(user_testfsipc);
	//ENV_CREATE(user_pipereadeof);
	// ENV_CREATE(user_pipewriteeof);
#endif

	// Should not be necessary - drain keyboard because interrupt has given up.
	kbd_intr();

	//while(login() != 0)
	//	continue;
	// Schedule and run the first user environment!
	sched_yield();


}


/*
 * Variable panicstr contains argument to first call to panic; used as flag
 * to indicate that the kernel has already called panic.
 */
static const char *panicstr;

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then enters the kernel monitor.
 */
void
_panic(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	if (panicstr)
		goto dead;
	panicstr = fmt;

	va_start(ap, fmt);
	cprintf("kernel panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

dead:
	/* break into the kernel monitor */
	while (1)
		monitor(NULL);
}

/* like panic, but don't */
void
_warn(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	va_start(ap, fmt);
	cprintf("kernel warning at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);
}
