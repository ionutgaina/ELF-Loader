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


	// printf("base_addr: %d\n", exec->base_addr);
	// printf("entry - %d\n", exec->entry);
	// printf("segments_no - %d\n", exec->segments_no);
	printf("address fault %d\n", (uintptr_t)info->si_addr);
	// printf("signum - %d\n", signum);
	// printf("sig code - %d\n", info->si_code);
	// printf("context - %p\n", context);

	// int segment_no = -1;
	so_seg_t *seg_err = NULL;
	uintptr_t seg_err_offset;
	uintptr_t seg_vaddr;
	void* page_address = (void*)  ( (int)info->si_addr - ( (int)info->si_addr % PAGESIZE ));
	
	for (int i = 0; i < exec->segments_no; i++)
	{
		// printf("segment no %d with addr %d \n", i , exec->segments[i].vaddr);
		seg_err = &exec->segments[i];
		seg_vaddr = seg_err->vaddr;
		
		int seg_err_offset2 = seg_vaddr - (uintptr_t)info->si_addr;
		// printf("offset %d \n\n", seg_err_offset2);
		if (seg_err_offset2 <= 0 && (seg_err_offset2 + (int)seg_err->mem_size >= 0))
		{
			// printf("%d  \n", seg_err_offset);
			// segment_no = i;
			break;
		}
	}
	seg_err_offset = (uintptr_t)page_address - seg_vaddr;
	
	if (!seg_err)
	{
		// printf("Segment doesn't exist");
		exit(139);
	}
	// printf("segment start  %d\n", (int)seg_err->vaddr);
	// printf("segment number - %d \n", segment_no);
	// printf("segment no %d\n", segment_no);
	// not mapped
	// printf("sig code %d\n",info->si_code);
	if (info->si_code == SEGV_MAPERR)
	{
		void *ptr = mmap(page_address, PAGESIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_FIXED | MAP_PRIVATE, -1, 0);
		// TODO copy data in my mapped page

		// printf("pointer mmap  %p\n", ptr);
		// printf("addres %d\n", page_address);
		// printf("file size %d\n", seg_err->file_size);
		// printf("mem size %d\n", seg_err->mem_size);
		lseek(fd, seg_err_offset + seg_err->offset, SEEK_SET);
		if (seg_err_offset + PAGESIZE <= seg_err->file_size)
		{
			// read a page from file to fault address
			// printf("aici1\n");
			read(fd, page_address , PAGESIZE);
		}
		else if (seg_err_offset <= seg_err->file_size)
		{	
			// printf("aici2\n");
			read(fd, page_address, seg_err->file_size - seg_err_offset);

		// printf("protect %d \n", seg_err->perm);
		mprotect(ptr, PAGESIZE, (int)seg_err->perm);
	}
	// mapped
	else
	{
	exit(139);
	}

}

int so_init_loader(void)
{
	int rc;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = segv_handler;
	sa.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGSEGV, &sa, NULL);
	if (rc < 0)
	{
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