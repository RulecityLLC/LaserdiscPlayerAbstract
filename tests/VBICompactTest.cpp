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

TEST(VBICompact, vbi_compact_1)
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

TEST(VBICompact, vbi_compact_2)
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

TEST(VBICompact, vbi_compact3)
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

TEST(VBICompact, vbi_compact4)
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

TEST(VBICompact, vbi_compact5_22)
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

TEST(VBICompact, vbi_compact6_mixed)
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

TEST(VBICompact, vbi_compact7_mixed)
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

TEST(VBICompact, vbi_compact_atari1)
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

TEST(VBICompact, vbi_compact_atari2)
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

TEST(VBICompact, vbi_compact_atari3)
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

TEST(VBICompact, vbi_compact_atari4)
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

TEST(VBICompact, vbi_compact23)
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

TEST(VBICompact, vbi_compact23_ace)
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

TEST(VBICompact, vbi_compact_picnum)
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

TEST(VBICompact, vbi_compact_zeroes)
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

TEST(VBICompact, vbi_compact_out_of_range_23_noOffset)
{
	test_vbi_compact_out_of_range_23(10, 1, 0, 1, 0, 4, 7);
	test_vbi_compact_out_of_range_23(9, 1, 0, 1, 0, 4, 7);
	test_vbi_compact_out_of_range_23(8, 1, 0, 1, 0, 4, 7);
	test_vbi_compact_out_of_range_23(7, 1, 0, 1, 0, 3, 5);
	test_vbi_compact_out_of_range_23(6, 1, 0, 1, 0, 3, 5);
}

TEST(VBICompact, vbi_compact_out_of_range_23_withOffset)
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

TEST(VBICompact, vbi_compact_out_of_range_22_1Offset)
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

TEST(VBICompact, vbi_compact_out_of_range_22_noOffset_leadout)
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

TEST(VBICompact, vbi_compact_out_of_range_22_1Offset_leadout)
{
	test_vbi_compact_out_of_range_22_1Offset_leadout();
}

void test_vbi_compact_tq_hal_side1()
{
	VBICompactEntry_t entries[2];

	entries[0].typePattern = PATTERN_LEADIN;
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 0;
	entries[0].u8PatternOffset = 0;

	entries[1].typePattern = PATTERN_23;
	entries[1].u32StartAbsField = 1;
	entries[1].i32StartPictureNumber = 0;
	entries[1].u8PatternOffset = 2;

	VBICompact_t vbi;
	vbi.pEntries = entries;
	vbi.uEntryCount = sizeof(entries) / sizeof(entries[0]);
	vbi.uTotalFields = 1000;	// arbitrary

	VBIC_Init(&vbi);

	// actual defect: seeking to picture number 1 was returning an error
	VBIC_SeekResult res = VBIC_SEEK_BUSY;
	res = VBIC_SEEK(1);

	// should succeed and land on picture number 1
	TEST_CHECK(res == VBIC_SEEK_SUCCESS);

	uint32_t u32PicNum = VBIC_GetCurPictureNum();
	TEST_CHECK_EQUAL(1, u32PicNum);
	uint32_t u32AbsField = VBIC_GetCurAbsField();
	TEST_CHECK_EQUAL(1, u32AbsField);
}

TEST(VBICompact, vbi_compact_tq_hal_side1)
{
	test_vbi_compact_tq_hal_side1();
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

TEST(VBICompact, vbi_compactor23)
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

TEST(VBICompact, vbi_compactor23_offset1)
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

TEST(VBICompact, vbi_compactor23_offset2)
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

TEST(VBICompact, vbi_compactor23_offset3)
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

TEST(VBICompact, vbi_compactor23_offset4)
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

TEST(VBICompact, vbi_compactor22_atari)
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

TEST(VBICompact, vbi_compactor22_offset)
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

TEST(VBICompact, vbi_compactor22_offset2)
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

TEST(VBICompact, vbi_compactor22_chapter)
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

TEST(VBICompact, vbi_compactor22_chapter2)
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

TEST(VBICompact, vbi_compactor22_chapter3)
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

TEST(VBICompact, vbi_compactor22_stopcode)
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

TEST(VBICompact, vbi_compactor22_nonstandard1)
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

TEST(VBICompact, vbi_autofix1)
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

TEST(VBICompact, vbi_autofix2)
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

TEST(VBICompact, vbi_autofix3)
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

TEST(VBICompact, vbi_autofix4)
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

TEST(VBICompact, vbi_autofix5)
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

TEST(VBICompact, vbi_autofix_chapter)
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

	// now try again with the proper max size — pin the exact success value so
	// the VBIC_TRUE assignment can't be replaced with 42 (both would satisfy != 0)
	res = VBIC_FromBuffer(&compact, 1, buf, stRes);

	TEST_REQUIRE_EQUAL(VBIC_TRUE, res);

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

TEST(VBICompact, vbi_buffer1)
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

TEST(VBICompact, set_vbi_invalid)
{
	test_set_vbi_invalid();
}

void test_set_vbi_boundary()
{
	// Verifies the range check in VBIC_SetField uses >= (not >), so the first
	// out-of-range field (uAbsoluteField == uTotalFields) is rejected.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();
	VBI_t vbi;

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

	// last valid field (uTotalFields - 1) must be accepted
	VBIC_BOOL bInside = VBIC_SetField(compact.uTotalFields - 1);
	TEST_REQUIRE(bInside == VBIC_TRUE);

	// exact boundary (uAbsoluteField == uTotalFields) must be rejected
	VBIC_BOOL bAt = VBIC_SetField(compact.uTotalFields);
	TEST_REQUIRE(bAt == VBIC_FALSE);
}

TEST(VBICompact, set_vbi_boundary)
{
	test_set_vbi_boundary();
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

TEST(VBICompact, set_loadline_invalid)
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

TEST(VBICompact, vbi_frame_conversion)
{
	test_vbi_frame_conversion();
}

void test_vbic_chapter_info()
{
	// Exercises VBIC_GetChapterInfo / VBIC_SetChapterInfo / VBIC_ClearChapterInfo
	// directly with exact-value comparisons so cxx_assign_const and
	// cxx_remove_void_call mutations on these helpers can be detected.
	VBICompactEntry_t entry;
	entry.u16Special = 0;

	uint8_t uChapter = 99;
	VBIC_BOOL b = VBIC_GetChapterInfo(&entry, &uChapter);
	TEST_CHECK_EQUAL(VBIC_FALSE, b);	// no chapter flag yet -> must be exactly 0

	// Pre-load a DIFFERENT chapter bit pattern so that if VBIC_SetChapterInfo
	// skips the internal VBIC_ClearChapterInfo call, the stale bits will leak
	// through and produce the wrong chapter number.
	entry.u16Special = 0x7F;	// stale "chapter 127" bits, no EVENT_CHAPTER flag

	VBIC_SetChapterInfo(&entry, 3);

	b = VBIC_GetChapterInfo(&entry, &uChapter);
	TEST_CHECK(b == VBIC_TRUE);	// exact-value comparison (42 != 1)
	TEST_CHECK_EQUAL(3, uChapter);	// would be 0x7F if ClearChapterInfo was skipped

	VBIC_ClearChapterInfo(&entry);
	b = VBIC_GetChapterInfo(&entry, &uChapter);
	TEST_CHECK_EQUAL(VBIC_FALSE, b);
}

TEST(VBICompact, vbic_chapter_info)
{
	test_vbic_chapter_info();
}

void test_vbic_buffer_too_small()
{
	// VBIC_ToBuffer must return 0 when the destination buffer can't fit the
	// serialized entries. This pins the `stRes = 0` initializer (otherwise the
	// mutated `stRes = 42` slips through for the too-small path).
	VBICompactEntry_t entries[1];
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 0;
	entries[0].typePattern = PATTERN_22;
	entries[0].u16Special = 0;
	entries[0].u8PatternOffset = 0;

	VBICompact_t compact;
	compact.uEntryCount = 1;
	compact.pEntries = entries;
	compact.uTotalFields = 100;

	// needs 6 + (1 * 12) = 18 bytes; give only 17
	unsigned char buf[17];
	size_t stRes = VBIC_ToBuffer(buf, sizeof(buf), &compact);
	TEST_CHECK_EQUAL(0u, stRes);

	// and invalid version byte in VBIC_FromBuffer must also return VBIC_FALSE
	unsigned char badBuf[18];
	memset(badBuf, 0, sizeof(badBuf));
	badBuf[0] = 1;	// non-zero version -> reject
	VBICompactEntry_t dstEntries[1];
	VBICompact_t dst;
	dst.pEntries = dstEntries;
	VBIC_BOOL res = VBIC_FromBuffer(&dst, 1, badBuf, sizeof(badBuf));
	TEST_CHECK_EQUAL(VBIC_FALSE, res);
}

TEST(VBICompact, vbic_buffer_too_small)
{
	test_vbic_buffer_too_small();
}

void test_vbic_tobuffer_bytes()
{
	// Asserts the specific byte layout VBIC_ToBuffer produces, so the two
	// `>> 8` / `>> 16` shifts on uTotalFields (would become `<<` under mutation)
	// produce clearly wrong output. Also verifies round-trip exact fields.
	VBICompactEntry_t src[2];
	src[0].u32StartAbsField = 0x11223344;
	src[0].i32StartPictureNumber = 10;
	src[0].typePattern = PATTERN_22;
	src[0].u16Special = 0x5566;
	src[0].u8PatternOffset = 1;

	src[1].u32StartAbsField = 0xAABBCCDD;
	src[1].i32StartPictureNumber = 20;
	src[1].typePattern = PATTERN_23;
	src[1].u16Special = 0x7788;
	src[1].u8PatternOffset = 2;

	VBICompact_t compact;
	compact.uEntryCount = 2;
	compact.pEntries = src;
	compact.uTotalFields = 0xA1B2C3D4;	// every byte distinct, so shifts are observable

	unsigned char buf[100];
	memset(buf, 0xEE, sizeof(buf));
	size_t stRes = VBIC_ToBuffer(buf, sizeof(buf), &compact);

	// 6 header bytes + 2 entries * 12 bytes each = 30
	TEST_CHECK_EQUAL((size_t) 30, stRes);

	// Exact-size buffer (30 bytes) must still succeed: pins the `stBufSize >= uBytesNeeded`
	// comparator at VBICompact.c:548 (original: 30 >= 30 passes; `>` variant
	// would treat 30 > 30 as false and return 0).
	unsigned char bufExact[30];
	stRes = VBIC_ToBuffer(bufExact, sizeof(bufExact), &compact);
	TEST_CHECK_EQUAL((size_t) 30, stRes);

	// One byte short must fail.
	unsigned char bufShort[29];
	stRes = VBIC_ToBuffer(bufShort, sizeof(bufShort), &compact);
	TEST_CHECK_EQUAL((size_t) 0, stRes);

	// Now rebuild stRes for the round-trip below by going back to the full buffer.
	memset(buf, 0xEE, sizeof(buf));
	stRes = VBIC_ToBuffer(buf, sizeof(buf), &compact);
	TEST_CHECK_EQUAL((size_t) 30, stRes);

	TEST_CHECK_EQUAL(0, buf[0]);		// version
	TEST_CHECK_EQUAL(2, buf[1]);		// entry count (2 & 0xFF)
	TEST_CHECK_EQUAL(0xD4, buf[2]);		// uTotalFields LSB
	TEST_CHECK_EQUAL(0xC3, buf[3]);		// byte 1 -> (>> 8)
	TEST_CHECK_EQUAL(0xB2, buf[4]);		// byte 2 -> (>> 16)
	TEST_CHECK_EQUAL(0xA1, buf[5]);		// MSB

	// Round-trip and verify entry fields preserved exactly.
	VBICompactEntry_t dstEntries[2];
	VBICompact_t dst;
	dst.pEntries = dstEntries;
	VBIC_BOOL res = VBIC_FromBuffer(&dst, 2, buf, stRes);
	TEST_CHECK_EQUAL(VBIC_TRUE, res);
	TEST_CHECK_EQUAL(2u, dst.uEntryCount);
	TEST_CHECK_EQUAL(0xA1B2C3D4u, dst.uTotalFields);

	TEST_CHECK_EQUAL(src[0].u32StartAbsField, dstEntries[0].u32StartAbsField);
	TEST_CHECK_EQUAL(src[0].i32StartPictureNumber, dstEntries[0].i32StartPictureNumber);
	TEST_CHECK_EQUAL(src[0].typePattern, dstEntries[0].typePattern);
	TEST_CHECK_EQUAL(src[0].u16Special, dstEntries[0].u16Special);
	TEST_CHECK_EQUAL(src[0].u8PatternOffset, dstEntries[0].u8PatternOffset);

	TEST_CHECK_EQUAL(src[1].u32StartAbsField, dstEntries[1].u32StartAbsField);
	TEST_CHECK_EQUAL(src[1].i32StartPictureNumber, dstEntries[1].i32StartPictureNumber);
	TEST_CHECK_EQUAL(src[1].typePattern, dstEntries[1].typePattern);
	TEST_CHECK_EQUAL(src[1].u16Special, dstEntries[1].u16Special);
	TEST_CHECK_EQUAL(src[1].u8PatternOffset, dstEntries[1].u8PatternOffset);
}

TEST(VBICompact, vbic_tobuffer_bytes)
{
	test_vbic_tobuffer_bytes();
}

void test_vbic_setfield_entry_walk()
{
	// Builds a 3-entry VBIC (PICNUM, PICNUM, PICNUM) and uses VBIC_SetField to
	// walk across entry boundaries in both directions. Pins the `pEntry->u32StartAbsField > u32CurAbsField`
	// boundary in the backward-walk loop (VBICompact.c:105) and `bMovedForward = VBIC_TRUE` at line 84.
	VBICompactEntry_t entries[3];

	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 10;
	entries[0].typePattern = PATTERN_PICNUM;
	entries[0].u16Special = 0;
	entries[0].u8PatternOffset = 0;

	entries[1].u32StartAbsField = 100;
	entries[1].i32StartPictureNumber = 20;
	entries[1].typePattern = PATTERN_PICNUM;
	entries[1].u16Special = 0;
	entries[1].u8PatternOffset = 0;

	entries[2].u32StartAbsField = 200;
	entries[2].i32StartPictureNumber = 30;
	entries[2].typePattern = PATTERN_PICNUM;
	entries[2].u16Special = 0;
	entries[2].u8PatternOffset = 0;

	VBICompact_t compact;
	compact.uEntryCount = 3;
	compact.pEntries = entries;
	compact.uTotalFields = 300;

	VBIC_Init(&compact);

	// Jump forward into the middle entry — picture number should come from
	// that entry, not the first.
	VBIC_SetField(150);
	TEST_CHECK_EQUAL(20u, VBIC_GetCurPictureNum());
	TEST_CHECK_EQUAL(150u, VBIC_GetCurAbsField());

	// Jump to the exact boundary of the 3rd entry. The backward-walk check
	// uses `>`, so u32StartAbsField (200) > u32CurAbsField (200) is FALSE —
	// we must land on entry 2, returning picnum 30, not walk backward.
	VBIC_SetField(200);
	TEST_CHECK_EQUAL(30u, VBIC_GetCurPictureNum());

	// Jump backward across two entries — must walk back to entry 0.
	VBIC_SetField(50);
	TEST_CHECK_EQUAL(10u, VBIC_GetCurPictureNum());

	// Jump forward again.
	VBIC_SetField(250);
	TEST_CHECK_EQUAL(30u, VBIC_GetCurPictureNum());

	// Backward walk exact-boundary: after landing in entry 2 (field 250), jump
	// to entry 1's start boundary (field 100). Backward walk must stop at
	// entry 1, NOT step further back to entry 0 — pins the `pEntry->u32StartAbsField > u32CurAbsField`
	// comparator at VBICompact.c:105 (a `>=` variant would step past).
	VBIC_SetField(100);
	TEST_CHECK_EQUAL(20u, VBIC_GetCurPictureNum());
}

TEST(VBICompact, vbic_setfield_entry_walk)
{
	test_vbic_setfield_entry_walk();
}

void test_vbic_loadline18_negative_offset()
{
	// Exercise VBIC_LoadLine18 with a negative field offset. This hits the
	// `while (iFieldOffsetFromEntryStart < 0)` loop that pins:
	//   - PATTERN_22 branch: uAdder = 2 (line 389)
	//   - PATTERN_23 branch: uAdder = 5 (line 392)
	//   - `iFieldOffsetFromEntryStart += uAdder` (line 404)
	//   - `lPicNumOffset--` (line 405)
	// and the `< 0` comparator on line 402.
	VBICompactEntry_t entry;

	// PATTERN_22: picnum 10 at field 10. LoadLine18 with offset -2 should walk
	// forward by 2 (adder=2), giving iFieldOffsetFromEntryStart=0 and
	// lPicNumOffset=-1, so the "rendered" picture number drops by 1 to 9.
	entry.u32StartAbsField = 10;
	entry.i32StartPictureNumber = 10;
	entry.typePattern = PATTERN_22;
	entry.u16Special = 0;
	entry.u8PatternOffset = 0;

	VBICompactEntry_t dummyEntries[1];
	dummyEntries[0] = entry;
	VBICompact_t compact;
	compact.uEntryCount = 1;
	compact.pEntries = dummyEntries;
	compact.uTotalFields = 100;
	VBIC_Init(&compact);

	VBIC_LoadLine18(&entry, -2);
	// After one iteration, iFieldOffsetFromEntryStart becomes 0 (even) so the
	// PATTERN_22 branch assigns u32CurPictureNum = basePictureNumber + 0 = 10.
	// If the `< 0` loop guard flips (loop over-runs), the offset walks to 2 and
	// u32CurPictureNum would become 11. If `+=` becomes `-=` the loop never
	// terminates.
	TEST_CHECK_EQUAL(10u, VBIC_GetCurPictureNum());

	// Offset -4 with adder=2 -> 2 iterations, offset ends at 0, picnum = 10.
	VBIC_LoadLine18(&entry, -4);
	TEST_CHECK_EQUAL(10u, VBIC_GetCurPictureNum());

	// PATTERN_23: picnum 10 at field 10. adder=5. offset -5 -> 1 iter, offset
	// ends at 0. Remainder 0 -> u32PicNum = base + 0 = 10.
	entry.typePattern = PATTERN_23;
	dummyEntries[0] = entry;
	VBIC_Init(&compact);
	VBIC_LoadLine18(&entry, -5);
	TEST_CHECK_EQUAL(10u, VBIC_GetCurPictureNum());
}

TEST(VBICompact, vbic_loadline18_negative_offset)
{
	test_vbic_loadline18_negative_offset();
}

void test_vbic_loadline18_22_bcd()
{
	// Load PATTERN_22 with picture number 12345 (non-trivial BCD digits so the
	// << 4 shift in VBIC_2BCD differs from >> 4). GetCurFieldLine18 returns
	// the 24-bit BCD-encoded line, which pins VBIC_2BCD's byte computations
	// (lines 33, 34) and the pu8CurLine18[0] |= 0xF8 bit.
	VBICompactEntry_t entry;
	entry.u32StartAbsField = 0;
	entry.i32StartPictureNumber = 12345;
	entry.typePattern = PATTERN_22;
	entry.u16Special = 0;
	entry.u8PatternOffset = 0;

	VBICompactEntry_t entries[1];
	entries[0] = entry;
	VBICompact_t compact;
	compact.uEntryCount = 1;
	compact.pEntries = entries;
	compact.uTotalFields = 100;
	VBIC_Init(&compact);

	// Field 0 is the even frame start of PATTERN_22 at picnum 12345.
	// VBIC_2BCD("012345") produces bytes 0x01, 0x23, 0x45; then [0] |= 0xF8 = 0xF9.
	// GetCurFieldLine18 packs as (0xF9 << 16) | (0x23 << 8) | 0x45 = 0xF92345.
	VBIC_SetField(0);
	TEST_CHECK_EQUAL(12345u, VBIC_GetCurPictureNum());
	TEST_CHECK_EQUAL(0xF92345u, VBIC_GetCurFieldLine18());
}

TEST(VBICompact, vbic_loadline18_22_bcd)
{
	test_vbic_loadline18_22_bcd();
}

void test_vbic_seek_clamping()
{
	// PATTERN_22 at picnum 1 starting at field 0. With uTotalFields = 10
	// (uMaxFieldIdx = 9), seeking frame 10 computes uFinalField = 18 which is
	// > 9, so the clamping branch at VBICompact.c:301-320 adjusts both the
	// final field and the frame number. Pins the `uFinalField > uMaxFieldIdx`
	// comparator (line 301) when it's genuinely out of range, and the PATTERN_22
	// adjustment path.
	VBICompactEntry_t entries[1];
	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 1;
	entries[0].typePattern = PATTERN_22;
	entries[0].u16Special = 0;
	entries[0].u8PatternOffset = 0;

	VBICompact_t compact;
	compact.uEntryCount = 1;
	compact.pEntries = entries;
	compact.uTotalFields = 10;

	VBIC_Init(&compact);

	// Seek frame 5 (within range): final field = 8, picnum still 5.
	VBIC_SeekResult r = VBIC_SEEK(5);
	TEST_CHECK_EQUAL(VBIC_SEEK_SUCCESS, r);
	TEST_CHECK_EQUAL(8u, VBIC_GetCurAbsField());
	TEST_CHECK_EQUAL(5u, VBIC_GetCurPictureNum());

	// Seek frame 10 (out of range): proposed final field = 18, uMaxFieldIdx = 9.
	// Delta = 9, odd, so delta++ -> 10. uFinalField = 18 - 10 = 8. uFrameNum = 10 - 5 = 5.
	r = VBIC_SEEK(10);
	TEST_CHECK_EQUAL(VBIC_SEEK_SUCCESS, r);
	TEST_CHECK_EQUAL(8u, VBIC_GetCurAbsField());
	TEST_CHECK_EQUAL(5u, VBIC_GetCurPictureNum());

	// Peek-only variant should not modify state.
	VBIC_Init(&compact);
	uint32_t uPeekField = 0xDEADBEEF;
	r = VBIC_LOOKUP_FIELD_FOR_FRAMENUM(3, &uPeekField);
	TEST_CHECK_EQUAL(VBIC_SEEK_SUCCESS, r);
	// Peek doesn't update state -> curAbsField still 0 from VBIC_Init.
	TEST_CHECK_EQUAL(0u, VBIC_GetCurAbsField());
}

TEST(VBICompact, vbic_seek_clamping)
{
	test_vbic_seek_clamping();
}

void test_vbic_frombuffer_zero_entries()
{
	// Build a valid VBIC header with zero entries. VBIC_FromBuffer must leave
	// dstEntries untouched (loop doesn't execute). Pins the `u < u8NumEntries`
	// loop guard at VBICompact.c:619 (`<= 0` variant would enter the loop once
	// and overwrite dstEntries[0]).
	unsigned char buf[6];
	buf[0] = 0;			// version
	buf[1] = 0;			// num entries
	buf[2] = 0x78;
	buf[3] = 0x56;
	buf[4] = 0x34;
	buf[5] = 0x12;		// uTotalFields = 0x12345678

	VBICompactEntry_t dstEntries[2];
	memset(dstEntries, 0xAA, sizeof(dstEntries));
	VBICompact_t dst;
	dst.pEntries = dstEntries;
	dst.uEntryCount = 99;
	dst.uTotalFields = 99;

	VBIC_BOOL res = VBIC_FromBuffer(&dst, 2, buf, sizeof(buf));
	TEST_CHECK_EQUAL(VBIC_TRUE, res);
	TEST_CHECK_EQUAL(0u, dst.uEntryCount);
	TEST_CHECK_EQUAL(0x12345678u, dst.uTotalFields);

	// dstEntries[0] must be untouched — all 0xAA bytes.
	const unsigned char *p = reinterpret_cast<const unsigned char *>(&dstEntries[0]);
	for (size_t i = 0; i < sizeof(VBICompactEntry_t); i++)
	{
		TEST_CHECK_EQUAL((unsigned char) 0xAA, p[i]);
	}
}

TEST(VBICompact, vbic_frombuffer_zero_entries)
{
	test_vbic_frombuffer_zero_entries();
}

void test_vbic_tobuffer_zero_entries_and_rejection()
{
	// 0 entries: ToBuffer needs 6 header bytes only. Verify bytes are correct
	// and the loop at VBICompact.c:559 doesn't run (similar to above).
	VBICompact_t compact;
	compact.uEntryCount = 0;
	compact.pEntries = NULL;
	compact.uTotalFields = 0x0A0B0C0D;

	unsigned char buf[20];
	memset(buf, 0xFF, sizeof(buf));
	size_t stRes = VBIC_ToBuffer(buf, sizeof(buf), &compact);
	TEST_CHECK_EQUAL((size_t) 6, stRes);

	TEST_CHECK_EQUAL(0, buf[0]);		// version
	TEST_CHECK_EQUAL(0, buf[1]);		// count
	TEST_CHECK_EQUAL(0x0D, buf[2]);	// low byte of totalFields
	TEST_CHECK_EQUAL(0x0C, buf[3]);
	TEST_CHECK_EQUAL(0x0B, buf[4]);
	TEST_CHECK_EQUAL(0x0A, buf[5]);	// high byte
	// Bytes past 5 must remain 0xFF (untouched).
	TEST_CHECK_EQUAL((unsigned char) 0xFF, buf[6]);
}

TEST(VBICompact, vbic_tobuffer_zero_entries_and_rejection)
{
	test_vbic_tobuffer_zero_entries_and_rejection();
}

void test_vbic_seek_fail_paths()
{
	// First entry is PATTERN_LEADIN with i32StartPictureNumber > sought frame
	// AND non-zero u8PatternOffset. VBIC_SeekInternal's entry-scan loop breaks
	// on the first entry because iStartFrameNum(10) > uFrameNum(1), leaving
	// i == 0 at the loop exit. That opens the `if (i == 0)` branch, which
	// consults u8PatternOffset; with a non-2:2/2:3 pattern it falls into the
	// default case at VBICompact.c:215-217 (res = VBIC_SEEK_FAIL; goto done).
	VBICompactEntry_t leadInOff[1];
	leadInOff[0].u32StartAbsField = 0;
	leadInOff[0].i32StartPictureNumber = 10;	// > sought frame below
	leadInOff[0].typePattern = PATTERN_LEADIN;
	leadInOff[0].u16Special = 0;
	leadInOff[0].u8PatternOffset = 1;	// non-zero -> hits the inner switch

	VBICompact_t c;
	c.uEntryCount = 1;
	c.pEntries = leadInOff;
	c.uTotalFields = 50;

	VBIC_Init(&c);

	VBIC_SeekResult r = VBIC_SEEK(1);
	TEST_CHECK_EQUAL(VBIC_SEEK_FAIL, r);

	// Second scenario: first entry is PATTERN_LEADOUT with offset 0 — this hits
	// the outer switch default (VBICompact.c:277-280) setting res = VBIC_SEEK_FAIL.
	VBICompactEntry_t leadOut[1];
	leadOut[0].u32StartAbsField = 0;
	leadOut[0].i32StartPictureNumber = 0;
	leadOut[0].typePattern = PATTERN_LEADOUT;
	leadOut[0].u16Special = 0;
	leadOut[0].u8PatternOffset = 0;

	c.pEntries = leadOut;
	VBIC_Init(&c);

	r = VBIC_SEEK(3);
	TEST_CHECK_EQUAL(VBIC_SEEK_FAIL, r);
}

TEST(VBICompact, vbic_seek_fail_paths)
{
	test_vbic_seek_fail_paths();
}

void test_vbic_line18_zeroes_and_leadin_leadout()
{
	// Builds an entry with PATTERN_ZEROES and verifies VBIC_GetCurFieldLine18
	// is exactly 0 after VBIC_LoadLine18 runs, pinning the three
	// `pu8CurLine18[i] = 0` assignments in the ZEROES branch.
	VBICompactEntry_t entries[3];
	VBICompact_t compact;
	compact.uEntryCount = 3;
	compact.pEntries = entries;
	compact.uTotalFields = 100;

	entries[0].u32StartAbsField = 0;
	entries[0].i32StartPictureNumber = 0;
	entries[0].typePattern = PATTERN_ZEROES;
	entries[0].u16Special = 0;
	entries[0].u8PatternOffset = 0;

	entries[1].u32StartAbsField = 10;
	entries[1].i32StartPictureNumber = 0;
	entries[1].typePattern = PATTERN_LEADIN;
	entries[1].u16Special = 0;
	entries[1].u8PatternOffset = 0;

	entries[2].u32StartAbsField = 20;
	entries[2].i32StartPictureNumber = 0;
	entries[2].typePattern = PATTERN_LEADOUT;
	entries[2].u16Special = 0;
	entries[2].u8PatternOffset = 0;

	VBIC_Init(&compact);

	// Pins VBIC_Init's `u32CurPictureNum = 0` initializer (line ~17). With the
	// first entry being PATTERN_ZEROES, LoadLine18 doesn't touch this field,
	// so the initializer is what gets observed.
	TEST_CHECK_EQUAL(0u, VBIC_GetCurPictureNum());

	// field 0 -> ZEROES pattern -> line18 must read as 0x000000
	VBIC_SetField(0);
	TEST_CHECK_EQUAL(0u, VBIC_GetCurFieldLine18());

	// field 10 -> LEADIN -> line18 is 0x88FFFF
	VBIC_SetField(10);
	TEST_CHECK_EQUAL(0x88FFFFu, VBIC_GetCurFieldLine18());

	// field 20 -> LEADOUT -> line18 is 0x80EEEE
	VBIC_SetField(20);
	TEST_CHECK_EQUAL(0x80EEEEu, VBIC_GetCurFieldLine18());
}

TEST(VBICompact, vbic_line18_zeroes_and_leadin_leadout)
{
	test_vbic_line18_zeroes_and_leadin_leadout();
}
