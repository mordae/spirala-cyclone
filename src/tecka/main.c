#include <pico/stdlib.h>

#include <stdio.h>
#include <stdlib.h>

#include <sdk.h>
#include <tft.h>

#define RED 240
#define YELLOW 242
#define GREEN 244
#define BLUE 250
#define GRAY 8
#define WHITE 15

static int ew;
static int eh;
static int ex;
static int ey;
static int ehp_max;
static int ehp;
static int exa;
static int eya;
static int score;

inline static int clamp(int x, int lo, int hi)
{
	if (x < lo)
		return lo;

	if (x > hi)
		return hi;

	return x;
}

void game_reset(void)
{
	ew = 30;
	eh = 30;
	ex = random() % (TFT_WIDTH - ew);
	ey = random() % (TFT_HEIGHT - eh);
	ehp_max = 59;
	ehp = ehp_max;
	exa = 2;
	eya = 2;
	score = 0;
}

void game_start(void)
{
	sdk_set_output_gain_db(6);
}

void game_audio(int nsamples)
{
	for (int s = 0; s < nsamples; s++) {
		int max = INT16_MAX / 6 * (ehp_max - ehp) / ehp_max;
		int sample = rand() % (2 * max) - max;

		sdk_write_sample(sample);
	}
}

void game_paint(unsigned __unused dt_usec)
{
	tft_fill(0);
	char buf[64];

	snprintf(buf, sizeof buf, "%i", score);
	tft_draw_string(0, 0, RED, buf);

	/* Calculate crosshair position */
	int tx = TFT_WIDTH / 2 * sdk_inputs.joy_x / 1400;
	int ty = TFT_HEIGHT / 2 * sdk_inputs.joy_y / 1400;

	tx += TFT_WIDTH / 2;
	ty += TFT_HEIGHT / 2;

	tx = clamp(tx, 0, TFT_WIDTH - 1);
	ty = clamp(ty, 0, TFT_HEIGHT - 1);

	/* Check for collission */
	if ((tx >= ex) && (tx <= (ex + ew)) && (ty >= ey) && (ty <= (ey + eh))) {
		ehp = MAX(0, ehp - 1);
	} else {
		ehp = MIN(ehp_max, ehp + 1);
	}

	if (!ehp) {
		ex = random() % (TFT_WIDTH - ew);
		ey = random() % (TFT_HEIGHT - eh);
		ehp = ehp_max;
		ew = clamp(ew - 2, 10, 30);
		eh = clamp(eh - 2, 10, 30);
		score++;
	} else {
		ex += exa * (random() % 3 - 1);
		ey += eya * (random() % 3 - 1);

		ex = clamp(ex, 0, TFT_WIDTH - ew);
		ey = clamp(ey, 0, TFT_HEIGHT - eh);
	}

	int ecolor = 249 + 6 - ehp / 10;

	/* draw THE enemy */
	tft_draw_rect(ex, ey, ex + ew, ey + eh, ecolor);

	/* Draw the crosshair */
	tft_draw_rect(tx - 2, ty, tx + 2, ty, GREEN);
	tft_draw_rect(tx, ty - 2, tx, ty + 2, GREEN);
}

int main()
{
	struct sdk_config config = {
		.wait_for_usb = true,
		.show_fps = true,
		.off_on_select = true,
	};

	sdk_main(&config);
}
