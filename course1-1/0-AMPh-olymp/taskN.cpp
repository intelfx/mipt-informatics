#include <iostream>
#include <algorithm>
#include <iterator>
#include <set>
#include <utility>
#include <vector>

typedef unsigned long data_t;
typedef unsigned long long moment_t;

static const size_t ACTIONS = 3;

struct WindowSet : private std::set<std::pair<moment_t, data_t>>
{
    data_t count;
    moment_t latency;

    void init (data_t c, moment_t l)
    {
        count = c;
        latency = l;

        for (data_t i = 0; i < count; ++i) {
            insert (std::make_pair (0, i));
        }
    }

    moment_t serve (moment_t arrival)
    {
        auto it = begin(); // window with least time to become-free
        auto win = *it;

        win.first = std::max (arrival, win.first) + latency;

        erase (it);
        insert (win);

        return win.first;
    }
};

int main()
{
    data_t window_counts[ACTIONS];
    std::copy_n (std::istream_iterator<data_t> (std::cin), ACTIONS, window_counts);

    moment_t window_latencies[ACTIONS];
    std::copy_n (std::istream_iterator<moment_t> (std::cin), ACTIONS, window_latencies);

    data_t N;
    std::cin >> N;

    WindowSet windows[ACTIONS];

    for (data_t i = 0; i < ACTIONS; ++i) {
        windows[i].init (std::min (N, window_counts[i]), window_latencies[i]);
    }

    moment_t answer = 0;

    for (data_t i = 0; i < N; ++i) {
        moment_t arrival_time;
        std::cin >> arrival_time;

        /*
         * Process each person in all windows at once because order of
         * people exiting stage X is the same as order of entering this stage.
         */

        moment_t departure_time = arrival_time;
        for (size_t i = 0; i < ACTIONS; ++i) {
            departure_time = windows[i].serve (departure_time);
        }

        answer = std::max (answer, departure_time - arrival_time);
    }

    std::cout << answer << std::endl;
    return 0;
}
