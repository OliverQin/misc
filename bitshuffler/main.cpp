#include "bit_shuffler.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdlib>

using namespace std; 

int main (int argc, char* argv[]){
    const uint64_t Len = 8;//=atoi(argv[1]);
    
    BitShuffler<Len> a(time(0));
    
    uint64_t sum = 0;
    for (uint64_t i=0; i<(1ull<<Len); ++i) {
        //sum += a.getNumber(i);
        //cout << (a.getNumber(i)&1);
        cout << setw(5) << a.getNumber(i);
        if (i+1==(1ull<<Len) || ((i & 7) == 7))
            cout << endl;
    }
    cout << sum << endl;
}