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
    Vertex get_cur() { return cur; }
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
    void print()
    {
      LOG(INFO) << "print:: printing graph...";
      vertex_iter_pair_t vertices = boost::vertices(graph);
      for (vertex_iter i = vertices.first; i != vertices.second; i++) {
        std::cout << vertex_to_str(*i) << "\n";
      }
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

      ss << "  adjacent vertices: \n";
      adjacency_iter_pair_t ai_pair = get_adj_vertices(v);
      for (adjacency_iter ai = ai_pair.first;  ai != ai_pair.second; ai++) {
        ss << properties(*ai).name <<  ", ";
      }
      ss << "\n";
      
      return ss.str();
    }
};

struct Vertex_Properties {
  std::string name;
  char key;
  int add_rank;
  int num_visit;
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
  typedef typename boost::graph_traits<GraphContainer>::adjacency_iterator adjacency_iter;
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
      root_prop.name = "root";
      root_prop.key = 'R';
      root_prop.add_rank = 0;
      root_prop.num_visit = 0;
      pt_graph.add_vertex(root_prop);
      // pt_graph.print();
      // 
      LOG(INFO) << "ParseTree:: constructed.";
    }
    
    ~ParseTree() { LOG(INFO) << "ParseTree:: destructed."; }
    
    bool does_vertex_have_key_in_adjs(Vertex v, char key, Vertex& adj_v)
    {
      adjacency_iter_pair_t ai_pair = get_adj_vertices(v);
      for (adjacency_iter ai = ai_pair.first;  ai != ai_pair.second; ai++) {
        Vertex_Properties adj_prop = pt_graph.properties(*ai);
        if (adj_prop.key == key) {
          adj_v = *ai;
          return true;
        }
      }
      return false;
    }
    
    void create__connect_leaf(Vertex v, std::string leaf_name, char leaf_key)
    {
      Vertex_Properties leaf_prop;
      leaf_prop.name = leaf_name;
      leaf_prop.key = leaf_key;
      leaf_prop.add_rank = num_access;
      leaf_prop.num_visit = 0;
      Vertex leaf = pt_graph.add_vertex(leaf_prop);
      
      Edge_Properties e_prop;
      e_prop.key = leaf_key;
      pt_graph.add_directed_edge(v, leaf, e_prop);
    }
    
    void move_cur(Vertex v)
    {
      pt_graph.set_cur(v);
      pt_graph.properties(v).num_visit += 1;
    }
    
    int add_access(char key)
    {
      Vertex cur_v = pt_graph.get_cur();
      Vertex_Properties cur_properties = pt_graph.properties(cur_v);
      if (cur_properties.key == 'R') {
        Vertex leaf_to_go;
        if (does_vertex_have_key_in_adjs(cur_v, key, leaf_to_go) ) { //go down
          move_cur(leaf_to_go);
          return 0;
        }
        else {
          create__connect_leaf(cur_v, boost::lexical_cast<std::string>(key), key);
          move_cur(new_leaf);
          return 0;
        }
        
      }
      else {
        
        if (cur_properties.num_visit == 1) { //no leaf
          Vertex_Properties v_prop;
          v_prop.name = boost::lexical_cast<std::string>(cur_properties.key) + boost::lexical_cast<std::string>(key);
          v_prop.key = key;
          v_prop.add_rank = num_access;
          v_prop.num_visit = 1;
          Vertex new_leaf = pt_graph.add_vertex(v_prop);
          
          Edge_Properties e_prop;
          e_prop.key = key;
          pt_graph.add_directed_edge(cur_v(), new_leaf, e_prop);
          
          pt_graph.set_cur(pt_graph.get_root() );
          return 0;
        }
        else {
          if (does_vertex_have_key_in_adjs(cur_v(), key) ) { //go down
            
          }
        }
      }
      
      num_access++;
    }
};

class LZAlgo {
  public:
    LZAlgo(int alphabet_length, char* alphabet_);
    
    int add_access(char key);
    int get_to_prefetch(int num_keys, char* key_);
    
    void print_parse_tree();
  private:
    int alphabet_length;
    char* alphabet_;
    // 
    std::vector<char> access_seq_vector;
    ParseTree parse_tree;
};

#endif // LZPREFETCH_H