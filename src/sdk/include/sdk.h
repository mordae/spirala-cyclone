/*
 * Copyright (C) Jan Hamal Dvořák <mordae@anilinux.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once
#include <pico/stdlib.h>

struct sdk_config {
	bool wait_for_usb;
	bool show_fps;
	bool off_on_select;

	float target_fps;

	uint8_t brightness;
};

struct sdk_inputs {
	float batt_mv;

	int a, b, x, y;
	int start, select;

	int joy_x, joy_y, joy_sw;

	int vol_up, vol_down, vol_sw;

	int aux[8];
};

extern struct sdk_inputs sdk_inputs;
extern struct sdk_inputs sdk_inputs_delta;

extern struct sdk_config sdk_config;

void game_reset(void);
void game_input(void);
void game_paint(unsigned dt);

void sdk_main(struct sdk_config *conf);
void sdk_set_screen_brightness(uint8_t level);
void sdk_set_amp_enabled(bool en);
