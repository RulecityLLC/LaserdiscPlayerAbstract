/*
 * ldpc.h
 *
 * Copyright (C) 2009 Matt Ownby
 *
 * This file is part of DAPHNE, a laserdisc arcade game emulator
 *
 * DAPHNE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * DAPHNE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef LDPC_H
#define LDPC_H

// ldpc.h
// This is basically a simple state machine that stores the current track/field that the virtual laserdisc player
//  is on depending on input it receives (commands and vblank).

// for VBIMini_t struct
#include "VBICompact.h"
#include "VBIMini.h"
#include "field.h"

// different states that the laserdisc player could be in
typedef enum
{
	LDPC_ERROR, LDPC_SEARCHING, LDPC_STOPPED, LDPC_PLAYING, LDPC_PAUSED, LDPC_SPINNING_UP, LDPC_STEPPING
} LDPCStatus_t;

typedef enum
{
	LDPC_FALSE = 0,
	LDPC_TRUE = 1
} LDPC_BOOL;

typedef enum
{
	LDPC_DISC_NTSC = 0,
	LDPC_DISC_PAL
} LDPCDiscType_t;

typedef enum
{
	LDPC_AUDIO_MUTED = 0,
	LDPC_AUDIO_LEFT_ONLY = 1,
	LDPC_AUDIO_RIGHT_ONLY = 2,
	LDPC_AUDIO_STEREO = 3
} LDPCAudioStatus_t;

typedef enum
{
	LDPC_FORWARD = 0,
	LDPC_BACKWARD = 1,
	LDPC_DIRECTION_COUNT = 2	/* this is just here to make arrFieldOffsetsPerVBlank clearer */
} LDPCDirection_t;

typedef enum
{
	LDPC_AUDIOSQUELCH_NO_CHANGE = 0,	// let ldpc decide whether audio should be squelched
	LDPC_AUDIOSQUELCH_FORCE_ON = 1,	// override ldpc's audio squelching rules and force audio to be squelched
	LDPC_AUDIOSQUELCH_FORCE_OFF = 2	// override ldpc's audio squelching rules and force audio to be unsquelched
} LDPCAudioSquelch_t;

// NTSC VBI cannot define any frame number over 79999
#define LDPC_MAX_FRAME_NUMS (80000)

// for tracking what field the next stop code comes relative to the current field.
// 'undefined' means that no stop code information is available for the current field.
#define LDPC_STOPCODE_UNDEFINED ~0

// holds all state
typedef struct ldpc_state_s
{
	LDPCDiscType_t disctype;
	LDPCStatus_t status;

	// If a disc was stopped and has been started (ie spun up), how many vblanks must occur before the disc has finished spinning up and is playing
	unsigned int stVblanksPerSpin;

	// how many vblanks have elapsed since we first started spinning up (not used unless disc is spinning up)
	unsigned int stVblankSpinupCount;

	// current field we're on (to get current track, divide by 2)
	uint32_t uCurrentField;

	// whether vblank is currently active
	LDPC_BOOL bVblankActive;

	uint32_t uCurrentFrameNum;

	uint32_t uLastSearchedFrameNum;
	uint32_t uLastSearchedField;

	VidField_t vfFrameFirstField;

	// I don't think a real laserdisc player could skip less than 1 track (ie skip just 1 field) due to even/odd field issues
	// Since we support skipping to a frame number, we need this to be a 32-bit value since Firefox is known to do a search from the end of the disc to the beginning during a game over scenario.
	int32_t iSkipTrackOffset;

	LDPC_BOOL bPendingSearchFinished;

	// multi-speed playback
	unsigned int uTracksToStallPerFrame;
	unsigned int uStallTracks;

	// offsets to be added to current field every vblank, this number can be negative (for normal forward play, the offset would be 1)
	// The first index into the array is the direction of play (enum is the index)
	// The second index is the new field represented by the vblank (field offset can be different depending on which field for speeds like 2X)
	int8_t arrFieldOffsetsPerVBlank[LDPC_DIRECTION_COUNT][VID_FIELD_COUNT];	

	// which direction fields are changing
	LDPCDirection_t direction;

	// video
	LDPC_BOOL bVideoMuted;

	// audio
	LDPCAudioStatus_t uAudioStatus;	// 0 - mute, 1 - left, 2 - right, 3 - stereo
	LDPC_BOOL	bAudioSquelchedInternal;	// whether audio is currently squelched (for example if disc is paused, audio is usually squelched)
	LDPCAudioSquelch_t audioSquelchPolicy;	// whether internal audio squelching rules are being overriden

	// contains complete VBI information for the disc
	const VBICompact_t *pVBIC;

	// whether the laserdisc player automatically handles track jumps for things like pausing and multispeed playback
	// (should almost always be false, PR-8210A can be configured to have the game control all track jumps)
	LDPC_BOOL	bDisableAutoTrackJumps;

	// the next field that has a stop code.  Will be LDPC_STOPCODE_UNDEFINED if there isn't one.
	uint32_t uNextStopCodeField;

	// this is so if we pause on a stop code field/frame and issue a play command that we don't pause again
	uint32_t uLastStopCodeFieldThatWePausedOn;

	// the last frame/picture number that we've seen before a step command was issued
	uint32_t uSteppingStartFrameNum;

} ldpc_state_t;

////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif // c++

LDPC_BOOL ldpc_init(LDPCDiscType_t type, const VBICompact_t *pVBIC);

const VBICompact_t *GetCompactVBIData();

LDPC_BOOL ldpc_begin_search(uint32_t uFrameNum);

uint32_t ldpc_get_last_searched_field();

void ldpc_end_search();

LDPC_BOOL ldpc_is_search_finish_pending();

// 16-bit value used here because most games would not skip more than 100 tracks in either direction (I suppose we could use 8-bit if we needed to)
LDPC_BOOL ldpc_skip_tracks(int16_t iTracks);

// Like ldpc_skip_tracks except this will automatically calculate how many tracks to skip in order to arrive at a target frame.
// The target frame will always be properly skipped into, but depending on the current top/bottom field, the skip may occur in the middle of the previous frame.
LDPC_BOOL ldpc_skip_to_frame(uint32_t uFrameNum);

void ldpc_step(LDPCDirection_t dir);

void ldpc_play(LDPCDirection_t dir);

void ldpc_pause();

void ldpc_stop();

LDPC_BOOL ldpc_change_speed(unsigned int uNumerator, unsigned int uDenominator);

// should be used to change status instead of modifying status directly because it recalculates audio squelch
void ldpc_change_status(LDPCStatus_t newStatus);

void ldpc_recalc_audio_squelch();

void ldpc_change_audio(unsigned int uChannel, LDPC_BOOL bEnable);

// returns: 0 - audio muted, 1 - left channel only, 2 - right channel only, 3 - full stereo
LDPCAudioStatus_t ldpc_get_audio_status();

// Manually sets whether the audio is squelched.
// This usually should not be done except in rare cases like Cube Quest where the disc is paused but the audio is supposed to be playing.
void ldpc_set_audio_squelched(LDPCAudioSquelch_t audioSquelchPolicy);

void ldpc_set_video_muted(LDPC_BOOL bMuted);

LDPC_BOOL ldpc_get_video_muted();

void ldpc_set_vblanks_per_spinup(unsigned int stVblanksPerSpin);

// can disable auto track jumps for PR-8210A operation, should be left enabled for (almost?) all other players
void ldpc_set_disable_auto_track_jump(LDPC_BOOL bDisabled);

// sets the next field that has a stopcode, so that we will autostop when we get there.
void ldpc_set_next_field_with_stopcode(uint32_t u32StopCodeField);

// Gets called every time vblank changes.
// Returns true if vblank was processed normally.
// Returns false if fields are out of sync (and thus vblank was ignored).
LDPC_BOOL ldpc_OnVBlankChanged(LDPC_BOOL bVBlankActive, VidField_t field);

uint32_t ldpc_get_cur_frame_num();
uint32_t ldpc_get_current_track();
uint32_t ldpc_get_current_field();
uint32_t ldpc_get_current_abs_field();
void ldpc_set_current_abs_field(unsigned int uField);
uint32_t ldpc_get_current_field_vbi_line18();	// get the VBI data from line 18 (this is to determine whether current field is the beginning of a frame)

LDPCStatus_t ldpc_get_status();

void ldpc_set_error_status();

#ifdef __cplusplus
}
#endif // c++

#endif // LDPC_H
