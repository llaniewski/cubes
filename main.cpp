#include <stdio.h>
#include <set>
#include <vector>
#include <array>
#include <iterator>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <assert.h>
const int M = 4;

#define TYPE_SET 1
#define TYPE_VECTOR 2
#define TYPE TYPE_VECTOR

typedef signed char crd_t;
const crd_t INF = 100;
typedef std::array<crd_t,3> coords_t;
#if TYPE == TYPE_SET
typedef std::set<coords_t> cubes_t;
#elif TYPE == TYPE_VECTOR
typedef std::vector<coords_t> cubes_t;
#endif
typedef std::set<cubes_t> setofcubes_t;
typedef std::map<cubes_t,int> mapofcubes_t;
//typedef std::unordered_map<cubes_t,int> mapofcubes_t;
typedef std::array<int,M+1> stats_t;

namespace std {
  template <> struct hash<cubes_t>
  {
    size_t operator()(const cubes_t & shape) const
    {
      /* your code here, e.g. "return hash<int>()(x.value);" */
      size_t w = 123;
      for (const coords_t& x : shape) {
        w += hash<int>()(static_cast<int>(x[0]) + (static_cast<int>(x[1])<<8) + (static_cast<int>(x[2])<<16));
      }
      return w;
    }
  };
}


mapofcubes_t all;
std::vector<cubes_t> uniq;
stats_t stats;
int total=0;

void print(const coords_t& x) {
    printf("(%2d,%2d,%2d) ",x[0],x[1],x[2]);
}

void print(const cubes_t& shape) {
    for (const coords_t& x : shape) print(x);
    printf("\n");
}

void print(const stats_t& vec) {
    for (const int& x : vec) printf("%5d ",x);
    printf("\n");
}

void print(const setofcubes_t& s) {
    for (const cubes_t& shape : s) print(shape);
}


cubes_t normalize(const cubes_t& shape) {
    coords_t mins={INF,INF,INF};
    for (const coords_t x : shape) {
        for (int i=0;i<3;i++) if (x[i]<mins[i]) mins[i] = x[i];
    }
    cubes_t ret;
    #if TYPE == TYPE_VECTOR
        ret.reserve(shape.size());
    #endif
    for (const coords_t x : shape) {
        coords_t n;
        for (int i=0;i<3;i++) n[i] = x[i]-mins[i];
        #if TYPE == TYPE_SET
            ret.insert(n);
        #elif TYPE == TYPE_VECTOR
            ret.push_back(n);
        #endif
    }
    #if TYPE == TYPE_VECTOR
        std::sort(ret.begin(),ret.end());
    #endif
    return ret;
}

const std::array<coords_t,6> perms = {{
    { 0, 1, 2},
    { 0, 2, 1},
    { 2, 0, 1},
    { 2, 1, 0},
    { 1, 2, 0},
    { 1, 0, 2}
}};

void addrot(const cubes_t& shape) {
    //print(shape);
    int idx = total;
    total++;
    uniq.push_back(shape);
    stats[shape.size()]++;
    if (total % 10000 == 0) { printf("%10d -> ",total); print(stats); }
    int par=0;
    for (const coords_t& perm : perms){
        int a=perm[0];
        int b=perm[1];
        int c=perm[2];
        for (int p=0; p<8; p++) {
            int npar = par + (p & 1 ? 1 : 0) + (p & 2 ? 1 : 0) + (p & 4 ? 1 : 0);
            //int npar=0;
            if (npar % 2 == 0) {
                cubes_t ret;
                #if TYPE == TYPE_VECTOR
                    ret.reserve(shape.size());
                #endif
                for (const coords_t& x : shape) {
                    coords_t n;
                    if (p & 1) n[0] = -x[a]; else n[0] = x[a];
                    if (p & 2) n[1] = -x[b]; else n[1] = x[b];
                    if (p & 4) n[2] = -x[c]; else n[2] = x[c];
                    #if TYPE == TYPE_SET
                        ret.insert(n);
                    #elif TYPE == TYPE_VECTOR
                        ret.push_back(n);
                    #endif
                }
                ret = normalize(ret);
                {
                    auto it = all.find(ret);
                    if (it != all.end()) {
                        if (it->second != idx) {
                            printf("%d %d %d %d\n",a,b,c,p);
                            print(shape);
                            print(ret);
                            print(it->first);
                            print(uniq[idx]);
                            print(uniq[it->second]);                                    
                            //printf("%d %d\n", it->second, idx);
                            exit(-1);
                        }
                    }
                }
                all.insert(std::make_pair(ret, idx));
            }
        }
        par++;
    }
}

void children(const cubes_t& shape) {
    if (shape.size() >= M) return;
    for (const coords_t& x : shape) {
        for (int j=0;j<6;j++) {
            coords_t n = x;
            cubes_t ret = shape;
            n[j>>1] += ((j & 1) ? 1 : -1);
            #if TYPE == TYPE_SET
                ret.insert(n);
                if (ret == shape) continue;
            #elif TYPE == TYPE_VECTOR
                if (std::binary_search(ret.begin(),ret.end(),n)) continue;
                ret.push_back(n);
            #endif
            ret = normalize(ret);
            if (all.find(ret) != all.end()) continue;
            addrot(ret);
            children(ret);
        }
    }
}


int main () {
    cubes_t b{{0,0,0}};
    addrot(b);
    children(b);
    //print(uniq);
    print(stats);

    for (const cubes_t shape : uniq) {
        std::array<int,3> mean;
        for (int i=0;i<3;i++) mean[i] = 0;
        for (const coords_t& x : shape) {
            for (int i=0;i<3;i++) mean[i] += x[i];
        }
        std::array<int,9> var;
        for (int i=0;i<9;i++) var[i] = 0;
        for (const coords_t& x : shape) {
            for (int i=0;i<3;i++) for (int j=0;j<3;j++) var[i+3*j] += ((int) x[i])*((int) x[j]);
        }
        for (int i=0;i<3;i++) for (int j=0;j<3;j++) var[i+3*j] = var[i+3*j]*shape.size() - mean[i]*mean[j];
        /* 
        printf("mean = [ %d %d %d ]\n",mean[0],mean[1],mean[2]);
        printf("       | %d %d %d |\n",var[0],var[1],var[2]);
        printf("var =  | %d %d %d |\n",var[3],var[4],var[5]);
        printf("       | %d %d %d |\n",var[6],var[7],var[8]);
        */
        printf("tr var = %d\n",var[0]+var[4]+var[8]);
    }

    return 0;
}
