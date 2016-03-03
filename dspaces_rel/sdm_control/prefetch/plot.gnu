set terminal epslatex standalone color colortext 10
set output 'errorf.tex'
set xrange [-3:3]
set yrange [-1:1]
set label 1 at -2, 0.5 "$erf(x) = \\frac{2}{\\sqrt{\\pi}}\\int_0^x\\, dt e^{-t^2}$" centre

plot erf(x)