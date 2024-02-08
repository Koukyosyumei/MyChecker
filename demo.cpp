#include <iostream>

#include "libmychecker/checker.h"

int main() {
    std::unordered_set<int> S;
    std::unordered_set<int> S0;
    std::vector<std::pair<int, int>> R = {{0, 0}, {0, 1}, {1, 2}, {2, 2}};
    std::unordered_map<int, std::unordered_set<std::string>> kL = {
        {0, {}}, {1, {"p"}}, {2, {"q"}}};
    Kripke kripke(S, S0, R, kL);

    std::unordered_map<std::string, std::unordered_set<int>> L;
    std::vector<std::unordered_set<int>> F;
    std::shared_ptr<Formula> formula = std::make_shared<CTL::Bool>("true");
    modelcheck(kripke, formula, L, F);

    for (auto const &t : L) {
        std::cout << t.first << ": [";
        for (int i : t.second) {
            std::cout << i << ", ";
        }
        std::cout << "]\n";
    }
}
