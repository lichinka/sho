/***************************************************************************
 *   Copyright (C) 2010 by Peter Korošec								   *
 *																		   *
 *   This material is provided "as is", with no warranty expressed or      *
 *   implied. Any use is at your own risk. Permission to use or copy this  *
 *   software forany purpose is hereby granted without fee, provided this  *
 *   notice isretained on all copies. Permission to modify the code and to *
 *   distributemodified code is granted, provided a notice that the code   *
 *   was modified isincluded with the above copyright notice.              *
 *                                                                         *
 *   Authors web page: http://csd.ijs.si/korosec/                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _CRT_SECURE_NO_WARNINGS

#include <time.h>
#include "dasa.h"
#include "memory.h"
#include "errors.h"

//
// Variables and functions that are specific to the
// "SHO balancing problem: CPICH optimization" for WCCI 2012
//
#include "sho_eval.h"



/* declaration of global variables */ 

int fFunction;                     // ID of the function to be optimized
int numOfParams;                   // dimension of the problem
int lastAlgIteration;              // counter of the tries/runs of the optimization algorithm
int fRandSeed;                     // seed of the random generator 
int numOfAnts;                     // number of ants 
int maxTime;                       // upper limit on the time of execution in terms of evaluations
int traceFrequnecy;                // the time period (in evalutions) of reporting/tracing results 
int numOfResets;                   // number of resets of the search process in the case it is trapped in local optimum
int accu;                          // accuracy of the algorithm 
int fDiscreteBase;                 // "discrete base" used for generation of the differential graph
double evapValue;                  // phermone evaporation: dispersion factor ρ in DASA papers
double CauchyIncPer;               // global scale-increasing factor (1+s+) in DASA papers
double CauchyDecPer;               // global scale-decreasing factor (1-s-) in DASA papers
TValue optimum;                    // the known best value of the objective function/score/error

TEVector fLow, fHigh, fStep,fStart;// vector of the values of the lower and higher bounds of the parameters' values, their precisions, and their initial values
TEVector currMeans;                // vector of the positions of the vertices on the X axis (local offsets of the Cauchy distr.), scaled to fit in the interval [-4,4]
TEVector LowCauchy, HighCauchy;    // vector of the Cauchy probablities at the bounds of the interval [-4,4], see DASA paper
TEVector CauchyXPosition;          // vector of the means (position of the vertices on the X axis ) of the Cauchy distributions 

Boolean_T printAll;                // control parameter: if set, print all intermidiate results on the screen 
Boolean_T logging;                 // control parameter: if set a log of the final results outputed on the screen
Boolean_T bHistory;                // control parameter: if set a brief report from the search proces is kept in a separte file
Boolean_T bStartSol;               // control parameter: if set, start solution is available
Boolean_T bAcceptEquals;           // control parameter: if set, accept the new solution if less or EQUAL to the old best solution



TEMatrix currentSolutions;         // matrix of the current solutions with size 'numOfAnts' by 'numOfParams'+1 
TEVector tempBestSolution;         // vector of the temporary best solution 
TEVector iterBestSolution;         // vector of the best solution in the whole iteration
TEVector bestSolution;             // vector of the global best solution
TEVector currentSolution;          // vector of the current solution 
TEVector scores;                   // vector with the values of the objective function 
TValue bestScore;                  // the best found objective function value
TMatrix antPaths;                  // matrix of the vertices of the path of the ants in the current iteration 
int bestFound;                     // counter of the number all found best solution 

TValue *history;                   // vector with the values of the objective function traced every 'traceFrequnecy' evaluations 

TConnection **graph=NULL;          // pointer to the differential graph (discrete form of the search contious domain) 
TVector graphWidth;                // width of the graph = number of "parameter diffrences"

int iTime;                         // curent number of evalutions
char problemName[256];             // string with the problem name
char fileName[256];                // used for formulation of the files' names that trace the results (report) of the optimization

char tempString[80];



/* local variables */

// for execution-time calculations
time_t startTime, stopTime; 
long long mStartTime, mStopTime;

/*  
**  Initialization of the algorithm: creates the necessary folder for 
**  saving the reports and results, and additionally sets the values of 
**  the global variables, using the deafult values specified in the 
**  INI_FILE ( "DASA.ini" defined in dasa.h). 

**  No input/output arguments.                        
*/

#include <sys/stat.h>

void InitVariables()
{
  DIR * pDIR;

  if ((pDIR = opendir("scores")))
		closedir(pDIR);
  else
		mkdir("scores",0777);

  if ((pDIR = opendir("history")))
		closedir(pDIR);
  else
		mkdir("history",0777);

  if ((pDIR = opendir("params")))
		closedir(pDIR);
  else
		mkdir("params",0777);

  if ((pDIR = opendir("stats")))
		closedir(pDIR);
  else
		mkdir("stats",0777);

  readIni(INI_FILE, SECTION, "NumOfParam", Cfg_Short, &numOfParams);
  readIni(INI_FILE, SECTION, "ChosenFunction", Cfg_Short, &fFunction);
  readIni(INI_FILE, SECTION, "LastAlgIteration", Cfg_Short, &lastAlgIteration);
  readIni(INI_FILE, SECTION, "NumOfAnts", Cfg_Short, &numOfAnts);
  readIni(INI_FILE, SECTION, "Time", Cfg_Short, &maxTime);
  readIni(INI_FILE, SECTION, "TraceFrequency", Cfg_Short, &traceFrequnecy);
  readIni(INI_FILE, SECTION, "RandSeed", Cfg_Short, &fRandSeed);
  readIni(INI_FILE, SECTION, "Accuracy", Cfg_Short, &accu);
  readIni(INI_FILE, SECTION, "DiscreteBase", Cfg_Short, &fDiscreteBase);
  readIni(INI_FILE, SECTION, "Evap", Cfg_Double, &evapValue);
  readIni(INI_FILE, SECTION, "CauchyIncPer", Cfg_Double, &CauchyIncPer);
  readIni(INI_FILE, SECTION, "CauchyDecPer", Cfg_Double, &CauchyDecPer);
  readIni(INI_FILE, SECTION, "Optimum", Cfg_Double, &optimum);	
};


/*  
**  This function loads the data necessary for the search process.
**  It loads the number of the parameters, the bounds on the  
**  parameters' values, their precisions, their initial values (if supplied)
**  the optimal score value and the accuracy of the search. Based on that 
**  allocates the memory for the global vectors/matrices used in the search.

**  Input:  name of the file with parameters info-data;
		    name of the file with the initial solution;
		    name of the file that contains the optimal score;
**  Output: integer { 1- successful load; -1 fail)               
*/

int LoadParams(const char * nameOfParams, const char * nameOfStartSol, const char * nameOfOptimum)
{
  FILE *inpFile;
  char FileName[80], data[80], line[80];
  int  lCount = -1;
  int  i, j, numOfAllParams = 0;
  int  multiply = 1;
  int  mulFrom = 0;
  char * lineStart;

  // create the full name of the file with parameters data and open it for reading 
  sprintf(FileName, "./params/%s.params", nameOfParams);
  inpFile = (FILE *) cant((char *)FileName, "r"); 
  
  // repeat the following: read a line from the file and parse the line to get the 
  // left bound and right bound of the prameter's values interval, parameters precision  
  // until'numOfParams' lines are read or end of the file is reached  
  while ((lCount<numOfParams)&&(EOF != ReadLine(inpFile, line))) {
		lineStart = &line[0];
		if (lCount == -1) {                               // only for the first line
			lineStart = ParseLine1(lineStart, data, ' '); // parse the first word which idicates the number of colums in the file (ignoring the format in the first line)
			if ((strlen(data)==0)||(atoi(data) != 3)) {
				printf("Wrong number of variables in ./params/%s.params!\n", nameOfParams);
				return -1;
			}
			lineStart = ParseLine1(lineStart, data, ' ');
			if (strlen(data)>0)
				numOfAllParams = atoi(data);              // the second word indicates the number of lines in the file without the first line (hight of the colums)
			
			lineStart = ParseLine1(lineStart, data, ' ');
			if (strlen(data)>0)
				multiply = atoi(data);                   /* if we have parameters with the same interval and precision, the third word indicates the number 
														    of times of their repetition (compressed respresantion of the problem in a file)*/
		    else
				multiply = 1;
			lineStart = ParseLine1(lineStart, data, ' '); 
			if (strlen(data)>0)
				mulFrom = atoi(data);                   /* the last word indicates the index of the last line with uniqe values, 
			                                               the next lines should be loaded 'multiply' times */
			else
				mulFrom = 0;
			if (multiply<1)
				multiply = 1;
			if (numOfParams == -1) {
				numOfParams = numOfAllParams+(numOfAllParams-mulFrom)*(multiply-1);
			} else if (numOfParams>numOfAllParams) {
				printf("Number of params (dimension) is higher than number of variables in ./params/%s.params!\n", nameOfParams);
				return -1;
			}
			allocate_memory();                            // allocate the necessary memory
		} else {                                          // for the line[i], i>1
			lineStart = ParseLine1(lineStart, data, ' '); // parse the left bound of the parameter[i]
			if (strlen(data)>0)
				fLow[lCount] = atof(data);
			lineStart = ParseLine1(lineStart, data, ' '); // parse the right bound of the parameter[i]
			if (strlen(data)>0)
				fHigh[lCount] = atof(data);
			lineStart = ParseLine1(lineStart, data, ' '); // parse the precision of the parameter[i]
			if (strlen(data)>0)
				fStep[lCount] = atof(data);
		}
		lCount++;
	}
	fclose(inpFile);                                     // close the file
	for (j=1; j<multiply; j++) {                         // multiplication of the data
		for (i=0; i<numOfAllParams-mulFrom; i++) {
			fLow[j*(numOfAllParams-mulFrom) + mulFrom+i]  = fLow[mulFrom+i];
			fHigh[j*(numOfAllParams-mulFrom) + mulFrom+i] = fHigh[mulFrom+i];
			fStep[j*(numOfAllParams-mulFrom) + mulFrom+i] = fStep[mulFrom+i];
		}
	}
	if (strcmp(nameOfStartSol, "") != 0) {               //if a file name of a initial solution is specified

		int numOfSols=0;
		lCount = -1;

        // create the full name of the file and open it for reading 
		sprintf(FileName, "./params/%s.startSol", nameOfStartSol);
		inpFile = (FILE *) cant((char *)FileName, "r");

        // repeat the following: read a line from the file and parse the line into necessary data, 
        // until 'numOfParams' lines are read or end of the file is reached  
		while ((lCount<numOfParams)&&(EOF != ReadLine(inpFile, line))) {
			lineStart = &line[0];
			if (lCount == -1) {                                // only for the first line
				lineStart = ParseLine1(lineStart, data, ' ');  // parse the number of solutions in the file 
				if ((strlen(data)==0)||(atoi(data)!=1))
					break;
				lineStart = ParseLine1(lineStart, data, ' ');  // parse the dimension of the solution
				if (strlen(data)>0)
					numOfSols = atoi(data);
				if (numOfParams != numOfSols) {
					printf("Wrong number of variables in ./params/%s.startSol!\n", nameOfStartSol);
					break;
				}
			} else {                                           // for the line[i], i>1
				lineStart = ParseLine1(lineStart, data, ' ');  // parse the initial value of the parameter[i]
				if (strlen(data)>0)
					fStart[lCount] = atof(data);
			}
			lCount++;
		}
		bStartSol = True_;                                    // set the control parameter: initial solution exsists
		fclose(inpFile);                                      // close the file
	}
	if (strcmp(nameOfOptimum, "") != 0) {                    // if a name of the file with the optimum is specified
		lCount = -1;

		// create the full name of the file and open it for reading 
		sprintf(FileName, "./params/%s.opt", nameOfOptimum);
		inpFile = (FILE *) cant((char *)FileName, "r");
		ReadLine(inpFile, line);                              // read a line
		ParseLine1(line, data, ' ');                          // parse the optimal value
		if (strlen(data)>0) 
			optimum = atof(data);
		ReadLine(inpFile, line);                              // read a line
		ParseLine1(line, data, ' ');                          // parse the accuracy
		if (strlen(data)>0) 
			accu = atoi(data);
		fclose(inpFile);                                      // close the file
	}
	return 1;
}



/*
** This function calcualtes the objective function of the optimization 
** problem, that is given by the sum of the squared error over all 
** parameters. Additonaly it is shifted by a specified bias, to avoid  
** the zero value as a global optimum.

** Input:  ID of the optimization problem;
		   dimension of the parameter vector;
		   constant Bolean_T (False_);
		   seed for a random generator (if noise is added in the function evaluation);
		   vector of the values of the solution to be evaluated;
** Output: value of the objective function (error).
*/

TValue CalcLocalFunc(const int aFunction, int aDimension, const Boolean_T aLog, const double aSeed, TEVector aParams)
{
	int j;
	TValue temp = 0.0, res = 0.0;
	TValue tParams[1001];

	if (aDimension > 1000)
        aDimension = 1000;
	

	switch (aFunction) {
		case 1: //shifted sphere
			for (j=0; j<aDimension; j++) 
				res += sqrv(aParams[j]-sphereShift[j]);
			res = res + f_bias[0];
			break;
		case 2: //shifted schwefel
			res = abss(aParams[0]);
			for (j=1; j<aDimension; j++)
            {
				temp = abss (aParams[j] - schwefelShift[j]);
                //
                // the 'max' function does not always work here,
                // most likely because of the different types 'res'
                // may take
                //
                // res = max (res, temp);
                //
                if (temp > res)
                    res = temp;
			}
			res = res + f_bias[1];
			break;
		case 3: //shifted rosenbrock
			for (j=0; j<aDimension; j++)
			tParams[j] = aParams[j] - rosenbrockShift[j] + 1.0;
			for (j=0; j< aDimension - 1; j++)
				res = res + 100.0*(sqrv(sqrv(tParams[j])-tParams[j+1])) + sqrv(tParams[j]-1.0);
			res = res + f_bias[2];
			break;
		case 4: //shifted rastrigin
			res = 10.0*aDimension;
			for (j=0; j<aDimension; j++)
				res = res + sqrv(aParams[j]-rastriginShift[j]) - (10.0*cos(2.0*M_PI*(aParams[j]-rastriginShift[j])));
			res = res + f_bias[3];
			break;
		case 5: //shifted griewank
			for (j=0; j<aDimension; j++)
				 res = res + sqrv(aParams[j]-griewankShift[j])/4000.0;
			temp = 1.0;
		    for (j=0; j<aDimension; j++)
				 temp = temp * (cos((aParams[j]-griewankShift[j])/sqrt(j+1.0)));
			res = res - temp + 1 + f_bias[4];
			break;
		case 6: //shifted ackley
			temp = 0.0;
			for (j=0; j<aDimension; j++) {
				res = res + sqrv(aParams[j]-ackleyShift[j]);
				temp = temp + cos(2.0*M_PI*(aParams[j]-ackleyShift[j]));
			};
		    res = -20.0*exp(-0.2*sqrt(res/aDimension)) - exp(temp/aDimension) + 20.0 + exp(1.0) + f_bias[5];
			break;
        //
        // SHO balancing evaluation function
        //
        case 101:
            //
            // this function is defined in 'sho_eval.h'
            //
            res = sho_evaluate (aParams, aDimension);
            break;
		default: ;
	}
	
	aParams[aDimension] = res;

	return res;
}


/*
** Function that randomly selects solution from the given interval 
** of values, evaluates the one and prints the result on the screen. 

** Input:  vector with the parameters values to be evaluated;
** Output: value of the objective function (error).
*/

TValue GetRandomSolution(TEVector solution)
{
  int j, temp;
  TValue res;
  char s1[80];
  char s2[80];
  
  for (j=0; j<numOfParams; j++) {                                  // for every parameter
	 temp = (int) (((random01)*(fHigh[j]-fLow[j]))/fStep[j]);      // sample a random number from a uniform distribution
    (solution)[j] = (TValue) fLow[j] + (TValue) temp*fStep[j];     // rescale the value to fit in the specified interval 
  };

  //
  // DEBUG !!!
  //
  //for (j=0; j<numOfParams; j++)
  //{
  //  printf ("random solution, param %i, %30.16f\n", j, solution[j]);
  //}

  (solution)[numOfParams] = CalcLocalFunc(fFunction, numOfParams, False_, 0.0, solution); // evaluate the sampled solution 

  iTime++;                         // increment the counter of the evaluations
  
  res = (solution)[numOfParams];  // place the result for output 

  if ((printAll)) {               // if printAll is set , print on the screen a short report on the search results
		FloatAsStr(res, optimum, False_, False_, s1);
		FloatAsStr(res, optimum, True_, True_, s2);

#ifdef longType
       printf("Rnd: %s %d t: %d s: %s e: %s b: %Lf\n", fileName, lastAlgIteration, iTime, s1, s2, bestScore);
#else
       printf("Rnd: %s %d t: %d s: %s e: %s b: %f\n", fileName, lastAlgIteration, iTime, s1, s2, bestScore);
#endif

  };
	return res;
}

/*
** Procedure that keeps a trace of the optimization hystory
** in separate .hystory file in 'hystory' folder for every  
** execution of a given problem.

** Input: name of the problem (to create the name of the .hystory file);
		  value of the best score/error;
		  number of evaluation;
		  number of parameter/dimension of the problem;
          id/index of the run;
		  seed of the random generator;
** No output arguments.
*/

void AddHistory(char * aFileName, const TValue aScore, const int aNumOfIter, const int aNumOfParam, const int aLastAlgIteration, const int aRandSeed)
{
  char statsFile[256];
  char tempStr[1000];
  char tempPar[100];
  time_t deltaTime;
  struct tm * delta;
  char s1[80];
  char s2[80];
  sprintf(statsFile, "./history/%s-%d.history", aFileName, numOfParams);
  sprintf(tempPar, "Alg. iteration %d score", lastAlgIteration);

  deltaTime = stopTime-startTime;
  if (deltaTime < 0)
		deltaTime = 0;
  delta = gmtime(&deltaTime);

  FloatAsStr(aScore, optimum, False_, False_, s1);
  FloatAsStr(aScore, optimum, True_, False_, s2);
  sprintf(tempStr, "%s err %s in %d (%d) iterations, time of %d day(s) %02d:%02d:%02d or %s ms and randSeed of %d", s1, s2, aNumOfIter, bestFound, (delta->tm_mday-1) ,delta->tm_hour, delta->tm_min, delta->tm_sec, commaprint((unsigned long) mStopTime-mStartTime), aRandSeed);

  writeIni(statsFile, "History", tempPar, tempStr);
}


/*
** Procedure that keeps a trace of all scores from a given run 
** and given problem in separate .score file in 'scores' folder. 

** Input: name of the problem (to create the name of the .score file);
          id/index of the run;
** No output arguments.
*/

void SaveScores(const char * afileName, const int aLastAlgIteration)
{
	char scoresFile[256];
	FILE *HistoryFile;
	int j;
			sprintf(scoresFile, "./scores/%s-%d-%d.scores", afileName, numOfParams, aLastAlgIteration);
			HistoryFile = cant(scoresFile, "w");
			for (j=0; j<(maxTime / traceFrequnecy); j++) {
			
#ifdef longType
				fprintf(HistoryFile, "%Lf\n", history[j]);
#else
				fprintf(HistoryFile, "%f\n", history[j]);
#endif

			}
			fclose(HistoryFile);
}


/*
** Procedure that saves a detailed log for every execution of 
** a given problem in a separate .stats file in 'stats' folder.

** Input: name of the problem (to create the name of the .stats file);
          id/index of the run;
		  seed of the random generator;
** No output arguments.
*/

void SaveStats(const char * aFileName, const int aLastAlgIteration, const int aRandSeed)
{
  int i;
  char statsFile[256];
  char tempStr[100];
  char tempPar[10];
  time_t deltaTime;
  struct tm * delta; 

  sprintf(statsFile, "./stats/%s-%d-%d.stats", aFileName, numOfParams, aLastAlgIteration);
  sprintf(tempStr, "%d", iTime);
  writeIni(statsFile, "Stats", "Num of iterations", tempStr);
  sprintf(tempStr, "%d", aRandSeed);
  writeIni(statsFile, "Stats", "RandSeed", tempStr);
  sprintf(tempStr, "%d", bestFound);
  writeIni(statsFile, "Stats", "Best found", tempStr);
#ifdef longType
  sprintf(tempStr, "%Lf", bestScore);
#else
  sprintf(tempStr, "%f", bestScore);
#endif
  writeIni(statsFile, "Stats", "Best score", tempStr);
  FloatAsStr(bestScore, optimum, True_, True_, tempStr);
  writeIni(statsFile, "Stats", "Err", tempStr);

  deltaTime = stopTime-startTime;
  delta = gmtime(&deltaTime);
  sprintf(tempStr, "%d day(s) %02d:%02d:%02d or %s ms", (delta->tm_mday-1) ,delta->tm_hour, delta->tm_min, delta->tm_sec, commaprint((unsigned long) mStopTime-mStartTime));
  writeIni(statsFile, "Stats", "Runtime", tempStr);
  for (i=0;i<numOfParams; i++) {
		
#ifdef longType
    sprintf(tempStr, "%5.20Lf", bestSolution[i]);
#else
    sprintf(tempStr, "%5.20f", bestSolution[i]);
#endif
    sprintf(tempPar, "p%d", i);
	writeIni(statsFile, "Solution", tempPar, tempStr);
  }
  
  sprintf(tempStr, "%d", numOfParams);
  writeIni(statsFile, "Params", "NumOfParam", tempStr);
  sprintf(tempStr, "%d", fFunction);
  writeIni(statsFile, "Params", "ChosenFunction", tempStr);
  sprintf(tempStr, "%d", numOfAnts);
  writeIni(statsFile, "Params", "NumOfAnts", tempStr);
  sprintf(tempStr, "%d", maxTime);
  writeIni(statsFile, "Params", "Time", tempStr);
  sprintf(tempStr, "%f", CauchyIncPer);
  writeIni(statsFile, "Params", "CauchyIncPer", tempStr);
  sprintf(tempStr, "%f", CauchyDecPer);
  writeIni(statsFile, "Params", "CauchyDecPer", tempStr);
  sprintf(tempStr, "%f", evapValue);
  writeIni(statsFile, "Params", "Evap", tempStr);
  sprintf(tempStr, "%d", fDiscreteBase);
  writeIni(statsFile, "Params", "DiscreteBase", tempStr);
}


/*
** The procedure strats the optimization process.
** Includes: additonal check-ups, settings, creation of the 
** the graph for the ants to wlak on,  starts the search process
** and at last prints and saves the results of the search.

** No input/otput arguments.
*/
void RunOptimization()
{
	int j;
	time_t deltaTime;
	struct tm * delta;

  //set the seed of the random generator
  if (fRandSeed == 0) {
     fRandSeed = (unsigned) time(NULL);
	 srand((unsigned) fRandSeed);
  } else
     srand((unsigned) fRandSeed );

  // check and set (if not set) the discrete base 
  if (fDiscreteBase < 2)	
	fDiscreteBase = 2;

  //start timing
  startTime = time(NULL);
  mStartTime = mtime();
	
  // initialize the counter of the evalutions
  iTime = -1;

  //create a graph for ants to walk on
  if (CreateGraph()) {

		//init some variables
		evapValue = 1-evapValue;
		bestFound = 0;

		//if starting solutions was defined evaluate it, otherwise get random one
		if (bStartSol) {
			 for (j=0; j<numOfParams; j++)
				 bestSolution[j] = fStart[j];
			bestSolution[numOfParams] = CalcLocalFunc(fFunction, numOfParams, False_, 0.0, bestSolution);
			bestScore = bestSolution[numOfParams];
		    if ((printAll)) { 
		      FloatAsStr(bestScore, optimum, True_, True_, tempString);
#ifdef longType
              printf("Init: %s %d t: 0 s: %Lf e: %s b: %Lf\n", fileName, lastAlgIteration, bestScore, tempString ,bestScore);
#else
              printf("Init: %s %d t: 0 s: %f e: %s b: %f\n", fileName, lastAlgIteration, bestScore, tempString ,bestScore);
#endif   

			  }
		  } else 
			    bestScore = GetRandomSolution(bestSolution);

          // initialize the counter of algorithm resets
		  numOfResets = 0;

		  // store RandSeed in case of error
		  AddHistory(fileName, bestScore, iTime, numOfParams, lastAlgIteration, fRandSeed);

		 if (maxTime>0) 
		 {
			// initiate the search procedure - ants start work
			AntsWork(); 
		 }	
        
		 // stop the timers
		 mStopTime = mtime();
		 stopTime = time(NULL);
		 // calcualte the elapsed execution time
		 deltaTime = stopTime-startTime; 

		 // get the time as structure( years, months, days, hours, seconds) for formated print 
		 delta = gmtime(&deltaTime);
		 printf("Stopwatch: %d day(s) %02d:%02d:%02d or %s ms\n", (delta->tm_mday-1) ,delta->tm_hour, delta->tm_min, delta->tm_sec, commaprint((unsigned long) mStopTime-mStartTime));
	    
		 // Store the short report of the completed search in the 'history' folder
		 AddHistory(fileName, bestScore, iTime, numOfParams, lastAlgIteration, fRandSeed);

		 // Store the a detailed report of the completed search in the 'stats' folder
		 SaveStats(fileName, lastAlgIteration, fRandSeed);

         // Store the vector of traced function evalutions/scores/errors in the 'scores' folder 
		 SaveScores(fileName, lastAlgIteration);
	
	}
    
    // de-allocate the memory 
	free_memory();
}


/*
** Function that creates the differential graph. It sets the 
** width of the graph and sets the values in the vertices of  
** the graph structure. 

** No input arguments.
** Output: Boolean_T value {True_ - created graph, False_ - fail}. 
*/

Boolean_T CreateGraph()
{
	int i, j;
	int valRange;
	double valInerStep;
	double valStep;
    
	// if the dimension of the problem is specified
	if  (numOfParams>0) {
		for (i=0; i<numOfParams; i++) { // for every parameter

            // define the range of the vertix values
			double valFrom = fLow[i]; 
			double valTo = fHigh[i];

           // define the multiplication factor-step
			valStep = abss(fStep[i]);
			if (valStep == 0)
				valStep = 1e-15;         // deafult value
			valInerStep = valStep;
			if ((valTo - valFrom) < 0) { // if not positive
               valRange = 20;            // deafult value
            } else {                     // calucate the number of vertices-parameter differences 
                  valRange = 0;
                  while (valInerStep <= (valTo-valFrom)) {
                      valRange++;
                      valInerStep = valInerStep * fDiscreteBase;
                  };
            };
		
			graphWidth[i] = valRange*2 + 1; // the width of the graph 
			graph[i][valRange].vert = 0;    // the middle vertex has value 0, and reppresents zero change of the paramteter
			valInerStep = valStep;    
			

			for (j=1; j<=valRange; j++) {   // calculation of the value of the rest vertices

				graph[i][valRange+j].vert = valInerStep;
				graph[i][valRange-j].vert = -graph[i][valRange+j].vert; // simetrical grpah
				valInerStep = valInerStep * fDiscreteBase;
			};
		};

		return True_;
	} else
		return False_;
}

/*
** Procedure that setups the working grpah for ants to walk on it,
** an initial amount of phermone is deposited on the graph and the
** single-colony ant search is started

** No input/otput arguments.
*/
void AntsWork()
{
	int j;

	// initialization of the positions (local offsets) of the 
	// Cauchy distributions used for initial phermone distribution  
	// on the search graph
    for (j=0; j<numOfParams; j++) {
      CauchyXPosition[j] = 0.0;
    };

    // new best solution found; increment the counter of improved best solutions
	bestFound++; 

	// make the best solution a temporal best one
	for (j=0; j<numOfParams; j++)
		tempBestSolution[j] = bestSolution[j]; 

    // start the search process with one group(colony) of ants
	AntGroup(0, 0, numOfAnts);

};

/*
** Iterative procedure that enables the ant-based 
** approach for solving numerical optimization

** Input: ID of the working colony (group) of ants;
		  number of colonies used in the search;
          number of ants used in the search;
  Remark: This implementation uses only single 
          colony, and the first two argument are 
		  set to 0 when procedure is called. 
		  Shoud be slithly modified to work with 
		  more colonies.
** No output arguments.
*/

void AntGroup(const int aGroup, const int numOfGroups, const int allAnts)
{
	
	TValue offset;    // parameter difference (found by the search) 
    double avgOffset; // avarage of found offsets
	int currAnt;      // index of the current ant
	int i;	
	int k;
	int j;
    char s1[80], s2[80], s3[80];

	//initialization 

	TValue tempBestScore = bestScore;            // initialize temporary best score
	TValue iterBestScore = tempBestScore;        // initialize temporary best score
	int resetPheromone = 0;                      // control parameter: if set, the search should be restarted
   
	double initCauchyScale = 1.0;                // global scale factor(manages the expoloartion/exploitation of the offset space)
	double cauchyScaleLocal = 0.0;               // local scale factor(temporary exploitation inside a possably good offset region)
	double cauchyScaleGlobal = initCauchyScale;
	double currScale = 0.0;                      // current scale factor in the Cauchy distribution

  // iterate the procedure while the maximal number of evalutions is reached	
  while (iTime<maxTime) {
	
		avgOffset = 0.0;
        
	    // if control parameter resetPhermone is set, 
		// a local optimum was found: restart the search process
		if (resetPheromone) {
             
			// reinitialize the phermone distribution by
			// the standard Cauchy distribution
			for (k=0; k<numOfParams; k++) {
				CauchyXPosition[k] = 0.0;
			};

			cauchyScaleGlobal = initCauchyScale;
			cauchyScaleLocal = 0.0;

            // select a temporal best solution randomly and evaluate the one
			tempBestScore = GetRandomSolution(tempBestSolution);
			iterBestScore = tempBestScore;

		    // increment the number of resets
			numOfResets++; 

			// reset the control parameter
			resetPheromone = 0;
		}
	
	   // all ants choose current parameter value according to the current solution
	   currScale = cauchyScaleGlobal-cauchyScaleLocal;

       // scale the local offsets to fit in the range [-4,4] w.r.t to the standard 
	   // Caushy distribution  
	   for (i=0; i<numOfParams; i++) {
		int valRange = (int) (graphWidth[i] / 2);
						
		if (valRange > 0) 
			// new means of the phermone distributions
			currMeans[i] = ((CauchyXPosition[i]/ (double) valRange) * 4.0);
		else
			currMeans[i] = 0;
						
		HighCauchy[i] = atan((4.0-currMeans[i])/currScale);
		LowCauchy[i] = atan((-4.0-currMeans[i])/currScale);
	}

    //start a new walk: construction of solutions (paths)
	for (currAnt=0; currAnt<allAnts; currAnt++) {

	        int noNewPathCount=0; // counter of succesive local optimal traps
			Boolean_T allNoNewPath = True_;

			if (resetPheromone)
				continue;

			// ant tries to find a new path at maximum 'numofParams' tries
			while ((allNoNewPath) && (noNewPathCount<numOfParams)) {
				int i;
					
				for (i=0; i<numOfParams; i++) {
	
		
						int valRange;
						double CauchyVal;
						
						valRange = (int) (graphWidth[i] / 2);
						
						// sample the Caushy distribution (general approach: use the inverse of the cumulative distribution function)
						CauchyVal = CalcPPFCauchy(currScale, currMeans[i], LowCauchy[i], HighCauchy[i]);

						// rescale back the positions of the chosen vertex
						antPaths[i][currAnt] = dasa_round (CauchyVal*(double)valRange/4.0);
                        if (antPaths[i][currAnt] != 0)
							allNoNewPath = False_;

						// to get only positive positions of the verices in [1,2*valRange+1]
						antPaths[i][currAnt] = valRange + antPaths[i][currAnt];



      	      };
				
		     // if no ant didn't choose different path to currently best
			 // increment the counter of failed tries
			 if ((allNoNewPath))
        	    noNewPathCount++;
	      };
			
          // if no new paths for some number of tries, reset the search process
		  if (((allNoNewPath))&&(noNewPathCount==numOfParams)) {
				resetPheromone = 1;
		  }

		  if (resetPheromone) {
		        continue;
		  }

         // make a walk for the current ant 
		  for (j=0; j<numOfParams; j++) {
				// shift factor to access values in betweeen the predefined parameter differences (vertices)
				int shiftFactor = (rand()%(fDiscreteBase-1)) + 1;

				// check if the new calculated path is valid otherwise correct it
				while (tempBestSolution[j] + shiftFactor*graph[j][(int) (antPaths[j][currAnt])].vert>fHigh[j]) {
					if (shiftFactor>1) 
						shiftFactor--;
					else {
						shiftFactor = (rand()%(fDiscreteBase-1)) + 1;
						antPaths[j][currAnt] = antPaths[j][currAnt] - 1;
					};
				};
				while (tempBestSolution[j] + shiftFactor*graph[j][(int) (antPaths[j][currAnt])].vert<fLow[j]) {
					if (shiftFactor>1) 
						shiftFactor--;
					else {
						shiftFactor = (rand()%(fDiscreteBase-1)) + 1;
						antPaths[j][currAnt] = antPaths[j][currAnt] + 1;
					};
				};
                // callculate the shifted parameter difference 
				offset = (TValue) shiftFactor*graph[j][(int) (antPaths[j][currAnt])].vert;

				avgOffset += abss(offset);

				// generate the parameter[j] of the new solution
				currentSolutions[currAnt][j] = tempBestSolution[j] + offset;
			};


			

	};

	if (resetPheromone)
			continue;

	// evaluation of the created solutions(paths)
	for (currAnt=0; currAnt<allAnts; currAnt++) {
		 
	        // evaluate 
			if (! resetPheromone)
				scores[currAnt] = CalcLocalFunc(fFunction, numOfParams, False_, 0.0, currentSolutions[currAnt]);
			
	       // keep track of all scores
           if ( (traceFrequnecy == 1) && (printAll)) {
        			
				FloatAsStr(scores[currAnt], optimum, True_, True_, s2);
				FloatAsStr(bestScore, optimum, True_, True_, s3);
#ifdef longType
				printf("%s %d t: %6d  o: %s s: %Lf e: %s b: %s\n", fileName, lastAlgIteration,  iTime, s1, scores[numOfAnts-1], s2, s3);
#else
				printf("Rank: %s %d t: %6d  s: %f e: %s b: %s\n", fileName, lastAlgIteration,  iTime, /*s1,*/ scores[currAnt], s2, s3);
#endif
	
           };
	}
	
	// iteration finished:increment the counter of evaluations
	iTime = iTime + numOfAnts;

	currAnt = -1;
	// determine the iteration best score	
	for (j=0; j<numOfAnts; j++) {
		if (IsLower(scores[j], iterBestScore)) {
				currAnt = j;
				iterBestScore = scores[j];
		}
	};		
		
	// if new best found in whole walk then
	if (IsLower(iterBestScore, tempBestScore)) {

		// make iter best solution new temporary best solution, redistribute the phermones around it
		int j;

		for (j=0; j<numOfParams; j++) {
			if (antPaths[j][currAnt]!=(graphWidth[j] / 2)) 
				CauchyXPosition[j] = (double) antPaths[j][currAnt]-(graphWidth[j] / 2);
			else
				CauchyXPosition[j] = 0.0;
			tempBestSolution[j] = currentSolutions[currAnt][j];
		}
		tempBestSolution[numOfParams] = currentSolutions[currAnt][numOfParams];
		tempBestScore = iterBestScore;

		// make a log of it
		if ((logging) && (! printAll)) {
        		FloatAsStr(avgOffset, optimum, False_, True_, s1);
				FloatAsStr(tempBestScore, optimum, True_, True_, s2);
				FloatAsStr(bestScore, optimum, True_, True_, s3);
#ifdef longType
				printf("%s %d t: %6d  o: %s  s: %Lf  e: %s b: %s\n",fileName, lastAlgIteration, iTime, s1, tempBestScore, s2, s3);
#else
				printf("%s %d t: %6d  o: %s  s: %f  e: %s b: %s\n",fileName, lastAlgIteration, iTime, s1, tempBestScore, s2, s3);
#endif

	   };

	
	   // update scale factor-s
       // decrease CauchyScaleGlobal but no lower than current minimum
       cauchyScaleGlobal *= (double) CauchyIncPer;

	   cauchyScaleLocal = (double) cauchyScaleGlobal*0.5;

	   // check if new global best solution
       if (IsLower(tempBestScore, bestScore)) {
				int j;
        	for (j=0; j<=numOfParams; j++)
          	bestSolution[j] = tempBestSolution[j];
					
					bestScore = tempBestScore;
       };
			
	   //increase the number of found best solutions
       bestFound++;

    } else {

      cauchyScaleGlobal *= (double) CauchyDecPer;

	  // evaporation
		cauchyScaleLocal *= (double) evapValue; 	
		for (i=0; i<numOfParams; i++) {
				CauchyXPosition[i] *= evapValue;
		};
	};

	// keep track of all scores
	if (((iTime % traceFrequnecy) < numOfAnts)) {
          if (printAll) {
			avgOffset /= numOfParams;
				FloatAsStr(avgOffset, optimum, False_, True_, s1);
				FloatAsStr(scores[numOfAnts-1], optimum, True_, True_, s2);
				FloatAsStr(bestScore, optimum, True_, True_, s3);
#ifdef longType
				printf("%s %d t: %6d  o: %s s: %Lf e: %s b: %s\n", fileName, lastAlgIteration, iTime, s1, scores[numOfAnts-1], s2, s3);
#else
				printf("%s %d t: %6d  o: %s s: %f e: %s b: %s\n", fileName, lastAlgIteration, iTime, s1, scores[numOfAnts-1], s2, s3);
#endif

		  };
		  history[(iTime / traceFrequnecy)-1] = iterBestScore;
      };

	}

} 

