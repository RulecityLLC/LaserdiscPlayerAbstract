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
