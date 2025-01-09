#ifndef VBIPARSE_H
#define VBIPARSE_H

#include <ldp-abst/VBICompact.h>
#include <ldp-abst/VideoStandard.h>
#include "mpo_deleter.h"
#include <list>
#include <vector>
#include <string>
using namespace std;

typedef struct VBI_s
{
	bool bWhiteFlag;
	unsigned int uVBI[3];	// VBI for lines 16, 17, 18 (in that order)
} VBI_t;

// a convenience struct so we can do sizeof's in our code for the target VBI size
typedef struct VBIPacked_s
{
	unsigned char arrData[10];
} VBIPacked_t;

class VBIParse;

typedef shared_ptr<VBIParse> VBIParseSPtr;

class VBIParse : public MpoDeleter
{
public:
	enum
	{
		PARSE_BLACK = 0,

		// this must be 24-bits because it needs to be storable by SaveVBIData().
		// It should not conflict with valid data, because valid data must have the most significant bit set.
		PARSE_FAILED = 0x7FFFFF
	};

	// corresponds to VBI_t struct
	enum
	{
		LINE16 = 0,
		LINE17 = 1,
		LINE18 = 2
	};

	// Parses manchester codes from RGB line (3 bytes per pixel).
	// Returns true if the function is confident in the result.
	// An example result would be 0xF80031 in 'uResult'.
	static bool ParseRGBManchester(unsigned int &uResult, const unsigned char *p8Line, unsigned int uLineWidth);

	// Returns true if the line is a 'white flag' or false if the line cannot be confidently determined to be a white flag.
	static bool IsWhiteFlag(const unsigned char *p8Line, unsigned int uLineWidth);

	// Tries to calculate what the correct manchester value is based on looking at lines 17 and 18 (which
	//  are supposed to be duplicates) and also optionally looking at the last 'picture number' (aka frame number)
	//  which we successfully parsed.
	// If successful, true is returned and uLine1718 is populated with the result.
	static bool GetBestLine1718(unsigned int &uLine1718, const VBI_t &vbi, VideoStandard_t vidStd,
		unsigned int uPreviousPictureNumVBI = 0);

	// returns true if this VBI is a picture number (frame number)
	static bool IsPictureNum(unsigned int uVBI);

	static bool IsChapterNum(unsigned int uVBI);

	// returns true if the VBI is a stop code
	static bool IsStopCode(unsigned int uVBI);

	static VBIParseSPtr GetInstance();

	// now come the non-static methods, for loading and storing VBI data

	// Parses an entire RGB field (including lines 11, 16, 17, and 18).
	// If parse succeeded, 'vbiDst' structure will be populated with results, and true will be returned.
	// Returns false if parse failed; call GetLastErrorMsg() for the reason.
	// NOTE : this cannot be static, because it needs to store a possible error message.
	bool ParseRGBField(VBI_t &vbiDst, const unsigned char *pField, unsigned int uLineWidth);

	// Adds VBI data to be saved with SaveVBIData() method.
	void AddVBIData(const VBI_t &vbiSrc);

	// Packs parsed VBI data into a binary format and returns it wrapped in a string (use .data() and .size() to extract)
	string SaveVBIData();

	// Unpacks binary VBI data and returns true if successful, or false if there was an error
	bool LoadVBIData(const void *pBuf, size_t stSizeBytes);

	// Clears any parsed VBI data
	void ClearVBIData();

	// Checks loaded VBI data for warnings and errors and returns false if at least one error was found.
	// Otherwise it returns true.
	// Call GetWarnings to get any warnings found and GetErrors to get any errors found.
	bool VerifyVBIData(VideoStandard_t vidStd, bool bAutoFix = true);

	// a less-friendly version of VerifyVBIData that receives an arbitrary vector of VBI data.
	static bool VerifyVBIData(vector <VBI_t> &vParseVBI, list <string> &lstWarnings, list<string> &lstErrors, VideoStandard_t vidStd, bool bAutoFix);

	static bool AutoFixField(vector <VBI_t> &vParseVBI, size_t stWhichField, double *pdConfidencePercentage, VideoStandard_t vidStd);

	// Compact VBI data into a list of compact entries (see VBICompact.h)
	bool CompactVBIData(list<VBICompactEntry_t> &lstEntries, VideoStandard_t vidStd);

	// This is a less-friendly version of the CompactVBIData function above.
	// It is used to compact a small portion of the VBI in order to detect patterns.
	static bool CompactVBIData(list<VBICompactEntry_t> &lstEntries, const vector <VBI_t> &vParseVBI, size_t stStartIdx,
		size_t stEntriesToCompact, VideoStandard_t vidStd);

	// retrieves VBI data for a specific index, or returns false if index is out of range
	// (used for unit testing)
	bool GetVBIData(VBI_t &vbiDst, size_t stIdx);

	// helper function that converts picture number from ascii hex (BCD) into decimal, stripping off headers
	// (this is public because it is useful for at least one external method in daphne's LDImage class)
	static unsigned int PictureNumVBIToDecimal(unsigned int uVBI, VideoStandard_t vidStd);

	static unsigned int ChapterNumVBIToDecimal(unsigned int uVBI);

	// helper function that converts a decimal number to a picture number (BCD), including the 0xF8 header
	// (this is public because it is useful for at least one external method in daphne's LDP class)
	static unsigned int DecimalToPictureNumVBI(unsigned int uNum, VideoStandard_t vidStd);

	static unsigned int DecimalToChapterNumVBI(unsigned int uNum);

	// convenience functions to generate some dummy VBI
	static VBI_t GenerateVBIFrame(unsigned int uFrameNum, VideoStandard_t vidStd);
	static VBI_t GenerateVBIEmpty();
	static VBI_t GenerateVBIChapter(unsigned int uChapterNum);
	static VBI_t GenerateVBIStopCode();
	static VBI_t GenerateVBILeadIn();
	static VBI_t GenerateVBILeadOut();

	// returns number of fields that have been successfully parsed
	size_t GetVBIDataCount();

	// Returns the last error message that occurred (can happen with ParseRGBField).
	string GetLastErrorMsg();

	// returns messages for all warnings found
	list<string> GetWarnings();

	// returns messages for all errors found
	list<string> GetErrors();

	// Returns true if VBI was parsed properly and is not black.
	// Returns false otherwise.
	static bool IsVBIDefined(unsigned int uVBI);

private:

	// similar to function of the same name; made private so caller doesn't need to understand uVBI array inside VBI_t structure
	static bool GetBestLine1718(unsigned int &uDst, unsigned int uLine0, unsigned int uLine1, VideoStandard_t vidStd,
		unsigned int uPreviousPictureNumVBI = 0);

	static bool GetThresholds(double &dHighThreshold, double &dLowThreshold, const unsigned char *p8Line,
		unsigned int uLineWidth);

	// transition function, only used by the white flag detector now
	static bool GetHighLowWidths(list <unsigned int> &lstTransWidths,
		const unsigned char *p8Line, unsigned int uLineWidth);

	static bool GetHighLowWidths(list <unsigned int> &lstTransWidths,
		double dHighThreshold, double dLowThreshold,
		const unsigned char *p8Line, unsigned int uLineWidth);

	// fixes widths when they are supposed to be for manchester encoding
	// IMPORTANT: this assumes first lead-in width has already been discarded
	// (and thus first width is the first high)
	static bool FixManchesterWidths(list <unsigned int> &lstTransWidths, unsigned int uLineWidth);

	void DeleteInstance();
	VBIParse();

	//

	// version ID so we can maintain backward compatibility if we ever change the VBI file format
	static unsigned char arrVersion1[4];

	// vector that is maintained while parsing fields so that we can eventually save it in a packed binary format
	// (it's a vector for random access, see GetVBIData function)
	vector <VBI_t> m_vParseVBI;

	// warning and error messages compiled during the life-time of this class
	list <string> m_lstWarnings;
	list <string> m_lstErrors;
};

#endif // VBIPARSE_H
