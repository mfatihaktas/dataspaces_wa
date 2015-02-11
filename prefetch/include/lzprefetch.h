#ifndef LZPREFETCH_H
#define LZPREFETCH_H

#include <iostream>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

#include <utility>                   // for std::pair
#include <algorithm>                 // for std::for_each
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <boost/config.hpp>
#include <boost/version.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/static_assert.hpp>
/* definition of basic boost::graph properties */
enum vertex_properties_t { vertex_properties };
enum edge_properties_t { edge_properties };
namespace boost {
  BOOST_INSTALL_PROPERTY(vertex, properties);
  BOOST_INSTALL_PROPERTY(edge, properties);
}

/* the graph base class template */
template <typename VERTEX_PROPERTIES, typename EDGE_PROPERTIES>
class Graph
{
  typedef boost::adjacency_list<
    boost::setS, // disallow parallel edges
    boost::listS, // vertex container
    boost::bidirectionalS, // directed graph
    boost::property<vertex_properties_t, VERTEX_PROPERTIES>,
    boost::property<edge_properties_t, EDGE_PROPERTIES>
  > GraphContainer;
    /* a bunch of graph-specific typedefs */
    typedef typename boost::graph_traits<GraphContainer>::vertex_descriptor Vertex;
    typedef typename boost::graph_traits<GraphContainer>::edge_descriptor Edge;
  
    typedef typename boost::graph_traits<GraphContainer>::vertex_iterator vertex_iter;
    typedef typename boost::graph_traits<GraphContainer>::edge_iterator edge_iter;
    typedef typename boost::graph_traits<GraphContainer>::adjacency_iterator adjacency_iter;
    typedef typename boost::graph_traits<GraphContainer>::out_edge_iterator out_edge_iter;
    typedef typename boost::graph_traits<GraphContainer>::in_edge_iterator in_edge_iter;
    
    typedef std::pair<vertex_iter, vertex_iter> vertex_iter_pair_t;
    typedef std::pair<adjacency_iter, adjacency_iter> adjacency_iter_pair_t;
  
  protected:
    GraphContainer graph;
    Vertex root, cur;
    int num_vertices;
    
  public:
    Graph()
    : num_vertices(0)
    {
      // 
      LOG(INFO) << "Graph:: constructed.\n";
    }

    virtual ~Graph() { LOG(INFO) << "Graph:: destructed.\n"; }
    /****************************************  structural  ****************************************/
    Vertex add_vertex(const VERTEX_PROPERTIES& prop)
    {
      Vertex v = boost::add_vertex(graph);
      properties(v) = prop;
      if (num_vertices == 0) {
        root = v;
        cur = v;
      }
      num_vertices++;
      
      return v;
    }
    
    void rm_vertex(const Vertex& v)
    {
      boost::clear_vertex(v, graph);
      boost::remove_vertex(v, graph);
    }
    
    Edge add_directed_edge(const Vertex& vs, const Vertex& vt, const EDGE_PROPERTIES& e_prop)
    {
      Edge e = boost::add_edge(vs, vt, graph).first;
      properties(e) = e_prop;
      
      return e;
    }
    /*******************************************  get/set  ****************************************/
    Vertex& get_cur() { return cur; }
    void set_cur(Vertex v) { cur = v; }
    Vertex get_root() { return root; }
    void set_root(Vertex v) { root = v; }
    
    adjacency_iter_pair_t get_adj_vertices(const Vertex& v) const
    {
      return boost::adjacent_vertices(v, graph);
    }
    
    VERTEX_PROPERTIES& properties(const Vertex& v)
    {
      typename boost::property_map<GraphContainer, vertex_properties_t>::type param = get(vertex_properties, graph);
      return param[v];
    }
  
    const VERTEX_PROPERTIES& properties(const Vertex& v) const
    {
      typename boost::property_map<GraphContainer, vertex_properties_t>::const_type param = get(vertex_properties, graph);
      return param[v];
    }
  
    EDGE_PROPERTIES& properties(const Edge& v)
    {
      typename boost::property_map<GraphContainer, edge_properties_t>::type param = get(edge_properties, graph);
      return param[v];
    }
  
    const EDGE_PROPERTIES& properties(const Edge& v) const
    {
      typename boost::property_map<GraphContainer, edge_properties_t>::const_type param = get(edge_properties, graph);
      return param[v];
    }
    /********************************************  to_str  ****************************************/
    std::string to_str()
    {
      std::stringstream ss;
      vertex_iter_pair_t vertices = boost::vertices(graph);
      for (vertex_iter i = vertices.first; i != vertices.second; i++) {
        ss << vertex_to_str(*i) << "\n";
      }
      
      ss << "\n";
      return ss.str();
    }
    
    std::string vertex_to_str(Vertex v)
    {
      VERTEX_PROPERTIES v_prop = properties(v);
      
      std::stringstream ss;
      ss << "vertex: \n";
      ss << "\t name= " << v_prop.name << "\n";
      ss << "\t key= " << v_prop.key << "\n";
      ss << "\t add_rank= " << v_prop.add_rank << "\n";
      ss << "\t num_visit= " << v_prop.num_visit << "\n";
      ss << "\t status= " << v_prop.status << "\n";
      
      ss << "  out-edges: \n";
      out_edge_iter out_i, out_end;
      Edge e;
      for (boost::tie(out_i, out_end) = out_edges(v, graph); out_i != out_end; ++out_i) {
        e = *out_i;
        ss << "\t src.name= " << properties(boost::source(e, graph)).name << " --> "
           << "\t targ.name= " << properties(boost::target(e, graph)).name << "\n";
      }
      
      ss << "  in-edges: \n";
      in_edge_iter in_i, in_end;
      for (boost::tie(in_i, in_end) = in_edges(v, graph); in_i != in_end; ++in_i) {
        e = *in_i;
        ss << "\t src.name= " << properties(boost::source(e, graph)).name << " --> " 
           << "targ.name= " << properties(boost::target(e, graph)).name << "\n";
      }

      ss << "  adjacent vertices: \n\t";
      adjacency_iter_pair_t ai_pair = get_adj_vertices(v);
      for (adjacency_iter ai = ai_pair.first;  ai != ai_pair.second; ai++) {
        ss << properties(*ai).name <<  ", ";
      }
      ss << "\n";
      
      return ss.str();
    }
    
    std::string to_pretty_str()
    {
      return tree_to_pretty_str(root, 0);
    }
    
    std::string tree_to_pretty_str(Vertex root, int level)
    {
      std::stringstream ss;
      if (level > 0 ) {
        for (int i = 0; i < level-1; i++)
          ss << "   ";
        ss << "|--";
      }
      
      VERTEX_PROPERTIES r_prop = properties(root);
      ss << r_prop.name << ", nv: " << r_prop.num_visit << "\n";
      adjacency_iter_pair_t ai_pair = get_adj_vertices(root);
      
      for (adjacency_iter ai = ai_pair.first;  ai != ai_pair.second; ai++) {
        ss << tree_to_pretty_str(*ai, level + 1);
      }
      
      return ss.str();
    }
};

struct Vertex_Properties {
  std::string name;
  char key;
  int add_rank;
  int num_visit;
  char status;
};

struct Edge_Properties {
  char key;
};

class ParseTree {
  typedef boost::adjacency_list<
    boost::setS, // disallow parallel edges
    boost::listS, // vertex container
    boost::bidirectionalS, // directed graph
    boost::property<vertex_properties_t, Vertex_Properties>,
    boost::property<edge_properties_t, Edge_Properties>
  > GraphContainer;
  typedef boost::graph_traits<GraphContainer>::vertex_descriptor Vertex;
  typedef boost::graph_traits<GraphContainer>::edge_descriptor Edge;
  typedef boost::graph_traits<GraphContainer>::adjacency_iterator adjacency_iter;
  typedef std::pair<adjacency_iter, adjacency_iter> adjacency_iter_pair_t;
    
  private:
    Graph<Vertex_Properties, Edge_Properties> pt_graph;
    int num_access;
    
  public:
    ParseTree()
    : pt_graph(),
      num_access(0)
    {
      Vertex_Properties root_prop;
      root_prop.name = "R";
      root_prop.key = 'r';
      root_prop.add_rank = 0;
      root_prop.num_visit = 0;
      root_prop.status = 'R';
      pt_graph.add_vertex(root_prop);
      // pt_graph.print();
      // 
      LOG(INFO) << "ParseTree:: constructed.";
    }
    
    ~ParseTree() { LOG(INFO) << "ParseTree:: destructed."; }
    
    bool does_vertex_have_key_in_adjs(Vertex& v, char key, Vertex& adj_v)
    {
      adjacency_iter_pair_t ai_pair = pt_graph.get_adj_vertices(v);
      for (adjacency_iter ai = ai_pair.first;  ai != ai_pair.second; ai++) {
        Vertex_Properties adj_prop = pt_graph.properties(*ai);
        if (adj_prop.key == key) {
          adj_v = *ai;
          return true;
        }
      }
      return false;
    }
    
    Vertex create__connect_leaf(Vertex& v, char leaf_key)
    {
      Vertex_Properties& v_prop = pt_graph.properties(v);
      if (v_prop.status == 'L') {
        v_prop.status = 'N';
        // LOG(INFO) << "create__connect_leaf:: status changed from L to N for v.name= " << v_prop.name << "\n";
      }
      
      Vertex_Properties leaf_prop;
      leaf_prop.name = boost::lexical_cast<std::string>(v_prop.name) + boost::lexical_cast<std::string>(leaf_key);
      leaf_prop.key = leaf_key;
      leaf_prop.add_rank = num_access;
      leaf_prop.num_visit = 1;
      leaf_prop.status = 'L';
      Vertex leaf = pt_graph.add_vertex(leaf_prop);
      
      Edge_Properties e_prop;
      e_prop.key = leaf_key;
      pt_graph.add_directed_edge(v, leaf, e_prop);
      
      return leaf;
    }
    
    void move_cur(Vertex v)
    {
      pt_graph.set_cur(v);
      pt_graph.properties(v).num_visit += 1;
    }
    
    int add_access(char key)
    {
      Vertex cur_v = pt_graph.get_cur();
      Vertex_Properties cur_prop = pt_graph.properties(cur_v);
      
      Vertex leaf_to_go;
      if (does_vertex_have_key_in_adjs(cur_v, key, leaf_to_go) ) { //go down
        // LOG(INFO) << "add_access:: cur_prop.name= " << cur_prop.name << " has key= " << key
        //           << " in adj leaf_to_go_prop.name= " << pt_graph.properties(leaf_to_go).name << "\n";
        move_cur(leaf_to_go);
        return 0;
      }
      else {
        create__connect_leaf(cur_v, key);
        
        if (cur_prop.status != 'R') {
          Vertex back_to_root_leaf;
          if (!does_vertex_have_key_in_adjs(cur_v, 'R', back_to_root_leaf) )
            create__connect_leaf(cur_v, 'R');
        }
          
        move_cur(pt_graph.get_root());
        return 0;
      }
      
      // if (cur_prop.status == 'R' || cur_prop.status == 'L') {
      // }
      
      num_access++;
    }
    
    std::string to_str()
    {
      return pt_graph.to_str();
    }
    
    std::string to_pretty_str()
    {
      return pt_graph.to_pretty_str();
    }
};

class LZAlgo {
  private:
    int alphabet_length;
    char* alphabet_;
    // 
    std::vector<char> access_seq_vector;
    ParseTree parse_tree;
    
  public:
    LZAlgo(int alphabet_length, char* alphabet_);
    ~LZAlgo();
    
    int add_access(char key);
    int get_to_prefetch(int num_keys, char* key_);
    
    void print_parse_tree();
    void pprint_parse_tree();
};

#endif // LZPREFETCH_H