#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class DiGraph {
   public:
    std::unordered_map<int, std::unordered_set<int>> _next;
    DiGraph(const std::unordered_set<int>& V,
            const std::vector<std::pair<int, int>>& E) {
        for (int v : V) {
            _next[v] = std::unordered_set<int>();
        }

        int src, dst;
        for (const auto& edge : E) {
            src = edge.first;
            dst = edge.second;

            if (_next.find(src) != _next.end()) {
                _next[src].insert(dst);
            } else {
                _next[src] = std::unordered_set<int>{dst};
            }

            if (_next.find(dst) == _next.end()) {
                _next[dst] = std::unordered_set<int>();
            }
        }
    }

    void add_node(const int v) {
        if (_next.find(v) != _next.end()) {
            throw std::runtime_error("Node already exists in the DiGraph");
        }
        _next[v] = std::unordered_set<int>();
    }

    void add_edge(const int src, const int dst) {
        if (_next.find(src) == _next.end()) {
            _next[src] = std::unordered_set<int>();
        } else {
            if (_next[src].find(dst) != _next[src].end()) {
                throw std::runtime_error("Edge already exists in the DiGraph");
            }
        }

        if (_next.find(dst) == _next.end()) {
            add_node(dst);
        }

        _next[src].insert(dst);
    }

    void sources(std::unordered_set<int>& result) const {
        for (const auto& entry : _next) {
            if (!entry.second.empty()) {
                result.insert(entry.first);
            }
        }
    }

    void nodes(std::vector<int>& result) const {
        for (const auto& entry : _next) {
            result.push_back(entry.first);
        }
    }

    void next(int src, std::unordered_set<int>& result) const {
        if (_next.find(src) == _next.end()) {
            throw std::runtime_error("Source node not found in the DiGraph");
        }
        result = _next.at(src);
    }

    void edges(std::vector<std::pair<int, int>>& result) const {
        for (const auto& entry : _next) {
            for (int dst : entry.second) {
                result.push_back(std::make_pair(entry.first, dst));
            }
        }
    }

    DiGraph clone() const {
        std::unordered_set<int> nV;
        std::vector<std::pair<int, int>> nE;
        DiGraph nDG(nV, nE);

        for (const auto& entry : _next) {
            nDG._next[entry.first] = entry.second;
        }

        return nDG;
    }

    std::string to_string() const {
        std::vector<int> vecN;
        std::vector<std::pair<int, int>> vecE;
        nodes(vecN);
        edges(vecE);

        std::string result = "(V={";
        for (int node : vecN) {
            result += std::to_string(node) + ",";
        }
        result.pop_back();  // remove the trailing comma
        result += "}, E={";
        for (const auto& edge : vecE) {
            result += "(" + std::to_string(edge.first) + "," +
                      std::to_string(edge.second) + "),";
        }
        result.pop_back();  // remove the trailing comma
        result += "})";
        return result;
    }

    DiGraph get_subgraph(const std::unordered_set<int>& nodes) const {
        std::unordered_set<int> V;
        for (int node : nodes) {
            if (_next.find(node) != _next.end()) {
                V.insert(node);
            }
        }

        std::vector<std::pair<int, int>> oE;
        edges(oE);
        std::vector<std::pair<int, int>> E;
        for (const auto& edge : oE) {
            if (V.find(edge.first) != V.end() &&
                V.find(edge.second) != V.end()) {
                E.push_back(edge);
            }
        }

        return DiGraph(V, E);
    }

    DiGraph get_reversed_graph() const {
        std::vector<int> vecV;
        nodes(vecV);
        std::unordered_set<int> V(vecV.begin(), vecV.end());
        std::vector<std::pair<int, int>> E;
        edges(E);

        std::vector<std::pair<int, int>> rE;
        for (const auto& edge : E) {
            rE.push_back(std::make_pair(edge.second, edge.first));
        }

        return DiGraph(V, rE);
    }

    std::unordered_set<int> get_reachable_set_from(
        const std::unordered_set<int>& nodes) const {
        std::vector<int> queue(nodes.begin(), nodes.end());
        std::unordered_set<int> R(nodes);

        while (!queue.empty()) {
            int s = queue.back();
            queue.pop_back();

            std::unordered_set<int> nexts;
            next(s, nexts);
            for (int d : nexts) {
                if (R.find(d) == R.end()) {
                    R.insert(d);
                    queue.push_back(d);
                }
            }
        }

        return R;
    }
};

inline void compute_SCCs(const DiGraph& G,
                  std::vector<std::unordered_set<int>>& result) {
    std::unordered_map<int, int> disc;
    std::unordered_map<int, int> lowlink;
    std::unordered_set<int> in_a_scc;
    std::vector<int> scc_stack;
    int time = 0;

    auto dfs = [&](int s) {
        disc[s] = time;
        lowlink[s] = time;

        std::unordered_set<int> next_s;
        G.next(s, next_s);
        std::vector<std::pair<int, std::unordered_set<int>::iterator>> stack{
            {s, next_s.begin()}};
        while (!stack.empty()) {
            try {
                auto& top = stack.back();
                int w = *(top.second);

                if (disc.find(w) == disc.end()) {
                    time++;
                    disc[w] = time;
                    lowlink[w] = time;

                    std::unordered_set<int> next_w;
                    G.next(w, next_w);
                    stack.push_back({w, next_w.begin()});
                }

            } catch (const std::out_of_range&) {
                int v = stack.back().first;
                stack.pop_back();

                std::unordered_set<int> next_v;
                G.next(v, next_v);
                for (int w : next_v) {
                    if (in_a_scc.find(w) == in_a_scc.end()) {
                        if (disc[w] > disc[v]) {
                            lowlink[v] = std::min(lowlink[v], lowlink[w]);
                        } else {
                            lowlink[v] = std::min(lowlink[v], disc[w]);
                        }
                    }
                }

                if (lowlink[v] == disc[v]) {
                    in_a_scc.insert(v);
                    std::unordered_set<int> scc = {v};
                    while (!scc_stack.empty() &&
                           disc[scc_stack.back()] > disc[v]) {
                        int k = scc_stack.back();
                        scc_stack.pop_back();
                        in_a_scc.insert(k);
                        scc.insert(k);
                    }
                    result.emplace_back(scc);
                } else {
                    scc_stack.push_back(v);
                }
            }
        }
    };

    std::vector<int> vecV;
    G.nodes(vecV);
    for (int s : vecV) {
        if (disc.find(s) == disc.end()) {
            dfs(s);
        }
    }
}

