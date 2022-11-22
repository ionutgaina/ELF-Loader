/*
 * Loader Implementation
 *
 * 2022, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>

#include "exec_parser.h"

#define PAGESIZE getpagesize() //PAGESIZE is 4096
static so_exec_t *exec;

static void segv_handler(int signum, siginfo_t *info, void *context)
{

	int segment_no = -1;
	so_seg_t *seg_err = NULL;

	// printf("base_addr: %d\n", exec->base_addr);
	// printf("entry - %d\n", exec->entry);
	printf("segments_no - %d\n", exec->segments_no);
	printf("address fault - %d\n", (uintptr_t)info->si_addr);
	// printf("signum - %d\n", signum);
	// printf("context - %p\n", context);

	uintptr_t fault_addr = (uintptr_t)info->si_addr;
	for( int i = 0; i < exec->segments_no ; i++ ) {
		// printf("segment no %d with addr %d \n", i , exec->segments[i].vaddr);
		uintptr_t seg_vaddr = exec->segments[i].vaddr;
		if ( (int)(seg_vaddr - fault_addr) <= 0){
			seg_err = &exec->segments[i];
			segment_no = i;
			break;
		}
	}

	if (seg_err == NULL) {
		printf("Don't exist segment");
		exit(0);
	}

	printf("segment number - %d \n", segment_no);
	printf("segment start - %p\n", (void*)seg_err->vaddr);

	exit(0);
	/* TODO - actual loader implementation */
}

int so_init_loader(void)
{
	int rc;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = segv_handler;
	sa.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGSEGV, &sa, NULL);
	if (rc < 0) {
		perror("sigaction");
		return -1;
	}
	return 0;
}

int so_execute(char *path, char *argv[])
{
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	so_start_exec(exec, argv);

	return -1;
}