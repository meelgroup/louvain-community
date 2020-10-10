// File: graph.h
// -- simple graph handling header file
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

#ifndef LOUVAIN_GRAPHPLAIN_H
#define LOUVAIN_GRAPHPLAIN_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#define WEIGHTED 0
#define UNWEIGHTED 1

using namespace std;

class GraphPlain
{
   public:
    vector<vector<pair<int, long double> > > links;

    GraphPlain();
    GraphPlain(const char *filename, int type);

    void add_edge(uint32_t src, uint32_t dst, long double weight = 1.0L);
    void clean(int type);
    void renumber(int type, char *filename);
    void display(int type);
    void display_binary(const char *filename, const char *filename_w, int type);
    void binary_to_mem(
        vector<uint64_t>& out_deg_seq,
        vector<int>& out_links,
        vector<long double>& out_w,
        int type);
};

#endif // LOUVAIN_GRAPHPLAIN
