// File: graph.cpp
// -- simple graph handling source file
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

#include "graph_plain.h"

using namespace std;

GraphPlain::GraphPlain()
{
}

void GraphPlain::add_edge(uint32_t src, uint32_t dest, long double weight)
{
    if (links.size() <= max(src, dest) + 1) {
        links.resize(max(src, dest) + 1);
    }

    links[src].push_back(make_pair(dest, weight));
    if (src != dest) {
        links[dest].push_back(make_pair(src, weight));
    }
}


GraphPlain::GraphPlain(const char *filename, int type)
{
    ifstream finput;
    finput.open(filename, fstream::in);
    if (finput.is_open() != true) {
        cerr << "The file " << filename << " does not exist" << endl;
        exit(EXIT_FAILURE);
    }

    unsigned long long nb_links = 0ULL;

    while (!finput.eof()) {
        unsigned int src, dest;
        long double weight = 1.0L;

        if (type == WEIGHTED) {
            finput >> src >> dest >> weight;
        } else {
            finput >> src >> dest;
        }

        if (finput) {
            if (links.size() <= max(src, dest) + 1) {
                links.resize(max(src, dest) + 1);
            }

            links[src].push_back(make_pair(dest, weight));
            if (src != dest)
                links[dest].push_back(make_pair(src, weight));

            nb_links += 1ULL;
        }
    }

    finput.close();
}

void GraphPlain::renumber(int type, char *filename)
{
    vector<int> linked(links.size(), -1);
    vector<int> renum(links.size(), -1);
    int nb = 0;

    ofstream foutput;
    foutput.open(filename, fstream::out);

    for (unsigned int i = 0; i < links.size(); i++) {
        if (links[i].size() > 0)
            linked[i] = 1;
    }

    for (unsigned int i = 0; i < links.size(); i++) {
        if (linked[i] == 1) {
            renum[i] = nb++;
            foutput << i << " " << renum[i] << endl;
        }
    }

    for (unsigned int i = 0; i < links.size(); i++) {
        if (linked[i] == 1) {
            for (unsigned int j = 0; j < links[i].size(); j++) {
                links[i][j].first = renum[links[i][j].first];
            }
            links[renum[i]] = links[i];
        }
    }
    links.resize(nb);
}

void GraphPlain::clean(int type)
{
    for (unsigned int i = 0; i < links.size(); i++) {
        map<int, long double> m;
        map<int, long double>::iterator it;

        for (unsigned int j = 0; j < links[i].size(); j++) {
            it = m.find(links[i][j].first);
            if (it == m.end())
                m.insert(make_pair(links[i][j].first, links[i][j].second));
            else if (type == WEIGHTED)
                it->second += links[i][j].second;
        }

        vector<pair<int, long double> > v;
        for (it = m.begin(); it != m.end(); it++)
            v.push_back(*it);
        links[i].clear();
        links[i] = v;
    }
}

void GraphPlain::display(int type)
{
    for (unsigned int i = 0; i < links.size(); i++) {
        for (unsigned int j = 0; j < links[i].size(); j++) {
            int dest = links[i][j].first;
            long double weight = links[i][j].second;
            if (type == WEIGHTED)
                cout << i << " " << dest << " " << weight << endl;
            else
                cout << i << " " << dest << endl;
        }
    }
}

void GraphPlain::binary_to_mem(
    vector<unsigned long long>& out_deg_seq,
    vector<int>& out_links,
    vector<long double>& out_w,
    int type)
{
    // outputs cumulative degree sequence
    unsigned long long tot = 0ULL;
    for (size_t i = 0; i < links.size(); i++) {
        tot += (unsigned long long)links[i].size();
        out_deg_seq.push_back(tot);
    }

    // outputs links
    for (size_t i = 0; i < links.size(); i++) {
        for (unsigned int j = 0; j < links[i].size(); j++) {
            int dest = links[i][j].first;
            out_links.push_back(dest);
        }
    }

    // outputs weights
    if (type == WEIGHTED) {
        for (size_t i = 0; i < links.size(); i++) {
            for (unsigned int j = 0; j < links[i].size(); j++) {
                long double weight = links[i][j].second;
                out_w.push_back(weight);
            }
        }
    }

}

void GraphPlain::display_binary(const char *filename, const char *filename_w, int type)
{
    ofstream foutput;
    foutput.open(filename, fstream::out | fstream::binary);

    int s = links.size();

    // outputs number of nodes
    foutput.write((char *)(&s), sizeof(int));

    // outputs cumulative degree sequence
    unsigned long long tot = 0ULL;
    for (int i = 0; i < s; i++) {
        tot += (unsigned long long)links[i].size();
        foutput.write((char *)(&tot), sizeof(unsigned long long));
    }

    // outputs links
    for (int i = 0; i < s; i++) {
        for (unsigned int j = 0; j < links[i].size(); j++) {
            int dest = links[i][j].first;
            foutput.write((char *)(&dest), sizeof(int));
        }
    }
    foutput.close();

    // outputs weights in a separate file
    if (type == WEIGHTED) {
        ofstream foutput_w;
        foutput_w.open(filename_w, fstream::out | fstream::binary);
        for (int i = 0; i < s; i++) {
            for (unsigned int j = 0; j < links[i].size(); j++) {
                long double weight = links[i][j].second;
                foutput_w.write((char *)(&weight), sizeof(long double));
            }
        }
        foutput_w.close();
    }
}
