# fcat

#### Introduction
`fcat` is a *flexible classification toolbox fo high throughput sequencing data*. 

Simply put, `fcat` is a supervised learning framework for predicting regulatory acitivties on the genome. `fcat` takes bam files and a list of training genomic regions (for which we know the truth) as input, train models with signals in the bam files around those given regions, and predict biological activities of interest genomewide with the trained models. 
Currently the models include: logistic regression with L1 penalty (Lasso regression), logistic regression with L2 penalty (Ridge regression), and random forest. A unique feature of `fcat` is that it provides an ensemble learning method that integrates prediction results from individual models to augment the final prediction.

If you have any questions on installation and usage of `fcat`, feel free to contact me at bhe3@jhu.edu. If there are error messages, please send those messages to me. Thanks!

#### How to install

* Step 1: Download the zipped repo and unzip `unzip fcat-master.zip`;
* Step 2: Change working directory to `fcat-master` and type `make` on the command line.

#### Quick Start

* Change working directory to `src/`
* Open `src/demo.sh` and copy the command you need to command line to run (see a simple example below)

#### An Example
Below is a simple example where we predict binding sites of GABP based on models trained from CMYC in `src/demo.sh`:

* First, we count the read coverage for bins with specified resolution from the bam files. We included two (truncated) bam files in `./src/data/`. `./countCoverage` will produce a binary file containing the coverage information. 

```bash
# count read coverage for bins
## here bam files are truncated bam files
./countCoverage -i ./data/bamFilesList -p ./data/param
```
* Second, we extract coverage as features for the given training genomic regions `trainingCMYC.txt` as well as testing regions `testingGABP.txt`.

```bash
# extract bin-wise log2 coverage for training genomic region
./extractFeature -i ./data/coverageFilesList -t ./data/trainingCMYC.txt -o ./data/feature_trainingCMYC_small.txt -p ./data/param
# extract bin-wise log2 coverage for testing genomic region
./extractFeature -i ./data/coverageFilesList -t ./data/testingGABP.txt -o ./data/feature_testingGABP_small.txt -p ./data/param
```

* Third, we make predictions using fcat. Use `fcat.py` to specify what individual model you want. 

```bash
# make prediction with fcat 
## using features extracted from full bam files
python fcat.py -model RandomForest,LogisticRegressionL1 -train ./data/feature_trainingCMYC_full.txt_5_1000 -test ./data/feature_testingGABP_full.txt_5_1000 -output ./data/finalResult.txt
```

#### Other requirements:
* `fcat` require python is installed (2.6.6 or higher) on the machine.
* Currently, `fcat` only supports linux/unix.
* `fcat` require `gcc/g++` 4.4.7 or higher.

#### How to use
After installation, four exectuable programs will appear in `\src`: `countCoverage`, `extractFeature`, `trainModel`, `predictModel` as well as `fcat.py`. In command line, type the program with no arguments to see options:

* `countCoverage` count read coverage based on bam files. It takes in a file containing a list of bam file names (with full path) and a parameter setting file that sets the resolution and single-end/paired-end of the bam files. 

```sh
./countCoverage
/*-----------------------------------*/
/*            extractFeature         */
/* -i bams file list                 */
/* -p parameter setting file         */
/*-----------------------------------*/
```

An example of parameter setting file is 
```
resolution=5
windowSize=1000
pairend=0
min=0
max=100
```

* `extractFeature` extract coverage around a given set of genomic regions (with information on biological activity of interest at those regions) from the resulted coverage files from `countCoverage`.
```sh
./extractFeature
/*-----------------------------------*/
/*            extractFeature         */
/* -i coverages file list            */
/* -t training region coordinate     */
/* -o output file                    */
/* -p parameter setting file         */
/*-----------------------------------*/
```

* `trainModel` train a specified model with resulted feature files f rom `extractFeature`.
```sh
./trainModel
/*----------------------------------*/
/*           menu_trainModel        */
/* -m method                        */
/*    LogisticRegressionL1          */
/*    LogisticRegressionL2          */
/*    RandomForest                  */
/* -c penalty tuning                */
/* -t training data file            */
/* -o output model                  */
/*----------------------------------*/
```

* `predictModel` predict test data using trained model from `trainModel`
```sh
/*------------------------------------*/
/*           menu_predictModel        */
/* -m method name                     */
/*    LogisticRegressionL1            */
/*    LogisticRegressionL2            */
/*    RandomFores                     */
/* -tm trainedModel                   */
/* -train trainFile                   */
/* -test testFile                     */
/* -o output results                  */
/*------------------------------------*/
```

#### Note:
* `fcat` uses codes from [`liblinear`](http://www.csie.ntu.edu.tw/~cjlin/liblinear/) and [`rt-rank`](https://sites.google.com/site/rtranking/) projects.
* Currently, `fcat` only works under linux/unix.

