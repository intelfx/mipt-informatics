void getData (MassP* m)
{
    scanf ("%d", &m->n);
    m->p = (Point*) malloc (m->n * sizeof (Point));

    for (int i = 0; i < m->n; ++i) {
        scanf ("%d %d", &m->p[i].x, &m->p[i].y);
    }
}

void prPoint (Point a)
{
    printf ("(%d,%d)", a.x, a.y);
}

void printMP (const MassP* a)
{
    for (int i = 0; i < a->n; ++i) {
        prPoint (a->p[i]);
        putchar (' ');
    }
    putchar ('\n');
}
