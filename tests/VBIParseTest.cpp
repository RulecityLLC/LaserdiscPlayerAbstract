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
