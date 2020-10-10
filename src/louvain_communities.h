/******************************************
Copyright (C) 2009-2020 Authors of CryptoMiniSat, see AUTHORS file

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

#ifndef LOUVAIN_COMMUNITIES_H__
#define LOUVAIN_COMMUNITIES_H__

#if defined _WIN32
    #define DLL_PUBLIC __declspec(dllexport)
#else
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
    #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#endif

#include <vector>
#include <utility>

namespace LouvainC {
    struct PrivateData;
    #ifdef _WIN32
    class __declspec(dllexport) Communities
    #else
    class Communities
    #endif
    {
    public:
        Communities();
        ~Communities();
//             The quality function used to compute partition of the graph (modularity is chosen by default)
//             id = 0\t -> the classical Newman-Girvan criterion (also called "Modularity"
//             id = 1\t -> the Zahn-Condorcet criterion
//             id = 2\t -> the Owsinski-Zadrozny criterion (you should specify
//                              the value of the parameter with option -c)
//             id = 3\t -> the Goldberg Density criterion
//             id = 4\t -> the A-weighted Condorcet criterion
//             id = 5\t -> the Deviation to Indetermination criterion
//             id = 6\t -> the Deviation to Uniformity criterion
//             id = 7\t -> the Profile Difference criterion
//             id = 8\t -> the Shi-Malik criterion (you should specify the value of kappa_min with option -k)
//             id = 9\t -> the Balanced Modularity criterion
        void set_quality_type(unsigned id = 0);
        void add_edge(unsigned src, unsigned dst, double weight = 1.0);
        void calculate(bool weighted = false);
        const char* get_version();
        void set_verbosity(unsigned verb);
        void set_precision(long double precision);
        std::vector<std::pair<unsigned int, int>> get_mapping();
    };

    PrivateData* data;
}

#endif
