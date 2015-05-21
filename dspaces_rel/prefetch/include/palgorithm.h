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
    
    std::map<Vertex, Vertex> ver__up_ver_map;
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
      
      ver__up_ver_map[vt] = vs;
      
      return e;
    }
    // -----------------------------------------  get/set  -------------------------------------- //
    Vertex& get_cur() { return cur; }
    
    void set_cur(Vertex v) { cur = v; }
    
    Vertex& get_pre_cur() { return pre_cur; }
    
    void set_pre_cur(Vertex v) { pre_cur = v; }
    
    Vertex& get_root() { return root; }
    
    void set_root(Vertex v) { root = v; }
    
    Vertex& get_up_ver(const Vertex& v)
    {
      return ver__up_ver_map[v];
    }
    
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
  int num_visit;
  char status;
  int level;
};

struct Edge_Properties {
  int key;
};

typedef int PREFETCH_T;
#define W_LZ 0
#define W_ALZ 1
#define W_PPM 2
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
  
  private:
    PREFETCH_T prefetch_t;
    int context_size;
  
    Graph<Vertex_Properties, Edge_Properties> pt_graph;
    std::deque<KEY_T> context;
    
    std::deque<KEY_T> sliding_win;
    int longest_phrase_length;
  public:
    // Note: context_size matters only for PPM Algo
    ParseTree(PREFETCH_T prefetch_t, int context_size)
    : prefetch_t(prefetch_t), context_size(context_size),
      pt_graph(),
      longest_phrase_length(0)
    {
      // Note: root level is 0
      Vertex_Properties root_prop;
      root_prop.name = "R";
      root_prop.key = PARSE_TREE_ROOT_KEY;
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
    
    int get_key_prob_map_for_prefetch(std::map<KEY_T, float>& key_prob_map)
    {
      if (prefetch_t == W_LZ)
        get_key_prob_map_for_prefetch_w_lz(key_prob_map);
      else if (prefetch_t == W_ALZ)
        get_key_prob_map_for_prefetch_w_lz(key_prob_map);
        // get_key_prob_map_for_prefetch_w_alz(key_prob_map);
      else if (prefetch_t == W_PPM)
        get_key_prob_map_for_prefetch_w_ppm(key_prob_map);
    }
    
    int add_access(KEY_T key)
    {
      if (prefetch_t == W_LZ)
        add_access_w_lz(key);
      else if (prefetch_t == W_ALZ)
        add_access_w_alz(key);
      else if (prefetch_t == W_PPM)
        add_access_w_ppm(key);
    }
    
    // ----------------------------------------  with_***lz  -------------------------------------- //
    int get_key_prob_map_for_prefetch_w_lz(std::map<KEY_T, float>& key_prob_map)
    {
      Vertex& cur_v = pt_graph.get_cur();
      int total_num_visit = pt_graph.properties(cur_v).num_visit;
      if (pt_graph.properties(cur_v).status != 'R')
        total_num_visit -= 1;
      
      srand(time(NULL) );
      adjacency_iter_pair_t ai_pair = pt_graph.get_adj_vertices(cur_v);
      for (adjacency_iter ai = ai_pair.first;  ai != ai_pair.second; ai++) {
        Vertex_Properties adj_prop = pt_graph.properties(*ai);
        key_prob_map[adj_prop.key] = (float)adj_prop.num_visit/total_num_visit + (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) / 100);
      }
      
      if (key_prob_map.empty() || (key_prob_map.size() == 1 && (key_prob_map.begin() )->first == BACK_TO_ROOT_LEAF_KEY) ) { // Prefetch according to root
        Vertex pt_root = pt_graph.get_root(); // pt_graph.get_pre_cur();
        
        key_prob_map.clear();
        total_num_visit = pt_graph.properties(pt_root).num_visit + 1;
        
        ai_pair = pt_graph.get_adj_vertices(pt_root);
        for (adjacency_iter ai = ai_pair.first;  ai != ai_pair.second; ai++) {
          Vertex_Properties adj_prop = pt_graph.properties(*ai);
          key_prob_map[adj_prop.key] = (float)adj_prop.num_visit/total_num_visit + (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) / 100);
        }
      }
      
      if (key_prob_map.count(BACK_TO_ROOT_LEAF_KEY) > 0)
        key_prob_map.erase(key_prob_map.find(BACK_TO_ROOT_LEAF_KEY) );
      
      return 0;
    }
    
    int get_key_prob_map_for_prefetch_w_alz(std::map<KEY_T, float>& key_prob_map)
    {
      srand(time(NULL) );
      get_key__blended_prob_map(pt_graph.get_cur(), 0, 1.0, key_prob_map);
      
      return 0;
    }
    
    void get_key__blended_prob_map(Vertex& v, int call_index, float prob_index, std::map<KEY_T, float>& key__blended_prob_map)
    {
      Vertex_Properties v_prop = pt_graph.properties(v);
      int total_num_visit = v_prop.num_visit;
      if (v_prop.status == 'R') {
        if (call_index > 0)
          total_num_visit += 1;
      }
      else { 
        if (call_index == 0)
          total_num_visit -= 1;
      }
      
      adjacency_iter_pair_t ai_pair = pt_graph.get_adj_vertices(v);
      int num_adj = 0;
      for (adjacency_iter ai = ai_pair.first;  ai != ai_pair.second; ai++) {
        Vertex_Properties adj_prop = pt_graph.properties(*ai);
        
        if (adj_prop.key != BACK_TO_ROOT_LEAF_KEY) {
          if (key__blended_prob_map.count(adj_prop.key) == 0)
            key__blended_prob_map[adj_prop.key] = prob_index * ( (float)adj_prop.num_visit/total_num_visit + (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) / 100) );
          else
            key__blended_prob_map[adj_prop.key] += prob_index * ( (float)adj_prop.num_visit/total_num_visit + (static_cast<float>(rand() ) / static_cast<float>(RAND_MAX) / 100) );
        }
        else { // Update key__blended_prob_map with the lower-order state models -- call the function with upper level vertex
          if (v_prop.status != 'R') {
            prob_index *= (float)adj_prop.num_visit/total_num_visit;
            // LOG(INFO) << "get_key__blended_prob_map:: #1 calling for vertex_to_str= \n" << pt_graph.vertex_to_str(pt_graph.get_up_ver(v) );
            // return;
            get_key__blended_prob_map(pt_graph.get_up_ver(v), ++call_index, prob_index, key__blended_prob_map);
          }
        }
        num_adj++;
      }
      
      if (num_adj == 0) { // Can happen when the cur_v is on the vertices connected to the root
        if (v_prop.status != 'R') {
          // LOG(INFO) << "get_key__blended_prob_map:: #2 calling for vertex_to_str= \n" << pt_graph.vertex_to_str(pt_graph.get_up_ver(v) );
          get_key__blended_prob_map(pt_graph.get_up_ver(v), ++call_index, prob_index, key__blended_prob_map);
        }
      }
    }
    
    int add_access_w_lz(KEY_T key)
    {
      Vertex cur_v = pt_graph.get_cur();
      Vertex_Properties cur_prop = pt_graph.properties(cur_v);
      
      Vertex leaf_to_go;
      if (does_vertex_have_key_in_adjs(cur_v, key, leaf_to_go) ) { //Go down
        // LOG(INFO) << "add_access:: cur_prop.name= " << cur_prop.name << " has key= " << key
        //           << " in adj leaf_to_go_prop.name= " << pt_graph.properties(leaf_to_go).name << "\n";
        move_cur(true, leaf_to_go);
      }
      else {
        Vertex leaf = create__connect_leaf(cur_v, key);
        
        // if (cur_prop.status != 'R') {
        //   Vertex back_to_root_leaf;
        //   if (!does_vertex_have_key_in_adjs(cur_v, BACK_TO_ROOT_LEAF_KEY, back_to_root_leaf) )
        //     create__connect_leaf(cur_v, BACK_TO_ROOT_LEAF_KEY);
        // }
        Vertex back_to_root_leaf;
        if (!does_vertex_have_key_in_adjs(leaf, BACK_TO_ROOT_LEAF_KEY, back_to_root_leaf) )
          create__connect_leaf(leaf, BACK_TO_ROOT_LEAF_KEY);
        
        move_cur(true, pt_graph.get_root() );
      }
      
      return 0;
    }
    
    int add_access_w_alz(KEY_T key)
    {
      if (add_access_w_lz(key) )
        return 1;
      Vertex _cur_v = pt_graph.get_cur();
      
      // Update sliding_win
      if (sliding_win.size() < 1) // longest_phrase_length
        sliding_win.push_back(key);
      else {
        sliding_win.pop_front();
        sliding_win.push_back(key);
      }
      
      // Add for all possible phrases from sliding_win
      // std::cout << "add_access_w_alz:: before updating sliding_win= ";
      // for (std::deque<KEY_T>::iterator it = sliding_win.begin(); it != sliding_win.end(); it++)
      //   std::cout << *it << ", ";
      // std::cout << "\n";
      
      // std::deque<KEY_T>::iterator it = sliding_win.begin();
      // for (int context_size = 1; context_size <= sliding_win.size(); context_size++) {
      //   move_cur(false, pt_graph.get_root() );
      //   for (int i = 0; i < context_size; i++) {
      //     Vertex cur_v = pt_graph.get_cur();
          
      //     Vertex leaf_to_go;
      //     if (does_vertex_have_key_in_adjs(cur_v, *(it + i), leaf_to_go) ) //Go down
      //       move_cur(false, leaf_to_go);
      //     else
      //       move_cur(false, create__connect_leaf(cur_v, *(it + i) ) );
      //   }
      // }
      
      // if (pt_graph.properties(_cur_v).status == 'R') {
      if (longest_phrase_length > 1 && pt_graph.properties(_cur_v).status == 'R') {
        move_cur(true, pt_graph.get_root() );
        
        Vertex cur_v = pt_graph.get_cur();
        for (std::deque<KEY_T>::iterator it = sliding_win.begin(); it != sliding_win.end(); it++) {
          Vertex leaf_to_go;
          if (does_vertex_have_key_in_adjs(cur_v, *it, leaf_to_go) ) //Go down
            move_cur(true, leaf_to_go);
          else
            move_cur(false, create__connect_leaf(cur_v, *it) );
            
          cur_v = pt_graph.get_cur();
        }
        
        Vertex back_to_root_leaf;
        if (does_vertex_have_key_in_adjs(cur_v, BACK_TO_ROOT_LEAF_KEY, back_to_root_leaf) )
          move_cur(true, back_to_root_leaf);
        else
          move_cur(false, create__connect_leaf(cur_v, BACK_TO_ROOT_LEAF_KEY) );
      }
      
      move_cur(false, _cur_v);
      
      return 0;
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
      leaf_prop.num_visit = 1;
      leaf_prop.status = 'L';
      leaf_prop.level = v_prop.level + 1;
      if (leaf_key != BACK_TO_ROOT_LEAF_KEY && leaf_prop.level > longest_phrase_length)
        longest_phrase_length = leaf_prop.level;
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
    
    // ----------------------------------------  with_ppm  -------------------------------------- //
    int get_key_prob_map_for_prefetch_w_ppm(std::map<KEY_T, float>& key_prob_map)
    {
      move_cur_on_context(false); // even if whole walk could not be finished, predict with smaller context
      // LOG(INFO) << "get_key_prob_map_for_prefetch_w_ppm:: cur= \n" << pt_graph.vertex_to_str(pt_graph.get_cur() );
      get_key_prob_map_for_prefetch_w_lz(key_prob_map);
      move_cur(false, pt_graph.get_root() );
    }
    
    int add_access_w_ppm(KEY_T key)
    {
      // LOG(INFO) << "add_access_w_ppm:: context= " << context_to_str();
      if (context.size() == context_size) {
        // Check if current context is present in the tree, if not create, move down and encode access
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
    
    std::map<KEY_T, float> key__last_arr_time_map;
  public:
    PrefetchAlgo(PREFETCH_T prefetch_t, size_t context_size);
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

class ALZAlgo : public PrefetchAlgo {
  public:
    ALZAlgo();
    ~ALZAlgo();
};

class PPMAlgo : public PrefetchAlgo {
  public:
    PPMAlgo(size_t context_size);
    ~PPMAlgo();
};

#endif // _PALGORITHM_H_