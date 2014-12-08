#include <iostream>
#include <iomanip>
using namespace std;

void write_time (int minutes_from_midnight)
{
    cout << setfill('0') << setw(2) << minutes_from_midnight / 60 << ":"
         << setfill('0') << setw(2) << minutes_from_midnight % 60 << std::endl;
}

int main()
{
    int lesson;
    cin >> lesson;
    write_time (8*60 + (lesson - 1) * 50 + 45);
}
