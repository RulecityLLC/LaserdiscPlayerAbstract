#include <ldp-abst/VBICompact.h>
#include <string.h>	// for memcpy

#ifdef DEBUG
#include <assert.h>
#endif //DEBUG

static VBICompactState_t g_VBIC_state;

//////////////////////////////////////////

void VBIC_Init(const VBICompact_t *pVBI)
{
	g_VBIC_state.u32CurAbsField = 0;
	g_VBIC_state.uCurEntryIdx = 0;
	g_VBIC_state.pVBI = pVBI;
	g_VBIC_state.u32CurPictureNum = 0;
	VBIC_AddField(0);	// initialize the line18 value to something sane
}

uint32_t VBIC_GetCurAbsField()
{
	return g_VBIC_state.u32CurAbsField;
}

void VBIC_2BCD(uint32_t u24)
{
	char s[10];	// if this is not a char (for example if it is a int8_t), then avr studio gives a warning

	// this routine is expensive, but on AVR, it's actually faster than doing the division manually
	sprintf(s, "%06lu", (uint32_t) u24);
	g_VBIC_state.pu8CurLine18[0] = ((s[0] & 0xF) << 4) | (s[1] & 0xF);
	g_VBIC_state.pu8CurLine18[1] = ((s[2] & 0xF) << 4) | (s[3] & 0xF);
	g_VBIC_state.pu8CurLine18[2] = ((s[4] & 0xF) << 4) | (s[5] & 0xF);
}

VBIC_BOOL VBIC_AddField(int16_t iOffset)
{
	// safety check.
	// This can probably be eliminated if this routine proves too slow.
	int32_t l = g_VBIC_state.u32CurAbsField;
	l += iOffset;

	// if we moved backward too many fields
	if (l < 0)
	{
		return VBIC_FALSE;
	}

	return VBIC_SetField(l);
}

VBIC_BOOL VBIC_SetField(uint32_t uAbsoluteField)
{
	VBIC_BOOL bMovedForward = VBIC_FALSE;
	const VBICompactEntry_t *pEntry = &g_VBIC_state.pVBI->pEntries[g_VBIC_state.uCurEntryIdx];
	uint32_t u32FieldDiff;

	// range check
	// (this check is the result of an actual defect on Dexter where the absolute field was -1 unsigned)
	if (uAbsoluteField >= g_VBIC_state.pVBI->uTotalFields)
	{
		return VBIC_FALSE;
	}

	g_VBIC_state.u32CurAbsField = uAbsoluteField;

	// check to see if we need to more to the next entry in the VBI array
	for (;;)
	{
		unsigned int uNextEntryIdx = g_VBIC_state.uCurEntryIdx + 1;

		// if there is no next entry after the one we're on
		if (uNextEntryIdx >= g_VBIC_state.pVBI->uEntryCount)
		{
			break;
		}

		// if the next entry better applies to our current field, then use it
		if (g_VBIC_state.pVBI->pEntries[uNextEntryIdx].u32StartAbsField <= g_VBIC_state.u32CurAbsField)
		{
			g_VBIC_state.uCurEntryIdx = uNextEntryIdx;	// can be changed to ++ if that proves faster
			pEntry = &g_VBIC_state.pVBI->pEntries[g_VBIC_state.uCurEntryIdx];
			bMovedForward = VBIC_TRUE;
		}
		// else we don't need to move to the next entry anymore
		else
		{
			break;
		}
	}

	// if we didn't move forward, check to see if we need to move backward
	if (!bMovedForward)
	{
		for (;;)
		{
			// if we're at the first entry, we can't move back
			if (g_VBIC_state.uCurEntryIdx == 0)
			{
				break;
			}

			// if the current entry's start is passed our position, then move back to the previous one
			if (pEntry->u32StartAbsField > g_VBIC_state.u32CurAbsField)
			{
				g_VBIC_state.uCurEntryIdx--;
				pEntry = &g_VBIC_state.pVBI->pEntries[g_VBIC_state.uCurEntryIdx];
			}
			// else we don't need to move to the previous entry anymore
			else
			{
				break;
			}
		}
	}

	// NOTE : this can be optimized for AVR if necessary (down to a 16-bit operation)

	// NOTE : We've already ensured that cur abs field is >= start abs field in the above for loops
	u32FieldDiff = g_VBIC_state.u32CurAbsField - pEntry->u32StartAbsField;

	VBIC_LoadLine18(pEntry, u32FieldDiff);

	return VBIC_TRUE;
}

uint32_t VBIC_GetCurFieldLine18()
{
	// IMPORTANT: these casts are required to ensure proper behavior on AVR
	uint32_t ulTop = ((uint32_t) g_VBIC_state.pu8CurLine18[0] << 16);
	uint16_t ulMid = ((uint16_t) g_VBIC_state.pu8CurLine18[1] << 8);
	uint32_t result = ulTop | ulMid | (g_VBIC_state.pu8CurLine18[2]);
	return result;
}

uint32_t VBIC_GetCurPictureNum()
{
	return g_VBIC_state.u32CurPictureNum;
}

VBIC_SeekResult VBIC_SeekInternal(uint32_t uFrameNum, uint32_t *pPeekOnly)
{
	const VBICompactEntry_t *pEntry = 0;
	VBIC_SeekResult res = VBIC_SEEK_BUSY;
	uint32_t uFieldDiff = 0;
	uint32_t uFrameDiff = 0;
	uint32_t uFinalField = 0;

	unsigned int i = 0;

	// figure out which entry contains the picture number
	for (i = 0; i < g_VBIC_state.pVBI->uEntryCount; i++)
	{
		int32_t iStartFrameNum;
		pEntry = &g_VBIC_state.pVBI->pEntries[i];
		iStartFrameNum = (pEntry->i32StartPictureNumber);

		// take offset into account
		switch (pEntry->typePattern)
		{
		case PATTERN_22:
		case PATTERN_22_ATARI:
			// if offset is 1, then the start frame number that we must compare against should be 1 more or else our results will be wrong
			iStartFrameNum += pEntry->u8PatternOffset;
			break;
		case PATTERN_23:
			// if offset is 1, then the start frame should be 1 more.  if offset is 0, then no change
			if (pEntry->u8PatternOffset < 2)
			{
				iStartFrameNum += pEntry->u8PatternOffset;
			}
			// else if offset is 2, 3, or 4 then the start frame should be 2 more.
			else
			{
				iStartFrameNum += 2;
			}
			break;
		default:
			// no change
			break;
		}

		// if we've gone too far
		if ((iStartFrameNum > (int32_t) uFrameNum) || (pEntry->typePattern == PATTERN_LEADOUT))
		{
			break;
		}
	}

	// if the sought picture number occurs before our first picture number, then we need to land on the first available picture number (real laserdisc players do this)
	// (real example: Mad Dog searches to frame 0 even though the first frame is frame 1)
	if (i == 0)
	{
		uint8_t u8PatternOffset = pEntry->u8PatternOffset;
		i = 1;	// prepare to decrement to 0
		pEntry = &g_VBIC_state.pVBI->pEntries[0];
		uFrameNum = pEntry->i32StartPictureNumber;

		// if there is an offset with this start pic number, then we need to land on the next picture number so that our absolute start frame is >= 0
		if (u8PatternOffset != 0)
		{
			switch (pEntry->typePattern)
			{
			case PATTERN_22:
			case PATTERN_22_ATARI:
				uFrameNum++;
				break;
			case PATTERN_23:
				if (u8PatternOffset <= 2)
				{
					uFrameNum++;
				}
				// else if offset is 3 or 4
				else
				{
					uFrameNum += 2;
				}
				break;
			default:	// unsupported
				res = VBIC_SEEK_FAIL;
				goto done;
				break;
			}
			
		}
	}

	// as long as there is at least one entry, i will always be > 0 at this point, and we need to go back to the entry we want to use
	i--;

	// go to the start of the entry in question
	// the current field points to the beginning of an entry, so we only need to move upward to find the frame
	pEntry = &g_VBIC_state.pVBI->pEntries[i];

	// ignore "zeroes" patterns as they cannot contain picture numbers and may mess up our results
	while (pEntry->typePattern == PATTERN_ZEROES)
	{
		// if we can still go backward to a non-zero pattern
		if (i > 0)
		{
			i--;
			pEntry = &g_VBIC_state.pVBI->pEntries[i];
		}
		// else the first pattern is a ZEROES pattern which we can't use so break which will eventually return an error
		else
		{
			break;
		}
	}

	uFieldDiff = 0;
	uFrameDiff = uFrameNum - (pEntry->i32StartPictureNumber);
	uFinalField = 0;

	switch (pEntry->typePattern)
	{
		// 2:2 is the same whether it's regular or ATARI styled
	case PATTERN_22:
	case PATTERN_22_ATARI:
		{
			uFieldDiff = uFrameDiff << 1;
			uFieldDiff -= pEntry->u8PatternOffset;	// take offset into account
			res = VBIC_SEEK_SUCCESS;
		}
		break;
	case PATTERN_23:
		{
			// the fields will be spaced 5 apart
			uFieldDiff = (uFrameDiff >> 1) * 5;

			// if the frame diff is an odd number, we need to add 2 more fields, always
			uFieldDiff += ((uFrameDiff & 1) << 1);

			uFieldDiff -= pEntry->u8PatternOffset;	// take offset into account
			res = VBIC_SEEK_SUCCESS;
		}
		break;
	case PATTERN_PICNUM:
		res = VBIC_SEEK_SUCCESS;
		break;
	default:
		// if they try to seek into the LEAD-OUT area, FAIL
		res = VBIC_SEEK_FAIL;
		break;
	}

	// compute final field
	if (res == VBIC_SEEK_SUCCESS)
	{
		// initialize as if this entry is the final pattern entry
		uint32_t uMaxFieldIdx = g_VBIC_state.pVBI->uTotalFields - 1;

		// if this is not the final pattern entry (for example, if lead-out comes after)
		if ((i + 1) != g_VBIC_state.pVBI->uEntryCount)
		{
			const VBICompactEntry_t *pEntryNext = &g_VBIC_state.pVBI->pEntries[i+1];

			uMaxFieldIdx = pEntryNext->u32StartAbsField - 1;
		}

		// compute proposed final field
		uFinalField = pEntry->u32StartAbsField + uFieldDiff;

		// if proposed final field is out of range
		if (uFinalField > uMaxFieldIdx)
		{
			switch (pEntry->typePattern)
			{
			case PATTERN_22:
			case PATTERN_22_ATARI:
				{
					uint32_t u32Delta = uFinalField - uMaxFieldIdx;

					// if the adjustment isn't a complete set of one or more frames (since we are 2:2)
					if ((u32Delta & 1) != 0)
					{
						// adjust so that it is a complete set of one or more frames
						u32Delta++;
					}

					uFinalField -= u32Delta;
					uFrameNum -= (u32Delta >> 1);
				}
				break;
			case PATTERN_23:
				{
					uint32_t u32MaxFieldIdxAdjusted = uMaxFieldIdx + pEntry->u8PatternOffset;
					uint32_t u32MaxIdxDiv5 = u32MaxFieldIdxAdjusted / 5;
					uint8_t u8MaxIdxMod5 = u32MaxFieldIdxAdjusted % 5;
					uint32_t u32PicNumBase = u32MaxIdxDiv5 << 1;

					// if the remainder is 2-4
					if (u8MaxIdxMod5 >= 2)
					{
						uFrameNum = u32PicNumBase + 2;
						uFinalField = (uMaxFieldIdx - u8MaxIdxMod5) + 2;
					}
					// else remainder is 0-1
					else
					{
						uFrameNum = u32PicNumBase + 1;
						uFinalField = (uMaxFieldIdx - u8MaxIdxMod5);
					}
				}
				break;
			default:	// unhandled
				res = VBIC_SEEK_FAIL;
				break;
			}			
		}
	}

	// if successful, adjust cur field and the line18 values accordingly
	// NOTE : this is intentionally not an "else if" because res can change in the previous if clause
	if (res == VBIC_SEEK_SUCCESS)
	{
		// if we are to perform the full seek
		if (pPeekOnly == 0)
		{
			g_VBIC_state.uCurEntryIdx = i;
			g_VBIC_state.u32CurAbsField = uFinalField;
			g_VBIC_state.u32CurPictureNum = uFrameNum;
			VBIC_2BCD(uFrameNum);
			g_VBIC_state.pu8CurLine18[0] |= 0xF8;
		}
		// else if we are just performing a lookup
		else
		{
			*pPeekOnly = uFinalField;
		}
	}

done:
	return res;
}

////////////////////////////////////

void VBIC_LoadLine18(const VBICompactEntry_t *pEntry, int32_t iFieldOffsetFromEntryStart)
{
	int32_t lPicNumOffset = 0;	// this will always be 0 unless the fieldoffset is negative
	int32_t i32BasePictureNumber = pEntry->i32StartPictureNumber;
	uint32_t u32FieldDiff = 0;

	// if field offset is negative, make it positive so we can do our calculations properly
	if (iFieldOffsetFromEntryStart < 0)
	{
		unsigned int uAdder = 0;
		switch (pEntry->typePattern)
		{
		case PATTERN_22:
		case PATTERN_22_ATARI:
			uAdder = 2;
			break;
		case PATTERN_23:
			uAdder = 5;
			break;
		default:
			// No adjustment can be done which means this is an error because only 2:2 or 2:3 patterns should have a negative start offset (this is not confirmed, I just wrote this comment from memory and don't seem to have any unit tests to back it up).
			// This scenario was found in the wild thanks on Dexter.
			// Since this is an error, returning immediately from this method seems right. (no need to actually return VBI data)
			return;
		}

		// adjust the starting picture number based on the field offset
		while (iFieldOffsetFromEntryStart < 0)
		{
			iFieldOffsetFromEntryStart += uAdder;
			lPicNumOffset--;
		}
	}

	u32FieldDiff = iFieldOffsetFromEntryStart;	// this will always be positive at this point
	u32FieldDiff += pEntry->u8PatternOffset;

	switch (pEntry->typePattern)
	{
	default:	// pattern_22, the most common
		// if we're at the start of a new frame
		if ((u32FieldDiff & 1) == 0)
		{
			// this number will end up being >=0 as long as our offset is correct even if i32BasePictureNumber is negative
			uint32_t u32PicNum = (i32BasePictureNumber + (u32FieldDiff >> 1));
			g_VBIC_state.u32CurPictureNum = u32PicNum;
			VBIC_2BCD(u32PicNum);
			g_VBIC_state.pu8CurLine18[0] |= 0xF8;
		}
		// else we're in the middle of the frame
		else
		{
			// if we have an active chapter
			if (pEntry->u16Special & EVENT_CHAPTER)
			{
				uint8_t u8Chapter = pEntry->u16Special & 0x7F;
				g_VBIC_state.pu8CurLine18[0] = 0x88 | (u8Chapter >> 4);
				g_VBIC_state.pu8CurLine18[1] = 0x0D | ((u8Chapter & 0xF) << 4);
				g_VBIC_state.pu8CurLine18[2] = 0xDD;
			}
			else
			{
				g_VBIC_state.pu8CurLine18[0] = 0;
				g_VBIC_state.pu8CurLine18[1] = 0;
				g_VBIC_state.pu8CurLine18[2] = 0;
			}
		}
		break;
	case PATTERN_22_ATARI:
		{
			uint32_t u32PicNum = (i32BasePictureNumber + (u32FieldDiff >> 1));
			g_VBIC_state.u32CurPictureNum = u32PicNum;
			VBIC_2BCD(u32PicNum);

			// if we're at the start of a new frame
			if ((u32FieldDiff & 1) == 0)
			{
				g_VBIC_state.pu8CurLine18[0] |= 0xF8;
			}
			// else we're in the middle of the frame
			else
			{
				g_VBIC_state.pu8CurLine18[0] |= 0xA8;
			}
		}
		break;
	case PATTERN_23:
		{
			uint32_t u32PicNum;
			uint8_t u8Remainder = u32FieldDiff % 5;	// the 2:3 pattern is 5 fields long
			switch (u8Remainder)
			{
			case 0:	// first field of the pattern
			case 2:
				// this number will end up being >=0 as long as our offset is correct even if i32BasePictureNumber is negative
				u32PicNum = (i32BasePictureNumber + ((u32FieldDiff / 5) << 1));

				// if reminder was 2, we want to add 1 to the picture number, always
				u32PicNum += (u8Remainder >> 1);

				g_VBIC_state.u32CurPictureNum = u32PicNum;
				VBIC_2BCD(u32PicNum);
				g_VBIC_state.pu8CurLine18[0] |= 0xF8;
				break;
			default:	// 1, 3, and 4
				g_VBIC_state.pu8CurLine18[0] = 0;
				g_VBIC_state.pu8CurLine18[1] = 0;
				g_VBIC_state.pu8CurLine18[2] = 0;
				break;
			}
		}
		break;
	case PATTERN_LEADIN:
		g_VBIC_state.pu8CurLine18[0] = 0x88;
		g_VBIC_state.pu8CurLine18[1] = 0xFF;
		g_VBIC_state.pu8CurLine18[2] = 0xFF;
		break;
	case PATTERN_LEADOUT:
		g_VBIC_state.pu8CurLine18[0] = 0x80;
		g_VBIC_state.pu8CurLine18[1] = 0xEE;
		g_VBIC_state.pu8CurLine18[2] = 0xEE;
		break;
	case PATTERN_ZEROES:
		g_VBIC_state.pu8CurLine18[0] = 0;
		g_VBIC_state.pu8CurLine18[1] = 0;
		g_VBIC_state.pu8CurLine18[2] = 0;
		break;
	case PATTERN_PICNUM:
		g_VBIC_state.u32CurPictureNum = pEntry->i32StartPictureNumber;
		VBIC_2BCD(pEntry->i32StartPictureNumber);
		g_VBIC_state.pu8CurLine18[0] |= 0xF8;
		break;
	}
}

void VBIC_SetChapterInfo(VBICompactEntry_t *pEntry, uint8_t uChapter)
{
	VBIC_ClearChapterInfo(pEntry);
	pEntry->u16Special |= EVENT_CHAPTER;
	pEntry->u16Special |= uChapter & 0x7F;	// chapter can't be bigger than this number
}

void VBIC_ClearChapterInfo(VBICompactEntry_t *pEntry)
{
	pEntry->u16Special = 0;
}

VBIC_BOOL VBIC_GetChapterInfo(const VBICompactEntry_t *pEntry, uint8_t *puChapter)
{
	VBIC_BOOL res = VBIC_FALSE;

	if (pEntry->u16Special & EVENT_CHAPTER)
	{
		res = VBIC_TRUE;
		*puChapter = pEntry->u16Special & 0x7F;
	}

	return res;
}

size_t VBIC_ToBuffer(void *pBuf, size_t stBufSize, const VBICompact_t *pEntries)
{
	size_t stRes = 0;
	unsigned int i = 0;
	unsigned int uAdjustedEntryCount = pEntries->uEntryCount;	// adjusted after we figure out how many entries we need to strip
	unsigned int uBytesNeeded = 0;

	// need 1 byte for version, 1 byte for the entry count (a 2K AVR can't hold even 256 entries so this is a reasonable restriction),
	//  4 bytes for the total field count,
	//  and 12 bytes for each entry.
	uBytesNeeded = 6 + (uAdjustedEntryCount * 12);

	// if we have enough space, then do it!
	if ((stBufSize >= uBytesNeeded) && (uAdjustedEntryCount <= 255))
	{
		uint8_t *p8Buf = (uint8_t *) pBuf;

		*(p8Buf++) = 0;	// version, always 00 for now
		*(p8Buf++) = uAdjustedEntryCount & 0xFF;
		*(p8Buf++) = pEntries->uTotalFields & 0xFF;
		*(p8Buf++) = (pEntries->uTotalFields >> 8) & 0xFF;
		*(p8Buf++) = (pEntries->uTotalFields >> 16) & 0xFF;
		*(p8Buf++) = (pEntries->uTotalFields >> 24);

		for (i = 0; i < pEntries->uEntryCount; i++)
		{
			const VBICompactEntry_t *pEntry = &pEntries->pEntries[i];

			// ATTENTION: The following code assumes the host architecture is little-endian!!

			// 4 bytes for absolute field
			memcpy(p8Buf, &pEntry->u32StartAbsField, 4);
			p8Buf += 4;

			// 4 bytes for picture number
			memcpy(p8Buf, &pEntry->i32StartPictureNumber, 4);
			p8Buf += 4;

			// 1 byte for the pattern
			*(p8Buf++) = pEntry->typePattern;

			// 2 bytes for stopcode (not used anymore)/chapter
			memcpy(p8Buf, &pEntry->u16Special, 2);
			p8Buf += 2;

			// 1 byte for pattern offset
			*(p8Buf++) = pEntry->u8PatternOffset;
		}

		stRes = uBytesNeeded;
	}
	// else not enough room, so just return 0

	return stRes;
}

VBIC_BOOL VBIC_FromBuffer(VBICompact_t *pDstEntries, size_t stMaxEntries, const void *pSrcBuf, size_t stSrcBufSize)
{
	VBIC_BOOL res = VBIC_FALSE;
	const uint8_t *p8Buf = (const uint8_t *) pSrcBuf;

	// check to see if there are enough entries to house our buffer

	// optimized for AVR
	uint32_t u32FieldCount;
	uint8_t u8NumEntries;

	// check to make sure the version is currect
	if (*(p8Buf++) != 0)
	{
		return VBIC_FALSE;
	}

	u8NumEntries = *(p8Buf++);
	u32FieldCount = *((uint32_t *) p8Buf);
	p8Buf += 4;

	// if we have enough room
	if (u8NumEntries <= stMaxEntries)
	{
		uint8_t u = 0;
		pDstEntries->uEntryCount = u8NumEntries;
		pDstEntries->uTotalFields = u32FieldCount;

		for (u = 0; u < u8NumEntries; u++)
		{
			VBICompactEntry_t *pEntry = &pDstEntries->pEntries[u];
			memcpy(&pEntry->u32StartAbsField, p8Buf, 4);
			p8Buf += 4;
			memcpy(&pEntry->i32StartPictureNumber, p8Buf, 4);
			p8Buf += 4;
			pEntry->typePattern = (FieldPattern) *(p8Buf++);
			memcpy(&pEntry->u16Special, p8Buf, 2);
			p8Buf += 2;
			pEntry->u8PatternOffset = *(p8Buf++);
		}

		res = VBIC_TRUE;
	}
	// else we don't have enough room, give up

	return res;
}
