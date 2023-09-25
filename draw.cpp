#include "leds_array.h"
#define PI 3.14159265359
#define NUM_PATTERNS count_of(pattern_table)

u8 *current_buffer_pixel;

void put_pixel_x(u8 *buffer, size_t x, uint32_t pixel_grb) {
	buffer += 4 * x;
	buffer[1] = pixel_grb & 0xffu;
	buffer[2] = (pixel_grb >> 8u) & 0xffu;
	buffer[3] = (pixel_grb >> 16u) & 0xffu;
}

void add_pixel_x(u8 *buffer, size_t x, uint32_t pixel_grb) {
	u16 tmp;
	buffer += 4 * x;
	tmp = pixel_grb & 0xffu + buffer[1];
	buffer[1] = (tmp > 255) ? 255 : tmp;
	tmp = ((pixel_grb >> 8u) & 0xffu) + buffer[2];
	buffer[2] = (tmp > 255) ? 255 : tmp;
	tmp = ((pixel_grb >> 16u) & 0xffu) + buffer[3];
	buffer[3] = (tmp > 255) ? 255 : tmp;
}

void put_pixel_xy(u8 *buffer, size_t x, size_t y, uint32_t pixel_grb) {
  const uint8_t XYTable[] = {
    29,  30,  89,  90, 149, 150, 209, 210,
    28,  31,  88,  91, 148, 151, 208, 211,
    27,  32,  87,  92, 147, 152, 207, 212,
    26,  33,  86,  93, 146, 153, 206, 213,
    25,  34,  85,  94, 145, 154, 205, 214,
    24,  35,  84,  95, 144, 155, 204, 215,
    23,  36,  83,  96, 143, 156, 203, 216,
    22,  37,  82,  97, 142, 157, 202, 217,
    21,  38,  81,  98, 141, 158, 201, 218,
    20,  39,  80,  99, 140, 159, 200, 219,
    19,  40,  79, 100, 139, 160, 199, 220,
    18,  41,  78, 101, 138, 161, 198, 221,
    17,  42,  77, 102, 137, 162, 197, 222,
    16,  43,  76, 103, 136, 163, 196, 223,
    15,  44,  75, 104, 135, 164, 195, 224,
    14,  45,  74, 105, 134, 165, 194, 225,
    13,  46,  73, 106, 133, 166, 193, 226,
    12,  47,  72, 107, 132, 167, 192, 227,
    11,  48,  71, 108, 131, 168, 191, 228,
    10,  49,  70, 109, 130, 169, 190, 229,
     9,  50,  69, 110, 129, 170, 189, 230,
     8,  51,  68, 111, 128, 171, 188, 231,
     7,  52,  67, 112, 127, 172, 187, 232,
     6,  53,  66, 113, 126, 173, 186, 233,
     5,  54,  65, 114, 125, 174, 185, 234,
     4,  55,  64, 115, 124, 175, 184, 235,
     3,  56,  63, 116, 123, 176, 183, 236,
     2,  57,  62, 117, 122, 177, 182, 237,
     1,  58,  61, 118, 121, 178, 181, 238,
     0,  59,  60, 119, 120, 179, 180, 239
  };

  assert(x < PIXELS_X);
  assert(y < PIXELS_Y);
  uint8_t i = ((PIXELS_Y - 1 - y) * PIXELS_X) + x;
  put_pixel_x(buffer, XYTable[i], pixel_grb);
}

void add_pixel_xy(u8 *buffer, size_t x, size_t y, uint32_t pixel_grb) {
  const uint8_t XYTable[] = {
    29,  30,  89,  90, 149, 150, 209, 210,
    28,  31,  88,  91, 148, 151, 208, 211,
    27,  32,  87,  92, 147, 152, 207, 212,
    26,  33,  86,  93, 146, 153, 206, 213,
    25,  34,  85,  94, 145, 154, 205, 214,
    24,  35,  84,  95, 144, 155, 204, 215,
    23,  36,  83,  96, 143, 156, 203, 216,
    22,  37,  82,  97, 142, 157, 202, 217,
    21,  38,  81,  98, 141, 158, 201, 218,
    20,  39,  80,  99, 140, 159, 200, 219,
    19,  40,  79, 100, 139, 160, 199, 220,
    18,  41,  78, 101, 138, 161, 198, 221,
    17,  42,  77, 102, 137, 162, 197, 222,
    16,  43,  76, 103, 136, 163, 196, 223,
    15,  44,  75, 104, 135, 164, 195, 224,
    14,  45,  74, 105, 134, 165, 194, 225,
    13,  46,  73, 106, 133, 166, 193, 226,
    12,  47,  72, 107, 132, 167, 192, 227,
    11,  48,  71, 108, 131, 168, 191, 228,
    10,  49,  70, 109, 130, 169, 190, 229,
     9,  50,  69, 110, 129, 170, 189, 230,
     8,  51,  68, 111, 128, 171, 188, 231,
     7,  52,  67, 112, 127, 172, 187, 232,
     6,  53,  66, 113, 126, 173, 186, 233,
     5,  54,  65, 114, 125, 174, 185, 234,
     4,  55,  64, 115, 124, 175, 184, 235,
     3,  56,  63, 116, 123, 176, 183, 236,
     2,  57,  62, 117, 122, 177, 182, 237,
     1,  58,  61, 118, 121, 178, 181, 238,
     0,  59,  60, 119, 120, 179, 180, 239
  };

  assert(x < PIXELS_X);
  assert(y < PIXELS_Y);
  uint8_t i = ((PIXELS_Y - 1 - y) * PIXELS_X) + x;
  add_pixel_x(buffer, XYTable[i], pixel_grb);
}


static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (g) << 16) |
            ((uint32_t) (r) << 8) |
            (uint32_t) (b);
}

u32 hsv_u32(float H, float S, float V) {
	float r, g, b;
	
	float h = H / 360;
	float s = S / 100;
	float v = V / 100;
	
	int i = floor(h * 6);
	float f = h * 6 - i;
	float p = v * (1 - s);
	float q = v * (1 - f * s);
	float t = v * (1 - (1 - f) * s);
	
	switch (i % 6) {
		case 0: r = v, g = t, b = p; break;
		case 1: r = q, g = v, b = p; break;
		case 2: r = p, g = v, b = t; break;
		case 3: r = p, g = q, b = v; break;
		case 4: r = t, g = p, b = v; break;
		case 5: r = v, g = p, b = q; break;
	}
	
	return urgb_u32(r * 255, g * 255, b * 255);
}

static void pattern_snakes(u8 *buffer, u64 t, float *dmx_channels) {
    for (uint i = 0; i < NUM_PIXELS; ++i) {
        uint x = (i + (t >> 1)) % 64;
        if (x < 10)
            put_pixel_x(buffer, i, urgb_u32(0xff, 0, 0));
        else if (x >= 15 && x < 25)
            put_pixel_x(buffer, i, urgb_u32(0, 0xff, 0));
        else if (x >= 30 && x < 40)
            put_pixel_x(buffer, i, urgb_u32(0, 0, 0xff));
        else
            put_pixel_x(buffer, i, 0);
    }
}

static void pattern_random(u8 *buffer, u64 t, float *dmx_channels) {
    if (t % 8)
        return;
    for (uint i = 0; i < NUM_PIXELS; ++i)
        put_pixel_x(buffer, i, rand());
}

static void pattern_sparkle(u8 *buffer, u64 t, float *dmx_channels) {
    if (t % 8)
        return;
    for (uint i = 0; i < NUM_PIXELS; ++i)
        put_pixel_x(buffer, i, rand() % 16 ? 0 : 0xffffffff);
}

static void pattern_greys(u8 *buffer, u64 t, float *dmx_channels) {
    uint max = 100; // let's not draw too much current!
    t %= max;
    for (uint i = 0; i < NUM_PIXELS; ++i) {
        put_pixel_x(buffer, i, t * 0x10101);
        if (++t >= max) t = 0;
    }
}

static void blank_screen(u8 *buffer) {
	for (uint i = 0; i < NUM_PIXELS; i++) {
		put_pixel_x(buffer, i, 0);
	}
}

static void pattern_blank(u8 *buffer, u64 t, float *dmx_channels) {
	(void)t;
	blank_screen(buffer);
}

static void pattern_solid(u8 *buffer, u64 t, float *dmx_channels) {
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;

	(void)t;
    for (uint i = 0; i < NUM_PIXELS; ++i) {
        put_pixel_x(buffer, i, urgb_u32(r, g, b));
    }
}

static inline float triangle(float x, float p) {
   	return 2. * abs(x / p - floor(x / p + 1. / 2.));
}

static inline float faster_edge(float x) {
	return pow(x, 3);
}

static void pattern_solid_breathe(u8 *buffer, u64 t, float *dmx_channels) {
	static u64 prev_t = 0;
	static float x = 0;

	float a = dmx_channels[4] / 255.;

	printf("here is chan 5: %f\n", dmx_channels[5]);
	x += (((float)(t - prev_t)) / 1000) * faster_edge((float)(dmx_channels[5]) / 256.);
	float p = 100;

	unsigned char r = dmx_channels[1] * (1. - (a * triangle(x, p)));
	unsigned char g = dmx_channels[2] * (1. - (a * triangle(x, p)));
	unsigned char b = dmx_channels[3] * (1. - (a * triangle(x, p)));

    for (uint i = 0; i < NUM_PIXELS; ++i) {
        put_pixel_x(buffer, i, urgb_u32(r, g, b));
    }

	prev_t = t;
}


static void pattern_demo(u8 *buffer, u64 t, float *dmx_channels) {
	static float cursor = 0;
	float x;
	float y;
	blank_screen(buffer);

	cursor = t / 1000.;
    for (uint i = 0; i < PIXELS_X; ++i) {
		for (uint j = 0; i < PIXELS_Y; ++j) {
			x = i / (float)PIXELS_X;
		}
	}
}


static void did_dmx_channels_change(bool *dmx_channels_changed, float *dmx_channels) {
	static float dmx_channels_prev[DMX_NUM_CHANNELS];
	dmx_channels += DMX_START_CHANNEL;

	printf("\n");

	for (int i = 0; i < DMX_NUM_CHANNELS; i++) {
		dmx_channels_changed[i] = (fabsf(dmx_channels_prev[i] - dmx_channels[i]) > 0.00001) ? true : false;
	}
	memcpy(dmx_channels_prev, dmx_channels, DMX_NUM_CHANNELS * sizeof(*dmx_channels));
}

static void pattern_sine_update_current_wave_params(wave_t *wave, float *dmx_channels, bool const *dmx_chan_changed, bool hardcore_mode, u32 meta) {
	if (!hardcore_mode) {
		if (dmx_chan_changed[0])
			wave->waveform = dmx_channels[1];
		if (dmx_chan_changed[1]) {
			wave->color_hue = dmx_channels[2];
		}
		if (dmx_chan_changed[2]) {
			wave->spread = dmx_channels[3];
		}
		if (dmx_chan_changed[3]) {
			wave->zoom.x = dmx_channels[4];
		}
		if (dmx_chan_changed[4]) {
			wave->freq = dmx_channels[5];
		}
	} else {
		wave->color_hue = -1.;
		if (meta == 0) {
			if (dmx_chan_changed[1]) {
				wave->color.r = dmx_channels[2];
			}
			if (dmx_chan_changed[2]) {
				wave->color.g = dmx_channels[3];
			}
			if (dmx_chan_changed[3]) {
				wave->color.b = dmx_channels[4];
			}
		}
		else if (meta == 1) {
			if (dmx_chan_changed[1]) {
				wave->waveform = dmx_channels[2];
			}
			if (dmx_chan_changed[2]) {
				wave->spread = dmx_channels[3];
			}
			if (dmx_chan_changed[3]) {
				wave->freq = dmx_channels[4];
			}
		}
		else if (meta == 2) {
			if (dmx_chan_changed[1]) {
				wave->zoom.x = dmx_channels[2];
			}
			if (dmx_chan_changed[2]) {
				wave->zoom.y = dmx_channels[3];
			}
			if (dmx_chan_changed[3]) {
				wave->offset.x = dmx_channels[4];
			}
			if (dmx_chan_changed[4]) {
				wave->offset.y = dmx_channels[5];
			}
		}
		else if (meta == 3) {
			if (dmx_chan_changed[1]) {
				(void)0;
			}
		}
	}
}

static void pattern_sine_draw_wave(u8 *buffer, wave_t *wave) {
	float x;
	float fx;
	float y;
	float diff;
	float lum;
	u32 color;
	
	for (uint i = 0; i < PIXELS_X; i++) {
		for (uint j = 0; j < PIXELS_Y; j++) {
			x = wave->cursor;
			x += i * (wave->zoom.x / DMX_MAX_VALUE) * 1.5;
			//x += (wave->offset.x / DMX_MAX_VALUE - 0.5) * 2;

			y = (float)j / (PIXELS_Y - 1) * 1;
			//y += ((wave->offset.y) / DMX_MAX_VALUE - 0.5) * 2);

			fx = (cos(x) + 1) / 2;

			if (fx < y) {
				diff = y - fx;
			}
			else {
				diff = fx - y;
			}

			lum = (1. - diff * ((DMX_MAX_VALUE -1 - wave->spread) / 5)); 
			if (lum >= 0.1) {
				if (wave->color_hue >= 0.) {
					color = hsv_u32(360 * wave->color_hue / DMX_MAX_VALUE, 100, 100 * lum);
				} else {
					color = urgb_u32(wave->color.r * lum, wave->color.g * lum, wave->color.b * lum);
					if (wave->invert_color) {
						color = (color) ? 0 : urgb_u32(0xff, 0xff, 0xff);
					}
				}
				put_pixel_xy(buffer, i, j, color);
			}
		}
	}
}

const u32 meta_colors[] = {
	0x00ff00,
	0x0000ff,
	0xff0000,
	0xffffff,
};

static wave_t easy_wave = {
};

static u64 ui_shown_ts = 0;
#define META_SYSTEM 3
static void pattern_sine(u8 *buffer, u64 t, float *dmx_channels, bool hardcore_mode) {
	static u64 prev_t = 0;
	static wave_t waves[6] = {{
		.zoom = {
			.x = 89.,
			.y = 0.,
		},
		.spread = 165.,
		.freq = 90.
	}};
	static int n_waves = sizeof(waves) / sizeof(*waves);
	bool dmx_chan_changed[DMX_NUM_CHANNELS];
	static bool const all_dmx_chan_changed[] = {true, true, true, true, true, true};
	u32 color;
	static wave_t *current_wave_edit = &waves[0];
	static wave_t *wave;
	static bool changing_current_wave = false;

	did_dmx_channels_change(dmx_chan_changed, dmx_channels);
	for (int i = 0; i < DMX_NUM_CHANNELS; i++) {
		if (dmx_chan_changed[i] && i != 1)
			changing_current_wave = false;
	}
	
	uint meta_state = dmx_channels[1] / (DMX_MAX_VALUE / 4);

	current_wave_edit->invert_color = false;
	if (meta_state == META_SYSTEM) {
		if (dmx_chan_changed[1]) {
			u8 current_wave_edit_index = (int)(dmx_channels[2] / DMX_MAX_VALUE * 6);
			current_wave_edit = &waves[current_wave_edit_index];
			changing_current_wave = true;
		}
		if (changing_current_wave)
			current_wave_edit->invert_color = (time_us_64() / 500000) % 2;
	}
	pattern_sine_update_current_wave_params(
		(hardcore_mode) ? current_wave_edit : &easy_wave,
		dmx_channels,
		(hardcore_mode) ? dmx_chan_changed : all_dmx_chan_changed,
		hardcore_mode,
		meta_state
	);

	blank_screen(buffer);
	if (hardcore_mode) {
		for (int wave_i = 0; wave_i < n_waves; wave_i++) {
			wave = &waves[wave_i];
			wave->cursor += (((float)(t - prev_t)) / 10000) * faster_edge(wave->freq / 256.);
			pattern_sine_draw_wave(buffer, wave);
		}
	}
	else {
		wave = &easy_wave;
		wave->cursor += (((float)(t - prev_t)) / 10000) * faster_edge(wave->freq / 256.);
		pattern_sine_draw_wave(buffer, wave);	
	}

	// draw UI
	if (hardcore_mode) {
		if (dmx_chan_changed[0]) {
			ui_shown_ts = time_us_64();
		}
#define UI_SHOWN_TIMEOUT 1500000
		if (ui_shown_ts + UI_SHOWN_TIMEOUT > time_us_64()) {
			color = meta_colors[meta_state];
			for (uint j = 0; j < PIXELS_Y; j++) {
				put_pixel_xy(buffer, 0, j, color);
				put_pixel_xy(buffer, PIXELS_X - 1, j, color);
			}
			for (uint i = 1; i < PIXELS_X - 1; i++) {
				put_pixel_xy(buffer, i, 0, color);
				put_pixel_xy(buffer, i, PIXELS_Y - 1, color);
			}
		}
	}

	prev_t = t;
}

static void pattern_sine_easy(u8 *buffer, u64 t, float *dmx_channels) {
	pattern_sine(buffer, t, dmx_channels, false);
}

static void pattern_sine_hard(u8 *buffer, u64 t, float *dmx_channels) {
	pattern_sine(buffer, t, dmx_channels, true);
}

const struct {
	pattern pat;
	const char *name;
} pattern_table[] = {
		{pattern_blank,  "Blank"},
//        {pattern_solid,  "White"},
        {pattern_random,  "Random data"},
//		{pattern_demo, "demo"},
        {pattern_sine_hard, "Sine hardcore mon gars"},
        {pattern_sine_easy, "Sine ez"},
		{pattern_solid_breathe, "Solid Breathe"},
};

void draw(uint8_t *buffer, volatile u8 *dmx_buffer, float *dmx_buffer_smooth) {
	u64 draw_start;

	uint pattern_index;
	draw_start = time_us_64();
	pattern_index = dmx_buffer[6] / (DMX_MAX_VALUE / NUM_PATTERNS);

	printf("pattern index: %d\ndmx buffer: %d\n", pattern_index, dmx_buffer[6]);
	printf("pattern: %s\n", pattern_table[pattern_index].name);
	pattern_table[pattern_index].pat(buffer, time_us_64(), dmx_buffer_smooth);
	printf("draw time %d us\n", (u32)(time_us_64() - draw_start));
}
