#include <string.h>
#include <stdio.h>
#include <3ds.h>
#include <sf2d.h>
#include <sfil.h>
#include <sftd.h>
#include "api.h"

#define NUMTHREADS 1
#define STACKSIZE (4 * 1024)

volatile bool runThreads = true;
cryptocurrency lst[5];
bool loaded_splash = false;

void worker_thread();

int main() {
	sf2d_init();
	sf2d_set_clear_color(RGBA8(41,75,86,255));
	sf2d_set_vblank_wait(0);
	httpcInit(0x1000);

	sftd_init();
	initialize_values();

	Thread threads[NUMTHREADS];
	s32 prio = 0;
	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);

	for (int i = 0; i < NUMTHREADS; i ++)
	{
		threads[i] = threadCreate(worker_thread, (void*)((i+1)*250), STACKSIZE, prio-1, -2, false);
	}

	while (aptMainLoop()) {
		if (loaded_splash) {
			display();
		} else {
			splash_screen();
			loaded_splash = true;
		}
		hidScanInput();
		u32 kDown = hidKeysDown();

		touchPosition touch;
		hidTouchRead(&touch);

		touch_screen(touch.px, touch.py);
		if (kDown & KEY_START) break;

		sf2d_swapbuffers();
	}

	runThreads = false;
	for (int i = 0; i < NUMTHREADS; i ++)
	{
		threadJoin(threads[i], U64_MAX);
		threadFree(threads[i]);
	}

	httpcExit();
	sftd_fini();
	sf2d_fini();
	return 0;
}

void worker_thread(void *arg)
{
	short cycles = 30;
	u64 sleepDuration = 1000000000;
	while (runThreads)
	{
		if (cycles == 30) {
			update_values();  //  Updates every 30 seconds, improve responsiveness
			cycles = 0;
		}
		cycles++;
		svcSleepThread(sleepDuration);
	}
}