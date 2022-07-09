#include "collision.h"
#include "blitter.h"

/*
    Per il momento considero tutto il blocco del bob, eventualmente potrei anche
    specificare un riquadro personalizzato di collisione

    TODO: Si potrebbe ottimizzare togliendo quelle assegnazioni x1, y1 ecc... Metto meno roba
    nello stack.
*/
BOOL checkCollision(BlitterBob* bob1, BlitterBob* bob2) {

    if ((bob1->state != DRAW) || (bob2->state != DRAW)) return FALSE;
    
    int x1 = bob1->x + bob1->header.colBound.xoffset;
    int y1 = bob1->y + bob1->header.colBound.yoffset;
    int x2 = bob1->x + bob1->header.colBound.width;
    int y2 = bob1->y + bob1->header.colBound.height;

    int x3 = bob2->x + bob2->header.colBound.xoffset;
    int y3 = bob2->y + bob2->header.colBound.yoffset;
    int x4 = bob2->x + bob2->header.colBound.width;
    int y4 = bob2->y + bob2->header.colBound.height;

    if ((x3 > x2) || (x1 > x4)) {
        return 0;
    }
    if ((y3 > y2) || (y1 > y4)) {
        return 0;
    }
    return 1;
}