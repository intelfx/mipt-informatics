void printTime (Time c)
{
    printf ("%02d:%02d\n", c.hour, c.min);
}

Time getTime()
{
    Time r;
    scanf ("%d:%d", &r.hour, &r.min);
    return r;
}

// 23:55+00:10=00:05
Time add (Time a, Time b)
{
    Time r = {
        .hour = a.hour + b.hour,
        .min = a.min + b.min
    };

    if (r.min >= 60) {
        r.hour += r.min / 60;
        r.min %= 60;
    }

    if (r.hour >= 24) {
        r.hour %= 24;
    }

    return r;
}

// 00:10-00:20=23:50
Time diff (Time a, Time b)
{
    Time r = {
        .hour = a.hour - b.hour,
        .min = a.min - b.min
    };

    while (r.min < 0) {
        r.min += 60;
        --r.hour;
    }

    while (r.hour < 0) {
        r.hour += 24;
    }

    return r;
}

