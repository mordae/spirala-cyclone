#include "sdk/input.h"
#include "sdk/util.h"
#include <pico/stdlib.h>

#include <stdbool.h>
#include <stdint.h>

#include <sdk.h>
#include <stdlib.h>
#include <tft.h>

#define SPACE_EMPTY 2
#define SPACE_0 4
#define SPACE_1 239
#define SPACE_2 224
#define SPACE_3 226
#define SPACE_4 228
#define SPACE_5 230
#define SPACE_6 234
#define SPACE_7 235
#define SPACE_8 237
#define SPACE_FLAG 191
#define WHITE 15

#define SPACE_SIZE 8
#define SPACE_COUNT_HOR 20
#define SPACE_COUNT_VER 15

static int movement_das = 150000;
static int movement_arr = 33333;
static int cursor_x = 0;
static int cursor_y = 0;
static int time_moving = 0;

static int space_colors[9] = {
	SPACE_0, SPACE_1, SPACE_2, SPACE_3, SPACE_4, SPACE_5, SPACE_6, SPACE_7, SPACE_8,
};

// inverse of the probability that a mine is going to spawn on a given space
static int mine_chance = 10;
static bool initial_state = true;

struct space {
	uint8_t neighbour_mines : 4;
	uint8_t has_mine : 1;
	uint8_t is_explored : 1;
	uint8_t is_flagged : 1;
	uint8_t triggered_neighbouring_cells : 1;
};

static struct space board[SPACE_COUNT_VER][SPACE_COUNT_HOR];

static void draw_space(int x, int y, int c)
{
	tft_draw_rect(SPACE_SIZE * x, SPACE_SIZE * y, SPACE_SIZE * x + SPACE_SIZE - 1,
		      SPACE_SIZE * y + SPACE_SIZE - 1, c);
}

static void draw_cursor()
{
	tft_draw_rect(SPACE_SIZE * cursor_x + 3, SPACE_SIZE * cursor_y + 3,
		      SPACE_SIZE * cursor_x + SPACE_SIZE - 4,
		      SPACE_SIZE * cursor_y + SPACE_SIZE - 4, WHITE);
}

static int get_neighbouring_mines(int x, int y)
{
	int neighbour_count = 0;
	for (int ay = -1; ay <= 1; ay++) {
		for (int ax = -1; ax <= 1; ax++) {
			if (x + ax < 0 || x + ax >= SPACE_COUNT_HOR || y + ay < 0 ||
			    y + ay >= SPACE_COUNT_VER) {
				continue;
			} else {
				neighbour_count += board[y + ay][x + ax].has_mine;
			}
		}
	}
	return neighbour_count;
}

static int should_space_have_mine()
{
	int n = rand() % mine_chance;
	return (n == 0);
}

static void trigger_neighbouring_cells(int x, int y)
{
	for (int ay = -1; ay <= 1; ay++) {
		for (int ax = -1; ax <= 1; ax++) {
			if (x + ax < 0 || x + ax >= SPACE_COUNT_HOR || y + ay < 0 ||
			    y + ay >= SPACE_COUNT_VER) {
				continue;
			} else if (board[y + ay][x + ax].is_flagged == 0) {
				board[y + ay][x + ax].is_explored = 1;
			}
		}
	}
	board[y][x].triggered_neighbouring_cells = 1;
}

static bool victorious = false;

void game_reset(void)
{
	srand(sdk_random());

	victorious = false;
	initial_state = true;

	for (int y = 0; y < SPACE_COUNT_VER; y++) {
		for (int x = 0; x < SPACE_COUNT_HOR; x++) {
			board[y][x].has_mine = should_space_have_mine();
			board[y][x].is_explored = 0;
			board[y][x].is_flagged = 0;
			board[y][x].triggered_neighbouring_cells = 0;
		}
	}

	for (int y = 0; y < SPACE_COUNT_VER; y++) {
		for (int x = 0; x < SPACE_COUNT_HOR; x++) {
			board[y][x].neighbour_mines = get_neighbouring_mines(x, y);
		}
	}
}

static uint16_t notes[256];
static const char tune[] = " cegCEGB";

void game_start(void)
{
	sdk_set_output_gain_db(6);

	notes['c'] = 131;
	notes['d'] = 147;
	notes['e'] = 165;
	notes['f'] = 175;
	notes['g'] = 196;
	notes['a'] = 220;
	notes['b'] = 247;

	notes['C'] = 261;
	notes['D'] = 293;
	notes['E'] = 329;
	notes['F'] = 349;
	notes['G'] = 392;
	notes['A'] = 440;
	notes['B'] = 494;
}

void game_audio(int nsamples)
{
	static int ellapsed = 0;
	static int tone_pos = 0;

	if (!victorious) {
		tone_pos = 0;
		ellapsed = 0;

		for (int s = 0; s < nsamples; s++)
			sdk_write_sample(0);

		return;
	}

	for (int s = 0; s < nsamples; s++) {
		if (ellapsed > SDK_AUDIO_RATE / 6) {
			tone_pos++;
			ellapsed = 0;
		}

		if (!tune[tone_pos]) {
			game_reset();
			return;
		}

		int freq = notes[(unsigned)tune[tone_pos]];

		if (freq) {
			int period = SDK_AUDIO_RATE / freq;
			int half_period = 2 * ellapsed / period;
			int modulo = half_period & 1;

			sdk_write_sample(4000 * (modulo ? 1 : -1));
		} else {
			sdk_write_sample(0);
		}

		ellapsed++;
	}
}

void game_input(unsigned dt_usec)
{
	if (sdk_inputs.joy_x > 1024 || sdk_inputs.joy_x < -1024) {
		if (time_moving == 0) {
			if (sdk_inputs.joy_x > 0) {
				cursor_x = clamp(cursor_x + 1, 0, SPACE_COUNT_HOR - 1);
			} else {
				cursor_x = clamp(cursor_x - 1, 0, SPACE_COUNT_HOR - 1);
			}
			time_moving++;
		} else {
			if (time_moving > movement_das &&
			    time_moving - movement_das > movement_arr) {
				if (sdk_inputs.joy_x > 0) {
					cursor_x = clamp(cursor_x + 1, 0, SPACE_COUNT_HOR - 1);
				} else {
					cursor_x = clamp(cursor_x - 1, 0, SPACE_COUNT_HOR - 1);
				}
				time_moving -= movement_arr;
			}
		}
		time_moving += dt_usec;
	} else if (sdk_inputs.joy_y > 1024 || sdk_inputs.joy_y < -1024) {
		if (time_moving == 0) {
			if (sdk_inputs.joy_y > 0) {
				cursor_y = clamp(cursor_y + 1, 0, SPACE_COUNT_VER - 1);
			} else {
				cursor_y = clamp(cursor_y - 1, 0, SPACE_COUNT_VER - 1);
			}
			time_moving++;
		} else {
			if (time_moving > movement_das &&
			    time_moving - movement_das > movement_arr) {
				if (sdk_inputs.joy_y > 0) {
					cursor_y = clamp(cursor_y + 1, 0, SPACE_COUNT_VER - 1);
				} else {
					cursor_y = clamp(cursor_y - 1, 0, SPACE_COUNT_VER - 1);
				}
				time_moving -= movement_arr;
			}
		}
		time_moving += dt_usec;
	} else {
		time_moving = 0;
	}

	if (sdk_inputs_delta.b > 0) {
		if (board[cursor_y][cursor_x].is_explored == 1) {
			trigger_neighbouring_cells(cursor_x, cursor_y);
		}
		if (board[cursor_y][cursor_x].is_flagged == 0) {
			board[cursor_y][cursor_x].is_explored = 1;
			if (initial_state) {
				if (board[cursor_y][cursor_x].has_mine == 1) {
					board[cursor_y][cursor_x].has_mine = 0;
					board[cursor_y][cursor_x].neighbour_mines =
						get_neighbouring_mines(cursor_x, cursor_y);
				}
				initial_state = false;
			}
		}
	}

	if (sdk_inputs_delta.a > 0) {
		if (board[cursor_y][cursor_x].is_flagged == 0)
			board[cursor_y][cursor_x].is_flagged = 1;
		else
			board[cursor_y][cursor_x].is_flagged = 0;
	}

	if (sdk_inputs_delta.x > 0) {
		game_reset();
	}
}

void game_paint(unsigned __unused dt_usec)
{
	tft_fill(0);

	int cleared_cells = 0;

	for (int y = 0; y < SPACE_COUNT_VER; y++) {
		for (int x = 0; x < SPACE_COUNT_HOR; x++) {
			if (board[y][x].is_explored == 1 && board[y][x].neighbour_mines == 0 &&
			    board[y][x].triggered_neighbouring_cells == 0) {
				trigger_neighbouring_cells(x, y);
			}

			if (board[y][x].has_mine == 1 && board[y][x].is_explored == 1) {
				draw_space(x, y, WHITE);
			}

			if (board[y][x].is_flagged == 1 && board[y][x].is_explored == 0) {
				draw_space(x, y, SPACE_FLAG);
			}

			if (board[y][x].is_flagged == 0 && board[y][x].is_explored == 0) {
				draw_space(x, y, SPACE_EMPTY);
			}

			if (board[y][x].has_mine == 0 && board[y][x].is_explored == 1) {
				draw_space(x, y, space_colors[board[y][x].neighbour_mines]);
			}

			if ((board[y][x].has_mine == 1 && board[y][x].is_explored == 0) ||
			    (board[y][x].has_mine == 0 && board[y][x].is_explored == 1)) {
				cleared_cells++;
			}
		}
	}

	draw_cursor();

	if (cleared_cells >= SPACE_COUNT_HOR * SPACE_COUNT_VER)
		victorious = true;
}

int main()
{
	struct sdk_config config = {
		.wait_for_usb = true,
		.show_fps = true,
		.off_on_select = true,
		.fps_color = WHITE,
	};

	sdk_main(&config);
}
