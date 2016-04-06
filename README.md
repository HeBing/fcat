# fcat

#### Introduction
`fcat` is a *flexible classification toolbox for high throughput sequencing data*. 

#### Features
* Whole analysis pipeline from bam files to prediction results
* Customizable calculation of read coverage
* Ensemble learning with multiple individual models provided for prediction
* Ease to use

#### Quick Start
* Step 1: Download [fcat-linux-v1.0.zip](https://github.com/HeBing/fcat/archive/linux-v1.0-beta.zip) for linux and [fcat-mac-v1.0.zip](https://github.com/HeBing/fcat/archive/mac-v1.0-beta.zip) for mac OS X.
* Step 2: unzip the download file
* Step 3: change working directory to the unzipped folder and type `make` from command line
* Step 4: change working directory to `src/` and run fcat with

```bash
python fcat.py -model RandomForest,LogisticRegressionL1 \
  -train ./data/trainData.txt \
  -test ./data/testData.txt \
  -output ./data/finalResult.txt
```

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
* Currently, `fcat` only supports linux/unix.
* `fcat` require python is installed (2.6.6 or higher) on the machine.
* `fcat` require `gcc/g++` 4.4.7 or higher. `fcat` does NOT work with `clang++`/`LLVM` on mac. Please install `gcc` first on mac os x (see [How to install `gcc`](#gcc)).

#### How to use
After installation, four exectuable programs will appear in `\src`: `countCoverage`, `extractFeature`, `trainModel`, `predictModel` as well as `fcat.py`. In command line, type the program with no arguments to see options:

* `countCoverage` count read coverage based on bam file (See [how to obtain bam files?](#alignment)). It takes in a file containing a list of bam file names (with full path) and a parameter setting file that sets the resolution and single-end/paired-end of the bam files. When using `countCoverage`, we need the size of genomes; currently `fcat` only supports hg19.

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
/*    RandomForest                    */
/* -tm trainedModel                   */
/* -train trainFile                   */
/* -test testFile                     */
/* -o output results                  */
/*------------------------------------*/
```
#### `fcat` output
* `[output]_result`: predicted probability from specified models. The 1st column is a placeholder for ground truth. The last column corresponds the integrated model output.
* `[output]_FDRresult`: false discovery rate from specified models. 

#### <a name="alignment">How to get bam files?</a>
* Start with `fastq` files, you can use [`bowtie`](http://bowtie-bio.sourceforge.net/index.shtml), [`bowtie2`](http://bowtie-bio.sourceforge.net/bowtie2/index.shtml), [`tophat`](https://ccb.jhu.edu/software/tophat/index.shtml), and [`HISAT`](http://www.ccb.jhu.edu/software/hisat/index.shtml) to obtain aligned reads in `bam` format.

#### <a name="gcc">How to install `gcc`?</a>
* Step 1: Open terminal and type the following command

```bash
brew tap homebrew/dupes
brew install gcc
```

* Step 2: type the following command

```bash
export CC=/usr/local/bin/gcc
```

#### Note:
* `fcat` uses codes from [`liblinear`](http://www.csie.ntu.edu.tw/~cjlin/liblinear/) and [`rt-rank`](https://sites.google.com/site/rtranking/) projects.

#### Contact
If you have any questions on installation and usage of `fcat`, feel free to contact me at bhe3@jhu.edu. If there are error messages, please send those messages to me. Thanks!

