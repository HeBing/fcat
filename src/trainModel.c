#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "fcatLib.h"

int menu_trainModel(int argc, char **argv);
int trainModel(char *method, float c, char *trainingFile, char *outputFile); 

/*------------------------*/
/*  trainModel            */
/*------------------------*/
int main(int argc, char **argv) {

	/* menu */
	menu_trainModel(argc, argv);

	/* exit */
	exit(EXIT_SUCCESS);
	
}

int menu_trainModel(int argc, char **argv) {

	/* ------------------------------- */
	/*           trainModel            */
	/* -m method                       */
	/* -c penalty tuning               */
  /*		only used in penalized methods*/
	/* -t training Data                */
	/* -o output model                 */
	/* ------------------------------- */

	if (argc == 1) {
		printf("/*----------------------------------*/\n");
		printf("/*           menu_trainModel        */\n");
		printf("/* -m method                        */\n");
		printf("/*    LogisticRegressionL1          */\n");
		printf("/*    LogisticRegressionL2          */\n");
		printf("/*    RandomForest                  */\n");
		printf("/* -c penalty tuning                */\n");
		printf("/* -t training data file            */\n");
		printf("/* -o output model                  */\n");
		printf("/*----------------------------------*/\n");
		exit(EXIT_SUCCESS);
	}

	char *method = (char *)calloc(MAX_DIR_LEN, sizeof(char));
	char *trainingFile = (char *)calloc(MAX_DIR_LEN, sizeof(char)); 
	char *outputFile = (char *)calloc(MAX_DIR_LEN, sizeof(char));
	float c;

	int ni;
	int mOK = 0, cOK = 0, tOK = 0, oOK = 0;

	ni = 1;
	while (ni < argc) {
		if (strcmp(argv[ni], "-m") == 0) {
			ni++;
			strcpy(method, argv[ni]);
			mOK = 1;
		}
		else if (strcmp(argv[ni], "-c") == 0){
			ni++;
			c = atof(argv[ni]);
			if(c <= 0) {
				printf("invalid value for c %f\n",c);
	      exit(EXIT_FAILURE);
			}
			cOK = 1;
		}
		else if (strcmp(argv[ni], "-t") == 0){
			ni++;
			strcpy(trainingFile, argv[ni]);
			tOK = 1;
		}
		else if (strcmp(argv[ni], "-o") == 0){
			ni++;
			strcpy(outputFile, argv[ni]);
			oOK = 1;
		}
		else {
			printf("Error: unkown parameters!\n");
			exit(EXIT_FAILURE);
		}
		ni++;
	}

	/* check args */
	if ((mOK + cOK + tOK + oOK) < 3){
		printf("Error: input arguments not correct!\n");
		exit(EXIT_FAILURE);
	}
	else {
		if(trainModel(method, c, trainingFile, outputFile) == 1) {
			printf("Error: trainModel failed!\n");
			exit(EXIT_FAILURE);
		}
	}
	

	/* free pointers */
	free(method);
	free(trainingFile);
	free(outputFile);

	return 0;
}

/* train model */
int trainModel(char *method, float c, char *trainingFile, char *outputFile) {

	char *cmd = (char *)calloc(MAX_DIR_LEN, sizeof(char));
	char strC[10];
	sprintf(strC, "%f ", c);

	if(strcmp(method, "LogisticRegressionL2") == 0) {
			strcpy(cmd, "../liblinear-1.96/train -s 7 -c ");
			strcat(cmd, strC);
	}
	else if (strcmp(method, "LogisticRegressionL1") == 0) {
		strcpy(cmd, "../liblinear-1.96/train -s 6 -c ");
    strcat(cmd, strC);
	}
	else if (strcmp(method, "SVM") == 0) {
		strcpy(cmd, "../liblinear-1.96/train -s 5 -c "); 
		strcat(cmd, strC);
	}
	else if (strcmp(method, "RandomForest") == 0) {
		printf("Info: c is not used in RandomForest\n");
		strcpy(cmd, "python ../rt-rank_1.5/do_forest.py ");
		strcat(cmd, trainingFile);
		strcat(cmd, " ");
	}
	else {
		printf("Cannot find method %s. Fitting default model LogisticRegressionL1. \n", method);
		strcpy(cmd, "../liblinear-1.96/train -s 6 -c ");
	}
	
	strcat(cmd, trainingFile);
	strcat(cmd, " ");
	
	// dynamically get the number of features
	char *tmpcmd = (char *)calloc(MAX_DIR_LEN, sizeof(char));
	FILE *tmpFp;
	int row, col;
	
	strcpy(tmpcmd, "awk -F' ' '{print NF; exit}' ");
	strcat(tmpcmd, trainingFile);
	tmpFp = popen(tmpcmd, "r");
	fscanf(tmpFp, "%d\n", &col);
	pclose(tmpFp);

	strcpy(tmpcmd, "wc -l ");
	strcat(tmpcmd, trainingFile);
	tmpFp = popen(tmpcmd, "r");
	fscanf(tmpFp, "%d ", &row);
	pclose(tmpFp);

	free(tmpcmd);

	if(strcmp(method, "RandomForest") == 0) {
		char tmpStr[10];
	
		sprintf(tmpStr, "%d ", col-1);
		strcat(cmd, tmpStr);
		sprintf(tmpStr, "%d ", (int)((col-1)*0.1));

		strcat(cmd, tmpStr);
		strcat(cmd, "100 1 > ");	
	}

	strcat(cmd, outputFile);

	// train model
	printf("Training model %s:\n", method);
	printf("cmd is %s\n", cmd);
	system(cmd);

	free(cmd);
	
	return 0;
}
