#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "math.h"
#include "fcarLib.h"
#include "sam.h"
#include "bam.h"


/*------------------------------*/
/*        static const          */
/*------------------------------*/

/* hard code hg19 genome length */
const uint32_t chrlen[] = { 249250621, 243199373, 198022430, 191154276, 180915260, \
171115067, 159138663, 146364022, 141213431, 135534747, 135006516, \
133851895, 115169878, 107349540, 102531392, 90354753, 81195210, \
78077248, 59128983, 63025520, 48129895, 51304566, 155270560 };

/*----------------------------*/
/*     extractFeature()       */
/* extract feature from input */
/*----------------------------*/
int extractFeature(char *coveragesFile, char *trainingFile, char *outputFile, char *paramFile) {

  /* initialize param */
  struct extractFeatureParam *param = (struct extractFeatureParam *)calloc(1, sizeof(struct extractFeatureParam));
  struct modelMatrix *modelMatrix = (struct modelMatrix *)calloc(1, sizeof(struct modelMatrix));

  /* parse pars */
  parseParam(paramFile, param);

  /* extract feature */
  modelMatrix = extract(coveragesFile, trainingFile, param);

  /* save feature */
  saveModelMatrix(modelMatrix, param, outputFile);

  free(param);
  free(modelMatrix);
  return 0;
}

/*-----------------------------------------*/
/*          saveModelMatrix                */
/* save modelMatirx                        */
/* i.e., extracted features                */
/*-----------------------------------------*/
int saveModelMatrix(struct modelMatrix *modelMatrix, struct extractFeatureParam *param, char *outputFile) {
  
  int i, j;
  char *outputFileName = (char *)calloc(MAX_DIR_LEN, sizeof(char));
  FILE *outputFileFp = NULL;

  /* add param to output file name */
  strcpy(outputFileName, outputFile);
  char *tmp = (char *)calloc(10, sizeof(char));
  sprintf(tmp, "_%d", param->resolution);
  strcat(outputFileName, tmp);
  sprintf(tmp, "_%d", param->windowSize);
  strcat(outputFileName, tmp);
  free(tmp);

  if ((outputFileFp = fopen(outputFileName, "w")) == NULL) {
    printf("Error: cannot open file %s\n", outputFileName);
    exit(EXIT_FAILURE);
  }
  // changed input format to below: 
  // 1 1:1.234 2:3.497
  for (i = 0; i < modelMatrix->n; i++) {
    fprintf(outputFileFp, "%d ", modelMatrix->trainingRegions[i].response);
    for (j = 0; j < modelMatrix->p-1; j++) {
      fprintf(outputFileFp, "%d:%.4f ", j+1, modelMatrix->features[i][j]);
    }
    fprintf(outputFileFp, "%d:%.4f\n", modelMatrix->p, modelMatrix->features[i][modelMatrix->p - 1]);
  }
  
  fclose(outputFileFp);
  printf("\nWriting features to output\n\n");
  free(outputFileName);
  
  return 0;
}

/*-----------------------------------------*/
/*                extract                  */
/* extract features around training region */
/*-----------------------------------------*/
struct modelMatrix *extract(char *coveragesFile, char *trainingFile, 
      struct extractFeatureParam *param){
  
  int i;

  /* read in bams file names */
  int totalCoverages;
  FILE *coveragesFileFp = NULL;
  char **coverages = (char **)calloc(MAX_BAM_FILES, sizeof(char *));
  for (i = 0; i < MAX_BAM_FILES; i++) {
    coverages[i] = (char *)calloc(MAX_DIR_LEN, sizeof(char));
  }

  if ((coveragesFileFp = fopen(coveragesFile, "r")) == NULL) {
    printf("Error: cannot open file %s\n", coveragesFile);
    return EXIT_SUCCESS;
  }

  i = 0;
  while (!feof(coveragesFileFp)) {
    if (i >= MAX_BAM_FILES) {
      printf("Error: the number of input coverages files exceeds the limit %d\n", i);
      return EXIT_SUCCESS;
    }
    fscanf(coveragesFileFp, "%s\n", coverages[i]);
    i++;
  }
  totalCoverages = i;
  fclose(coveragesFileFp);

  /* read in training regions */
  int totalTrainingRegions;
  FILE *trainingFileFp = NULL;
  struct trainingRegion *trainingRegions = (struct trainingRegion *)calloc(MAX_TRAINING_REGIONS, sizeof(struct trainingRegion));
  
  if ((trainingFileFp = fopen(trainingFile, "r")) == NULL) {
    printf("Error: cannot open file %s\n", trainingFile);
    exit(EXIT_FAILURE);
  }
  i = 0;
  while(!feof(trainingFileFp)) {  
    
    if (i >= MAX_TRAINING_REGIONS) {
      printf("Error: the number of training regions exceed the maximum allowed %d\n", MAX_TRAINING_REGIONS);
      printf("i is %d\n", i);
      exit(EXIT_FAILURE);
    }

    if( (fscanf(trainingFileFp, "%d %d %d\n", &trainingRegions[i].chr, &trainingRegions[i].coordinate,&trainingRegions[i].response)) != 3) {
      printf("error reading from %dth row from %s\n", i, trainingFile);
      exit(EXIT_FAILURE);
    }

    // if(i % 1000 == 0) {
      // printf("reading %d %d %d\n", trainingRegions[i].chr, trainingRegions[i].coordinate, trainingRegions[i].response);
    // }
    i++;
  }
  totalTrainingRegions = i;
  fclose(trainingFileFp);
  
  /* extract features */
  struct modelMatrix *modelMatrix = (struct modelMatrix *)calloc(1, sizeof(struct modelMatrix));
  modelMatrix->n = totalTrainingRegions;
  // note windowSize's unit is bp
  modelMatrix->p = (param->windowSize / param->resolution) * totalCoverages; 

  float **features = (float **)calloc(modelMatrix->n, sizeof(float *));
  for (i = 0; i < modelMatrix->n; i++) {
    features[i] = (float *)calloc(modelMatrix->p, sizeof(float));
  }
  
  extract_core(features, trainingRegions, totalTrainingRegions, coverages, totalCoverages, param);

  /* return model matrix */
  modelMatrix->trainingRegions = trainingRegions;
  modelMatrix->features = features;

  /* free pointers */
  free(coverages);

  /* return model matrix pointer */
  return modelMatrix;
}


/*----------------------------*/
/*     extract_core           */
/*----------------------------*/
int extract_core(float **features, struct trainingRegion *trainingRegions, 
          int totalTrainingRegions, char **coverages, int totalCoverages, 
          struct extractFeatureParam *param) {

  int i, j, k;
  printf("\n");
  printf("Extracting features for training regions:\n");
  /* loop through each training region */
  for (i = 0; i < totalTrainingRegions; i++){

    /* check training case */
    if (trainingRegions[i].coordinate - param->windowSize / 2 < 0 &&
      trainingRegions[i].coordinate + param->windowSize / 2 > chrlen[trainingRegions[i].chr - 1]) {
      printf("Warning: cannot extract feature for training case %d %d %d, ignored\n",
        trainingRegions[i].chr, trainingRegions[i].coordinate, trainingRegions[i].response);
      exit(EXIT_FAILURE);
    }

    if((i+1) % 1000 == 0) {
      printf("\tProcessing %dth training region\n", i+1);
    }

    /* find position for training region */
    /* Note: position's unit is bin! not bp! */
    int position = 0;
    if (trainingRegions[i].chr >= 2) { // not chr1
      for (j = 0; j < trainingRegions[i].chr - 1; j++) {
        position += (int)(chrlen[j] / param->resolution) + 1;
      }
    }
    // now position is in the training region
    position += (int)((float)trainingRegions[i].coordinate / (float)param->resolution + 0.5);
    // now change position to begining of the windowSize around training region
    position -= (int)((float)(param->windowSize / 2) / (float)param->resolution + 0.5);
    
    /* extract and write feature from each bam */
    for (k = 0; k < totalCoverages; k++) {

      FILE *coverageFp = NULL;
      
      if ((coverageFp = fopen(coverages[k], "rb")) == NULL) {
        printf("Error: cannot open %s\n", coverages[k]);
        printf("Info: run ./coverage -i bamfiles -p param\n");
        exit(EXIT_FAILURE);
      }
      
      fseek(coverageFp, position*sizeof(float), SEEK_SET);
      
      /* note windowSize's unit is bp not bins! */
      fread(&features[i][(param->windowSize / param->resolution)*k], sizeof(float), param->windowSize / param->resolution, coverageFp);
      
      fclose(coverageFp);
    }
  }

  return 0;
}

/*----------------------------*/
/*       parseParam           */
/*    parse param settings    */
/*----------------------------*/
int parseParam(char *paramFile, struct extractFeatureParam *param){

  FILE *paramFileFp = NULL;

  if ((paramFileFp = fopen(paramFile, "r")) == NULL) {
    printf("Error: cannot open file %s\n", paramFile);
    exit(EXIT_FAILURE);
  }

  // printf("\n\n/*------------------------------------*/\n");
  while (!feof(paramFileFp)){
    char *name = (char *)calloc(MAX_DIR_LEN, sizeof(char));
    char *value = (char *)calloc(MAX_DIR_LEN, sizeof(char));

    fscanf(paramFileFp, "%[^=]=%s\n", name, value);

    if (strcmp(name, "resolution") == 0){
      param->resolution = atoi(value);
      //printf("/* %s is %s\n", name, value);
    }
    else if (strcmp(name, "windowSize") == 0){
      param->windowSize = atoi(value);
      //printf("/* %s is %s\n", name, value);
    }
    else if (strcmp(name, "pairend") == 0) {
      param->pairend = atoi(value);
      //printf("/* %s is %s\n", name, value);
    }
    // added min max length of fragment for pairend data
    else if (strcmp(name, "min") == 0) {
      param->min = atoi(value);
      //printf("/* %s is %s\n", name, value);
    }
    else if (strcmp(name, "max") == 0) {
      param->max = atoi(value);
      //printf("/* %s is %s\n", name, value);
    }
    else {
      //printf("/* Warning: unkown parameters %s=%s, ignored\n", name, value);
    }

    free(name);
    free(value);
  }
  // printf("/*------------------------------------*/\n\n");
  
  fclose(paramFileFp);
  return 0;
}

/*-------------------------------*/
/*          coverage             */
/* count coverage for bams       */
/* Note: by convention, coverage */
/* output filename is the bam file name */
/* plus '.coverage%d', resolution */
/*-------------------------------*/
int coverage(char *bamsFile, char *paramFile) {

  int i;
  int totalBams;
  FILE *bamsFileFp = NULL;
  char **bams = (char **)calloc(MAX_BAM_FILES, sizeof(char *));
  for (i = 0; i < MAX_BAM_FILES; i++) {
    bams[i] = (char *)calloc(MAX_DIR_LEN, sizeof(char));
  }

  /* parse param file */
  struct extractFeatureParam *param = (struct extractFeatureParam *)calloc(1, sizeof(struct extractFeatureParam));
  parseParam(paramFile, param);
  
  /* read in input bam file names */
  if ((bamsFileFp = fopen(bamsFile, "r")) == NULL){
    printf("Error: cannot open file %s\n", bamsFile);
    return EXIT_SUCCESS;
  }

  i = 0;
  while (!feof(bamsFileFp)) {
    if (i >= MAX_BAM_FILES) {
      printf("Error: the number of input bam files exceeds the limit %d\n", i);
      return EXIT_SUCCESS;
    }
    fscanf(bamsFileFp, "%s\n", bams[i]);
    i++;
  }
  totalBams = i;
  fclose(bamsFileFp);

  /* count coverage for each bam */
  for (i = 0; i < totalBams; i++) {
    // note the outputfile name convention here
    char *outputFileName = (char*)calloc(MAX_DIR_LEN, sizeof(char));
    strcpy(outputFileName, bams[i]);
    strcat(outputFileName, ".coverage");
    char tmp[10];
    sprintf(tmp, "%d", param->resolution);
    strcat(outputFileName, tmp);

    printf("Calculating coverage for: \n\t%s and saving to %s\n", bams[i], outputFileName);

    /* count coverage */
    coverage_core(bams[i], outputFileName, param);

    free(outputFileName);
  }

  /* free mem */
  for (i = 0; i < MAX_BAM_FILES; i++) {
    free(bams[i]);
  }
  free(bams);

  return EXIT_SUCCESS;
}

/*--------------------------------*/
/*         coverage_core          */
/* count coverage core            */
/* 09-09-2014 @UPDATE: previously,  each read contribute to only one bin's signal,*/
/*   We now update it to contribute to all the bins covered by it. */
/*   This is necessary because when the resolution is high (10bp), */
/*   the old way will cause the signal to be not smooth and too sparse */
/* 01-16-2015 @UPDATE: added pair-end coverage counting */
/* @NOTE: float is 4 byte on jhpce01 */
/*--------------------------------*/
int coverage_core(char *bam, char *outputFile, struct extractFeatureParam *param) {

  int i, j;

  /* write num of bins at specified resolution */
  /*
  FILE *binNumFp = NULL;
  char *binNumFileName = (char *)calloc(MAX_DIR_LEN, sizeof(char));
  char *binSizeStr = (char *)calloc(10, sizeof(char));
  sprintf(binSizeStr, "%d", param->resolution);
  strcpy(binNumFileName, "binNum");
  strcat(binNumFileName, binSizeStr);
  strcat(binNumFileName, "bp");
  if ((binNumFp = fopen(binNumFileName, "w")) == NULL) {
    printf("cannot open %s\n", binNumFileName);
    return EXIT_SUCCESS;
  }
  for (i = 0; i < NUM_SEQ; i++) {
    fprintf(binNumFp, "%d\n", (int)(chrlen[i] / param->resolution) + 1);
  }
  fclose(binNumFp);
  free(binSizeStr);
  free(binNumFileName);
  */

  /* open bam */
  samfile_t *bamFp = NULL;

  float **binCnt = (float **)calloc(NUM_SEQ, sizeof(float *));
  for (i = 0; i < NUM_SEQ; i++) {
    binCnt[i] = (float *)calloc(((int)(chrlen[i] / param->resolution) + 1), sizeof(float));
  }

  if ((bamFp = samopen(bam, "rb", 0)) == NULL) {
    printf("Error: Cannot open the file %s\n", bam);
    return EXIT_SUCCESS;
  }

  /* adjust bam header ref chr order */
  int tidmap[NUM_SEQ + 1];
  for (i = 0; i < NUM_SEQ + 1; i++) {
    tidmap[i] = i;
  }
  if (strcmp(bamFp->header->target_name[1], "chr11") == 0) {
    tidmap[0] = 9; tidmap[1] = 10; tidmap[2] = 11; tidmap[3] = 12;
    tidmap[4] = 13; tidmap[5] = 14; tidmap[6] = 15; tidmap[7] = 16;
    tidmap[8] = 17; tidmap[9] = 18;
    tidmap[10] = 0;
    tidmap[11] = 19; tidmap[12] = 20; tidmap[13] = 21;
    tidmap[14] = 1; tidmap[15] = 2; tidmap[16] = 3; tidmap[17] = 4;
    tidmap[18] = 5; tidmap[19] = 6; tidmap[20] = 7; tidmap[21] = 8;
    tidmap[22] = 23; tidmap[23] = 22;
  }

  /* start count coverage */
  int nread = 0;
  bam1_t *read = NULL;

  read = bam_init1();

  if(param->pairend == 0) {
    while (samread(bamFp, read) > 0) {
      if (read->core.tid < 0 || read->core.tid > 23) {
        continue;
      }
      /* @UPDATE on 09-09-2014 */
      /* increment signals at all covered bins */
      if (tidmap[read->core.tid] < NUM_SEQ) {
        int startBin, endBin;
        startBin = (int)((float)read->core.pos / (float)param->resolution + 0.5);
        endBin = (int)((float)(read->core.pos + read->core.l_qseq) / (float)param->resolution + 0.5);

        int b;
        for(b = startBin; b <= endBin; b++) {
          binCnt[tidmap[read->core.tid]][b]++;
        }
      }
      nread++;
      if (nread % 10000000 == 0) {
        printf("\t\tProcessing %dth read\n", nread);
      }
    }
  }

  if(param->pairend == 1) {
    // added for pair end coverage counting
    int is2ndMate = 0;
    while (samread(bamFp, read) > 0) {
      if (read->core.tid < 0 || read->core.tid > 23) {
        continue;
      }
      /* @UPDATE on 09-09-2014 */
      /* increment signals at all covered bins */
      if (tidmap[read->core.tid] < NUM_SEQ) {
        if(is2ndMate == 1) {
          is2ndMate = 0; // assuming bam is sorted by id number
          continue;
        } 
        else {
          uint32_t left, right;
          if(read->core.pos > read->core.mpos) {
            left = read->core.mpos;
            right = read->core.pos;
          }
          else {
            left = read->core.pos;
            right = read->core.mpos;
          }
          // filter out fragments with specified [min,max]
          if(right - left + read->core.l_qseq < param->min || right - left + read->core.l_qseq > param->max) {
            is2ndMate = 1;
            continue;
          }
          int startBin, endBin;
          startBin = (int)((float) left / (float)param->resolution + 0.5);
          endBin = (int)((float)(right + read->core.l_qseq) / (float)param->resolution + 0.5);
          is2ndMate = 1;
          int b;
          // printf("startBin is %d and endBin is %d\n", startBin, endBin);
          for(b = startBin; b <= endBin; b++) {
            binCnt[tidmap[read->core.tid]][b]++;
          }
        }
      }
      nread++;
      if (nread % 10000000 == 0) {
        printf("\t\tProcessing %dth read\n", nread);
      }
    }
  }

  bam_destroy1(read);
  samclose(bamFp);

  /* take log2 of coverage */
  for(i = 0; i < NUM_SEQ; i++) {
    for(j = 0; j < (int)(chrlen[i] / param->resolution) + 1; j++) {
      binCnt[i][j] = (float)(log2((double)(binCnt[i][j]+1)));
    }
  }
  
  /* write coverage */
  FILE *binCntFp = NULL;

  if ((binCntFp = fopen(outputFile, "wb")) == NULL) {
    printf("Error: Cannot open the file %s\n", outputFile);
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < NUM_SEQ; i++) {
    fwrite(binCnt[i], sizeof(float), ((int)(chrlen[i] / param->resolution) + 1), binCntFp);
  }
  fclose(binCntFp);

  // free binCnt
  for (i = 0; i < NUM_SEQ; i++) {
    free(binCnt[i]);
  }
  free(binCnt);

  return 0;
}

