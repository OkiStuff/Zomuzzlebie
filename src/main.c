#include <Muzzle.h>
#include <stdio.h>
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

void applet_dispatch(mz_applet* applet)
{
    while (mz_keep_applet(applet))
    {
        mz_begin_drawing(applet);
            mz_clear_screen(TINT_BLACK);
        mz_end_drawing(applet);
    }
}

int main(void)
{
    mz_applet applet = mz_initialize_applet("Muzombie", SCREEN_WIDTH, SCREEN_HEIGHT, APPLET_FLAG_RESIZBALE | APPLET_FLAG_VSYNC | APPLET_FLAG_TRACK_DELTA_TIME);
    mz_start_applet(&applet, applet_dispatch);

    mz_terminate_applet(&applet);
    return 0;
}
