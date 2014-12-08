char* my_strncpy (char* dest, const char* src, size_t n)
{
    char* ret = dest;
    while ((n != 0) && (*dest++ = *src++)) {
        --n;
    }

    return ret;
}
