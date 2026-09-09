#ifndef MUZOMBIE_PLAYER_H
#define MUZOMBIE_PLAYER_H

#include <Muzzle.h>

typedef struct player
{
	mz_vec2 world_position;
} player;

void update_player(mz_applet* applet, player* player);
void render_player(mz_applet* applet, player* player);

#endif // MUZOMBIE_PLAYER_H
