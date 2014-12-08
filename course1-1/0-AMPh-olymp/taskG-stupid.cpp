unsigned fib (unsigned N)
{
    // No time limit? Really?
    return (N > 1) ? fib (N-1) + fib (N-2)
                   : 1;
}
