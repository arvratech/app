#ifndef _CONSOLE_H_
#define _CONSOLE_H_

void SetConsoleSioPort(int Port);
void SetConsoleSinPort(int Port);
int  s_getc(void);
void s_putc(int c);
void s_puts(char *s);
int  s_read(void *buf, int size);
void s_write(void *buf, int size);
int  t_getc(void);
void t_putc(int c);
void t_puts(char *s);
int  t_read(void *buf, int size);
void t_write(void *buf, int size);
int  u_getc(void);
void u_putc(int c);
void u_puts(char *s);
int  u_read(void *buf, int size);
void u_write(void *buf, int size);


#endif	/* _CONSOLE_H_ */
