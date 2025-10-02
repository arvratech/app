#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "sysprim.h"


/*
int main(int argc, char *argv[])
{
	char	temp[1024];

	ExecPipe("/bin/ps", "ps", "-ef", NULL);
	ExecPipeFinal(temp, "/bin/grep", "grep", "secu", NULL);
printf("...Start.........\n");
	printf("%s", temp);
printf("...End.........\n");
    return 0;
}
*/

int ExecForeground(const char *path, const char *arg, ...)
{
	va_list	ap;
    char	*array[64];
	int		i;

	//execl(path, arg, ap, NULL);
	va_start(ap, arg);
	array[0] = (char *)arg;
	if(array[0]) {
		i = 1;
		while(array[i++] = va_arg(ap, char *)) ;
	}
	va_end(ap);
	execv(path, array);
}

int ExecOnly(const char *path, const char *arg, ...)
{
	va_list	ap;
    char	*array[64];
	pid_t	pid;
	int		fd[2];
	int		i;

    pipe(fd);
	pid = fork();
    if(pid == 0) {
		close(fd[1]);
		dup2(fd[0], 0); // copy the read only pipe[0] to stdin(1) 
		close(fd[0]);
 		//execl(path, arg, ap, NULL);
		va_start(ap, arg);
		array[0] = (char *)arg;
		if(array[0]) {
			i = 1;
			while(array[i++] = va_arg(ap, char *)) ;
		}
		va_end(ap);
		execv(path, array);
	} else {
		close(fd[0]);
	//	rval = write(fd[1], p, 256);
		return fd[1];
	}
}

int ExecOnlyWithArray(const char *path, char **array)
{
	va_list	ap;
	pid_t	pid;
	int		fd[2];
	int		i;

    pipe(fd);
	pid = fork();
    if(pid == 0) {
		close(fd[1]);
		dup2(fd[0], 0); // copy the read only pipe[0] to stdin(1) 
		close(fd[0]);
		execv(path, array);
	} else {
		close(fd[0]);
	//	rval = write(fd[1], p, 256);
		return fd[1];
	}
}

int ExecPipe(const char *path, const char *arg, ...)
{
	va_list	ap;
    char	*array[64];
	pid_t	pid;
	int		fd[2];
	int		i, status;

    pipe(fd);
	pid = fork();
	if(pid == 0) {	// child
		close(fd[0]);
        dup2(fd[1], 1);	// copy the write only pipe[1] to stdout(1) 
		close(fd[1]);
 		//execl(path, arg, ap, NULL);
		va_start(ap, arg);
		array[0] = (char *)arg;
		if(array[0]) {
			i = 1;
			while(array[i++] = va_arg(ap, char *)) ;
		}
		va_end(ap);
		execv(path, array);
	} else {		// parent
		close(fd[1]);
		dup2(fd[0], 0); // copy the read only pipe[0] to stdin(1) 
		close(fd[0]);
		waitpid(pid, &status, 0);
		return status;
	}
}

int ExecPipeFinal(char *outputBuffer, const char *path, const char *arg, ...)
{
	va_list	ap;
    char	*p, *array[128];
	pid_t	pid;
	int		fd[2];
	int		i, rval, status;

    pipe(fd);
	pid = fork();
	if(pid == 0) {	// child
		close(fd[0]);
		dup2(fd[1], 1);	// copy the write only pipe[1] to stdout(1) 
		close(fd[1]);
		va_start(ap, arg);
		array[0] = (char *)arg;
		if(array[0]) {
			i = 1;
			while(array[i++] = va_arg(ap, char *)) ;
		}
		va_end(ap);
		execv(path, array);
	} else {		// parent
		close(fd[1]);
		waitpid(pid, &status, 0);
		p = outputBuffer;
		while(1) {
			rval = read(fd[0], p, 256);
			if(rval <= 0) break;
			p += rval;
		}
		*p = 0;
//printf("...start...\n%s...end...\n", outputBuffer);
		return status;
	}
}

/*
nt fd[2];

    pipe(fd);
    if (fork() == 0) {
        dup2(fd[1], 1);
        closepipes();
        execl("/bin/ps", "ps", "-ef", NULL);
    } else {
        dup2(fd[0], 0);
        closepipes();
        execl("/bin/grep", "grep", "root", NULL);
    }
    return (0);
}
*/ 

void print_id(char *comment)
{
	fprintf(stderr, "sid: %5d, pgid: %5d, pid: %5d, ppid: %5d  # %s\n",
			(int)getsid(0), (int)getpgid(0), (int)getpid(), (int)getppid(), comment);
}

static struct sigaction	sigactHup, sigactTerm, sigactInt;
static int	sig_bg;

void setExitSignal(int bg, void (*sigHandler)(int))
{
	struct sigaction	sigact;

	memset(&sigactHup, 0, sizeof(struct sigaction));
	memset(&sigactTerm, 0, sizeof(struct sigaction));
	memset(&sigactInt, 0, sizeof(struct sigaction));
	memset(&sigact, 0, sizeof(struct sigaction));
	sigact.sa_handler = sigHandler;
	sigaction(SIGHUP, &sigact, &sigactHup);
	sigaction(SIGTERM, &sigact, &sigactTerm);
	if(!bg) {
		memset(&sigact, 0, sizeof(struct sigaction));
		sigact.sa_handler = sigHandler;
		sigaction(SIGINT, &sigact, &sigactInt);
	}
	sig_bg = bg;
} 

void restoreExitSignal(void)
{
	sigactHup.sa_handler = SIG_IGN;
	sigaction(SIGHUP, &sigactHup, NULL);
	sigactTerm.sa_handler = SIG_IGN;
	sigaction(SIGTERM, &sigactTerm, NULL);
	if(!sig_bg) {
		sigactInt.sa_handler = SIG_IGN;
		sigaction(SIGINT, &sigactInt, NULL);
	}
} 

int initDaemon(void)
{
	pid_t	pid;
	int		n;

print_id("start");
	pid = fork();
	if(pid < 0) {
		fprintf(stderr, "fork() failed: %s\n", strerror(errno));
		return -1;
	} else if(pid > 0) {	// parent
		_exit(0);
	}
	// child
print_id("after fork()");
	n = setsid();
	if(n == -1) {
		fprintf(stderr, "setsid() failed: %s\n", strerror(errno));
		return -1;
	}
print_id("after setsid()");
	pid = fork();
	if(pid < 0) {
		fprintf(stderr, "fork() failed: %s\n", strerror(errno));
		return -1;
	} else if(pid > 0) {	// parent
		_exit(0);
	}
	// child
	chdir("/mnt/nand1-1");
	umask(0);
print_id("after fork() 2");
//	close(STDIN_FILENO); close(STDOUT_FILEN); close(STDERR_FILENO);
/*
	close(0); close(1); close(2);
	stdin = freopen("/dev/null", "r", stdin);
	stdout = freopen("/dev/null", "w", stdout);
	stderr = freopen("/dev/null", "w", stderr);
*/
	return 0;
}

static char usb_dir[16];
static char *tgt_dir = "/media/usb0";

int mountUsb(void)
{
	FILE	*fp;
	char	*p, usb[12], token[80], temp[512];
	int		rval;

	fp = fopen("/proc/partitions", "r");
	if(fp) {
		temp[0] = 0;
		while(1) {
			strcpy(token, temp);
			if(!fgets(temp, 100, fp)) break;
		}
		fclose(fp);
		p = token;
		p = read_token(p, usb);
		p = read_token(p, usb);
		p = read_token(p, usb);
		p = read_token(p, usb);
		if(usb[0] == 's' && usb[1] == 'd') {
			sprintf(usb_dir, "/dev/%s", usb);
printf("mount usb: [%s]\n", usb_dir);
			rval = ExecPipeFinal(temp, "/bin/mount", "mount", usb_dir, tgt_dir, NULL);
			if(rval) rval = -1;
		} else {
printf("mount usb: not insrted\n");
			rval = 1;	// USB not inserted
		}
	} else {
		rval = -1;
	}
	return rval;
}

void umountUsb(void)
{
	char	temp[512];

	ExecPipeFinal(temp, "/bin/umount", "umount", usb_dir, NULL);
}

void getKernelVersion(char *version)
{
	FILE	*fp;
	char	*s, *d, *p, *p1, temp[256], token[80], tm[8];
	int		rval, yyyy, mm, dd;	

// Linux version 2.6.35.4 (root@localhost) (gcc version 4.2.1) #976 PREEMPT Thu Aug 8 10:47:03 KST 2019
// Linux version 3.4.103-00033-g9a1cd034181a-dirty (root@localhost.localdomain) (gcc version 4.9.4 (Linaro GCC 4.9-2017.01) ) #460 SMP PREEMPT Tue Feb 4 11:50:54 KST 2020
// Linux version 3.4.104-2.2-00033-g9a1cd034181a-dirty (root@localhost) (gcc version 4.9.4 (Linaro GCC 4.9-2017.01) ) #1097 SMP PREEMPT Thu Apr 28 16:51:00 KST 2022
	fp = fopen("/proc/version", "r");
	if(fp) {
		rval = fread(temp, 1, 256, fp);
		temp[rval] = 0;
		fclose(fp);
	} else {
		rval = -1;
	}
	if(rval > 20) {
		s = temp; d = version;
		s = read_token_ch(s, d, '(');
		str_chop(d);
		p = strchr(d, '-');
		if(p) {
			p1 = strchr(p, '.');
			if(p1) {
				p1 = strchr(p+1, '-');
				*p1 = 0;
			} else	*p = 0;
		}
		d += strlen(d); 	// Linux version 2.6.35.43
		while(*s) {
			s = read_token(s, token);
			if(!strcmp(token, "PREEMPT")) break;
		}
		/*
		s = read_token(s, token);						// skip week
		s = read_token(s, token);						// Aug
		mm = dtShortMonthString2Month(token);
		s = read_token(s, token); dd = n_atoi(token);	// 8
		s = read_token(s, tm); tm[5] = 0;				// 10:47:03
		s = read_token(s, token);						// KST
		s = read_token(s, token); yyyy = n_atoi(token);	// 2019
		sprintf(d, " %04d/%02d/%02d %s", yyyy, mm, dd, tm);
printf("2[%s]\n", version);
		*/
	} else {
		strcpy(version, "Unknown");
	}
}

void getKernelLocalVersion(char *version)
{
	char	*p, temp[80];

	getKernelVersion(temp);
	p = strchr(temp, '-');
	if(p) strcpy(version, p+1);
	else  version[0] = 0;
}

