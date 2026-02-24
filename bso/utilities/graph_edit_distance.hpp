#ifndef GRAPH_EDIT_DISTANCE_HPP
#define GRAPH_EDIT_DISTANCE_HPP

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <Eigen/Dense>
#include <vector>
#include <functional>
#include <limits>
#include <memory>
#include <chrono>

namespace ged {

// Boost.Graph type definition - matches your usage
using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
using Edge = boost::graph_traits<Graph>::edge_descriptor;

// Edit operations
struct NodeEdit {
    int u; // vertex in G1 (-1 for insertion)
    int v; // vertex in G2 (-1 for deletion)

    NodeEdit(int u_, int v_) : u(u_), v(v_) {}
};

struct EdgeEdit {
    int g; // edge index in G1 (-1 for insertion)
    int h; // edge index in G2 (-1 for deletion)

    EdgeEdit(int g_, int h_) : g(g_), h(h_) {}
};

// Complete edit path result
struct EditPath {
    std::vector<NodeEdit> node_edits;
    std::vector<EdgeEdit> edge_edits;
    double cost;

    EditPath() : cost(0.0) {}
};

// Cost matrix structure with linear sum assignment
struct CostMatrix {
    Eigen::MatrixXd C;
    std::vector<int> row_ind;  // LSA row indices
    std::vector<int> col_ind;  // LSA column indices
    double ls;                  // sum of assigned costs

    CostMatrix() : ls(0.0) {}
};

// Configuration for GED computation
struct GEDConfig {
    // Cost functions - take vertex/edge descriptors as input
    // If nullptr, uses default unit costs
    std::function<double(Vertex, Vertex, const Graph&, const Graph&)> node_subst_cost;
    std::function<double(Vertex, const Graph&)> node_del_cost;
    std::function<double(Vertex, const Graph&)> node_ins_cost;
    std::function<double(Edge, Edge, const Graph&, const Graph&)> edge_subst_cost;
    std::function<double(Edge, const Graph&)> edge_del_cost;
    std::function<double(Edge, const Graph&)> edge_ins_cost;

    // Search parameters
    double upper_bound = std::numeric_limits<double>::infinity();
    bool strictly_decreasing = true;
    std::pair<Vertex, Vertex> roots = {-1, -1}; // -1 means no root
    double timeout = -1.0; // seconds, -1 means no timeout
};

// Hungarian algorithm for linear sum assignment
class HungarianAlgorithm {
public:
    static std::pair<std::vector<int>, std::vector<int>>
    solve(const Eigen::MatrixXd& cost_matrix);
};

// Main Graph Edit Distance calculator
class GraphEditDistance {
public:
    GraphEditDistance(const Graph& G1, const Graph& G2,
                     const GEDConfig& config = GEDConfig());

    // Iterator interface for edit paths (yields paths in non-increasing cost order)
    class Iterator {
    public:
        bool has_next();
        EditPath next();

    private:
        friend class GraphEditDistance;
        explicit Iterator(GraphEditDistance* ged);

        GraphEditDistance* ged_;
        bool initialized_;
        bool finished_;
    };

    Iterator begin();

    // Convenience function to get just the minimum distance
    double get_edit_distance();

	// Get the optimal edit distance (searches until no better path found)
    double get_optimal_edit_distance();

    // Get the optimal edit path with node and edge mappings
    EditPath get_optimal_edit_path();

    // Get just the node mapping from the optimal path
    std::vector<std::pair<int, int>> get_optimal_node_mapping();

private:
    const Graph& G1_;
    const Graph& G2_;
    GEDConfig config_;

    // Vertex and edge lists
    std::vector<Vertex> vertices1_;
    std::vector<Vertex> vertices2_;
    std::vector<std::pair<Vertex, Vertex>> edges1_;
    std::vector<std::pair<Vertex, Vertex>> edges2_;

    // Working structures
    std::vector<int> pending_u_;  // indices into vertices1_
    std::vector<int> pending_v_;  // indices into vertices2_
    std::vector<int> pending_g_;  // indices into edges1_
    std::vector<int> pending_h_;  // indices into edges2_

    CostMatrix Cv_;  // vertex cost matrix
    CostMatrix Ce_;  // edge cost matrix

    double maxcost_value_;
    double initial_cost_;
    std::vector<NodeEdit> done_uv_;

    std::chrono::steady_clock::time_point start_time_;

    // Initialization
    void initialize();
    void build_vertex_lists();
    void build_edge_lists();
    CostMatrix build_vertex_cost_matrix();
    CostMatrix build_edge_cost_matrix();

    // Cost matrix operations
    CostMatrix make_cost_matrix(const Eigen::MatrixXd& C, int m, int n);
    Eigen::MatrixXd extract_C(const Eigen::MatrixXd& C,
                             const std::vector<int>& i,
                             const std::vector<int>& j,
                             int m, int n);
    Eigen::MatrixXd reduce_C(const Eigen::MatrixXd& C,
                            const std::vector<int>& i,
                            const std::vector<int>& j,
                            int m, int n);
    std::vector<int> reduce_ind(const std::vector<int>& ind,
                                const std::vector<int>& remove);

    // Edge matching
    struct MatchEdgesResult {
        std::vector<std::pair<int, int>> ij;
        CostMatrix local_Ce;
    };

    MatchEdgesResult match_edges(int u_idx, int v_idx,
                                 const std::vector<int>& pending_g,
                                 const std::vector<int>& pending_h,
                                 const CostMatrix& Ce,
                                 const std::vector<NodeEdit>& matched_uv);

    CostMatrix reduce_Ce(const CostMatrix& Ce,
                        const std::vector<std::pair<int, int>>& ij,
                        int m, int n);

    // Edit operations generation
    struct EditOp {
        std::pair<int, int> ij;
        CostMatrix Cv_ij;
        std::vector<std::pair<int, int>> xy;
        CostMatrix Ce_xy;
        double edit_cost;
    };

    std::vector<EditOp> get_edit_ops(
        const std::vector<NodeEdit>& matched_uv,
        const std::vector<int>& pending_u,
        const std::vector<int>& pending_v,
        const CostMatrix& Cv,
        const std::vector<int>& pending_g,
        const std::vector<int>& pending_h,
        const CostMatrix& Ce,
        double matched_cost);

    // Search state for iterative path generation
    struct SearchState {
        std::vector<NodeEdit> matched_uv;
        std::vector<int> pending_u;
        std::vector<int> pending_v;
        CostMatrix Cv;
        std::vector<EdgeEdit> matched_gh;
        std::vector<int> pending_g;
        std::vector<int> pending_h;
        CostMatrix Ce;
        double matched_cost;
        int op_index;
        std::vector<EditOp> edit_ops;
        std::unique_ptr<SearchState> child_state;
    };

    std::unique_ptr<SearchState> root_state_;

    bool get_next_path(SearchState& state, EditPath& result);
    bool prune(double cost);
};

} // namespace ged

#endif // GRAPH_EDIT_DISTANCE_HPP
