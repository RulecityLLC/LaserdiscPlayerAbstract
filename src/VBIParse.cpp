#include <ldp-abst/VideoStandard.h>
#include <ldp-abst/VBIParse.h>
#include <cstring>	// for memcpy
#include <sstream>
#include <list>

using namespace std;

#define READ_INTENSITY(p8) ((*p8 + *(p8+1) + *(p8+2)) / 3.0)

// version ID that must come at the beginning of the VBI file
unsigned char VBIParse::arrVersion1[4] = { '1', 'V', 'B', 'I' };

string UintHexToStr(unsigned int src)
{
	stringstream ss;
	// Set the output format to hexadecimal
	ss << std::hex << std::uppercase << src;
	return ss.str();
}

bool VBIParse::ParseRGBManchester(unsigned int &uResult, const unsigned char *p8Line, unsigned int uLineWidth)
{
	list<unsigned int>::iterator li;
	bool bConfident = true;
//	unsigned int uIdx = 0;

//	const unsigned char *p8Ptr = p8Line;
	uResult = PARSE_FAILED;

	// this is generously conservative
	double dMaxWidth = uLineWidth / 24.0;

	// Step 1: get all transition widths so we can figure out clock width
//	bool bHigh = false;	// assume we start off low
	list <unsigned int> lstTransWidths;
	unsigned int uCurWidth = 0;

	double dHighThreshold = 0.0;
	double dLowThreshold = 0.0;

	// need to detect whether this is just a black line
	if (!GetThresholds(dHighThreshold, dLowThreshold, p8Line, uLineWidth))
	{
		return false;
	}

	// is this just a black line?
	if (dHighThreshold < 100.0)
	{
		uResult = PARSE_BLACK;
		return true;	// and we return true to show confidence
	}

	if (!GetHighLowWidths(lstTransWidths, dHighThreshold, dLowThreshold, p8Line, uLineWidth))
	{
		return false;
	}

	// get rid of first width because it is a lead-in width and not useful
	lstTransWidths.erase(lstTransWidths.begin());

	// this happens if there was only one high/low width (the leading black)
	if (lstTransWidths.empty())
	{
		return false;
	}

	// step 1.5: fix any errors in widths
	FixManchesterWidths(lstTransWidths, uLineWidth);

	// step 2: figure out max and mins of each width so we can come up with a threshold
	li = lstTransWidths.begin();

	unsigned int uMaxWidth = 0;
	unsigned int uMinWidth = (unsigned int) (dMaxWidth + 0.5);	// start high

	// NOTE : the last width (trailing black space) was never included
	while (li != lstTransWidths.end())
	{
		uCurWidth = *li;
		if (uCurWidth > uMaxWidth) uMaxWidth = uCurWidth;
		else if (uCurWidth < uMinWidth) uMinWidth = uCurWidth;
		li++;
	}

	double dMidWidth = (uMaxWidth + uMinWidth) / 2.0;
	unsigned int uMidWidth = (unsigned int) (dMidWidth + 0.5);

	// step 3 : split into equal sized highs and lows to make things clearer
	// (this will hurt performance, but VBI parsing is a one-time thing so I'd rather have the procedure clear)
	list <unsigned int> lstClocks;
	li = lstTransWidths.begin();

	// figure out first bit, it's a special case because it has no previous history
	uCurWidth = *li;	// first transition, part of which is in first bit

	// the first transition will _always_ be from low to high, by design

	// if it's long, then first clocks are low, high, high
	if (uCurWidth > uMidWidth)
	{
		lstClocks.push_back(0);
		lstClocks.push_back(1);
		lstClocks.push_back(1);
	}
	// else if it's short, then first clocks is just high
	else
	{
		lstClocks.push_back(0);
		lstClocks.push_back(1);
	}
	unsigned int uBit = 1;	// at this point, we will be high no matter what
	li++;

	// Whether any short intervals are extra short;
	// This helps us correct problems where high signals drop off prematurely
//	bool bExtraShort = false;

	while (li != lstTransWidths.end())
	{
		// whether we are at the beginning of a new bit
		// (used to help fix problems of high signals dropping off prematurely)
//		bool bBitStart = ((lstClocks.size() & 1) == 0);

		uCurWidth = *li;
		uBit ^= 1;	// flip bit

		// if it's long
		if (uCurWidth > uMidWidth)
		{
			lstClocks.push_back(uBit);
			lstClocks.push_back(uBit);
		}
		// else if it's short, the next clock will be bit
		else
		{
			lstClocks.push_back(uBit);

			// track to see if current width is extra short; it could mean the next width will be extra long
//			bExtraShort = (uCurWidth < uMinClockWidth);
		}
		li++;
	}

	// check to see if last transition was high->low, in which case it was not accounted for
	if ((lstClocks.size() == 47) && (uBit == 1))
	{
		lstClocks.push_back(0);	// the last bit will be a 0
	}

	// if we don't have the correct count here, we can't be confident in our result
	if (lstClocks.size() != 48)
	{
		return false;
	}

	// step 4: read the clocks to get our final value
	uResult = 0;
	li = lstClocks.begin();
	while (li != lstClocks.end())
	{
		unsigned int uBitHalf0 = *li;
		li++;
		unsigned int uBitHalf1 = *li;
		li++;

		uResult <<= 1;	// shift left by 1

		// low->high is 1
		if ((uBitHalf0 == 0) && (uBitHalf1 == 1))
		{
			uResult |= 1;
		}
		// else high->low is 0
		else if ((uBitHalf0 == 1) && (uBitHalf1 == 0))
		{
			//else uResult |= 0;
		}
		// else they are both 0 or both 1 which is a parse error
		else
		{
			uResult = PARSE_FAILED;
			bConfident = false;
			break;
		}
	}

	return bConfident;
}

bool VBIParse::IsWhiteFlag(const unsigned char *p8Line, unsigned int uLineWidth)
{
	bool bRes = false;

	list <unsigned int> lstWidths;
	list <unsigned int>::iterator li;
	if (!GetHighLowWidths(lstWidths, p8Line, uLineWidth))
	{
		return false;
	}

	// a white flag will have 2 entries because the last entry (trailing black space) is ignored
	// The first entry will be a short black, followed by a long white
	if (lstWidths.size() == 2)
	{
		li = lstWidths.begin();
		li++;	// go to the second entry

		double dWidth = (double) *li;

		// if the long line occupies more than a certain percentage of the line, then we dub it a white flag
		if ((dWidth / uLineWidth) > 0.75)
		{
			bRes = true;
		}
	}

	return bRes;
}

bool VBIParse::GetThresholds(double &dHighThreshold, double &dLowThreshold, const unsigned char *p8Line,
	unsigned int uLineWidth)
{
	unsigned int uIdx = 0;
	const unsigned char *p8Ptr = p8Line;
	double dMaxIntensity = 0.0, dMinIntensity = 255.0;	// start at extremes

	for (uIdx = 0; uIdx < uLineWidth; uIdx++)
	{
		double dIntensity = READ_INTENSITY(p8Ptr);	// get intensity value by averaging components together

		if (dIntensity > dMaxIntensity)
		{
			dMaxIntensity = dIntensity;
		}
		else if (dIntensity < dMinIntensity)
		{
			dMinIntensity = dIntensity;
		}

		p8Ptr += 3;	// skip to the next pixel
	}

	double dMidIntensity = (dMaxIntensity + dMinIntensity) / 2.0;

	// a 'high' value will be greater than this value
	dHighThreshold = dMidIntensity + ((dMaxIntensity - dMidIntensity) / 2.0);

	// a 'low' value will be less than this value
	dLowThreshold = ((dMidIntensity - dMinIntensity) / 2.0) + dMinIntensity;

	return true;
}

bool VBIParse::GetHighLowWidths(list <unsigned int> &lstTransWidths,
	const unsigned char *p8Line, unsigned int uLineWidth)
{
	double dHighThreshold = 0.0, dLowThreshold = 0.0;
	bool bRes = GetThresholds(dHighThreshold, dLowThreshold, p8Line, uLineWidth);

	if (bRes)
	{
		// if the high threshold is too low, it could mean the line is just noisy black
		if (dHighThreshold < 100.0)
		{
			bRes = false;
		}

		else
		{
			bRes = GetHighLowWidths(lstTransWidths,
				dHighThreshold, dLowThreshold,
				p8Line, uLineWidth);
		}
	}

	return bRes;
}

bool VBIParse::GetHighLowWidths(list <unsigned int> &lstTransWidths,
	double dHighThreshold, double dLowThreshold,								
	const unsigned char *p8Line, unsigned int uLineWidth)
{
	bool bRes = true;
	const unsigned char *p8Ptr = NULL;
	unsigned int uIdx = 0;

	bool bHigh = false;	// VBI data must start off low by definition
	p8Ptr = p8Line;
	lstTransWidths.clear();
	unsigned int uCurWidth = 0;
	for (uIdx = 0; uIdx < uLineWidth; uIdx++)
	{
		double dIntensity = READ_INTENSITY(p8Ptr);

		// if current pixel adds to the current width
		if (
			((dIntensity < dLowThreshold) && !bHigh) ||
			((dIntensity > dHighThreshold) && bHigh)
			)
		{
			uCurWidth++;
		}
		// else the width is changing so start over
		else if (
			(dIntensity < dLowThreshold) ||
			(dIntensity > dHighThreshold)
			)
		{
			bHigh = !bHigh;
			lstTransWidths.push_back(uCurWidth);	// add previous width to list
			uCurWidth = 1;	// start over with 1 because the next width is at least 1 pixel wide (or we wouldn't be here)
		}
		// else it is neutral, therefore it must be ignored because it could be the beginning or end of a new transition

		p8Ptr += 3;	// go to next pixel
	}

	// safety check: if this happens, we can't continue
	if (lstTransWidths.empty())
	{
		bRes = false;
	}

	return bRes;
}

bool VBIParse::FixManchesterWidths(list <unsigned int> &lstTransWidths, unsigned int uLineWidth)
{
	bool bRes = true;
	list <unsigned int>::const_iterator li;
	list <unsigned int> lstNew;

	// a small width cannot be bigger than this value because there must be 48 of these.
	// All small widths must be smaller than this value, and all large widths must be greater.
	// We use this value to determine a rough small clock width for this line.
	double dMaxShortWidth = uLineWidth / 48.0;

	// conversative estimate of the smallest possible valid width
	double dMinShortWidth = uLineWidth / (48.0 * 1.66);

	// rough estimate of how wide the short width is
	double dRoughShortWidth = 0.0;

	li = lstTransWidths.begin();
	unsigned int u = *li;

	// Use first high as an estimate for the short width length.
	// If it becomes necessary, we could scan through all widths and come up with a good average.
	// if first high is long
	if (u > dMaxShortWidth)
	{
		dRoughShortWidth = u * 0.5;
	}
	else
	{
		dRoughShortWidth = u;
	}

	// safety check, should never happen
	if ((dRoughShortWidth < dMinShortWidth) || (dRoughShortWidth > dMaxShortWidth))
	{
		return false;
	}

	lstNew.push_back(u);	// store initial value
	li++;

	// how much to adjust two neighboring widths
	unsigned int uFudge = 0;

	// scan through and make sure no widths are too short (that is the real problem we are trying to address)
	while (li != lstTransWidths.end())
	{
		u = *li;

		// if we previously added to a value, then we need to compensate by subtracting from this value
		if (uFudge != 0)
		{
			// make sure we don't go negative
			if (u > uFudge)
			{
				u -= uFudge;

				// if our adjustment messed up the other portion, than just set it to the rough width
				// (this can happen if we ignore the 'grey' values that are neither high nor low)
				if (u < dMinShortWidth)
				{
					u = (unsigned int) (dRoughShortWidth + 0.5);
				}

				uFudge = 0;
			}
		}

		// if width is too short
		else if (u < dMinShortWidth)
		{
			// add enough so that we are about right
			uFudge = (unsigned int) (dRoughShortWidth - u + 0.5);
			u += uFudge;
		}

		// store adjusted value
		lstNew.push_back(u);

		li++;
	}

	if (bRes)
	{
		lstTransWidths = lstNew;
	}

	return bRes;
}

bool VBIParse::GetBestLine1718(unsigned int &uLine1718, const VBI_t &vbi,
	VideoStandard_t vidStd,
	unsigned int uPreviousPictureNumVBI)
{
	unsigned int uLine17 = vbi.uVBI[1];
	unsigned int uLine18 = vbi.uVBI[2];
	return GetBestLine1718(uLine1718, uLine17, uLine18, vidStd, uPreviousPictureNumVBI);
}

bool VBIParse::GetBestLine1718(unsigned int &uLine1718, unsigned int uLine17, unsigned int uLine18, VideoStandard_t vidStd,
	unsigned int uPreviousPictureNumVBI)
{
	bool bRes = false;
	
	uLine1718 = PARSE_FAILED;	// make sure it returns 'undefined' if we don't define something

	// If they're already the same AND they are both known quantities, we're done
	// (we want to know if both lines had HIGHs on them and could not be parsed; this would be a situation where the two results may _not_ equal if they had parsed correctly)
	if ((uLine17 == uLine18) && (uLine17 != PARSE_FAILED))
	{
		uLine1718 = uLine17;
		bRes = true;
	}

	// if one line parsed and the other didn't or is black, defer to the parsed line
	else if (!IsVBIDefined(uLine17) && IsVBIDefined(uLine18))
	{
		uLine1718 = uLine18;
		bRes = true;
	}

	// if one line parsed and the other didn't or is black, defer to the parsed line
	else if (IsVBIDefined(uLine17) && !IsVBIDefined(uLine18))
	{
		uLine1718 = uLine17;
		bRes = true;
	}

	// else if one line is black and the other is undefined
	else if ((uLine17 == PARSE_BLACK) && (uLine18 == PARSE_FAILED))
	{
		uLine1718 = uLine17;
		bRes = true;
	}
	else if ((uLine18 == PARSE_BLACK) && (uLine17 == PARSE_FAILED))
	{
		uLine1718 = uLine18;
		bRes = true;
	}

	// else if both lines parsed and are different and we have a previous picture number to go from
	else if (IsVBIDefined(uLine17) && IsVBIDefined(uLine18) && (uPreviousPictureNumVBI != 0))
	{
		// make sure all numbers are picture numbers and not some other type of number
		if (
			IsPictureNum(uLine17) &&
			IsPictureNum(uLine18) &&
			IsPictureNum(uPreviousPictureNumVBI)
			)
		{
			// must compare decimal values (because 9 + 1 in hex is 0xA, not 0x10 as we need it to be)
			unsigned int uLine17Dec = PictureNumVBIToDecimal(uLine17, vidStd);
			unsigned int uLine18Dec = PictureNumVBIToDecimal(uLine18, vidStd);
			unsigned int uPreviousPictureNumVBIDec = PictureNumVBIToDecimal(uPreviousPictureNumVBI, vidStd);

			// if line 17 is the next picture number up from the last one
			if (uLine17Dec == (uPreviousPictureNumVBIDec + 1))
			{
				uLine1718 = uLine17;
				bRes = true;
			}
			else if (uLine18Dec == (uPreviousPictureNumVBIDec + 1))
			{
				uLine1718 = uLine18;
				bRes = true;
			}
			// else there's nothing we can do, we're toast
		}
	}

	return bRes;
}

bool VBIParse::IsPictureNum(unsigned int uVBI)
{
	bool bRes = ((uVBI & 0xF00000) == 0xF00000);
	return bRes;
}

bool VBIParse::IsChapterNum(unsigned int uVBI)
{
	bool bRes = ((uVBI & 0xF00FFF) == 0x800DDD);
	return bRes;
}

bool VBIParse::IsStopCode(unsigned int uVBI)
{
	return (uVBI == 0x82CFFF);
}

unsigned int VBIParse::PictureNumVBIToDecimal(unsigned int uVBI, VideoStandard_t vidStd)
{
	// isolate BCD picture number.  NTSC has an extra bit that is not part of the picture number so we have to check the video standard.

	if (vidStd == NTSC)
	{
		uVBI &= 0x7FFFF;
	}
	else
	{
		uVBI &= 0xFFFFF;	// isolate BCD picture number
	}

	string intermediate = UintHexToStr(uVBI);
	unsigned int uResult = 0;

	// it's possible for the conversion to fail in which case we return '0' to match legacy behavior
	try
	{
		uResult = std::stoi(intermediate);
	}
	catch (std::invalid_argument const& ex)
	{
		uResult = 0;
	}

	return uResult;
}

unsigned int VBIParse::ChapterNumVBIToDecimal(unsigned int uVBI)
{
	uVBI = (uVBI >> 12) & 0x7F;	// isolate BCD chapter number

	std::stringstream ss;

	// Set the output format to hexadecimal
	ss << std::hex << std::uppercase << uVBI;
	unsigned int uResult = std::stoi(ss.str());

	return uResult;
}

unsigned int VBIParse::DecimalToPictureNumVBI(unsigned int uNum, VideoStandard_t vidStd)
{
	// convert frame number to BCD
	string strFrame = std::to_string(uNum);
	
	// convert back to uint, assuming source was hex
	unsigned int uResult = std::stoi(strFrame, nullptr, 16);

	// add header.
	// NTSC has an extra bit that means something else, so we have to check which video standard we're using.
	if (vidStd == NTSC)
	{
		uResult |= 0xF80000;
	}
	else
	{
		uResult |= 0xF00000;
	}

	return uResult;
}

unsigned int VBIParse::DecimalToChapterNumVBI(unsigned int uNum)
{
	// convert frame number to BCD
	string strFrame = std::to_string(uNum);
	
	// convert back to uint, assuming source was hex
	unsigned int uResult = std::stoi(strFrame, nullptr, 16);
	uResult = 0x880DDD | (uResult << 12);

	return uResult;
}

VBI_t VBIParse::GenerateVBIFrame(unsigned int uFrameNum, VideoStandard_t vidStd)
{
	VBI_t vbi;

	// white flag is only used in NTSC and often ignored, so when we are generating fake VBI, we just always set it to false to keep things simple
	vbi.bWhiteFlag = false;

	vbi.uVBI[0] = 0;
	vbi.uVBI[1] = DecimalToPictureNumVBI(uFrameNum, vidStd);
	vbi.uVBI[2] = vbi.uVBI[1];
	return vbi;
}

VBI_t VBIParse::GenerateVBIEmpty()
{
	VBI_t vbi;
	vbi.bWhiteFlag = false;
	vbi.uVBI[0] = 0;
	vbi.uVBI[1] = 0;
	vbi.uVBI[2] = 0;
	return vbi;
}

VBI_t VBIParse::GenerateVBIChapter(unsigned int uChapterNum)
{
	VBI_t vbi;
	vbi.bWhiteFlag = false;
	vbi.uVBI[0] = 0;
	vbi.uVBI[1] = DecimalToChapterNumVBI(uChapterNum);
	vbi.uVBI[2] = vbi.uVBI[1];
	return vbi;
}

VBI_t VBIParse::GenerateVBIStopCode()
{
	VBI_t vbi;
	vbi.bWhiteFlag = false;
	vbi.uVBI[0] = 0x82CFFF;
	vbi.uVBI[1] = 0x82CFFF;
	vbi.uVBI[2] = 0;
	return vbi;
}

VBI_t VBIParse::GenerateVBILeadIn()
{
	VBI_t vbi;
	vbi.bWhiteFlag = false;
	vbi.uVBI[0] = 0;
	vbi.uVBI[1] = 0x88FFFF;
	vbi.uVBI[2] = 0x88FFFF;
	return vbi;
}

VBI_t VBIParse::GenerateVBILeadOut()
{
	VBI_t vbi;
	vbi.bWhiteFlag = false;
	vbi.uVBI[0] = 0;
	vbi.uVBI[1] = 0x80EEEE;
	vbi.uVBI[2] = 0x80EEEE;
	return vbi;
}

bool VBIParse::IsVBIDefined(unsigned int uVBI)
{
	// valid VBI data must have the most significant bit set
	return ((uVBI >> 23) != 0);
}

/////////////////////////

VBIParseSPtr VBIParse::GetInstance()
{
	return VBIParseSPtr(new VBIParse(), VBIParse::deleter());
}

void VBIParse::DeleteInstance()
{
	delete this;
}

VBIParse::VBIParse()
{
}

bool VBIParse::ParseRGBField(VBI_t &vbiDst, const unsigned char *p8Field, unsigned int uLineWidth)
{
	bool bRes = true;
	unsigned int uPitch = uLineWidth * 3;
	const unsigned char *p8Line = NULL;

	// do line 11
	p8Line = p8Field + (uPitch * 11);
	vbiDst.bWhiteFlag = VBIParse::IsWhiteFlag(p8Line, uLineWidth);

	// do lines 16-18
	for (unsigned int uIdx = 0; uIdx <= 2; uIdx++)
	{
		unsigned int uLine = uIdx + 16;
		p8Line = p8Field + (uPitch * uLine);

		// it doesn't matter if parse fails because we will be checking things later with the VerifyVBI function
		VBIParse::ParseRGBManchester(vbiDst.uVBI[uIdx],
			p8Line, uLineWidth);
	}

	return bRes;
}

void VBIParse::AddVBIData(const VBI_t &vbiSrc)
{
	m_vParseVBI.push_back(vbiSrc);
}

/*
 packed format (each section of VBI data takes 10 bytes):
byte	value
0		white flag (1 = yes, 0 = no)
1-3		VBI 16, big endian
4-6		VBI 17, big endian
7-9		VBI 18, big endian
 */

string VBIParse::SaveVBIData()
{
	string strRes;	// the string that holds binary result
	size_t stEntryCount = m_vParseVBI.size();
	size_t stByteCount = (stEntryCount * sizeof(VBIPacked_t)) + sizeof(arrVersion1);
	shared_ptr<unsigned char[]> BufSA(new unsigned char[stByteCount]);
	unsigned char *pBuf = BufSA.get();

	// add version header to beginning
	memcpy(pBuf, arrVersion1, sizeof(arrVersion1));
	pBuf += sizeof(arrVersion1);	// change pointer to begin writing data

	vector<VBI_t>::const_iterator vi;
	for (vi = m_vParseVBI.begin(); vi != m_vParseVBI.end(); vi++)
	{
		VBI_t vbi = *vi;

		// do white flag
		*(pBuf++) = (unsigned char) vbi.bWhiteFlag;	// I hope this goes to 0 or 1 hehe
		
		// lines 16-18
		for (unsigned int uIdx = 0; uIdx < 3; uIdx++)
		{
			*(pBuf++) = vbi.uVBI[uIdx] >> 16;
			*(pBuf++) = (vbi.uVBI[uIdx] >> 8) & 0xFF;
			*(pBuf++) = vbi.uVBI[uIdx] & 0xFF;
		}
	}

	pBuf = BufSA.get();	// rewind to beginning
	strRes = string((char *) pBuf, stByteCount);
	return strRes;
}

bool VBIParse::LoadVBIData(const void *pBuf, size_t stSizeBytes)
{
	bool bRes = false;
	const unsigned char *p8Buf  = (const unsigned char *) pBuf;

	// check the version to make sure we support it
	if (memcmp(p8Buf, arrVersion1, sizeof(arrVersion1)) != 0)
	{
		m_lstErrors.push_back("Unknown VBI version header.");
		return false;
	}

	// adjust stSizeBytes for upcoming size check
	stSizeBytes -= sizeof(arrVersion1);
	// advance to actual VBI data
	p8Buf += sizeof(arrVersion1);

	m_vParseVBI.clear();

	// the VBI data must be a multiple of the size of the VBI packed struct because that's how big each individual entry is
	if ((stSizeBytes % sizeof(VBIPacked_t)) == 0)
	{
		size_t stByteCount = 0;
		VBI_t vbi;
		
		while (stByteCount < stSizeBytes)
		{
			// grab white flag
			vbi.bWhiteFlag = (*(p8Buf++) != 0);

			// grab all the lines
			for (unsigned int uIdx = 0; uIdx < 3; uIdx++)
			{
				vbi.uVBI[uIdx] = (*(p8Buf++) << 16);	// not using OR, because I want to clear old value
				vbi.uVBI[uIdx] |= (*(p8Buf++) << 8);
				vbi.uVBI[uIdx] |= (*(p8Buf++));
			}

			m_vParseVBI.push_back(vbi);
			stByteCount += sizeof(VBIPacked_t);
		}

		bRes = true;
	}
	// else the size is wrong
	else
	{
		string s = "Size must be a multiple of ";
		s += std::to_string(sizeof(VBIPacked_t));
		s += ".";
		m_lstErrors.push_back(s);
	}

	return bRes;
}

void VBIParse::ClearVBIData()
{
	m_vParseVBI.clear();
}

bool VBIParse::VerifyVBIData(VideoStandard_t vidStd, bool bAutoFix)
{
	return VerifyVBIData(m_vParseVBI, m_lstWarnings, m_lstErrors, vidStd, bAutoFix);
}

bool VBIParse::VerifyVBIData(vector <VBI_t> &vParseVBI, list<string> &lstWarnings, list<string> &lstErrors, VideoStandard_t vidStd, bool bAutoFix)
{
	bool bGotAtLeastOneError = false;

	vector <VBI_t>::iterator vi;
	bool bPreviouslyGotPicNum = false;
	unsigned int uLastPicNum = 0, uCurPicNum = 0;
	unsigned int uFieldsSincePicNum = 1;
	unsigned int uVBI16 = 0, uVBI17 = 0, uVBI18 = 0;
	unsigned int uVBI = 0;
	unsigned int uFieldIdx = 0;
	string s;

	// fresh set of errors/warnings for new verification
	lstWarnings.clear();
	lstErrors.clear();

	for (vi = vParseVBI.begin(); vi != vParseVBI.end(); vi++)
	{
		uVBI16 = vi->uVBI[0];
		uVBI17 = vi->uVBI[1];
		uVBI18 = vi->uVBI[2];

		unsigned int uTrackIdx = uFieldIdx >> 1;
		unsigned int uWhichField = uFieldIdx & 1;

		// Check to see if we have a black line and an unparseable line.
		// This is not an error since one line is black, it probably means the line is not important.
		// However, a human may be able to determine what the unparseable line was supposed to and fix the VBI for accuracy's sake.
		// Hence, we throw a warning.
		if (
			((uVBI17 == PARSE_BLACK) && (uVBI18 == PARSE_FAILED)) ||
			((uVBI18 == PARSE_BLACK) && (uVBI17 == PARSE_FAILED))
			)
		{
			s = "Track ";
			s += std::to_string(uTrackIdx);
			s += ", Field ";
			s += std::to_string(uWhichField);
			s += " has a black line and a non-black unparseable line.";
			lstWarnings.push_back(s);

			if (bAutoFix)
			{
				// if we have at least 2 fields before to refer to, this might be a Cube Quest styled chapter
				// (0 for line 17, chapter code for line 18, which violates the official spec)
				if (uFieldIdx > 2)
				{
					// if the previous field was a picture number and the field before it had a chapter number,
					//   then assume that we are using Cube Quest styled VBI and make this field also a chapter number
					if (IsPictureNum(vParseVBI[uFieldIdx-1].uVBI[VBIParse::LINE18]) &&
						(IsChapterNum(vParseVBI[uFieldIdx-2].uVBI[VBIParse::LINE18])))
					{
						vi->uVBI[VBIParse::LINE18] = uVBI18 = vParseVBI[uFieldIdx-2].uVBI[VBIParse::LINE18];
					}
				}
			}
		}

		// Make sure line 18 is unambiguous
		bool bBestLineFound = GetBestLine1718(uVBI, uVBI17, uVBI18, vidStd);
		if (bBestLineFound)
		{
			if (bAutoFix)
			{
				// TODO : make sure line17 and line18 match here
			}
		}
		// else the value of line 18 could not be validated based on the value of line 17
		else
		{
			bool bFixed = false;

			// We can only proceed if the previous fields have already been fixed, because the compactor requires good data
			if (bAutoFix)
			{
				double dConfidencePercentage = 0.0;
				bFixed = AutoFixField(vParseVBI, uFieldIdx, &dConfidencePercentage, vidStd);
			} // end if auto-fix was enabled

			if (bFixed)
			{
				uVBI = vi->uVBI[LINE18];
			}
			// if we didn't end up fixing the error, then log it
			else
			{
				// If we couldn't discern correct 
				s = "Conflict found which could not be automatically resolved.  On track ";
				s += std::to_string(uTrackIdx) + ", field " + std::to_string(uWhichField) + " (";
				s += UintHexToStr((uFieldIdx*10) + 4) + "), Line 17 is ";
				s += UintHexToStr(uVBI17);
				s += " and Line 18 is ";
				s += UintHexToStr(uVBI18);
				s += ".";
				lstErrors.push_back(s);
				bGotAtLeastOneError = true;
				uVBI = 0;
			}
		} // end if line 17/18 could not be quickly resolved

		// if this is a picture number
		// (we won't log these as errors because they may not be; we will log unexpected behavior as warnings only)
		if (IsPictureNum(uVBI))
		{
			bool bSuspicious = false;
			uCurPicNum = PictureNumVBIToDecimal(uVBI, vidStd);

			if (bPreviouslyGotPicNum)
			{
				if (uCurPicNum > uLastPicNum)
				{
					// if the picture number jumped too much
					if ((uCurPicNum - uLastPicNum) > 1)
					{
						s = "Picture number ";
						s += std::to_string(uCurPicNum);
						s += " jumps from previous picture number ";
						s += std::to_string(uLastPicNum);
						lstWarnings.push_back(s);
						bSuspicious = true;
					}
					// else it increases by one but we got a pic num in the very next field
					else if (uFieldsSincePicNum < 1)
					{
						s = "Picture number ";
						s += std::to_string(uCurPicNum);
						s += " occurs too frequently from last picture number ";
						s += std::to_string(uLastPicNum);
						lstWarnings.push_back(s);
						bSuspicious = true;
					}
					// else it's ok
				}
				else if (uCurPicNum < uLastPicNum)
				{
					s = "Picture number ";
					s += std::to_string(uCurPicNum);
					s += " is less than the previous picture number ";
					s += std::to_string(uLastPicNum);
					lstWarnings.push_back(s);
					bSuspicious = true;
				}
				// else it's the same pic num; I'm not sure if this ever really happens, I think it might with DL2

				// if we see suspicious frame numbering, see if we can confidently determine it to be an error and fix it
				if (bSuspicious && bAutoFix)
				{
					double dConfidencePercentage;

					// we don't care about the result here because either way we will say that the VBI verification passed
					// (we can't necessarily say susipicous picture numbers are errors because they are not)
					AutoFixField(vParseVBI, uFieldIdx, &dConfidencePercentage, vidStd);
				}
			}
			// else this is our first pic num, so we have nothing to compare against

			bPreviouslyGotPicNum = true;
			uLastPicNum = uCurPicNum;
			uFieldsSincePicNum = 0;
		}
		// else it's not a picture number
		else
		{
			uFieldsSincePicNum++;
		}
		uFieldIdx++;
	}

	return !bGotAtLeastOneError;
}

bool VBIParse::AutoFixField(vector <VBI_t> &vParseVBI, size_t uFieldIdx, double *pdConfidencePercentage, VideoStandard_t vidStd)
{
	bool bFixed = false;
	bool bFailed = false;
//	unsigned int uThisLine18 = 0;

	VBI_t *vi = &vParseVBI[uFieldIdx];

	// Try to discern correct value of line 18 (and 17) by looking for a predictable pattern

	// can we use the previous 9 fields to find a pattern?
	// (We choose 9 because we need 5 fields for the 2:3 pattern and we need 9 fields to ensure we have
	// a complete 2:3 pattern)
	if (uFieldIdx >= 9)
	{
		// copy previous fields
		vector <VBI_t> vVBI;
		for (unsigned int u = uFieldIdx - 9; u < uFieldIdx; u++)
		{
			vVBI.push_back(vParseVBI[u]);
		}

		// analyze the fields for patterns
		list<VBICompactEntry_t> lstEntries;

		// if we're able to successfully compact, then check for the pattern
		if (CompactVBIData(lstEntries, vVBI, 0, 9, vidStd))
		{
			VBICompactEntry_t entry = lstEntries.back();
			int iFieldOffset = 9 - entry.u32StartAbsField;	// the field we are concerned with
			VBIC_LoadLine18(&entry, iFieldOffset);	// get the expected value 
			unsigned int uThisLine18 = VBIC_GetCurFieldLine18();

			// if there are two more fields after this one, compare with it just to double-check our result
			if (uFieldIdx+2 < vParseVBI.size())
			{
				unsigned int uNextLine17 = vParseVBI[uFieldIdx+1].uVBI[LINE17];
				unsigned int uNextLine18 = vParseVBI[uFieldIdx+1].uVBI[LINE18];
				unsigned int uLine17AfterNext = vParseVBI[uFieldIdx+2].uVBI[LINE17];
				unsigned int uLine18AfterNext = vParseVBI[uFieldIdx+2].uVBI[LINE18];

				// if the next lines have some data we can compare against
				if (((uNextLine18 != PARSE_FAILED) || (uNextLine17 != PARSE_FAILED)) && ((uLine18AfterNext != PARSE_FAILED) || (uLine17AfterNext != PARSE_FAILED)))
				{
					VBIC_LoadLine18(&entry, ++iFieldOffset);	// get the value of the next line 18 and compare it to our next field
					unsigned int uExpectedNextLine18 = VBIC_GetCurFieldLine18();
					VBIC_LoadLine18(&entry, ++iFieldOffset);	// " "
					unsigned int uExpectedLine18AfterNext = VBIC_GetCurFieldLine18();

					// if the expected values matches either lines 17 or 18 then we can be very confident that our guess for the current line 18 is correct
					// (it's possible that one of the lines 17 or 18 will be corrupt so we can't just compare against one of them)
					if (((uNextLine18 == uExpectedNextLine18) || (uNextLine17 == uExpectedNextLine18)) &&
						((uLine18AfterNext == uExpectedLine18AfterNext) || (uLine17AfterNext == uExpectedLine18AfterNext)))
					{
						*pdConfidencePercentage = 95.0;
						vi->uVBI[LINE17] = uThisLine18;	// TODO : handle stop codes
						vi->uVBI[LINE18] = uThisLine18;
						bFixed = true;
					}
					// Else they do not match, so we can be confident that our guess would be wrong.
					// It would be interesting to examine this scenario with real data and come up with a solution.
					// (A human probably could figure out the right value here without too much trouble)
					else
					{
						bFailed = true;
					}
				}
				// Else the next lines did not parse correctly, so we are going to fix this line and assume we're correct.
				// For a string of consecutive parse errors, the final parse error will compare against a valid result and know whether all of these fixes were right or not.
			} // end if there are two more fields after this one

			// if we weren't able to fix anything but we didn't get an error, then we can make a good guess here based on the pattern
			if (!bFixed && !bFailed)
			{
				*pdConfidencePercentage = 66.0;
				vi->uVBI[LINE17] = uThisLine18;	// TODO : handle stop codes
				vi->uVBI[LINE18] = uThisLine18;
				bFixed = true;
			}
		} // end if we were able to successfully compact the VBI Data
	} // end if we had enough fields to find a pattern

	return bFixed;
}

bool VBIParse::CompactVBIData(list<VBICompactEntry_t> &lstEntries, VideoStandard_t vidStd)
{
	return CompactVBIData(lstEntries, m_vParseVBI, 0, m_vParseVBI.size(), vidStd);
}

bool VBIParse::CompactVBIData(list<VBICompactEntry_t> &lstEntries, const vector <VBI_t> &vParseVBI, size_t stFieldIdx,
		size_t stEntriesToCompact, VideoStandard_t vidStd)
{
	bool bRes = true;
	list <VBICompactEntry_t> lstProposed;
	lstEntries.clear();

	size_t stTotalFields = stFieldIdx + stEntriesToCompact;
	size_t stPatternIdx = 0;
	bool bSeekingNewPattern = true;
	unsigned int uExpectedLine18 = 0;
	VBICompactEntry_t entry;
	int iLastPictureNum = 0;	// this must be an int because it has the same value as i32StartPictureNum
	uint32_t uPatternMask = 0xffffff;	// so we can support non-standard picture numbers used by Gallagher's  Gallery (0xf00001 is picture number 1, instead of 0xf80001)

	while (stFieldIdx < stTotalFields)
	{
		size_t stFieldsRemaining = stTotalFields - stFieldIdx;
		unsigned int uLine18 = vParseVBI[stFieldIdx].uVBI[VBIParse::LINE18];

		if (uLine18 == VBIParse::PARSE_FAILED)
		{
			uLine18 = vParseVBI[stFieldIdx].uVBI[VBIParse::LINE17];
		}

		if (!bSeekingNewPattern)
		{
			// if the pattern is still going
			if (uLine18 == uExpectedLine18)
			{
				// calculate next expected line18 value
				switch (entry.typePattern)
				{
				case PATTERN_22:

					stPatternIdx ^= 1;	// alternates between 0 and 1

					// if we're at the beginning of the pattern, we expect a picture number
					if (!stPatternIdx)
					{
						iLastPictureNum++;
						uExpectedLine18 = DecimalToPictureNumVBI(iLastPictureNum, vidStd);

						// in case pattern is 0xf00001 instead of 0xf80001 (for example)
						uExpectedLine18 &= uPatternMask;
					}
					else
					{
						uint8_t uChapter = 0;;

						// if the entry contains a chapter, then we will expect chapter VBI
						if (VBIC_GetChapterInfo(&entry, &uChapter))
						{
							uExpectedLine18 = DecimalToChapterNumVBI(uChapter);
						}
						else
						{
							uExpectedLine18 = PARSE_BLACK;
						}
					}
					break;
				case PATTERN_22_ATARI:
					stPatternIdx ^= 1;	// alternates between 0 and 1

					// if we're at the beginning of the pattern, we expect a picture number
					if (!stPatternIdx)
					{
						iLastPictureNum++;
						uExpectedLine18 = DecimalToPictureNumVBI(iLastPictureNum, vidStd);
					}
					else
					{
						uExpectedLine18 &= 0xAFFFFF;	// turn the F????? into A?????
					}
					break;
				case PATTERN_23:
					stPatternIdx++;
					stPatternIdx %= 5;	// 5-field cycle in pattern

					switch (stPatternIdx)
					{
					case 0:
					case 2:
						iLastPictureNum++;
						uExpectedLine18 = DecimalToPictureNumVBI(iLastPictureNum, vidStd);
						break;
					default:
						uExpectedLine18 = 0;
						break;
					}
					break;
				default:
					// no change for the expected line18
					break;
				}

				stFieldIdx++;
			}
			// else the pattern is broken, so figure out the new pattern
			else
			{
				bSeekingNewPattern = true;
			}
		}
		// else if we're seeking a new pattern
		else
		{
			unsigned int uNextLine18 = 0;
			bool bIsNextLineAChapterNum = false;
			uExpectedLine18 = uLine18;
			bSeekingNewPattern = false;
			memset(&entry, 0, sizeof(entry));	// initialize entry with defaults
			entry.u32StartAbsField = stFieldIdx;
			stPatternIdx = 0;	// we're looking for the start of a new pattern

			if (stFieldsRemaining > 1)
			{
				uNextLine18 = vParseVBI[stFieldIdx+1].uVBI[LINE18];
				bIsNextLineAChapterNum = IsChapterNum(uNextLine18);
			}

			// Check to see if it's 2:3 (this check must come first because it will look like 2:2 up to a point)
			if ((stFieldsRemaining >= 5) && IsPictureNum(uLine18) &&
				(uNextLine18 == PARSE_BLACK) &&
				(vParseVBI[stFieldIdx+2].uVBI[LINE18] == (DecimalToPictureNumVBI(PictureNumVBIToDecimal(uLine18, vidStd) + 1, vidStd)) &&
				(vParseVBI[stFieldIdx+3].uVBI[LINE18] == PARSE_BLACK) &&
				(vParseVBI[stFieldIdx+4].uVBI[LINE18] == PARSE_BLACK)
				))
			{
				entry.i32StartPictureNumber = iLastPictureNum = PictureNumVBIToDecimal(uLine18, vidStd);
				entry.typePattern = PATTERN_23;

				// check to see if the pattern actually started 1 or more fields before
				if (stFieldIdx > 0)
				{
					// 8-bits to optimize for AVR
					uint8_t u8StartPictureNumberAdjustment = 0;
					uint8_t u8StartAbsFieldAdjustment = 0;
					uint8_t u8NewPatternOffset = 0;

					// the pattern could've started between 1-4 fields preceeding our current position
					for (size_t iFieldsWherePatternIsValid = 1; iFieldsWherePatternIsValid < 5; iFieldsWherePatternIsValid++)
					{
						// if we can look back on our VBI array without overflowing (ie array index becoming -1)
						if (stFieldIdx >= iFieldsWherePatternIsValid)
						{
							bool bPatternIsValidOnCandidateField = false;
							unsigned int uLastLine18 = vParseVBI[stFieldIdx-iFieldsWherePatternIsValid].uVBI[LINE18];

							switch (iFieldsWherePatternIsValid)
							{
							default:	// 1, 2, or 4 fields before
								if (uLastLine18 == PARSE_BLACK)
								{
									u8NewPatternOffset = 5 - iFieldsWherePatternIsValid;
									bPatternIsValidOnCandidateField = true;
								}
								break;
							case 3:	// 3 fields before (we would've already checked 1 and 2 fields before successfully)
								if (uLastLine18 == DecimalToPictureNumVBI(entry.i32StartPictureNumber-1, vidStd))
								{
									u8NewPatternOffset = 2;
									bPatternIsValidOnCandidateField = true;
								}
								break;
							}

							// if we found that the pattern is field on the candidate field, then we may be able to get rid of previous entries and save space
							if (bPatternIsValidOnCandidateField)
							{
								// the beginning of the 2:3 pattern will always be 2 picture numbers less because 2:3 encompasses 2 picture numbers and we're subtracting from the next complete pattern's start picture number
								u8StartPictureNumberAdjustment = 2;

								// u32StartAbsField needs to refer to the start of the current pattern, even if it starts in the middle of the pattern
								u8StartAbsFieldAdjustment = iFieldsWherePatternIsValid;

								// if the previous entry also covers the same area they our adjustment will cover, then we need to get rid of it now because we have a more accurate entry
								if (!lstEntries.empty())
								{
									VBICompactEntry_t entryPrev = lstEntries.back();
									if (entryPrev.u32StartAbsField == (entry.u32StartAbsField - iFieldsWherePatternIsValid))
									{
										lstEntries.pop_back();	// get rid of it, our new entry supercedes it
									}
								}
							}
							// if we did not find a matching candidate, then our journey is finished
							else
							{
								break;
							}
						}
						// else there are no more fields to examine, so we're done
						else
						{
							break;
						}
					} // end for loop

					// even if we found that the pattern did not extend into the past, the default values will cause no change to happen
					entry.i32StartPictureNumber -= u8StartPictureNumberAdjustment;
					entry.u32StartAbsField -= u8StartAbsFieldAdjustment;
					entry.u8PatternOffset = u8NewPatternOffset;
				}

			}
			// else check to see if it's 2:2
			else if ((stFieldsRemaining >= 2) && IsPictureNum(uLine18) &&
				(bIsNextLineAChapterNum || (uNextLine18 == PARSE_BLACK))
				)
			{
				entry.i32StartPictureNumber = iLastPictureNum = PictureNumVBIToDecimal(uLine18, vidStd);
				entry.typePattern = PATTERN_22;
				uPatternMask = (uLine18 & 0x080000) | 0xf7ffff;	// so we can support non-standard pic nums used by Gallagher's Gallery

				if (bIsNextLineAChapterNum)
				{
					VBIC_SetChapterInfo(&entry, ChapterNumVBIToDecimal(uNextLine18));
				}

				// check to see if the pattern actually started one field before
				// (if we are starting a new stop code, then we don't care if the pattern started one field before because the stop code is always the beginning of the entry)
				if (stFieldIdx > 0)
				{
					unsigned int uLastLine18 = vParseVBI[stFieldIdx-1].uVBI[LINE18];

					// if this pattern did start one field before, then adjust our entry
					if (
						// Previous and next lines must both be consistent (either both chapters or both black)
						// Otherwise, we have no pattern.
						((uLastLine18 == PARSE_BLACK) && (!bIsNextLineAChapterNum))
							||
						(IsChapterNum(uLastLine18) && (bIsNextLineAChapterNum))
						)
					{
						entry.i32StartPictureNumber--;
						entry.u32StartAbsField--;
						entry.u8PatternOffset = 1;

						// if the previous entry also starts one field before, then we need to get rid of it now because we have a more accurate entry
						if (!lstEntries.empty())
						{
							VBICompactEntry_t entryPrev = lstEntries.back();
							if (entryPrev.u32StartAbsField == entry.u32StartAbsField)
							{
								lstEntries.pop_back();	// get rid of it, our new entry supercedes it
							}
						}
					}
				}
			}
			// else check to see if it's 2:2 Atari
			else if ((stFieldsRemaining >= 2) && ((uLine18 & 0xF80000) == 0xF80000) &&
				(vParseVBI[stFieldIdx+1].uVBI[LINE18] == (DecimalToPictureNumVBI(PictureNumVBIToDecimal(uLine18, vidStd) + 1, vidStd) & 0xAFFFFF))
				)
			{
				entry.i32StartPictureNumber = iLastPictureNum = PictureNumVBIToDecimal(uLine18, vidStd);
				entry.typePattern = PATTERN_22_ATARI;
			}
			// else check to see if it's 2:2 Atari starting in the middle of the pattern
			else if ((uLine18 & 0xF80000) == 0xA80000)
			{
				entry.i32StartPictureNumber = iLastPictureNum = PictureNumVBIToDecimal(uLine18, vidStd);
				entry.typePattern = PATTERN_22_ATARI;
				entry.u8PatternOffset = 1;
				stPatternIdx = 1;
			}
			// else check to see if it's a chapter number
			else if (IsChapterNum(uLine18))
			{
				entry.typePattern = PATTERN_ZEROES;	// the zero pattern will include the chapter code if a chapter is present
				entry.u16Special = ChapterNumVBIToDecimal(uLine18);
				entry.u16Special |= EVENT_CHAPTER;
			}
			// else check to see if it's lead-in
			else if (uLine18 == 0x88FFFF)
			{
				entry.typePattern = PATTERN_LEADIN;
			}
			// else check to see if it's lead-out
			else if (uLine18 == 0x80EEEE)
			{
				entry.typePattern = PATTERN_LEADOUT;
			}
			else if (uLine18 == PARSE_BLACK)
			{
				entry.typePattern = PATTERN_ZEROES;
			}
			// else if it's a picture number with an unknown pattern
			else if (IsPictureNum(uLine18))
			{
				entry.typePattern = PATTERN_PICNUM;
				entry.i32StartPictureNumber = PictureNumVBIToDecimal(uLine18, vidStd);
			}

			// else we can't figure out the pattern, so return with an error
			else
			{
				bRes = false;
				break;	// abort
			}

			lstEntries.push_back(entry);
		} // end new pattern
	} // end while we're not done going through all the fields

	return bRes;
}

bool VBIParse::GetVBIData(VBI_t &vbiDst, size_t stIdx)
{
	bool bRes = false;
	if (stIdx < m_vParseVBI.size())
	{
		vbiDst = m_vParseVBI[stIdx];
		bRes = true;
	}
	else
	{
		m_lstErrors.push_back("Out of range.");
	}
	return bRes;
}

size_t VBIParse::GetVBIDataCount()
{
	return m_vParseVBI.size();
}

string VBIParse::GetLastErrorMsg()
{
	string strResult;

	if (!m_lstErrors.empty())
	{
		return m_lstErrors.back();
	}

	return strResult;
}

list<string> VBIParse::GetWarnings()
{
	return m_lstWarnings;
}

list<string> VBIParse::GetErrors()
{
	return m_lstErrors;
}
