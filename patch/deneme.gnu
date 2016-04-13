reset
n=100 #number of intervals
min=0. #min value
max=20. #max value
width=(max-min)/n #interval width
#function used to map a value to the intervals
hist(x,width)=width*floor(x/width)+width/2.0
set term png #output terminal and file
set term png size 1200,800 enhanced font '/usr/share/fonts/dejavu/DejaVuSans.ttf' 22
set output "histogram.png"

set style line 1 lt 1 lc rgb '#7F7F7F' # '#FB9A99' # light red
set style line 2 lt 1 lc rgb '#483D8B' # '#A6CEE3' # light blue
unset key
# axes
set style line 11 lc rgb '#808080' lt 1
set border 3 front ls 11
set tics nomirror out scale 0.75
# grid
set style line 12 lc rgb'#808080' lt 0 lw 1
set grid back ls 12
set grid xtics ytics mxtics

set xrange [min:max]
set yrange [0:]
set xtics 45
set mxtics 2

# to put an empty boundary around the data inside an autoscaled graph.
# set offset graph 0.05,0.05,0.05,0.0
set xtics min,(max-min)/5,max
set boxwidth width*0.9
set style fill solid 0.5 #fillstyle
set tics out nomirror
set xlabel "Access interarrival time"
set ylabel "Frequency"

set table 'nstx_hist_fitting.dat'
plot 'nstx_hist.dat' u (hist($1,width)):(1.0) smooth freq w boxes ls 1 notitle
unset table

# fit Gaussian
Gauss(x) = a/(sigma*sqrt(2*pi)) * exp(-(x-mu)**2 / (2*sigma**2) )
a = 30.
mu = 10.
sigma = 3.
fit Gauss(x) 'nstx_hist_fitting.dat' u 1:2 via a,mu,sigma

plot 'nstx_hist_fitting.dat' using 1:2 w boxes ls 1,\
  Gauss(x) w lines ls 2

# count and plot
# plot 'nstx_hist.dat' u (hist($1,width)):(1.0) smooth freq w boxes ls 1 notitle