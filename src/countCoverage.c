#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "fcarLib.h"

int menu_countCoverage(int argc, char **argv);

int main(int argc, char **argv) {

	/* menu */
	menu_countCoverage(argc, argv);

	/* exit */
	exit(EXIT_SUCCESS);
}

int menu_countCoverage(int argc, char **argv) {

	/* ------------------------------- */
	/*        countCoverage            */
	/* -i bamsFile                     */
	/* -p paramFile                    */
	/* ------------------------------- */

	if (argc == 1) {
		printf("/*-----------------------------------*/\n");
		printf("/*            extractFeature         */\n");
		printf("/* -i bams file list                 */\n");
		printf("/* -p parameter setting file         */\n");
		printf("/*-----------------------------------*/\n");
		exit(EXIT_SUCCESS);
	}

	char *bamsFile = (char *)calloc(MAX_DIR_LEN, sizeof(char)); 
	char *paramFile = (char *)calloc(MAX_DIR_LEN, sizeof(char));
	int ni;
	int iOK = 0, pOK = 0;

	ni = 1;
	while (ni < argc) {
		if (strcmp(argv[ni], "-i") == 0) {
			ni++;
			strcpy(bamsFile, argv[ni]);
			iOK = 1;
		}
		else if (strcmp(argv[ni], "-p") == 0){
			ni++;
			strcpy(paramFile, argv[ni]);
			pOK = 1;
		}
		else {
			printf("Error: unkown parameters!\n");
			exit(EXIT_FAILURE);
		}
		ni++;
	}

	/* check args */
	if ((iOK + pOK) < 2){
		printf("Error: input arguments not correct!\n");
		exit(EXIT_FAILURE);
	}
	else {
		coverage(bamsFile, paramFile);
	}

	/* free pointers */
	free(bamsFile);
	free(paramFile);

	return 0;
}
