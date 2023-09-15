#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"

#define PIXELS_X 8
#define PIXELS_Y 30
#define NUM_PIXELS 240
#define DMX_MAX_VALUE 256.
#define DMX_START_CHANNEL 1
#define DMX_NUM_CHANNELS 6

typedef uint64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;


typedef void (*pattern)(u8 *buffer, u64 t, float *dmx_channels);
void draw(uint8_t *buffer, volatile u8 *dmx_buffer, float *dmx_buffer_smooth);

typedef struct {
	float	x;
	float	y;
} vec2_t;

typedef struct {
	float	r;
	float	g;
	float	b;
} color_t;

typedef struct {
	float cursor;

	// below values in dmx range
	vec2_t	zoom;
	vec2_t	offset;
	float	waveform;
	float	spread;
	float	freq;
	color_t	color;
	float	color_hue;
	bool 	invert_color;
} wave_t;

static const color_t nice_skin_color = {
	.r=240,
	.g=108,
	.b=54,
};
