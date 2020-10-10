/******************************************
Copyright (C) 2020 Mate Soos

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
***********************************************/

#include "louvain_communities/louvain_communities.h"

#include <unistd.h>
#include "commlouvain/graph_binary.h"
#include "commlouvain/graph_plain.h"
#include "commlouvain/louvain.h"
#include "commlouvain/balmod.h"
#include "commlouvain/condora.h"
#include "commlouvain/devind.h"
#include "commlouvain/devuni.h"
#include "commlouvain/dp.h"
#include "commlouvain/goldberg.h"
#include "commlouvain/modularity.h"
#include "commlouvain/owzad.h"
#include "commlouvain/shimalik.h"
#include "GitSHA1.h"

namespace LouvainC {

struct PrivateData {
    GraphPlain gplain;
    unsigned short nb_calls = 0;
    long double precision = 0.000001L;
    uint32_t verbosity = 0;
};

DLL_PUBLIC Communities::Communities()
{
    data = new PrivateData;
}

DLL_PUBLIC void Communities::calculate(bool weighted)
{
    vector<unsigned long long> deg_seq;
    vector<int> out_links;
    vector<long double> out_w;
    data->gplain.binary_to_mem(deg_seq, out_links, out_w, WEIGHTED);
    GraphBin g(deg_seq, out_links, out_w, WEIGHTED);
    Quality *q = new Modularity(g);
    data->nb_calls++;

    cerr << "Computation of communities with the " << q->name << " quality function" << endl;
    Louvain c(-1, data->precision, q);

    bool improvement = true;

    long double quality = (c.qual)->quality();
    long double new_qual;

    int level = 0;
    vector<vector<int> > levels;

    do {
        if (data->verbosity) {
            cout << "level " << level << ":\n";
            cout << "  network size: " << (c.qual)->g.nb_nodes << " nodes, " << (c.qual)->g.nb_links
                 << " links, " << (c.qual)->g.total_weight << " weight" << endl;
        }

        improvement = c.one_level();
        new_qual = (c.qual)->quality();

        levels.push_back(vector<int>());
        c.display_partition(&(levels[level]));

        g = c.partition2graph_binary();
        delete q;
        q = new Modularity(g);
        data->nb_calls++;

        c = Louvain(-1, data->precision, q);

        if (data->verbosity) {
            cout << "  quality increased from " << quality << " to " << new_qual << endl;
        }

        quality = new_qual;
        level++;
    } while (improvement);

    if (data->verbosity) {
        cout << "Quality: " << new_qual << endl;
    }
    //print_final(levels);
}

DLL_PUBLIC std::vector<std::pair<unsigned int, unsigned int> > Communities::get_mapping()
{
    std::vector<std::pair<unsigned int, unsigned int> > ret;

    return ret;
}


DLL_PUBLIC const char* get_version()
{
    return get_version_sha1();
}




}
