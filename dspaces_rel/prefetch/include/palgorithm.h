#ifndef _PALGORITHM_H_
#define _PALGORITHM_H_

#include <iostream>
#include <vector>
#include <stdlib.h>     // srand, rand
#include <time.h>       // time
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

#include <utility>                   // std::pair
#include <algorithm>                 // std::for_each
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
// #include <boost/graph/dijkstra_shortest_paths.hpp>

#include <boost/config.hpp>
#include <boost/version.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/static_assert.hpp>

/********************************************  Cache  **********************************************/
template <typename T>
class Cache {
  private:
    size_t cache_size;
    std::deque<T> cache;
  public:
    Cache(size_t cache_size) 
    : cache_size(cache_size)
    {
      LOG(INFO) << "Cache:: constructed.";
    }
    
    ~Cache() { LOG(INFO) << "Cache:: destructed."; }
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "cache_size= " << boost::lexical_cast<std::string>(cache_size) << "\n";
      ss << "cache_content= \n";
      typename std::deque<T>::iterator it;
      for (it = cache.begin(); it != cache.end(); it++) {
        ss << "<" << boost::lexical_cast<std::string>(it->first) << ", " << boost::lexical_cast<std::string>(it->second) << "> \n";
      }
      ss << "\n";
      
      return ss.str();
    }
    
    int push(T key)
    {
      if (contains(key) ) {
        // LOG(ERROR) << "push:: already existing key!";
        return 1;
      }
        
      if (cache.size() == cache_size)
        cache.pop_front();
      
      cache.push_back(key);
      
      return 0;
    }
    
    int del(T key)
    {
      if (!contains(key) ) {
        // LOG(ERROR) << "del:: non-existing key!";
        return 1;
      }
      cache.erase(std::find(cache.begin(), cache.end(), key) );
      
      return 0;
    }
    
    bool contains(T key)
    {
      return (std::find(cache.begin(), cache.end(), key) != cache.end() );
    }
    
    size_t size() { return cache.size(); }
    
    std::vector<T> get_content_vec()
    {
      std::vector<T> content_vec;
      for (typename std::deque<T>::iterator it = cache.begin(); it != cache.end(); it++)
        content_vec.push_back(*it);
        
      return content_vec;
    }
};

// Definition of basic boost::graph properties
enum vertex_properties_t { vertex_properties };
enum edge_properties_t { edge_properties };
namespace boost {
  BOOST_INSTALL_PROPERTY(vertex, properties);
  BOOST_INSTALL_PROPERTY(edge, properties);
}

/*******************************************  Graph  **********************************************/
template <typename VERTEX_PROPERTIES, typename EDGE_PROPERTIES>
class Graph // The graph base class template
{
  typedef boost::adjacency_list<
    boost::setS, // disallow parallel edges
    boost::listS, // vertex container
    boost::bidirectionalS, // directed graph
    boost::property<vertex_properties_t, VERTEX_PROPERTIES>,
    boost::property<edge_properties_t, EDGE_PROPERTIES>
  > GraphContainer;
    // A bunch of graph-specific typedefs
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
    Vertex root, pre_cur, cur;
    int num_vertices;
    
  public:
    Graph()
    : num_vertices(0)
    {
      // 
      LOG(INFO) << "Graph:: constructed.\n";
    }
    
    ~Graph() { LOG(INFO) << "Graph:: destructed.\n"; }
    // ----------------------------------------  structural  ---------------------------------------- //
    Vertex add_vertex(const VERTEX_PROPERTIES& prop)
    {
      Vertex v = boost::add_vertex(graph);
      properties(v) = prop;
      if (num_vertices == 0) {
        root = v;
        cur = v;
        pre_cur = v;
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
    // -----------------------------------------  get/set  -------------------------------------- //
    Vertex& get_cur() { return cur; }
    
    void set_cur(Vertex v) { cur = v; }
    
    Vertex& get_pre_cur() { return pre_cur; }
    
    void set_pre_cur(Vertex v) { pre_cur = v; }
    
    Vertex& get_root() { return root; }
    
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
    // -------------------------------------------  to_str  ------------------------------------- //
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
      ss << "\t level= " << v_prop.level << "\n";
      
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
      ss << r_prop.name << ", nv: " << r_prop.num_visit;
      if ( (properties(cur).name).compare(r_prop.name) == 0) // At cur
        ss << " <<<";
      ss << "\n";
      
      adjacency_iter_pair_t ai_pair = get_adj_vertices(root);
      
      for (adjacency_iter ai = ai_pair.first;  ai != ai_pair.second; ai++) {
        ss << tree_to_pretty_str(*ai, level + 1);
      }
      
      return ss.str();
    }
    
};

// Note: has to change key type to whatever KEY_T set for ParseTree, making these properties template does not work with boost::property
// typedef char KEY_T
typedef int KEY_T;
#define PARSE_TREE_ROOT_KEY -2
#define BACK_TO_ROOT_LEAF_KEY -1

struct Vertex_Properties {
  std::string name;
  int key;
  int add_rank;
  int num_visit;
  char status;
  int level;
};

struct Edge_Properties {
  int key;
};

#define RANDOM_INT_RANGE 1000

class ParseTree {
  typedef boost::adjacency_list<
    boost::setS, // disallow parallel edges
    boost::listS, // vertex container
    boost::bidirectionalS, // directed graph
    boost::property<vertex_properties_t, Vertex_Properties>,
    boost::property<edge_properties_t, Edge_Properties >
  > GraphContainer;
  typedef boost::graph_traits<GraphContainer>::vertex_descriptor Vertex;
  typedef boost::graph_traits<GraphContainer>::edge_descriptor Edge;
  typedef boost::graph_traits<GraphContainer>::adjacency_iterator adjacency_iter;
  typedef std::pair<adjacency_iter, adjacency_iter> adjacency_iter_pair_t;
    
  // Note: root level is 0
  private:
    Graph<Vertex_Properties, Edge_Properties> pt_graph;
    int num_access;
    bool with_context;
    int context_size;
    std::deque<KEY_T> context;
    
  public:
    ParseTree(bool with_context, int context_size)
    : pt_graph(),
      num_access(0),
      with_context(with_context),
      context_size(context_size)
    {
      Vertex_Properties root_prop;
      root_prop.name = "R";
      root_prop.key = PARSE_TREE_ROOT_KEY;
      root_prop.add_rank = 0;
      root_prop.num_visit = 0;
      root_prop.status = 'R';
      root_prop.level = 0;
      pt_graph.add_vertex(root_prop);
      // pt_graph.print();
      // 
      LOG(INFO) << "ParseTree:: constructed.";
    }
    ~ParseTree() { LOG(INFO) << "ParseTree:: destructed."; }
    
    std::string to_str() { return pt_graph.to_str(); }
    
    std::string to_pretty_str() { return pt_graph.to_pretty_str(); }
    
    bool does_vertex_have_key_in_adjs(Vertex& v, KEY_T key, Vertex& adj_v)
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
    
    int add_access(KEY_T key)
    {
      if (with_context)
        add_access_with_context(key);
      else
        add_access_without_context(key);
    }
    
    int get_key_prob_map_for_prefetch(std::map<KEY_T, float>& key_prob_map)
    {
      if (with_context)
        get_key_prob_map_for_prefetch_with_context(key_prob_map);
      else
        get_key_prob_map_for_prefetch_without_context(key_prob_map);
    }
    
    Vertex create__connect_leaf(Vertex& v, KEY_T leaf_key)
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
      leaf_prop.level = v_prop.level + 1;
      Vertex leaf = pt_graph.add_vertex(leaf_prop);
      
      Edge_Properties e_prop;
      e_prop.key = leaf_key;
      pt_graph.add_directed_edge(v, leaf, e_prop);
      
      return leaf;
    }
    
    void move_cur(bool inc_num_visit, Vertex v)
    {
      pt_graph.set_pre_cur(pt_graph.get_cur() );
      pt_graph.set_cur(v);
      if (inc_num_visit)
        (pt_graph.properties(v) ).num_visit += 1;
    }
    
    int add_access_without_context(KEY_T key)
    {
      LOG(INFO) << "*** add_access_without_context:: called with key= " << key;
      
      Vertex cur_v = pt_graph.get_cur();
      Vertex_Properties cur_prop = pt_graph.properties(cur_v);
      
      Vertex leaf_to_go;
      if (does_vertex_have_key_in_adjs(cur_v, key, leaf_to_go) ) { //go down
        // LOG(INFO) << "add_access:: cur_prop.name= " << cur_prop.name << " has key= " << key
        //           << " in adj leaf_to_go_prop.name= " << pt_graph.properties(leaf_to_go).name << "\n";
        move_cur(true, leaf_to_go);
      }
      else {
        create__connect_leaf(cur_v, key);
        
        if (cur_prop.status != 'R') {
          Vertex back_to_root_leaf;
          if (!does_vertex_have_key_in_adjs(cur_v, BACK_TO_ROOT_LEAF_KEY, back_to_root_leaf) )
            create__connect_leaf(cur_v, BACK_TO_ROOT_LEAF_KEY);
        }
        move_cur(true, pt_graph.get_root() );
      }
      
      num_access++;
      return 0;
    }
    
    int get_to_prefetch(size_t& num_keys, KEY_T*& keys_)
    {
      std::map<KEY_T, float> key_prob_map;
      get_key_prob_map_for_prefetch(key_prob_map);
      
      if (num_keys > key_prob_map.size() ) {
        // LOG(WARNING) << "get_to_prefetch:: num_keys= " << num_keys << " > key_prob_map.size()= " 
        //             << key_prob_map.size() << "! Updating num_keys= " << key_prob_map.size();
        num_keys = key_prob_map.size();
      }
      // Note: std::map is ordered -- either aphabetical or numerical with keys
      std::map<float, KEY_T> prob_key_map;
      for (std::map<KEY_T, float>::iterator it = key_prob_map.begin(); it != key_prob_map.end(); it++)
        prob_key_map[it->second] = it->first;
      
      keys_ = (KEY_T*) malloc(num_keys*sizeof(KEY_T) );
      std::map<float, KEY_T>::reverse_iterator rit = prob_key_map.rbegin();
      for (int i = 0; i < num_keys; i++) {
        keys_[i] = rit->second;
        rit++;
      }
      
      return 0;
    }
    
    int get_key_prob_map_for_prefetch_without_context(std::map<KEY_T, float>& key_prob_map)
    {
      Vertex& cur_v = pt_graph.get_cur();
      int total_num_visit = pt_graph.properties(cur_v).num_visit;
      
      srand (time(NULL) );
      adjacency_iter_pair_t ai_pair = pt_graph.get_adj_vertices(cur_v);
      for (adjacency_iter ai = ai_pair.first;  ai != ai_pair.second; ai++) {
        Vertex_Properties adj_prop = pt_graph.properties(*ai);
        
        int random_int = rand() % RANDOM_INT_RANGE;
        float random_float = (float)random_int/(RANDOM_INT_RANGE*RANDOM_INT_RANGE);
        
        key_prob_map[adj_prop.key] = (float)adj_prop.num_visit/total_num_visit + random_float;
      }
      
      if (key_prob_map.empty() || (key_prob_map.size() == 1 && (key_prob_map.begin())->first == BACK_TO_ROOT_LEAF_KEY) ) { // Prefetch according to root
        Vertex pt_root = pt_graph.get_root(); // pt_graph.get_pre_cur();
        
        key_prob_map.clear();
        total_num_visit = pt_graph.properties(pt_root).num_visit + 1;
        
        ai_pair = pt_graph.get_adj_vertices(pt_root);
        for (adjacency_iter ai = ai_pair.first;  ai != ai_pair.second; ai++) {
          Vertex_Properties adj_prop = pt_graph.properties(*ai);
          
          int random_int = rand() % RANDOM_INT_RANGE;
          float random_float = (float)random_int/(RANDOM_INT_RANGE*RANDOM_INT_RANGE);
          
          key_prob_map[adj_prop.key] = (float)adj_prop.num_visit/total_num_visit + random_float;
        }
      }
      
      if (key_prob_map.count(BACK_TO_ROOT_LEAF_KEY) > 0)
        key_prob_map.erase(key_prob_map.find(BACK_TO_ROOT_LEAF_KEY) );
      
      return 0;
    }
    // -------------------------------------  with_context  ------------------------------------- //
    int get_key_prob_map_for_prefetch_with_context(std::map<KEY_T, float>& key_prob_map)
    {
      move_cur_on_context(false); // even if whole walk could not be finished, predict with smaller context
      // LOG(INFO) << "get_key_prob_map_for_prefetch_with_context:: cur= \n" << pt_graph.vertex_to_str(pt_graph.get_cur() );
      get_key_prob_map_for_prefetch_without_context(key_prob_map);
      move_cur(false, pt_graph.get_root() );
    }
    
    int add_access_with_context(KEY_T key)
    {
      // LOG(INFO) << "add_access_with_context:: context= " << context_to_str();
      if (context.size() == context_size) {
        // check if current context is present in the tree, if not create, move down and encode access
        create__move_cur_on_context(move_cur_on_context(true) );
        
        Vertex leaf_to_go;
        if (does_vertex_have_key_in_adjs(pt_graph.get_cur(), key, leaf_to_go) )
          move_cur(true, leaf_to_go);
        else
          create__connect_leaf(pt_graph.get_cur(), key);
        
        move_cur(true, pt_graph.get_root() );
        
        context.pop_front();
      }
      context.push_back(key);
      
      return 0;
    }
    
    std::string context_to_str()
    {
      std::stringstream ss;
      for (std::deque<KEY_T>::iterator it = context.begin(); it != context.end(); it++) {
        ss << boost::lexical_cast<std::string>(*it) << ",";
      }
      
      return ss.str();
    }
    
    int move_cur_on_context(bool inc_num_visit)
    {
      int walked_upto_index_on_context = 0;
      for (int i = 0; i < context.size(); i++) {
        Vertex leaf_to_go;
        if (does_vertex_have_key_in_adjs(pt_graph.get_cur(), context[i], leaf_to_go) ) { //go down
          move_cur(inc_num_visit, leaf_to_go);
          walked_upto_index_on_context++;
        }
        else {
          // LOG(INFO) << "move_cur_on_context:: could not walk on context= " << context_to_str();
          return walked_upto_index_on_context;
        }
      }
      // LOG(INFO) << "move_cur_on_context:: could walk on context= " << context_to_str();
      return walked_upto_index_on_context;
    }
    
    void create__move_cur_on_context(int walked_upto_index_on_context)
    {
      Vertex cur_v = pt_graph.get_cur();
      Vertex_Properties cur_prop = pt_graph.properties(cur_v);
      // LOG(WARNING) << "create__move_cur_on_context:: cur= \n" << pt_graph.vertex_to_str(cur_v);
      
      for (int i = walked_upto_index_on_context; i < context.size(); i++) {
        KEY_T leaf_key = context[i];
        Vertex_Properties leaf_prop;
        leaf_prop.name = boost::lexical_cast<std::string>(cur_prop.name) + boost::lexical_cast<std::string>(leaf_key);
        leaf_prop.key = leaf_key;
        leaf_prop.add_rank = num_access;
        leaf_prop.num_visit = 0;
        if (i != context.size() - 1)
          leaf_prop.status = 'N';
        else
          leaf_prop.status = 'L';
        leaf_prop.level = cur_prop.level + 1;
        Vertex leaf = pt_graph.add_vertex(leaf_prop);
        
        Edge_Properties e_prop;
        e_prop.key = leaf_key;
        pt_graph.add_directed_edge(cur_v, leaf, e_prop);
        
        move_cur(true, leaf);
        cur_v = pt_graph.get_cur();
        cur_prop = pt_graph.properties(cur_v);
      }
    }
};

class PrefetchAlgo {
  protected:
    std::vector<KEY_T> access_vec;
    ParseTree parse_tree;
  public:
    PrefetchAlgo(bool with_context, size_t context_size);
    ~PrefetchAlgo();
    
    std::string parse_tree_to_str();
    std::string parse_tree_to_pstr();
    
    std::string access_seq_to_str();
    std::vector<KEY_T> get_access_vec();
    
    int add_access(KEY_T key);
    // TODO: will be deprecated
    int get_key_prob_map_for_prefetch(std::map<KEY_T, float>& key_prob_map);
    int get_to_prefetch(size_t& num_keys, KEY_T*& keys_);
    int sim_prefetch_accuracy(float& hit_rate, size_t cache_size, std::vector<KEY_T> access_seq_v, std::vector<char>& accuracy_seq_v);
};

class LZAlgo : public PrefetchAlgo {
  public:
    LZAlgo();
    ~LZAlgo();
};

class PPMAlgo : public PrefetchAlgo {
  public:
    PPMAlgo(size_t context_size);
    ~PPMAlgo();
};

#endif // _PALGORITHM_H_