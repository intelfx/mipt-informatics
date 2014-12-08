int my_strcmp (const char* s1, const char* s2)
{
    while (*s1 && *s2 && (*s1 == *s2)) {
        ++s1;
        ++s2;
    }
    if ((unsigned)*s1 < (unsigned)*s2) return -1;
    else if ((unsigned)*s1 > (unsigned)*s2) return 1;
    else return 0;
}
