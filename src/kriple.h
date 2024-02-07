#include <iostream>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "graph.h"

class Kripke : public DiGraph {
   public:
    Kripke(const std::unordered_set<int>& S, const std::unordered_set<int>& S0,
           const std::vector<std::pair<int, int>>& R,
           std::unordered_map<int, std::unordered_set<std::string>> L)
        : DiGraph(S, R), S0(S0) {
        std::vector<int> vecN;
        nodes(vecN);
        for (int state : vecN) {
            if (L.find(state) != L.end()) {
                _labels[state] = L[state];
            } else {
                _labels[state] = std::unordered_set<std::string>();
            }
        }
    }

    std::unordered_map<int, std::unordered_set<std::string>>
    labelling_function() const {
        return _labels;
    }

    std::unordered_map<int, std::unordered_set<std::string>>
    replace_labelling_function(
        const std::unordered_map<int, std::unordered_set<std::string>>& L) {
        auto old_L = _labels;
        _labels = L;
        std::vector<int> vecS;
        states(vecS);

        for (int s : vecS) {
            if (_labels.find(s) == _labels.end()) {
                _labels[s] = std::unordered_set<std::string>();
            }
        }

        return old_L;
    }

    std::unordered_set<std::string> labels(int state = -1) const {
        if (state != -1) {
            if (_next.find(state) == _next.end()) {
                throw std::runtime_error(
                    "State not found in the Kripke structure");
            }
            return _labels.at(state);
        }

        std::unordered_set<std::string> AP;
        for (const auto& ap : _labels) {
            AP.insert(ap.second.begin(), ap.second.end());
        }

        return AP;
    }

    void states(std::vector<int>& result) const { return nodes(result); }

    void next(int src, std::unordered_set<int>& result) const {
        try {
            return DiGraph::next(src, result);
        } catch (const std::exception&) {
            throw std::runtime_error(
                "Source state not found in the Kripke structure");
        }
    }

    void transitions(std::vector<std::pair<int, int>>& result) const {
        return edges(result);
    }

    // std::vector<std::pair<int, int>> transitions_iter() const {
    //    return edges_iter();
    // }

    Kripke clone() const {
        std::unordered_map<int, std::unordered_set<std::string>> L;
        for (const auto& entry : _labels) {
            L[entry.first] = entry.second;
        }

        std::vector<int> vecS;
        states(vecS);
        std::unordered_set<int> setS(vecS.begin(), vecS.end());
        std::vector<std::pair<int, int>> T;
        transitions(T);

        return Kripke(setS, S0, T, L);
    }

    Kripke get_substructure(const std::unordered_set<int>& V) const {
        std::vector<int> vecS;
        states(vecS);
        std::unordered_set<int> setS(vecS.begin(), vecS.end());

        std::unordered_set<int> S;
        for (int i : setS) {
            if (V.find(i) != V.end()) {
                S.insert(i);
            }
        }

        std::unordered_set<int> S0_sub;
        for (int i : S0) {
            if (V.find(i) != V.end()) {
                S0_sub.insert(i);
            }
        }

        std::vector<std::pair<int, int>> E;
        std::vector<std::pair<int, int>> Ts;
        transitions(Ts);
        for (const auto& edge : Ts) {
            if (S.find(edge.first) != S.end() &&
                S.find(edge.second) != S.end()) {
                E.push_back(edge);
            }
        }

        std::unordered_map<int, std::unordered_set<std::string>> L;
        for (const auto& entry : _labels) {
            if (S.find(entry.first) != S.end()) {
                L[entry.first] = entry.second;
            }
        }

        return Kripke(S, S0_sub, E, L);
    }

    std::unordered_set<int> get_fair_states(
        const std::vector<std::unordered_set<int>>& F) const {
        std::unordered_set<int> F_set;
        std::vector<std::unordered_set<int>> sccs;
        compute_SCCs(*this, sccs);
        for (const auto& SCC : sccs) {
            if (is_a_fair_SCC(SCC, F)) {
                F_set.insert(SCC.begin(), SCC.end());
            }
        }

        DiGraph R_graph = get_reversed_graph();

        return R_graph.get_reachable_set_from(F_set);
    }

    std::string label_fair_states(
        const std::vector<std::unordered_set<int>>& F) {
        std::string f_label = "fair";
        int i = 0;
        std::unordered_set<std::string> aps = labels();
        while (aps.find(f_label) != aps.end()) {
            f_label = "fair" + std::to_string(i);
            i++;
        }

        std::unordered_set<int> fair_states = get_fair_states(F);
        for (int s : fair_states) {
            _labels[s].insert(f_label);
        }

        return f_label;
    }

    /*
    std::string to_string() const {
        return "(S=" + set_to_string(states()) + ",S0=" + set_to_string(S0) +
               ",R=" + set_of_pairs_to_string(transitions()) +
               ",L=" + map_to_string(_labels) + ")";
    }
    */

   private:
    std::unordered_set<int> S0;
    std::unordered_map<int, std::unordered_set<std::string>> _labels;

    bool is_a_fair_SCC(const std::unordered_set<int>& scc,
                       const std::vector<std::unordered_set<int>>& F) const {
        int v = *(scc.begin());
        std::unordered_set<int> next_v;
        next(v, next_v);
        if (scc.size() == 1 || next_v.find(v) == next_v.end()) {
            return false;
        }

        for (const auto& P : F) {
            std::unordered_set<int> tmp;
            for (int i : scc) {
                if (P.begin(i) != P.end()) {
                    tmp.insert(i);
                }
            }
            if (tmp.empty()) {
                return false;
            }
        }

        return true;
    }
};
