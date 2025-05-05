#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct Haddock Haddock;
typedef struct HaddockVar HaddockVar;
typedef struct MDDHandle MDDHandle;

struct CMDDNode {
    unsigned int layer;
    unsigned int node_index;
};

typedef struct CMDDNode CMDDNode;

struct CMDDEdge {
    CMDDNode from;
    CMDDNode to;
    int value;
};

typedef struct CMDDEdge CMDDEdge;

struct CMDDGraph {
   HaddockVar** variables;
   int n_variables;
   CMDDEdge* edges;
   int n_edges;
   CMDDNode sink;
};

typedef struct CMDDGraph CMDDGraph;

Haddock* init_haddock();
void release_haddock(Haddock*);
HaddockVar* add_variable(Haddock*, int, int);

void impose_linear(Haddock*, MDDHandle*, HaddockVar**, const int*, size_t, int, int);
void impose_alldiff(Haddock*, MDDHandle*, HaddockVar**, size_t);

MDDHandle* init_mdd(Haddock*, size_t);
CMDDGraph post_mdd(Haddock*, MDDHandle*);
void release_mdd(MDDHandle*);

#ifdef __cplusplus
}
#endif