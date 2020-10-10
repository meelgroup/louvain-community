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

#include "louvain_communities.h"

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
#include "commlouvain/zahn.h"
#include "GitSHA1.h"

namespace LouvainC {

struct PrivateData {
    ~PrivateData()
    {
        delete q;
    }
    GraphPlain gplain;
    unsigned short nb_calls = 0;
    long double precision = 0.000001L;
    uint32_t verbosity = 0;
    vector<vector<int>> levels;

    //quality measure
    Quality *q = NULL;
    int id_qual = 0;
    long double max_w = 1.0L;
    long double alpha = 0.5L;
    int kmin = 1;
    long double sum_se = 0.0L;
    long double sum_sq = 0.0L;
};

DLL_PUBLIC Communities::Communities()
{
    data = new PrivateData;
}

DLL_PUBLIC Communities::~Communities()
{
    delete data;
}

DLL_PUBLIC void Communities::set_quality_type(unsigned id)
{
    data->id_qual = id;
}

void init_quality(PrivateData* data, GraphBin* g)
{
    delete data->q;
    switch (data->id_qual) {
        case 0:
            data->q = new Modularity(*g);
            break;
        case 1:
            if (data->nb_calls == 0)
                data->max_w = g->max_weight();
            data->q = new Zahn(*g, data->max_w);
            break;
        case 2:
            if (data->nb_calls == 0)
                data->max_w = g->max_weight();
            if (data->alpha <= 0.0L || data->alpha >= 1.0L)
                data->alpha = 0.5L;
            data->q = new OwZad(*g, data->alpha, data->max_w);
            break;
        case 3:
            if (data->nb_calls == 0)
                data->max_w = g->max_weight();
            data->q = new Goldberg(*g, data->max_w);
            break;
        case 4:
            if (data->nb_calls == 0) {
                g->add_selfloops();
                data->sum_se = CondorA::graph_weighting(g);
            }
            data->q = new CondorA(*g, data->sum_se);
            break;
        case 5:
            data->q = new DevInd(*g);
            break;
        case 6:
            data->q = new DevUni(*g);
            break;
        case 7:
            if (data->nb_calls == 0) {
                data->max_w = g->max_weight();
                data->sum_sq = DP::graph_weighting(g);
            }
            data->q = new DP(*g, data->sum_sq, data->max_w);
            break;
        case 8:
            if (data->kmin < 1)
                data->kmin = 1;
            data->q = new ShiMalik(*g, data->kmin);
            break;
        case 9:
            if (data->nb_calls == 0)
                data->max_w = g->max_weight();
            data->q = new BalMod(*g, data->max_w);
            break;
        default:
            data->q = new Modularity(*g);
            break;
    }
}

DLL_PUBLIC void Communities::calculate(bool weighted)
{
    vector<unsigned long long> deg_seq;
    vector<int> out_links;
    vector<long double> out_w;
    data->gplain.binary_to_mem(deg_seq, out_links, out_w, WEIGHTED);
    GraphBin g(deg_seq, out_links, out_w, WEIGHTED);
    init_quality(data, &g);
    data->nb_calls++;

    if (data->verbosity) {
        cout << "Computation of communities with the " << data->q->name
        << " quality function" << endl;
    }
    Louvain c(-1, data->precision, data->q);

    bool improvement = true;

    long double quality = (c.qual)->quality();
    long double new_qual;

    int level = 0;

    do {
        if (data->verbosity) {
            cout << "level " << level << ":\n";
            cout << "  network size: " << (c.qual)->g.nb_nodes << " nodes, " << (c.qual)->g.nb_links
                 << " links, " << (c.qual)->g.total_weight << " weight" << endl;
        }

        improvement = c.one_level();
        new_qual = (c.qual)->quality();

        data->levels.push_back(vector<int>());
        c.display_partition(&(data->levels[level]));

        g = c.partition2graph_binary();
        init_quality(data, &g);
        data->nb_calls++;

        c = Louvain(-1, data->precision, data->q);

        if (data->verbosity) {
            cout << "  quality increased from " << quality << " to " << new_qual << endl;
        }

        quality = new_qual;
        level++;
    } while (improvement);

    if (data->verbosity) {
        cout << "Quality: " << new_qual << endl;
    }
}


DLL_PUBLIC void Communities::set_precision(long double precision)
{
    data->precision = precision;
}

DLL_PUBLIC std::vector<std::pair<unsigned int, int> > Communities::get_mapping()
{
    std::vector<std::pair<unsigned int, int> > ret;
    vector<int> n2c(data->levels[0].size());

    for (unsigned int i = 0; i < data->levels[0].size(); i++)
        n2c[i] = i;

    for (unsigned l = 0; l < data->levels.size(); l++) {
        for (unsigned int node = 0; node < data->levels[0].size(); node++) {
            n2c[node] = data->levels[l][n2c[node]];
        }
    }

    for (unsigned int node = 0; node < data->levels[0].size(); node++) {
        ret.push_back(std::make_pair(node, n2c[node]));
    }

    return ret;
}


DLL_PUBLIC const char* Communities::get_version()
{
    return get_version_sha1();
}

DLL_PUBLIC void LouvainC::Communities::set_verbosity(unsigned int verb)
{
    data->verbosity = verb;
}


}
