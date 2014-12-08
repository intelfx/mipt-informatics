char* my_strchr (const char* s, int c)
{
    do {
        if (*s == c) {
            return (char*)s;
        }
    } while (*s++);

    return 0;
}
