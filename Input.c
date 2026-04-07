
#include <exec/semaphores.h>
#include <exec/memory.h>
#include <exec/interrupts.h>

#include <devices/input.h>
#include <devices/inputevent.h>

#include <intuition/intuitionbase.h>

#include <clib/exec_protos.h>
#include <clib/layers_protos.h>

#include "Input.h"

#ifdef AMIGA
#define REG( r ) register __##r
#else
#define REG( r )
#endif

extern struct IntuitionBase *IntuitionBase;

#ifdef AMIGA
__saveds __asm
#endif

static struct InputEvent *myInput( REG( a0 ) struct InputEvent *ie, REG( a1 ) struct inputData *id )
{
    struct InputEvent *saveie = ie;
    struct inputMessage *im;

    while( ie )
    {
        if( ie->ie_Class == IECLASS_RAWMOUSE )
        {
            if( ie->ie_Code == IECODE_LBUTTON )
            {
                if( IntuitionBase->FirstScreen == id->s )
                {
                    WORD x, y;
                    struct Layer *l;

                    x = id->s->MouseX;
                    y = id->s->MouseY;

                    if( l = WhichLayer( &id->s->LayerInfo, x, y ) )
                    {
                        struct Window *w = ( struct Window * )l->Window;

                        if( w )
                        {
                            struct Gadget *gad;

                            x -= w->LeftEdge;
                            y -= w->TopEdge;

                            for( gad = w->FirstGadget; gad != NULL; gad = gad->NextGadget )
                            {
                                if( gad->LeftEdge <= x && gad->TopEdge <= y && gad->LeftEdge + gad->Width - 1 >= x && gad->TopEdge + gad->Height - 1 >= y )
                                {
                                    if( im = AllocMem( sizeof( *im ), MEMF_PUBLIC | MEMF_CLEAR ) )
                                    {
                                        im->msg.mn_Length = sizeof( *im );
                                        im->w = w;
                                        im->gad = gad;
                                        PutMsg( id->mp, &im->msg );
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            else if( ie->ie_Code == ( IECODE_LBUTTON | IECODE_UP_PREFIX ) )
            {
                if( im = AllocMem( sizeof( *im ), MEMF_PUBLIC | MEMF_CLEAR ) )
                {
                    im->msg.mn_Length = sizeof( *im );
                    im->w = NULL;
                    im->gad = NULL;
                    PutMsg( id->mp, &im->msg );
                }
            }
        }
        ie = ie->ie_NextEvent;
    }

    return( saveie );
}

BOOL openInput( struct inputData *id, struct Screen *s )
{
    struct MsgPort *mp;
    if( id->mp = CreateMsgPort() )
    {
        id->s = s;
        if( mp = CreateMsgPort() )
        {
            if( id->io = ( struct IOStdReq * )CreateIORequest( mp, sizeof( *id->io ) ) )
            {
                if( OpenDevice( "input.device", 0, ( struct IORequest * )id->io, 0L ) == 0 )
                {
                    id->is.is_Code = ( void( * )( ) )myInput;
                    id->is.is_Data = ( APTR )id;
                    id->is.is_Node.ln_Pri = 51;
                    id->is.is_Node.ln_Name = "GameX Input";

                    id->io->io_Command = IND_ADDHANDLER;
                    id->io->io_Data = &id->is;
                    DoIO( ( struct IORequest * )id->io );
                    return( TRUE );
                }
                DeleteIORequest( id->io );
            }
            DeleteMsgPort( mp );
        }
        DeleteMsgPort( id->mp );
    }
    return( FALSE );
}

void closeInput( struct inputData *id )
{
    struct MsgPort *mp = id->io->io_Message.mn_ReplyPort;
    struct inputMessage *im;

    id->io->io_Command = IND_REMHANDLER;
    id->io->io_Data = &id->is;
    DoIO( ( struct IORequest * )id->io );

    while( im = ( struct inputMessage * )GetMsg( id->mp ) )
    {
        FreeMem( im, sizeof( *im ) );
    }

    CloseDevice( ( struct IORequest * )id->io );
    DeleteIORequest( id->io );
    DeleteMsgPort( mp );
    DeleteMsgPort( id->mp );
}
