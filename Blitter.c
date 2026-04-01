
#include <hardware/custom.h>
#include <hardware/blit.h>

#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>

#include "Blitter.h"

#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MAX(a, b) ((a) >= (b) ? (a) : (b))

#ifdef AMIGA
__far
#endif
extern struct Custom custom;

static void drawIcon( struct BitMap *sbm, WORD sx, WORD sy, struct BitMap *dbm, WORD dx, WORD dy, WORD width, WORD height, UBYTE minterm, UBYTE mask, struct BitMap *mbm, WORD mx, WORD my )
{
    struct Custom *c;    
    WORD bpr;
    LONG sset, dset;
    WORD smod, dmod, mmod;
    WORD wwidth;
    PLANEPTR mplane;
    UBYTE p;

    OwnBlitter();

    mask &= ( 1 << MIN( sbm->Depth, dbm->Depth ) ) - 1;

    wwidth = ( width + 15 ) >> 4;

    bpr = sbm->BytesPerRow;

    sset = bpr * sy + ( ( sx >> 4 ) << 1 );
    smod = bpr - ( wwidth << 1 );

    bpr = dbm->BytesPerRow;

    dset = bpr * dy + ( ( dx >> 4 ) << 1 );
    dmod = bpr - ( wwidth << 1 );

    if( mbm )
    {
        bpr = mbm->BytesPerRow;

        mplane = mbm->Planes[ 0 ] + bpr * my + ( ( mx >> 4 ) << 1 );
        mmod = bpr - ( wwidth << 1 );
    }

    c = &custom;

    for( p = 0; mask; p++, mask >>= 1 )
    {
        if( mask & 1 )
        {
            WaitBlit();
            
            c->bltcon0 = minterm | 0x0a | SRCB | SRCC | DEST | ( mbm ? SRCA : 0 );
            c->bltcon1 = 0;
            c->bltadat = 0xffff;
            if( mbm )
            {
                c->bltapt = mplane;
            }
            c->bltbpt = sbm->Planes[ p ] + sset;
            c->bltcpt = dbm->Planes[ p ] + dset;
            c->bltdpt = dbm->Planes[ p ] + dset;
            c->bltamod = mmod;
            c->bltbmod = smod;
            c->bltcmod = dmod;
            c->bltdmod = dmod;
            c->bltafwm = 0xffff >> ( dx & 0xf );
            c->bltalwm = 0xffff << ( 15 - ( ( dx + width - 1 ) & 0xf ) );
            c->bltsize = ( height << HSIZEBITS ) | wwidth;
        }
    }

    DisownBlitter();
}

void drawIconRastPort( struct BitMap *sbm, WORD sx, WORD sy, struct RastPort *rp, WORD dx, WORD dy, WORD width, WORD height, UBYTE minterm, struct BitMap *mbm, WORD mx, WORD my )
{
    struct BitMap *dbm = rp->BitMap;
    struct Layer *l = rp->Layer;
    UBYTE mask = rp->Mask;
    struct Rectangle *r;
    struct ClipRect *cr;
    WORD x0, y0, x1, y1, ox, oy;

    if( l == NULL )
    {
        drawIcon( sbm, sx, sy, dbm, dx, dy, width, height, minterm, mask, mbm, mx, my );
        return;
    }

    LockLayer( 0, l );

    r = &l->bounds;

    dx += r->MinX;
    dy += r->MinY;

    for( cr = l->ClipRect; cr != NULL; cr = cr->Next )
    {
        if( cr->lobs != NULL )
        {
            continue;
        }

        r = &cr->bounds;

        x0 = MAX( r->MinX, dx );
        y0 = MAX( r->MinY, dy );
        x1 = MIN( r->MaxX, dx + width - 1 );
        y1 = MIN( r->MaxY, dy + height - 1 );

        if( x0 > x1 || y0 > y1 )
        {
            continue;
        }

        ox = x0 - dx;
        oy = y0 - dy;

        drawIcon( sbm, sx + ox, sy + oy, dbm, x0, y0, x1 - x0 + 1, y1 - y0 + 1, minterm, mask, mbm, mx + ox, my + oy );
    }

    UnlockLayer( l );
}
