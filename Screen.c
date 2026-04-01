
#include <intuition/screens.h>
#include <exec/memory.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/exec_protos.h>

#include <assert.h>

#include "iffp/ilbm.h"

#include "Screen.h"

#define RGB(c) ( (c) | ((c)<<8) | ((c)<<16) | ((c)<<24) )

extern WORD *obtainPens( struct ColorMap *cm, struct IFFHandle *iff, WORD *pcount );
extern void releasePens( struct ColorMap *cm, WORD *pens, WORD count );

struct IFFHandle *openILBM( STRPTR name )
{
    struct IFFHandle *iff;
    LONG err;

    if( iff = AllocIFF() )
    {
        if( iff->iff_Stream = Open( name, MODE_OLDFILE ) )
        {
            InitIFFasDOS( iff );
            if( OpenIFF( iff, IFFF_READ ) == 0 )
            {
                if( PropChunk( iff, ID_ILBM, ID_BMHD ) == 0 )
                {
                    if( PropChunk( iff, ID_ILBM, ID_CMAP ) == 0 )
                    {
                        if( PropChunk( iff, ID_ILBM, ID_CAMG ) == 0 )
                        {
                            if( StopChunk( iff, ID_ILBM, ID_BODY ) == 0 )
                            {
                                if( StopOnExit( iff, ID_ILBM, ID_FORM ) == 0 )
                                {
                                    err = ParseIFF( iff, IFFPARSE_SCAN );
                                    if( err == 0 || err == IFFERR_EOC || err == IFFERR_EOF )
                                    {
                                        return( iff );
                                    }
                                }
                            }
                        }
                    }
                }
                CloseIFF( iff );
            }
            Close( iff->iff_Stream );
        }
        FreeIFF( iff );
    }
    return( NULL );
}

void closeILBM( struct IFFHandle *iff )
{
    CloseIFF( iff );
    Close( iff->iff_Stream );
    FreeIFF( iff );
}

static BOOL unpackRow( BYTE **pbuffer, LONG *psize, BYTE *plane, UBYTE cmp, WORD bpr )
{
    BYTE *buffer = *pbuffer;
    LONG size = *psize;

    if( cmp == cmpNone )
    {
        if( size < bpr )
        {
            return( FALSE );
        }
        size -= bpr;
        while( bpr-- )
        {
            *plane++ = *buffer++;
        }
    }
    else if( cmp == cmpByteRun1 )
    {
        while( bpr > 0 )
        {
            BYTE c;
            if( size < 1 )
            {
                return( FALSE );
            }
            size--;
            if( ( c = *buffer++ ) >= 0 )
            {
                WORD count = c + 1;
                if( size < count || bpr < count )
                {
                    return( FALSE );
                }
                size -= count;
                bpr -= count;
                while( count-- )
                {
                    *plane++ = *buffer++;
                }
            }
            else if( c != -128 )
            {
                WORD count = -c + 1;
                BYTE data;
                if( size < 1 || bpr < count )
                {
                    return( FALSE );
                }
                size--;
                bpr -= count;
                data = *buffer++;
                while( count-- )
                {
                    *plane++ = data;
                }
            }
        }
    }
    else
    {
        return( FALSE );
    }

    *pbuffer = buffer;
    *psize = size;
    return( TRUE );
}

static BOOL unpackPicture( BYTE *buffer, LONG size, struct BitMap *bm, BitMapHeader *bmhd )
{
    WORD y, p;
    PLANEPTR planes[ 8 + 1 ];
    WORD bpr = RowBytes( bmhd->w );

    assert( bm->Depth <= 9 );

    for( p = 0; p < bm->Depth; p++ )
    {
        planes[ p ] = bm->Planes[ p ];
    }

    for( y = 0; y < bm->Rows; y++ )
    {
        for( p = 0; p < bm->Depth; p++ )
        {
            if( !unpackRow( &buffer, &size, planes[ p ], bmhd->compression, bpr ) )
            {
                return( FALSE );
            }
            planes[ p ] += bm->BytesPerRow;
        }
    }

    return( TRUE );
}

struct BitMap *readPicture( struct IFFHandle *iff )
{
    struct StoredProperty *sp;
    BitMapHeader *bmhd;
    struct ContextNode *cn;
    struct BitMap *bm = NULL;
    BYTE *buffer;
    LONG size;
    BOOL result = FALSE;
    UBYTE depth;

    if( sp = FindProp( iff, ID_ILBM, ID_BMHD ) )
    {
        bmhd = ( BitMapHeader * )sp->sp_Data;
        depth = bmhd->nPlanes;

        if( bmhd->masking == mskHasMask )
        {
            depth++;
        }

        if( cn = CurrentChunk( iff ) )
        {
            size = cn->cn_Size;
            if( buffer = AllocMem( size, MEMF_PUBLIC ) )
            {
                if( ReadChunkBytes( iff, buffer, size ) == size )
                {
                    if( bm = AllocBitMap( bmhd->w, bmhd->h, depth, 0, NULL ) )
                    {
                        if( !( result = unpackPicture( buffer, size, bm, bmhd ) ) )
                        {
                            FreeBitMap( bm );
                            bm = NULL;
                        }
                    }
                }
                FreeMem( buffer, size );
            }
        }
    }
    return( result ? bm : NULL );
}

struct Window *openScreen( UBYTE depth, struct IFFHandle *iff, WORD **pens, WORD *pcount )
{
    struct Screen *s;
    WORD pens[] = { ~0 };

    if( s = OpenScreenTags( NULL,
        SA_DisplayID, PAL_MONITOR_ID | LORES_KEY,
        SA_Depth, depth,
        SA_Quiet, TRUE,
        SA_Exclusive, TRUE,
        SA_ShowTitle, FALSE,
        SA_BackFill, LAYERS_NOBACKFILL,
        SA_SharePens, TRUE,
        SA_Pens, pens,
        SA_Title, "SysRobbo Screen",
        TAG_DONE ) )
    {
        if( *pens = obtainPens( s->ViewPort.ColorMap, iff, pcount ) )
        {
            struct Window *w;

            if( w = OpenWindowTags( NULL,
                WA_CustomScreen, s,
                WA_Left, 0,
                WA_Top, 0,
                WA_Width, s->Width,
                WA_Height, s->Height,
                WA_Backdrop, TRUE,
                WA_Borderless, TRUE,
                WA_SimpleRefresh, TRUE,
                WA_BackFill, LAYERS_NOBACKFILL,
                WA_Activate, TRUE,
                WA_IDCMP, IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_REFRESHWINDOW | IDCMP_RAWKEY,
                TAG_DONE ) )
            {
                return( w );
            }
            releasePens( s->ViewPort.ColorMap, pens, *pcount );
        }
        CloseScreen( s );
    }
    return( NULL );
}

void closeScreen( struct Window *w, WORD *pens, WORD count )
{
    struct Screen *s = w->WScreen;

    CloseWindow( w );
    releasePens( s->ViewPort.ColorMap, pens, count );    
    CloseScreen( s );    
}

/* Allocate colors */

static WORD *obtainPens( struct ColorMap *cm, struct IFFHandle *iff, WORD *pcount )
{
    struct StoredProperty *sp;

    if( sp = FindProp( iff, ID_ILBM, ID_CMAP ) )
    {
        WORD count = sp->sp_Size / 3;
        WORD i;
        WORD *pens;
        UBYTE *data = ( UBYTE * )sp->sp_Data;

        if( pens = ( WORD * )AllocVec( count * sizeof( WORD ), MEMF_PUBLIC ) )
        {
            for( i = 0; i < count; i++ )
            {
                UBYTE red = *data++;
                UBYTE green = *data++;
                UBYTE blue = *data++;

                pens[ i ] = ObtainPen( cm, i, RGB( red ), RGB( green ), RGB( blue ), 0 );
            }
            *pcount = count;
            return( pens );
        }
    }
    return( NULL );
}

static void releasePens( struct ColorMap *cm, WORD *pens, WORD count )
{
    WORD i;

    for( i = 0; i < count; i++ )
    {
        if( pens[ count - 1 - i ] != -1 )
        {
            ReleasePen( cm, pens[ count - 1 - i ] );
        }
    }
    FreeVec( pens );
}

/* Remap picture */

BOOL remapPicture( struct BitMap *bm, struct ColorMap *cm, struct IFFHandle *iff, UBYTE maxPen )
{
    struct StoredProperty *sp;
    BOOL result = FALSE;

    if( sp = FindProp( iff, ID_ILBM, ID_CMAP ) )
    {
        WORD count = sp->sp_Size / 3;
        WORD i;
        WORD *pens;
        UBYTE *data = ( UBYTE * )sp->sp_Data;
        UBYTE *buffer;
        WORD width = ( ( GetBitMapAttr( bm, BMA_WIDTH ) + 15 ) >> 4 ) << 4;
        WORD height = GetBitMapAttr( bm, BMA_HEIGHT );
        UBYTE depth = GetBitMapAttr( bm, BMA_DEPTH );

        if( pens = AllocVec( count * sizeof( WORD ), MEMF_PUBLIC ) )
        {
            for( i = 0; i < count; i++ )
            {
                UBYTE red = *data++;
                UBYTE green = *data++;
                UBYTE blue = *data++;

                pens[ i ] = FindColor( cm, RGB( red ), RGB( green ), RGB( blue ), maxPen );
            }

            if( buffer = AllocMem( width, MEMF_PUBLIC ) )
            {
                struct RastPort rp, temprp;

                InitRastPort( &rp );
                temprp = rp;
                rp.BitMap = bm;
                if( temprp.BitMap = AllocBitMap( width, 1, depth, BMF_INTERLEAVED, NULL ) )
                {
                    WORD y;

                    for( y = 0; y < height; y++ )
                    {
                        WORD x;
                        ReadPixelLine8( &rp, 0, i, width, buffer, &temprp );

                        for( x = 0; x < width; x++ )
                        {
                            buffer[ x ] = pens[ buffer[ x ] ];
                        }

                        WritePixelLine8( &rp, 0, i, width, buffer, &temprp );
                    }

                    result = TRUE;
                    WaitBlit();
                    FreeBitMap( temprp.BitMap );
                }
                FreeMem( buffer, width );
            }
            FreeVec( pens );
        }
    }
    return( result );
}
