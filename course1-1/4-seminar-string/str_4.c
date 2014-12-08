char* my_strcat (char* dest, const char* src)
{
    strcpy (dest + strlen (dest), src);
    return dest;
}
