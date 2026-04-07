
#include <stdlib.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include "iffp/ilbm.h"

#include "Screen.h"
#include "Blitter.h"
#include "Map.h"
#include "Input.h"

struct Library *IntuitionBase, *GfxBase, *LayersBase, *IFFParseBase;

int main( void )
{
    static Map map = { 0 };
    Block *me;
    struct IFFHandle *iff;
    struct Window *w;
    WORD *pens, count;
    struct BitMap *back, *bm;
    struct StoredProperty *sp;
    BitMapHeader *bmhd;
    struct IntuiMessage *imsg;
    struct inputData id;
    struct screenData sd;

    srand( 9511 );

    clearMap( &map );

    sumBase();

#if 0    

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
#endif

    if( IntuitionBase = OpenLibrary( "intuition.library", 39L ) )
    {
        GfxBase = OpenLibrary( "graphics.library", 39L );
        LayersBase = OpenLibrary( "layers.library", 39L );
        if( IFFParseBase = OpenLibrary( "iffparse.library", 39L ) )
        {
            if( !( iff = openILBM( "Back.iff" ) ) )
                printf( "Couldn't open Robbo.iff\n" );
            else
            {
                if( sp = FindProp( iff, ID_ILBM, ID_BMHD ) )
                {
                    bmhd = ( BitMapHeader * )sp->sp_Data;

                    if( w = openScreen( bmhd->nPlanes, iff, &pens, &count ) )
                    {
                        MakeScreen( w->WScreen );
                        RethinkDisplay();
                        if( back = readPicture( iff ) )
                        {
                            closeILBM( iff );
                            if( iff = openILBM( "Tiles.iff" ) )
                            {
                                if( bm = readPicture( iff ) )
                                {
                                    if( remapPicture( bm, w->WScreen->ViewPort.ColorMap, iff, 31 ) )
                                    {
                                        closeILBM( iff );
                                        iff = NULL;
                                        if( sd.buf[ 0 ] = AllocScreenBuffer( w->WScreen, NULL, SB_SCREEN_BITMAP ) )
                                        {
                                            if( sd.buf[ 1 ] = AllocScreenBuffer( w->WScreen, NULL, SB_COPY_BITMAP ) )
                                            {
                                                struct DrawInfo *dri;

                                                if( dri = GetScreenDrawInfo( w->WScreen ) )
                                                {
                                                    if( sd.gad = NewObject( NULL, "propgclass",
                                                        GA_Left, 0,
                                                        GA_Top, 32,
                                                        GA_Width, 16,
                                                        GA_Height, 160,
                                                        GA_ID, 5,
                                                        GA_DrawInfo, dri,
                                                        PGA_Borderless, TRUE,
                                                        PGA_NewLook, FALSE,
                                                        PGA_Top, 0,
                                                        PGA_Visible, 10,
                                                        PGA_Total, MAP_HEIGHT,
                                                        GA_RelVerify, TRUE,
                                                        TAG_DONE ) )
                                                    {
                                                        WORD pos = 0;

                                                        AddGadget( w, sd.gad, -1 );
                                                        RefreshGList( sd.gad, w, NULL, 1 );
                                                        if( openInput( &id, w->WScreen ) )
                                                        {
                                                            BOOL done = FALSE;
                                                            WORD frame = 1;

                                                            w->RPort->BitMap = sd.buf[ frame ]->sb_BitMap;

                                                            drawIconRastPort( back, 0, 0, w->RPort, 0, 0, 320, 256, 0xc0, NULL, 0, 0 );
                                                            updateMap( &map, w->RPort, frame, 0, 0, bm, back );
                                                            /* drawIconRastPort( bm, 0, 0, w->RPort, 16, 32, 240, 160, 0xc0, back, 16, 32 ); */
                                                            drawIcon( sd.buf[ frame ^ 1 ]->sb_BitMap, w->LeftEdge + sd.gad->LeftEdge, w->TopEdge + sd.gad->TopEdge, sd.buf[ frame ]->sb_BitMap, w->LeftEdge + sd.gad->LeftEdge, w->TopEdge + sd.gad->TopEdge, sd.gad->Width, sd.gad->Height, 0xc0, 0xff, NULL, 0, 0 );
                                                            WaitBlit();

                                                            while( !ChangeScreenBuffer( w->WScreen, sd.buf[ frame ] ) )
                                                            {
                                                                WaitTOF();
                                                            }
                                                            frame ^= 1;

                                                            WaitTOF();
                                                            w->RPort->BitMap = sd.buf[ frame ]->sb_BitMap;

                                                            drawIconRastPort( back, 0, 0, w->RPort, 0, 0, 320, 256, 0xc0, NULL, 0, 0 );

                                                            while( !done )
                                                            {
                                                                ULONG result = Wait( ( 1L << w->UserPort->mp_SigBit ) | ( 1L << id.mp->mp_SigBit ) );
                                                                if( result & ( 1L << id.mp->mp_SigBit ) )
                                                                {
                                                                    struct inputMessage *im;

                                                                    while( im = ( struct inputMessage * )GetMsg( id.mp ) )
                                                                    {
                                                                        if( im->gad )
                                                                        {

                                                                        }
                                                                        else
                                                                        {
                                                                        }
                                                                        FreeMem( im, sizeof( *im ) );
                                                                    }
                                                                }
                                                                if( result & ( 1L << w->UserPort->mp_SigBit ) )
                                                                {
                                                                    while( imsg = ( struct IntuiMessage * )GetMsg( w->UserPort ) )
                                                                    {
                                                                        if( imsg->Class == IDCMP_REFRESHWINDOW )
                                                                        {
                                                                            BeginRefresh( w );
                                                                            w->RPort->BitMap = sd.buf[ frame ^ 1 ]->sb_BitMap;
                                                                            drawIconRastPort( back, 0, 0, w->RPort, 0, 0, 320, 256, 0xc0, NULL, 0, 0 );
                                                                            /* updateMap( &map, w->RPort, frame, 0, pos, bm, back ); */
                                                                            EndRefresh( w, TRUE );
                                                                        }
                                                                        else if( imsg->Class == IDCMP_RAWKEY )
                                                                        {
                                                                            if( imsg->Code == 0x45 )
                                                                            {
                                                                                done = TRUE;
                                                                            }
                                                                        }
                                                                        else if( imsg->Class == IDCMP_GADGETUP )
                                                                        {
                                                                            struct Gadget *gad = ( struct Gadget * )imsg->IAddress;

                                                                            if( gad->GadgetID == 5 )
                                                                            {
                                                                                LONG i;

                                                                                GetAttr( PGA_Top, gad, &i );

                                                                                if( i != pos )
                                                                                {
                                                                                    pos = i;
                                                                                    w->RPort->BitMap = sd.buf[ frame ]->sb_BitMap;

                                                                                    updateMap( &map, w->RPort, frame, 0, pos, bm, back );
                                                                                    /* drawIconRastPort( bm, 0, i, w->RPort, 16, 32, 240, 160, 0xc0, back, 16, 32 ); */
                                                                                    drawIcon( sd.buf[ frame ^ 1 ]->sb_BitMap, w->LeftEdge + gad->LeftEdge, w->TopEdge + gad->TopEdge, sd.buf[ frame ]->sb_BitMap, w->LeftEdge + gad->LeftEdge, w->TopEdge + gad->TopEdge, gad->Width, gad->Height, 0xc0, 0xff, NULL, 0, 0 );
                                                                                    WaitBlit();
                                                                                    while( !ChangeScreenBuffer( w->WScreen, sd.buf[ frame ] ) )
                                                                                    {
                                                                                        drawIcon( sd.buf[ frame ^ 1 ]->sb_BitMap, w->LeftEdge + gad->LeftEdge, w->TopEdge + gad->TopEdge, sd.buf[ frame ]->sb_BitMap, w->LeftEdge + gad->LeftEdge, w->TopEdge + gad->TopEdge, gad->Width, gad->Height, 0xc0, 0xff, NULL, 0, 0 );
                                                                                        WaitTOF();
                                                                                        WaitBlit();
                                                                                    }

                                                                                    frame ^= 1;
                                                                                }
                                                                            }
                                                                        }
                                                                        ReplyMsg( ( struct Message * )imsg );
                                                                    }
                                                                }
                                                            }
                                                            closeInput( &id );
                                                        }
                                                        RemoveGadget( w, sd.gad );
                                                        DisposeObject( sd.gad );
                                                    }
                                                    FreeScreenDrawInfo( w->WScreen, dri );
                                                }
                                                FreeScreenBuffer( w->WScreen, sd.buf[ 1 ] );
                                            }
                                            FreeScreenBuffer( w->WScreen, sd.buf[ 0 ] );
                                        }
                                    }
                                    FreeBitMap( bm );
                                }
                            }
                            FreeBitMap( back );
                        }
                        closeScreen( w, pens, count );
                    }
                }
                if( iff )
                {
                    closeILBM( iff );
                }
            }
            CloseLibrary( IFFParseBase );
        }
        CloseLibrary( LayersBase );
        CloseLibrary( GfxBase );
        CloseLibrary( IntuitionBase );
    }
    return( 0 );
}
