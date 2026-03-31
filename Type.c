
#include <stdio.h>
#include <stdlib.h>

#include "Map.h"

void  Delay(time) {}

#include "debug.h"

extern Props props[ TYPES ];

static char *name[ TYPES ] =
{
	"Space", "Wall", "Robbo", "Screw", "Key", "Ammo", "Box", "Box Wheel", "Door", "Debris", "Bomb", "Bomb Explode", "Bullet",
	"Beam", "Beam Extend", "Beam Shrink",  "Stream", "Fire", "Smoke", "Cannon", "Cannon Rotate", "Cannon Move", "Laser",
	"Blaster", "Teleport", "Teleport In", "Teleport Out", "Magnet Left", "Magnet Right", "Bat", "Bat Shoot", "Creature Left", "Creature Right",
	"Eyes", "Surprise", "Surprise Show", "Extra Life", "Capsule"
};

void clearMap( Map *map )
{
	short x, y;

	for( y = 0; y < MAP_HEIGHT; y++ )
	{
		Block *row = map->blocks[ y ];

		for( x = 0; x < MAP_WIDTH; x++ )
		{
			row[ x ].type = x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1 ? WALL : SPACE;
		}
	}

	map->robbo = map->blocks[ 1 ] + 1;

	map->robbo->type = ROBBO;
}

static short getClockWise( short dir )
{
	if( dir == LEFT )
	{
		dir = UP;
	}
	else if( dir == UP )
	{
		dir = RIGHT;
	}
	else if( dir == RIGHT )
	{
		dir = DOWN;
	}
	else
	{
		dir = LEFT;
	}
	return( dir );
}

static short getAntiClockWise( short dir )
{
	if( dir == LEFT )
	{
		dir = DOWN;
	}
	else if( dir == DOWN )
	{
		dir = RIGHT;
	}
	else if( dir == RIGHT )
	{
		dir = UP;
	}
	else
	{
		dir = LEFT;
	}
	return( dir );
}

static Result enter( Map *map, Block *me, short dir, short as, short frame )
{
	Type type = me->type;

	if( props[ type ].enter )
	{
		return( props[ type ].enter( map, me, dir, as, frame ) );
	}
	return( BLOCK );
}

static void scan( Map *map, Block *me )
{
	if( me->scanned == NIL )
	{
		Type type = me->type;

		if( props[ type ].scan )
		{
			map->cur = me;
			props[ type ].scan( map, me );
		}
	}
	else
	{
		me->scanned = NIL;
	}
}

void scanMap( Map *map )
{
	Block *block;
	short x, y;

	D( bug( "Scan:\n" ) );

	for( y = 0; y < MAP_HEIGHT; y++ )
	{
		block = map->blocks[ y ];

		for( x = 0; x < MAP_WIDTH; x++ )
		{						
			scan( map, block + x );
		}
	}
}

static char *dirName( short dir )
{
	if( dir == 0 )
	{
		return( "" );
	}
	if( dir == LEFT )
	{
		return( "left " );
	}
	if( dir == RIGHT )
	{
		return( "right " );
	}
	if( dir == UP )
	{
		return( "up " );
	}
	if( dir == DOWN )
	{
		return( "down " );
	}
	return( "other " );
}

static void set( Map *map, Block *me, short dir, short as, short frame )
{
	Bool move = dir && as == me[ -dir ].type;

	D( bug( "Set %16s (%16s) %d %6sat %2d/%2d\n", name[ as ], name[ me->type ], frame, dirName( dir ), ( int )( me - map->blocks[ 0 ] ) % DOWN, ( int )( me - map->blocks[ 0 ] ) / DOWN ) );

	me->type = as;
	me->dir = dir;
	
	me->frame = frame;
	me->counter = 0;

	if( me > map->cur )
	{
		/* Mark as scanned */
		me->scanned = T;
	}
}

static void update( Map *map, Block *me, short dir, short frame )
{
	me->dir = dir;
	me->frame = frame;
	D( bug( "Upd %16s %d %6sat %2d/%2d\n", name[ me->type ], frame, dirName( dir ), ( int )( me - map->blocks[ 0 ] ) % DOWN, ( int )( me - map->blocks[ 0 ] ) / DOWN ) );
}

static void scanBullet( Map *map, Block *me )
{
	short dir = me->dir;

	if( enter( map, me + dir, dir, me->type, me->frame ) == ACCEPT )
	{
		set( map, me, 0, SPACE, 0 );
	}
	else
	{
		set( map, me, 0, SMOKE, 0 );
	}
}

static void scanBoxWheel( Map *map, Block *me )
{
	short dir = me->dir;

	if( dir )
	{
		if( enter( map, me + dir, dir, me->type, me->frame ) )
		{
			set( map, me, 0, SPACE, 0 );
		}
		else
		{
			me->dir = 0;
		}
	}
}

static void scanBombExplode( Map *map, Block *me )
{
	short counter = me->counter, c;
	short sib[] =
	{
		UP,
		LEFT,
		RIGHT + DOWN,
		RIGHT,
		RIGHT + UP,
		DOWN,
		LEFT + UP,
		LEFT + DOWN
	};

	me->counter += 4;

	for( c = 0; c < 4; c++ )
	{
		enter( map, me + sib[ counter + c ], 0, FIRE, 0 );
	}

	if( me->counter == 8 )
	{
		set( map, me, 0, FIRE, 0 );
	}
}

static void scanRobbo( Map *map, Block *me )
{
	short dir = map->dir;
	Block *sib;
	short i, type;
	static short dirs[] = { LEFT, UP, RIGHT, DOWN };

	for( i = 0; i < 4; i++ ) 
	{
		type = me[ dirs[ i ] ].type;
		if( isHostile( type ) )
		{
			set( map, me, 0, FIRE, 0 );
			return;
		}
	}

	for( sib = me + LEFT; sib->type == SPACE; sib += LEFT )
		;

	if( sib->type == MAGNET_LEFT )
	{
		if( sib - me == LEFT )
		{
			set( map, me, 0, FIRE, 0 );
			return;
		}
		map->fire = NIL;
		dir = LEFT;
	}

	for( sib = me + RIGHT; sib->type == SPACE; sib += RIGHT )
		;

	if( sib->type == MAGNET_RIGHT )
	{
		if( sib - me == RIGHT )
		{
			set( map, me, 0, FIRE, 0 );
			return;
		}
		map->fire = NIL;
		dir = RIGHT;
	}

	if( dir )
	{
		if( map->fire == NIL )
		{
			if( enter( map, me + dir, dir, me->type, me->frame ) == ACCEPT )
			{
				map->robbo = me + dir;
				set( map, me, 0, SPACE, 0 );
			}
		}
		else if( map->ammo > 0 )
		{
			if( enter( map, me + dir, dir, BULLET, 0 ) != BLOCK )
			{
				map->ammo--;				
			}
			map->fire = NIL;
		}
	}
}

static void scanFire( Map *map, Block *me )
{
	Props *prop = props + me->type;

	if( me->frame < prop->frames - 1 )
	{
		update( map, me, 0, me->frame + 1 );
	}
	else
	{
		set( map, me, 0, SPACE, 0 );
	}
}

static void scanBeamExtend( Map *map, Block *me )
{
	short dir = me->dir;

	if( enter( map, me + dir, dir, me->type, me->frame ) == ACCEPT )
	{
		set( map, me, dir, BEAM, me->frame );
	}
	else
	{
		set( map, me, dir, BEAM_SHRINK, me->frame );
	}
}

static void scanBeamShrink( Map *map, Block *me )
{
	short dir = me->dir;

	if( me[ -dir ].type == BEAM && me[ -dir ].dir == dir )
	{
		set( map, me - dir, dir, BEAM_SHRINK, me->frame );
		set( map, me, 0, SPACE, 0 );
	}
	else
	{
		set( map, me, 0, SMOKE, 0 );
	}
}

static void scanStream( Map *map, Block *me )
{
	short dir = me->dir;

	enter( map, me + dir, dir, STREAM, me->frame );
}

static void scanCannon( Map *map, Block *me )
{
	short dir = me->dir;
	short rnd = rand() & 0xFF;
	const short prob = 0x0F;

	if( rnd < prob )
	{
		enter( map, me + dir, dir, BULLET, 0 );
	}
}

static void scanCannonMove( Map *map, Block *me )
{
	short dir = me->dir;
	short rnd = rand() & 0xFF;
	short prob = 0x0F;

	if( rnd < prob )
	{
		enter( map, me + UP, UP, BULLET, 0 );
	}

	if( enter( map, me + dir, dir, me->type, me->frame ) == ACCEPT )
	{
		set( map, me, 0, SPACE, 0 );
	}
	else
	{
		update( map, me, -dir, me->frame );
	}
}

static void scanLaser( Map *map, Block *me )
{
	short dir = me->dir;
	short rnd = rand() & 0xFF;
	short prob = 0x0F;

	if( rnd < prob )
	{
		enter( map, me + dir, dir, BEAM_EXTEND, 0 );
	}
}

static void scanBlaster( Map *map, Block *me )
{
	short dir = me->dir;
	short rnd = rand() & 0xFF;
	short prob = 0x0F;
	const short max = 6;

	if( me->counter == 0 && rnd < prob )
	{
		me->counter = 1;
	}

	if( me->counter > 0 )
	{
		enter( map, me + dir, dir, STREAM, me->counter - 1 );
		if( ++me->counter == max )
		{
			me->counter = 0;
		}
	}
}

static void scanCannonRotate( Map *map, Block *me )
{
	short dir = me->dir;
	short rnd = rand() & 0xFF;
	const short prob = 0x0F, rotateLeft = 0x1F, rotateRight = 0x2F;

	if( rnd < prob )
	{
		enter( map, me + dir, dir, BULLET, 0 );
	}
	else if( rnd < rotateLeft )
	{
		dir = getAntiClockWise( dir );
		update( map, me, dir, me->frame );
	}
	else if( rnd < rotateRight )
	{
		dir = getClockWise( dir );
		update( map, me, dir, me->frame );
	}
}

static void scanTeleportOut( Map *map, Block *me )
{
	Props *prop = props + me->type;

	if( me->frame < prop->frames - 1 )
	{
		update( map, me, me->dir, me->frame + 1 );
	}
	else
	{
		set( map, me, me->dir, ROBBO, 0 );
	}
}

static void scanBat( Map *map, Block *me )
{
	short dir = me->dir;

	if( enter( map, me + dir, dir, me->type, me->frame ) == ACCEPT )
	{
		set( map, me, 0, SPACE, 0 );
	}
	else
	{
		update( map, me, -dir, me->frame );
	}
}

static void scanBatShoot( Map *map, Block *me )
{
	short rnd = rand() & 0xFF;
	short dir = me->dir;
	const short prob = 0x0F;

	if( rnd < prob )
	{
		enter( map, me + DOWN, DOWN, BULLET, 0 );
	}

	if( enter( map, me + dir, dir, me->type, me->frame ) == ACCEPT )
	{
		set( map, me, 0, SPACE, 0 );
	}
	else
	{
		update( map, me, -dir, me->frame );
	}
}

static void scanCreatureLeft( Map *map, Block *me )
{ 
	short dir = me->dir;
	short dest = getAntiClockWise( dir );

	if( enter( map, me + dest, dest, me->type, me->frame ) == ACCEPT )
	{
		set( map, me, 0, SPACE, 0 );
		return;
	}
	
	if( enter( map, me + dir, dir, me->type, me->frame ) == ACCEPT )
	{
		set( map, me, 0, SPACE, 0 );
		return;
	}	

	update( map, me, getClockWise( dir ), me->frame );
}

static void scanCreatureRight( Map *map, Block *me )
{
	short dir = me->dir;
	short dest = getClockWise( dir );

	if( enter( map, me + dest, dest, me->type, me->frame ) == ACCEPT )
	{
		set( map, me, 0, SPACE, 0 );
		return;
	}

	if( enter( map, me + dir, dir, me->type, me->frame ) == ACCEPT )
	{
		set( map, me, 0, SPACE, 0 );
	}

	update( map, me, getAntiClockWise( dir ), me->frame );
}

static void scanEyes( Map *map, Block *me )
{
	short diff = ( short )( map->robbo - me );
	short dx, dy;
	short rnd = rand() & 0xFF;
	short dir;

	if( diff > 0 )
	{
		dx = SIGN( diff % DOWN );
		dy = SIGN( diff / DOWN );		
	}
	else
	{
		diff = -diff;
		dx = -SIGN( diff % DOWN );
		dy = -SIGN( diff / DOWN );
	}	

	if( rnd < 0x80 || dy == 0 )
	{
		dir = dx;
	}
	else
	{
		dir = dy * DOWN;
	}
	
	if( enter( map, me + dir, dir, me->type, me->frame ) )
	{
		set( map, me, 0, SPACE, 0 );
	}
}

static Result enterSpace( Map *map, Block *me, short dir, short as, short frame )
{
	set( map, me, dir, as, frame );
	return( ACCEPT );
}

static Result enterWall( Map *map, Block *me, short dir, short as, short frame )
{
	return( BLOCK );
}

static Result enterScrew( Map *map, Block *me, short dir, short as, short frame )
{
	if( as == ROBBO )
	{
		map->screws.collected++;
		set( map, me, dir, as, frame );
		return( ACCEPT );
	}
	return( BLOCK );
}

static Result enterKey( Map *map, Block *me, short dir, short as, short frame )
{
	if( as == ROBBO )
	{
		map->keys++;
		set( map, me, dir, as, frame );
		return( ACCEPT );
	}
	return( BLOCK );
}

static Result enterExtraLife( Map *map, Block *me, short dir, short as, short frame )
{
	if( as == ROBBO )
	{
		map->lives++;
		set( map, me, dir, as, frame );
		return( ACCEPT );
	}
	return( BLOCK );
}

static Result enterCapsule( Map *map, Block *me, short dir, short as, short frame )
{
	if( as == ROBBO )
	{
		if( map->screws.collected >= map->screws.required )
		{
			map->done = T;
		}
	}
	return( BLOCK );
}

static Result enterDoor( Map *map, Block *me, short dir, short as, short frame )
{
	if( as == ROBBO )
	{
		if( map->keys > 0 )
		{
			map->keys--;
			set( map, me, 0, SPACE, 0 );
		}
	}
	return( BLOCK );
}

static Result enterAmmo( Map *map, Block *me, short dir, short as, short frame )
{
	if( as == ROBBO )
	{
		map->ammo += AMMO_PACK;
		set( map, me, dir, as, frame );
		return( ACCEPT );
	}
	return( BLOCK );
}

static Result enterBox( Map *map, Block *me, short dir, short as, short frame )
{
	if( as == ROBBO )
	{
		if( enter( map, me + dir, dir, me->type, me->frame ) == ACCEPT )
		{
			set( map, me, dir, as, frame );
			return( ACCEPT );
		}
	}
	return( BLOCK );
}

static Result enterRobbo( Map *map, Block *me, short dir, short as, short frame )
{
	if( isExplosive( as ) || isHostile( as ) || as == STREAM )
	{
		set( map, me, 0, FIRE, 0 );
	}
	return( BLOCK );
}

static Result enterDebris( Map *map, Block *me, short dir, short as, short frame )
{
	if( isExplosive( as ) )
	{
		set( map, me, 0, FIRE, 0 );
		return( CONSUME );
	}
	else if( as == STREAM )
	{
		set( map, me, dir, as, frame );
		return( ACCEPT );
	}
	return( BLOCK );
}

static Result enterBomb( Map *map, Block *me, short dir, short as, short frame )
{
	if( as == ROBBO )
	{
		if( enter( map, me + dir, dir, me->type, me->frame ) == ACCEPT )
		{
			set( map, me, dir, as, frame );
			return( ACCEPT );
		}
	}
	else if( isExplosive( as ) || as == STREAM )
	{
		set( map, me, 0, BOMB_EXPLODE, 0 );
		return( CONSUME );
	}
	return( BLOCK );
}

static Result enterStream( Map *map, Block *me, short dir, short as, short frame )
{
	if( as == STREAM )
	{
		enter( map, me + me->dir, me->dir, me->type, me->frame );
		set( map, me, dir, as, frame );
		return( ACCEPT );
	}
	return( BLOCK );
}

static Result enterTeleport( Map *map, Block *me, short dir, short as, short frame )
{
	Block *tele = me++,  *last = map->blocks[ MAP_HEIGHT - 2 ] + MAP_WIDTH - 2;
	short counter = me->counter;
	short orig = dir, i;

	while( me++ <= last )
	{
		if( me->type == TELEPORT && me->counter == counter )
		{
			for( i = 0; i < 4; i++ )
			{
				if( enter( map, me + dir, dir, TELEPORT_OUT, frame ) == ACCEPT )
				{
					set( map, tele - orig, 0, TELEPORT_IN, 0 );
					return( BLOCK );
				}
				dir = getClockWise( dir );
			}
		}
	}

	me = map->blocks[ 1 ] + 1;

	while( me++ <= tele )
	{
		if( me->type == TELEPORT && me->counter == counter )
		{
			for( i = 0; i < 4; i++ )
			{
				if( enter( map, me + dir, dir, TELEPORT_OUT, frame ) == ACCEPT )
				{
					set( map, tele - orig, 0, TELEPORT_IN, 0 );
					return( BLOCK );
				}
				dir = getClockWise( dir );
			}
		}
	}
	return( BLOCK );
}

static Result enterSurprise( Map *map, Block *me, short dir, short as, short frame )
{
	if( as == ROBBO )
	{
		if( enter( map, me + dir, dir, me->type, me->frame ) == ACCEPT )
		{
			set( map, me, dir, as, frame );
			return( ACCEPT );
		}
	}
	else if( isExplosive( as ) || as == STREAM )
	{
		set( map, me, 0, SURPRISE_SHOW, 0 );
		return( CONSUME );
	}
	return( BLOCK );
}

void scanSurpriseShow( Map *map, Block *me )
{
	Props *prop = props + me->type;

	if( me->frame < prop->frames - 1 )
	{
		update( map, me, 0, me->frame + 1 );
	}
	else
	{
		short rnd = rand() & 0xFF;
		short type, dir = 0;

		if( rnd < 0x20 )
		{
			type = SCREW;
		}
		else if( rnd < 0x40 )
		{
			type = AMMO;
		}
		else if( rnd < 0x60 )
		{
			type = KEY;
		}
		else if( rnd < 0x80 )
		{
			type = EXTRA_LIFE;
		}
		else if( rnd < 0xA0 )
		{
			type = DEBRIS;
		}
		else if( rnd < 0xC0 )
		{
			type = BOX;
		}
		else if( rnd < 0xE0 )
		{
			type = CANNON_ROTATE;
			dir = RIGHT;
		}
		else
		{
			type = CAPSULE;
			map->screws.collected = map->screws.required;
		}

		set( map, me, dir, type, 0 );
	}
}

void sumBase( void )
{
	short i, sum = 0, add;
	Props *prop;

	for( i = 0; i < TYPES; i++ )
	{
		prop = props + i;

		add = prop->frames;

		if( prop->flags & DIR )
		{
			add *= 4;
		}

		prop->base = sum;
		D( bug( "%16s: %4d %4d %4s\n", name[ i ], sum, prop->frames, prop->flags & DIR ? "dir" : "" ) );
		sum += add;
	}
}

short get( Map *map, Block *me )
{
	short type = me->type;	
	Props *prop = props + type;
	short frame = 0;

	if( prop->flags & DIR )
	{
		short dir = me->dir, i;
		
		if( dir == LEFT )
		{
			i = 0;
		}
		else if( dir == RIGHT )
		{
			i = 1;
		}
		else if( dir == UP )
		{
			i = 2;
		}
		else /* Down */
		{
			i = 3;
		}

		frame += i * prop->frames;
	}

	frame += me->frame;

	return( frame );
}

static Props props[ TYPES ] =
{
	{ enterSpace, NIL, 0, 1 },
	{ enterWall, NIL, 0, 1 },
	{ enterRobbo, scanRobbo, DIR, 4 },
	{ enterScrew, NIL, 0, 1 },
	{ enterKey, NIL, 0, 1 },
	{ enterAmmo, NIL, 0, 1 },
	{ enterBox, NIL, 0, 1 },
	{ enterBox, scanBoxWheel, 0, 1 },
	{ enterDoor, NIL, 0, 1 },
	{ enterDebris, NIL, 0, 1 },
	{ enterBomb, NIL, 0, 1 },
	{ enterWall, scanBombExplode, 0, 1 },
	{ enterWall, scanBullet, DIR, 2 },
	{ enterWall, NIL, DIR, 2 },
	{ enterWall, scanBeamExtend, DIR, 2 },
	{ enterWall, scanBeamShrink, DIR, 2 },
	{ enterStream, scanStream, DIR, 4 },
	{ enterWall, scanFire, 0, 4 },
	{ enterWall, scanFire, 0, 3 },
	{ enterWall, scanCannon, DIR, 1 },
	{ enterWall, scanCannonRotate, DIR, 1 },
	{ enterWall, scanCannonMove, 0, 1 },
	{ enterWall, scanLaser, DIR, 1 },
	{ enterWall, scanBlaster, DIR, 1 },
	{ enterTeleport, NIL, 0, 2 },
	{ enterWall, scanFire, 0, 4 },
	{ enterWall, scanTeleportOut, 0, 4 },
	{ enterWall, NIL, 0, 1 },
	{ enterWall, NIL, 0, 1 },
	{ enterWall, scanBat, 0, 2 },
	{ enterWall, scanBatShoot, 0, 2 },
	{ enterWall, scanCreatureLeft, 0, 2 },
	{ enterWall, scanCreatureRight, 0, 2 },
	{ enterWall, scanEyes, 0, 2 },
	{ enterSurprise, NIL, 0, 1 },
	{ enterWall, scanSurpriseShow, 0, 3 },
	{ enterExtraLife, NIL, 0, 1 },
	{ enterCapsule, NIL, 0, 2 }
};
