
#include <exec/types.h>

extern void drawIcon( struct BitMap *sbm, WORD sx, WORD sy, struct BitMap *dbm, WORD dx, WORD dy, WORD width, WORD height, UBYTE minterm, UBYTE mask, struct BitMap *mbm, WORD mx, WORD my );
extern void drawIconRastPort( struct BitMap *sbm, WORD sx, WORD sy, struct RastPort *rp, WORD dx, WORD dy, WORD width, WORD height, UBYTE minterm, struct BitMap *mbm, WORD mx, WORD my );
