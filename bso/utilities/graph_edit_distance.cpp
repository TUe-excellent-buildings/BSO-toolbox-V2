#ifndef GRAPH_EDIT_DISTANCE_CPP
#define GRAPH_EDIT_DISTANCE_CPP

// #include "graph_edit_distance.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <algorithm>
#include <queue>
#include <set>
#include <cmath>
#include <iostream>

namespace ged {

// Hungarian Algorithm Implementation using Eigen
std::pair<std::vector<int>, std::vector<int>>
HungarianAlgorithm::solve(const Eigen::MatrixXd& cost_matrix) {

    if (cost_matrix.rows() == 0 || cost_matrix.cols() == 0) {
        return {{}, {}};
    }

    int n = cost_matrix.rows();
    int m = cost_matrix.cols();
    int size = std::max(n, m);


    // Create square matrix
    Eigen::MatrixXd C = Eigen::MatrixXd::Zero(size, size);
    C.block(0, 0, n, m) = cost_matrix;

    const double INF = std::numeric_limits<double>::infinity();

    // Row reduction
    for (int i = 0; i < size; ++i) {
        double min_val = C.row(i).minCoeff();
        if (min_val != INF && !std::isinf(min_val)) {
            for (int j = 0; j < size; ++j) {
                if (!std::isinf(C(i, j))) {
                    C(i, j) -= min_val;
                }
            }
        }
    }

    // Column reduction
    for (int j = 0; j < size; ++j) {
        double min_val = C.col(j).minCoeff();
        if (min_val != INF && !std::isinf(min_val) && min_val > 0) {
            for (int i = 0; i < size; ++i) {
                if (!std::isinf(C(i, j))) {
                    C(i, j) -= min_val;
                }
            }
        }
    }

    // Find assignment using augmenting paths
    std::vector<int> row_assignment(size, -1);
    std::vector<int> col_assignment(size, -1);

    for (int iter = 0; iter < size; ++iter) {
        // Find unmatched row
        int unmatched_row = -1;
        for (int i = 0; i < size; ++i) {
            if (row_assignment[i] == -1) {
                unmatched_row = i;
                break;
            }
        }
        if (unmatched_row == -1) {
            break;
        }


        std::vector<bool> row_covered(size, false);
        std::vector<bool> col_covered(size, false);
        std::vector<int> parent_row(size, -1);
        std::vector<double> slack(size, INF);

        int matched_col = -1;

        // Keep trying until we find a match
        while (matched_col == -1) {

            // BFS to find augmenting path
            std::queue<int> q;
            q.push(unmatched_row);
            row_covered[unmatched_row] = true;

            while (!q.empty() && matched_col == -1) {
                int i = q.front();
                q.pop();

                for (int j = 0; j < size; ++j) {
                    if (col_covered[j] || std::isinf(C(i, j))) continue;

                    double gap = C(i, j);
                    if (gap < slack[j]) {
                        slack[j] = gap;
                        parent_row[j] = i;
                    }

                    if (std::abs(gap) < 1e-10) {  // essentially zero
                        if (col_assignment[j] == -1) {
                            matched_col = j;
                            break;
                        } else {
                            int next_row = col_assignment[j];
                            if (!row_covered[next_row]) {
                                row_covered[next_row] = true;
                                q.push(next_row);
                            }
                        }
                        col_covered[j] = true;
                    }
                }
            }

            if (matched_col == -1) {
                // Update dual variables
                double min_slack = INF;
                for (int j = 0; j < size; ++j) {
                    if (!col_covered[j] && slack[j] < min_slack) {
                        min_slack = slack[j];
                    }
                }

                if (std::isinf(min_slack)) {
                    // No solution possible - shouldn't happen with valid input
                    matched_col = -2;  // Signal to break outer loop
                    break;
                }

                for (int i = 0; i < size; ++i) {
                    if (row_covered[i]) {
                        for (int j = 0; j < size; ++j) {
                            if (!std::isinf(C(i, j))) {
                                C(i, j) += min_slack;
                            }
                        }
                    }
                }

                for (int j = 0; j < size; ++j) {
                    if (col_covered[j]) {
                        for (int i = 0; i < size; ++i) {
                            if (!std::isinf(C(i, j))) {
                                C(i, j) -= min_slack;
                            }
                        }
                    } else {
                        slack[j] -= min_slack;
                        if (std::abs(slack[j]) < 1e-10 && parent_row[j] != -1) {
                            if (col_assignment[j] == -1) {
                                matched_col = j;
                            }
                        }
                    }
                }
            }
        }

        // Augment path
        if (matched_col >= 0) {
            int j = matched_col;
            while (j != -1 && parent_row[j] != -1) {
                int i = parent_row[j];
                int prev_j = row_assignment[i];
                row_assignment[i] = j;
                col_assignment[j] = i;
                j = prev_j;
            }
        } else {
            // matched_col == -2 means no solution
            break;
        }
    }

    // Extract results
    std::vector<int> result_rows, result_cols;
    for (int i = 0; i < size; ++i) {
        if (row_assignment[i] != -1) {
            result_rows.push_back(i);
            result_cols.push_back(row_assignment[i]);
        }
    }


    return {result_rows, result_cols};
}

// GraphEditDistance implementation
GraphEditDistance::GraphEditDistance(const Graph& G1, const Graph& G2,
                                     const GEDConfig& config)
    : G1_(G1), G2_(G2), config_(config), maxcost_value_(0.0), initial_cost_(0.0) {
    initialize();
}

void GraphEditDistance::build_vertex_lists() {
    auto [vi1, vi1_end] = boost::vertices(G1_);
    for (auto it = vi1; it != vi1_end; ++it) {
        vertices1_.push_back(*it);
    }

    auto [vi2, vi2_end] = boost::vertices(G2_);
    for (auto it = vi2; it != vi2_end; ++it) {
        vertices2_.push_back(*it);
    }
}

void GraphEditDistance::build_edge_lists() {
    auto [ei1, ei1_end] = boost::edges(G1_);
    for (auto it = ei1; it != ei1_end; ++it) {
        Vertex src = boost::source(*it, G1_);
        Vertex tgt = boost::target(*it, G1_);
        edges1_.push_back({src, tgt});
    }

    auto [ei2, ei2_end] = boost::edges(G2_);
    for (auto it = ei2; it != ei2_end; ++it) {
        Vertex src = boost::source(*it, G2_);
        Vertex tgt = boost::target(*it, G2_);
        edges2_.push_back({src, tgt});
    }
}

void GraphEditDistance::initialize() {
    build_vertex_lists();
    build_edge_lists();


    // Initialize pending lists
    for (int i = 0; i < static_cast<int>(vertices1_.size()); ++i) {
        pending_u_.push_back(i);
    }
    for (int i = 0; i < static_cast<int>(vertices2_.size()); ++i) {
        pending_v_.push_back(i);
    }

    // Handle roots if specified
    if (config_.roots.first != static_cast<Vertex>(-1) &&
        config_.roots.second != static_cast<Vertex>(-1)) {

        auto it_u = std::find(vertices1_.begin(), vertices1_.end(), config_.roots.first);
        auto it_v = std::find(vertices2_.begin(), vertices2_.end(), config_.roots.second);

        if (it_u != vertices1_.end() && it_v != vertices2_.end()) {
            int idx_u = std::distance(vertices1_.begin(), it_u);
            int idx_v = std::distance(vertices2_.begin(), it_v);

            pending_u_.erase(std::remove(pending_u_.begin(), pending_u_.end(), idx_u),
                           pending_u_.end());
            pending_v_.erase(std::remove(pending_v_.begin(), pending_v_.end(), idx_v),
                           pending_v_.end());

            done_uv_.push_back(NodeEdit(idx_u, idx_v));

            if (config_.node_subst_cost) {
                initial_cost_ = config_.node_subst_cost(
                    config_.roots.first, config_.roots.second, G1_, G2_);
            }
        }
    }

    // Build cost matrices
    Cv_ = build_vertex_cost_matrix();

    for (int i = 0; i < static_cast<int>(edges1_.size()); ++i) {
        pending_g_.push_back(i);
    }
    for (int i = 0; i < static_cast<int>(edges2_.size()); ++i) {
        pending_h_.push_back(i);
    }

    Ce_ = build_edge_cost_matrix();

    // Calculate max cost
    maxcost_value_ = Cv_.C.sum() + Ce_.C.sum() + 1.0;

    if (config_.timeout > 0) {
        start_time_ = std::chrono::steady_clock::now();
    }
}

CostMatrix GraphEditDistance::build_vertex_cost_matrix() {
    int m = pending_u_.size();
    int n = pending_v_.size();

    Eigen::MatrixXd C = Eigen::MatrixXd::Zero(m + n, m + n);

    // Substitution costs
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (config_.node_subst_cost) {
                C(i, j) = config_.node_subst_cost(
                    vertices1_[pending_u_[i]], vertices2_[pending_v_[j]], G1_, G2_);
            } else {
                C(i, j) = 0.0;
            }
        }
    }

    // Calculate deletion/insertion costs and inf value
    double sum = C.block(0, 0, m, n).sum();

    std::vector<double> del_costs(m);
    for (int i = 0; i < m; ++i) {
        if (config_.node_del_cost) {
            del_costs[i] = config_.node_del_cost(vertices1_[pending_u_[i]], G1_);
        } else {
            del_costs[i] = 1.0;
        }
        sum += del_costs[i];
    }

    std::vector<double> ins_costs(n);
    for (int j = 0; j < n; ++j) {
        if (config_.node_ins_cost) {
            ins_costs[j] = config_.node_ins_cost(vertices2_[pending_v_[j]], G2_);
        } else {
            ins_costs[j] = 1.0;
        }
        sum += ins_costs[j];
    }

    double inf = sum + 1.0;

    // Deletion block (bottom-left)
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            C(i, n + j) = (i == j) ? del_costs[i] : inf;
        }
    }

    // Insertion block (top-right)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            C(m + i, j) = (i == j) ? ins_costs[i] : inf;
        }
    }

    return make_cost_matrix(C, m, n);
}

CostMatrix GraphEditDistance::build_edge_cost_matrix() {
    int m = pending_g_.size();
    int n = pending_h_.size();

    Eigen::MatrixXd C = Eigen::MatrixXd::Zero(m + n, m + n);

    // Substitution costs
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (config_.edge_subst_cost) {
                auto [src1, tgt1] = edges1_[pending_g_[i]];
                auto [src2, tgt2] = edges2_[pending_h_[j]];
                Edge e1, e2;
                bool found1 = false, found2 = false;
                std::tie(e1, found1) = boost::edge(src1, tgt1, G1_);
                std::tie(e2, found2) = boost::edge(src2, tgt2, G2_);
                if (found1 && found2) {
                    C(i, j) = config_.edge_subst_cost(e1, e2, G1_, G2_);
                }
            } else {
                C(i, j) = 0.0;
            }
        }
    }

    double sum = C.block(0, 0, m, n).sum();

    std::vector<double> del_costs(m);
    for (int i = 0; i < m; ++i) {
        if (config_.edge_del_cost) {
            auto [src, tgt] = edges1_[pending_g_[i]];
            Edge e;
            bool found;
            std::tie(e, found) = boost::edge(src, tgt, G1_);
            if (found) {
                del_costs[i] = config_.edge_del_cost(e, G1_);
            } else {
                del_costs[i] = 1.0;
            }
        } else {
            del_costs[i] = 1.0;
        }
        sum += del_costs[i];
    }

    std::vector<double> ins_costs(n);
    for (int j = 0; j < n; ++j) {
        if (config_.edge_ins_cost) {
            auto [src, tgt] = edges2_[pending_h_[j]];
            Edge e;
            bool found;
            std::tie(e, found) = boost::edge(src, tgt, G2_);
            if (found) {
                ins_costs[j] = config_.edge_ins_cost(e, G2_);
            } else {
                ins_costs[j] = 1.0;
            }
        } else {
            ins_costs[j] = 1.0;
        }
        sum += ins_costs[j];
    }

    double inf = sum + 1.0;

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            C(i, n + j) = (i == j) ? del_costs[i] : inf;
        }
    }

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            C(m + i, j) = (i == j) ? ins_costs[i] : inf;
        }
    }

    return make_cost_matrix(C, m, n);
}

CostMatrix GraphEditDistance::make_cost_matrix(const Eigen::MatrixXd& C, int m, int n) {
    auto [row_ind, col_ind] = HungarianAlgorithm::solve(C);

    CostMatrix result;
    result.C = C;
    result.row_ind = row_ind;
    result.col_ind = col_ind;

    // Fix dummy assignments
    for (size_t k = 0; k < row_ind.size(); ++k) {
        bool is_subst = (row_ind[k] < m) && (col_ind[k] < n);
        bool is_dummy = (row_ind[k] >= m) && (col_ind[k] >= n);

        if (is_dummy) {
            // Find corresponding substitution
            for (size_t j = 0; j < row_ind.size(); ++j) {
                if ((row_ind[j] < m) && (col_ind[j] < n)) {
                    result.row_ind[k] = col_ind[j] + m;
                    result.col_ind[k] = row_ind[j] + n;
                    break;
                }
            }
        }
    }

    // Calculate assignment cost
    result.ls = 0.0;
    for (size_t k = 0; k < row_ind.size(); ++k) {
        if (result.row_ind[k] < C.rows() && result.col_ind[k] < C.cols()) {
            result.ls += C(result.row_ind[k], result.col_ind[k]);
        }
    }

    return result;
}

Eigen::MatrixXd GraphEditDistance::extract_C(
    const Eigen::MatrixXd& C,
    const std::vector<int>& i,
    const std::vector<int>& j,
    int m, int n) {

    std::set<int> i_set(i.begin(), i.end());
    std::set<int> j_set(j.begin(), j.end());

    std::vector<int> row_ind, col_ind;

    for (int k = 0; k < m + n; ++k) {
        if (i_set.count(k) || (k >= m && j_set.count(k - m))) {
            row_ind.push_back(k);
        }
    }

    for (int k = 0; k < m + n; ++k) {
        if (j_set.count(k) || (k >= n && i_set.count(k - n))) {
            col_ind.push_back(k);
        }
    }

    Eigen::MatrixXd result(row_ind.size(), col_ind.size());
    for (size_t r = 0; r < row_ind.size(); ++r) {
        for (size_t c = 0; c < col_ind.size(); ++c) {
            result(r, c) = C(row_ind[r], col_ind[c]);
        }
    }

    return result;
}

Eigen::MatrixXd GraphEditDistance::reduce_C(
    const Eigen::MatrixXd& C,
    const std::vector<int>& i,
    const std::vector<int>& j,
    int m, int n) {

    std::set<int> i_set(i.begin(), i.end());
    std::set<int> j_set(j.begin(), j.end());

    std::vector<int> row_ind, col_ind;

    for (int k = 0; k < m + n; ++k) {
        if (!i_set.count(k) && (k < m || !j_set.count(k - m))) {
            row_ind.push_back(k);
        }
    }

    for (int k = 0; k < m + n; ++k) {
        if (!j_set.count(k) && (k < n || !i_set.count(k - n))) {
            col_ind.push_back(k);
        }
    }

    Eigen::MatrixXd result(row_ind.size(), col_ind.size());
    for (size_t r = 0; r < row_ind.size(); ++r) {
        for (size_t c = 0; c < col_ind.size(); ++c) {
            result(r, c) = C(row_ind[r], col_ind[c]);
        }
    }

    return result;
}

std::vector<int> GraphEditDistance::reduce_ind(
    const std::vector<int>& ind,
    const std::vector<int>& remove) {

    std::set<int> remove_set(remove.begin(), remove.end());
    std::vector<int> result;

    for (int k : ind) {
        if (!remove_set.count(k)) {
            result.push_back(k);
        }
    }

    // Adjust indices
    for (int r : remove) {
        for (int& k : result) {
            if (k >= r) k--;
        }
    }

    return result;
}


GraphEditDistance::MatchEdgesResult GraphEditDistance::match_edges(
    int u_idx, int v_idx,
    const std::vector<int>& pending_g,
    const std::vector<int>& pending_h,
    const CostMatrix& Ce,
    const std::vector<NodeEdit>& matched_uv) {

    MatchEdgesResult result;

    int M = pending_g.size();
    int N = pending_h.size();

    // Get actual vertex descriptors
    Vertex u = (u_idx >= 0 && u_idx < static_cast<int>(vertices1_.size()))
               ? vertices1_[u_idx] : static_cast<Vertex>(-1);
    Vertex v = (v_idx >= 0 && v_idx < static_cast<int>(vertices2_.size()))
               ? vertices2_[v_idx] : static_cast<Vertex>(-1);

    std::vector<int> g_ind, h_ind;

    // Build node mapping from matched_uv + current (u,v)
    std::map<Vertex, Vertex> node_map;  // G1 vertex -> G2 vertex

    for (const auto& edit : matched_uv) {
        if (edit.u >= 0 && edit.v >= 0 &&
            edit.u < static_cast<int>(vertices1_.size()) &&
            edit.v < static_cast<int>(vertices2_.size())) {
            node_map[vertices1_[edit.u]] = vertices2_[edit.v];
        }
    }

    // Add current match
    if (u != static_cast<Vertex>(-1) && v != static_cast<Vertex>(-1)) {
        node_map[u] = v;
    }

    // Find edges in G1 where BOTH endpoints are in the node mapping
    for (int i = 0; i < M; ++i) {
        const auto& [s, t] = edges1_[pending_g[i]];

        // Check if both endpoints have been matched
        bool s_matched = node_map.find(s) != node_map.end();
        bool t_matched = node_map.find(t) != node_map.end();

        if (s_matched && t_matched) {
            g_ind.push_back(i);
        }
    }

    // Find edges in G2 where BOTH endpoints are in the node mapping (as targets)
    for (int j = 0; j < N; ++j) {
        const auto& [s, t] = edges2_[pending_h[j]];

        // Check if both endpoints are targets of some node match
        bool s_is_target = false;
        bool t_is_target = false;

        for (const auto& [key, val] : node_map) {
            if (val == s) s_is_target = true;
            if (val == t) t_is_target = true;
        }

        if (s_is_target && t_is_target) {
            h_ind.push_back(j);
        }
    }

    int m = g_ind.size();
    int n = h_ind.size();

    if (m > 0 || n > 0) {
        auto C = extract_C(Ce.C, g_ind, h_ind, M, N);

        // Forbid structurally invalid matches
        const double INF = std::numeric_limits<double>::infinity();
        int valid_count = 0;

        for (int k = 0; k < m; ++k) {
            const auto& [g_src, g_tgt] = edges1_[pending_g[g_ind[k]]];

            for (int l = 0; l < n; ++l) {
                const auto& [h_src, h_tgt] = edges2_[pending_h[h_ind[l]]];

                bool valid = false;

                // Check if edge g's endpoints map to edge h's endpoints
                auto it_src = node_map.find(g_src);
                auto it_tgt = node_map.find(g_tgt);

                if (it_src != node_map.end() && it_tgt != node_map.end()) {
                    Vertex mapped_src = it_src->second;
                    Vertex mapped_tgt = it_tgt->second;

                    // Valid if endpoints match in either order
                    if ((mapped_src == h_src && mapped_tgt == h_tgt) ||
                        (mapped_src == h_tgt && mapped_tgt == h_src)) {
                        valid = true;
                        valid_count++;
                    }
                }

                if (!valid) {
                    C(k, l) = INF;
                }
            }
        }

        result.local_Ce = make_cost_matrix(C, m, n);

        for (size_t idx = 0; idx < result.local_Ce.row_ind.size(); ++idx) {
            int k = result.local_Ce.row_ind[idx];
            int l = result.local_Ce.col_ind[idx];

            if (k < m || l < n) {
                int gi, hj;
                if (k < m) {
                    // Real edge from G1
                    gi = g_ind[k];
                } else {
                    // Dummy for insertion - map back to G2 edge
                    gi = M + h_ind[k - m];
                }

                if (l < n) {
                    // Real edge from G2
                    hj = h_ind[l];
                } else {
                    // Dummy for deletion - map back to G1 edge
                    hj = N + g_ind[l - n];
                }

                result.ij.push_back({gi, hj});
            }
        }
    } else {
        result.local_Ce.C = Eigen::MatrixXd(0, 0);
        result.local_Ce.ls = 0.0;
    }

    return result;
}


CostMatrix GraphEditDistance::reduce_Ce(
    const CostMatrix& Ce,
    const std::vector<std::pair<int, int>>& ij,
    int m, int n) {

    if (ij.empty()) return Ce;

    std::vector<int> i_list, j_list;
    for (const auto& [i, j] : ij) {
        i_list.push_back(i);
        j_list.push_back(j);
    }

    int m_i = m;
    for (int i : i_list) {
        if (i < m) m_i--;
    }

    int n_j = n;
    for (int j : j_list) {
        if (j < n) n_j--;
    }

    auto reduced = reduce_C(Ce.C, i_list, j_list, m, n);
    return make_cost_matrix(reduced, m_i, n_j);
}

bool GraphEditDistance::prune(double cost) {
    if (config_.timeout > 0) {
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - start_time_).count();
        if (elapsed > config_.timeout) return true;
    }

    if (!std::isinf(config_.upper_bound) && cost > config_.upper_bound) {
        return true;
    }

    if (cost > maxcost_value_) return true;

    if (config_.strictly_decreasing && cost >= maxcost_value_) {
        return true;
    }

    return false;
}

std::vector<GraphEditDistance::EditOp> GraphEditDistance::get_edit_ops(
    const std::vector<NodeEdit>& matched_uv,
    const std::vector<int>& pending_u,
    const std::vector<int>& pending_v,
    const CostMatrix& Cv,
    const std::vector<int>& pending_g,
    const std::vector<int>& pending_h,
    const CostMatrix& Ce,
    double matched_cost) {

    std::vector<EditOp> result;

    int m = pending_u.size();
    int n = pending_v.size();

    // Find best from LSA
    int best_i = -1, best_j = -1;
    for (size_t idx = 0; idx < Cv.row_ind.size(); ++idx) {
        int i = Cv.row_ind[idx];
        int j = Cv.col_ind[idx];
        if (i < m || j < n) {
            best_i = i;
            best_j = j;
            break;
        }
    }


    if (best_i == -1) {
        return result;
    }

    // First operation
    auto match_result = match_edges(
        best_i < m ? pending_u[best_i] : -1,
        best_j < n ? pending_v[best_j] : -1,
        pending_g, pending_h, Ce, matched_uv);


    auto Ce_xy = reduce_Ce(Ce, match_result.ij, pending_g.size(), pending_h.size());


    if (!prune(matched_cost + Cv.ls + match_result.local_Ce.ls + Ce_xy.ls)) {
        EditOp op;
        op.ij = {best_i, best_j};

        auto reduced = reduce_C(Cv.C, {best_i}, {best_j}, m, n);
        std::vector<int> remove_indices = {best_i, m + best_j};

        op.Cv_ij.C = reduced;
        op.Cv_ij.row_ind = reduce_ind(Cv.row_ind, remove_indices);
        op.Cv_ij.col_ind = reduce_ind(Cv.col_ind, remove_indices);
        op.Cv_ij.ls = Cv.ls - Cv.C(best_i, best_j);

        op.xy = match_result.ij;
        op.Ce_xy = Ce_xy;
        op.edit_cost = Cv.C(best_i, best_j) + match_result.local_Ce.ls;

        result.push_back(std::move(op));
    }

    // Other candidates
    std::vector<std::pair<int, int>> candidates;
    if (m <= n) {
        for (int t = 0; t < m + n; ++t) {
            if (t != best_i && (t < m || t == m + best_j)) {
                candidates.push_back({t, best_j});
            }
        }
    } else {
        for (int t = 0; t < m + n; ++t) {
            if (t != best_j && (t < n || t == n + best_i)) {
                candidates.push_back({best_i, t});
            }
        }
    }

    std::vector<EditOp> other_ops;
    for (const auto& [i, j] : candidates) {
        if (prune(matched_cost + Cv.C(i, j) + Ce.ls)) continue;

        auto reduced = reduce_C(Cv.C, {i}, {j}, m, n);
        auto Cv_ij = make_cost_matrix(reduced,
                                      m - (i < m ? 1 : 0),
                                      n - (j < n ? 1 : 0));

        if (prune(matched_cost + Cv.C(i, j) + Cv_ij.ls + Ce.ls)) continue;

        auto mr = match_edges(
            i < m ? pending_u[i] : -1,
            j < n ? pending_v[j] : -1,
            pending_g, pending_h, Ce, matched_uv);

        if (prune(matched_cost + Cv.C(i, j) + Cv_ij.ls + mr.local_Ce.ls)) continue;

        auto Ce_xy2 = reduce_Ce(Ce, mr.ij, pending_g.size(), pending_h.size());

        if (prune(matched_cost + Cv.C(i, j) + Cv_ij.ls + mr.local_Ce.ls + Ce_xy2.ls)) continue;

        EditOp op;
        op.ij = {i, j};
        op.Cv_ij = Cv_ij;
        op.xy = mr.ij;
        op.Ce_xy = Ce_xy2;
        op.edit_cost = Cv.C(i, j) + mr.local_Ce.ls;

        other_ops.push_back(std::move(op));
    }

    std::sort(other_ops.begin(), other_ops.end(),
              [](const EditOp& a, const EditOp& b) {
                  return (a.edit_cost + a.Cv_ij.ls + a.Ce_xy.ls) <
                         (b.edit_cost + b.Cv_ij.ls + b.Ce_xy.ls);
              });

    result.insert(result.end(), other_ops.begin(), other_ops.end());

    return result;
}

bool GraphEditDistance::get_next_path(SearchState& state, EditPath& result) {
    if (prune(state.matched_cost + state.Cv.ls + state.Ce.ls)) {
        return false;
    }

	if (state.pending_u.empty() && state.pending_v.empty()) {
		maxcost_value_ = std::min(maxcost_value_, state.matched_cost);

		result.node_edits = state.matched_uv;
		result.edge_edits = state.matched_gh;
		result.cost = state.matched_cost;
		return true;
	}

    // Process child
    if (state.child_state) {
        if (get_next_path(*state.child_state, result)) {
            return true;
        }
        state.child_state.reset();
        state.op_index++;
    }

    // Generate ops if needed
    if (state.edit_ops.empty()) {
        state.edit_ops = get_edit_ops(
            state.matched_uv, state.pending_u, state.pending_v, state.Cv,
            state.pending_g, state.pending_h, state.Ce, state.matched_cost);
        state.op_index = 0;
    }

    // Try next op
    while (state.op_index < static_cast<int>(state.edit_ops.size())) {
        const auto& op = state.edit_ops[state.op_index];

        if (prune(state.matched_cost + op.edit_cost + op.Cv_ij.ls + op.Ce_xy.ls)) {
            state.op_index++;
            continue;
        }

        // Create child
        auto child = std::make_unique<SearchState>();
        child->matched_uv = state.matched_uv;
        child->pending_u = state.pending_u;
        child->pending_v = state.pending_v;
        child->Cv = op.Cv_ij;
        child->matched_gh = state.matched_gh;
        child->pending_g = state.pending_g;
        child->pending_h = state.pending_h;
        child->Ce = op.Ce_xy;
        child->matched_cost = state.matched_cost + op.edit_cost;
        child->op_index = 0;

        int i = op.ij.first;
        int j = op.ij.second;


        int u = (i < static_cast<int>(child->pending_u.size())) ? child->pending_u[i] : -1;
        int v = (j < static_cast<int>(child->pending_v.size())) ? child->pending_v[j] : -1;


        child->matched_uv.push_back(NodeEdit(u, v));

        if (i < static_cast<int>(child->pending_u.size())) {
            child->pending_u.erase(child->pending_u.begin() + i);
        }
        if (j < static_cast<int>(child->pending_v.size())) {
            child->pending_v.erase(child->pending_v.begin() + j);
        }

        for (const auto& [x, y] : op.xy) {
            int g = (x < static_cast<int>(child->pending_g.size())) ? child->pending_g[x] : -1;
            int h = (y < static_cast<int>(child->pending_h.size())) ? child->pending_h[y] : -1;
            child->matched_gh.push_back(EdgeEdit(g, h));
        }

        std::vector<int> sortedx, sortedy;
        for (const auto& [x, y] : op.xy) {
            if (x < static_cast<int>(child->pending_g.size())) sortedx.push_back(x);
            if (y < static_cast<int>(child->pending_h.size())) sortedy.push_back(y);
        }
        std::sort(sortedx.rbegin(), sortedx.rend());
        std::sort(sortedy.rbegin(), sortedy.rend());

        for (int x : sortedx) {
            child->pending_g.erase(child->pending_g.begin() + x);
        }
        for (int y : sortedy) {
            child->pending_h.erase(child->pending_h.begin() + y);
        }

        state.child_state = std::move(child);


        if (get_next_path(*state.child_state, result)) {
            return true;
        }

        state.child_state.reset();
        state.op_index++;
    }

    return false;
}

GraphEditDistance::Iterator::Iterator(GraphEditDistance* ged)
    : ged_(ged), initialized_(false), finished_(false) {}

bool GraphEditDistance::Iterator::has_next() {
    return !finished_;
}

EditPath GraphEditDistance::Iterator::next() {
    if (finished_) {
        // Already finished - return sentinel
        EditPath empty;
        empty.cost = std::numeric_limits<double>::infinity();
        return empty;
    }

    if (!initialized_) {
        ged_->root_state_ = std::make_unique<SearchState>();
        ged_->root_state_->matched_uv = ged_->done_uv_;
        ged_->root_state_->pending_u = ged_->pending_u_;
        ged_->root_state_->pending_v = ged_->pending_v_;
        ged_->root_state_->Cv = ged_->Cv_;
        ged_->root_state_->pending_g = ged_->pending_g_;
        ged_->root_state_->pending_h = ged_->pending_h_;
        ged_->root_state_->Ce = ged_->Ce_;
        ged_->root_state_->matched_cost = ged_->initial_cost_;
        ged_->root_state_->op_index = 0;

        initialized_ = true;
    }

    EditPath result;
    if (ged_->get_next_path(*ged_->root_state_, result)) {
        return result;
    }

    // No more paths found
    finished_ = true;
    result.cost = std::numeric_limits<double>::infinity();
    return result;
}


GraphEditDistance::Iterator GraphEditDistance::begin() {
    return Iterator(this);
}

double GraphEditDistance::get_edit_distance() {
    auto it = begin();

    if (it.has_next()) {
        // With strictly_decreasing=true (default), paths are yielded in
        // order of strictly decreasing cost, so the first path is optimal
        return it.next().cost;
    }

    return std::numeric_limits<double>::infinity();
}


double GraphEditDistance::get_optimal_edit_distance() {
    auto it = begin();
    double best_cost = std::numeric_limits<double>::infinity();

    while (it.has_next()) {
        auto path = it.next();

        // Skip sentinel values
        if (std::isinf(path.cost)) break;

        if (path.cost < best_cost) {
            best_cost = path.cost;
        }

        // Early exit if we find a perfect match (cost 0)
        if (best_cost == 0.0) break;
    }

    return best_cost;
}


EditPath GraphEditDistance::get_optimal_edit_path() {
    auto it = begin();
    EditPath best_path;
    best_path.cost = std::numeric_limits<double>::infinity();

    while (it.has_next()) {
        auto path = it.next();

        // Skip sentinel values
        if (std::isinf(path.cost)) break;

        if (path.cost < best_path.cost) {
            best_path = path;
        }

        // Early exit if we find a perfect match
        if (best_path.cost == 0.0) break;
    }

    return best_path;
}


std::vector<std::pair<int, int>> GraphEditDistance::get_optimal_node_mapping() {
    EditPath optimal = get_optimal_edit_path();

    std::vector<std::pair<int, int>> mapping;

    for (const auto& edit : optimal.node_edits) {
        if (edit.u >= 0 && edit.v >= 0) {
            // Only include substitutions (actual mappings)
            mapping.push_back({edit.u, edit.v});
        }
    }

    return mapping;
}
} // namespace ged

#endif // GRAPH_EDIT_DISTANCE_CPP
