# fcat

#### Introduction
`fcat` is short for *flexible classification toolbox fo high throughput sequencing data*. 

Simply put, `fcat` take bam files and a list genomic regions of interest as input, train models with signals in the bam files around those given regions, and predict biological activities of interest genomewide. 

Currently the models include: logistic regression with L1 penalty (Lasso regression), logistic regression with L2 penalty (Ridge regression), and random forest. Model averaging utility is also provided that averages predicted results from different models.

If you have any questions on installation and usage of `fcat`, feel free to contact me at bhe3@jhu.edu.

#### How to install

* Step 1: Download the whole directory
* Step 2: Type 'make'

#### Quick Start

* Change working directory to `src/`
* Run `python modelAveragingWrapper.py`


#### Other requirements:
* `fcat` require python is installed (2.6.6 or higher) on the machine
* `fcat` depends on `boost` (1.55.0 or higher)
* Currently, `fcat` only supports linux/unix.

#### How to use
After installation, four exectuable programs will appear in `\src`: `countCoverage`, `extractFeature`, `trainModel`, `predictModel`. In command line, type the program with no arguments to see options:

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
