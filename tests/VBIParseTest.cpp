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

static void addPicNum(VBIParse *pVBI, unsigned int uVBI)
{
	VBI_t v;
	memset(&v, 0, sizeof(v));
	v.uVBI[1] = v.uVBI[2] = uVBI;
	pVBI->AddVBIData(v);
}

void test_vbi_verify_branches()
{
	// Exercises every surviving branch of VerifyVBIData's picture-number
	// checker, plus the black+failed warning path, so that the boundary
	// comparators (>, <, ==) and the `s += ...` chains can be pinned.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	// 1. Clean ascending sequence with non-picnum spacer: no warnings.
	pVBI->ClearVBIData();
	addPicNum(pVBI, 0xF80001);
	addPicNum(pVBI, 0);
	addPicNum(pVBI, 0xF80002);
	TEST_CHECK(pVBI->VerifyVBIData(NTSC, false));
	TEST_CHECK_EQUAL((size_t) 0, pVBI->GetWarnings().size());

	// 2. Same picnum reappears after a non-picnum: expect 0 warnings. Pins
	//    the `uCurPicNum > uLastPicNum` and `uCurPicNum < uLastPicNum` boundaries.
	pVBI->ClearVBIData();
	addPicNum(pVBI, 0xF80001);
	addPicNum(pVBI, 0);
	addPicNum(pVBI, 0xF80001);
	TEST_CHECK(pVBI->VerifyVBIData(NTSC, false));
	TEST_CHECK_EQUAL((size_t) 0, pVBI->GetWarnings().size());

	// 3. Back-to-back same picnum: expect 0 warnings. Pins the `> vs >=`
	//    mutation at line 928 (the `>=` variant would enter the block and
	//    fire a "too frequent" warning).
	pVBI->ClearVBIData();
	addPicNum(pVBI, 0xF80001);
	addPicNum(pVBI, 0xF80001);
	TEST_CHECK(pVBI->VerifyVBIData(NTSC, false));
	TEST_CHECK_EQUAL((size_t) 0, pVBI->GetWarnings().size());

	// 4. Jump forward by 2 with one spacer: "jumps" warning. Pins the
	//    `diff > 1` boundary and the sub_to_add mutation on diff.
	pVBI->ClearVBIData();
	addPicNum(pVBI, 0xF80001);
	addPicNum(pVBI, 0);
	addPicNum(pVBI, 0xF80003);
	TEST_CHECK(pVBI->VerifyVBIData(NTSC, false));
	TEST_CHECK_EQUAL((size_t) 1, pVBI->GetWarnings().size());

	// 5. Two consecutive picnums (picnum 1 then picnum 2 directly): expect
	//    "too frequent" warning. Pins the `uFieldsSincePicNum < 1` boundary.
	pVBI->ClearVBIData();
	addPicNum(pVBI, 0xF80001);
	addPicNum(pVBI, 0xF80002);
	TEST_CHECK(pVBI->VerifyVBIData(NTSC, false));
	TEST_CHECK_EQUAL((size_t) 1, pVBI->GetWarnings().size());

	// 6. Descending picnum: expect "less than" warning. Pins the `<` branch
	//    and the `uCurPicNum < uLastPicNum` comparator.
	pVBI->ClearVBIData();
	addPicNum(pVBI, 0xF80005);
	addPicNum(pVBI, 0);
	addPicNum(pVBI, 0xF80003);
	TEST_CHECK(pVBI->VerifyVBIData(NTSC, false));
	TEST_CHECK_EQUAL((size_t) 1, pVBI->GetWarnings().size());

	// 7. Black-line / failed-line combo: expect a "black line" warning.
	//    Pins the `(uVBI17 == PARSE_BLACK) && (uVBI18 == PARSE_FAILED)`
	//    predicate and the ||-partner.
	pVBI->ClearVBIData();
	{
		VBI_t v;
		memset(&v, 0, sizeof(v));
		v.uVBI[1] = VBIParse::PARSE_BLACK;
		v.uVBI[2] = VBIParse::PARSE_FAILED;
		pVBI->AddVBIData(v);
		(void) pVBI->VerifyVBIData(NTSC, false);
	}
	TEST_CHECK(pVBI->GetWarnings().size() >= 1);

	// 8. Swapped variant: line18 black, line17 failed (pins the ||-partner
	//    in the predicate).
	pVBI->ClearVBIData();
	{
		VBI_t v;
		memset(&v, 0, sizeof(v));
		v.uVBI[1] = VBIParse::PARSE_FAILED;
		v.uVBI[2] = VBIParse::PARSE_BLACK;
		pVBI->AddVBIData(v);
		(void) pVBI->VerifyVBIData(NTSC, false);
	}
	TEST_CHECK(pVBI->GetWarnings().size() >= 1);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_verify_branches)
{
	test_vbi_verify_branches();
}

void test_vbi_is_white_flag()
{
	// Synthetic RGB line: a real white flag has ~5% leading black followed by
	// ~95% white, so the long "high" width exceeds 75% of the line.
	const unsigned int width = 100;
	unsigned char line[width * 3];

	// Case 1: 10 black + 80 white + 10 black -> 2 widths {10, 80}, 80/100 > 0.75 -> TRUE
	memset(line, 0, sizeof(line));
	for (unsigned int i = 10; i < 90; i++)
	{
		line[i*3 + 0] = 255;
		line[i*3 + 1] = 255;
		line[i*3 + 2] = 255;
	}
	TEST_CHECK(VBIParse::IsWhiteFlag(line, width) == true);

	// Case 2: 10 black + 30 white + 60 black -> 2 widths {10, 30}, 30/100 < 0.75 -> FALSE
	memset(line, 0, sizeof(line));
	for (unsigned int i = 10; i < 40; i++)
	{
		line[i*3 + 0] = 255;
		line[i*3 + 1] = 255;
		line[i*3 + 2] = 255;
	}
	TEST_CHECK(VBIParse::IsWhiteFlag(line, width) == false);

	// Case 3: all black -> high threshold is 0 or very low, GetHighLowWidths
	// returns false -> IsWhiteFlag returns false.
	memset(line, 0, sizeof(line));
	TEST_CHECK(VBIParse::IsWhiteFlag(line, width) == false);

	// Case 4: exactly 75% white (boundary). 10 black + 75 white + 15 black ->
	// 75/100 = 0.75, not strictly greater than 0.75, so NOT a white flag.
	// Pins the `> 0.75` comparator at line 213 (a `>=` variant would pass
	// 0.75 >= 0.75 and return true).
	memset(line, 0, sizeof(line));
	for (unsigned int i = 10; i < 85; i++)
	{
		line[i*3 + 0] = 255;
		line[i*3 + 1] = 255;
		line[i*3 + 2] = 255;
	}
	TEST_CHECK(VBIParse::IsWhiteFlag(line, width) == false);
}

TEST(VBIParse, vbi_is_white_flag)
{
	test_vbi_is_white_flag();
}

void test_vbi_load_bad_version_pushes_error()
{
	// Pins the m_lstErrors.push_back("Unknown VBI version header.") call at
	// VBIParse.cpp:760 (cxx_remove_void_call). Clear errors via Verify, then
	// call LoadVBIData with a bad header and verify that exactly one new error
	// appears with the expected text.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();
	VBI_t v;
	memset(&v, 0, sizeof(v));
	v.uVBI[1] = v.uVBI[2] = 0xF80001;
	pVBI->AddVBIData(v);
	(void) pVBI->VerifyVBIData(NTSC, false);
	TEST_CHECK_EQUAL((size_t) 0, pVBI->GetErrors().size());

	unsigned char badHeader[14] = { 'X', 'V', 'B', 'I', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	bool b = pVBI->LoadVBIData(badHeader, sizeof(badHeader));
	TEST_CHECK(b == false);
	TEST_CHECK_EQUAL((size_t) 1, pVBI->GetErrors().size());
	TEST_CHECK_EQUAL(string("Unknown VBI version header."), pVBI->GetLastErrorMsg());

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_load_bad_version_pushes_error)
{
	test_vbi_load_bad_version_pushes_error();
}

void test_vbi_compact_lead_in_out_transitions()
{
	// Feed lead-in -> picnums -> lead-out so CompactVBIData produces three
	// entries: LEADIN, 22, LEADOUT. This covers additional branches of the
	// "seeking new pattern" switch at the end of CompactVBIData (lead-in
	// detection at 0x88FFFF and lead-out at 0x80EEEE).
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();

	pVBI->AddVBIData(VBIParse::GenerateVBILeadIn());
	pVBI->AddVBIData(VBIParse::GenerateVBILeadIn());

	for (unsigned int pn = 1; pn <= 4; pn++)
	{
		pVBI->AddVBIData(VBIParse::GenerateVBIFrame(pn, NTSC));
		pVBI->AddVBIData(VBIParse::GenerateVBIEmpty());
	}

	pVBI->AddVBIData(VBIParse::GenerateVBILeadOut());
	pVBI->AddVBIData(VBIParse::GenerateVBILeadOut());

	list<VBICompactEntry_t> lst;
	bool b = pVBI->CompactVBIData(lst, NTSC);
	TEST_CHECK(b == true);
	TEST_REQUIRE_EQUAL((size_t) 3, lst.size());

	VBICompactEntry_t a = lst.front(); lst.pop_front();
	TEST_CHECK_EQUAL(PATTERN_LEADIN, a.typePattern);
	TEST_CHECK_EQUAL(0u, a.u32StartAbsField);

	VBICompactEntry_t b2 = lst.front(); lst.pop_front();
	TEST_CHECK_EQUAL(PATTERN_22, b2.typePattern);
	TEST_CHECK_EQUAL(2u, b2.u32StartAbsField);	// after 2 lead-in fields
	TEST_CHECK_EQUAL(1, b2.i32StartPictureNumber);

	VBICompactEntry_t c = lst.front(); lst.pop_front();
	TEST_CHECK_EQUAL(PATTERN_LEADOUT, c.typePattern);
	TEST_CHECK_EQUAL(10u, c.u32StartAbsField);	// 2 lead-in + 8 pattern fields

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_compact_lead_in_out_transitions)
{
	test_vbi_compact_lead_in_out_transitions();
}

void test_vbi_compact_23_pattern()
{
	// Exercise 2:3 (PATTERN_23) detection in CompactVBIData. 2:3 is a 5-field
	// cycle: picnum, black, picnum+1, black, black.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();

	for (unsigned int pn = 1; pn <= 4; pn++)
	{
		pVBI->AddVBIData(VBIParse::GenerateVBIFrame(2*pn - 1, NTSC));
		pVBI->AddVBIData(VBIParse::GenerateVBIEmpty());
		pVBI->AddVBIData(VBIParse::GenerateVBIFrame(2*pn, NTSC));
		pVBI->AddVBIData(VBIParse::GenerateVBIEmpty());
		pVBI->AddVBIData(VBIParse::GenerateVBIEmpty());
	}

	list<VBICompactEntry_t> lst;
	bool b = pVBI->CompactVBIData(lst, NTSC);
	TEST_CHECK(b == true);
	TEST_REQUIRE(lst.size() >= 1);

	VBICompactEntry_t a = lst.front();
	TEST_CHECK_EQUAL(PATTERN_23, a.typePattern);
	TEST_CHECK_EQUAL(1, a.i32StartPictureNumber);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_compact_23_pattern)
{
	test_vbi_compact_23_pattern();
}

void test_vbi_compact_clears_existing_entries()
{
	// Pins the `lstEntries.clear()` at VBIParse.cpp:1085. Pre-populate the list
	// with a fake entry; CompactVBIData must clear it before emitting its own.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();
	for (unsigned int pn = 1; pn <= 4; pn++)
	{
		pVBI->AddVBIData(VBIParse::GenerateVBIFrame(pn, NTSC));
		pVBI->AddVBIData(VBIParse::GenerateVBIEmpty());
	}

	list<VBICompactEntry_t> lstEntries;
	VBICompactEntry_t fake;
	memset(&fake, 0, sizeof(fake));
	fake.typePattern = PATTERN_LEADOUT;
	fake.i32StartPictureNumber = 9999;
	lstEntries.push_back(fake);
	lstEntries.push_back(fake);

	TEST_CHECK(pVBI->CompactVBIData(lstEntries, NTSC));

	// The first entry must be the PATTERN_22 one that the compactor emits, not
	// the fake PATTERN_LEADOUT that we pre-loaded.
	TEST_REQUIRE(lstEntries.size() >= 1);
	VBICompactEntry_t front = lstEntries.front();
	TEST_CHECK_EQUAL(PATTERN_22, front.typePattern);
	TEST_CHECK_EQUAL(1, front.i32StartPictureNumber);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_compact_clears_existing_entries)
{
	test_vbi_compact_clears_existing_entries();
}

void test_vbi_picnum_from_line17_fallback()
{
	// When line 18 is PARSE_FAILED, the compactor falls back to line 17 (pins
	// the assign at VBIParse.cpp:1102). Build 4 fields where every line-18 is
	// PARSE_FAILED but line-17 carries a proper 2:2 pattern.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();
	for (unsigned int pn = 1; pn <= 4; pn++)
	{
		// Picnum field: line 17 carries the picnum, line 18 is PARSE_FAILED so
		// the compactor must fall back to line 17 to recognise it.
		VBI_t v;
		memset(&v, 0, sizeof(v));
		v.uVBI[1] = VBIParse::DecimalToPictureNumVBI(pn, NTSC);
		v.uVBI[2] = VBIParse::PARSE_FAILED;
		pVBI->AddVBIData(v);

		// Spacer (PARSE_BLACK) field — not exercising the fallback here.
		memset(&v, 0, sizeof(v));
		v.uVBI[1] = 0;
		v.uVBI[2] = 0;	// PARSE_BLACK
		pVBI->AddVBIData(v);
	}

	list<VBICompactEntry_t> lst;
	TEST_CHECK(pVBI->CompactVBIData(lst, NTSC));
	TEST_REQUIRE(lst.size() >= 1);

	VBICompactEntry_t front = lst.front();
	TEST_CHECK_EQUAL(PATTERN_22, front.typePattern);
	TEST_CHECK_EQUAL(1, front.i32StartPictureNumber);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_picnum_from_line17_fallback)
{
	test_vbi_picnum_from_line17_fallback();
}

void test_vbi_verify_error_message_content()
{
	// Verifies the exact error-message substrings produced by the conflict
	// branch in VerifyVBIData. Pins the `s += UintHexToStr(...)` / `std::to_string`
	// chain at lines 907-911 (arithmetic and concat mutations). We do not
	// assert the full message so the test remains robust to minor formatting
	// tweaks, but we do assert that the specific hex values of line 17 and
	// line 18 appear in it.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();

	// Conflict: line 17 and line 18 are both defined but different, with no
	// previous picture number to resolve. This produces an unresolvable error.
	VBI_t v;
	memset(&v, 0, sizeof(v));
	v.uVBI[1] = 0xF80011;	// line 17
	v.uVBI[2] = 0xF80099;	// line 18
	pVBI->AddVBIData(v);

	bool b = pVBI->VerifyVBIData(NTSC, false);
	TEST_CHECK(b == false);
	list<string> errs = pVBI->GetErrors();
	TEST_REQUIRE(errs.size() >= 1);

	const string &msg = errs.front();
	// The message includes "Line 17 is F80011" and "Line 18 is F80099"
	// verbatim (UintHexToStr uppercases and drops leading zeros).
	TEST_CHECK(msg.find("F80011") != string::npos);
	TEST_CHECK(msg.find("F80099") != string::npos);
	// And the track index / field index are computed from uFieldIdx = 0 ->
	// track 0, field 0. Any mutation of `uFieldIdx*10 + 4` or `uFieldIdx >> 1`
	// would change the substring.
	TEST_CHECK(msg.find("track 0") != string::npos);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_verify_error_message_content)
{
	test_vbi_verify_error_message_content();
}

void test_vbi_compact_22_with_exactly_two_fields()
{
	// The 2:2 detection check at VBIParse.cpp:1287 requires stFieldsRemaining
	// >= 2. With exactly 2 fields of (picnum, BLACK), the original passes
	// (>=) and produces PATTERN_22. A `>` mutation would reject and fall
	// through to PATTERN_PICNUM.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();
	pVBI->AddVBIData(VBIParse::GenerateVBIFrame(1, NTSC));
	pVBI->AddVBIData(VBIParse::GenerateVBIEmpty());

	list<VBICompactEntry_t> lst;
	TEST_CHECK(pVBI->CompactVBIData(lst, NTSC));
	TEST_REQUIRE(lst.size() >= 1);
	TEST_CHECK_EQUAL(PATTERN_22, lst.front().typePattern);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_compact_22_with_exactly_two_fields)
{
	test_vbi_compact_22_with_exactly_two_fields();
}

void test_vbi_compact_22_atari_with_exactly_two_fields()
{
	// The 2:2 Atari detection at VBIParse.cpp:1332 requires stFieldsRemaining
	// >= 2. With 2 fields (0xF80001 then 0xA80002), the original detects
	// PATTERN_22_ATARI; a `>` variant at line 1332:32 would fall through.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();
	VBI_t v;
	memset(&v, 0, sizeof(v));

	v.uVBI[1] = v.uVBI[2] = 0xF80001;	// picnum 1
	pVBI->AddVBIData(v);

	v.uVBI[1] = v.uVBI[2] = VBIParse::DecimalToPictureNumVBI(2, NTSC) & 0xAFFFFF;
	pVBI->AddVBIData(v);

	list<VBICompactEntry_t> lst;
	TEST_CHECK(pVBI->CompactVBIData(lst, NTSC));
	TEST_REQUIRE(lst.size() >= 1);
	TEST_CHECK_EQUAL(PATTERN_22_ATARI, lst.front().typePattern);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_compact_22_atari_with_exactly_two_fields)
{
	test_vbi_compact_22_atari_with_exactly_two_fields();
}

void test_vbi_compact_ignores_prev_field_for_first_entry()
{
	// Pins VBIParse.cpp:1213 (`if (stFieldIdx > 0)` - `>=` variant would try to
	// peek at vParseVBI[-1], which is out-of-bounds). Start with a 2:2 pattern
	// beginning at field 0 - compactor must NOT look at the (non-existent)
	// previous field.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();
	for (unsigned int pn = 1; pn <= 4; pn++)
	{
		pVBI->AddVBIData(VBIParse::GenerateVBIFrame(pn, NTSC));
		pVBI->AddVBIData(VBIParse::GenerateVBIEmpty());
	}

	list<VBICompactEntry_t> lst;
	TEST_CHECK(pVBI->CompactVBIData(lst, NTSC));
	TEST_REQUIRE(lst.size() >= 1);
	VBICompactEntry_t front = lst.front();
	TEST_CHECK_EQUAL(PATTERN_22, front.typePattern);
	TEST_CHECK_EQUAL(0u, front.u32StartAbsField);	// starts at 0, not -1
	TEST_CHECK_EQUAL(1, front.i32StartPictureNumber);
	TEST_CHECK_EQUAL((uint8_t) 0, front.u8PatternOffset);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_compact_ignores_prev_field_for_first_entry)
{
	test_vbi_compact_ignores_prev_field_for_first_entry();
}

void test_vbi_verify_cube_quest_field_index_guard()
{
	// VerifyVBIData's Cube Quest autofix is gated by `if (uFieldIdx > 2)`
	// at line 864. With uFieldIdx==2, original: skip the fix (line 18 stays
	// PARSE_FAILED). `>=` mutant: enter the fix when prev-field is a picnum
	// AND the field before that is a chapter -> overwrite line 18 with the
	// chapter code. Our test places a chapter/picnum/black-failed trio at
	// indices 0/1/2 so that the mutant's fix triggers, while the original
	// leaves field-2's line 18 untouched.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();

	// field 0: chapter
	VBI_t v;
	memset(&v, 0, sizeof(v));
	v.uVBI[1] = v.uVBI[2] = 0x885DDD;	// chapter 5
	pVBI->AddVBIData(v);

	// field 1: picnum
	memset(&v, 0, sizeof(v));
	v.uVBI[1] = v.uVBI[2] = 0xF80001;
	pVBI->AddVBIData(v);

	// field 2: line17=BLACK, line18=FAILED
	memset(&v, 0, sizeof(v));
	v.uVBI[1] = VBIParse::PARSE_BLACK;
	v.uVBI[2] = VBIParse::PARSE_FAILED;
	pVBI->AddVBIData(v);

	(void) pVBI->VerifyVBIData(NTSC, true);

	VBI_t got;
	TEST_REQUIRE(pVBI->GetVBIData(got, 2));
	// With original >, field 2's line 18 is still PARSE_FAILED. Mutant >=
	// would have fixed it to 0x885DDD.
	TEST_CHECK_EQUAL((unsigned int) VBIParse::PARSE_FAILED, got.uVBI[2]);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_verify_cube_quest_field_index_guard)
{
	test_vbi_verify_cube_quest_field_index_guard();
}

void test_vbi_verify_cube_quest_autofix_applies()
{
	// Complement of the previous test: with uFieldIdx=3 and the pattern
	// [unused, chapter, picnum, black-failed], autoFix MUST apply the fix.
	// Pins the `IsPictureNum(vParseVBI[uFieldIdx-1])` and
	// `IsChapterNum(vParseVBI[uFieldIdx-2])` calls at lines 868-869 and the
	// assignment at line 871.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();

	// field 0: anything (e.g., picnum)
	VBI_t v;
	memset(&v, 0, sizeof(v));
	v.uVBI[1] = v.uVBI[2] = 0xF80005;
	pVBI->AddVBIData(v);

	// field 1: chapter
	memset(&v, 0, sizeof(v));
	v.uVBI[1] = v.uVBI[2] = 0x885DDD;	// chapter 5
	pVBI->AddVBIData(v);

	// field 2: picnum
	memset(&v, 0, sizeof(v));
	v.uVBI[1] = v.uVBI[2] = 0xF80006;
	pVBI->AddVBIData(v);

	// field 3: line17=BLACK, line18=FAILED
	memset(&v, 0, sizeof(v));
	v.uVBI[1] = VBIParse::PARSE_BLACK;
	v.uVBI[2] = VBIParse::PARSE_FAILED;
	pVBI->AddVBIData(v);

	(void) pVBI->VerifyVBIData(NTSC, true);

	VBI_t got;
	TEST_REQUIRE(pVBI->GetVBIData(got, 3));
	// Fix must have been applied: field-3's line 18 now equals field-1's
	// line 18 (the chapter code).
	TEST_CHECK_EQUAL(0x885DDDu, got.uVBI[2]);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_verify_cube_quest_autofix_applies)
{
	test_vbi_verify_cube_quest_autofix_applies();
}

void test_vbi_compact_standalone_chapter()
{
	// Two consecutive chapter-only fields compact into a single PATTERN_ZEROES
	// entry whose u16Special has EVENT_CHAPTER set and the chapter number
	// stored in the low byte. Pins VBIParse.cpp:1350 (typePattern = PATTERN_ZEROES),
	// :1351 (u16Special = ChapterNumVBIToDecimal(...)), :1352 (u16Special |= EVENT_CHAPTER).
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();
	pVBI->AddVBIData(VBIParse::GenerateVBIChapter(5));
	pVBI->AddVBIData(VBIParse::GenerateVBIChapter(5));

	list<VBICompactEntry_t> lst;
	TEST_CHECK(pVBI->CompactVBIData(lst, NTSC));
	TEST_REQUIRE(lst.size() >= 1);

	VBICompactEntry_t e = lst.front();
	TEST_CHECK_EQUAL(PATTERN_ZEROES, e.typePattern);
	uint8_t ch = 0;
	TEST_CHECK_EQUAL(VBIC_TRUE, VBIC_GetChapterInfo(&e, &ch));
	TEST_CHECK_EQUAL(5, ch);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_compact_standalone_chapter)
{
	test_vbi_compact_standalone_chapter();
}

void test_vbi_compact_multi_entry_u32StartAbsField()
{
	// Forces CompactVBIData to produce multiple entries with non-zero
	// u32StartAbsField. Pins VBIParse.cpp:1192 (`entry.u32StartAbsField = stFieldIdx`)
	// where a mutant = 42 would collapse every second entry's start field to 42.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();

	pVBI->AddVBIData(VBIParse::GenerateVBILeadIn());
	pVBI->AddVBIData(VBIParse::GenerateVBILeadIn());
	for (unsigned int pn = 1; pn <= 4; pn++)
	{
		pVBI->AddVBIData(VBIParse::GenerateVBIFrame(pn, NTSC));
		pVBI->AddVBIData(VBIParse::GenerateVBIEmpty());
	}
	pVBI->AddVBIData(VBIParse::GenerateVBIChapter(3));
	pVBI->AddVBIData(VBIParse::GenerateVBIChapter(3));
	pVBI->AddVBIData(VBIParse::GenerateVBILeadOut());
	pVBI->AddVBIData(VBIParse::GenerateVBILeadOut());

	list<VBICompactEntry_t> lst;
	TEST_CHECK(pVBI->CompactVBIData(lst, NTSC));
	TEST_REQUIRE(lst.size() >= 4);

	// LEADIN at field 0
	VBICompactEntry_t e = lst.front(); lst.pop_front();
	TEST_CHECK_EQUAL(PATTERN_LEADIN, e.typePattern);
	TEST_CHECK_EQUAL(0u, e.u32StartAbsField);

	// PATTERN_22 at field 2 (after 2 lead-in fields)
	e = lst.front(); lst.pop_front();
	TEST_CHECK_EQUAL(PATTERN_22, e.typePattern);
	TEST_CHECK_EQUAL(2u, e.u32StartAbsField);

	// CHAPTER at field 10 (after 8 pattern fields + 2 lead-in)
	e = lst.front(); lst.pop_front();
	TEST_CHECK_EQUAL(PATTERN_ZEROES, e.typePattern);
	TEST_CHECK_EQUAL(10u, e.u32StartAbsField);

	// LEADOUT at field 12
	e = lst.front(); lst.pop_front();
	TEST_CHECK_EQUAL(PATTERN_LEADOUT, e.typePattern);
	TEST_CHECK_EQUAL(12u, e.u32StartAbsField);

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_compact_multi_entry_u32StartAbsField)
{
	test_vbi_compact_multi_entry_u32StartAbsField();
}

void test_vbi_verify_no_warnings_for_valid_sequence()
{
	// A clean 2:2 pattern of 10 frames should produce ZERO warnings and ZERO
	// errors. Anything wrong with the counter/condition mutants in VerifyVBIData
	// (e.g., uFieldsSincePicNum++ flipped to --, or `>` flipped to `>=` on
	// the picnum comparator, or the uTrackIdx computation) would manifest as
	// spurious warnings. This is a regression-style test that pins many
	// arithmetic and comparator mutations at once.
	VBIParseSPtr VBISPtr = VBIParse::GetInstance();
	VBIParse *pVBI = VBISPtr.get();

	pVBI->ClearVBIData();
	for (unsigned int pn = 1; pn <= 10; pn++)
	{
		pVBI->AddVBIData(VBIParse::GenerateVBIFrame(pn, NTSC));
		pVBI->AddVBIData(VBIParse::GenerateVBIEmpty());
	}

	TEST_CHECK(pVBI->VerifyVBIData(NTSC, false));
	TEST_CHECK_EQUAL((size_t) 0, pVBI->GetWarnings().size());
	TEST_CHECK_EQUAL((size_t) 0, pVBI->GetErrors().size());

	pVBI->ClearVBIData();
}

TEST(VBIParse, vbi_verify_no_warnings_for_valid_sequence)
{
	test_vbi_verify_no_warnings_for_valid_sequence();
}

