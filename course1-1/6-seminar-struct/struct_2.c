int areaRect0 (struct Rect0 rect)
{
    return rect.width * rect.height;
}

int areaRect1 (struct Rect1 rect)
{
    return (rect.rb.x - rect.lt.x) * (rect.lt.y - rect.rb.y);
}
