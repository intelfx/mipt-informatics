char* my_strstr (const char* s, const char* ss)
{
    size_t ss_len = strlen (ss);

    do {
        if (!strncmp (s, ss, ss_len)) {
            return (char*)s;
        }
    } while (*s++);

    return 0;
}
