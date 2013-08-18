/* Option here to check turn on and off debug mode compile. */
//#define NDEBUG

/* Packed structures */
#define PACK

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <io.h>
#include <mem.h>
#include <string.h>

#include "GENBOR.H"
#include "MEMORY.H"
#include "DEBUG.H"
#include "FILE.H"
#include "ditalk.h"
#include "packet.h"
#include "ipx.h"

#define REAL_MODE