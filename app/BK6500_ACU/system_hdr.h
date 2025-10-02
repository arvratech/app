#include "..\BootLoader\portdef.h"
#include "..\BootLoader\def.h"
#if defined(BK5100) || defined(CPU2410A)
#define CPU2410A		1
#include "..\BootLoader\2410addr.h"
#include "..\BootLoader\2410slib.h" 
#include "..\BootLoader\2410lib.h"
#else
#ifdef CPU2440A
#include "..\BootLoader\2440addr.h"
#include "..\BootLoader\2440slib.h" 
#include "..\BootLoader\2440lib.h"
#else
#define CPU2442B		1
#include "..\BootLoader\2440addr.h"
#include "..\BootLoader\2440slib.h" 
#include "..\BootLoader\2440lib.h"
#endif
#endif