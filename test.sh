
make bf
	cat examples/bitwidth.b 	| ./cccc  
	cat examples/mandelbrot.b 	| ./cccc  
make gl
	cat examples/scanline.c4 	| ./cccc  

