#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include "shm.h"


int shmCreate(SHM *shm)
{
	int		fd, rval;

	fd = shm_open(shm->name, O_RDWR|O_CREAT, 0666);
	if(fd < 0) {
		printf("shmCreate: shm_open() error: %s\n", strerror(errno));
		return -1;
	}
	rval = ftruncate(fd, shm->size);
	if(rval < 0) {
		printf("shmCreate: ftruncate() error: %s\n", strerror(errno));
		close(fd);
		shm_unlink(shm->name);
		return -1;
	}
	shm->buf = mmap(NULL, shm->size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(shm->buf == MAP_FAILED) {
		printf("shmCreate: mmap() error: %s\n", strerror(errno));
		close(fd);
		shm_unlink(shm->name);
		return -1;
	}
	shm->fd = fd;
	return 0;
}

void shmDelete(SHM *shm)
{
	munmap(shm->buf, shm->size);
	close(shm->fd);
	shm_unlink(shm->name);
}

int shmOpen(SHM *shm)
{
	int		fd, rval;

	fd = shm_open(shm->name, O_RDWR, 0);
	if(fd < 0) {
		printf("shmOpen: shm_open() error: %s\n", strerror(errno));
		return -1;
	}
	shm->buf = mmap(NULL, shm->size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(shm->buf == MAP_FAILED) {
		printf("shmOpen: mmap() error: %s\n", strerror(errno));
		close(fd);
		return -1;
	}
	shm->fd = fd;
	return 0;
}

void shmClose(SHM *shm)
{
	munmap(shm->buf, shm->size);
	close(shm->fd);
}

int main(void)
{
	SHM		*shm, _shm;
	int		rval;
	
	shm = &_shm;
	shm->size = 4096;
	strcpy(shm->name, "/shmtst");
	rval = shmCreate(shm);
	printf("shmCreate() = %d\n", rval);	
	if(rval < 0) return -1;
printf("addr=%p\n", shm->buf);
	memset(shm->buf, '1', 10);
	sleep(30);
	shmDelete(shm);
	printf("shmDelete()\n");
	return 0;
}

/*
int main(void)
{
	SHM		*shm, _shm;
	unsigned char	*p;
	int		i, rval;
	
	shm = &_shm;
	shm->size = 4096;
	strcpy(shm->name, "/shmtst");
	rval = shmOpen(shm);
	printf("shmOpen() = %d\n", rval);	
	if(rval < 0) return -1;
printf("addr=%p\n", shm->buf);
	p = shm->buf;
	for(i = 0;i < 16;i++) printf("%02x-", p[i]);
	printf("\n");
	sleep(30);
	shmClose(shm);
	printf("shmClose()\n");
	return 0;
}
*/

