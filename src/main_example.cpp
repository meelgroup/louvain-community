// File: main_louvain.cpp
// -- community detection, sample main file
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

#include <unistd.h>
#include "commlouvain/graph_binary.h"
#include "commlouvain/graph.h"
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

int type = UNWEIGHTED;
long double precision = 0.000001L;
int display_level = -2;
unsigned short id_qual = 0;
long double alpha = 0.5L;
int kmin = 1;
long double sum_se = 0.0L;
long double sum_sq = 0.0L;
long double max_w = 1.0L;
Quality *q;
bool verbose = true;

using namespace std;

void print_final(vector<vector<int>>& levels)
{
    vector<int> n2c(levels[0].size());

    for (unsigned int i = 0; i < levels[0].size(); i++)
        n2c[i] = i;

    for (unsigned l = 0; l < levels.size(); l++) {
        for (unsigned int node = 0; node < levels[0].size(); node++) {
            n2c[node] = levels[l][n2c[node]];
        }
    }

    for (unsigned int node = 0; node < levels[0].size(); node++) {
        cout << node << " " << n2c[node] << endl;
    }
}

int main(int /*argc*/, char ** /*argv*/)
{
    srand(0);

    time_t time_begin, time_end;
    time(&time_begin);

    unsigned short nb_calls = 0;

    GraphPlain gplain("graph.txt", WEIGHTED);
    Graph g("graph.bin", "graph.weights", WEIGHTED);
    q = new Modularity(g);
    nb_calls++;

    cerr << "Computation of communities with the " << q->name << " quality function" << endl;
    Louvain c(-1, precision, q);

    bool improvement = true;

    long double quality = (c.qual)->quality();
    long double new_qual;

    int level = 0;
    vector<vector<int> > levels;

    do {
        cerr << "level " << level << ":\n";
        cerr << "  network size: " << (c.qual)->g.nb_nodes << " nodes, " << (c.qual)->g.nb_links
             << " links, " << (c.qual)->g.total_weight << " weight" << endl;

        improvement = c.one_level();
        new_qual = (c.qual)->quality();

        levels.push_back(vector<int>());
        c.display_partition(&(levels[level]));

        g = c.partition2graph_binary();
        delete q;
        q = new Modularity(g);
        nb_calls++;

        c = Louvain(-1, precision, q);

        cerr << "  quality increased from " << quality << " to " << new_qual << endl;

        quality = new_qual;
        level++;
    } while (improvement);

    time(&time_end);
    cerr << "Total duration: " << (time_end - time_begin) << " sec" << endl;
    cerr << new_qual << endl;
    print_final(levels);

    delete q;
}
