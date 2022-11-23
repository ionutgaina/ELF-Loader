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

#define PAGESIZE getpagesize() // PAGESIZE is 4096
static so_exec_t *exec;
static int fd;

static void segv_handler(int signum, siginfo_t *info, void *context)
{

	so_seg_t *seg_err = NULL;
	uintptr_t page_offset, seg_vaddr;
	void *page_address;

	for (int i = 0; i < exec->segments_no; i++) {
		seg_err = &exec->segments[i];
		seg_vaddr = seg_err->vaddr;

		int seg_err_offset = seg_vaddr - (uintptr_t)info->si_addr;

		if (seg_err_offset <= 0 && (seg_err_offset + (int)seg_err->mem_size >= 0))
			break;
	}

	if (!seg_err)
		exit(139);

	page_address = (void *)((int)info->si_addr - ((int)info->si_addr % PAGESIZE));
	page_offset = (uintptr_t)page_address - seg_vaddr;

	if (info->si_code != SEGV_MAPERR)
		exit(139);

	void *ptr = mmap(page_address, PAGESIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_FIXED | MAP_PRIVATE, -1, 0);

	lseek(fd, page_offset + seg_err->offset, SEEK_SET);
	if (page_offset + PAGESIZE <= seg_err->file_size)
		read(fd, page_address, PAGESIZE);

	else if (page_offset <= seg_err->file_size)
		read(fd, page_address, seg_err->file_size - page_offset);

	mprotect(ptr, PAGESIZE, (int)seg_err->perm);
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
	fd = open(path, O_RDONLY);

	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	so_start_exec(exec, argv);

	return -1;
}
