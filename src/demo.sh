# count read coverage for bins
./countCoverage -i ./data/bamFilesList -p ./data/param
# extract bin-wise log2 coverage for training genomic region
./extractFeature -i ./data/coverageFilesList -t ./data/trainingCMYC.txt -o feature_trainingCMYC.txt -p param
# extract bin-wise log2 coverage for testing genomic region
./extractFeature -i ./data/coverageFilesList -t ./data/testingGABP.txt -o feature_testingGABP.txt -p ./data/param
# make prediction with fcat
python fcat.py -model RandomForest,LogisticRegressionL1 -train ./data/feature_trainingCMYC.txt_5_1000 -test ./data/feature_trainingGABP.txt_5_1000 -output ./data/finalResult.txt

