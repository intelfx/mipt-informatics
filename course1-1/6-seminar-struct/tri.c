void getCoord (Coord* a)
{
    scanf ("%d %d", &a->x, &a->y);
}

void printCoord (Coord a)
{
    printf ("(%d,%d)", a.x, a.y);
}

inline int sq (int arg)
{
    return arg * arg;
}

inline int distance (Coord* a, Coord* b)
{
    return sq (a->x - b->x) +
           sq (a->y - b->y);
}

Triangle* getDataTri()
{
    Triangle* r = (Triangle*) malloc (sizeof (Triangle));

    for (size_t i = 0; i < 3; ++i) {
        getCoord (&r->lin[i]);
    }

    for (size_t i = 0; i < 3; ++i) {
        r->size[i] = distance (&r->lin[i], &r->lin[(i + 1) % 3]);
    }

    return r;
}

void printTri (Triangle *a)
{
    for (size_t i = 0; i < 3; ++i) {
        printCoord (a->lin[i]);
        putchar (' ');
    }
    putchar ('\n');
}

int cmpTri (const Triangle* a, const Triangle* b)
{
    // we track b's sides
    int matched[3];
    memset (matched, 0, sizeof (matched));

    for (size_t i = 0; i < 3; ++i) {
        int ok = 0;
        for (size_t j = 0; j < 3; ++j) {
            if (!matched[j] &&
                (a->size[i] == b->size[j])) {
                    matched[j] = 1;
                    ok = 1;
                    break;
            }
        }

        if (!ok) {
            return 0;
        }
    }

    return 1;
}

int deleteTri (Triangle* a)
{
    free (a);
    return 0;
}
