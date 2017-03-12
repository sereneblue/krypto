#include <string.h>
#include <stdio.h>
#include <3ds.h>
#include <sf2d.h>
#include <sfil.h>
#include <sftd.h>
#include "api.h"
#include "raleway_regular_ttf.h"
#include "raleway_semibold_ttf.h"

short active_currency = 0;
short active_fiat = 0;

const u32 BG_COLOR = RGBA8(31,31,31,255);
const u32 WHITE = RGBA8(255, 255, 255, 255);
const u32 GRAY_1 = RGBA8(64,64,64,255);
const u32 GRAY_2 = RGBA8(158, 158, 158, 255);
const u32 GRAY_3 = RGBA8(140,140,140,255);
const u32 GRAY_4 = RGBA8(92,92,92,255);

char * fmt_num(double i) {
	char *display_num;
	if (i > 1000000000) {
		asprintf(&display_num, "%.1fB", i / 1000000000.0);
	} else if (i > 1000000) {
		asprintf(&display_num, "%.1fM", i / 1000000.0);
	} else if (i > 1000) {
		asprintf(&display_num, "%.1fK", i / 1000.0);
	} else {
		asprintf(&display_num, "%.1f", 0.0);
	}
	return display_num;
}

void splash_screen() {
	sftd_font *raleway_semibold = sftd_load_font_mem(raleway_semibold_ttf, raleway_semibold_ttf_size);

	int logo_width = sftd_get_text_width(raleway_semibold, 60, "krypto");
	short x = 255;
	while (x > 0) {
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
			sf2d_draw_rectangle(0, 0, 400, 240, BG_COLOR);
			sftd_draw_text(raleway_semibold, (400 - logo_width) / 2, 80, RGBA8(255, 255, 255, x), 60, "krypto");
		sf2d_end_frame();

		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
			sf2d_draw_rectangle(0, 0, 400, 240, BG_COLOR);
		sf2d_end_frame();

		x--;
		sf2d_swapbuffers();
		svcSleepThread(200000);
	}

	sftd_free_font(raleway_semibold);
}

void display() {
	char *hi_lo;
	char *change_str;
	char *price_str;
	char *header;

	sftd_font *raleway = sftd_load_font_mem(raleway_regular_ttf, raleway_regular_ttf_size);
	sftd_font *raleway_semibold = sftd_load_font_mem(raleway_semibold_ttf, raleway_semibold_ttf_size);

	asprintf(&hi_lo, "High %.2f | %.2f Low", lst[active_currency].currency[active_fiat].high, lst[active_currency].currency[active_fiat].low);
	asprintf(&price_str, "%.2f", lst[active_currency].currency[active_fiat].price);
	asprintf(&header, "%s (%s)", lst[active_currency].full, lst[active_currency].sym);

	int logo_width = sftd_get_text_width(raleway_semibold, 30, "krypto");
	int hi_lo_width = sftd_get_text_width(raleway_semibold, 20, hi_lo);
	int currency_width = sftd_get_text_width(raleway_semibold, 20, lst[0].currency[active_fiat].symbol);
	int price_width = sftd_get_text_width(raleway_semibold, 80, price_str);

	asprintf(&change_str, "%+.2f%% (%+.2f)", lst[active_currency].currency[active_fiat].change_price, lst[active_currency].currency[active_fiat].change_amt);
	int change_price_str = sftd_get_text_width(raleway_semibold, 30, change_str);

	sf2d_start_frame(GFX_TOP, GFX_LEFT);
		sf2d_draw_rectangle(0, 0, 400, 240, BG_COLOR);
		sf2d_draw_rectangle(10, 200, 380, 30, GRAY_1);
		sftd_draw_text(raleway_semibold, (400 - logo_width) / 2, 10, WHITE, 30, "krypto");
		sftd_draw_text(raleway, (400 - currency_width - price_width) /2, 60, WHITE, 80, price_str);
		sftd_draw_text(raleway, price_width + (350 - price_width) /2, 80, WHITE, 20, lst[0].currency[active_fiat].symbol);
		sftd_draw_text(raleway, (400 - change_price_str) / 2, 150, WHITE, 30, change_str);
		sftd_draw_text(raleway_semibold, (400 - hi_lo_width) / 2, 203, GRAY_2, 20, hi_lo);
	sf2d_end_frame();

	int crypto_width = sftd_get_text_width(raleway_semibold, 24, header);

	sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		sf2d_draw_rectangle(0, 0, 320, 240, BG_COLOR);
		sftd_draw_text(raleway_semibold, (320 - crypto_width) / 2, 5, WHITE, 24, header);
		sftd_draw_textf(raleway, 10, 50, WHITE, 22, "Market Cap: %s %s", fmt_num(lst[active_currency].currency[active_fiat].market_cap), lst[0].currency[active_fiat].symbol);
		sftd_draw_textf(raleway, 10, 72, WHITE, 22, "Circulating: %s", fmt_num(lst[active_currency].circulating));
		sftd_draw_textf(raleway, 10, 95, WHITE, 22, "24h Volume: %s %s", fmt_num(lst[active_currency].currency[active_fiat].volume), lst[0].currency[active_fiat].symbol);
		sf2d_draw_rectangle(10, 160, 300, 28, GRAY_3);
		sf2d_draw_rectangle(10 + (60 * active_currency), 160, 60, 28, GRAY_1);
		sf2d_draw_rectangle(10, 188, 300, 2, GRAY_4);
		for (int i = 0; i < 5; i++) {
			sftd_font *font = raleway;
			if (active_currency == i) {
				font = raleway_semibold;
			}
			int width = sftd_get_text_width(font, 14, lst[i].sym);
			sftd_draw_text(font, 10 + (60 * i) + (60 - width) / 2, 165, WHITE, 14, lst[i].sym);
		}

		sf2d_draw_rectangle(10, 200, 300, 28, GRAY_3);
		sf2d_draw_rectangle(10 + (60 * active_fiat), 200, 60, 28, GRAY_1);
		sf2d_draw_rectangle(10, 228, 300, 2, GRAY_4);

		for (int i = 0; i < 5; i++) {
			sftd_font *font = raleway;
			if (active_fiat == i) {
				font = raleway_semibold;
			}
			int width = sftd_get_text_width(font, 14, lst[0].currency[i].symbol);
			sftd_draw_text(font, 10 + (60 * i) + (60 - width) / 2, 205, WHITE, 14, lst[0].currency[i].symbol);
		}
	sf2d_end_frame();

	sftd_free_font(raleway);
	sftd_free_font(raleway_semibold);
}

void touch_screen(int x, int y){
	// change the cryptocurrency
	if (x > 10 && x < 310 && y > 160 && y < 188) {
		active_currency = x / 60;
	}

	// change the currency
	if (x > 10 && x < 310 && y > 200 && y < 228) {
		active_fiat = x / 60;
	}
}