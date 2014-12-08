char* my_strdup (const char* p)
{
    char* ret = malloc (strlen (p) + 1);
    strcpy (ret, p);
    return ret;
}
