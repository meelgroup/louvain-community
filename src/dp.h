// File: dp.h
// -- quality functions (for Profile Difference criterion) header file
//-----------------------------------------------------------------------------
// Community detection
// Based on the article "Fast unfolding of community hierarchies in large networks"
// Copyright (C) 2008 V. Blondel, J.-L. Guillaume, R. Lambiotte, E. Lefebvre
//
// And based on the article
// Copyright (C) 2013 R. Campigotto, P. Conde Céspedes, J.-L. Guillaume
//
// This file is part of Louvain algorithm.
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
// Author   : E. Lefebvre, adapted by J.-L. Guillaume and R. Campigotto
// Email    : jean-loup.guillaume@lip6.fr
// Location : Paris, France
// Time	    : July 2013
//-----------------------------------------------------------------------------
// see README.txt for more details

#ifndef LOUVAIN_DP_H
#define LOUVAIN_DP_H

#include "quality.h"

using namespace std;

class DP : public Quality
{
   public:
    vector<long double> in; // used to compute the quality participation of each community
    vector<int> w;          // used to store size of communities
    int kappa;              // number of communities

    long double sum_sq; // used to store Sum Âij^2
    long double max;

    DP(GraphBin &gr, long double sum, long double max_w);
    ~DP();

    // change the weight of each link ij in the graph, from Aij to 2Aij / (d(i)+d(j))
    // return the result of Sum Âij^2
    static long double graph_weighting(GraphBin *g);

    inline void remove(int node, int comm, long double dnodecomm);

    inline void insert(int node, int comm, long double dnodecomm);

    inline long double gain(int node, int comm, long double dnodecomm, long double w_degree);

    long double quality();
};

inline void DP::remove(int node, int comm, long double dnodecomm)
{
    assert(node >= 0 && node < size);

    in[comm] -= 2.0L * dnodecomm + g.nb_selfloops(node);

    w[comm] -= g.nodes_w[node];

    if (w[comm] == 0)
        kappa--;

    n2c[node] = -1;
}

inline void DP::insert(int node, int comm, long double dnodecomm)
{
    assert(node >= 0 && node < size);

    in[comm] += 2.0L * dnodecomm + g.nb_selfloops(node);

    if (w[comm] == 0)
        kappa++;

    w[comm] += g.nodes_w[node];

    n2c[node] = comm;
}

inline long double DP::gain(int node, int comm, long double dnc, long double /*degc*/)
{
    assert(node >= 0 && node < size);

    long double inc = in[comm];
    long double self = g.nb_selfloops(node);
    long double wc = (long double)w[comm];
    long double wu = (long double)g.nodes_w[node];

    long double gain;

    if (wc == 0.0L) {
        gain = (2.0L * dnc + self) / wu;
        gain -= 1.0L / 2.0L;
    } else {
        gain = (inc + 2.0L * dnc + self) / (wc + wu);
        gain -= inc / wc;
    }

    return gain;
}

#endif // LOUVAIN_DP_H
