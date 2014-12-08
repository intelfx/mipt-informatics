int in_region (float x, float y)
{
    static const double eps = 1e-9;

    if (((x + 4) * (x + 4) + (y - 4) * (y - 4)) <= 9 + eps) {
        return 1;
    }

    if (((x - 4) * (x - 4) + (y + 4) * (y + 4)) <= 9 + eps) {
        return 1;
    }

    return 0;
}
