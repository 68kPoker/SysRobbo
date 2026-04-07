
#include <exec/types.h>
#include <exec/interrupts.h>

struct inputData
{
    struct IOStdReq *io;
    struct Interrupt is;
    struct Screen *s;
    struct MsgPort *mp;
};

struct inputMessage
{
    struct Message msg;
    struct Window *w;
    struct Gadget *gad;
};

extern BOOL openInput( struct inputData *id, struct Screen *s );
extern void closeInput( struct inputData *id );
