#include "stdafx.h"
#include <ldp-abst/VBICompact.h>
#include <ldp-abst/VBIParse.h>
#include <ldp-abst/VideoStandard.h>
#include <cstdio>	// for fopen

template <typename T>
using shared_array = std::shared_ptr<T[]>;

typedef shared_array<unsigned char> byteSA;

void load_file_to_buf(unsigned char *pBuf, size_t bufLen, const char* filename)
{
	FILE* F = fopen(filename, "rb");
	if (F == nullptr)
	{
		throw std::runtime_error("Failed to open file");
	}

    size_t stBytesRead = fread(pBuf, 1, bufLen, F);

    ASSERT_EQ(bufLen, stBytesRead);

    fclose(F);
}

byteSA load_file(const char* filename)
{
	FILE* F = fopen(filename, "rb");
	if (F == nullptr)
    {
          throw std::runtime_error("Failed to open file");
    }

    fseek(F, 0, SEEK_END);
    size_t stBytesTotal = ftell(F);
    fseek(F, 0, SEEK_SET);

    byteSA result = byteSA(new unsigned char[stBytesTotal]);

	size_t stBytesRead = fread(result.get(), 1, stBytesTotal, F);

	if (stBytesRead != stBytesTotal)
    {
          throw std::runtime_error("Failed to read file");
    }

	fclose(F);

    return result;
}

void test_vbi1_helper()
{
	unsigned char bufLine[720 * 3];
	bool bRes = false;
	unsigned int uResult = 0;

	// Freedom Fighter
	// read RGB line 17 into buffer
    load_file_to_buf(bufLine, sizeof(bufLine), "ffrA80030-line17.bin");
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_CHECK(bRes);
	TEST_CHECK_EQUAL(uResult, 0xA80030);

	// now try line 18
	load_file_to_buf(bufLine, sizeof(bufLine), "ffrA80030-line18.bin");
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_CHECK(bRes);
	TEST_CHECK_EQUAL(uResult, 0xA80030);

	// try line 16 which is empty
	load_file_to_buf(bufLine, sizeof(bufLine), "ffrA80030-line16.bin");
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_CHECK(bRes);	// black lines now return true when parsed
	TEST_CHECK_EQUAL(uResult, 0);

	// line 11 (should not be white flag)
	load_file_to_buf(bufLine, sizeof(bufLine), "ffrA80030-line11.bin");
	bRes = VBIParse::IsWhiteFlag(bufLine, 720);
	TEST_CHECK(!bRes);

	// now try line 18 of the other field
	load_file_to_buf(bufLine, sizeof(bufLine), "ffrF80031-line18.bin");
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(bRes);
	TEST_CHECK_EQUAL(uResult, 0xF80031);

	// now try line 17 of the other field
	load_file_to_buf(bufLine, sizeof(bufLine), "ffrF80031-line17.bin");
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(bRes);
	TEST_CHECK_EQUAL(uResult, 0xF80031);

	// Dragon's Lair
	// line 17
	load_file_to_buf(bufLine, sizeof(bufLine), "lairF85807-line17.bin");
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(bRes);
	TEST_CHECK_EQUAL(uResult, 0xF85807);

	// line 18
	load_file_to_buf(bufLine, sizeof(bufLine), "lairF85807-line18.bin");
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(bRes);
	TEST_CHECK_EQUAL(uResult, 0xF85807);

	// line 11 (should be a white flag)
	load_file_to_buf(bufLine, sizeof(bufLine), "lairF85807-line11.bin");
	bRes = VBIParse::IsWhiteFlag(bufLine, 720);
	TEST_REQUIRE(bRes);
	// try parsing the white flag as manchester data; it should fail without crashing
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(!bRes);

	// this unit test in response to a defect I discovered
	memset(bufLine, 0, sizeof(bufLine));
	memset(bufLine + 3, 0xFF, sizeof(bufLine)-3);	// make white until the end so that there is only one transition
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(!bRes);

	// Cube Quest, Line 17
	load_file_to_buf(bufLine, sizeof(bufLine), "cqF80015-line17.bin");
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(bRes);
	TEST_CHECK_EQUAL(uResult, 0xF80015);

	// Cube Quest, Line 18
	load_file_to_buf(bufLine, sizeof(bufLine), "cqF80015-line18.bin");
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(bRes);
	TEST_CHECK_EQUAL(uResult, 0xF80015);

	// Cube Quest, track 199, Line 17, one of the 'high' lines is bad, so this should not parse
	load_file_to_buf(bufLine, sizeof(bufLine), "cqF80198-line17.bin");
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(!bRes);

	// Cube Quest, track 199, Line 18
	load_file_to_buf(bufLine, sizeof(bufLine), "cqF80198-line18.bin");
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(bRes);
	TEST_CHECK_EQUAL(uResult, 0xF80198);

	// Cube Quest, track 1271, Line 17
	// In Warren's capture, the VBI is bad, so the parse should fail.
	// (but it wasn't failing, it was returning incorrect results, hence this unit test)
	load_file_to_buf(bufLine, sizeof(bufLine), "cqF81270-line17.bin");
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(!bRes);

	// Cube Quest, track 91, Line 18
	// (one of the HIGH lines is way too short on this one)
	load_file_to_buf(bufLine, sizeof(bufLine), "cq-91-1-line18.bin");
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(bRes);
	TEST_CHECK_EQUAL(uResult, 0x880DDD);
}

TEST(VBIParse, vbi1)
{
	test_vbi1_helper();
}

void test_vbi2_helper()
{
	bool bRes = false;
	unsigned int uResult = 0;
	unsigned char bufLine[720 * 3];

	// make line all white
	memset(bufLine, 0xFF, sizeof(bufLine));

	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(!bRes);	// parse should fail
	TEST_CHECK_EQUAL(uResult, VBIParse::PARSE_FAILED);	// and the result should indicate a failed parse

	// make all black
	memset(bufLine, 0, sizeof(bufLine));
	bRes = VBIParse::ParseRGBManchester(uResult, bufLine, 720);
	TEST_REQUIRE(bRes);	// parse should succeed
	TEST_CHECK_EQUAL(uResult, 0);	// and the result should indicate a black line
}

TEST(VBIParse, vbi2)
{
	test_vbi2_helper();
}

void test_vbi1718_chooser()
{
	bool bRes = false;
	unsigned int uRes = 0;
	VBI_t vbi;

	// both line parses failed
	vbi.uVBI[1] = vbi.uVBI[2] = VBIParse::PARSE_FAILED;
	bRes = VBIParse::GetBestLine1718(uRes, vbi, NTSC, 0xF80001);
	TEST_CHECK(!bRes);
	TEST_CHECK_EQUAL(uRes, VBIParse::PARSE_FAILED);

	// try where one parsed and one didn't parse
	vbi.uVBI[2] = 0xF80001;
	bRes = VBIParse::GetBestLine1718(uRes, vbi, NTSC);
	TEST_CHECK(bRes);
	TEST_CHECK_EQUAL(uRes, 0xF80001);

	// try two parsed frame numbers where one is wrong (and supply the previous correct frame number)
	vbi.uVBI[1] = 0xF80200;
	vbi.uVBI[2] = 0xF80002;
	bRes = VBIParse::GetBestLine1718(uRes, vbi, NTSC, 0xF80001);
	TEST_CHECK(bRes);
	TEST_CHECK_EQUAL(uRes, 0xF80002);

	// try again on a boundary that works for decimal but not for hex
	vbi.uVBI[2] = 0xF80010;
	bRes = VBIParse::GetBestLine1718(uRes, vbi, NTSC, 0xF80009);
	TEST_REQUIRE(bRes);
	TEST_CHECK_EQUAL(uRes, 0xF80010);

	// try two parsed frame numbers where both are wrong (and supply the previous correct frame number)
	vbi.uVBI[2] = 0xF80002;
	bRes = VBIParse::GetBestLine1718(uRes, vbi, NTSC, 0xF80100);
	TEST_CHECK(!bRes);

	// try two parsed non-frame numbers(and supply the previous correct frame number)
	vbi.uVBI[1] = 0x880200;
	vbi.uVBI[2] = 0x880002;
	bRes = VBIParse::GetBestLine1718(uRes, vbi, NTSC, 0xF80001);
	TEST_CHECK(!bRes);

	// try one black line and one chapter line (cube quest does this a lot)
	vbi.uVBI[1] = VBIParse::PARSE_BLACK;
	vbi.uVBI[2] = 0x880ddd;
	bRes = VBIParse::GetBestLine1718(uRes, vbi, NTSC);
	TEST_CHECK(bRes);
	TEST_CHECK_EQUAL(uRes, 0x880ddd);

	// try one black line and failed parse (this happens with cube quest occasionally)
	vbi.uVBI[2] = VBIParse::PARSE_FAILED;
	bRes = VBIParse::GetBestLine1718(uRes, vbi, NTSC);
	TEST_CHECK(bRes);
	TEST_CHECK_EQUAL(uRes, VBIParse::PARSE_BLACK);

}

TEST(VBIParse, vbi1718_chooser)
{
	test_vbi1718_chooser();
}

void test_vbi_field_parse()
{
	bool bRes = false;
	byteSA BufSPtr = load_file("cq-16153-1-all.bin");

	unsigned char *pField = BufSPtr.get();
     size_t stRes;

	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	VBI_t vbi;
	bRes = pVBI->ParseRGBField(vbi, pField, 720);
	TEST_REQUIRE(bRes);

	// expected results
	TEST_CHECK(!vbi.bWhiteFlag);
	TEST_CHECK_EQUAL(vbi.uVBI[2], 0x880DDD);

	pVBI->AddVBIData(vbi);
	stRes = pVBI->GetVBIDataCount();
	TEST_REQUIRE_EQUAL(stRes, 1);	// we've added one entry, it should be there

	string strBuf = pVBI->SaveVBIData();

	// try loading VBI data, but provide the wrong size
	bRes = pVBI->LoadVBIData(strBuf.data(), 2);
	TEST_REQUIRE(!bRes);

	bRes = pVBI->LoadVBIData(strBuf.data(), strBuf.size());
	TEST_REQUIRE(bRes);

	// verify contents of loaded VBI data
	memset(&vbi, 0, sizeof(vbi));	// clear VBI structure to prepare for test
	bRes = pVBI->GetVBIData(vbi, 1);	// out of range
	TEST_REQUIRE(!bRes);

	bRes = pVBI->GetVBIData(vbi, 0);	// in range
	TEST_REQUIRE(bRes);
	TEST_CHECK_EQUAL(vbi.uVBI[2], 0x880DDD);
}

TEST(VBIParse, vbi_field_parse)
{
	test_vbi_field_parse();
}

void test_vbi_verify()
{
	VBI_t vbi;
	bool bRes = false;
	memset(&vbi, 0, sizeof(vbi));

	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	vbi.uVBI[1] = vbi.uVBI[2] = 0xF80001;
	pVBI->AddVBIData(vbi);

	vbi.uVBI[1] = vbi.uVBI[2] = 0x0;	// non-dominant field
	pVBI->AddVBIData(vbi);

	vbi.uVBI[1] = vbi.uVBI[2] = 0xF80003;	// skip one frame
	pVBI->AddVBIData(vbi);

	bRes = pVBI->VerifyVBIData(NTSC);
	TEST_REQUIRE(bRes);	// should pass, but because we skipped a frame we should have a warning
	list<string> lstWarnings = pVBI->GetWarnings();
	TEST_REQUIRE(lstWarnings.size() == 1);

	pVBI->ClearVBIData();

	vbi.uVBI[1] = vbi.uVBI[2] = 0xF80001;
	pVBI->AddVBIData(vbi);

	vbi.uVBI[1] = vbi.uVBI[2] = 0xF80002;	// new frame number only one field later, that is odd and bad
	pVBI->AddVBIData(vbi);

	bRes = pVBI->VerifyVBIData(NTSC);
	TEST_REQUIRE(bRes);	// should pass, but because we skipped a frame we should have a warning
	lstWarnings = pVBI->GetWarnings();
	TEST_REQUIRE(lstWarnings.size() == 1);

	pVBI->ClearVBIData();

	vbi.uVBI[1] = vbi.uVBI[2] = 0xF80009;
	pVBI->AddVBIData(vbi);

	vbi.uVBI[1] = vbi.uVBI[2] = 0x0;
	pVBI->AddVBIData(vbi);

	vbi.uVBI[1] = vbi.uVBI[2] = 0xF80010;	// this should be ok
	pVBI->AddVBIData(vbi);

	bRes = pVBI->VerifyVBIData(NTSC);
	TEST_REQUIRE(bRes);	// should succeed

}

TEST(VBIParse, vbi_verify)
{
	test_vbi_verify();
}

void test_vbi_pic_num()
{
	bool bRes = false;

	// make sure the failed parse value is clearly distinct from picture nums
	bRes = VBIParse::IsPictureNum(VBIParse::PARSE_FAILED);
	TEST_REQUIRE(!bRes);

	bRes = VBIParse::IsPictureNum(0xF80578);
	TEST_REQUIRE(bRes);
}

TEST(VBIParse, vbi_pic_num)
{
	test_vbi_pic_num();
}

void test_vbi_generators()
{
	// Direct field assertions on every GenerateVBI* helper. These kill a cluster
	// of cxx_assign_const mutations (0 or specific-hex becomes 42) because most
	// callers consume the output through downstream code that can't distinguish
	// all the values from 42.

	VBI_t vbi;

	// GenerateVBIFrame (NTSC)
	vbi = VBIParse::GenerateVBIFrame(12345, NTSC);
	TEST_CHECK(vbi.bWhiteFlag == false);
	TEST_CHECK_EQUAL(0, vbi.uVBI[0]);
	// uVBI[1] == DecimalToPictureNumVBI(12345, NTSC) == (0x12345 | 0xF80000)
	TEST_CHECK_EQUAL(0xF92345, vbi.uVBI[1]);
	TEST_CHECK_EQUAL(vbi.uVBI[1], vbi.uVBI[2]);

	// GenerateVBIFrame (PAL) — no NTSC white-flag bit, so top nibble is 0xF
	vbi = VBIParse::GenerateVBIFrame(12345, PAL);
	TEST_CHECK(vbi.bWhiteFlag == false);
	TEST_CHECK_EQUAL(0, vbi.uVBI[0]);
	TEST_CHECK_EQUAL(0xF12345, vbi.uVBI[1]);
	TEST_CHECK_EQUAL(vbi.uVBI[1], vbi.uVBI[2]);

	// GenerateVBIEmpty
	vbi = VBIParse::GenerateVBIEmpty();
	TEST_CHECK(vbi.bWhiteFlag == false);
	TEST_CHECK_EQUAL(0, vbi.uVBI[0]);
	TEST_CHECK_EQUAL(0, vbi.uVBI[1]);
	TEST_CHECK_EQUAL(0, vbi.uVBI[2]);

	// GenerateVBIChapter
	vbi = VBIParse::GenerateVBIChapter(7);
	TEST_CHECK(vbi.bWhiteFlag == false);
	TEST_CHECK_EQUAL(0, vbi.uVBI[0]);
	// DecimalToChapterNumVBI(7) = 0x880DDD | (0x7 << 12) = 0x887DDD
	TEST_CHECK_EQUAL(0x887DDD, vbi.uVBI[1]);
	TEST_CHECK_EQUAL(vbi.uVBI[1], vbi.uVBI[2]);

	// GenerateVBIStopCode
	vbi = VBIParse::GenerateVBIStopCode();
	TEST_CHECK(vbi.bWhiteFlag == false);
	TEST_CHECK_EQUAL(0x82CFFF, vbi.uVBI[0]);
	TEST_CHECK_EQUAL(0x82CFFF, vbi.uVBI[1]);
	TEST_CHECK_EQUAL(0, vbi.uVBI[2]);

	// GenerateVBILeadIn
	vbi = VBIParse::GenerateVBILeadIn();
	TEST_CHECK(vbi.bWhiteFlag == false);
	TEST_CHECK_EQUAL(0, vbi.uVBI[0]);
	TEST_CHECK_EQUAL(0x88FFFF, vbi.uVBI[1]);
	TEST_CHECK_EQUAL(0x88FFFF, vbi.uVBI[2]);

	// GenerateVBILeadOut
	vbi = VBIParse::GenerateVBILeadOut();
	TEST_CHECK(vbi.bWhiteFlag == false);
	TEST_CHECK_EQUAL(0, vbi.uVBI[0]);
	TEST_CHECK_EQUAL(0x80EEEE, vbi.uVBI[1]);
	TEST_CHECK_EQUAL(0x80EEEE, vbi.uVBI[2]);
}

TEST(VBIParse, vbi_generators)
{
	test_vbi_generators();
}

void test_vbi_classifiers()
{
	// IsStopCode: equality check against a fixed constant.
	TEST_CHECK(VBIParse::IsStopCode(0x82CFFF) == true);
	TEST_CHECK(VBIParse::IsStopCode(0x82CFFE) == false);
	TEST_CHECK(VBIParse::IsStopCode(0) == false);

	// IsChapterNum: masked-equality check.
	TEST_CHECK(VBIParse::IsChapterNum(0x800DDD) == true);	// chapter 0
	TEST_CHECK(VBIParse::IsChapterNum(0x887DDD) == true);	// chapter 7 (middle bits vary, mask ignores them)
	TEST_CHECK(VBIParse::IsChapterNum(0x800DDE) == false);	// differs in low bits
	TEST_CHECK(VBIParse::IsChapterNum(0x900DDD) == false);	// differs in top nibble

	// IsVBIDefined: top bit of a 24-bit value must be set.
	TEST_CHECK(VBIParse::IsVBIDefined(0x800000) == true);
	TEST_CHECK(VBIParse::IsVBIDefined(0xF12345) == true);
	TEST_CHECK(VBIParse::IsVBIDefined(0x7FFFFF) == false);
	TEST_CHECK(VBIParse::IsVBIDefined(0) == false);
}

TEST(VBIParse, vbi_classifiers)
{
	test_vbi_classifiers();
}

void test_vbi_decimal_conversions()
{
	// Round-trip picture number (NTSC). DecimalToPictureNumVBI treats the
	// decimal digits as a hex literal, so 54321 -> 0x54321 | NTSC_HEADER(0xF80000).
	unsigned int uVBI = VBIParse::DecimalToPictureNumVBI(54321, NTSC);
	TEST_CHECK_EQUAL(0xFD4321, uVBI);
	TEST_CHECK_EQUAL(54321u, VBIParse::PictureNumVBIToDecimal(uVBI, NTSC));

	// PAL variant: 0x54321 | PAL_HEADER(0xF00000).
	uVBI = VBIParse::DecimalToPictureNumVBI(54321, PAL);
	TEST_CHECK_EQUAL(0xF54321, uVBI);
	TEST_CHECK_EQUAL(54321u, VBIParse::PictureNumVBIToDecimal(uVBI, PAL));

	// Round-trip chapter number: 0x880DDD | (0x42 << 12).
	unsigned int uCh = VBIParse::DecimalToChapterNumVBI(42);
	TEST_CHECK_EQUAL(0x8C2DDD, uCh);
	TEST_CHECK_EQUAL(42u, VBIParse::ChapterNumVBIToDecimal(uCh));

	// stoi-throw path in PictureNumVBIToDecimal: PAL mask keeps top nibble 0xA,
	// which makes the intermediate string start with a non-decimal digit and
	// std::stoi throws std::invalid_argument. Expected result is 0 (from the
	// catch block), which is distinct from 42.
	TEST_CHECK_EQUAL(0u, VBIParse::PictureNumVBIToDecimal(0xA0000, PAL));
}

TEST(VBIParse, vbi_decimal_conversions)
{
	test_vbi_decimal_conversions();
}

void test_vbi_save_load_round_trip()
{
	// Tests SaveVBIData / LoadVBIData / GetVBIData / GetVBIDataCount with enough
	// specificity to kill off-by-one loop mutants (< vs <=), ne/eq swaps on the
	// white-flag byte, and error-path mutants that remove error-message pushes.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();
	TEST_CHECK_EQUAL(0u, pVBI->GetVBIDataCount());

	VBI_t a;
	a.bWhiteFlag = true;	// one true, one false so mutants that flip != to == on the byte get caught
	a.uVBI[0] = 0x112233;
	a.uVBI[1] = 0x445566;
	a.uVBI[2] = 0x778899;

	VBI_t b;
	b.bWhiteFlag = false;
	b.uVBI[0] = 0xAABBCC;
	b.uVBI[1] = 0xDDEEFF;
	b.uVBI[2] = 0x010203;

	pVBI->AddVBIData(a);
	pVBI->AddVBIData(b);
	TEST_CHECK_EQUAL(2u, pVBI->GetVBIDataCount());

	string strBuf = pVBI->SaveVBIData();

	// header (4 bytes "1VBI") + 2 entries * 10 bytes each = 24 bytes exactly.
	TEST_CHECK_EQUAL(24u, strBuf.size());

	// Verify header, then byte-by-byte contents of each entry — this pins
	// the inner `< 3` loop in SaveVBIData (so `<= 3` would write extra bytes)
	// and the bWhiteFlag byte assignment.
	const unsigned char *p = reinterpret_cast<const unsigned char *>(strBuf.data());
	TEST_CHECK_EQUAL('1', p[0]);
	TEST_CHECK_EQUAL('V', p[1]);
	TEST_CHECK_EQUAL('B', p[2]);
	TEST_CHECK_EQUAL('I', p[3]);

	// entry a
	TEST_CHECK_EQUAL(1, p[4]);		// bWhiteFlag = true
	TEST_CHECK_EQUAL(0x11, p[5]);
	TEST_CHECK_EQUAL(0x22, p[6]);
	TEST_CHECK_EQUAL(0x33, p[7]);
	TEST_CHECK_EQUAL(0x44, p[8]);
	TEST_CHECK_EQUAL(0x55, p[9]);
	TEST_CHECK_EQUAL(0x66, p[10]);
	TEST_CHECK_EQUAL(0x77, p[11]);
	TEST_CHECK_EQUAL(0x88, p[12]);
	TEST_CHECK_EQUAL(0x99, p[13]);

	// entry b
	TEST_CHECK_EQUAL(0, p[14]);		// bWhiteFlag = false
	TEST_CHECK_EQUAL(0xAA, p[15]);
	TEST_CHECK_EQUAL(0xBB, p[16]);
	TEST_CHECK_EQUAL(0xCC, p[17]);
	TEST_CHECK_EQUAL(0xDD, p[18]);
	TEST_CHECK_EQUAL(0xEE, p[19]);
	TEST_CHECK_EQUAL(0xFF, p[20]);
	TEST_CHECK_EQUAL(0x01, p[21]);
	TEST_CHECK_EQUAL(0x02, p[22]);
	TEST_CHECK_EQUAL(0x03, p[23]);

	// Round-trip through LoadVBIData. This also pins the `stByteCount += ...`
	// accumulator (mutating it to `-=` makes the loop terminate immediately,
	// leaving the loaded count at 0).
	pVBI->ClearVBIData();
	TEST_CHECK_EQUAL(0u, pVBI->GetVBIDataCount());
	bool bLoaded = pVBI->LoadVBIData(strBuf.data(), strBuf.size());
	TEST_CHECK(bLoaded == true);
	TEST_CHECK_EQUAL(2u, pVBI->GetVBIDataCount());

	// Verify each loaded entry field-for-field. White-flag true must come back
	// as true (pins the `!= 0` test on the byte).
	VBI_t got;
	TEST_CHECK(pVBI->GetVBIData(got, 0) == true);
	TEST_CHECK(got.bWhiteFlag == true);
	TEST_CHECK_EQUAL(0x112233u, got.uVBI[0]);
	TEST_CHECK_EQUAL(0x445566u, got.uVBI[1]);
	TEST_CHECK_EQUAL(0x778899u, got.uVBI[2]);

	TEST_CHECK(pVBI->GetVBIData(got, 1) == true);
	TEST_CHECK(got.bWhiteFlag == false);
	TEST_CHECK_EQUAL(0xAABBCCu, got.uVBI[0]);
	TEST_CHECK_EQUAL(0xDDEEFFu, got.uVBI[1]);
	TEST_CHECK_EQUAL(0x010203u, got.uVBI[2]);

	// Out-of-range GetVBIData must return false.
	TEST_CHECK(pVBI->GetVBIData(got, 2) == false);

	// --- error paths: each must record a message (pins the m_lstErrors.push_back
	// calls that would otherwise be dropped by cxx_remove_void_call) ---

	// bad version header
	unsigned char bad[14] = { 'X', 'V', 'B', 'I', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	bool bLoad = pVBI->LoadVBIData(bad, sizeof(bad));
	TEST_CHECK(bLoad == false);
	list<string> errs = pVBI->GetErrors();
	TEST_CHECK(errs.size() > 0);

	// bad size (not a multiple of 10 after the 4-byte header)
	unsigned char badSize[7] = { '1', 'V', 'B', 'I', 0, 0, 0 };
	bLoad = pVBI->LoadVBIData(badSize, sizeof(badSize));
	TEST_CHECK(bLoad == false);
	errs = pVBI->GetErrors();
	TEST_CHECK(errs.size() > 0);

	pVBI->ClearVBIData();
	TEST_CHECK_EQUAL(0u, pVBI->GetVBIDataCount());
}

TEST(VBIParse, vbi_save_load_round_trip)
{
	test_vbi_save_load_round_trip();
}

void test_vbi_get_best_line1718()
{
	// Exercises each branch of GetBestLine1718 with exact input/output checks so
	// cxx_assign_const, cxx_replace_scalar_call, and cxx_eq_to_ne mutants in the
	// branch ladder are killed.
	unsigned int uLine1718 = 0;
	bool bRes = false;

	// Branch 1: both lines equal, both not PARSE_FAILED -> result is that value.
	VBI_t vgb;
	vgb.bWhiteFlag = false;
	vgb.uVBI[0] = 0;
	vgb.uVBI[1] = 0xF80042;
	vgb.uVBI[2] = 0xF80042;
	bRes = VBIParse::GetBestLine1718(uLine1718, vgb, NTSC);
	TEST_CHECK(bRes == true);
	TEST_CHECK_EQUAL(0xF80042u, uLine1718);

	// Branch 2: line17 undefined (0), line18 defined -> result is line18.
	vgb.uVBI[1] = 0;
	vgb.uVBI[2] = 0xF80055;
	uLine1718 = 0;
	bRes = VBIParse::GetBestLine1718(uLine1718, vgb, NTSC);
	TEST_CHECK(bRes == true);
	TEST_CHECK_EQUAL(0xF80055u, uLine1718);

	// Branch 3: line17 defined, line18 undefined (0) -> result is line17.
	vgb.uVBI[1] = 0xF80077;
	vgb.uVBI[2] = 0;
	uLine1718 = 0;
	bRes = VBIParse::GetBestLine1718(uLine1718, vgb, NTSC);
	TEST_CHECK(bRes == true);
	TEST_CHECK_EQUAL(0xF80077u, uLine1718);

	// Branch 4: line17 black, line18 FAILED -> result is line17 (BLACK).
	vgb.uVBI[1] = VBIParse::PARSE_BLACK;
	vgb.uVBI[2] = VBIParse::PARSE_FAILED;
	uLine1718 = 0xDEAD;
	bRes = VBIParse::GetBestLine1718(uLine1718, vgb, NTSC);
	TEST_CHECK(bRes == true);
	TEST_CHECK_EQUAL((unsigned int) VBIParse::PARSE_BLACK, uLine1718);

	// Branch 5: line18 black, line17 FAILED -> result is line18 (BLACK).
	vgb.uVBI[1] = VBIParse::PARSE_FAILED;
	vgb.uVBI[2] = VBIParse::PARSE_BLACK;
	uLine1718 = 0xDEAD;
	bRes = VBIParse::GetBestLine1718(uLine1718, vgb, NTSC);
	TEST_CHECK(bRes == true);
	TEST_CHECK_EQUAL((unsigned int) VBIParse::PARSE_BLACK, uLine1718);

	// Branch 6a: both lines defined, different, with previous picnum, line17 is
	// (prev + 1) -> pick line17.
	vgb.uVBI[1] = 0xF80011;
	vgb.uVBI[2] = 0xF80099;
	uLine1718 = 0;
	bRes = VBIParse::GetBestLine1718(uLine1718, vgb, NTSC, 0xF80010);
	TEST_CHECK(bRes == true);
	TEST_CHECK_EQUAL(0xF80011u, uLine1718);

	// Branch 6b: line18 is (prev + 1) -> pick line18.
	vgb.uVBI[1] = 0xF80099;
	vgb.uVBI[2] = 0xF80011;
	uLine1718 = 0;
	bRes = VBIParse::GetBestLine1718(uLine1718, vgb, NTSC, 0xF80010);
	TEST_CHECK(bRes == true);
	TEST_CHECK_EQUAL(0xF80011u, uLine1718);

	// Failure: both undefined (PARSE_FAILED) -> return false, uLine1718 set to
	// PARSE_FAILED by the function's own initializer. This pins
	// uLine1718 = PARSE_FAILED at line 433.
	vgb.uVBI[1] = VBIParse::PARSE_FAILED;
	vgb.uVBI[2] = VBIParse::PARSE_FAILED;
	uLine1718 = 0x1234;
	bRes = VBIParse::GetBestLine1718(uLine1718, vgb, NTSC);
	TEST_CHECK(bRes == false);
	TEST_CHECK_EQUAL((unsigned int) VBIParse::PARSE_FAILED, uLine1718);
}

TEST(VBIParse, vbi_get_best_line1718)
{
	test_vbi_get_best_line1718();
}

void test_vbi_verify_warnings()
{
	// Triggers each branch of VerifyVBIData's picture-number checks so the
	// warning strings that go into lstWarnings are pinned (pins the
	// `s += ...` chains and the `bSuspicious = true` assign_const mutants,
	// and the `jump > 1` vs `jump >= 1` comparator).
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	// Descending picture number: 5 then 3.
	pVBI->ClearVBIData();
	VBI_t vbi;
	memset(&vbi, 0, sizeof(vbi));
	vbi.uVBI[1] = vbi.uVBI[2] = 0xF80005;
	pVBI->AddVBIData(vbi);
	vbi.uVBI[1] = vbi.uVBI[2] = 0xF80003;
	pVBI->AddVBIData(vbi);

	bool bVer = pVBI->VerifyVBIData(NTSC, false);	// no auto-fix so warnings stick
	TEST_CHECK(bVer == true);
	list<string> warnings = pVBI->GetWarnings();
	TEST_CHECK(warnings.size() >= 1);

	// Picture number jumping forward by more than 1.
	pVBI->ClearVBIData();
	vbi.uVBI[1] = vbi.uVBI[2] = 0xF80001;
	pVBI->AddVBIData(vbi);
	vbi.uVBI[1] = vbi.uVBI[2] = 0xF80005;	// +4 jump
	pVBI->AddVBIData(vbi);

	bVer = pVBI->VerifyVBIData(NTSC, false);
	TEST_CHECK(bVer == true);
	warnings = pVBI->GetWarnings();
	TEST_CHECK(warnings.size() >= 1);

	// Conflict between line17 and line18 that cannot be resolved -> error.
	pVBI->ClearVBIData();
	memset(&vbi, 0, sizeof(vbi));
	vbi.uVBI[1] = 0xF80001;
	vbi.uVBI[2] = 0xF80099;	// completely different value, not resolvable
	pVBI->AddVBIData(vbi);

	bVer = pVBI->VerifyVBIData(NTSC, false);
	TEST_CHECK(bVer == false);
	list<string> errors = pVBI->GetErrors();
	TEST_CHECK(errors.size() >= 1);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_verify_warnings)
{
	test_vbi_verify_warnings();
}

void test_vbi_get_vbi_data_out_of_range_pushes_error()
{
	// GetVBIData must push "Out of range." into m_lstErrors when stIdx is too
	// large. Pins the cxx_remove_void_call at VBIParse.cpp:1399. VerifyVBIData
	// clears the error/warning lists, so it's a clean baseline before the
	// out-of-range call.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();

	// Add one entry of good data and run VerifyVBIData to clear error list.
	VBI_t v;
	memset(&v, 0, sizeof(v));
	v.uVBI[1] = v.uVBI[2] = 0xF80001;
	pVBI->AddVBIData(v);
	(void) pVBI->VerifyVBIData(NTSC, false);

	TEST_CHECK_EQUAL((size_t) 0, pVBI->GetErrors().size());

	// index 1 is out of range (only 1 entry). Function returns false and
	// pushes exactly one "Out of range." message.
	VBI_t got;
	bool bRes = pVBI->GetVBIData(got, 1);
	TEST_CHECK(bRes == false);
	TEST_CHECK_EQUAL((size_t) 1, pVBI->GetErrors().size());
	TEST_CHECK_EQUAL(string("Out of range."), pVBI->GetLastErrorMsg());

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_get_vbi_data_out_of_range_pushes_error)
{
	test_vbi_get_vbi_data_out_of_range_pushes_error();
}

void test_vbi_compact_22_atari_pattern()
{
	// Feeds a 2:2 Atari pattern so CompactVBIData recognizes PATTERN_22_ATARI.
	// 2:2 Atari VBI has 0xF..... on dominant fields and 0xA..... (top nibble
	// 0xA is the Atari variant) on intermediate fields. This exercises the
	// branch at VBIParse.cpp:1332-1333 and the subsequent entry assignments.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();

	VBI_t vbi;
	memset(&vbi, 0, sizeof(vbi));

	// The Atari detection branch (VBIParse.cpp:1332) fires when field N has
	// header 0xF8 and field N+1 equals (next_picnum & 0xAFFFFF). So field 0
	// encodes picnum 1 fully, field 1 encodes picnum 2 masked to 0xA8,
	// field 2 encodes picnum 2 fully (treated as wrap-around to next pattern
	// after detection), etc.
	for (unsigned int pn = 1; pn <= 5; pn++)
	{
		vbi.uVBI[1] = vbi.uVBI[2] = VBIParse::DecimalToPictureNumVBI(pn, NTSC);
		pVBI->AddVBIData(vbi);

		unsigned int uAtari = VBIParse::DecimalToPictureNumVBI(pn + 1, NTSC) & 0xAFFFFF;
		vbi.uVBI[1] = vbi.uVBI[2] = uAtari;
		pVBI->AddVBIData(vbi);
	}

	list<VBICompactEntry_t> lstEntries;
	bool bRes = pVBI->CompactVBIData(lstEntries, NTSC);
	TEST_CHECK(bRes == true);
	TEST_REQUIRE(lstEntries.size() >= 1);

	VBICompactEntry_t front = lstEntries.front();
	TEST_CHECK_EQUAL(PATTERN_22_ATARI, front.typePattern);
	TEST_CHECK_EQUAL(1, front.i32StartPictureNumber);
	TEST_CHECK_EQUAL(0u, front.u32StartAbsField);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_compact_22_atari_pattern)
{
	test_vbi_compact_22_atari_pattern();
}
