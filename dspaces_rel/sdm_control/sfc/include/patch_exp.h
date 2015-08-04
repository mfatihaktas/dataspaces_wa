#ifndef _PATCH_EXP_H_
#define _PATCH_EXP_H_

#include "patch_sfc.h"
#include "sim.h"

#include "gnuplot-iostream.h"

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
  
  hilbert_box_pt(nDims, 0, nBits, 1, lcoor_, ucoor_);
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
  // std::cout << "index_1= " << hilbert_c2i(nDims, nBits, coord_1_) << "\n";
  // std::cout << "index_2= " << hilbert_c2i(nDims, nBits, coord_2_) << "\n";
  
  typedef std::pair<bitmask_t, bitmask_t> coor_p;
  std::vector<bitmask_t> index_v;
  std::map<bitmask_t, coor_p> index__coor_p_map;
  
  int up_limit = pow(2, nBits);
  for (int x = 0; x < up_limit; x++) {
    for (int y = 0; y < up_limit; y++) {
      bitmask_t coord_[] = {x, y};
      bitmask_t index = hilbert_c2i(nDims, nBits, coord_);
      
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
  // std::cout << "index_1= " << hilbert_c2i(nDims, nBits, coord_1_) << "\n";
  // std::cout << "index_2= " << hilbert_c2i(nDims, nBits, coord_2_) << "\n";
  
  typedef boost::tuple<bitmask_t, bitmask_t, bitmask_t> coor_t;
  std::vector<bitmask_t> index_v;
  std::map<bitmask_t, coor_t> index__coor_t_map;
  
  int up_limit = pow(2, nBits);
  for (int x = 0; x < up_limit; x++) {
    for (int y = 0; y < up_limit; y++) {
      for (int z = 0; z < up_limit; z++) {
        bitmask_t coord_[] = {x, y, z};
        bitmask_t index = hilbert_c2i(nDims, nBits, coord_);
        
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

#endif // _PATCH_EXP_H_