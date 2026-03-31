
#include <stdlib.h>

#include "Map.h"

int main( void )
{
    Map map = { 0 };
    Block *me;
    srand( 9511 );

    clearMap( &map );

    sumBase();

    me = &map.blocks[ 1 ][ 1 ];

    me->type = ROBBO;
    
    me[ 3 ].type = SURPRISE;
    me[ 4 ].type = DOOR;
    map.blocks[ 2 ][ 8 ].type = EYES;
    
    map.ammo += AMMO_PACK;
    map.fire = T;

    map.dir = RIGHT;

    scanMap( &map );
    scanMap( &map );
    scanMap( &map );
    scanMap( &map );
    scanMap( &map );    
    scanMap( &map );
    scanMap( &map );
    scanMap( &map );
    scanMap( &map );
    scanMap( &map );
    return( 0 );
}
