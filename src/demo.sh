# count read coverage for bins
## here bam files are truncated bam files
./countCoverage -i ./data/bamFilesList -p ./data/param
# extract bin-wise log2 coverage for training genomic region
./extractFeature -i ./data/coverageFilesList -t ./data/trainingCMYC.txt -o ./data/feature_trainingCMYC_small.txt -p ./data/param
# extract bin-wise log2 coverage for testing genomic region
./extractFeature -i ./data/coverageFilesList -t ./data/testingGABP.txt -o ./data/feature_testingGABP_small.txt -p ./data/param
# make prediction with fcat 
## using features extracted from full bam files
python fcat.py -model RandomForest,LogisticRegressionL1 -train ./data/feature_trainingCMYC_full.txt_5_1000 -test ./data/feature_testingGABP_full.txt_5_1000 -output ./data/finalResult.txt

