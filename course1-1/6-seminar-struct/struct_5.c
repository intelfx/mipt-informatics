struct Rect* max (struct Rect* pa, struct Rect* pb)
{
    if (pa->width * pa->height >= pb->width * pb->height) {
        return pa;
    } else {
        return pb;
    }
}
