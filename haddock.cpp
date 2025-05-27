#include "mddConstraints.hpp"
#include "mddrelax.hpp"
#include "solver.hpp"

#include <iostream>
#include <tuple>

extern "C" {
    #include "haddock.h"

    Haddock* init_haddock() {
        auto solver = Factory::makeSolver();
        return reinterpret_cast<Haddock*>(solver.get());
    }

    void release_haddock(Haddock* haddock_handle) {
        delete haddock_handle;
    }

    HaddockVar* add_variable(Haddock* haddock_handle, int lb, int ub) {
        auto haddock = CPSolver::Ptr(reinterpret_cast<CPSolver*>(haddock_handle));
        auto var = Factory::makeIntVar(haddock, lb, ub);
        return reinterpret_cast<HaddockVar*>(var.get());
    }

    bool impose_linear(
        Haddock* haddock_handle,
        MDDHandle* mdd_handle,
        HaddockVar** vars, const int* weights, size_t n_vars,
        int lb, int ub) {
        auto mdd = MDD::Ptr(reinterpret_cast<MDDRelax*>(mdd_handle));
        auto haddock = CPSolver::Ptr(reinterpret_cast<CPSolver*>(haddock_handle));
        auto vars_vec = Factory::intVarArray(
            haddock,
            n_vars,
            [&vars](int index) {
                auto ptr = reinterpret_cast<var<int>*>(vars[index]);
                return var<int>::Ptr(ptr);
            }
        );
        auto weights_vec = std::vector<int> { };
        for (size_t index = 0; index < n_vars; ++index) {
            weights_vec.push_back(weights[index]);
        }
        try {
            auto cons = Factory::sum(mdd, vars_vec, weights_vec, lb, ub);
            mdd->post(cons);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool impose_alldiff(
        Haddock* haddock_handle,
        MDDHandle* mdd_handle,
        HaddockVar** vars, size_t n_vars) {
        auto mdd = MDD::Ptr(reinterpret_cast<MDDRelax*>(mdd_handle));
        auto haddock = CPSolver::Ptr(reinterpret_cast<CPSolver*>(haddock_handle));
        auto vars_vec = Factory::intVarArray(
            haddock,
            n_vars,
            [&vars](int index) {
                auto ptr = reinterpret_cast<var<int>*>(vars[index]);
                return var<int>::Ptr(ptr);
            }
        );
        try {
            mdd->post(Factory::allDiffMDD(vars_vec));
            return true;
        } catch (...) {
            return false;
        }
    }

    MDDHandle* init_mdd(Haddock* haddock_handle, size_t width) {
        auto haddock = CPSolver::Ptr(reinterpret_cast<CPSolver*>(haddock_handle));
        auto mdd = new MDDRelax(haddock, width = width);
        return reinterpret_cast<MDDHandle*>(mdd);
    }

    CMDDGraph post_mdd(Haddock* haddock_handle, MDDHandle* mdd_handle) {
        auto mdd = MDD::Ptr(reinterpret_cast<MDDRelax*>(mdd_handle));
        auto haddock = CPSolver::Ptr(reinterpret_cast<CPSolver*>(haddock_handle));
        CMDDGraph result {};
        try {
            haddock->post(mdd);
            result.success = true;
        } catch (...) {
            result.success = false;
        }
        auto g = mdd->returnGraph();
        result.n_variables = g.variables.size();
        result.variables = reinterpret_cast<HaddockVar**>(
            malloc(result.n_variables * sizeof(HaddockVar*))
        );
        for (size_t index = 0; index < result.n_variables; ++index) {
            result.variables[index] = reinterpret_cast<HaddockVar*>(g.variables[index].get());
        }
        result.n_edges = g.edges.size();
        result.edges = reinterpret_cast<CMDDEdge*>(
            malloc(result.n_edges * sizeof(CMDDEdge))
        );
        for (size_t index = 0; index < result.n_edges; ++index) {
            auto& edge = g.edges[index];
            result.edges[index].from.layer = std::get<0>(edge).first;
            result.edges[index].from.node_index = std::get<0>(edge).second;
            result.edges[index].to.layer = std::get<1>(edge).first;
            result.edges[index].to.node_index = std::get<1>(edge).second;
            result.edges[index].value = std::get<2>(edge);
        }
        result.sink.layer = g.sink.first;
        result.sink.node_index = g.sink.second;
        return result;
    }

    void release_mdd(MDDHandle* mdd_handle) {
        auto mdd = reinterpret_cast<MDDRelax*>(mdd_handle);
        delete mdd;
    }
}