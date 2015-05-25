#include <stdio.h>
#include <dlfcn.h>

int main()
{
        char* name = 0;

        scanf ("%ms", &name);
        if (!name) {
                return 1;
        }

        void* handle = dlopen (name, RTLD_LAZY);
        printf ("%s\n", handle ? "YES" : "NO");
        return 0;
}