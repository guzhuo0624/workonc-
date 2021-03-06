/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * triplehugemod.c
 *
 * 	Runs three copies of huge.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include "triplehugemod.h"

#define PageSize	4096
#define NumPages	512

int sparse[NumPages][PageSize];	/* use only the first element in the row */

int
huge()
{
	int i,j;

	printf("Entering the huge program - I will stress test your VM\n");

	/* move number in so that sparse[i][0]=i */
	for (i=0; i<NumPages; i++) {
		sparse[i][0]=i;
	}
	
	printf("stage [1] done\n");
	
	/* increment each location 5 times */
	for (j=0; j<5; j++) {
		for (i=0; i<NumPages; i++) {
			sparse[i][0]++;
		}
		printf("stage [2.%d] done\n", j);
	}
	
	printf("stage [2] done\n");
	
	/* check if the numbers are sane */
	for (i=NumPages-1; i>=0; i--) {
		if (sparse[i][0]!=i+5) {
			printf("BAD NEWS!!! - your VM mechanism has a bug!\n");
			exit(1);
		}
	}
	
	printf("You passed!\n");
	
	return 0;
}

static
pid_t
spawnv()
{
	pid_t pid = fork();
	switch (pid) {
	    case -1:
		err(1, "fork");
	    case 0:
		/* child */
	    huge();

	    default:
		/* parent */
		break;
	}
	return pid;
}

static
int
dowait(int index, int pid)
{
	int status;

	if (waitpid(pid, &status, 0)<0) {
		warn("waitpid for copy #%d (pid %d)", index, pid);
		return 1;
	}
	else if (WIFSIGNALED(status)) {
		warnx("copy #%d (pid %d): signal %d", index, pid,
		      WTERMSIG(status));
		return 1;
	}
	else if (WEXITSTATUS(status) != 0) {
		warnx("copy #%d (pid %d): exit %d", index, pid,
		      WEXITSTATUS(status));
		return 1;
	}
	return 0;
}

int
main()
{
	pid_t pids[3];
	int i, failures = 0;

	warnx("Starting: running three copies of huge");

	for (i=0; i<3; i++) {
		pids[i]=spawnv();
	}

	for (i=0; i<3; i++) {
		failures += dowait(i, pids[i]);
	}

	if (failures > 0) {
		warnx("%d failures", failures);
	}
	else {
		warnx("Congratulations! You passed.");
	}
	return 0;
}

