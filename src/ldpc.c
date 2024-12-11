/*
 * ldpc.c
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

#include <string.h>
#include <assert.h>
#include <ldp-abst/ldpc.h>

// does 'static' make sense here?  I'm not sure.  Whichever one interfers less with real-time OS is preferred.
static ldpc_state_t g_ldpc_state;

LDPC_BOOL ldpc_init(LDPCDiscType_t type, const VBICompact_t *pVBIC)
{
	LDPC_BOOL res = LDPC_TRUE;

	memset(&g_ldpc_state, 0, sizeof(g_ldpc_state));
	g_ldpc_state.disctype = type;
	g_ldpc_state.status = LDPC_STOPPED;	// disc starts off stopped
	g_ldpc_state.bVblankActive = LDPC_FALSE;
	g_ldpc_state.uCurrentField = (uint32_t) -1;	// so when we play the disc, the next field is the first one (0)
	g_ldpc_state.uCurrentFrameNum = VBIMiniNoFrame;
	g_ldpc_state.direction = LDPC_FORWARD;	// default to playing forward
	g_ldpc_state.bVideoMuted = LDPC_FALSE;	// default is video is not muted
	g_ldpc_state.uAudioStatus = 3;	// full stereo by default
	g_ldpc_state.bAudioSquelchedInternal = LDPC_TRUE;	// audio squelched is a safe default
	g_ldpc_state.audioSquelchPolicy = LDPC_AUDIOSQUELCH_NO_CHANGE;
	g_ldpc_state.pVBIC = pVBIC;
	g_ldpc_state.uNextStopCodeField = LDPC_STOPCODE_UNDEFINED;
	ldpc_change_speed(1, 1);	// setup field offsets for default values (1X)

	// initialize compact VBI routines
	VBIC_Init(pVBIC);

	// no way to fail right now
	res = LDPC_TRUE;

	return res;
}

const VBICompact_t *GetCompactVBIData()
{
	return g_ldpc_state.pVBIC;
}

LDPC_BOOL ldpc_begin_search(uint32_t uFrameNum)
{
	LDPC_BOOL bRes = LDPC_FALSE;

	VBIC_SeekResult res;
	res = VBIC_SEEK(uFrameNum);
	if (res == VBIC_SEEK_SUCCESS)
	{
		bRes = LDPC_TRUE;
		ldpc_change_status(LDPC_SEARCHING);

		g_ldpc_state.uLastSearchedField = VBIC_GetCurAbsField();
		g_ldpc_state.uLastSearchedFrameNum = uFrameNum;
		g_ldpc_state.uLastStopCodeFieldThatWePausedOn = 0;	// searching resets this
		g_ldpc_state.vfFrameFirstField = (VidField_t) (g_ldpc_state.uLastSearchedField & 1);	// so we know when we can end search
	}

	if (!bRes)
	{
		ldpc_change_status(LDPC_ERROR);
	}

	return bRes;
}

uint32_t ldpc_get_last_searched_field()
{
	return g_ldpc_state.uLastSearchedField;
}

void ldpc_end_search()
{
	g_ldpc_state.bPendingSearchFinished = LDPC_TRUE;
}

LDPC_BOOL ldpc_is_search_finish_pending()
{
	return g_ldpc_state.bPendingSearchFinished;
}

LDPC_BOOL ldpc_skip_tracks(int16_t iTracks)
{
	LDPC_BOOL bRes = LDPC_FALSE;
	if ((g_ldpc_state.status == LDPC_PLAYING) || (g_ldpc_state.status == LDPC_PAUSED))
	{
		g_ldpc_state.iSkipTrackOffset += iTracks;
		bRes = LDPC_TRUE;
	}
	return bRes;
}

LDPC_BOOL ldpc_skip_to_frame(uint32_t uFrameNum)
{
	LDPC_BOOL bRes = LDPC_FALSE;

	// if current field is valid
	if (g_ldpc_state.uCurrentField != ~0)
	{
		// if we're paused or playing
		//if ((g_ldpc_state.status == LDPC_PLAYING) || (g_ldpc_state.status == LDPC_PAUSED))
		// UPDATE: for now only going to support PLAYING because I don't know of any LDP driver that needs to skip while paused and there is more dev work involved for adding paused support
		if (g_ldpc_state.status == LDPC_PLAYING)
		{
			uint32_t uTargetField = 0;

			// if we are able to figure out what the field that corresponds to the frame number is
			if (VBIC_LOOKUP_FIELD_FOR_FRAMENUM(uFrameNum, &uTargetField) == VBIC_SEEK_SUCCESS)
			{
				// it is possible to skip from the end of the disc to the beginning, so we need more than a signed 16-bit value
				int32_t iSkipFieldOffset = (int32_t) (uTargetField - g_ldpc_state.uCurrentField);

				// if disc is playing and we aren't in the middle of vblank, subtract 1 because the field will increase by one on next vblank
				if ((g_ldpc_state.status == LDPC_PLAYING) && (!g_ldpc_state.bVblankActive))
				{
					iSkipFieldOffset--;
				}

				g_ldpc_state.iSkipTrackOffset = (iSkipFieldOffset >> 1);
				bRes = LDPC_TRUE;
			}
		}
	}

	return bRes;
}

void ldpc_step(LDPCDirection_t dir)
{
	g_ldpc_state.uSteppingStartFrameNum = g_ldpc_state.uCurrentFrameNum;
	g_ldpc_state.direction = dir;
	ldpc_change_status(LDPC_STEPPING);
}

void ldpc_play(LDPCDirection_t dir)
{
	g_ldpc_state.direction = dir;

	// if the disc is stopped, always go into SPINNING_UP status because our current track/field is -1 so we want to 
	//  wait until vblank before deciding whether the disc is playing
	if (g_ldpc_state.status == LDPC_STOPPED)
	{
		ldpc_change_status(LDPC_SPINNING_UP);
		g_ldpc_state.stVblankSpinupCount = 0;
	}
	// for VP-931 support, play commands should be ignored if we are in the middle of a search
	else if (g_ldpc_state.status == LDPC_SEARCHING)
	{
		// nothing to do
	}
	// else no spinup delay because the disc was not stopped
	else
	{
		ldpc_change_status(LDPC_PLAYING);
	}
}

void ldpc_pause()
{
	// only honor pause command if the disc is currently playing/stepping.  If it's already paused, in the middle of a seek, etc,
	//  just ignore it.
	if ((g_ldpc_state.status != LDPC_PLAYING) && (g_ldpc_state.status != LDPC_STEPPING))
	{
		return;
	}

	// only allow pausing if we have already established ourselves on a real track
	if (g_ldpc_state.uCurrentField != ~0)
	{
		ldpc_change_status(LDPC_PAUSED);
	}
	else
	{
		ldpc_change_status(LDPC_ERROR);
	}
}

void ldpc_stop()
{
	ldpc_change_status(LDPC_STOPPED);

	g_ldpc_state.uCurrentFrameNum = VBIMiniNoFrame;
	g_ldpc_state.uCurrentField = (uint32_t) -1;	// so that when we play the disc we'll move to field 0

	// this probably doesn't matter but it's a good default
	VBIC_SetField(0);
}

LDPC_BOOL ldpc_change_speed(unsigned int uNumerator, unsigned int uDenominator)
{
	LDPC_BOOL bRes = LDPC_TRUE;

	// if this is >= 1X
	if (uDenominator == 1)
	{
		g_ldpc_state.uTracksToStallPerFrame = 0;	// don't want to stall at all

		// if it's 0, it is illegal (use pause() instead unless the game driver specifically wants to do this, in which case more coding is needed)
		if (uNumerator == 0)
		{
			bRes = LDPC_FALSE;
		}
		// else it's okay, we'll handle it at the end of the function
	}
	// else if this is < 1X
	else if (uNumerator == 1)
	{
		// protect against divide by zero
		if (uDenominator > 0)
		{
			g_ldpc_state.uTracksToStallPerFrame = uDenominator - 1;	// show 1, stall for the rest
			g_ldpc_state.uStallTracks = g_ldpc_state.uTracksToStallPerFrame;	// so that our changes take effect immediately
		}
		// divide by zero situation
		else
		{
			g_ldpc_state.uTracksToStallPerFrame = 0;
			bRes = LDPC_FALSE;
		}
	}
	// else it's a non-standard speed, so do some kind of error
	else
	{
		bRes = LDPC_FALSE;
		uNumerator = uDenominator = 1;
	}

	// if we didn't have any errors, set the field offsets based on whatever numerator we got
	if (bRes)
	{
		// Algorithm: if it's an odd speed, set field offset to the speed (simple).
		// if it's an even speed, set top field offset to the speed + 1, set bottom field offset to the speed - 1
		// I got the odd speed algorithm by observing how the LDP-1450 behaves.
		// I arrived at the even speed algorithm by manually figuring out what 2X, 4X, 6X, 8X should be and observing a pattern.

		// if it's odd
		if ((uNumerator & 1) == 1)
		{
			g_ldpc_state.arrFieldOffsetsPerVBlank[LDPC_FORWARD][VID_FIELD_TOP_ODD] = uNumerator;
			g_ldpc_state.arrFieldOffsetsPerVBlank[LDPC_FORWARD][VID_FIELD_BOTTOM_EVEN] = uNumerator;
			g_ldpc_state.arrFieldOffsetsPerVBlank[LDPC_BACKWARD][VID_FIELD_TOP_ODD] = -((int) uNumerator);
			g_ldpc_state.arrFieldOffsetsPerVBlank[LDPC_BACKWARD][VID_FIELD_BOTTOM_EVEN] = -((int) uNumerator);
		}
		// else if it's even
		else
		{
			g_ldpc_state.arrFieldOffsetsPerVBlank[LDPC_FORWARD][VID_FIELD_TOP_ODD] = uNumerator + 1;
			g_ldpc_state.arrFieldOffsetsPerVBlank[LDPC_FORWARD][VID_FIELD_BOTTOM_EVEN] = uNumerator - 1;
			g_ldpc_state.arrFieldOffsetsPerVBlank[LDPC_BACKWARD][VID_FIELD_TOP_ODD] = -((int) uNumerator + 1);
			g_ldpc_state.arrFieldOffsetsPerVBlank[LDPC_BACKWARD][VID_FIELD_BOTTOM_EVEN] = -((int) uNumerator - 1);
		}
	}

	return bRes;
}

void ldpc_change_status(LDPCStatus_t newStatus)
{
	g_ldpc_state.status = newStatus;

	ldpc_recalc_audio_squelch();
}

void ldpc_recalc_audio_squelch()
{
	LDPC_BOOL squelched = LDPC_TRUE;

	// if we are playing forward, audio should not be squelched by default
	if ((g_ldpc_state.direction == LDPC_FORWARD) &&
		(g_ldpc_state.status == LDPC_PLAYING)
		)
	{
		squelched = LDPC_FALSE;
	}

	g_ldpc_state.bAudioSquelchedInternal = squelched;
}

void ldpc_change_audio(unsigned int uChannel, LDPC_BOOL bEnable)
{
	if (bEnable)
	{
		g_ldpc_state.uAudioStatus |= (1 << uChannel);
	}
	else
	{
		g_ldpc_state.uAudioStatus &= (~(1 << uChannel));
	}
}

// returns: 0 - audio muted, 1 - left channel only, 2 - right channel only, 3 - full stereo
LDPCAudioStatus_t ldpc_get_audio_status()
{
	LDPCAudioStatus_t uRes = g_ldpc_state.uAudioStatus;

	// if the audio is squelched, it overrides any other status
	if ((g_ldpc_state.bAudioSquelchedInternal && (g_ldpc_state.audioSquelchPolicy != LDPC_AUDIOSQUELCH_FORCE_OFF)) ||
		(g_ldpc_state.audioSquelchPolicy == LDPC_AUDIOSQUELCH_FORCE_ON))
	{
		uRes = LDPC_AUDIO_MUTED;
	}

	return uRes;
}

void ldpc_set_audio_squelched(LDPCAudioSquelch_t policy)
{
	g_ldpc_state.audioSquelchPolicy = policy;
}

void ldpc_set_video_muted(LDPC_BOOL bMuted)
{
	g_ldpc_state.bVideoMuted = bMuted;
}

LDPC_BOOL ldpc_get_video_muted()
{
	return g_ldpc_state.bVideoMuted;
}

void ldpc_set_vblanks_per_spinup(unsigned int stVblanksPerSpin)
{
	g_ldpc_state.stVblanksPerSpin = stVblanksPerSpin;
}

void ldpc_set_disable_auto_track_jump(LDPC_BOOL bDisabled)
{
	g_ldpc_state.bDisableAutoTrackJumps = bDisabled;
}

void ldpc_set_next_field_with_stopcode(uint32_t u32StopCodeField)
{
	g_ldpc_state.uNextStopCodeField = u32StopCodeField;
}

// should be called every time the current field changes
void ldpc_on_current_field_changed()
{
	// Check for out of range.
	// If we're out of range, then start back at the beginning
	// (this should also handle trying to skip before the first field too since uCurrentField is unsigned
	if (g_ldpc_state.uCurrentField >= g_ldpc_state.pVBIC->uTotalFields)
	{
		// go back to absolute field 0 or 1, depending on the current relative field that we're on
		g_ldpc_state.uCurrentField = (g_ldpc_state.uCurrentField & 1);
	}

	VBIC_SetField(g_ldpc_state.uCurrentField);

	// current picture number will be up-to-date at this point
	g_ldpc_state.uCurrentFrameNum = VBIC_GetCurPictureNum();
}

void ldpc_process_skipping()
{
	if (g_ldpc_state.iSkipTrackOffset != 0)
	{
		g_ldpc_state.uCurrentField += (g_ldpc_state.iSkipTrackOffset << 1);
		g_ldpc_state.iSkipTrackOffset = 0;

		ldpc_on_current_field_changed();
	}
}

LDPC_BOOL ldpc_OnVBlankChanged(LDPC_BOOL bVBlankActive, VidField_t field)
{
	LDPC_BOOL bRes = LDPC_FALSE;

	if (bVBlankActive)
	{
		LDPC_BOOL bSkipPlayPausedStuff = LDPC_FALSE;

		// if we are in the middle of a search, find out if the search has completed
		// NOTE : this must come before we check to see if uWhichField aligns with uCurrentField because our current field is undefined if we're searching
		if (g_ldpc_state.status == LDPC_SEARCHING)
		{
			// if the search has finished
			if (g_ldpc_state.bPendingSearchFinished)
			{
				// if we're on the first field of the frame, then the search can complete now
				if (field == g_ldpc_state.vfFrameFirstField)
				{
					g_ldpc_state.bPendingSearchFinished = LDPC_FALSE;

					g_ldpc_state.uCurrentFrameNum = g_ldpc_state.uLastSearchedFrameNum;
					g_ldpc_state.uCurrentField = g_ldpc_state.uLastSearchedField;

					ldpc_change_status(LDPC_PAUSED);

					bSkipPlayPausedStuff = LDPC_TRUE;	// since we just changed our status to PAUSED, we don't want to keep going and change our current field again
				}
				// else wait for the next field to end the search
			}
			// else the search is busy and still going, so don't change status
		}

		// Else if we're spinning up, add to the spinning up vblank counter.
		// (this check must come here because the current field (uWhichField) may not match the disc current field)
		else if (g_ldpc_state.status == LDPC_SPINNING_UP)
		{
			g_ldpc_state.stVblankSpinupCount++;
		}

		// if disc and video display are not in sync, then the following assumptions are wrong and therefore the code must be skipped (wait til next vblank)
		// (this happens when disc is first spun up, or when we do a search that takes an unknown amount of time)
		if (field == (VidField_t) (g_ldpc_state.uCurrentField & 1))
		{
			goto skip;
		}

		if (!bSkipPlayPausedStuff)
		{
			// check to see if we need to transition from 'spinning up' to 'playing'
			if (g_ldpc_state.status == LDPC_SPINNING_UP)
			{
				// if we're done spinning up
				if (g_ldpc_state.stVblankSpinupCount >= g_ldpc_state.stVblanksPerSpin)
				{
					ldpc_change_status(LDPC_PLAYING);
					g_ldpc_state.stVblankSpinupCount = 0;
				}
			}

			// if we are playing or if auto track jumps are disabled
			if ((g_ldpc_state.status == LDPC_PLAYING) || (g_ldpc_state.bDisableAutoTrackJumps))
			{
#ifndef __AVR__
				// flag us if we need to implement no track jump for multispeed playback
				assert((g_ldpc_state.bDisableAutoTrackJumps == LDPC_FALSE) || (g_ldpc_state.arrFieldOffsetsPerVBlank[g_ldpc_state.direction][field] == 1));
#endif // AVR

				// move fields according to our offset (this handles reverse and/or multispeed playback)
				g_ldpc_state.uCurrentField += g_ldpc_state.arrFieldOffsetsPerVBlank[g_ldpc_state.direction][field];

				// if we've hit a stop code and we haven't already paused on it
				if ((g_ldpc_state.uCurrentField == g_ldpc_state.uNextStopCodeField) && (g_ldpc_state.uLastStopCodeFieldThatWePausedOn != g_ldpc_state.uNextStopCodeField))
				{
					// call the method instead of setting the variable ourselves to make sure everything that needs to happen when pausing does happen
					ldpc_pause();

					// make sure we can issue a play command from this frame and move passed the stop code
					g_ldpc_state.uLastStopCodeFieldThatWePausedOn = g_ldpc_state.uCurrentField;
				}

				// if we've just started a new track ...
				if ((g_ldpc_state.uCurrentField & 1) == 0)
				{
					// FOR PLAYING AT SLOWER THAN 1X (such as 1/2X)
					// if we have no frames to stall, then check to see if we need some frames to stall
					if (g_ldpc_state.uStallTracks == 0)
					{
						g_ldpc_state.uStallTracks = g_ldpc_state.uTracksToStallPerFrame;
					}
					// otherwise, we do have frames to stall, so adjust the current field accordingly
					else
					{
#ifndef __AVR__
						// flag us if we need to implement no track jump for multispeed playback
						assert(g_ldpc_state.bDisableAutoTrackJumps == LDPC_FALSE);
#endif // AVr

						// Go back an entire frame so that the frame we just displayed will be displayed again.
						// NOTE : this is safe because this number will never be negative because we incremented it before we got to this point,
						//  and it must be an even number, so it must be >=2
						g_ldpc_state.uCurrentField -= 2;
						
						g_ldpc_state.uStallTracks--;
					}
					// END PLAYING AT SLOWER THAN 1X

				} // end if this is a new track

				// current field has changed at this point
				ldpc_on_current_field_changed();

			} // end if disc is playing

			else if (g_ldpc_state.status == LDPC_PAUSED)
			{
				// When paused we must alternate back and forth between the fields that make up the current frame.

				uint32_t u32VBI = VBIC_GetCurFieldLine18();
				uint8_t u8Top = (u32VBI >> 16) & 0xF0;

				// NOTE: overflow situations are handled by ldpc_on_current_field_changed()
				// if we are currently on a picture number, advance to the next field
				if (u8Top == 0xF0)
				{
					g_ldpc_state.uCurrentField++;
				}
				// else go backward until we land on a picnum again
				else
				{
					g_ldpc_state.uCurrentField--;
				}

				ldpc_on_current_field_changed();
			}
			else if (g_ldpc_state.status == LDPC_STEPPING)
			{
				// if we are stepping forward
				if (g_ldpc_state.direction == LDPC_FORWARD)
				{
					// fwd stepping is simple
					g_ldpc_state.uCurrentField++;
				}
				// else reverse
				else
				{
					// if we aren't at the beginning of the disc
					if (g_ldpc_state.uCurrentField > 0)
					{
						g_ldpc_state.uCurrentField --;
					}
					// else we can't step backward any further, so pause
					else
					{
						g_ldpc_state.uCurrentField++;
						ldpc_pause();
					}
				}

				ldpc_on_current_field_changed();

				// if we've reached a new picture number than when we started stepping
				if (g_ldpc_state.uCurrentFrameNum != g_ldpc_state.uSteppingStartFrameNum)
				{
					// we're done stepping
					ldpc_pause();
				}
				// else keep stepping until we get to the next new picture number
			}

		} // end if we checked for play/pause status

		// do any skippin that took place before vblank started
		// (VP931 will know its new VBI info before vblank ends so we need to process it here)
		ldpc_process_skipping();

	}
	// else vblank is ending, not starting
	else
	{
		// do any skipping that took place during vblank
		// (VBI info should be up to date when vblank ends)
		ldpc_process_skipping();
	}

	bRes = LDPC_TRUE;

skip:

#ifndef __AVR__
	// when paused or playing,
	//  disc field should be lined up with video field
	assert (
		((g_ldpc_state.status != LDPC_PLAYING) && (g_ldpc_state.status != LDPC_PAUSED))
		||
		(!bVBlankActive)
		||
		// vblank must be active for this assert because that's the only time we adjust the current field to pass this assert
		(field == (g_ldpc_state.uCurrentField & 1))
		);
#endif // not AVR

	g_ldpc_state.bVblankActive = bVBlankActive;

	return bRes;
}

uint32_t ldpc_get_cur_frame_num()
{
	return g_ldpc_state.uCurrentFrameNum;
}

uint32_t ldpc_get_current_track()
{
	uint32_t uRes = VBIMiniNoFrame;

	if ((g_ldpc_state.status == LDPC_PLAYING) || (g_ldpc_state.status == LDPC_PAUSED))
	{
		uRes = g_ldpc_state.uCurrentField >> 1;
	}

	return uRes;
}

uint32_t ldpc_get_current_field()
{
	uint32_t uRes = VBIMiniNoFrame;

	// if disc is not stopped
	if ((g_ldpc_state.status == LDPC_PLAYING) || (g_ldpc_state.status == LDPC_PAUSED))
	{
		uRes = g_ldpc_state.uCurrentField & 1;
	}

	return uRes;
}

uint32_t ldpc_get_current_abs_field()
{
	uint32_t uRes = VBIMiniNoFrame;

	// if disc is not stopped
	if ((g_ldpc_state.status == LDPC_PLAYING) || (g_ldpc_state.status == LDPC_PAUSED) || (g_ldpc_state.status == LDPC_STEPPING))
	{
		uRes = g_ldpc_state.uCurrentField;
	}

	return uRes;
}

void ldpc_set_current_abs_field(unsigned int uField)
{
	g_ldpc_state.status = LDPC_PAUSED;	// only VLDP-HW uses this and it needs valid results from ldpc_get_current_abs_field
	g_ldpc_state.uCurrentField = uField;
	VBIC_SetField(g_ldpc_state.uCurrentField);
}

uint32_t ldpc_get_current_field_vbi_line18()
{
	return VBIC_GetCurFieldLine18();
}

LDPCStatus_t ldpc_get_status()
{
	return g_ldpc_state.status;
}

void ldpc_set_error_status()
{
	g_ldpc_state.status = LDPC_ERROR;
}
