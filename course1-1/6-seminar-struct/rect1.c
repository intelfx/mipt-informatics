void getCoord (Coord* a)
{
    scanf ("%d %d", &a->x, &a->y);
}

void printCoord (Coord a)
{
    printf ("(%d,%d)", a.x, a.y);
}

void getDataRec (Rectangle* a)
{
    getCoord (&a->leftH);
    getCoord (&a->rightL);
}

void printRec(Rectangle a)
{
    printCoord (a.leftH);
    putchar (' ');
    printCoord (a.rightL);
}

Rectangle* findCrossRec(Rectangle a, Rectangle b)
{
         Rectangle* r;

         if ((a.rightL.x < b.leftH.x) ||
             (b.rightL.x < a.leftH.x)) {
             return NULL;
         }

         if ((a.leftH.y < b.rightL.y) ||
             (b.leftH.y < a.rightL.y)) {
             return NULL;
         }

         r = (Rectangle*) malloc (sizeof (Rectangle));

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) < (y)) ? (y) : (x))
         r->leftH.x = MAX (a.leftH.x, b.leftH.x);
         r->leftH.y = MIN (a.leftH.y, b.leftH.y);
         r->rightL.x = MIN (a.rightL.x, b.rightL.x);
         r->rightL.y = MAX (a.rightL.y, b.rightL.y);
#undef MIN
#undef MAX

         return r;
}

int deleteRec (Rectangle* a)
{
    free (a);
    return 0;
}
