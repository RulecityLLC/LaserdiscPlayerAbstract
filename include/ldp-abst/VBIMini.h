#ifndef VBIMINI_H
#define VBIMINI_H

#include "datatypes.h"

// contains the relevant data from laserdisc VBI that we actually care about per field

typedef struct VBIMini_s
{
	// frame number for this field (or ~0 if none)
	unsigned int uFrameNum;

	// non-zero if this field has a stop code
	unsigned int uIsStopCode;
} VBIMini_t;

#define VBIMiniNoFrame ((uint32_t) ~0)

#endif // VBIMINI_H
