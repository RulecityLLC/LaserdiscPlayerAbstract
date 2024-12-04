#ifndef FIELD_H
#define FIELD_H

// this enum is to make it easy to remember which field is which
// (in NTSC convention, the odd field is actually the first field which is opposite from the computer science convention of the first item being index 0, an even number)
typedef enum
{
	VID_FIELD_TOP_ODD = 0,
	VID_FIELD_BOTTOM_EVEN = 1,
	VID_FIELD_COUNT = 2	/* this is just here to make arrFieldOffsetsPerVBlank clearer */
} VidField_t;

#endif // FIELD_H
