#include <iostream>
#include <string>
#include <cmath>
#include <stdlib.h>

typedef long double data_t;

static const data_t PI = atan2l (0, -1);

data_t sqrl (data_t arg)
    { return arg * arg; }

data_t to_rad (data_t deg, int denominator = 1)
    { return (deg * PI) / (denominator * 180); }

data_t read_latlon_radians (std::istream& in, const std::string& hemi_pos, const std::string& hemi_neg)
{
    int deg, min, sec;
    std::string hemisphere;
    in >> deg >> min >> sec >> hemisphere;

    data_t radians = to_rad (deg) + to_rad (min, 60) + to_rad (sec, 3600);

    if (hemisphere == hemi_pos) {
        return radians;
    } else if (hemisphere == hemi_neg) {
        return -radians;
    } else {
        abort();
    }
}

struct point
{
    data_t lon, lat;

    friend std::istream& operator>> (std::istream& in, point& pt)
    {
        pt.lon = read_latlon_radians (in, "e", "w");
        pt.lat = read_latlon_radians (in, "n", "s");
        return in;
    }

    static data_t central_angle (const point& p1, const point& p2)
    {
        data_t dlon = fabsl (p1.lon - p2.lon),
               sinlat1sinlat2 = sinl (p1.lat) * sinl (p2.lat),
               coslat1coslat2 = cosl (p1.lat) * cosl (p2.lat),
               cosdlon = cosl (dlon);

        return acosl (sinlat1sinlat2 + coslat1coslat2 * cosdlon);
    }
};

int main()
{
    data_t R, min_central_angle = INFINITY;
    point plane;
    size_t airport_count;
    std::cin >> R >> plane >> airport_count;

    for (size_t i = 0; i < airport_count; ++i) {
        point airport;
        std::cin >> airport;

        min_central_angle = std::min (min_central_angle,
                                      point::central_angle (plane, airport));
    }

    std::cout << min_central_angle * R << std::endl;
    return 0;
}
