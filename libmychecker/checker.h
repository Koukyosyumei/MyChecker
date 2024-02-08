#include <iterator>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "formula.h"
#include "graph.h"
#include "kripke.h"

void _checkNot(Kripke &kripke, std::shared_ptr<Formula> formula,
               std::unordered_map<std::string, std::unordered_set<int>> &L);
void _checkOr(Kripke &kripke, std::shared_ptr<Formula> formula,
              std::unordered_map<std::string, std::unordered_set<int>> &L);
void _checkAP(Kripke &kripke, std::shared_ptr<Formula> formula,
              std::unordered_map<std::string, std::unordered_set<int>> &L);
void _checkEG(Kripke &kripke, std::shared_ptr<Formula> formula,
              std::unordered_map<std::string, std::unordered_set<int>> &L);
void _checkEU(Kripke &kripke, std::shared_ptr<Formula> formula,
              std::unordered_map<std::string, std::unordered_set<int>> &L);
void _checkEX(Kripke &kripke, std::shared_ptr<Formula> formula,
              std::unordered_map<std::string, std::unordered_set<int>> &L);
void _checkStateFormula(
    Kripke &kripke, std::shared_ptr<Formula> formula,
    std::unordered_map<std::string, std::unordered_set<int>> &L);

inline void modelcheck(
    Kripke &kripke, std::shared_ptr<Formula> formula,
    std::unordered_map<std::string, std::unordered_set<int>> &L,
    std::vector<std::unordered_set<int>> &F) {
    if (F.size() != 0) {
        std::string fair_label = kripke.label_fair_states(F);
        formula = formula->get_equivalent_non_fair_formula(
            std::make_shared<CTL::AtomicProposition>(fair_label));
    }

    return _checkStateFormula(kripke, formula, L);
}

inline void _checkStateFormula(
    Kripke &kripke, std::shared_ptr<Formula> formula,
    std::unordered_map<std::string, std::unordered_set<int>> &L) {
    switch (formula->opcode) {
        case (OpCode::Not): {
            return _checkNot(kripke, formula, L);
        }
        case (OpCode::Or): {
            return _checkOr(kripke, formula, L);
        }
        case (OpCode::Bool): {
            if (formula->str() == "true") {
                std::vector<int> _Lformula;
                kripke.states(_Lformula);
                std::unordered_set<int> Lformula(_Lformula.begin(),
                                                 _Lformula.end());
                L["true"] = Lformula;
            } else {
                L["false"] = {};
            }
            return;
        }
        case (OpCode::Atomic): {
            return _checkAP(kripke, formula, L);
        }
        case (OpCode::E): {
            switch (formula->subformulas[0]->opcode) {
                case (OpCode::G): {
                    return _checkEG(kripke, formula, L);
                }
                case (OpCode::U): {
                    return _checkEU(kripke, formula, L);
                }
                case (OpCode::X): {
                    return _checkEX(kripke, formula, L);
                }
            }
        }
    }

    std::shared_ptr<Formula> restr_f =
        formula->get_equivalent_restricted_formula();
    _checkStateFormula(kripke, restr_f, L);
}

inline void _checkAP(
    Kripke &kripke, std::shared_ptr<Formula> formula,
    std::unordered_map<std::string, std::unordered_set<int>> &L) {
    std::string s = formula->str();
    if (L.find(s) == L.end()) {
        std::unordered_set<int> Lformula;
        L.emplace(s, Lformula);

        std::vector<int> states;
        kripke.states(states);

        for (int v : states) {
            std::unordered_set<std::string> tmp_l = kripke.labels(v);
            if (tmp_l.find(s) != tmp_l.end()) {
                L[s].insert(v);
            }
        }
    }
}

inline void _checkNot(
    Kripke &kripke, std::shared_ptr<Formula> formula,
    std::unordered_map<std::string, std::unordered_set<int>> &L) {
    std::string s = formula->str();
    if (L.find(s) == L.end()) {
        std::unordered_set<int> Lformula;
        L.emplace(s, Lformula);

        std::string s_phi = formula->subformulas[0]->str();
        _checkStateFormula(kripke, formula->subformulas[0], L);

        std::vector<int> states;
        kripke.states(states);

        for (int v : states) {
            if (L[s_phi].find(v) == L[s_phi].end()) {
                L[s].insert(v);
            }
        }
    }
}

inline void _checkOr(
    Kripke &kripke, std::shared_ptr<Formula> formula,
    std::unordered_map<std::string, std::unordered_set<int>> &L) {
    std::string s = formula->str();

    if (L.find(s) == L.end()) {
        std::unordered_set<int> Lformula;
        L.emplace(s, Lformula);

        for (std::shared_ptr<Formula> sf : formula->subformulas) {
            _checkStateFormula(kripke, sf, L);
            std::string sf_str = sf->str();
            for (int v : L[sf_str]) {
                L[s].insert(v);
            }
        }
    }
}

inline void _checkEX(
    Kripke &kripke, std::shared_ptr<Formula> formula,
    std::unordered_map<std::string, std::unordered_set<int>> &L) {
    std::string s = formula->str();

    if (L.find(s) == L.end()) {
        std::unordered_set<int> Lformula;
        L.emplace(s, Lformula);

        std::shared_ptr<Formula> target_formula =
            formula->subformulas[0]->subformulas[0];
        _checkStateFormula(kripke, target_formula, L);
        std::string t_str = target_formula->str();

        std::vector<std::pair<int, int>> transitions;
        kripke.transitions(transitions);

        for (std::pair<int, int> e : transitions) {
            if (L[t_str].find(e.second) != L[t_str].end()) {
                L[s].insert(e.first);
            }
        }
    }
}

inline void _checkEU(
    Kripke &kripke, std::shared_ptr<Formula> formula,
    std::unordered_map<std::string, std::unordered_set<int>> &L) {
    std::string s = formula->str();

    if (L.find(s) == L.end()) {
        std::unordered_set<int> Lformula;
        L.emplace(s, Lformula);

        std::shared_ptr<Formula> psi = formula->subformulas[0]->subformulas[0];
        std::shared_ptr<Formula> chi = formula->subformulas[0]->subformulas[1];
        std::string psi_str = psi->str();
        std::string chi_str = chi->str();

        _checkStateFormula(kripke, psi, L);
        _checkStateFormula(kripke, chi, L);

        std::unordered_set<int> T = L[chi_str];
        L[s] = L[chi_str];

        std::vector<std::pair<int, int>> transitions;
        kripke.transitions(transitions);

        while (!T.empty()) {
            int v = *T.begin();
            T.erase(T.begin());

            for (std::pair<int, int> e : transitions) {
                if (e.second == v) {
                    int t = e.first;
                    if (L[s].find(t) == L[s].end() &&
                        L[psi_str].find(t) == L[psi_str].end()) {
                        L[s].insert(t);
                        T.insert(t);
                    }
                }
            }
        }
    }
}

inline void _checkEG(
    Kripke &kripke, std::shared_ptr<Formula> formula,
    std::unordered_map<std::string, std::unordered_set<int>> &L) {
    std::string s = formula->str();

    if (L.find(s) == L.end()) {
        std::unordered_set<int> Lformula;
        L.emplace(s, Lformula);

        std::shared_ptr<Formula> phi = formula->subformulas[0]->subformulas[0];
        _checkStateFormula(kripke, phi, L);

        DiGraph subgraph = kripke.get_subgraph(L[phi->str()]);
        // subgraph = subgraph.get_reversed_graph();

        std::vector<std::unordered_set<int>> SCCs;
        compute_SCCs(subgraph, SCCs);

        std::unordered_set<int> T;
        int sccs_size = SCCs.size();
        for (int i = 0; i < sccs_size; i++) {
            for (int v : SCCs[i]) {
                T.insert(v);
            }
        }

        std::vector<std::pair<int, int>> transitions;
        kripke.transitions(transitions);

        while (!T.empty()) {
            int v = *T.begin();
            T.erase(T.begin());

            for (int t : L[phi->str()]) {
                if (kripke._next[t].find(v) != kripke._next[t].end() &&
                    L[s].find(t) == L[s].end()) {
                    L[s].insert(t);
                    T.insert(t);
                }
            }
        }
    }
}
