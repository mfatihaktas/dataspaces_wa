#ifndef _PLOT_H_
#define _PLOT_H_

#include <string>
#include <vector>

#include "gnuplot-iostream.h"
#include "patch.h"

namespace patch {
  static std::string exec(std::string cmd)
  {
    boost::shared_ptr<FILE> pipe_(popen(cmd.c_str(), "r"), pclose);
    if (!pipe_) {
      log_(ERROR, "opening pipe_ failed!")
      return "ERROR";
    }
    
    char buffer_[128];
    std::string result = "";
    while (!feof(pipe_.get() ) ) {
      if (fgets(buffer_, 128, pipe_.get() ) != NULL)
        result += buffer_;
    }
    return result;
  }
  
  // Note: enclosing any latex code ($code$) with '' is required
  template<typename T>
  void make_latex_plot(std::vector<std::vector<T> >& x_v_v, std::vector<std::vector<T> >& y_v_v, std::vector<std::string>& title_v,
                       std::string x_label, std::string y_label,
                       std::string plot_title, std::string out_name)
  {
    std::cout << "x_v_v= \n";
    for (typename std::vector<std::vector<T> >::iterator it = x_v_v.begin(); it != x_v_v.end(); it++)
      std::cout << patch::vec_to_str<>(*it) << "\n";
    
    std::cout << "y_v_v= \n";
    for (typename std::vector<std::vector<T> >::iterator it = y_v_v.begin(); it != y_v_v.end(); it++)
      std::cout << patch::vec_to_str<>(*it) << "\n";
    // 
    std::string color_code_[] = {"#0060ad", "#DAA520", "#7F7F7F", "#D2691E", "#556B2F", "#DC143C", "#DA70D6", "#8B008B", "#1E90FF", "#9ACD32"};
    
    std::vector<T> x_v;
    for (typename std::vector<std::vector<T> >::iterator it = x_v_v.begin(); it != x_v_v.end(); it++)
      x_v.insert(x_v.end(), it->begin(), it->end() );
    
    T min_x = *std::min_element(x_v.begin(), x_v.end() );
    T max_x = *std::max_element(x_v.begin(), x_v.end() );
    
    std::vector<T> y_v;
    for (typename std::vector<std::vector<T> >::iterator it = y_v_v.begin(); it != y_v_v.end(); it++)
      y_v.insert(y_v.end(), it->begin(), it->end() );
    
    T min_y = *std::min_element(y_v.begin(), y_v.end() );
    T max_y = *std::max_element(y_v.begin(), y_v.end() );
    
    // out_name = "gnuplot/" + out_name;
    {
      Gnuplot gp;
      gp << "set terminal epslatex standalone color colortext font ',8' size 20cm,15cm\n"
         << "set output '" << out_name << ".tex'\n"
         << "set key right bottom\n"
        // << "set key outside\n"
        // << "set key width -8\n"
         << "set title '" << plot_title << "'\n" // offset 0,1 // to lift title up
         << "set xrange [" << min_x*0.9 << ":" << max_x*1.1 << "]\nset yrange [" << min_y*0.95 << ":" << max_y*1.1 << "]\n"
         << "set xlabel '" << x_label << "'\n"
         << "set ylabel '" << y_label << "'\n"
         << "set grid\n";
      
      for (int i = 0; i < x_v_v.size(); i++)
        gp << "set style line " << boost::lexical_cast<std::string>(i + 1) << " lc rgb '" << color_code_[i] << "' " << "pt " << boost::lexical_cast<std::string>(i + 1) << " lt 1 lw 2 ps 1\n";
      
      gp << "plot "
         << "x lw 1 lt 3 title 'y=x',";
      for (int i = 0; i < x_v_v.size(); i++) {
        gp << "'-' u 1:2 title '" << title_v[i] << "' w linespoints ls " << boost::lexical_cast<std::string>(i + 1);
        if (i == x_v_v.size() - 1)
          gp << "\n";
        else
          gp << ", ";
      }
      
      typename std::vector<std::vector<T> >::iterator it_x_v, it_y_v;
      for (it_x_v = x_v_v.begin(), it_y_v = y_v_v.begin(); it_x_v != x_v_v.end(); it_x_v++, it_y_v++)
        gp.send1d(boost::make_tuple(*it_x_v, *it_y_v) );
    }
    // log_(INFO, "exec(latex " << out_name << ".tex) returned= \n"
    //           << exec("latex " + out_name + ".tex") << "\n"
    //           << "exec(dvips -o " << out_name << ".ps " << out_name << ".dvi) returned= \n"
    //           << exec("dvips -o " + out_name + ".ps " + out_name + ".dvi") )
    log_(INFO, "exec(latex " << out_name << ".tex) returned= \n"
               << exec("latex " + out_name + ".tex") )
    log_(INFO, "exec(dvips -o " << out_name << ".ps " << out_name << ".dvi) returned= \n"
               << exec("dvips -o " + out_name + ".ps " + out_name + ".dvi") )
  }
  
  template<typename T>
  void make_plot(std::vector<std::vector<T>& > x_v_v, std::vector<std::vector<T> >& y_v_v, std::vector<std::string>& title_v,
                 std::string x_label, std::string y_label,
                 std::string plot_title, std::string out_url)
  {
    std::cout << "x_v_v= \n";
    for (typename std::vector<std::vector<T> >::iterator it = x_v_v.begin(); it != x_v_v.end(); it++)
      std::cout << patch::vec_to_str<>(*it) << "\n";
    
    std::cout << "y_v_v= \n";
    for (typename std::vector<std::vector<T> >::iterator it = y_v_v.begin(); it != y_v_v.end(); it++)
      std::cout << patch::vec_to_str<>(*it) << "\n";
    // 
    std::string color_code_[] = {"#0060ad", "#DAA520", "#7F7F7F", "#D2691E", "#556B2F", "#DC143C", "#DA70D6", "#8B008B", "#1E90FF", "#9ACD32"};
    Gnuplot gp;
    
    std::vector<T> x_v;
    for (typename std::vector<std::vector<T> >::iterator it = x_v_v.begin(); it != x_v_v.end(); it++)
      x_v.insert(x_v.end(), it->begin(), it->end() );
    
    T min_x = *std::min_element(x_v.begin(), x_v.end() );
    T max_x = *std::max_element(x_v.begin(), x_v.end() );
    
    std::vector<T> y_v;
    for (typename std::vector<std::vector<T> >::iterator it = y_v_v.begin(); it != y_v_v.end(); it++)
      y_v.insert(y_v.end(), it->begin(), it->end() );
    
    T min_y = *std::min_element(y_v.begin(), y_v.end() );
    T max_y = *std::max_element(y_v.begin(), y_v.end() );
    
    if (out_url.compare("") != 0) {
        // << "set term png size 1200,800 enhanced font '/usr/share/fonts/dejavu/DejaVuSans.ttf' 12\n";
        // << "set term post eps enh \"Helvetica\" 12 size 5,4\n"; // For Symbols
      gp << "set term png enhanced font '/usr/share/fonts/dejavu/DejaVuSans.ttf' 12\n"
         << "set output \"" << out_url << "\"\n";
    }
    gp << "set key right top\n"
       << "set key outside\n"
      // << "set title \"" << plot_title << "\"\n";
       << "set xrange [" << min_x*0.9 << ":" << max_x*1.05 << "]\nset yrange [" << min_y*0.95 << ":" << max_y*1.1 << "]\n"
       << "set xlabel '" << x_label << "'\n"
       << "set ylabel '" << y_label << "'\n"
       << "set grid\n";
    
    for (int i = 0; i < x_v_v.size(); i++)
      gp << "set style line " << boost::lexical_cast<std::string>(i + 1) << " lc rgb '" << color_code_[i] << "' " << "pt " << boost::lexical_cast<std::string>(i + 1) << " lt 1 lw 2 ps 1\n";
      // gp << "set style line " << boost::lexical_cast<std::string>(i + 1) << " lc rgb '" << color_code_[i] << "' lt 1 lw 2 pt 5 ps 1.5\n";
    // gp << "set logscale xy\n";
    
    gp << "plot";
    for (int i = 0; i < x_v_v.size(); i++) {
      gp << "'-' u 1:2 title '" << title_v[i] << "' w linespoints ls " << boost::lexical_cast<std::string>(i + 1);
      if (i == x_v_v.size() - 1)
        gp << "\n";
      else
        gp << ", ";
    }
    
    typename std::vector<std::vector<T> >::iterator it_x_v, it_y_v;
    for (it_x_v = x_v_v.begin(), it_y_v = y_v_v.begin(); it_x_v != x_v_v.end(); it_x_v++, it_y_v++)
      gp.send1d(boost::make_tuple(*it_x_v, *it_y_v) );
  }
  
  template<typename T>
  void make_plot(std::vector<T>& x1_v, std::vector<T>& y1_v, std::string title1,
                 std::vector<T>& x2_v, std::vector<T>& y2_v, std::string title2,
                 std::string x_label, std::string y_label, 
                 std::string plot_title, std::string out_url)
  {
    Gnuplot gp;
    T min_x = std::min<T>(*std::min_element(x1_v.begin(), x1_v.end() ), *std::min_element(x2_v.begin(), x2_v.end() ) );
    T max_x = std::max<T>(*std::max_element(x1_v.begin(), x1_v.end() ), *std::max_element(x2_v.begin(), x2_v.end() ) );
    
    T min_y = std::min<T>(*std::min_element(y1_v.begin(), y1_v.end() ), *std::min_element(y2_v.begin(), y2_v.end() ) );
    T max_y = std::max<T>(*std::max_element(y1_v.begin(), y1_v.end() ), *std::max_element(y2_v.begin(), y2_v.end() ) );
    
    if (out_url.compare("") != 0) {
      gp << "set term png enhanced font '/usr/share/fonts/dejavu/DejaVuSans.ttf' 12\n";
      gp << "set output \"" << out_url << "\"\n";
    }
    gp << "set key left top\n"
       << "set title '" << plot_title << "'\n"
       << "set style line 1 lc rgb '#7F7F7F' lt 1 lw 2 pt 5 ps 1.5\n"
       << "set style line 2 lc rgb '#0060ad' lt 1 lw 2 pt 5 ps 1.5\n"
       << "set xrange [" << min_x << ":" << max_x*1.2 << "]\nset yrange [" << min_y << ":" << max_y*1.2 << "]\n"
       << "set xlabel '" << x_label << "'\n"
       << "set ylabel '" << y_label << "'\n"
       << "set grid\n";
    
    // gp << "set logscale xy\n";
    // typename std::vector<T>::iterator it_y1, it_y2; // supposedly vectors of same size
    // for (it_y1 = y1_v.begin(), it_y2 = y2_v.begin(); 
    //     it_y1 != y1_v.end(), it_y2 != y2_v.end(); it_y1++, it_y2++) {
    //   gp << "set ytics add (" << boost::lexical_cast<std::string>(*it_y1) << ")\n";
    //   gp << "set ytics add (" << boost::lexical_cast<std::string>(*it_y2) << ")\n";
    // }
    gp << "set ytics add (" << boost::lexical_cast<std::string>(min_y) << ")\n";
    for (T f = 0; f < max_y; f += 20) {
      if (f > min_y)
        gp << "set ytics add (" << boost::lexical_cast<std::string>(f) << ")\n";
    }
    
    gp << "plot '-' u 1:2 title '" << title1 << "' w linespoints ls 1, "
            << "'-' u 1:2 title '" << title2 << "' w linespoints ls 2\n";
    gp.send1d(boost::make_tuple(x1_v, y1_v) );
    gp.send1d(boost::make_tuple(x2_v, y2_v) );
  }
  
  /*
  n=100 #number of intervals
  max=3. #max value
  min=-3. #min value
  width=(max-min)/n #interval width
  #function used to map a value to the intervals
  hist(x,width)=width*floor(x/width)+width/2.0
  set term png #output terminal and file
  set output "histogram.png"
  set xrange [min:max]
  set yrange [0:]
  #to put an empty boundary around the
  #data inside an autoscaled graph.
  set offset graph 0.05,0.05,0.05,0.0
  set xtics min,(max-min)/5,max
  set boxwidth width*0.9
  set style fill solid 0.5 #fillstyle
  set tics out nomirror
  set xlabel "x"
  set ylabel "Frequency"
  #count and plot
  plot "data.dat" u (hist($1,width)):(1.0) smooth freq w boxes lc rgb"green" notitle
  */
  template<typename T>
  void make_hist(std::vector<T>& x_v,
                 std::string x_label, std::string y_label, 
                 std::string plot_title, std::string out_url)
  {
    Gnuplot gp;
    T min_x = *std::min_element(x_v.begin(), x_v.end() );
    T max_x = *std::max_element(x_v.begin(), x_v.end() );
    
    // int num_intervals = 100;
    // float width = (float)(max_x - min_x) / num_intervals;
    // std::vector<T> x_v
    
    if (out_url.compare("") != 0) {
      gp << "set term png enhanced font '/usr/share/fonts/dejavu/DejaVuSans.ttf' 12\n"
         << "set output \"" << out_url << "\"\n";
    }
    
    gp << "n=100\n"
      // << "min=" << boost::lexical_cast<std::string>(min_x) << "\n"
      // << "max=" << boost::lexical_cast<std::string>(max_x) << "\n"
       << "min=-3\n"
       << "max=3\n"
       << "width=(max-min)/n\n"
       << "hist(x,width)=width*floor(x/width)+width/2.0\n"
      // << "set xrange [" << min_x << ":" << max_x*1.2 << "]\n";
      << "set xrange [min:max]\n"
      // << "set yrange [0:*]\n"
      << "set yrange [0:1.2]\n"
      // << "set offset graph 0.05,0.05,0.05,0.0\n" // to put an empty boundary around the data inside an autoscaled graph.
       << "set xtics min,(max-min)/5,max\n"
       << "set boxwidth width*0.9\n"
       << "set style fill solid 0.5\n"
       << "set tics out nomirror\n";
    
    gp << "plot 'data.dat' u (hist($1,width)):(1.0) smooth freq w boxes lc rgb 'green' notitle\n";
    // gp << "plot '-' u (hist($1,width)):(1.0) smooth freq w boxes lc rgb 'green' notitle\n";
    // gp.send1d(boost::make_tuple(x_v) );
    // gp.send1d(x_v);
    // gp.file1d(x_v, "make_hist.dat");
    // gp << "plot 'make_hist.dat' w lp\n";
  }
};

#endif // _PLOT_H_