
#include "louvain_communities.h"
#include <unistd.h>
#include <iostream>


using namespace LouvainC;
using std::cout;
using std::endl;

int main(int /*argc*/, char ** /*argv*/)
{
    Communities gplain;
    #include "mygraph.cpp"
    gplain.calculate(true);
    auto r = gplain.get_mapping();
    for(auto& c: r) {
        cout << c.first << " " << c.second << endl;
    }
}
