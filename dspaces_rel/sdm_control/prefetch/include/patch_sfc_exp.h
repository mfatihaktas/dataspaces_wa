#ifndef _PATCH_SFC_EXP_H_
#define _PATCH_SFC_EXP_H_

/***************************************  Check Hilbert  ******************************************/
template<typename T>
void make_2dscatter_plot(std::vector<T> x_v, std::vector<T> y_v,
                         std::string x_label, std::string y_label,
                         std::string plot_title, std::string out_url)
{
  Gnuplot gp;
  
  T min_x = *std::min_element(x_v.begin(), x_v.end() );
  T max_x = *std::max_element(x_v.begin(), x_v.end() );
  
  T min_y = *std::min_element(y_v.begin(), y_v.end() );
  T max_y = *std::max_element(y_v.begin(), y_v.end() );
  
  if (out_url.compare("") != 0) {
    gp << "set term png enhanced font '/usr/share/fonts/dejavu/DejaVuSans.ttf' 12\n";
    gp << "set output \"" << out_url << "\"\n";
  }
  gp << "set key left top\n";
  gp << "set title '" << plot_title << "'\n";
  gp << "set style data lines\n";
  gp << "set style line 1 lc rgb '#7F7F7F' lt 1 lw 1 pt 5 ps 1\n";
  gp << "set xrange [" << min_x << ":" << max_x*1.2 << "]\nset yrange [" << min_y << ":" << max_y*1.2 << "]\n";
  gp << "set xlabel '" << x_label << "'\n";
  gp << "set ylabel '" << y_label << "'\n";
  gp << "set grid\n";
  
  gp << "plot '-' u 1:2 title '" << plot_title << "' w linespoints ls 1\n";
  gp.send1d(boost::make_tuple(x_v, y_v) );
}

void check_2d_hilbert_curve()
{
  unsigned nDims = 2;
  unsigned nBits = 4;
  // bitmask_t coord_1_[] = {5, 2};
  // bitmask_t coord_2_[] = {5, 5};
  // LOG(INFO) << "index_1= " << mfa_hilbert_c2i(nDims, nBits, coord_1_) << "\n";
  // LOG(INFO) << "index_2= " << mfa_hilbert_c2i(nDims, nBits, coord_2_) << "\n";
  
  typedef std::pair<bitmask_t, bitmask_t> coor_p;
  std::vector<bitmask_t> index_v;
  std::map<bitmask_t, coor_p> index__coor_p_map;
  
  int up_limit = pow(2, nBits);
  for (int x = 0; x < up_limit; x++) {
    for (int y = 0; y < up_limit; y++) {
      bitmask_t coord_[] = {x, y};
      bitmask_t index = mfa_hilbert_c2i(nDims, nBits, coord_);
      
      index_v.push_back(index);
      index__coor_p_map[index] = std::make_pair(x, y);
    }
  }
  std::sort(index_v.begin(), index_v.end() );
  
  std::vector<int> x_v, y_v;
  // std::ofstream myfile("hilber_cpp.csv");
  // if (myfile.is_open() ) {
    for (std::vector<bitmask_t>::iterator it = index_v.begin(); it != index_v.end(); it++) {
      coor_p p = index__coor_p_map[*it];
      x_v.push_back(p.first + 1);
      y_v.push_back(p.second + 1);
      // myfile << p.first << "," << p.second << "\n";
      // LOG(INFO) << p.first << "," << p.second << "\n";
    }
    // myfile.close();
  // }
  
  std::string out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/sdm_control/sfc/hilbert_curve.png";
  make_2dscatter_plot<int>(x_v, y_v, "X", "Y",
                           "Hilbert curve; order= " + boost::lexical_cast<std::string>(nBits),
                           out_url);
}

template<typename T>
void make_3dscatter_plot(std::vector<T> x_v, std::vector<T> y_v, std::vector<T> z_v,
                         std::string x_label, std::string y_label, std::string z_label,
                         std::string plot_title, std::string out_url)
{
  Gnuplot gp;
  
  T min_x = *std::min_element(x_v.begin(), x_v.end() );
  T max_x = *std::max_element(x_v.begin(), x_v.end() );
  
  T min_y = *std::min_element(y_v.begin(), y_v.end() );
  T max_y = *std::max_element(y_v.begin(), y_v.end() );
  
  T min_z = *std::min_element(z_v.begin(), z_v.end() );
  T max_z = *std::max_element(z_v.begin(), z_v.end() );
  
  if (out_url.compare("") != 0) {
    gp << "set term png enhanced font '/usr/share/fonts/dejavu/DejaVuSans.ttf' 12\n";
    gp << "set output \"" << out_url << "\"\n";
  }
  gp << "set key left top\n";
  gp << "set title '" << plot_title << "'\n";
  // gp << "set style data lines\n";
  gp << "set style line 1 lc rgb '#7F7F7F' lt 1 lw 1 pt 5 ps 1\n";
  gp << "set xrange [" << min_x << ":" << max_x*1.2 << "]\n"
     << "set yrange [" << min_y << ":" << max_y*1.2 << "]\n"
     << "set zrange [" << min_z << ":" << max_z*1.2 << "]\n";
  gp << "set xlabel '" << x_label << "'\n";
  gp << "set ylabel '" << y_label << "'\n";
  gp << "set zlabel '" << z_label << "'\n";
  // gp << "set grid\n";
  
  gp << "splot '-' u 1:2:3 title '" << plot_title << "' w linespoints ls 1\n";
  gp.send1d(boost::make_tuple(x_v, y_v, z_v) );
}

void check_3d_hilbert_curve()
{
  unsigned nDims = 3;
  unsigned nBits = 4;
  // bitmask_t coord_1_[] = {5, 2, 2};
  // bitmask_t coord_2_[] = {5, 5, 5};
  // LOG(INFO) << "index_1= " << mfa_hilbert_c2i(nDims, nBits, coord_1_) << "\n";
  // LOG(INFO) << "index_2= " << mfa_hilbert_c2i(nDims, nBits, coord_2_) << "\n";
  
  typedef boost::tuple<bitmask_t, bitmask_t, bitmask_t> coor_t;
  std::vector<bitmask_t> index_v;
  std::map<bitmask_t, coor_t> index__coor_t_map;
  
  int up_limit = pow(2, nBits);
  for (int x = 0; x < up_limit; x++) {
    for (int y = 0; y < up_limit; y++) {
      for (int z = 0; z < up_limit; z++) {
        bitmask_t coord_[] = {x, y, z};
        bitmask_t index = mfa_hilbert_c2i(nDims, nBits, coord_);
        
        index_v.push_back(index);
        index__coor_t_map[index] = boost::make_tuple(x, y, z);
      }
    }
  }
  std::sort(index_v.begin(), index_v.end() );
  
  std::vector<int> x_v, y_v, z_v;
  for (std::vector<bitmask_t>::iterator it = index_v.begin(); it != index_v.end(); it++) {
    coor_t t = index__coor_t_map[*it];
    x_v.push_back(t.get<0>() + 1);
    y_v.push_back(t.get<1>() + 1);
    z_v.push_back(t.get<2>() + 1);
  }
  
  std::string out_url = "/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/sdm_control/sfc/hilbert_curve.png";
  make_3dscatter_plot(x_v, y_v, z_v, "X", "Y", "Z",
                      "Hilbert curve; order= " + boost::lexical_cast<std::string>(nBits),
                      out_url);
}

void test_hsalgo()
{
  char ds_id_[] = {'a', 'b', 'c'};
  srand(time(NULL) );
  COOR_T lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  COOR_T ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 16) };
  HSAlgo hsalgo(lcoor_, ucoor_, 1);
  
  LOG(INFO) << "test_hsalgo:: index_interval_set= " << *(hsalgo.coor_to_index_interval_set_(lcoor_, ucoor_) ) << "\n";
  
  MULTI_FOR(lcoor_, ucoor_)
    COOR_T walk_lcoor_[NDIM] = { BOOST_PP_ENUM(NDIM, VAR_REP, d) };
    COOR_T walk_ucoor_[NDIM];
    for (int i = 0; i < NDIM; i++)
      walk_ucoor_[i] = walk_lcoor_[i] + 1;
    
    LOG(INFO) << "walk_lucoor_= " << LUCOOR_TO_STR(walk_lcoor_, walk_ucoor_) << "\n";
    
    hsalgo.add_access(walk_lcoor_, walk_ucoor_);
    std::vector<lcoor_ucoor_pair> lucoor_to_fetch_v;
    hsalgo.get_to_fetch(walk_lcoor_, walk_ucoor_, lucoor_to_fetch_v);
    for (std::vector<lcoor_ucoor_pair>::iterator it = lucoor_to_fetch_v.begin(); it != lucoor_to_fetch_v.end(); it++)
      LOG(INFO) << "lucoor_= " << LUCOOR_TO_STR(it->first, it->second) << "\n";
  END_MULTI_FOR()
  LOG(INFO) << "hsalgo= \n" << hsalgo.to_str();
}

void spcsim_test()
{
  // std::vector<char> ds_id_v = boost::assign::list_of('a')('b');
  int num_p, num_c;
  std::vector<char> p_id__ds_id_v, c_id__ds_id_v;
  std::vector<int> p_id__num_put_v, c_id__num_get_v;
  std::vector<float> p_id__put_rate_v, c_id__get_rate_v;
  std::vector<std::vector<float> > p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v;
  
  int num_ds = 2;
  int max_num_p = 1;
  int max_num_c = 1;
  int num_putget_mean = 20;
  float put_rate_mean = 5;
  float get_rate_mean = 0.2;
  
  std::vector<char> ds_id_v;
  gen_scenario(num_ds, ds_id_v,
               max_num_p, max_num_c, num_putget_mean, put_rate_mean, get_rate_mean,
               num_p, num_c,
               p_id__ds_id_v, c_id__ds_id_v,
               p_id__num_put_v, c_id__num_get_v,
               p_id__put_rate_v, c_id__get_rate_v,
               p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v);
  
  int volume = 100;
  int dim_length = (int) pow(volume, (float)1/NDIM);
  LOG(INFO) << "spcsim_test:: dim_length= " << dim_length << "\n";
  COOR_T lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  COOR_T ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, dim_length) };
  int sexpand_length = 1;
  bool w_prefetch = true;
  
  SPCSim spc_sim(ds_id_v, num_p, num_c,
                 p_id__ds_id_v, c_id__ds_id_v,
                 p_id__num_put_v, c_id__num_get_v,
                 p_id__put_rate_v, c_id__get_rate_v,
                 p_id__inter_arr_time_v_v, c_id__inter_arr_time_v_v,
                 SALGO_H, sexpand_length, lcoor_, ucoor_, w_prefetch);
  
  spc_sim.sim_all();
  
  LOG(INFO) << "sim:: spc_sim.get_c_id__get_lperc_map= \n"
            << patch_all::map_to_str<>(spc_sim.get_c_id__get_lperc_map() ) << "\n";
  
  LOG(INFO) << "sim:: spc_sim= \n" << spc_sim.to_str();
}

#endif // _PATCH_SFC_EXP_H_
