#include <stdio.h>

#define ONE		256

int As = 0xb0;
int Cs = 0xb0;
int Ad = 0x60;
int Cd = 0x60;
int Ag = 0x90;

int main(int argc, char *argv[])
{
	int		i, a, c;

	// 1  S_ALPHA=1
	a = As + Ad * (ONE-As) / ONE;
	c = (Cs * As + Cd * (ONE-As)) / ONE;
printf("a=%02x c=%02x\n", a, c);
	// 1  S_ALPHA=0
	a = As + Ad * (ONE-As) / ONE;
	c = Cs + Cd * (ONE-As) / ONE;
printf("a=%02x c=%02x\n", a, c);
	// 6  S_ALPHA=0/1
	a = Ag + Ad * (ONE-Ag) / ONE;
	c = (Cs * Ag + Cd * (ONE-Ag)) / ONE;
printf("a=%02x c=%02x\n", a, c);
	// 7  S_ALPHA=1
for(i = 0;i < 8;i++) { 
Ag = 0xf0 - 0x10 * i;
	a = As + Ad * (ONE-As) / ONE;
	c = (Cs * As + Cd * (ONE-As)) / ONE;
	As = a; Cs = c;
	a = (As * Ag + Ad * (ONE-Ag)) / ONE;
	c = (Cs * Ag + Cd * (ONE-Ag)) / ONE;
printf("Ag=%02x a=%02x c=%02x\n", Ag, a, c);
As = 0xb0; Cs = 0xb0;
}
printf("\n");
	// 7  S_ALPHA=0
for(i = 0;i < 8;i++) { 
Ag = 0xf0 - 0x10 * i;
	a = As + Ad * (ONE-As) / ONE;
	c = Cs + Cd * (ONE-As) / ONE;
	As = a; Cs = c;
	a = (As * Ag + Ad * (ONE-Ag)) / ONE;
	c = (Cs * Ag + Cd * (ONE-Ag)) / ONE;
printf("Ag=%02x a=%02x c=%02x\n", Ag, a, c);
As = 0xb0; Cs = 0xb0;
}



}


