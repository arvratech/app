#ifndef _SHM_H_
#define _SHM_H_


typedef struct _SHM {
	char	name[64];
	int		fd;
	int		size;
	void	*buf;
} SHM;


int  shmCreate(SHM *shm);
void shmDelete(SHM *shm);
int  shmOpen(SHM *shm);
void shmClose(SHM *shm);


#endif


