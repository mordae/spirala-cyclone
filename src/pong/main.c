#include <pico/stdlib.h>

#include <stdlib.h>

#include <sdk.h>
#include <tft.h>

#define PADDLE_WIDTH 4
#define PADDLE_HEIGHT (int)(TFT_HEIGHT / 3)
#define BALL_WIDTH 5
#define BALL_HEIGHT 5

#define BALL_SPEED 70

#define RED 240
#define YELLOW 242
#define GREEN 244
#define BLUE 250
#define GRAY 8
#define WHITE 15

struct paddle {
	float y;
	int score;
};

struct ball {
	float x, y;
	float dx, dy;
};

static struct paddle paddle1, paddle2;
static struct ball ball;

struct effect;

typedef int16_t (*effect_gen_fn)(struct effect *eff);

struct effect {
	int offset;
	int length;
	int volume;
	int period;
	effect_gen_fn generator;
};

#define MAX_EFFECTS 8
struct effect effects[MAX_EFFECTS];

static int16_t __unused square_wave(struct effect *eff)
{
	if ((eff->offset % eff->period) < (eff->period / 2))
		return eff->volume;
	else
		return -eff->volume;
}

static int16_t __unused noise(struct effect *eff)
{
	return rand() % (2 * eff->volume) - eff->volume;
}

void game_start(void)
{
	sdk_set_output_gain_db(6);
}

void game_audio(int nsamples)
{
	for (int s = 0; s < nsamples; s++) {
		int sample = 0;

		for (int i = 0; i < MAX_EFFECTS; i++) {
			struct effect *e = effects + i;

			if (!e->volume)
				continue;

			sample += e->generator(e);

			if (e->offset++ >= e->length)
				e->volume = 0;
		}

		sdk_write_sample(sample);
	}
}

static void __unused play_effect(int volume, int frequency, int length, effect_gen_fn gen)
{
	for (int i = 0; i < MAX_EFFECTS; i++) {
		struct effect *e = effects + i;

		if (e->volume)
			continue;

		e->offset = 0;
		e->volume = volume;
		e->length = length;
		e->period = frequency ? 48000 / frequency : 1;
		e->generator = gen;

		break;
	}
}

void game_reset(void)
{
	paddle1.y = PADDLE_HEIGHT * 2;
	paddle1.score = 0;

	paddle2.y = PADDLE_HEIGHT * 2;
	paddle2.score = 0;

	ball.x = TFT_WIDTH / 2;
	ball.y = TFT_HEIGHT / 2;

	ball.dx = BALL_SPEED;
	ball.dy = BALL_SPEED;
}

void game_input(unsigned dt_usec)
{
	float dt = dt_usec / 1000000.0f;

	/* Joys have value from -2048 to +2047. */
	if (sdk_inputs.joy_y > 500)
		paddle1.y += 1;
	else if (sdk_inputs.joy_y < -500)
		paddle1.y -= 1;

	if (paddle1.y < 0)
		paddle1.y = 0;
	else if (paddle1.y > TFT_BOTTOM - PADDLE_HEIGHT)
		paddle1.y = TFT_BOTTOM - PADDLE_HEIGHT;

	if (sdk_inputs.a)
		paddle2.y += 1;
	else if (sdk_inputs.y)
		paddle2.y -= 1;

	if (paddle2.y < 0)
		paddle2.y = 0;
	else if (paddle2.y > TFT_BOTTOM - PADDLE_HEIGHT)
		paddle2.y = TFT_BOTTOM - PADDLE_HEIGHT;

	/* ball is moving */
	ball.x += ball.dx * dt;
	ball.y += ball.dy * dt;

	/* ball can bounce from top and bottom */
	if (ball.y < 0)
		ball.dy *= -1;
	else if (ball.y > TFT_BOTTOM - BALL_HEIGHT)
		ball.dy *= -1;

	if (ball.x < 0 + PADDLE_WIDTH)
		ball.dx *= -1;
	else if (ball.x + BALL_WIDTH > TFT_RIGHT - PADDLE_WIDTH)
		ball.dx *= -1;
}

void game_paint(unsigned __unused dt_usec)
{
	tft_fill(0);

	/* draw paddles */
	tft_draw_rect(0, paddle1.y, PADDLE_WIDTH - 1, paddle1.y + PADDLE_HEIGHT, WHITE);
	tft_draw_rect(TFT_RIGHT, paddle2.y, TFT_RIGHT - PADDLE_WIDTH + 1, paddle2.y + PADDLE_HEIGHT,
		      WHITE);

	/* draw ball */
	tft_draw_rect(ball.x, ball.y, ball.x + BALL_WIDTH, ball.y + BALL_HEIGHT, WHITE); 
}

int main()
{
	struct sdk_config config = {
		.wait_for_usb = true,
		.show_fps = true,
		.off_on_select = true,
		.fps_color = GRAY,
	};

	sdk_main(&config);
}
