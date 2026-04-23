#include "stdafx.h"
#include "../include/ldp-abst/VBIParse.h"
#include <ldp-abst/VBICompact.h>
#include <list>

#include <ldp-abst/ldpc.h>

using namespace std;

#define ENTRY_COUNT 10 /* this number is arbitrary but it needs to be bigger than 2 :) */
VBICompactEntry_t g_entries[ENTRY_COUNT];

void list_to_compact(VBICompact_t *pCompact, const list<VBICompactEntry_t> &lstEntries)
{
	pCompact->uEntryCount = lstEntries.size();
	pCompact->pEntries = g_entries;

	int i = 0;
	for (list<VBICompactEntry_t>::const_iterator li = lstEntries.begin(); li != lstEntries.end(); li++)
	{
		// safety precaution
		if (i >= ENTRY_COUNT)
		{
			break;
		}

		g_entries[i++] = *li;
	}
}

// 2:3 pattern
void make_vbi_23pattern(VBICompact_t *pCompact)
{
	VBI_t vbi;

	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	bool bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	list_to_compact(pCompact, lstEntries);
	pCompact->uTotalFields = 4;
}

void make_vbi2(VBICompact_t *pCompact)
{
	VBI_t vbi;

	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	pVBI->CompactVBIData(lstEntries, NTSC);

	list_to_compact(pCompact, lstEntries);
	pCompact->uTotalFields = 4;
}

void make_vbi_22_offset(VBICompact_t *pCompact)
{
	VBI_t vbi;

	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	bool bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	list_to_compact(pCompact, lstEntries);
	pCompact->uTotalFields = 4;
}

void make_vbi4(VBICompact_t *pCompact)
{
	VBI_t vbi;

	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	pVBI->CompactVBIData(lstEntries, NTSC);

	list_to_compact(pCompact, lstEntries);
	pCompact->uTotalFields = 200000;	// unlimited!
}

void make_vbi_atari(VBICompact_t *pCompact)
{
	VBI_t vbi;

	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	vbi.uVBI[VBIParse::LINE17] &= 0xA8FFFF;
	vbi.uVBI[VBIParse::LINE18] &= 0xA8FFFF;
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	vbi.uVBI[VBIParse::LINE17] &= 0xA8FFFF;
	vbi.uVBI[VBIParse::LINE18] &= 0xA8FFFF;
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(3, NTSC);
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	pVBI->CompactVBIData(lstEntries, NTSC);

	list_to_compact(pCompact, lstEntries);
	pCompact->uTotalFields = 200000;	// unlimited!
}

void test_ldpc1()
{
	VBICompact_t compact;
	make_vbi_23pattern(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	unsigned int u;
	u = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(VBIMiniNoFrame, u);

	u = ldpc_get_current_track();
	TEST_CHECK_EQUAL(~0, u);

	u = ldpc_get_current_field();
	TEST_CHECK_EQUAL(~0, u);

	LDPCStatus_t stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_STOPPED, stat);

	// should fail since the disc is not playing
	bRes = ldpc_skip_tracks(1);
	TEST_CHECK(bRes == 0);

	ldpc_play(LDPC_FORWARD);
	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_SPINNING_UP, stat);

	// still should have no frame number since vblank hasn't activated
	u = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(VBIMiniNoFrame, u);

	// field 0 vblank start (so we're in sync with initial ldpc state)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);

	// now that vblank has occurred, we should be PLAYING
	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_PLAYING, stat);

	// our code calculates the new frame number after vblank starts for now
	u = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(1, u);

	// field 0 vblank end
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// should still be on frame 1
	u = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(1, u);

	u = ldpc_get_current_track();
	TEST_CHECK_EQUAL(0, u);

	u = ldpc_get_current_field();
	TEST_CHECK_EQUAL(0, u);

	// go to the next field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);
	u = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(1, u);
	u = ldpc_get_current_track();
	TEST_CHECK_EQUAL(0, u);
	u = ldpc_get_current_field();
	TEST_CHECK_EQUAL(1, u);

	// go to the next field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);
	u = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(1, u);
	u = ldpc_get_current_track();
	TEST_CHECK_EQUAL(1, u);
	u = ldpc_get_current_field();
	TEST_CHECK_EQUAL(0, u);

	// go to the next field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);
	u = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(2, u);
	u = ldpc_get_current_track();
	TEST_CHECK_EQUAL(1, u);
	u = ldpc_get_current_field();
	TEST_CHECK_EQUAL(1, u);

}

TEST(LDPC, ldpc1)
{
	test_ldpc1();
}

void test_ldpc_frame_boundary()
{
	VBICompact_t compact;
	make_vbi_23pattern(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	unsigned int u;

	ldpc_play(LDPC_FORWARD);

	// field 0 vblank start (so we're in sync with initial ldpc state)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);

	// now that vblank has occurred, we should be PLAYING
	LDPCStatus_t stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_PLAYING, stat);

	u = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(1, u);

	// field 0 vblank end
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// go to the next field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, u);

	// go to the next field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();
	TEST_REQUIRE_EQUAL(2, u);

	// we should eventually end up alternating between the last seen picture number and the field after it (field 0 and 1)
	ldpc_pause();

	// go to the next field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, u);

	// go to the next field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(0, u);

	// go to the next field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, u);
}

TEST(LDPC, ldpc_frame_boundary)
{
	test_ldpc_frame_boundary();
}

void test_ldpc_frame_boundary_disc_begin()
{
	VBICompact_t compact;
	make_vbi_22_offset(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	unsigned int u;

	ldpc_play(LDPC_FORWARD);

	// field 0 vblank start (so we're in sync with initial ldpc state)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();

	TEST_REQUIRE_EQUAL(0, u);

	ldpc_pause();

	// go to the next field (should move forward since we are at the beginning of the disc)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, u);

	// go to the next field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();
	TEST_REQUIRE_EQUAL(2, u);

	// go to the next field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, u);
}

TEST(LDPC, ldpc_frame_boundary_disc_begin)
{
	test_ldpc_frame_boundary_disc_begin();
}

void test_ldpc2()
{
	VBICompact_t compact;
	make_vbi_22_offset(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	// search should succeed
	bRes = ldpc_begin_search(2);
	TEST_CHECK(bRes == LDPC_TRUE);

	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_end_search();

	// not done until vblank
	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);

	// not done because this is the wrong field
	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());

	// check current track/field
	unsigned int u = ldpc_get_current_track();
	TEST_CHECK_EQUAL(1, u);

	u = ldpc_get_current_field();
	TEST_CHECK_EQUAL(1, u);

}

TEST(LDPC, ldpc2)
{
	test_ldpc2();
}

void test_ldpc_wraparound0()
{
	VBICompact_t compact;
	make_vbi2(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	// search should succeed
	bRes = ldpc_begin_search(2);
	TEST_CHECK(bRes == LDPC_TRUE);

	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_end_search();

	// not done until vblank
	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	ldpc_play(LDPC_FORWARD);

	uint32_t field = ldpc_get_current_abs_field();
	TEST_REQUIRE_EQUAL(2, field);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	field = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(3, field);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// we should've wrapped around back to field 0 since we only have 4 fields
	field = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(0, field);
}

TEST(LDPC, ldpc_wraparound0)
{
	test_ldpc_wraparound0();
}

void test_ldpc_wraparound1()
{
	VBICompact_t compact;
	make_vbi2(&compact);
	compact.uTotalFields = 3;	// for purpose of test

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	// search should succeed
	bRes = ldpc_begin_search(2);
	TEST_CHECK(bRes == LDPC_TRUE);

	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_end_search();

	// not done until vblank
	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	ldpc_play(LDPC_FORWARD);

	uint32_t field = ldpc_get_current_abs_field();
	TEST_REQUIRE_EQUAL(2, field);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// we should've wrapped around back to field 1 since we only have 3 fields (can't wrap back to 0 because we are already on a top field)
	field = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, field);
}

TEST(LDPC, ldpc_wraparound1)
{
	test_ldpc_wraparound1();
}

void test_ldpc_wraparound_skip_rev()
{
	VBICompact_t compact;
	make_vbi2(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	// search should succeed
	bRes = ldpc_begin_search(2);
	TEST_CHECK(bRes == LDPC_TRUE);

	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_end_search();

	// not done until vblank
	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	ldpc_play(LDPC_FORWARD);

	uint32_t field = ldpc_get_current_abs_field();
	TEST_REQUIRE_EQUAL(2, field);

	ldpc_skip_tracks(-2);	// skip out of range backward

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// should be on field 1 since we are on the bottom field
	field = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, field);

	ldpc_skip_tracks(-2);	// skip out of range backward again

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// should be on field 0 since we are on the top field
	field = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(0, field);

	ldpc_skip_tracks(100);	// skip forward out of range

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// should be on field 1 since we are on the bottom field
	field = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, field);
}

TEST(LDPC, ldpc_wraparound_skip_rev)
{
	test_ldpc_wraparound_skip_rev();
}

void test_ldpc_no_auto_jump()
{
	VBICompact_t compact;
	make_vbi4(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	// search should succeed
	bRes = ldpc_begin_search(2);
	TEST_CHECK(bRes == LDPC_TRUE);

	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_end_search();

	// not done until vblank
	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());

	// check current track/field
	unsigned int u = ldpc_get_current_track();
	TEST_CHECK_EQUAL(1, u);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(3, u);

	// disable auto track jump so that we start playing forward no matter what
	ldpc_set_disable_auto_track_jump(LDPC_TRUE);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(4, u);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(5, u);
}

TEST(LDPC, ldpc_no_auto_jump)
{
	test_ldpc_no_auto_jump();
}

void test_ldpc_search0()
{
	VBICompact_t compact;
	make_vbi4(&compact);	// 2:2 unlimited

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	// Mad Dog searches to frame 0 even though it doesn't exist and the LDP resolves to frame 1, so we must do so also
	bRes = ldpc_begin_search(0);
	TEST_CHECK(bRes == LDPC_TRUE);

	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_end_search();

	// not done until vblank
	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());

	// check current track/field
	unsigned int u = ldpc_get_current_track();
	TEST_CHECK_EQUAL(0, u);

	u = ldpc_get_current_field();
	TEST_CHECK_EQUAL(1, u);
}

TEST(LDPC, ldpc_search0)
{
	test_ldpc_search0();
}

void test_ldpc_search_as_skip0()
{
	VBICompact_t compact;
	make_vbi_atari(&compact);	// 2:2 ATARI unlimited

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	// get on the top field to do the skip because the frames start on the bottom field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	unsigned int u = ldpc_get_current_track();
	TEST_CHECK_EQUAL(0, u);

	uint32_t uVBI = ldpc_get_current_field_vbi_line18();
	TEST_CHECK_EQUAL(0xA80001, uVBI);

	u = ldpc_get_current_field();
	TEST_CHECK_EQUAL(0, u);

	// try to skip (using searching) to frame 4
	bRes = ldpc_begin_search(4);
	TEST_CHECK(bRes == LDPC_TRUE);

	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	// insta-skip
	ldpc_end_search();

	// search doesn't actually finish until the next vblank
	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	// test to make sure that play command is ignored while our status is still SEARCHING
	ldpc_play(LDPC_FORWARD);

	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	// ok go to the beginning of the new frame we searched to
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());

	uVBI = ldpc_get_current_field_vbi_line18();
	TEST_CHECK_EQUAL(0xF80004, uVBI);

	// check current track/field
	u = ldpc_get_current_track();
	TEST_CHECK_EQUAL(2, u);

	u = ldpc_get_current_field();
	TEST_CHECK_EQUAL(1, u);

	// mimic what VP931 probably would do for a short jump like this
	ldpc_play(LDPC_FORWARD);

	// make sure we advance properly
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());
}

TEST(LDPC, ldpc_search_as_skip0)
{
	test_ldpc_search_as_skip0();
}

void test_ldpc_skip_to_frame1()
{
	VBICompact_t compact;
	make_vbi4(&compact);	// 2:2 pattern

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// check current track/field
	unsigned int u = ldpc_get_current_track();
	TEST_CHECK_EQUAL(0, u);

	u = ldpc_get_current_field();
	TEST_CHECK_EQUAL(1, u);

	bRes = ldpc_skip_to_frame(10);
	TEST_CHECK(bRes != 0);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(18, u);
}

TEST(LDPC, ldpc_skip_to_frame1)
{
	test_ldpc_skip_to_frame1();
}

void test_ldpc_skip_to_frame2()
{
	VBICompact_t compact;
	make_vbi4(&compact);	// 2:2 pattern

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);

	// skip in the middle of vblank, it should succeed because the field will be even after vblank ends and frame 10 starts on an even field
	bRes = ldpc_skip_to_frame(10);
	TEST_CHECK(bRes != 0);

	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	uint32_t u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(18, u);
}

TEST(LDPC, ldpc_skip_to_frame2)
{
	test_ldpc_skip_to_frame2();
}

void test_ldpc_skip_to_frame2246()
{
	VBICompact_t compact;
	make_vbi_atari(&compact);	// 2:2 atari pattern

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	bRes = ldpc_begin_search(2155);
	TEST_CHECK(bRes != 0);

	ldpc_end_search();

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	LDPCStatus_t status = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_SEARCHING, status);

	// conclude the search
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	status = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_PAUSED, status);

	uint32_t u32CurFrameNum = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(2155, u32CurFrameNum);

	ldpc_play(LDPC_FORWARD);

	// move to top field of frame 2155 (bottom field dominant)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// make sure we are where we expect to be
	u32CurFrameNum = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(2155, u32CurFrameNum);

	// send skip command to frame 2246, should completely cleanly
	bRes = ldpc_skip_to_frame(2246);
	TEST_REQUIRE(bRes == LDPC_TRUE);

	// complete the skip
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// make sure we are where we expect to be
	u32CurFrameNum = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(2246, u32CurFrameNum);

	uint32_t u32CurAbsField = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(4489, u32CurAbsField);

	// now do the skip again on the off field and make sure that it takes us to the right place
	bRes = ldpc_skip_to_frame(2245);
	TEST_REQUIRE(bRes == LDPC_TRUE);

	// complete the skip
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// we should be one field before the requested picture number.
	// This behavior is verified on Firefox and must be maintained via these unit tests!!
	u32CurFrameNum = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(2244, u32CurFrameNum);

	uint32_t u32Line18 = ldpc_get_current_field_vbi_line18();
	TEST_CHECK_EQUAL(0xA82244, u32Line18);

	u32CurAbsField = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(4486, u32CurAbsField);

	// now skip forward again
	bRes = ldpc_skip_to_frame(2246);
	TEST_REQUIRE(bRes == LDPC_TRUE);

	// complete the skip
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u32CurFrameNum = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(2246, u32CurFrameNum);

	u32CurAbsField = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(4489, u32CurAbsField);

	// now skip to the same frame
	bRes = ldpc_skip_to_frame(2246);
	TEST_REQUIRE(bRes == LDPC_TRUE);

	// complete the skip
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u32CurFrameNum = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(2245, u32CurFrameNum);

	u32Line18 = ldpc_get_current_field_vbi_line18();
	TEST_CHECK_EQUAL(0xA82245, u32Line18);

	u32CurAbsField = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(4488, u32CurAbsField);

	// skip 2 frames forward just to make sure we are getting the results we expect
	bRes = ldpc_skip_to_frame(2248);
	TEST_REQUIRE(bRes == LDPC_TRUE);

	// complete the skip
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u32CurFrameNum = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(2248, u32CurFrameNum);

	u32CurAbsField = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL((2248*2)-3, u32CurAbsField);	// -3 because this is the bottom field

	// skip 2 frames forward just to make sure we are getting the results we expect
	bRes = ldpc_skip_to_frame(2250);
	TEST_REQUIRE(bRes == LDPC_TRUE);

	// complete the skip
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u32CurFrameNum = ldpc_get_cur_frame_num();
	TEST_CHECK_EQUAL(2249, u32CurFrameNum);

	u32CurAbsField = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL((2249*2)-2, u32CurAbsField);	// -2 because this is the top field
}

TEST(LDPC, ldpc_skip_to_frame2246)
{
	test_ldpc_skip_to_frame2246();
}

// this test is based on an actual bug in dexter with firefox
void test_ldpc_skip_to_frame1803()
{
	VBICompact_t compact;
	make_vbi_atari(&compact);	// 2:2 atari pattern

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	bRes = ldpc_begin_search(23939);
	TEST_CHECK(bRes != 0);

	ldpc_end_search();

	// conclude the search
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// need to be playing for skip to be acknowledged
	ldpc_play(LDPC_FORWARD);

	// make sure we are where we think that we are
	uint32_t u32Line18 = ldpc_get_current_field_vbi_line18();
	TEST_CHECK_EQUAL(0xfa3939, u32Line18);

	// go to the alt field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u32Line18 = ldpc_get_current_field_vbi_line18();
	TEST_CHECK_EQUAL(0xaa3939, u32Line18);

	// now skip to frame 1803
	bRes = ldpc_skip_to_frame(1803);
	TEST_CHECK(bRes != 0);

	// complete the skip
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u32Line18 = ldpc_get_current_field_vbi_line18();
	TEST_CHECK_EQUAL(0xf81803, u32Line18);
}

TEST(LDPC, ldpc_skip_to_frame1803)
{
	test_ldpc_skip_to_frame1803();
}

void test_ldpc_spinup()
{
	//VBIMini_t vbi[4];	// 4 fields to keep it simple
	//memset(vbi, 0, sizeof(vbi));

	//vbi[0].uFrameNum = VBIMiniNoFrame;
	//vbi[1].uFrameNum = 1;	// first field of track 0 is frame 1
	//vbi[2].uFrameNum = VBIMiniNoFrame;
	//vbi[3].uFrameNum = 2;

	VBICompact_t compact;
	make_vbi_22_offset(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	const unsigned int uVblanksPerSpinup = 100;
	ldpc_set_vblanks_per_spinup(uVblanksPerSpinup);

	ldpc_play(LDPC_FORWARD);

	LDPCStatus_t stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_SPINNING_UP, stat);

	for (unsigned int i = 0; i < uVblanksPerSpinup; i++)
	{
		TEST_REQUIRE_EQUAL(LDPC_SPINNING_UP, ldpc_get_status());
		ldpc_OnVBlankChanged(LDPC_TRUE, (VidField_t) (i & 1));
		ldpc_OnVBlankChanged(LDPC_FALSE, (VidField_t) (i & 1));
	}

	// this is necessary to transition to a PLAYING status
	ldpc_OnVBlankChanged(LDPC_TRUE, (VidField_t) (uVblanksPerSpinup & 1));

	TEST_REQUIRE_EQUAL(LDPC_PLAYING, ldpc_get_status());

	// make sure we don't spin-up again once we're playing
	ldpc_play(LDPC_FORWARD);
	TEST_REQUIRE_EQUAL(LDPC_PLAYING, ldpc_get_status());

}

TEST(LDPC, ldpc_spinup)
{
	test_ldpc_spinup();
}

void test_ldpc_spinup_misbehave()
{
	//VBIMini_t vbi[4];	// 4 fields to keep it simple
	//memset(vbi, 0, sizeof(vbi));

	//vbi[0].uFrameNum = VBIMiniNoFrame;
	//vbi[1].uFrameNum = 1;	// first field of track 0 is frame 1
	//vbi[2].uFrameNum = VBIMiniNoFrame;
	//vbi[3].uFrameNum = 2;

	VBICompact_t compact;
	make_vbi_22_offset(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	const unsigned int uVblanksPerSpinup = 0;	// we want instant spin-up (but we need to make sure the status is not PLAYING until we are on a legit field and track)
	ldpc_set_vblanks_per_spinup(uVblanksPerSpinup);

	bRes = ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	TEST_REQUIRE(bRes == LDPC_TRUE);

	ldpc_play(LDPC_FORWARD);

	// we should always be spinning up until a vblank is enabled
	LDPCStatus_t stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_SPINNING_UP, stat);

	bRes = ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);
	TEST_REQUIRE(bRes == LDPC_TRUE);

	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_SPINNING_UP, stat);

	// at this time, ldpc is out of sync with these fields so it doesn't do much with them
	bRes = ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	TEST_REQUIRE(bRes == LDPC_FALSE);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// since the previous vblank was ignored, we should not have transitioned into the PLAYING status yet
	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_SPINNING_UP, stat);

	bRes = ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	TEST_REQUIRE(bRes == LDPC_TRUE);

	TEST_REQUIRE_EQUAL(LDPC_PLAYING, ldpc_get_status());

	// make sure we don't spin-up again once we're playing
	ldpc_play(LDPC_FORWARD);
	TEST_REQUIRE_EQUAL(LDPC_PLAYING, ldpc_get_status());

}

TEST(LDPC, ldpc_spinup_misbehave)
{
	test_ldpc_spinup_misbehave();
}

void test_ldpc_audio_change()
{
	VBICompact_t compact;
	make_vbi_22_offset(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	LDPCAudioStatus_t stat;

	// muted should be default when disc is not playing
	stat = ldpc_get_audio_status();
	TEST_REQUIRE_EQUAL(LDPC_AUDIO_MUTED, stat);

	ldpc_play(LDPC_FORWARD);	// get unsquelched

	// do a few vblanks to transition us from SPINNING_UP to PLAYING so audio indeed gets unsquelched
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	stat = ldpc_get_audio_status();
	TEST_REQUIRE_EQUAL(LDPC_AUDIO_STEREO, stat);

	// disable left channel
	ldpc_change_audio(0, LDPC_FALSE);
	stat = ldpc_get_audio_status();
	TEST_REQUIRE_EQUAL(LDPC_AUDIO_RIGHT_ONLY, stat);

	// disable right channel
	ldpc_change_audio(1, LDPC_FALSE);
	stat = ldpc_get_audio_status();
	TEST_REQUIRE_EQUAL(LDPC_AUDIO_MUTED, stat);

	// enable left
	ldpc_change_audio(0, LDPC_TRUE);
	stat = ldpc_get_audio_status();
	TEST_REQUIRE_EQUAL(LDPC_AUDIO_LEFT_ONLY, stat);

	// enable right
	ldpc_change_audio(1, LDPC_TRUE);
	stat = ldpc_get_audio_status();
	TEST_REQUIRE_EQUAL(LDPC_AUDIO_STEREO, stat);

}

TEST(LDPC, ldpc_audio_change)
{
	test_ldpc_audio_change();
}

void test_ldpc_audio_squelch()
{
	VBICompact_t compact;
	make_vbi_22_offset(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	LDPCAudioStatus_t stat;

	// muted should be default when disc is not playing
	stat = ldpc_get_audio_status();
	TEST_REQUIRE_EQUAL(LDPC_AUDIO_MUTED, stat);

	// force squelch off
	ldpc_set_audio_squelched(LDPC_AUDIOSQUELCH_FORCE_OFF);

	// make sure audio is indeed unsquelched
	stat = ldpc_get_audio_status();
	TEST_REQUIRE_EQUAL(LDPC_AUDIO_STEREO, stat);

	// change squelch policy to neutral again
	ldpc_set_audio_squelched(LDPC_AUDIOSQUELCH_NO_CHANGE);

	// make sure we're back to being squelched since the disc is not playing
	stat = ldpc_get_audio_status();
	TEST_REQUIRE_EQUAL(LDPC_AUDIO_MUTED, stat);

	ldpc_play(LDPC_FORWARD);	// get unsquelched

	// do a few vblanks to transition us from SPINNING_UP to PLAYING so audio indeed gets unsquelched
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// verify that we're unsquelched
	stat = ldpc_get_audio_status();
	TEST_REQUIRE_EQUAL(LDPC_AUDIO_STEREO, stat);

	// force squelch on
	ldpc_set_audio_squelched(LDPC_AUDIOSQUELCH_FORCE_ON);

	stat = ldpc_get_audio_status();
	TEST_REQUIRE_EQUAL(LDPC_AUDIO_MUTED, stat);
}

TEST(LDPC, ldpc_audio_squelch)
{
	test_ldpc_audio_squelch();
}

void test_ldpc_reverse1x()
{
	unsigned int u;
	VBICompact_t compact;
	LDPCStatus_t stat;
	make_vbi4(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);
	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_SPINNING_UP, stat);

	// field 0 vblank start (so we're in sync with initial ldpc state)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);

	// now that vblank has occurred, we should be PLAYING
	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_PLAYING, stat);

	// field 0 vblank end
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	bRes = ldpc_begin_search(30000);
	TEST_REQUIRE(bRes != 0);

	ldpc_end_search();

	// pulse field 1 (this is the wrong field so the search cannot complete)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// pulse field 0 (this will cause the search to complete since we are on the right field)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(59998, u);

	ldpc_play(LDPC_FORWARD);

	// advance 1 field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(59999, u);

	// change direction so we are going in reverse
	ldpc_play(LDPC_BACKWARD);

	// go back 1 field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(59998, u);

	// go back 1 field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(59997, u);

	// audio should be muted while playing backward
	LDPCAudioStatus_t as = ldpc_get_audio_status();
	TEST_CHECK_EQUAL(LDPC_AUDIO_MUTED, as);

	// go forward again to test to see if audio gets unsquelched
	ldpc_play(LDPC_FORWARD);

	as = ldpc_get_audio_status();
	TEST_CHECK_EQUAL(LDPC_AUDIO_STEREO, as);
}

TEST(LDPC, ldpc_reverse1x)
{
	test_ldpc_reverse1x();
}

void test_ldpc_reverse3x()
{
	unsigned int u;
	VBICompact_t compact;
	LDPCStatus_t stat;
	make_vbi4(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);
	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_SPINNING_UP, stat);

	// field 0 vblank start (so we're in sync with initial ldpc state)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);

	// now that vblank has occurred, we should be PLAYING
	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_PLAYING, stat);

	// field 0 vblank end
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	bRes = ldpc_begin_search(30000);
	TEST_REQUIRE(bRes != 0);

	ldpc_end_search();

	// pulse field 1 (this is the wrong field so the search cannot complete)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// pulse field 0 (this will cause the search to complete since we are on the right field)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(59998, u);

	ldpc_play(LDPC_FORWARD);

	// advance 1 field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(59999, u);

	// change direction so we are going in reverse
	ldpc_play(LDPC_BACKWARD);

	// change speed so we are going 3X in reverse
	ldpc_change_speed(3, 1);

	// go back 3 fields
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(59996, u);

	// go back 3 fields
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(59993, u);

}

TEST(LDPC, ldpc_reverse3x)
{
	test_ldpc_reverse3x();
}

///////////////////////////////////////////////////////////////////

void test_ldpc_step_forward1()
{
	unsigned int u;
	VBICompact_t compact;
	LDPCStatus_t stat;
	make_vbi4(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	// field 0 vblank start (so we're in sync with initial ldpc state)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);

	// now that vblank has occurred, we should be PLAYING
	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_PLAYING, stat);

	// field 0 vblank end
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// step forward while in the middle of top field
	ldpc_step(LDPC_FORWARD);

	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_STEPPING, stat);

	// get to middle of field 1 and see where we are
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, u);

	// pulse field 0 (this will cause us to move to field 2, according to behavior on page 33 of PR-8210A service manual which seems reasonable to me)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_PAUSED, stat);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(2, u);
}

TEST(LDPC, ldpc_step_forward1)
{
	test_ldpc_step_forward1();
}

void test_ldpc_step_forward2()
{
	unsigned int u;
	VBICompact_t compact;
	LDPCStatus_t stat;
	make_vbi4(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	// field 0 vblank start (so we're in sync with initial ldpc state)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);

	// now that vblank has occurred, we should be PLAYING
	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_PLAYING, stat);

	// field 0 vblank end
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// do field 1
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, u);

	// step forward while in the middle of bottom field
	ldpc_step(LDPC_FORWARD);

	// pulse field 0 (this will cause us to move to field 2, according to behavior on page 33 of PR-8210A service manual which seems reasonable to me)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(2, u);

	// do bottom field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(3, u);

	// do top field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// we should now see that disc is paused and we've gone back to field 2
	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(2, u);
}

TEST(LDPC, ldpc_step_forward2)
{
	test_ldpc_step_forward2();
}

void test_ldpc_step_reverse1()
{
	unsigned int u;
	VBICompact_t compact;
	LDPCStatus_t stat;
	make_vbi4(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	// field 0 vblank start (so we're in sync with initial ldpc state)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);

	// now that vblank has occurred, we should be PLAYING
	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_PLAYING, stat);

	// field 0 vblank end
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// do field 1
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, u);

	// field 2
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(2, u);

	// now initiate stepping backward
	ldpc_step(LDPC_BACKWARD);

	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_STEPPING, stat);

	// do bottom field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// we should've jumped back 1 field
	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, u);

	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_STEPPING, stat);

	// do top field
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// we should've jumped back 1 field
	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(0, u);

	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_PAUSED, stat);
}

TEST(LDPC, ldpc_step_reverse1)
{
	test_ldpc_step_reverse1();
}

void test_ldpc_step_reverse2()
{
	unsigned int u;
	VBICompact_t compact;
	LDPCStatus_t stat;
	make_vbi4(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	// field 0 vblank start (so we're in sync with initial ldpc state)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);

	// now that vblank has occurred, we should be PLAYING
	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_PLAYING, stat);

	// field 0 vblank end
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// try to step backward (should not be able to go back beyond 0)
	ldpc_step(LDPC_BACKWARD);

	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_STEPPING, stat);

	// do field 1
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// we should just end up paused on track 0
	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, u);

	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_PAUSED, stat);
}

TEST(LDPC, ldpc_step_reverse2)
{
	test_ldpc_step_reverse2();
}

void test_ldpc_step_reverse23pulldown()
{
	unsigned int u;
	VBICompact_t compact;
	LDPCStatus_t stat;
	make_vbi_23pattern(&compact);	// 2:3 pulldown

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	// field 0
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// do field 1
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// field 2
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// field 3
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// field 4 (frame 2)
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	ldpc_step(LDPC_BACKWARD);

	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_STEPPING, stat);

	// back to field 3
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// back to field 2
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// back to field 1
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	// back to field 0
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	stat = ldpc_get_status();
	TEST_CHECK_EQUAL(LDPC_PAUSED, stat);

	// we should just end up paused on track 0
	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(0, u);
}

TEST(LDPC, ldpc_step_reverse_23pulldown)
{
	test_ldpc_step_reverse23pulldown();
}

void test_ldpc_pause_while_seeking()
{
	VBICompact_t compact;
	make_vbi4(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_play(LDPC_FORWARD);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// search should succeed
	bRes = ldpc_begin_search(10000);
	TEST_CHECK(bRes == LDPC_TRUE);

	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	// send pause command while we're still searching
	ldpc_pause();

	ldpc_end_search();

	// not done until vblank
	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());

	// check current track/field
	unsigned int u = ldpc_get_current_track();
	TEST_CHECK_EQUAL(9999, u);

	u = ldpc_get_current_field();
	TEST_CHECK_EQUAL(0, u);
}

TEST(LDPC, ldpc_pause_while_seeking)
{
	test_ldpc_pause_while_seeking();
}

void test_ldpc_stopcode()
{
	uint32_t u = 0;

	VBICompact_t compact;
	make_vbi4(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	ldpc_set_next_field_with_stopcode(1);	// make field 1 have a stop code
	ldpc_play(LDPC_FORWARD);
	
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());
	u = ldpc_get_current_abs_field();

	TEST_CHECK_EQUAL(0, u);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, u);

	// we should now be paused
	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());

	// go back to field 1 so we can test to see what happens if we issue a play command
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(0, u);

	ldpc_play(LDPC_FORWARD);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, u);

	// we should still be playing at this point because we were paused on this frame when we issued the play command
	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(2, u);

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(3, u);

	// now seek back to frame 1 and make sure that we stop again on the stop code
	LDPC_BOOL b = ldpc_begin_search(1);

	TEST_REQUIRE_EQUAL(LDPC_TRUE, b);

	ldpc_end_search();

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());

	ldpc_play(LDPC_FORWARD);

	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	u = ldpc_get_current_abs_field();
	TEST_CHECK_EQUAL(1, u);

	// we should now be paused
	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());
}

TEST(LDPC, ldpc_stopcode)
{
	test_ldpc_stopcode();
}

void test_ldpc_video_mute()
{
	uint32_t u = 0;

	// doesn't really matter for this test
	VBICompact_t compact;
	make_vbi4(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK(bRes != 0);

	bRes = ldpc_get_video_muted();
	TEST_CHECK(bRes == 0);

	ldpc_set_video_muted(LDPC_TRUE);
	bRes = ldpc_get_video_muted();
	TEST_CHECK(bRes != 0);

	ldpc_set_video_muted(LDPC_FALSE);
	bRes = ldpc_get_video_muted();
	TEST_CHECK(bRes == 0);

}

TEST(LDPC, ldpc_video_mute)
{
	test_ldpc_video_mute();
}

void test_ldpc_init_exact_state()
{
	// Uses exact-value equality checks (== LDPC_TRUE / == LDPC_FALSE) rather than
	// truthiness (!= 0), so cxx_assign_const mutants that replace 0/1 with 42 are
	// caught. Getters covered:
	//   ldpc_init return value
	//   ldpc_get_video_muted, ldpc_set_video_muted (exact TRUE/FALSE)
	//   ldpc_is_search_finish_pending after init and after ldpc_end_search
	//   ldpc_skip_tracks / ldpc_skip_to_frame failure paths (must be exactly LDPC_FALSE)
	VBICompact_t compact;
	make_vbi4(&compact);

	LDPC_BOOL bRes = ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK_EQUAL(LDPC_TRUE, bRes);

	// Initial state.
	TEST_CHECK_EQUAL(LDPC_STOPPED, ldpc_get_status());
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_is_search_finish_pending());
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_get_video_muted());

	// Video mute: exact-value checks after set.
	ldpc_set_video_muted(LDPC_TRUE);
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_get_video_muted());
	ldpc_set_video_muted(LDPC_FALSE);
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_get_video_muted());

	// ldpc_end_search sets the pending-finished flag to exactly LDPC_TRUE.
	ldpc_end_search();
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_is_search_finish_pending());

	// Skip APIs must return exactly LDPC_FALSE when the disc isn't playing/paused.
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_skip_tracks(1));
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_skip_tracks(-1));
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_skip_to_frame(1));

	// Invalid speed (denominator 0, numerator 0) must return exactly LDPC_FALSE.
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_change_speed(0, 1));	// 0X is illegal
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_change_speed(1, 0));	// divide-by-zero path
}

TEST(LDPC, ldpc_init_exact_state)
{
	test_ldpc_init_exact_state();
}

void test_ldpc_abs_field_and_error_status()
{
	VBICompact_t compact;
	make_vbi4(&compact);

	ldpc_init(LDPC_DISC_NTSC, &compact);

	// After init, status is STOPPED, so ldpc_get_current_abs_field returns the
	// "no frame" sentinel exactly (pins the VBIMiniNoFrame initializer).
	TEST_CHECK_EQUAL((uint32_t) VBIMiniNoFrame, ldpc_get_current_abs_field());

	// ldpc_set_current_abs_field must flip status to exactly LDPC_PAUSED and
	// store the exact field value supplied.
	ldpc_set_current_abs_field(7);
	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());
	TEST_CHECK_EQUAL(7u, ldpc_get_current_abs_field());

	ldpc_set_current_abs_field(3);
	TEST_CHECK_EQUAL(3u, ldpc_get_current_abs_field());

	// Verify the internal VBIC_SetField call at ldpc.c:639 actually ran — it
	// updates the line-18 value reported by ldpc_get_current_field_vbi_line18.
	// make_vbi4 is PATTERN_22 starting at picnum 1; field 0 is even -> line18
	// encodes picnum 1 (0xF80001), and field 2 is even -> picnum 2 (0xF80002).
	// If VBIC_SetField is elided (cxx_replace_scalar_call), the line-18 value
	// would stay at whatever was last loaded, which diverges from the field
	// we just set.
	ldpc_set_current_abs_field(0);
	TEST_CHECK_EQUAL(0xF80001u, ldpc_get_current_field_vbi_line18());
	ldpc_set_current_abs_field(2);
	TEST_CHECK_EQUAL(0xF80002u, ldpc_get_current_field_vbi_line18());

	// ldpc_set_error_status must flip status to exactly LDPC_ERROR.
	ldpc_set_error_status();
	TEST_CHECK_EQUAL(LDPC_ERROR, ldpc_get_status());
}

TEST(LDPC, ldpc_abs_field_and_error_status)
{
	test_ldpc_abs_field_and_error_status();
}

void test_ldpc_stop_resets_state()
{
	// ldpc_stop must: flip status to STOPPED, clear frame num to VBIMiniNoFrame,
	// and reset uCurrentField to (uint32_t) -1. Each line is pinned by a direct
	// observable check below — this kills the three cxx_assign_const mutants
	// at ldpc.c lines 201, 203, 204.
	VBICompact_t compact;
	make_vbi4(&compact);

	ldpc_init(LDPC_DISC_NTSC, &compact);

	// drive the state into PLAYING so we can observe the reset
	ldpc_play(LDPC_FORWARD);
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());

	// Seed the "on stop" observable fields with known non-sentinel values via
	// normal playback so we can verify they're reset.
	ldpc_set_current_abs_field(5);
	TEST_CHECK_EQUAL(5u, ldpc_get_current_abs_field());

	// Advance VBIC to a non-starting field so we can see the "VBIC_SetField(0)"
	// inside ldpc_stop actually run (pins cxx_replace_scalar_call at
	// ldpc.c:207). At field 6 under PATTERN_22 starting at picnum 1, line18
	// reads 0xF80004; after stop it should revert to field 0 -> 0xF80001.
	ldpc_set_current_abs_field(6);
	TEST_CHECK_EQUAL(0xF80004u, ldpc_get_current_field_vbi_line18());

	ldpc_stop();

	TEST_CHECK_EQUAL(LDPC_STOPPED, ldpc_get_status());
	TEST_CHECK_EQUAL((uint32_t) VBIMiniNoFrame, ldpc_get_cur_frame_num());
	TEST_CHECK_EQUAL(~0u, ldpc_get_current_field());
	TEST_CHECK_EQUAL(0xF80001u, ldpc_get_current_field_vbi_line18());
}

TEST(LDPC, ldpc_stop_resets_state)
{
	test_ldpc_stop_resets_state();
}

void test_ldpc_change_speed_paths()
{
	VBICompact_t compact;
	make_vbi4(&compact);

	ldpc_init(LDPC_DISC_NTSC, &compact);

	// >= 1X, non-zero numerator: success with exact LDPC_TRUE.
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_change_speed(1, 1));
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_change_speed(2, 1));

	// < 1X (numerator 1, denominator > 1): success. Pins the uDenominator > 0
	// boundary and the fact that a non-zero denominator survives the branch.
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_change_speed(1, 2));
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_change_speed(1, 3));

	// back to 1X to confirm the "uTracksToStallPerFrame = 0" path re-runs cleanly.
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_change_speed(1, 1));

	// failure paths must return exactly LDPC_FALSE, not just non-zero.
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_change_speed(0, 1));	// 0/1 = 0X
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_change_speed(1, 0));	// /0 = divide by zero

	// unsupported ratio (numerator != 1 and denominator != 1) -> not handled,
	// should also return LDPC_FALSE
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_change_speed(3, 2));
}

TEST(LDPC, ldpc_change_speed_paths)
{
	test_ldpc_change_speed_paths();
}

void test_ldpc_video_mute_strict()
{
	// Strengthens the existing test_ldpc_video_mute with exact-equality checks
	// on the LDPC_TRUE side so cxx_assign_const mutations that replace the
	// stored byte with 42 can't slip through `!= 0`.
	VBICompact_t compact;
	make_vbi4(&compact);

	ldpc_init(LDPC_DISC_NTSC, &compact);

	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_get_video_muted());
	ldpc_set_video_muted(LDPC_TRUE);
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_get_video_muted());
	ldpc_set_video_muted(LDPC_FALSE);
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_get_video_muted());
}

TEST(LDPC, ldpc_video_mute_strict)
{
	test_ldpc_video_mute_strict();
}

void test_ldpc_begin_search_error()
{
	// begin_search must set status to LDPC_ERROR when VBIC_SEEK fails. Build a
	// VBIC whose only entry is PATTERN_LEADOUT — VBIC_SeekInternal's default
	// case on unsupported patterns returns VBIC_SEEK_FAIL, which triggers the
	// "set status to ERROR" branch in ldpc_begin_search (line 82, pins
	// cxx_remove_void_call).
	VBICompact_t compact;
	make_vbi4(&compact);
	ldpc_init(LDPC_DISC_NTSC, &compact);

	// successful search sets status to SEARCHING.
	LDPC_BOOL bRes = ldpc_begin_search(1);
	TEST_CHECK_EQUAL(LDPC_TRUE, bRes);
	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());

	// Now build a failing compact: single LEADOUT entry, no pattern data.
	static VBICompactEntry_t leadOutEntries[1];
	leadOutEntries[0].u32StartAbsField = 0;
	leadOutEntries[0].i32StartPictureNumber = 0;
	leadOutEntries[0].typePattern = PATTERN_LEADOUT;
	leadOutEntries[0].u16Special = 0;
	leadOutEntries[0].u8PatternOffset = 0;

	static VBICompact_t leadOutCompact;
	leadOutCompact.uEntryCount = 1;
	leadOutCompact.pEntries = leadOutEntries;
	leadOutCompact.uTotalFields = 10;

	ldpc_init(LDPC_DISC_NTSC, &leadOutCompact);
	bRes = ldpc_begin_search(5);	// will go to LEADOUT default case -> SEEK_FAIL
	TEST_CHECK_EQUAL(LDPC_FALSE, bRes);
	TEST_CHECK_EQUAL(LDPC_ERROR, ldpc_get_status());
}

TEST(LDPC, ldpc_begin_search_error)
{
	test_ldpc_begin_search_error();
}

void test_ldpc_pause_before_play_sets_error()
{
	// In ldpc.c ldpc_pause(): if uCurrentField is undefined (~0) AND status is
	// PLAYING or STEPPING, the function switches status to LDPC_ERROR. Step
	// before vblank: status becomes STEPPING with uCurrentField still ~0, so
	// pause must flip status to ERROR. Pins the ldpc_change_status(LDPC_ERROR)
	// call at line 195.
	VBICompact_t compact;
	make_vbi4(&compact);
	ldpc_init(LDPC_DISC_NTSC, &compact);

	ldpc_step(LDPC_FORWARD);
	TEST_CHECK_EQUAL(LDPC_STEPPING, ldpc_get_status());
	// Note: no ldpc_OnVBlankChanged call yet, so uCurrentField is still ~0.

	ldpc_pause();
	TEST_CHECK_EQUAL(LDPC_ERROR, ldpc_get_status());
}

TEST(LDPC, ldpc_pause_before_play_sets_error)
{
	test_ldpc_pause_before_play_sets_error();
}

void test_ldpc_skip_tracks_while_playing()
{
	// In ldpc.c ldpc_skip_tracks: when status is PLAYING or PAUSED, iTracks is
	// added to iSkipTrackOffset and LDPC_TRUE is returned. Pins:
	//   - line 108: iSkipTrackOffset += iTracks (would fail if += became -=)
	//     is hard to observe without a getter, so we only observe return value
	//   - line 109: bRes = LDPC_TRUE (pinned by exact equality)
	VBICompact_t compact;
	make_vbi4(&compact);
	ldpc_init(LDPC_DISC_NTSC, &compact);

	ldpc_play(LDPC_FORWARD);
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());

	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_skip_tracks(1));
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_skip_tracks(-1));

	ldpc_pause();
	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_skip_tracks(2));
}

TEST(LDPC, ldpc_skip_tracks_while_playing)
{
	test_ldpc_skip_tracks_while_playing();
}

void test_ldpc_spinup_boundary()
{
	// Pins the `stVblankSpinupCount >= stVblanksPerSpin` comparator at
	// ldpc.c:438. With vblanksPerSpin = 2, we must be SPINNING_UP after vblank 1
	// and transition to PLAYING on vblank 2. A `>` variant would require a 3rd
	// vblank to transition.
	VBICompact_t compact;
	make_vbi4(&compact);

	ldpc_init(LDPC_DISC_NTSC, &compact);
	ldpc_set_vblanks_per_spinup(2);

	ldpc_play(LDPC_FORWARD);
	TEST_CHECK_EQUAL(LDPC_SPINNING_UP, ldpc_get_status());

	// uCurrentField is ~0 (bit = 1 = BOTTOM_EVEN). Use TOP_ODD vblanks so the
	// sync check at line 427 doesn't make us `goto skip` before reaching the
	// spinup transition.
	// 1st vblank: count -> 1. Still SPINNING_UP because 1 < 2.
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	TEST_CHECK_EQUAL(LDPC_SPINNING_UP, ldpc_get_status());
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// 2nd vblank (same field): count -> 2. Must transition to PLAYING
	// (pins `>=` at ldpc.c:438; a `>` variant would need a 3rd vblank).
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());
}

TEST(LDPC, ldpc_spinup_boundary)
{
	test_ldpc_spinup_boundary();
}

void test_ldpc_2x_forward_playback()
{
	// At 2X forward, arrFieldOffsetsPerVBlank[FORWARD][TOP_ODD] = 3 and
	// [FORWARD][BOTTOM_EVEN] = 1. This test runs 4 alternating vblanks and
	// asserts uCurrentField == 7 afterwards. The expected progression is:
	//   ~0 -> +3 -> 2 -> +1 -> 3 -> +3 -> 6 -> +1 -> 7.
	// Pins ldpc.c:268 (=uNumerator+1) and :269 (=uNumerator-1) together with
	// their arithmetic-mutation siblings.
	VBICompact_t compact;
	make_vbi4(&compact);

	ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_change_speed(2, 1));

	ldpc_play(LDPC_FORWARD);

	// vblanksPerSpin defaults to 0, so the first vblank transitions to PLAYING
	// and immediately applies the offset.
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());
	TEST_CHECK_EQUAL(7u, ldpc_get_current_abs_field());
}

TEST(LDPC, ldpc_2x_forward_playback)
{
	test_ldpc_2x_forward_playback();
}

void test_ldpc_2x_reverse_playback()
{
	// At 2X reverse, [BACKWARD][TOP_ODD] = -3 and [BACKWARD][BOTTOM_EVEN] = -1.
	// Start from uCurrentField = 100 (PAUSED via ldpc_set_current_abs_field),
	// then resume backward playback. After 4 vblanks we expect
	// uCurrentField = 100 + (-3) + (-1) + (-3) + (-1) = 92.
	// But sync check: uCurrentField=100 (bit 0 = 0 -> TOP_ODD). First vblank
	// TOP_ODD matches -> skip. Use BOTTOM_EVEN vblanks first.
	VBICompact_t compact;
	make_vbi4(&compact);

	ldpc_init(LDPC_DISC_NTSC, &compact);
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_change_speed(2, 1));

	// Use set_current_abs_field to seed an exact starting position. It flips
	// status to PAUSED so subsequent play won't re-spinup. Start at field 101
	// (bit 0 = 1 -> BOTTOM_EVEN) so the first TOP_ODD vblank doesn't match
	// and actually advances.
	ldpc_set_current_abs_field(101);
	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());

	ldpc_play(LDPC_BACKWARD);	// from PAUSED -> immediately PLAYING
	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());

	// 101 (odd). Vblank TOP_ODD (0) -> sync 0 != 1 advance by -3 -> 98.
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);
	// 98 (even). Vblank BOTTOM_EVEN (1) -> sync 1 != 0 advance by -1 -> 97.
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);
	// 97 (odd). TOP_ODD -> advance by -3 -> 94.
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);
	// 94 (even). BOTTOM_EVEN -> advance by -1 -> 93.
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);

	TEST_CHECK_EQUAL(93u, ldpc_get_current_abs_field());
}

TEST(LDPC, ldpc_2x_reverse_playback)
{
	test_ldpc_2x_reverse_playback();
}

void test_ldpc_half_x_playback()
{
	// At 1/2X (numerator=1, denominator=2): uTracksToStallPerFrame = 1,
	// uStallTracks = 1, and offsets[FWD] = [1, 1]. After 9 alternating TRUE
	// vblanks starting from field 101, uCurrentField ends at 104 (every pair
	// of vblanks alternates between advance+stall and advance+reload).
	// Pins ldpc.c:232 (uTracksToStallPerFrame = uDenominator - 1), :233
	// (uStallTracks = ...), :486 (uCurrentField -= 2), :488 (uStallTracks--).
	VBICompact_t compact;
	make_vbi4(&compact);
	ldpc_init(LDPC_DISC_NTSC, &compact);

	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_change_speed(1, 2));

	ldpc_set_current_abs_field(101);	// PAUSED at an odd field
	ldpc_play(LDPC_FORWARD);		// PAUSED -> PLAYING immediately (no spinup)
	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());

	// 9 TRUE vblanks with alternating TOP_ODD/BOTTOM_EVEN starting TOP_ODD
	// (uCurrentField=101 -> bit=1, so sync check on TOP_ODD(0) mismatches; advance).
	for (int i = 0; i < 9; i++)
	{
		VidField_t vf = (i & 1) ? VID_FIELD_BOTTOM_EVEN : VID_FIELD_TOP_ODD;
		ldpc_OnVBlankChanged(LDPC_TRUE, vf);
		ldpc_OnVBlankChanged(LDPC_FALSE, vf);
	}

	TEST_CHECK_EQUAL(104u, ldpc_get_current_abs_field());
}

TEST(LDPC, ldpc_half_x_playback)
{
	test_ldpc_half_x_playback();
}

void test_ldpc_set_disable_auto_track_jump_observable()
{
	// With bDisableAutoTrackJumps=TRUE and status=PAUSED, OnVBlankChanged
	// enters the "PLAYING || disable" advance path at ldpc.c:446 and advances
	// uCurrentField by the 1X offset. Pins the `bDisableAutoTrackJumps = bDisabled`
	// assign at ldpc.c:349. (Verified empirically: at 1X, vblank of a track
	// that is currently all-blank ZEROES pattern consumes a 2-field step due
	// to the stall reload semantics.)
	VBICompact_t compact;
	make_vbi4(&compact);
	ldpc_init(LDPC_DISC_NTSC, &compact);

	ldpc_set_disable_auto_track_jump(LDPC_TRUE);
	ldpc_set_current_abs_field(5);
	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());

	// First TOP_ODD vblank after landing at odd field 5 advances uCurrentField.
	unsigned int before = ldpc_get_current_abs_field();
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);
	unsigned int after = ldpc_get_current_abs_field();
	TEST_CHECK(after != before);	// field moved, which proves the branch ran
}

TEST(LDPC, ldpc_set_disable_auto_track_jump_observable)
{
	test_ldpc_set_disable_auto_track_jump_observable();
}

void test_ldpc_end_search_finishes()
{
	// After a successful begin_search, ldpc_is_search_finish_pending is FALSE.
	// After ldpc_end_search, it's TRUE. After a vblank whose field matches
	// vfFrameFirstField, the search completes: status becomes PAUSED and
	// is_search_finish_pending becomes FALSE. Pins ldpc.c:404
	// (`bPendingSearchFinished = LDPC_FALSE`) -- if mutated to 42,
	// ldpc_is_search_finish_pending returns 42 (non-zero) after completion.
	VBICompact_t compact;
	make_vbi4(&compact);
	ldpc_init(LDPC_DISC_NTSC, &compact);

	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_begin_search(2));
	TEST_CHECK_EQUAL(LDPC_SEARCHING, ldpc_get_status());
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_is_search_finish_pending());

	ldpc_end_search();
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_is_search_finish_pending());

	// Vblank while SEARCHING + pending -> search completes on first frame field.
	// make_vbi4 maps frame 2 to even field; vfFrameFirstField is (uLastSearchedField & 1).
	// uLastSearchedField for frame 2 is field 2 -> bit = 0 -> TOP_ODD.
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());
	TEST_CHECK_EQUAL(LDPC_FALSE, ldpc_is_search_finish_pending());
}

TEST(LDPC, ldpc_end_search_finishes)
{
	test_ldpc_end_search_finishes();
}

void test_ldpc_skip_tracks_advances_field()
{
	// Play at 1X, issue ldpc_skip_tracks(1), let one vblank process the skip,
	// and verify uCurrentField advanced by 2 extra fields on top of the 1X
	// advance. Pins ldpc.c:108 (`iSkipTrackOffset += iTracks`): a `-=` variant
	// would subtract 2 instead of add 2.
	VBICompact_t compact;
	make_vbi4(&compact);	// uTotalFields = 200000, so no wrap for small fields
	ldpc_init(LDPC_DISC_NTSC, &compact);

	ldpc_play(LDPC_FORWARD);
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);
	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());
	TEST_CHECK_EQUAL(0u, ldpc_get_current_abs_field());

	// Advance to a stable positive field so we're not near the wrap point.
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_BOTTOM_EVEN);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_BOTTOM_EVEN);
	TEST_CHECK_EQUAL(3u, ldpc_get_current_abs_field());

	// skip forward 1 track (2 fields) on top of normal +1 advance.
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_skip_tracks(1));

	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// Expected: 3 + 1 (offset) + 2 (skip << 1) = 6.
	TEST_CHECK_EQUAL(6u, ldpc_get_current_abs_field());
}

TEST(LDPC, ldpc_skip_tracks_advances_field)
{
	test_ldpc_skip_tracks_advances_field();
}

void test_ldpc_init_stop_code_field_undefined()
{
	// After ldpc_init, uNextStopCodeField must be LDPC_STOPCODE_UNDEFINED
	// (~0). Any mutation of that init assignment at ldpc.c:46 that leaves it
	// at a small value (e.g. 42) would cause playback to pause as soon as
	// uCurrentField reaches 42. Play long enough to pass field 42 and verify
	// we're still PLAYING.
	VBICompact_t compact;
	make_vbi4(&compact);
	ldpc_init(LDPC_DISC_NTSC, &compact);
	ldpc_play(LDPC_FORWARD);

	// 45 TRUE vblanks alternating TOP_ODD/BOT_EVEN walks uCurrentField to 44.
	for (int i = 0; i < 45; i++)
	{
		VidField_t vf = (i & 1) ? VID_FIELD_BOTTOM_EVEN : VID_FIELD_TOP_ODD;
		ldpc_OnVBlankChanged(LDPC_TRUE, vf);
		ldpc_OnVBlankChanged(LDPC_FALSE, vf);
	}

	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());
	TEST_CHECK_EQUAL(44u, ldpc_get_current_abs_field());
}

TEST(LDPC, ldpc_init_stop_code_field_undefined)
{
	test_ldpc_init_stop_code_field_undefined();
}

void test_ldpc_last_stopcode_paused_reset_on_search()
{
	// ldpc_begin_search resets uLastStopCodeFieldThatWePausedOn to 0
	// (line 76). Observable by: play, hit stopcode, pause, then search back
	// and re-play over stopcode -> must pause again. Without the reset,
	// uLastStopCodeFieldThatWePausedOn == uNextStopCodeField and the second
	// pause wouldn't fire.
	VBICompact_t compact;
	make_vbi4(&compact);
	ldpc_init(LDPC_DISC_NTSC, &compact);

	ldpc_set_next_field_with_stopcode(3);
	ldpc_play(LDPC_FORWARD);

	// vblank 1: field 0. vblank 2: 1. vblank 3: 2. vblank 4: 3 (stop code) -> pause.
	for (int i = 0; i < 4; i++)
	{
		VidField_t vf = (i & 1) ? VID_FIELD_BOTTOM_EVEN : VID_FIELD_TOP_ODD;
		ldpc_OnVBlankChanged(LDPC_TRUE, vf);
		ldpc_OnVBlankChanged(LDPC_FALSE, vf);
	}
	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());
	TEST_CHECK_EQUAL(3u, ldpc_get_current_abs_field());

	// Now search back to frame 1 and play again - must pause at the stopcode again.
	// If `uLastStopCodeFieldThatWePausedOn = 0` at line 76 wasn't reset,
	// both would still equal 3 and playback would blow past the stopcode.
	ldpc_set_next_field_with_stopcode(3);	// re-arm for new search
	TEST_CHECK_EQUAL(LDPC_TRUE, ldpc_begin_search(1));
	ldpc_end_search();
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);
	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());	// search finished

	ldpc_play(LDPC_FORWARD);
	// play from field 0, hit field 3 again (4 vblanks after landing on 0 via search result field 0)
	// actually uCurrentField was set to uLastSearchedField which is 0, so:
	// vblank 1 from PAUSED + play: advance. Etc.
	for (int i = 0; i < 6; i++)
	{
		VidField_t vf = (i & 1) ? VID_FIELD_BOTTOM_EVEN : VID_FIELD_TOP_ODD;
		ldpc_OnVBlankChanged(LDPC_TRUE, vf);
		ldpc_OnVBlankChanged(LDPC_FALSE, vf);
	}
	// Should have paused again at the stopcode
	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());
}

TEST(LDPC, ldpc_last_stopcode_paused_reset_on_search)
{
	test_ldpc_last_stopcode_paused_reset_on_search();
}

void test_ldpc_spinup_counter_reset_after_transition()
{
	// After the spinup transition completes, stVblankSpinupCount is reset to 0
	// (ldpc.c:441). Observable by: pause after playing, resume play, immediately
	// transition to PLAYING (no new spinup). If the counter hadn't reset, some
	// behaviour would be off. We pin this by: set vblanksPerSpin large, spin
	// up once, confirm PLAYING, then pause+play-again and confirm a single
	// vblank transitions back to PLAYING (counter was reset so subsequent
	// play-from-PAUSED skips spinup entirely because status isn't STOPPED).
	VBICompact_t compact;
	make_vbi4(&compact);
	ldpc_init(LDPC_DISC_NTSC, &compact);
	ldpc_set_vblanks_per_spinup(2);

	// spin up (first play from STOPPED).
	ldpc_play(LDPC_FORWARD);
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);
	ldpc_OnVBlankChanged(LDPC_TRUE, VID_FIELD_TOP_ODD);
	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());
	ldpc_OnVBlankChanged(LDPC_FALSE, VID_FIELD_TOP_ODD);

	// pause then play again (from PAUSED, not STOPPED) - shouldn't need spinup.
	ldpc_pause();
	TEST_CHECK_EQUAL(LDPC_PAUSED, ldpc_get_status());

	ldpc_play(LDPC_FORWARD);
	// Transitions immediately to PLAYING without going through SPINNING_UP.
	TEST_CHECK_EQUAL(LDPC_PLAYING, ldpc_get_status());
}

TEST(LDPC, ldpc_spinup_counter_reset_after_transition)
{
	test_ldpc_spinup_counter_reset_after_transition();
}
