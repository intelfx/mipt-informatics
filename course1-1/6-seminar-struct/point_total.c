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

inline int sq (int arg)
{
    return arg * arg;
}

inline float get_distance (const Point* a, const Point* b)
{
    return sqrt (sq (a->x - b->x) + sq (a->y - b->y));
}

void Distance (const MassP* m, MassPair* massp)
{
    massp->n = m->n * (m->n - 1) / 2;
    massp->para = (Pair*) malloc (massp->n * sizeof (Pair));

    int current_pair = 0;

    for (int i = 1; i < m->n; ++i) {
        for (int j = 0; j < i; ++j) {
            massp->para[current_pair].a = m->p[i];
            massp->para[current_pair].b = m->p[j];
            massp->para[current_pair].len = get_distance (&m->p[i], &m->p[j]);
            ++current_pair;
        }
    }
}

void prPair (Pair du)
{
    prPoint (du.a);
    putchar (' ');
    prPoint (du.b);
    putchar (' ');
    printf (" - %.2f\n", du.len);
}

void printMPair (const MassPair *mp)
{
    for (int i = 0; i < mp->n; ++i) {
        prPair (mp->para[i]);
    }
}

int fp_cmp (float a, float b)
{
    static const float eps = 0.001;

    return (fabs (a - b) <= eps);
}

MassPair minDistance (const MassPair* m)
{
    MassPair r;

    float min_dist = INFINITY;
    int count = 0;
    for (int i = 0; i < m->n; ++i) {
        float candidate_dist = m->para[i].len;

        if (fp_cmp (candidate_dist, min_dist)) {
            ++count;
        } else if (candidate_dist < min_dist) {
            min_dist = candidate_dist;
            count = 1;
        }
    }

    r.n = count;
    r.para = (Pair*) malloc (count * sizeof (Pair));

    int current = 0;
    for (int i = 0; i < m->n; ++i) {
        if (fp_cmp (m->para[i].len, min_dist)) {
            r.para[current++] = m->para[i];
        }
    }

    return r;
}

int sort_predicate (const void* a, const void* b)
{
    const Pair *p_a = (const Pair*) a,
               *p_b = (const Pair*) b;

    if (p_a->len < p_b->len) {
        return -1;
    } else if (p_a->len > p_b->len) {
        return 1;
    } else {
        return 0;
    }
}

void sortDistance (MassPair* m)
{
    qsort (m->para, m->n, sizeof (Pair), sort_predicate);
}
