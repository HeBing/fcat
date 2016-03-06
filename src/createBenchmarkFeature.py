#!/usr/bin/python

#----------------------------------------------
# @INPUT: feature file
# @OUTPUT: feature file with specified
#		col's averaged
# @USAGE: python createBenchmarkFeature.py inputfile k
# @DESC: split features into k groups
#		return each group's average as new feature
#		(i.e., benchmark feature)
#----------------------------------------------
import os.path, sys, subprocess, shlex

def createBenchmarkFeature(inputFile, k) :
	try : inputFp = open(inputFile, 'r');
	except : print 'Cannot open ', inputFile; return -1
	
	line = inputFp.readline()
	line.strip('\n')
	terms = line.split(' ')
	if (len(terms)-1)%k != 0 :
		print 'Cannot split features into k equal-sized groups\n',
		return -1
	else : groupsize = (len(terms)-1)/k
	inputFp.seek(0)

	try : outputFp = open(inputFile+'_Benchmark', 'w')
	except : print 'Cannot open ', outputFile; return -1

	for line in inputFp :
		line.strip('\n')
		terms = line.split(' ')

		y = int(terms[0])
		x = [ float(a.split(':')[1])  for a in terms[1:] ]
		x_benchmark = [ sum(x[(a*groupsize):(a*groupsize+groupsize)])/groupsize for a in range(k) ]
	
		outputFp.write(' '.join([repr(y), \
			' '.join([repr(a+1)+':%.4f' % x_benchmark[a]  for a in range(len(x_benchmark)) ]), '\n'] ) )

	inputFp.close()
	outputFp.close()
	return inputFile+'_Benchmark'

def main(argv):
	if len(argv) != 3 :
		print "Usage: python createBenchmarkFeature.py inputfile k\n",
		return -1
	else :
		print 'created ', createBenchmarkFeature(argv[1], int(argv[2]))
		return 0
		
if __name__ == '__main__':
	main(sys.argv)
