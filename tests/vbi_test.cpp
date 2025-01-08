#include "stdafx.h"
#include <ldp-abst/VBICompact.h>
#include <ldp-abst/VBIParse.h>
#include <ldp-abst/VideoStandard.h>

void test_vbi_compact_1()
{
	VBICompactEntry_t entries[1];
	memset(entries, 0, sizeof(entries));
	entries[0].typePattern = PATTERN_22;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 0;
	entries[0].u8PatternOffset = 1;	// start with the second field being dominant

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(VBICompactEntry_t);

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 0);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0);

	VBIC_AddField(1);	// advance by 1

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 1);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80001);

	ul = VBIC_GetCurPictureNum();
	TEST_CHECK(ul == 1);

	VBIC_AddField(1);	// advance by 1

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 2);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0);

	ul = VBIC_GetCurPictureNum();
	TEST_CHECK(ul == 1);
	
	VBIC_AddField(1);	// advance by 1

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 3);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80002);

	ul = VBIC_GetCurPictureNum();
	TEST_CHECK(ul == 2);
}

TEST_CASE(vbi_compact_1)
{
	test_vbi_compact_1();
}

void test_vbi_compact_2()
{
	VBICompactEntry_t entries[1];
	memset(entries, 0, sizeof(entries));
	entries[0].typePattern = PATTERN_22;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 70000;	// test all 24-bits of BCD convertor
	entries[0].u8PatternOffset = 0;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(VBICompactEntry_t);

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 0);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xFF0000);

	VBIC_AddField(1);	// advance by 1

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 1);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0);

	VBIC_AddField(1);	// advance by 1

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 2);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xFF0001);

	VBIC_AddField(1);	// advance by 1

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 3);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0);

}

TEST_CASE(vbi_compact_2)
{
	test_vbi_compact_2();
}

void test_vbi_compact3()
{
	VBICompactEntry_t entries[3];

	entries[0].typePattern = PATTERN_LEADIN;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 0;
	entries[0].u8PatternOffset = 0;

	entries[1].typePattern = PATTERN_22;
	entries[1].u32StartAbsField = 24;
	entries[1].i32StartPictureNumber = 0;	// yes, it starts on frame 0
	entries[1].u8PatternOffset = 0;	// specifically want no offset to test seeking

	entries[2].typePattern = PATTERN_LEADOUT;
	entries[2].u32StartAbsField = 108030;
	entries[2].i32StartPictureNumber = 0;
	entries[2].u8PatternOffset = 0;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 200000;	// high number just to be safe

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	VBIC_SeekResult res = VBIC_SEEK_BUSY;
	res = VBIC_SEEK(54002);

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 108028);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xfd4002);

	VBIC_AddField(2);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 108030);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0x80EEEE);
}

TEST_CASE(vbi_compact3)
{
	test_vbi_compact3();
}

void test_vbi_compact4()
{
	VBICompactEntry_t entries[1];

	entries[0].typePattern = PATTERN_22;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 0;	// yes, it starts on frame 0
	entries[0].u8PatternOffset = 0;	// specifically want no offset to test seeking

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 200000;	// high number just to be safe

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	VBIC_SeekResult res = VBIC_SEEK_BUSY;
	res = VBIC_SEEK(6000);


	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 12000);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xf86000);
}

TEST_CASE(vbi_compact4)
{
	test_vbi_compact4();
}

void test_vbi_compact5_22()
{
	VBICompactEntry_t entries[3];

	entries[0].typePattern = PATTERN_22;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 1;
	entries[0].u8PatternOffset = 0;
	entries[0].u16Special = 0;

	entries[1].typePattern = PATTERN_22;
	entries[1].u32StartAbsField = 2;
	entries[1].i32StartPictureNumber = 1;
	entries[1].u8PatternOffset = 1;
	entries[1].u16Special = 0;

	entries[2].typePattern = PATTERN_22;
	entries[2].u32StartAbsField = 5;
	entries[2].i32StartPictureNumber = 2;
	entries[2].u8PatternOffset = 1;
	entries[2].u16Special = 0;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 200000;	// high number just to be safe

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	VBIC_SeekResult res = VBIC_SEEK_BUSY;
	res = VBIC_SEEK(2);

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 3);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xf80002);

	ul = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(2, ul);
}

TEST_CASE(vbi_compact5_22)
{
	test_vbi_compact5_22();
}

void test_vbi_compact6_mixed()
{
	VBICompactEntry_t entries[3];

	entries[0].typePattern = PATTERN_22;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 1;
	entries[0].u8PatternOffset = 0;
	entries[0].u16Special = 0;

	entries[1].typePattern = PATTERN_22;
	entries[1].u32StartAbsField = 2;
	entries[1].i32StartPictureNumber = 1;
	entries[1].u8PatternOffset = 1;
	entries[1].u16Special = 0;

	entries[2].typePattern = PATTERN_23;
	entries[2].u32StartAbsField = 5;
	entries[2].i32StartPictureNumber = 2;
	entries[2].u8PatternOffset = 1;
	entries[2].u16Special = 0;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 200000;	// high number just to be safe

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	VBIC_SeekResult res = VBIC_SEEK_BUSY;
	res = VBIC_SEEK(2);

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 3);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xf80002);

	ul = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(2, ul);
}

TEST_CASE(vbi_compact6_mixed)
{
	test_vbi_compact6_mixed();
}

void test_vbi_compact7_mixed()
{
	VBICompactEntry_t entries[4];

	entries[0].typePattern = PATTERN_22;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 1;
	entries[0].u8PatternOffset = 0;
	entries[0].u16Special = 0;

	entries[1].typePattern = PATTERN_22;
	entries[1].u32StartAbsField = 2;
	entries[1].i32StartPictureNumber = 1;
	entries[1].u8PatternOffset = 1;
	entries[1].u16Special = 0;

	entries[2].typePattern = PATTERN_23;
	entries[2].u32StartAbsField = 5;
	entries[2].i32StartPictureNumber = 1;
	entries[2].u8PatternOffset = 4;
	entries[2].u16Special = 0;

	entries[3].typePattern = PATTERN_23;
	entries[3].u32StartAbsField = 8;
	entries[3].i32StartPictureNumber = 2;
	entries[3].u8PatternOffset = 4;
	entries[3].u16Special = 0;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 200000;	// high number just to be safe

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	VBIC_SeekResult res = VBIC_SEEK_BUSY;
	res = VBIC_SEEK(2);

	ul = VBIC_GetCurAbsField();
	TEST_REQUIRE_EQUAL(ul,3);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xf80002);

	ul = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(2, ul);

	VBIC_AddField(3);

	ul = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(3, ul);

	VBIC_AddField(3);

	ul = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(4, ul);

}

TEST_CASE(vbi_compact7_mixed)
{
	test_vbi_compact7_mixed();
}

void test_vbi_compact_atari1()
{
	VBICompactEntry_t entries[1];

	entries[0].typePattern = PATTERN_22_ATARI;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 0;
	entries[0].u8PatternOffset = 1;	// start with the second field being dominant

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 200000;	// high number just to be safe

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 0);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xA80000);

	VBIC_AddField(1);	// advance by 1

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 1);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80001);

	VBIC_AddField(1);	// advance by 1

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 2);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xA80001);

	VBIC_AddField(1);	// advance by 1

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 3);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80002);

}

TEST_CASE(vbi_compact_atari1)
{
	test_vbi_compact_atari1();
}

void test_vbi_compact_atari2()
{
	VBICompactEntry_t entries[2];

	entries[0].typePattern = PATTERN_LEADIN;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 0;
	entries[0].u8PatternOffset = 0;

	entries[1].typePattern = PATTERN_22_ATARI;
	entries[1].u32StartAbsField = 2;
	entries[1].i32StartPictureNumber = 0;
	entries[1].u8PatternOffset = 1;	// start with the second field being dominant

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 200000;	// high number just to be safe

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 0);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0x88FFFF);

	VBIC_AddField(1);	// advance by 1

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 1);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0x88FFFF);

	VBIC_AddField(1);	// advance by 1

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 2);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xA80000);

	VBIC_AddField(1);	// advance by 1

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 3);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80001);

}

TEST_CASE(vbi_compact_atari2)
{
	test_vbi_compact_atari2();
}

void test_vbi_compact_atari3()
{
	VBICompactEntry_t entries[3];

	entries[0].typePattern = PATTERN_LEADIN;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 0;
	entries[0].u8PatternOffset = 0;

	entries[1].typePattern = PATTERN_22_ATARI;
	entries[1].u32StartAbsField = 2;
	entries[1].i32StartPictureNumber = 0;
	entries[1].u8PatternOffset = 1;	// start with the second field being dominant

	entries[2].typePattern = PATTERN_LEADOUT;
	entries[2].u32StartAbsField = 4;
	entries[2].i32StartPictureNumber = 0;
	entries[2].u8PatternOffset = 0;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 200000;	// high number just to be safe

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 0);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0x88FFFF);

	VBIC_AddField(1);	// advance by 1

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 1);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0x88FFFF);

	VBIC_AddField(3);	// skip over a few and see if we end up at the right place

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 4);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0x80EEEE);

	VBIC_AddField(-1);	// move back one (over a field boundary)

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 3);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80001);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 4);

	VBIC_AddField(-4);	// back over two boundaries
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 0);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0x88FFFF);
}

TEST_CASE(vbi_compact_atari3)
{
	test_vbi_compact_atari3();
}

void test_vbi_compact_atari4()
{
	VBICompactEntry_t entries[3];

	entries[0].typePattern = PATTERN_LEADIN;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 0;
	entries[0].u8PatternOffset = 0;

	entries[1].typePattern = PATTERN_22_ATARI;
	entries[1].u32StartAbsField = 2;
	entries[1].i32StartPictureNumber = 0;
	entries[1].u8PatternOffset = 1;	// start with the second field being dominant

	entries[2].typePattern = PATTERN_LEADOUT;
	entries[2].u32StartAbsField = 86104;
	entries[2].i32StartPictureNumber = 0;
	entries[2].u8PatternOffset = 0;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 200000;	// high number just to be safe

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	VBIC_SeekResult res = VBIC_SEEK_BUSY;
	res = VBIC_SEEK(43050);

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 86101);

	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xFC3050);

	VBIC_AddField(2);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 86103);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xFC3051);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 86104);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0x80EEEE);
}

TEST_CASE(vbi_compact_atari4)
{
	test_vbi_compact_atari4();
}

void test_vbi_compact23()
{
	VBICompactEntry_t entries[1];

	entries[0].typePattern = PATTERN_23;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 0;
	entries[0].u8PatternOffset = 0;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 200000;	// high number just to be safe

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 0);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80000);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 1);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 2);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80001);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 3);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 4);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 5);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80002);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 6);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0);

	VBIC_SEEK(4);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 10);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80004);

	VBIC_SEEK(5);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 12);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80005);

	VBIC_SEEK(6);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 15);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80006);

}

TEST_CASE(vbi_compact23)
{
	test_vbi_compact23();
}

void test_vbi_compact23_ace()
{
	VBICompactEntry_t entries[2];

	entries[0].typePattern = PATTERN_23;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = -1;
	entries[0].u8PatternOffset = 4;

	entries[1].typePattern = PATTERN_LEADOUT;
	entries[1].u32StartAbsField = (47272 << 1) + 1;
	entries[1].i32StartPictureNumber = 0;
	entries[1].u8PatternOffset = 0;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 200000;	// high number just to be safe

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 0);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 1);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80001);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 2);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 3);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80002);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 4);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 5);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 6);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80003);

	VBIC_SEEK(37818);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == ((47271 * 2) + 1));
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xFB7818);

	VBIC_AddField(2);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == ((47272 * 2) + 1));
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0x80EEEE);
}

TEST_CASE(vbi_compact23_ace)
{
	test_vbi_compact23_ace();
}

void test_vbi_compact_picnum()
{
	VBICompactEntry_t entries[3];
	memset(entries, 0, sizeof(entries));

	entries[0].typePattern = PATTERN_LEADIN;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 0;
	entries[0].u8PatternOffset = 0;

	entries[1].typePattern = PATTERN_PICNUM;
	entries[1].u32StartAbsField = 1;
	entries[1].i32StartPictureNumber = 1;
	entries[1].u8PatternOffset = 0;

	entries[2].typePattern = PATTERN_LEADOUT;
	entries[2].u32StartAbsField = 60000;
	entries[2].i32StartPictureNumber = 0;
	entries[2].u8PatternOffset = 0;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 200000;	// high number just to be safe

	VBIC_Init(&vbi);

	unsigned long ul = 0;

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 0);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 1);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80001);

	VBIC_AddField(1);
	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 2);
	ul = VBIC_GetCurFieldLine18();
	TEST_CHECK(ul == 0xF80001);

	VBIC_SeekResult res = VBIC_SEEK(1);
	TEST_CHECK_EQUAL(VBIC_SEEK_SUCCESS, res);

	ul = VBIC_GetCurAbsField();
	TEST_CHECK(ul == 1);
}

TEST_CASE(vbi_compact_picnum)
{
	test_vbi_compact_picnum();
}

void test_vbi_compact_zeroes()
{
	VBICompactEntry_t entries[3];

	entries[0].typePattern = PATTERN_22;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 0;
	entries[0].u8PatternOffset = 0;

	entries[1].typePattern = PATTERN_ZEROES;
	entries[1].u32StartAbsField = 3;
	entries[1].i32StartPictureNumber = 0;	// start picture number does not apply for a ZEROES pattern
	entries[1].u8PatternOffset = 0;

	entries[2].typePattern = PATTERN_22;
	entries[2].u32StartAbsField = 4;
	entries[2].i32StartPictureNumber = 2;
	entries[2].u8PatternOffset = 0;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 200000;	// high number just to be safe

	VBIC_Init(&vbi);

	VBIC_SeekResult res = VBIC_SEEK_BUSY;
	res = VBIC_SEEK(1);

	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	// now change the first entry to be ZEROES also to check for proper error handling
	entries[0].typePattern = PATTERN_ZEROES;

	VBIC_Init(&vbi);

	res = VBIC_SEEK(1);
	TEST_CHECK(res == VBIC_SEEK_FAIL);
}

TEST_CASE(vbi_compact_zeroes)
{
	test_vbi_compact_zeroes();
}

void test_vbi_compact_out_of_range_23(uint32_t u32TotalFields, int32_t i32StartPictureNumberVbi, uint8_t u8PatternOffset,
									  uint32_t u32FirstValidPicNum, uint32_t u32FirstValidPicNumField,
											   uint32_t u32MaxValidPicNum, uint32_t u32AbsFieldOfMaxValidPicNum)
{
	VBICompactEntry_t entries[1];

	entries[0].typePattern = PATTERN_23;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = i32StartPictureNumberVbi;
	entries[0].u8PatternOffset = u8PatternOffset;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = u32TotalFields;

	VBIC_Init(&vbi);

	// seek before first picture number
	VBIC_SeekResult res = VBIC_SEEK_BUSY;
	res = VBIC_SEEK(u32FirstValidPicNum - 1);

	// should succeed and land on picture number 1
	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	uint32_t u32PicNum = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(u32FirstValidPicNum, u32PicNum);
	uint32_t u32AbsField = VBIC_GetCurAbsField();
	TEST_CHECK_EQUAL(u32FirstValidPicNumField, u32AbsField);

	// seek to the maximum valid frame
	res = VBIC_SEEK(u32MaxValidPicNum);

	// should succeed
	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	u32PicNum = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(u32MaxValidPicNum, u32PicNum);
	u32AbsField = VBIC_GetCurAbsField();
	TEST_CHECK_EQUAL(u32AbsFieldOfMaxValidPicNum, u32AbsField);

	// now seek out of range going the other direction
	res = VBIC_SEEK(u32MaxValidPicNum + 1);

	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	u32PicNum = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(u32MaxValidPicNum, u32PicNum);
	u32AbsField = VBIC_GetCurAbsField();
	TEST_CHECK_EQUAL(u32AbsFieldOfMaxValidPicNum, u32AbsField);
}

TEST_CASE(vbi_compact_out_of_range_23_noOffset)
{
	test_vbi_compact_out_of_range_23(10, 1, 0, 1, 0, 4, 7);
	test_vbi_compact_out_of_range_23(9, 1, 0, 1, 0, 4, 7);
	test_vbi_compact_out_of_range_23(8, 1, 0, 1, 0, 4, 7);
	test_vbi_compact_out_of_range_23(7, 1, 0, 1, 0, 3, 5);
	test_vbi_compact_out_of_range_23(6, 1, 0, 1, 0, 3, 5);
}

TEST_CASE(vbi_compact_out_of_range_23_withOffset)
{
	test_vbi_compact_out_of_range_23(10, 1, 1, 2, 1, 5, 9);
	test_vbi_compact_out_of_range_23(10, 1, 2, 2, 0, 5, 8);
	test_vbi_compact_out_of_range_23(10, 1, 3, 3, 2, 6, 9);
	test_vbi_compact_out_of_range_23(10, 1, 4, 3, 1, 6, 8);
}

void test_vbi_compact_out_of_range_22_1Offset()
{
	VBICompactEntry_t entries[1];

	entries[0].typePattern = PATTERN_22;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 1;
	entries[0].u8PatternOffset = 1;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 60000;	// want picture numbers from 2 to 30001

	VBIC_Init(&vbi);

	// seek before first picture number
	VBIC_SeekResult res = VBIC_SEEK_BUSY;
	res = VBIC_SEEK(1);

	// should succeed and land on picture number 2
	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	uint32_t u32PicNum = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(2, u32PicNum);
	uint32_t u32AbsField = VBIC_GetCurAbsField();
	TEST_CHECK_EQUAL(1, u32AbsField);

	// seek to the maximum valid frame
	res = VBIC_SEEK(30001);

	// should succeed and land on picture number 30000
	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	u32PicNum = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(30001, u32PicNum);
	u32AbsField = VBIC_GetCurAbsField();
	TEST_CHECK_EQUAL(59999, u32AbsField);

	// now seek out of range
	res = VBIC_SEEK(30002);

	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	u32PicNum = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(30001, u32PicNum);
	u32AbsField = VBIC_GetCurAbsField();
	TEST_CHECK_EQUAL(59999, u32AbsField);
}

TEST_CASE(vbi_compact_out_of_range_22_1Offset)
{
	test_vbi_compact_out_of_range_22_1Offset();
}

void test_vbi_compact_out_of_range_22_noOffset_leadout()
{
	VBICompactEntry_t entries[2];

	entries[0].typePattern = PATTERN_22;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 1;
	entries[0].u8PatternOffset = 0;

	entries[1].typePattern = PATTERN_LEADOUT;
	entries[1].u32StartAbsField = 60000;
	entries[1].i32StartPictureNumber = 0;
	entries[1].u8PatternOffset = 0;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 60001;	// want picture numbers from 1 to 30000

	VBIC_Init(&vbi);

	// seek before first picture number
	VBIC_SeekResult res = VBIC_SEEK_BUSY;
	res = VBIC_SEEK(0);

	// should succeed and land on picture number 1
	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	uint32_t u32PicNum = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(1, u32PicNum);
	uint32_t u32AbsField = VBIC_GetCurAbsField();
	TEST_CHECK_EQUAL(0, u32AbsField);

	// seek to the maximum valid frame
	res = VBIC_SEEK(30000);

	// should succeed and land on picture number 30000
	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	u32PicNum = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(30000, u32PicNum);
	u32AbsField = VBIC_GetCurAbsField();
	TEST_CHECK_EQUAL(59998, u32AbsField);

	// now seek out of range going the other direction
	res = VBIC_SEEK(30001);

	// should succeed and land on picture number 30000
	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	u32PicNum = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(30000, u32PicNum);
	u32AbsField = VBIC_GetCurAbsField();
	TEST_CHECK_EQUAL(59998, u32AbsField);
}

TEST_CASE(vbi_compact_out_of_range_22_noOffset_leadout)
{
	test_vbi_compact_out_of_range_22_noOffset_leadout();
}

void test_vbi_compact_out_of_range_22_1Offset_leadout()
{
	VBICompactEntry_t entries[2];

	entries[0].typePattern = PATTERN_22;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 1;
	entries[0].u8PatternOffset = 1;

	entries[1].typePattern = PATTERN_LEADOUT;
	entries[1].u32StartAbsField = 60000;
	entries[1].i32StartPictureNumber = 0;
	entries[1].u8PatternOffset = 0;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 60000;	// want picture numbers from 2 to 30001

	VBIC_Init(&vbi);

	// seek before first picture number
	VBIC_SeekResult res = VBIC_SEEK_BUSY;
	res = VBIC_SEEK(1);

	// should succeed and land on picture number 2
	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	uint32_t u32PicNum = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(2, u32PicNum);
	uint32_t u32AbsField = VBIC_GetCurAbsField();
	TEST_CHECK_EQUAL(1, u32AbsField);

	// seek to the maximum valid frame
	res = VBIC_SEEK(30001);

	// should succeed and land on picture number 30000
	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	u32PicNum = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(30001, u32PicNum);
	u32AbsField = VBIC_GetCurAbsField();
	TEST_CHECK_EQUAL(59999, u32AbsField);

	// now seek out of range
	res = VBIC_SEEK(30002);

	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	u32PicNum = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(30001, u32PicNum);
	u32AbsField = VBIC_GetCurAbsField();
	TEST_CHECK_EQUAL(59999, u32AbsField);
}

TEST_CASE(vbi_compact_out_of_range_22_1Offset_leadout)
{
	test_vbi_compact_out_of_range_22_1Offset_leadout();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void test_vbi_compactor23()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	pVBI->AddVBIData(vbi);
	vbi.uVBI[VBIParse::LINE17] = vbi.uVBI[VBIParse::LINE18] = 0x80EEEE;	// lead-out
	pVBI->AddVBIData(vbi);

	std::list<VBICompactEntry_t> lstEntries;
	bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	TEST_CHECK_EQUAL(2, lstEntries.size());
}

TEST_CASE(vbi_compactor23)
{
	test_vbi_compactor23();
}

void test_vbi_compactor23_offset1()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBILeadIn();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(3, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	TEST_CHECK_EQUAL(2, lstEntries.size());

	VBICompactEntry_t entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_LEADIN, entry.typePattern);
	TEST_CHECK_EQUAL(0, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_23, entry.typePattern);
	TEST_CHECK_EQUAL(1, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 1);
}

TEST_CASE(vbi_compactor23_offset1)
{
	test_vbi_compactor23_offset1();
}

void test_vbi_compactor23_offset2()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBILeadIn();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(3, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	TEST_CHECK_EQUAL(2, lstEntries.size());

	VBICompactEntry_t entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_LEADIN, entry.typePattern);
	TEST_CHECK_EQUAL(0, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_23, entry.typePattern);
	TEST_CHECK_EQUAL(1, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 2);
}

TEST_CASE(vbi_compactor23_offset2)
{
	test_vbi_compactor23_offset2();
}

void test_vbi_compactor23_offset3()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBILeadIn();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(3, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	TEST_CHECK_EQUAL(2, lstEntries.size());

	VBICompactEntry_t entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_LEADIN, entry.typePattern);
	TEST_CHECK_EQUAL(0, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_23, entry.typePattern);
	TEST_CHECK_EQUAL(1, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 3);
}

TEST_CASE(vbi_compactor23_offset3)
{
	test_vbi_compactor23_offset3();
}

void test_vbi_compactor23_offset4()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(3, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	TEST_REQUIRE_EQUAL(1, lstEntries.size());

	VBICompactEntry_t entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_23, entry.typePattern);
	TEST_CHECK_EQUAL(0, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 4);
}

TEST_CASE(vbi_compactor23_offset4)
{
	test_vbi_compactor23_offset4();
}

void test_vbi_compactor22_atari()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi.uVBI[VBIParse::LINE17] = vbi.uVBI[VBIParse::LINE18] = 0x88FFFF;	// lead-in
	pVBI->AddVBIData(vbi);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(0, NTSC);
	vbi.uVBI[VBIParse::LINE18] &= 0xAFFFFF;
	vbi.uVBI[VBIParse::LINE17] = vbi.uVBI[VBIParse::LINE18];
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	pVBI->AddVBIData(vbi);
	vbi.uVBI[VBIParse::LINE18] &= 0xAFFFFF;
	vbi.uVBI[VBIParse::LINE17] = vbi.uVBI[VBIParse::LINE18];
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi.uVBI[VBIParse::LINE17] = vbi.uVBI[VBIParse::LINE18] = 0x80EEEE;	// lead-out
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	TEST_CHECK_EQUAL(3, lstEntries.size());

	VBICompactEntry_t entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_LEADIN, entry.typePattern);
	TEST_CHECK_EQUAL(0, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_22_ATARI, entry.typePattern);
	TEST_CHECK_EQUAL(2, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 1);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_LEADOUT, entry.typePattern);
	TEST_CHECK_EQUAL(6, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);
}

TEST_CASE(vbi_compactor22_atari)
{
	test_vbi_compactor22_atari();
}

void test_vbi_compactor22_offset()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBILeadIn();
	pVBI->AddVBIData(vbi);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBILeadOut();
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	TEST_REQUIRE_EQUAL(3, lstEntries.size());

	VBICompactEntry_t entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_LEADIN, entry.typePattern);
	TEST_CHECK_EQUAL(0, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_22, entry.typePattern);
	TEST_CHECK_EQUAL(2, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 1);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_LEADOUT, entry.typePattern);
	TEST_CHECK_EQUAL(6, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);
}

TEST_CASE(vbi_compactor22_offset)
{
	test_vbi_compactor22_offset();
}

void test_vbi_compactor22_offset2()
{
	VBI_t vbi;
	bool bRes;

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
	bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	TEST_REQUIRE_EQUAL(3, lstEntries.size());

	VBICompactEntry_t entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(1, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_22, entry.typePattern);
	TEST_CHECK_EQUAL(0, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_ZEROES, entry.typePattern);
	TEST_CHECK_EQUAL(2, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(2, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_PICNUM, entry.typePattern);
	TEST_CHECK_EQUAL(3, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);

}

TEST_CASE(vbi_compactor22_offset2)
{
	test_vbi_compactor22_offset2();
}

void test_vbi_compactor22_chapter()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBILeadIn();
	pVBI->AddVBIData(vbi);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);	// putting the chapter here is non-standard but I want to make sure it is handled properly
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBILeadOut();
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	TEST_REQUIRE_EQUAL(3, lstEntries.size());

	VBICompactEntry_t entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_LEADIN, entry.typePattern);
	TEST_CHECK_EQUAL(0, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_22, entry.typePattern);
	TEST_CHECK_EQUAL(2, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 1);

	uint8_t uChapter = 0;
	VBIC_BOOL bChapterSet = VBIC_GetChapterInfo(&entry, &uChapter);
	TEST_CHECK(bChapterSet != 0);
	TEST_CHECK_EQUAL(1, uChapter);


	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_LEADOUT, entry.typePattern);
	TEST_CHECK_EQUAL(7, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);
}

TEST_CASE(vbi_compactor22_chapter)
{
	test_vbi_compactor22_chapter();
}

void test_vbi_compactor22_chapter2()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBILeadIn();
	pVBI->AddVBIData(vbi);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();	// test to make sure the pattern parser gets this right
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBILeadOut();
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	TEST_REQUIRE_EQUAL(4, lstEntries.size());

	VBICompactEntry_t entry;

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_LEADIN, entry.typePattern);
	TEST_CHECK_EQUAL(0, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_ZEROES, entry.typePattern);
	TEST_CHECK_EQUAL(2, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(1, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_22, entry.typePattern);
	TEST_CHECK_EQUAL(3, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);
	uint8_t uChapter = 0;
	VBIC_BOOL bChapterSet = VBIC_GetChapterInfo(&entry, &uChapter);
	TEST_CHECK(bChapterSet != 0);
	TEST_CHECK_EQUAL(1, uChapter);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_LEADOUT, entry.typePattern);
	TEST_CHECK_EQUAL(7, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);
}

TEST_CASE(vbi_compactor22_chapter2)
{
	test_vbi_compactor22_chapter2();
}

void test_vbi_compactor22_chapter3()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(3, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(4, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(5, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(6, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(7, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	TEST_CHECK_EQUAL(1, lstEntries.size());
}

TEST_CASE(vbi_compactor22_chapter3)
{
	test_vbi_compactor22_chapter3();
}

void test_vbi_compactor22_stopcode()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBILeadIn();
	pVBI->AddVBIData(vbi);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(3, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIStopCode();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBILeadOut();
	pVBI->AddVBIData(vbi);
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	TEST_REQUIRE_EQUAL(3, lstEntries.size());

	VBICompactEntry_t entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_LEADIN, entry.typePattern);
	TEST_CHECK_EQUAL(0, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(1, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_22, entry.typePattern);
	TEST_CHECK_EQUAL(2, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);
	TEST_CHECK_EQUAL(entry.u16Special, 0x0);

	entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(0, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_LEADOUT, entry.typePattern);
	TEST_CHECK_EQUAL(8, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 0);
}

TEST_CASE(vbi_compactor22_stopcode)
{
	test_vbi_compactor22_stopcode();
}

void test_vbi_compactor22_nonstandard1()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);

	// strip off "stop code" bit
	vbi.uVBI[VBIParse::LINE17] &= 0xF7FFFF;
	vbi.uVBI[VBIParse::LINE18] = vbi.uVBI[VBIParse::LINE17];

	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(3, NTSC);

	// strip off "stop code" bit
	vbi.uVBI[VBIParse::LINE17] &= 0xF7FFFF;
	vbi.uVBI[VBIParse::LINE18] = vbi.uVBI[VBIParse::LINE17];
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_REQUIRE(bRes);

	TEST_REQUIRE_EQUAL(1, lstEntries.size());

	VBICompactEntry_t entry = lstEntries.front();
	lstEntries.pop_front();
	TEST_CHECK_EQUAL(1, entry.i32StartPictureNumber);
	TEST_CHECK_EQUAL(PATTERN_22, entry.typePattern);
	TEST_CHECK_EQUAL(0, entry.u32StartAbsField);
	TEST_CHECK_EQUAL(entry.u8PatternOffset, 1);
	TEST_CHECK_EQUAL(entry.u16Special, 0x0);
}

TEST_CASE(vbi_compactor22_nonstandard1)
{
	test_vbi_compactor22_nonstandard1();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_vbi_autofix1()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBIFrame(1, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(3, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(4, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(5, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);
	vbi.uVBI[VBIParse::LINE17] = VBIParse::PARSE_FAILED;	// this is supposed to be frame 6
	vbi.uVBI[VBIParse::LINE18] = VBIParse::PARSE_FAILED;
	pVBI->AddVBIData(vbi);
	// do another error again, we should be able to figure out the right value here based on the pattern
	pVBI->AddVBIData(vbi);	// should be chapter 1
	vbi = VBIParse::GenerateVBIFrame(7, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(1);
	pVBI->AddVBIData(vbi);

	bRes = pVBI->VerifyVBIData(NTSC, true);
	TEST_CHECK(bRes);

	pVBI->GetVBIData(vbi, 10);
	TEST_CHECK_EQUAL(0xF80006, vbi.uVBI[VBIParse::LINE18]);
	pVBI->GetVBIData(vbi, 11);
	TEST_CHECK_EQUAL(0x881DDD, vbi.uVBI[VBIParse::LINE18]);
}

TEST_CASE(vbi_autofix1)
{
	test_vbi_autofix1();
}

void test_vbi_autofix2()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBIFrame(20454, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(20455, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(20456, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(20457, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(20458, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	vbi.uVBI[VBIParse::LINE17] = 0;
	vbi.uVBI[VBIParse::LINE18] |= 0xF00000;	// simulate an actual parse error I experienced, where the chapter number got corrupted and looked like a frame number
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(20459, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(20460, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	pVBI->AddVBIData(vbi);

	bRes = pVBI->VerifyVBIData(NTSC, true);
	TEST_REQUIRE(bRes);

	pVBI->GetVBIData(vbi, 9);
	TEST_CHECK_EQUAL(0x880DDD, vbi.uVBI[VBIParse::LINE18]);
}

TEST_CASE(vbi_autofix2)
{
	test_vbi_autofix2();
}

void test_vbi_autofix3()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	// THIS TEST IS BASED OFF OF A REAL DEFECT I FOUND WITH REAL DATA

	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2312, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2313, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2314, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2315, NTSC);
	vbi.uVBI[VBIParse::LINE17] = 0xF82317;	// line17 is wrong, line18 is correct
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2316, NTSC);
	vbi.uVBI[VBIParse::LINE17] = 0xF82317;	// line17 is wrong, line18 is correct
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2317, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2318, NTSC);
	pVBI->AddVBIData(vbi);

	bRes = pVBI->VerifyVBIData(NTSC, true);
	TEST_REQUIRE(bRes);

	pVBI->GetVBIData(vbi, 9);
	TEST_CHECK_EQUAL(0xF82315, vbi.uVBI[VBIParse::LINE17]);
	TEST_CHECK_EQUAL(0xF82315, vbi.uVBI[VBIParse::LINE18]);

	pVBI->GetVBIData(vbi, 11);
	TEST_CHECK_EQUAL(0xF82316, vbi.uVBI[VBIParse::LINE17]);
	TEST_CHECK_EQUAL(0xF82316, vbi.uVBI[VBIParse::LINE18]);

}

TEST_CASE(vbi_autofix3)
{
	test_vbi_autofix3();
}

void test_vbi_autofix4()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	// THIS TEST IS A MODIFIED VERSION OF THE PREVIOUS TEST

	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2312, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2313, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2314, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2315, NTSC);
	vbi.uVBI[VBIParse::LINE17] = 0xF82317;	// both lines are wrong
	vbi.uVBI[VBIParse::LINE18] = 0xF82318;	// 
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2316, NTSC);
	vbi.uVBI[VBIParse::LINE17] = 0xF82317;	// both lines are wrong
	vbi.uVBI[VBIParse::LINE18] = 0xF82318;	//
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2317, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2318, NTSC);
	pVBI->AddVBIData(vbi);

	bRes = pVBI->VerifyVBIData(NTSC, true);
	TEST_REQUIRE(!bRes);
}

TEST_CASE(vbi_autofix4)
{
	test_vbi_autofix4();
}

void test_vbi_autofix5()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	// THIS TEST IS A MODIFIED VERSION OF THE PREVIOUS TEST

	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2312, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2313, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2314, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2315, NTSC);
	vbi.uVBI[VBIParse::LINE18] = 0xF82318;	// line 18 is wrong, 17 is ok
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2316, NTSC);
	vbi.uVBI[VBIParse::LINE18] = 0xF82318;	// line 18 is wrong, 17 is ok
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2317, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIEmpty();
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(2318, NTSC);
	pVBI->AddVBIData(vbi);

	bRes = pVBI->VerifyVBIData(NTSC, true);
	TEST_REQUIRE(bRes);

	pVBI->GetVBIData(vbi, 9);
	TEST_CHECK_EQUAL(0xF82315, vbi.uVBI[VBIParse::LINE17]);
	TEST_CHECK_EQUAL(0xF82315, vbi.uVBI[VBIParse::LINE18]);

	pVBI->GetVBIData(vbi, 11);
	TEST_CHECK_EQUAL(0xF82316, vbi.uVBI[VBIParse::LINE17]);
	TEST_CHECK_EQUAL(0xF82316, vbi.uVBI[VBIParse::LINE18]);

}

TEST_CASE(vbi_autofix5)
{
	test_vbi_autofix5();
}

void test_vbi_autofix_chapter()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;


	vbi = VBIParse::GenerateVBIFrame(136, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(137, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	vbi.uVBI[VBIParse::LINE17] = 0;
	vbi.uVBI[VBIParse::LINE18] = 0x7fffff;	// parse error
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(138, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	pVBI->AddVBIData(vbi);

	bRes = pVBI->VerifyVBIData(NTSC, true);
	TEST_REQUIRE(bRes);

	// make sure that the parse error got fixed to be a chapter
	pVBI->GetVBIData(vbi, 3);
	TEST_CHECK_EQUAL(0x0, vbi.uVBI[VBIParse::LINE17]);
	TEST_CHECK_EQUAL(0x880ddd, vbi.uVBI[VBIParse::LINE18]);
}

TEST_CASE(vbi_autofix_chapter)
{
	test_vbi_autofix_chapter();
}

void test_vbi_buffer1()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBIFrame(20454, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(20455, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	pVBI->CompactVBIData(lstEntries, NTSC);

	// our test only is set up for 1 entry
	TEST_REQUIRE(lstEntries.size() == 1);

	VBICompactEntry_t entries[1];
	VBICompact_t compact;

	compact.uEntryCount = 1;
	compact.pEntries = entries;
	compact.uTotalFields = 5678;

	entries[0] = lstEntries.front();

	unsigned char buf[80];

	size_t stRes = VBIC_ToBuffer(buf, sizeof(buf), &compact);
	TEST_REQUIRE_EQUAL(18, stRes);

	// now convert it back and compare the results

	memset(entries, 0, sizeof(entries));
	compact.uEntryCount = 0;
	compact.uTotalFields = 0;

	VBIC_BOOL res;
	res = VBIC_FromBuffer(&compact, 0, buf, stRes);

	TEST_REQUIRE(!res);	// should fail if we set the max size too low

	// now try again with the proper max size
	res = VBIC_FromBuffer(&compact, 1, buf, stRes);

	TEST_REQUIRE(res != 0);

	// compare the results to see if we got it right
	VBICompactEntry_t correct = lstEntries.front();

	TEST_CHECK_EQUAL(correct.i32StartPictureNumber, entries[0].i32StartPictureNumber);
	TEST_CHECK_EQUAL(correct.typePattern, entries[0].typePattern);
	TEST_CHECK_EQUAL(correct.u32StartAbsField, entries[0].u32StartAbsField);
	TEST_CHECK_EQUAL(correct.u16Special, entries[0].u16Special);
	TEST_CHECK_EQUAL(correct.u8PatternOffset, entries[0].u8PatternOffset);

	TEST_CHECK_EQUAL(1, compact.uEntryCount);
	TEST_CHECK_EQUAL(5678, compact.uTotalFields);
}

TEST_CASE(vbi_buffer1)
{
	test_vbi_buffer1();
}

void test_set_vbi_invalid()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	vbi = VBIParse::GenerateVBIFrame(20454, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIFrame(20455, NTSC);
	pVBI->AddVBIData(vbi);
	vbi = VBIParse::GenerateVBIChapter(0);
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	pVBI->CompactVBIData(lstEntries, NTSC);

	VBICompactEntry_t entries[1];
	VBICompact_t compact;

	compact.uEntryCount = 1;
	compact.pEntries = entries;
	compact.uTotalFields = 5678;

	entries[0] = lstEntries.front();

	VBIC_Init(&compact);

	// try setting a field that is invalid
	VBIC_BOOL b = VBIC_SetField(-1);

	TEST_REQUIRE(b == VBIC_FALSE);
}

TEST_CASE(set_vbi_invalid)
{
	test_set_vbi_invalid();
}

void test_set_loadline_invalid()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;
	bool bRes = false;

	// using lead-out intentionally because of an actual Dexter defect
	vbi = VBIParse::GenerateVBILeadOut();
	pVBI->AddVBIData(vbi);

	list<VBICompactEntry_t> lstEntries;
	pVBI->CompactVBIData(lstEntries, NTSC);

	VBICompactEntry_t entries[1];
	VBICompact_t compact;

	compact.uEntryCount = 1;
	compact.pEntries = entries;
	compact.uTotalFields = 5678;

	entries[0] = lstEntries.front();

	VBIC_Init(&compact);

	// try setting a negative field
	VBIC_LoadLine18(&compact.pEntries[0], -1);

	// if we don't lock-up, test has passed
}

TEST_CASE(set_loadline_invalid)
{
	test_set_loadline_invalid();
}

void test_vbi_frame_conversion()
{
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	unsigned int u = 0;

	// this test covers a specific defect.  I had the picture number conversion methods wrong.

	u = VBIParse::DecimalToPictureNumVBI(79999, NTSC);
	TEST_CHECK_EQUAL(0xff9999, u);

	u = VBIParse::PictureNumVBIToDecimal(0xff9999, NTSC);
	TEST_CHECK_EQUAL(79999, u);

	u = VBIParse::DecimalToPictureNumVBI(99999, PAL);
	TEST_CHECK_EQUAL(0xf99999, u);

	u = VBIParse::PictureNumVBIToDecimal(0xf99999, PAL);
	TEST_CHECK_EQUAL(99999, u);

}

TEST_CASE(vbi_frame_conversion)
{
	test_vbi_frame_conversion();
}
