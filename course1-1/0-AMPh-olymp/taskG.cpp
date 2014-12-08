unsigned fib (unsigned N)
{
    // No time limit? Really? I'm tempted to do it the recursive way.
    /*
    return (N > 1) ? fib (N-1) + fib (N-2)
                   : 1;
    */

    unsigned* fib_temp = new unsigned[N+1];
    fib_temp[0] = 1;
    fib_temp[1] = 1;
    for (unsigned i = 2; i <= N; ++i) {
        fib_temp[i] = fib_temp[i-1] + fib_temp[i-2];
    }

    unsigned result = fib_temp[N];
    delete[] fib_temp;
    return result;
}
