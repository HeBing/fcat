#include "stdint.h"

/*-----------------------------*/
/*         macro               */
/*-----------------------------*/
#define MAX_DIR_LEN 1000
#define MAX_BAM_FILES 400
#define MAX_TRAINING_REGIONS 1000000
#define NUM_SEQ 23 // chrY & chrM excluded

extern const uint32_t chrlen[];

/*-----------------------------*/
/*         struct              */
/*-----------------------------*/

/* extract feature parameters */
struct extractFeatureParam {
	
	/* length of bin for calculating coverage*/
	int resolution;
	
	/* window size for extracting feature */
	int windowSize;

	/* bams are pairend */
	int pairend;

  /* min fragment length filter for pairend */
  int min;
  
  /* max fragment length filter for pairend */
  int max;
};

/* training region */
struct trainingRegion{
	int chr;
	int coordinate;
	int response;
};

/* model matrix */
struct modelMatrix {
	int n;
	int p;
	float **features;
	struct trainingRegion *trainingRegions;
};

/*------------------------------*/
/*        extractFeature        */
/*------------------------------*/
int extractFeature(char *coveragesFile, char *trainingFile, char *outputFile, char *paramFile);

/*------------------------------*/
/*        parseParam            */
/*------------------------------*/
int parseParam(char *paramFile, struct extractFeatureParam *param);

/*------------------------------*/
/*            extract           */
/*------------------------------*/
struct modelMatrix *extract(char *coveragesFile, char *trainingFile, struct extractFeatureParam *param);

/*------------------------------*/
/*    extractFeature core       */
/*    lower level function      */
/*------------------------------*/
int extract_core(float **features, struct trainingRegion *trainingRegions, int totalTrainingRegions,
	char **coverages, int totalCoverages, struct extractFeatureParam *param);

/*--------------------------------*/
/*      saveModelMatrix           */
/* get feature from coverage file */
/* as required input format       */
/*--------------------------------*/
int saveModelMatrix(struct modelMatrix *modelMatrix, struct extractFeatureParam *param, char *outputFile);

/*------------------------------*/
/*           coverage           */
/*------------------------------*/
int coverage(char *bamsFile, char *paramFile);

/*------------------------------*/
/*      coverage_core           */
/*------------------------------*/
int coverage_core(char *bam, char *outputFile, struct extractFeatureParam *param);
