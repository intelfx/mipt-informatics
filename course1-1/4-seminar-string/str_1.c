size_t my_strlen (const char* s)
{
    const char *end = s;
    while (*end++);
    return end - s - 1;
}
