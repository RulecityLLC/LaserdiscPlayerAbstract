#ifndef VBICOMPACT_H
#define VBICOMPACT_H

#include <stdio.h>	// for size_t
#include "datatypes.h"

/*
THE PURPOSE OF COMPACT VBI:
So that the entire VBI of a disc can be loaded onto an AVR microcontroller, which has very little RAM.
Using a VBI lookup table requires about 512k of RAM, and an AVR may have 2k.  So instead of using a lookup table, the VBI
 has to be describable in a very small amount of space.  Since most VBI follows a pattern, this is doable.
*/

#ifdef __cplusplus
extern "C"
{
#endif // C++

typedef enum
{
	// no change (not supported currently)
	PATTERN_NOCHANGE = 0,

	// 2:2 pattern, frame numbers match up with laserdisc tracks 1:1 (~30 FPS)
	// Pattern starts with the picture number on the first field, then nothing on the second field, repeat ...
	PATTERN_22,

	// 2:2 pattern used by Atari for Firefox/Road Runner, and also used on the Freedom Fighter disc.
	// Pattern starts with the picture number of the first field in the standard form (0xF8????) followed by
	//  the same picture number presented in a non-standard way (0xA8????).
	// On Firefox and Freedom Fighter, a new frame starts on the second field of each track.
	// So the picture number would be 0 but the offset would be 1 to start off a disc in these cases.
	// So for example, field 0 (track 0) would be A80000, field 1 would be F80001, field 2 (track 1) would be A80001, etc
	PATTERN_22_ATARI,

	// 2:3 pattern, frame numbers match up with film frames (~24 FPS)
	// Patterns goes PN1-_-PN2-_-_-PN3-_-PN4-_-_ for every 10 fields, where PN# is a picture number.
	PATTERN_23,

	// lead-in and lead-out on the laserdisc
	PATTERN_LEADIN,
	PATTERN_LEADOUT,

	// generic patterns to cover unknown cases

	// all zeroes (no VBI)
	PATTERN_ZEROES,

	// all the same picture number (this combined with the ZEROES pattern can be used inefficiently to describe any VBI)
	PATTERN_PICNUM,
} FieldPattern;

typedef enum
{
	VBIC_FALSE = 0,
	VBIC_TRUE = 1
} VBIC_BOOL;

// these bits are OR'd with u16Special
#define EVENT_STOPCODE_NOTUSED 0x8000	// this is not used anymore
#define EVENT_CHAPTER  0x4000

typedef struct VBICompactEntry_s
{
	// Field index where the new pattern begins, including if it's incomplete with an offset.
	// It does not refer to where the pattern would begin if it were complete (ie this number can never be negative).
	// For example, if this value is 0, and if pattern type is 2:3 and the offset is 4, then field 0
	//   would be the final field of a 2:3 pattern and the next field would be the beginning of a new complete 2:3 pattern.
	// (this must be a long because on AVR-GCC, int is 16-bits and this number can go up to 120,000 conceivably)
	uint32_t u32StartAbsField;

	// The picture number that correspond to the start of the pattern if it were complete.
	// For example, if the offset is not 0, the start picture number refers to the picture number if the offset were 0.
	// (this must be a long because on AVR-GCC, int is 16-bits)
	// (this must be signed because on 2:3 patterns, the start picture number may be negative)
	int32_t i32StartPictureNumber;

	// which type of pattern it is (if any)
	FieldPattern typePattern;

	// For stop-codes and chapter numbers.
	// Top two bits indicate a stop-code and/or a chapter number.
	// The lower bits are the chapter number (if applicable) and it will be applied to this entry (and only this entry).
	// If the chapter number bit is clear, it means no chapter number for this entry, even if the FieldPattern type is "no change".
	// In other words, if a chapter exists, it must always be specified for every entry.
	uint16_t u16Special;

	// Offset from the start of the pattern (as defined in the comments for the enum's above)
	// 0 means no offset.  Else, the first pattern will be incomplete.  Subsequent patterns will be complete until the pattern changes.
	// Can be 0-1 for 2:2, or anywhere from 0-4 on 2:3.  Anything beyond these ranges will be undefined.
	// (This is a byte to save space on the AVR)
	uint8_t u8PatternOffset;

} VBICompactEntry_t;

typedef struct VBICompact_s
{
	VBICompactEntry_t *pEntries;		// pointer to an array of entries
	uint16_t uEntryCount;			// # of entries in the array	(on most discs this can be stored in 1 byte)
	uint32_t uTotalFields;			// total number of fields that this VBI data represents
} VBICompact_t;

typedef struct VBICompactState_s
{
	uint32_t u32CurAbsField;		// current absolute field
	uint32_t u32CurPictureNum;	// the current (or last) picture number that we've seen
	uint16_t uCurEntryIdx;		// current entry
	const VBICompact_t *pVBI;

	// Holds the 24-bit value of line 18 of the current field's VBI (big endian)
	// This is an 8-bit array because I am targeting the AVR for max speed.  Other architectures will already be fast enough.
	uint8_t pu8CurLine18[3];
} VBICompactState_t;

///////////////////////////////////////////////////////////

// resets all variables to a default state
void VBIC_Init(const VBICompact_t *pVBI);

// returns field that we're currently on
uint32_t VBIC_GetCurAbsField();

// Changes VBI field pointer to a new field.
// 'iOffset' should be 1 if the disc is playing, to advance the VBI pointer to the next field.
// For skipping, larger offsets may be used
// Returns 0 if the field could not be changed (if trying to go to a negative field) or 1 on success.
VBIC_BOOL VBIC_AddField(int16_t iOffset);

// Sets the field that we should be on (AddField will call this)
// Returns non-zero on success
VBIC_BOOL VBIC_SetField(uint32_t uAbsoluteField);

// returns the VBI data for the current field's line 18 (useful for getting the frame number)
uint32_t VBIC_GetCurFieldLine18();

// returns the current (or last) picture number that has been present in VBI
uint32_t VBIC_GetCurPictureNum();

typedef enum
{
	VBIC_SEEK_BUSY,	// not found yet, call the seek function again
	VBIC_SEEK_SUCCESS,	// found it!
	VBIC_SEEK_FAIL	// search finished, frame number does not exist
} VBIC_SeekResult;

// This function finishes instantly; caller will need to add seek delay
// pPeekOnly should be NULL by default.
// If 'pPeekOnly' is not null, it will point to a buffer to receive the absolute field that the framenum corresponds to and no seek will take place (ie this function turns into a lookup function only).
// Reason for this confusion is for performance on an AVR.
VBIC_SeekResult VBIC_SeekInternal(uint32_t uFrameNum, uint32_t *pPeekOnly);

// macros to hopefully make VBIC_SeekInternal less confusing
#define VBIC_SEEK(u) VBIC_SeekInternal(u, 0)
#define VBIC_LOOKUP_FIELD_FOR_FRAMENUM(u, p) VBIC_SeekInternal(u, p)

//////////////////////////////////////////////
// Utility functions

// Calculate what line18 should be based on the current entry and based on the field offset.
// Call VBIC_GetCurFieldLine18 to get the result of this call.
// This is optimized for the AVR which is why it is done this way instead of returning the value here.
void VBIC_LoadLine18(const VBICompactEntry_t *pEntry, int32_t iFieldOffsetFromEntryStart);

void VBIC_SetChapterInfo(VBICompactEntry_t *pEntry, uint8_t uChapter);
void VBIC_ClearChapterInfo(VBICompactEntry_t *pEntry);

// returns TRUE and sets 'puChapter' if a chapter exists
// else returns FALSE
VBIC_BOOL VBIC_GetChapterInfo(const VBICompactEntry_t *pEntry, uint8_t *puChapter);

// Converts a VBICompact_t struct into a byte array, suitable for sending to a remote host (like the AVR)
// Returns the size of resulting byte array, or 0 if there was not enough space in the buffer to hold everything.
size_t VBIC_ToBuffer(void *pBuf, size_t stBufSize, const VBICompact_t *pEntries);

// Populates a pre-existing VBICompact_t structure.
// pEntries must already point to a valid array which will be populated.
// The array's size must be indicated by 'stMaxEntries'.
// The VBI entries must be stored in the 'pSrcBuf' buffer, created using the VBIC_ToBuffer routine.
// Will return TRUE on success or FALSE if 'stMaxEntries' is not big enough.
VBIC_BOOL VBIC_FromBuffer(VBICompact_t *pDstEntries, size_t stMaxEntries, const void *pSrcBuf, size_t stSrcBufSize);

#ifdef __cplusplus
}
#endif // C++

#endif // VBICOMPACT_H
