all:
	gcc -o fileCompressor fileCompressor.c  

clean:
	rm fileCompressor; rm HuffmanCodebook
