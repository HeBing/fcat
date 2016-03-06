linuxgcc:
	g++ -Wall -I ../samtools-0.1.18 -L../samtools-0.1.18 -c fcarLib.c -lbam -lz -lm -o fcarLib.o
	g++ countCoverage.c fcarLib.o -L../samtools-0.1.18 -lbam -lz -lm -o countCoverage
	g++ extractFeature.c fcarLib.o -L../samtools-0.1.18 -lbam -lz -lm -o extractFeature
	g++ predictModel.c fcarLib.o -L../samtools-0.1.18 -lbam -lz -lm -o predictModel
	g++ trainModel.c fcarLib.o -L../samtools-0.1.18 -lbam -lz -lm -o trainModel
#	g++ -Wall -o predictModelWholeGenome predictModelWholeGenome.c ../liblinear-1.96/tron.o ../liblinear-1.96/linear.o fcarLib.o ../liblinear-1.96/blas/blas.a -L../samtools-0.1.18 -lbam -lz

