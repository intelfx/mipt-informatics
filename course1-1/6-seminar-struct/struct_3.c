struct Rect1 convert01 (struct Rect0 rect)
{
    struct Rect1 ret = { { rect.x, rect.y },
                         { rect.x + rect.width, rect.y - rect.height } };
    return ret;
}

struct Rect0 convert10 (struct Rect1 rect)
{
    struct Rect0 ret = { rect.lt.x, rect.lt.y,
                         rect.rb.x - rect.lt.x, rect.lt.y - rect.rb.y };
    return ret;
}
