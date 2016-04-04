all:
	g++ -I../samtools-0.1.18 -L../samtools-0.1.18 -c fcatLib.c -lbam -lz -lm -o fcatLib.o
	g++ countCoverage.c fcatLib.o -L../samtools-0.1.18 -lbam -lz -lm -o countCoverage
	g++ extractFeature.c fcatLib.o -L../samtools-0.1.18 -lbam -lz -lm -o extractFeature
	g++ predictModel.c fcatLib.o -L../samtools-0.1.18 -lbam -lz -lm -o predictModel
	g++ trainModel.c fcatLib.o -L../samtools-0.1.18 -lbam -lz -lm -o trainModel

