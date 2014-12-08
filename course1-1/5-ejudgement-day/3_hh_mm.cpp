#include <cstdio>

int main()
{
    int h_dep, m_dep, h_arr, m_arr;
    std::scanf ("%d:%d", &h_dep, &m_dep);
    std::scanf ("%d:%d", &h_arr, &m_arr);

    int m_enr = m_arr - m_dep,
        h_enr = h_arr - h_dep;

    if (m_enr < 0) {
        m_enr += 60;
        --h_enr;
    }

    if (h_enr < 0) {
        h_enr += 24;
    }

    std::printf ("%02d:%02d\n", h_enr, m_enr);
}
