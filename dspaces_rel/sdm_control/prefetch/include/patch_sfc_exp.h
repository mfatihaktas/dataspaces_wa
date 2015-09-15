#ifndef _PATCH_SFC_EXP_H_
#define _PATCH_SFC_EXP_H_

/***************************************  Check Hilbert  ******************************************/
void check_boost_geometry_api()
{
  // http://boost-geometry.203548.n3.nabble.com/3D-box-gt-3D-multi-polygon-conversion-td4025353.html
  int lcoor_1_[] = {0, 0};
  int ucoor_1_[] = {10, 10};
  CREATE_BOX(0, box_1, lcoor_1_, ucoor_1_)
  std::cout << "box_1= " << boost::geometry::dsv(box_1) << "\n";
  
  int lcoor_2_[] = {5, 5};
  int ucoor_2_[] = {10, 10};
  CREATE_BOX(1, box_2, lcoor_2_, ucoor_2_)
  std::cout << "box_2= " << boost::geometry::dsv(box_2) << "\n";
  
  // std::vector<box_t> output_v;
  // boost::geometry::union_(box_1, box_2, output_v);

  // std::cout << "check_boost_geometry_api:: \n";
  // for (std::vector<box_t>::iterator it = output_v.begin(); it != output_v.end(); it++) {
  //   std::cout << "box= " << boost::geometry::dsv(*it) << "\n";
  // }
}

void check_hilbert_api()
{
  unsigned nDims = 2;
  unsigned nBits = 4;
  // double *lcoor_, *ucoor_;
  double lcoor_[] = {0, 0};
  double ucoor_[] = {10, 10};
  
  mfa_hilbert_box_pt(nDims, 0, nBits, 1, lcoor_, ucoor_);
  // hilbert_ieee_box_pt(nDims, 0, lcoor_, ucoor_);
  // hilbert_ieee_box_vtx(nDims, 0, lcoor_, ucoor_);
  std::cout << "check_hilbert_api:: lcoor_= " << patch_sfc::arr_to_str<>(NDIM, lcoor_) << ","
            << "ucoor_= " << patch_sfc::arr_to_str<>(NDIM,  ucoor_) << "\n";
}

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
  // std::cout << "index_1= " << mfa_hilbert_c2i(nDims, nBits, coord_1_) << "\n";
  // std::cout << "index_2= " << mfa_hilbert_c2i(nDims, nBits, coord_2_) << "\n";
  
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
      // std::cout << p.first << "," << p.second << "\n";
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
  // std::cout << "index_1= " << mfa_hilbert_c2i(nDims, nBits, coord_1_) << "\n";
  // std::cout << "index_2= " << mfa_hilbert_c2i(nDims, nBits, coord_2_) << "\n";
  
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

// Walk whole space with incremental boxes
void s_sim_test()
{
  int volume = 1000;
  int dim_length = (int) pow(volume, (float)1/NDIM);
  LOG(INFO) << "sim:: dim_length= " << dim_length << "\n";
  
  std::vector<char> ds_id_v = boost::assign::list_of('a')('b');
  int pbuffer_size = 0;
  int pexpand_length = 1;
  COOR_T lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  COOR_T ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, dim_length) };
  std::vector<char> p_id__ds_id_v = boost::assign::list_of('a');
  std::vector<char> c_id__ds_id_v = boost::assign::list_of('b');
  SPCSim spc_sim(HILBERT_PREDICTOR, ds_id_v,
                 pbuffer_size, pexpand_length,
                 lcoor_, ucoor_,
                 p_id__ds_id_v, c_id__ds_id_v);
  
  std::vector<app_id__lcoor_ucoor_pair> p_id__lcoor_ucoor_pair_v, c_id__lcoor_ucoor_pair_v;
  
  COOR_T put_lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  COOR_T put_ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, dim_length) };
  p_id__lcoor_ucoor_pair_v.push_back(std::make_pair(0, std::make_pair(put_lcoor_, put_ucoor_) ) );
  // MULTI_FOR(put_lcoor_, put_ucoor_)
  //   COOR_T walk_lcoor_[NDIM] = { BOOST_PP_ENUM(NDIM, VAR_REP, d) };
  //   COOR_T walk_ucoor_[NDIM];
  //   for (int i = 0; i < NDIM; i++)
  //     walk_ucoor_[i] = walk_lcoor_[i] + 1;
    
  //   COOR_T* dwalk_lcoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
  //   COOR_T* dwalk_ucoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
  //   for (int i = 0; i < NDIM; i++) {
  //     dwalk_lcoor_[i] = walk_lcoor_[i];
  //     dwalk_ucoor_[i] = walk_ucoor_[i];
  //   }
    
  //   p_id__lcoor_ucoor_pair_v.push_back(std::make_pair(0, std::make_pair(dwalk_lcoor_, dwalk_ucoor_) ) );
  // END_MULTI_FOR()
  
  COOR_T get_lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  COOR_T get_ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, dim_length) };
  MULTI_FOR(get_lcoor_, get_ucoor_)
    COOR_T walk_lcoor_[NDIM] = { BOOST_PP_ENUM(NDIM, VAR_REP, d) };
    
    COOR_T* dwalk_lcoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
    COOR_T* dwalk_ucoor_ = (COOR_T*)malloc(NDIM*sizeof(COOR_T) );
    for (int i = 0; i < NDIM; i++) {
      dwalk_lcoor_[i] = walk_lcoor_[i];
      dwalk_ucoor_[i] = walk_lcoor_[i] + 1;
    }
    
    c_id__lcoor_ucoor_pair_v.push_back(std::make_pair(1, std::make_pair(dwalk_lcoor_, dwalk_ucoor_) ) );
    // LOG(INFO << "d0= " << d0 << ", d1= " << d1 << "\n";
    // LOG(INFO << "put_test:: " << LUCOOR_TO_STR(walk_lcoor_, walk_ucoor_) << "\n";
      // wa_space.put(0, walk_lcoor_, walk_ucoor_);
  END_MULTI_FOR()
  
  spc_sim.sim(p_id__lcoor_ucoor_pair_v, c_id__lcoor_ucoor_pair_v);
  LOG(INFO) << "sim:: spc_sim.get_c_id__get_lperc_map= \n" << patch_all::map_to_str<>(spc_sim.get_c_id__get_lperc_map() ) << "\n";
  
  LOG(INFO) << "sim:: spc_sim= \n" << spc_sim.to_str();
}

void test_hpredictor()
{
  char ds_id_[] = {'a', 'b', 'c'};
  srand(time(NULL) );
  COOR_T lcoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 0) };
  COOR_T ucoor_[] = { BOOST_PP_ENUM(NDIM, FIXED_REP, 16) };
  HPredictor hpredictor(1, lcoor_, ucoor_);
  
  LOG(INFO) << "test_hpredictor:: index_interval_set= " << *(hpredictor.coor_to_index_interval_set_(lcoor_, ucoor_) ) << "\n";
  
  // MULTI_FOR(lcoor_, ucoor_)
  //   COOR_T walk_lcoor_[NDIM] = { BOOST_PP_ENUM(NDIM, VAR_REP, d) };
  //   COOR_T walk_ucoor_[NDIM];
  //   for (int i = 0; i < NDIM; i++)
  //     walk_ucoor_[i] = walk_lcoor_[i] + 1;
    
  //   LOG(INFO << "test_hpredictor:: ----------- \n";
  //   LOG(INFO << "walk_lucoor_: \n" << LUCOOR_TO_STR(walk_lcoor_, walk_ucoor_) << "\n";
    
  //   std::vector<lcoor_ucoor_pair> next_lcoor_ucoor_pair_v;
  //   // hpredictor.add_acc__predict_next_acc(ds_id_[rand() % sizeof(ds_id_)/sizeof(*ds_id_) ], walk_lcoor_, walk_ucoor_, next_lcoor_ucoor_pair_v);
  //   hpredictor.add_acc__predict_next_acc('a', walk_lcoor_, walk_ucoor_, 
  //                                       1, next_lcoor_ucoor_pair_v);
  //   for (int i = 0; i < next_lcoor_ucoor_pair_v.size(); i++) {
  //     lcoor_ucoor_pair lu_pair = next_lcoor_ucoor_pair_v[i];
  //     LOG(INFO << "next_lucoor_: \n" << LUCOOR_TO_STR(lu_pair.first, lu_pair.second) << "\n";
  //   }
  // END_MULTI_FOR()
  LOG(INFO) << "hpredictor= \n" << hpredictor.to_str();
}

#endif // _PATCH_SFC_EXP_H_