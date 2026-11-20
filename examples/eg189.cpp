#include <iostream>
#include <algorithm>

using namespace std;

int main()
{
    string s = "God is great";
    transform(
        s.begin(), s.end(),
        s.begin(),
        [](char c)
        {
            return toupper(c);
        });

    cout << s << endl;

    return 0;
}