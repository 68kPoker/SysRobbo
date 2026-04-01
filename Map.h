
#ifndef MAP_H
#define MAP_H

#include "Type.h"

#define MAP_WIDTH  16
#define MAP_HEIGHT 32

#define AMMO_PACK  8

#define LEFT  -1
#define RIGHT 1
#define UP    -MAP_WIDTH
#define DOWN  MAP_WIDTH

#define SIGN(x) (((x) > 0) - ((x) < 0))

#define DELAY 6

#define isExplosive( as ) ( as ) == BULLET || ( as ) == BEAM_EXTEND || ( as ) == FIRE
#define isHostile( as ) ( as ) == BAT || ( as ) == BAT_SHOOT || ( as ) == CREATURE_LEFT || ( as ) == CREATURE_RIGHT || ( as ) == EYES

typedef struct Block
{
    Type type;
    short dir, counter;
    short frame;
    Bool scanned;
} Block;

typedef struct Map
{
    Block blocks[ MAP_HEIGHT ][ MAP_WIDTH ], *cur, *robbo;
    struct Screws
    {
        short collected, required;
    } screws;
    short keys, ammo, lives;
    short dir;
    Bool fire;
    short delay;
    short done;
} Map;

extern void scanMap( Map *map );
extern void sumBase( void );
extern short get( Map *map, Block *me ); /* Get animation frame */
extern void clearMap( Map *map );

#endif
