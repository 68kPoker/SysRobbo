
#ifndef TYPE_H
#define TYPE_H

#define DIR 1

typedef enum Bool
{
    NIL,
    T
} Bool;

typedef enum Type
{
    SPACE,
    WALL,
    ROBBO,
    SCREW,
    KEY,
    AMMO,
    BOX,
    BOX_WHEEL,
    DOOR,
    DEBRIS,
    BOMB,
    BOMB_EXPLODE,
    BULLET,
    BEAM,
    BEAM_EXTEND,
    BEAM_SHRINK,
    STREAM,
    FIRE,
    SMOKE,
    CANNON,
    CANNON_ROTATE,
    CANNON_MOVE,
    LASER,
    BLASTER,
    TELEPORT,
    TELEPORT_IN,
    TELEPORT_OUT,
    MAGNET_LEFT,
    MAGNET_RIGHT,
    BAT,
    BAT_SHOOT,
    CREATURE_LEFT,
    CREATURE_RIGHT,
    EYES,
    SURPRISE,
    SURPRISE_SHOW,
    EXTRA_LIFE,
    CAPSULE,    
    TYPES
} Type;

typedef enum Result
{
    BLOCK,
    CONSUME,
    ACCEPT
} Result;

typedef struct Props
{
    Result( *enter )( struct Map *map, struct Block *me, short dir, short as, short frame );
    void( *scan )( struct Map *map, struct Block *me );
    short flags, frames, base;
} Props;

#endif
