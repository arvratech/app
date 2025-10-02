/*
 * GyS-TermIO v2.0 (for GySmail v3)
 * a very small replacement of ncurses library
 *
 * copyright (C) 1999 A'rpi/ESP-team
 *
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"

//#define HAVE_TERMCAP
#if !defined(__OS2__) && !defined(__MORPHOS__)
#define CONFIG_IOCTL
#endif

#define MAX_KEYS 64
#define BUF_LEN 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef CONFIG_IOCTL
#include <sys/ioctl.h>
#endif

#ifdef HAVE_TERMIOS
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#ifdef HAVE_SYS_TERMIOS_H
#include <sys/termios.h>
#endif
#endif

#if defined(HAVE_LANGINFO) && defined(CONFIG_ICONV)
#include <locale.h>
#include <langinfo.h>
#endif

#include <unistd.h>

#include "keycodes.h"
#include "getch2.h"

#ifdef HAVE_TERMIOS
static struct termios tio_orig;
#endif
static int getch2_len=0;
static char getch2_buf[BUF_LEN];

int screen_width=80;
int screen_height=24;
char * erase_to_end_of_line = NULL;

typedef struct {
  int len;
  int code;
  char chars[8];
} keycode_st;
static keycode_st getch2_keys[MAX_KEYS];
static int getch2_key_db=0;

#ifdef HAVE_TERMCAP

#if 0
#include <termcap.h>
#else
int tgetent(char *BUFFER, char *TERMTYPE);
int tgetnum(char *NAME);
int tgetflag(char *NAME);
char *tgetstr(char *NAME, char **AREA);
#endif

static char term_buffer[4096];
static char term_buffer2[4096];
static char *term_p=term_buffer2;

static void termcap_add(char *id,int code){
char *p=tgetstr(id,&term_p);
  if(!p) return;
  if(getch2_key_db>=MAX_KEYS) return;
  getch2_keys[getch2_key_db].len=strlen(p);
  strncpy(getch2_keys[getch2_key_db].chars,p,8);
  getch2_keys[getch2_key_db].code=code;
  ++getch2_key_db;
/*  printf("%s=%s\n",id,p); */
}

static int success=0;

int load_termcap(char *termtype){
  if(!termtype) termtype=getenv("TERM");
  if(!termtype) termtype="unknown";
  success=tgetent(term_buffer, termtype);
  if(success<0){ printf("Could not access the 'termcap' data base.\n"); return 0; }
  if(success==0){ printf("Terminal type `%s' is not defined.\n", termtype);return 0;}

  screen_width=tgetnum("co");
  screen_height=tgetnum("li");
  if(screen_width<1 || screen_width>255) screen_width=80;
  if(screen_height<1 || screen_height>255) screen_height=24;
  erase_to_end_of_line= tgetstr("cd",&term_p);

  termcap_add("kP",KEY_PGUP);
  termcap_add("kN",KEY_PGDWN);
  termcap_add("kh",KEY_HOME);
  termcap_add("kH",KEY_END);
  termcap_add("kI",KEY_INS);
  termcap_add("kD",KEY_DEL);
  termcap_add("kb",KEY_BS);
  termcap_add("kl",KEY_LEFT);
  termcap_add("kd",KEY_DOWN);
  termcap_add("ku",KEY_UP);
  termcap_add("kr",KEY_RIGHT);
  termcap_add("k0",KEY_F+0);
  termcap_add("k1",KEY_F+1);
  termcap_add("k2",KEY_F+2);
  termcap_add("k3",KEY_F+3);
  termcap_add("k4",KEY_F+4);
  termcap_add("k5",KEY_F+5);
  termcap_add("k6",KEY_F+6);
  termcap_add("k7",KEY_F+7);
  termcap_add("k8",KEY_F+8);
  termcap_add("k9",KEY_F+9);
  termcap_add("k;",KEY_F+10);
  return getch2_key_db;
}

#endif

void get_screen_size(void){
#ifdef CONFIG_IOCTL
  struct winsize ws;
  if (ioctl(0, TIOCGWINSZ, &ws) < 0 || !ws.ws_row || !ws.ws_col) return;
/*  printf("Using IOCTL\n"); */
  screen_width=ws.ws_col;
  screen_height=ws.ws_row;
#endif
}

