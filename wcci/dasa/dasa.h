/***************************************************************************
 *   Copyright (C) 2008 by Peter Korošec								   *
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

#ifndef _dasa_h_included_
#define _dasa_h_included_

#define PROBLEM_SIZE 501

#ifndef dasa_round
    #define dasa_round(v) ((v < 0) ? (-(floor( -v + 0.5))) : (floor( v + 0.5)))
#endif

#define random01   ((double)rand() / ((double)RAND_MAX + 1))//32767.0) /* uniformly random in [0, 1) */
#define abss(x)    ((x) < 0 ? (-x) : (x))
#define sqrv(x) ((x)*(x))
#define M_PI            3.14159265358979323846

#define INI_FILE "DASA.ini"
#define SECTION "Initialization"

#include <math.h>
#include "sniptype.h"
#include "utils.h"



// values of the parameters of te optimal solutions for the 
// example function problems, defined in funcOptSol.c
extern const TValue sphereShift[];
extern const TValue schwefelShift[];
extern const TValue rosenbrockShift[];
extern const TValue rastriginShift[];
extern const TValue griewankShift[];
extern const TValue ackleyShift[];
extern const TValue f_bias[];

// global variables declared in dasa.c
extern TConnection **graph;
extern TEVector fLow, fHigh, fStep, fStart;
extern TEVector bestSolution, currentSolution;
extern TVector graphWidth;
extern TEVector CauchyXPosition;
extern TEVector tempBestSolution, iterBestSolution;
extern TMatrix antPaths;
extern TEMatrix currentSolutions;
extern TEVector scores;
extern TEVector currMeans, LowCauchy, HighCauchy;

extern int numOfParams;
extern int numOfAnts;
extern Boolean_T printAll;
extern Boolean_T logging;
extern Boolean_T bHistory;
extern Boolean_T bStartSol;
extern Boolean_T bAcceptEquals;

extern TValue bestScore;
extern TValue optimum;
extern int fFunction;
extern int lastAlgIteration;
extern int bestFound;
extern int maxTime;
extern int iTime;
extern int traceFrequnecy;
extern int numOfResets;

extern char fileName[256];
extern char problemName[256];
extern char tempString[80];

extern TValue *history;

/*
Declaration of the core procedures and functions used for imlementation of DASA
*/

//Initializatiion of algorithm parameters and loading problem-definition paramteres
void InitVariables();
int LoadParams(const char * nameOfParams, const char * nameOfStartSol, const char * nameOfOptimum);

// Generation of random solution and evaluation of solution (calculation of objective function)
TValue GetRandomSolution(TEVector solution);
TValue CalcLocalFunc(const int aFunction, int aDimension, const Boolean_T aLog, const double aSeed, TEVector aParams);

// Optimization: ant-based search 
void RunOptimization();
Boolean_T CreateGraph();
void AntsWork();
void AntGroup(const int aGroup, const int numOfGroups, const int allAnts);

// Logs and reports 
void AddHistory(char * aFileName, const TValue aScore, const int aNumOfIter, const int aNumOfParam, const int aLastAlgIteration, const int aRandSeed);
void SaveScores(const char * afileName, const int aLastAlgIteration);
void SaveStats(const char * aFileName, const int aLastAlgIteration, const int aRandSeed);



#endif /* _dasa_h_included_ */
