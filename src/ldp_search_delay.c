#include <ldp-abst/ldp_search_delay.h>

uint16_t g_u16FieldsLeftBeforeSearchFinished = 0;

void ldpsd_start_counter(uint16_t u16FieldsToDelay)
{
	g_u16FieldsLeftBeforeSearchFinished = u16FieldsToDelay;
}

void ldpsd_on_vsync()
{
	if (g_u16FieldsLeftBeforeSearchFinished != 0)
	{
		g_u16FieldsLeftBeforeSearchFinished--;
	}
}

LDPSD_BOOL ldpsd_can_search_finish()
{
	return (g_u16FieldsLeftBeforeSearchFinished == 0);
}
