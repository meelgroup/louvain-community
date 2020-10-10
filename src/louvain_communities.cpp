// Community detection
// Copyright by Mate Soos, based on the work below.
//
// Based on the article "Fast unfolding of community hierarchies in large networks"
// Copyright (C) 2008 V. Blondel, J.-L. Guillaume, R. Lambiotte, E. Lefebvre
//
// And based on the article
// Copyright (C) 2013 R. Campigotto, P. Conde Céspedes, J.-L. Guillaume
//
// Louvain algorithm is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Louvain algorithm is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Louvain algorithm.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------
// Authors   : E. Lefebvre, adapted by J.-L. Guillaume and R. Campigotto, Mate Soos
//-----------------------------------------------------------------------------
// see README.txt for more details

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
    PrivateData()
    {
        unsigned seed = 0;
        mtrand.seed(seed);
    }
    ~PrivateData()
    {
        delete q;
    }
    GraphPlain gplain;
    unsigned short nb_calls = 0;
    long double precision = 0.000001L;
    uint32_t verbosity = 0;
    vector<vector<int>> levels;
    MTRand mtrand;

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

DLL_PUBLIC void Communities::set_random_seed(unsigned seed)
{
    data->mtrand.seed(seed);
}

DLL_PUBLIC void Communities::add_edge(unsigned int src, unsigned int dst, long double weight)
{
    data->gplain.add_edge(src, dst, weight);
}

DLL_PUBLIC void Communities::set_sum_se(long double sum_se)
{
    assert(data->id_qual == 4);
    data->sum_se = sum_se;
}

DLL_PUBLIC void Communities::set_kmin(int kmin)
{
    assert(data->id_qual == 8);
    data->kmin = kmin;
}

DLL_PUBLIC void Communities::set_max_w(long double max_w)
{
    assert(
        data->id_qual == 1 ||
        data->id_qual == 2 ||
        data->id_qual == 3 ||
        data->id_qual == 7 ||
        data->id_qual == 9
    );
    data->max_w = max_w;
}

DLL_PUBLIC void Communities::set_alpha(long double alpha)
{
    assert(data->id_qual == 2);
    data->alpha = alpha;
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
    data->gplain.binary_to_mem(deg_seq, out_links, out_w, weighted ? WEIGHTED : UNWEIGHTED);
    GraphBin g(deg_seq, out_links, out_w, weighted ? WEIGHTED : UNWEIGHTED);
    init_quality(data, &g);
    data->nb_calls++;

    if (data->verbosity) {
        cout << "Computation of communities with the " << data->q->name
        << " quality function" << endl;
    }
    Louvain* c = new Louvain(-1, data->precision, data->q, data->mtrand);

    bool improvement = true;

    long double quality = (c->qual)->quality();
    long double new_qual;

    int level = 0;

    do {
        if (data->verbosity) {
            cout << "level " << level << ":\n";
            cout << "  network size: " << (c->qual)->g.nb_nodes << " nodes, " << (c->qual)->g.nb_links
                 << " links, " << (c->qual)->g.total_weight << " weight" << endl;
        }

        improvement = c->one_level();
        new_qual = (c->qual)->quality();

        data->levels.push_back(vector<int>());
        c->display_partition(&(data->levels[level]));

        g = c->partition2graph_binary();
        init_quality(data, &g);
        data->nb_calls++;

        delete c;
        c = new Louvain(-1, data->precision, data->q, data->mtrand);

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
