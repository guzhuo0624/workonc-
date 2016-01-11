/*
 * Process-related syscalls.
 * New for ASST1.
 */

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <current.h>
#include <pid.h>
#include <machine/trapframe.h>
#include <syscall.h>
#include <kern/wait.h>

/*
 * sys_fork
 * 
 * create a new process, which begins executing in md_forkentry().
 */


int
sys_fork(struct trapframe *tf, pid_t *retval)
{
	struct trapframe *ntf; /* new trapframe, copy of tf */
	int result;

	/*
	 * Copy the trapframe to the heap, because we might return to
	 * userlevel and make another syscall (changing the trapframe)
	 * before the child runs. The child will free the copy.
	 */

	ntf = kmalloc(sizeof(struct trapframe));
	if (ntf==NULL) {
		return ENOMEM;
	}
	*ntf = *tf; /* copy the trapframe */

	result = thread_fork(curthread->t_name, enter_forked_process, 
			     ntf, 0, retval);
	if (result) {
		kfree(ntf);
		return result;
	}

	return 0;
}

pid_t
sys_getpid(pid_t *retval)
{
	*retval = curthread->t_pid;
	return 0;
}


pid_t
sys_waitpid(pid_t pid, int *status, int options, pid_t *retval) 
{
	int result;

	if (options != 0 && options != WNOHANG) {
		return -EINVAL;
	}

	result = pid_is_parent(curthread->t_pid, pid);

	if (result < 0) {
		return result;
	}

	result = pid_join(pid, status, options);
	*retval = result;


	if (result < 0) {
		return result;
	}

	return 0; 
}


/*
 * sys_kill
 *
 * The kill system call is used to send a signal (specified by sig) to
 * any process (specified by pid).
 */
int sys_kill(pid_t pid, int sig, int *retval) {

	int result = pid_set_sig_flag(pid, sig);
	*retval = result;
	
	if (result != 0) {
		return -1; 
	}	

	return 0;
}

