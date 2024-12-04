#ifndef LDP_SEARCH_DELAY_H
#define LDP_SEARCH_DELAY_H

#include "datatypes.h"

typedef enum
{
	LDPSD_FALSE = 0,
	LDPSD_TRUE = 1
} LDPSD_BOOL;

#ifdef __cplusplus
extern "C"
{
#endif // c++

void ldpsd_start_counter(uint16_t u16FieldsToDelay);
void ldpsd_on_vsync();	// count down one field

// returns true if search can finish (ie seek delay period has ended)
LDPSD_BOOL ldpsd_can_search_finish();

#ifdef __cplusplus
}
#endif // c++

#endif // LDP_SEARCH_DELAY_H
