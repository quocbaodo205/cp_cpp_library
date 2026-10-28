// Lib: https://github.com/quocbaodo205/cp_cpp_library

#include <algorithm>
#include <iostream>
#include <string>

#include "cp/io.hpp"
#include "cp/math/modint.hpp"
#include "cp/types.hpp"

using namespace std;
using namespace cp;

using Mint = ModInt1000000007;

void solve() {
    int h, w;
    cin >> h >> w;
    Vec<string> s(h);
    for (int i = 0; i < h; ++i)
        cin >> s[i];

    int mmax = max(h, w);
    Vec<Mint> rows(h + 1, 0);
    Vec<Mint> cols(w + 1, 0);
    Vec<Mint> dia(2 * mmax + 1, 0);
    Vec<Vec<Mint>> dp(h + 1, Vec<Mint>(w + 1, 0));

    rows[1] = 1;
    cols[1] = 1;
    dia[mmax] = 1;
    dp[1][1] = 1;
    for (int i = 1; i <= h; ++i) {
        for (int j = 1; j <= w; ++j) {
            if (i == 1 && j == 1)
                continue;
            if (s[i - 1][j - 1] == '.') {
                dp[i][j] = rows[i] + cols[j] + dia[i - j + mmax];
                rows[i] += dp[i][j];
                cols[j] += dp[i][j];
                dia[i - j + mmax] += dp[i][j];
            } else {
                rows[i] = 0;
                cols[j] = 0;
                dia[i - j + mmax] = 0;
            }
        }
    }

    cout << dp[h][w].value() << "\n";
}

int main() {
    init_io();
    solve();
}
