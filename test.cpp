#include <bits/stdc++.h>

using namespace std;

int main() 
{
    std::string buff;
    bool first = true;
    int num = 0;

    while (getline(cin, buff))
    {
        if (first)
            num = atoi(buff.c_str());
        else
            num = num - atoi(buff.c_str());
        buff.clear();
    }
    return 0;
}