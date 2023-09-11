#include "hal.h"
#define TIME_INTERVAL 10 // ms

static volatile uint32_t jiffy = 0;
static int fps = 0;
static int nr_draw = 0;

void incr_nr_draw(void)
{
	nr_draw++;
}

int get_fps()
{
	return fps;
}

void timer_event(void)
{
	jiffy++;
	if (jiffy % (HZ / 2) == 0)
	{
		fps = nr_draw * 2 + 1;
		nr_draw = 0;
	}
}

uint32_t SDL_GetTicks()
{
	return jiffy * TIME_INTERVAL;
}

void SDL_Delay(uint32_t ms)
{
	uint32_t start = SDL_GetTicks();
    while (SDL_GetTicks() - start < ms);
}
