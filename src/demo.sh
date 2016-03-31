# count read coverage for bins
./countCoverage -i bamFilesList -p param
# extract bin-wise log2 coverage for training genomic region
./extractFeature -i coverageFilesList -t trainingCMYC.txt -o feature_trainingCMYC.txt -p param
# extract bin-wise log2 coverage for testing genomic region
./extractFeature -i coverageFilesList -t testingGABP.txt -o feature_testingGABP.txt -p param
# make prediction with fcat
python fcat.py -model RandomForest,LogisticRegressionL1 -train ../data/feature_trainingCMYC.txt_5_1000 -test ../data/feature_trainingGABP.txt_5_1000 -output finalResult.txt

