#ifndef _PALGORITHM_H_
#define _PALGORITHM_H_

#include <iostream>
#include <vector>
#include <set>
#include <cstdlib>
#include <ctime>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

#include <utility>                   // std::pair
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
// #include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/make_shared.hpp>
#include <boost/config.hpp>
#include <boost/version.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/static_assert.hpp>

#include "patch_pre.h"

/********************************************  Cache  **********************************************/
template <typename ACC_T, typename T>
class Cache {
  typedef boost::function<void(T)> func_handle_del_cb;
  private:
    int cache_size;
    std::deque<T> cache;
    
    std::map<T, ACC_T> e_acc_map;
    std::map<ACC_T, int> acc_num_map;
    
    func_handle_del_cb _handle_del_cb;
  public:
    Cache(int cache_size, func_handle_del_cb _handle_del_cb = 0)
    : cache_size(cache_size),
      _handle_del_cb(_handle_del_cb)
    {
      LOG(INFO) << "Cache:: constructed.";
    }
    
    ~Cache() { LOG(INFO) << "Cache:: destructed."; }
    
    std::string to_str()
    {
      std::stringstream ss;
      ss << "cache_size= " << boost::lexical_cast<std::string>(cache_size) << "\n";
      ss << "acc_num_map= \n" << patch_pre::map_to_str<ACC_T, int>(acc_num_map) << "\n";
      ss << "cache_content= \n";
      for (typename std::deque<T>::iterator it = cache.begin(); it != cache.end(); it++)
        ss << "\t <" << boost::lexical_cast<std::string>(it->first) << ", " << boost::lexical_cast<std::string>(it->second) << "> \n";
      ss << "\n";
      
      return ss.str();
    }
    
    int push(ACC_T acc, T e)
    {
      if (contains(e) )
        return 1;
      
      if (acc_num_map.count(acc) == 0)
        acc_num_map[acc] = 0;
      acc_num_map[acc] += 1;
      
      e_acc_map[e] = acc;
      
      if (cache.size() == cache_size) {
        T e_to_del = cache.front();
        if (_handle_del_cb != 0)
          _handle_del_cb(e_to_del);
        del(e_acc_map[e_to_del], e_to_del);
      }
      cache.push_back(e);
      
      return 0;
    }
    
    int del(ACC_T acc, T e)
    {
      if (!contains(e) )
        return 1;
      
      acc_num_map[acc] -= 1;
      e_acc_map.erase(e_acc_map.find(e) );
      cache.erase(std::find(cache.begin(), cache.end(), e) );
      
      return 0;
    }
    
    bool contains(T e)
    {
      return (std::find(cache.begin(), cache.end(), e) != cache.end() );
    }
    
    int size() { return cache.size(); }
    
    std::vector<T> get_content_v()
    {
      std::vector<T> v;
      for (typename std::deque<T>::iterator it = cache.begin(); it != cache.end(); it++)
        v.push_back(*it);
      
      return v;
    }
    
    std::vector<ACC_T> get_cached_acc_v()
    {
      std::vector<ACC_T> acc_v;
      for (typename std::map<ACC_T, int>::iterator it = acc_num_map.begin(); it != acc_num_map.end(); it++) {
        if (it->second > 0)
          acc_v.push_back(it->first);
      }
      
      return acc_v;
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
    
    void clear()
    {
      std::vector<Vertex> vertex_to_rm_v;
      
      vertex_iter_pair_t vertices = boost::vertices(graph);
      for (vertex_iter i = vertices.first; i != vertices.second; i++)
        vertex_to_rm_v.push_back(*i);
      
      for (typename std::vector<Vertex>::iterator it = vertex_to_rm_v.begin(); it != vertex_to_rm_v.end(); it++)
        rm_vertex(*it);
      // 
      num_vertices = 0;
    }
    // --------------------------------------  structural  -------------------------------------- //
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
#define W_PO 3
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
    
    std::map<KEY_T, int> key__num_arr_map;
    std::map<KEY_T, timeval> key__last_arr_tv_map;
    std::map<KEY_T, float> key__sum_arr_t_map;
  public:
    ParseTree(PREFETCH_T prefetch_t)
    : prefetch_t(prefetch_t),
      pt_graph(),
      longest_phrase_length(0)
    {
      if (prefetch_t == W_PPM)
        context_size = 2;
      else
        context_size = 0;
        
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
    
    void reset()
    {
      pt_graph.clear();
      
      Vertex_Properties root_prop;
      root_prop.name = "R";
      root_prop.key = PARSE_TREE_ROOT_KEY;
      root_prop.num_visit = 0;
      root_prop.status = 'R';
      root_prop.level = 0;
      pt_graph.add_vertex(root_prop);
      // 
      context.clear();
      sliding_win.clear();
      longest_phrase_length = 0;
      key__num_arr_map.clear();
      key__last_arr_tv_map.clear();
      key__sum_arr_t_map.clear();
    }
    
    PREFETCH_T get_prefetch_t() { return prefetch_t; }
    
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
    
    int get_to_prefetch(int& num_keys, std::vector<KEY_T>& key_v)
    {
      std::map<KEY_T, float> key_prob_map;
      get_key_prob_map_for_prefetch(key_prob_map);
      
      // Note: std::map is ordered -- either aphabetical or numerical with keys
      std::map<float, KEY_T> prob_key_map;
      for (std::map<KEY_T, float>::iterator it = key_prob_map.begin(); it != key_prob_map.end(); it++)
        prob_key_map[it->second] = it->first;
      
      for (std::map<float, KEY_T>::reverse_iterator rit = prob_key_map.rbegin(); rit != prob_key_map.rend(); rit++) {
        key_v.push_back(rit->second);
        if (key_v.size() == num_keys)
          break;
      }
      num_keys = key_v.size();
      
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
      else if (prefetch_t == W_PO)
        get_key_prob_map_for_prefetch_w_poisson(key_prob_map);
    }
    
    int add_access(KEY_T key)
    {
      if (prefetch_t == W_LZ)
        return add_access_w_lz(key);
      else if (prefetch_t == W_ALZ)
        return add_access_w_alz(key);
      else if (prefetch_t == W_PPM)
        return add_access_w_ppm(key);
      else if (prefetch_t == W_PO) {
        timeval tv;
        gettimeofday(&tv, NULL);
        if (key__num_arr_map.count(key) == 0)
          key__num_arr_map[key] = 0;
        else {
          // Calculate arr_rate_est to compute the prediction faster
          timeval last_arr_tv = key__last_arr_tv_map[key];
          key__sum_arr_t_map[key] += ( (last_arr_tv.tv_sec - tv.tv_sec)*1000.0 + (last_arr_tv.tv_usec - tv.tv_usec)/1000.0 )/1000.0;
        }
        key__num_arr_map[key] += 1;
        key__last_arr_tv_map[key] = tv;
        
        return 0;
      }
    }
    
    // ----------------------------------------  with_***lz  -------------------------------------- //
    int get_key_prob_map_for_prefetch_w_lz(std::map<KEY_T, float>& key_prob_map)
    {
      Vertex& cur_v = pt_graph.get_cur();
      int total_num_visit = pt_graph.properties(cur_v).num_visit;
      if (pt_graph.properties(cur_v).status != 'R' && prefetch_t != W_PPM)
        total_num_visit -= 1;
      
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
    // --------------------------------------  with_poisson  ------------------------------------ //
    int get_key_prob_map_for_prefetch_w_poisson(std::map<KEY_T, float>& key_prob_map)
    {
      float sum_of_arr_rates = 0;
      for (std::map<KEY_T, int>::iterator it = key__num_arr_map.begin(); it != key__num_arr_map.end(); it++)
        sum_of_arr_rates += (float)it->second / key__sum_arr_t_map[it->first];
      
      for (std::map<KEY_T, int>::iterator it = key__num_arr_map.begin(); it != key__num_arr_map.end(); it++)
        key_prob_map[it->first] = (float)it->second / key__sum_arr_t_map[it->first] / sum_of_arr_rates;
      
      return 0;
    }
};

typedef KEY_T ACC_T;
typedef std::pair<ACC_T, int> acc_step_pair;

typedef int M_PREFETCH_T;
#define W_MP_WEIGHT 0
#define W_MP_MAX 1
class MPrefetchAlgo { // Mixed
  private:
    M_PREFETCH_T m_prefetch_t;
    std::map<PREFETCH_T, float> prefetch_t__weight_map;
    
    std::vector<boost::shared_ptr<ParseTree> > parse_tree_v;
    std::map<int, float> pt_id__weight_map;
    
    std::set<ACC_T> acc_s;
    std::vector<ACC_T> acc_v;
  public:
    MPrefetchAlgo(M_PREFETCH_T m_prefetch_t,
                  std::map<PREFETCH_T, float> prefetch_t__weight_map);
    ~MPrefetchAlgo();
    void reset();
    
    std::vector<ACC_T> get_acc_v();
    int train(std::vector<ACC_T> acc_v);
    int add_access(ACC_T acc);
    int get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v);
    
    int get_to_prefetch_w_max(int& num_acc, std::vector<ACC_T>& acc_v);
    int get_to_prefetch_w_weight(int& num_acc, std::vector<ACC_T>& acc_v);
};

class PrefetchAlgo {
  protected:
    ParseTree parse_tree;
    std::set<ACC_T> acc_s;
    std::vector<ACC_T> acc_v;
    
  public:
    PrefetchAlgo(PREFETCH_T prefetch_t, int context_size);
    ~PrefetchAlgo();
    void reset();
    
    std::string parse_tree_to_pstr();
    std::vector<ACC_T> get_acc_v();
    int train(std::vector<ACC_T> acc_v);
    int add_access(ACC_T acc);
    int get_acc_prob_map_for_prefetch(std::map<ACC_T, float>& acc_prob_map); // TODO: will be deprecated
    int get_to_prefetch(int& num_acc, std::vector<ACC_T>& acc_v,
                        const std::vector<ACC_T>& cached_acc_v, std::vector<ACC_T>& eacc_v);
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
    PPMAlgo(int context_size);
    ~PPMAlgo();
};

class POAlgo : public PrefetchAlgo {
  public:
    POAlgo();
    ~POAlgo();
};

template <typename PALGO_T>
void sim_prefetch_accuracy(PALGO_T& palgo,
                           int cache_size, std::vector<acc_step_pair> acc_step_v, 
                           float& hit_rate, std::vector<char>& accuracy_v )
{
  Cache<ACC_T, acc_step_pair> cache(cache_size, boost::function<void(acc_step_pair)>() );
  int num_miss = 0;
  
  std::map<ACC_T, int> acc__last_acced_step_map;
  
  for (std::vector<acc_step_pair>::iterator it = acc_step_v.begin(); it != acc_step_v.end(); it++) {
    // std::cout << "sim_prefetch_accuracy:: is <" << it->first << ", " << it->second << ">"
    //           << " in the cache= \n" << cache.to_str() << "\n";
    acc__last_acced_step_map[it->first] = it->second;
    
    if (!cache.contains(*it) ) {
      accuracy_v.push_back('f');
      num_miss++;
    }
    else
      accuracy_v.push_back('-');
    
    // In wa-dataspaces scenario data is used only once
    cache.del(it->first, *it);
    
    palgo.add_access(it->first); // Reg only the acc
    
    int num_acc = 1; //cache_size;
    std::vector<ACC_T> acc_v, eacc_v;
    palgo.get_to_prefetch(num_acc, acc_v, std::vector<ACC_T>(), eacc_v);
    
    // Update cache
    for (std::vector<ACC_T>::iterator iit = acc_v.begin(); iit != acc_v.end(); iit++)
      cache.push(*iit, std::make_pair(*iit, acc__last_acced_step_map[*iit] + 1) );
  }
  
  hit_rate = 1.0 - (float)num_miss / acc_step_v.size();
}

#endif // _PALGORITHM_H_