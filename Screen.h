
#include <exec/types.h>

#define DEPTH 5

extern BOOL remapPicture( struct BitMap *bm, struct ColorMap *cm, struct IFFHandle *iff, UBYTE maxPen );
extern struct Window *openScreen( UBYTE depth, struct IFFHandle *iff, WORD **pens, WORD *pcount );
extern void closeScreen( struct Window *w, WORD *pens, WORD count );
extern struct IFFHandle *openILBM( STRPTR name );
extern void closeILBM( struct IFFHandle *iff );
struct BitMap *readPicture( struct IFFHandle *iff );
