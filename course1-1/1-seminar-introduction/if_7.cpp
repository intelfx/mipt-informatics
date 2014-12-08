#include <iostream>

bool rook_beats_piece (int rook_r, int rook_c, int piece_r, int piece_c)
{
    return (rook_r == piece_r) || (rook_c == piece_c);
}

int main()
{
    int rook_r, rook_c, piece_r, piece_c;
    std::cin >> rook_r >> rook_c >> piece_r >> piece_c;
    std::cout << (rook_beats_piece (rook_r, rook_c, piece_r, piece_c) ? "YES" : "NO") << std::endl;
}
