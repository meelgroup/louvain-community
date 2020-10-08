#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2009-2020 Authors of CryptoMiniSat, see AUTHORS file
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; version 2
# of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.

import matplotlib.cm as cm
import matplotlib.pyplot as plt
import networkx as nx
import random
import community as community_louvain
#from networkx.algorithms import community
import time
import sys


def generate_graph():
    n = 500
    random.seed(1)
    pos = {i: (random.gauss(0.1, 2), random.gauss(0.9, 2)) for i in range(n)}
    G = nx.random_geometric_graph(n, 2, pos=pos)
    #G = nx.barbell_graph(50, 9)
    return G


def read_cnf(fname):
    cls = []
    maxvar = 0
    with open(fname, "r") as f:
        for l in f:
            l = l.strip()
            if len(l) == 0:
                continue

            if l[0] == "c" or l[0] == "p":
                continue

            #print(l)
            cl = []
            for l in l.split():
                lit = int(l)
                maxvar = max(abs(lit),maxvar)
                if lit != 0:
                    cl.append(lit)
            cls.append(cl)

    print("maxvar: ", maxvar, " numcls: ", len(cls))
    return maxvar, cls

def convert_cnf_to_vig_graph(maxvar, cls):
    w = {}
    for cl_orig in cls:
        cl = [abs(x) for x in cl_orig]
        cl.sort()
        for i1 in range(len(cl)):
            for i2 in range(i1+1, len(cl)):
                x = abs(cl[i1])
                y = abs(cl[i2])
                name = "%d,%d" % (x,y)
                amount = 1.0/(len(cl)*(len(cl)-1.0)/2.0)
                if name not in w:
                    w[name] = amount
                else:
                    w[name]+= amount

    G = nx.Graph()
    for a,b in w.items():
        vs = [int(x) for x in a.split(",")]
        assert len(vs) == 2
        G.add_edge(vs[0], vs[1], weight=b)

    return G

def print_graph(G):
    with open("graph.txt", "w") as f:
        for a, b, weight in G.edges.data("weight"):
            f.write("%d  %d %f\n" % (a,b, weight))

    with open("graph_unw.txt", "w") as f:
        for a, b in G.edges:
            f.write("%d  %d\n" % (a,b))


def compute_best_part(G, show=True):
    T = time.time()
    # compute the best partition
    if True:
        partition = community_louvain.best_partition(G, random_state=1)
    else:
        partition = community.greedy_modularity_communities(G)
        #partition = community.asyn_lpa_communities(G)

        print ("partition: ", partition)
        #for x in partition:
            #print ("x:", x)

        part = {}
        i = 0
        for p in partition:
            for x in p:
                part[x] = i
            i += 1

        print("part:", part)
        partition = part

    print("partition done. T:", time.time()-T)
    #print("partition: ", partition)
    return partition


def dump_partition(partition):
    with open("part", "w") as f:
        for var,part in partition.items():
            f.write("%d %d\n" % (var, part))

def draw_graph(G, partition):
    # draw the graph
    pos = nx.spring_layout(G)
    # color the nodes according to their partition
    cmap = cm.get_cmap('viridis', max(partition.values()) + 1)
    nx.draw_networkx_nodes(G, pos, partition.keys(), node_size=40,
                       cmap=cmap, node_color=list(partition.values()))
    nx.draw_networkx_edges(G, pos, alpha=0.5)
    plt.show()

# ***** test
#maxvar = 10
#cls = [[-1, 2, 3], [4, 5, 6, 7], [1,2]]
#convert_cnf_to_vig_graph(maxvar, cls)


if len(sys.argv) < 2:
    print("ERROR: file not given")
    exit(-1)

fname = sys.argv[1]
maxvar, cls = read_cnf(fname)
G = convert_cnf_to_vig_graph(maxvar, cls)
print_graph(G)
partition = compute_best_part(G)
dump_partition(partition)
#draw_graph(G, partition)
exit(0)

#G = generate_graph()

