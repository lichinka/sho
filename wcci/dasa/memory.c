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

#define MAX_ACCUARCY 201

#include "dasa.h"

//
// Variables and functions that are specific to the
// "SHO balancing problem: CPICH optimization" for WCCI 2012
//
#include "sho_eval.h"


 
/* Code to allocate memory to global variables being used in evaluation of functions */
void allocate_memory ()
{ 
	int i;
	int accuWidth = 201;
	if (numOfParams>accuWidth)
		accuWidth = numOfParams+1;

	graph =  malloc((numOfParams) * sizeof(* graph));
	for (i=0; i<numOfParams; i++)
  {
     graph[i] =  calloc(accuWidth, sizeof(* graph[i]));
  }
	
	currentSolutions =  malloc((numOfAnts) * sizeof(* currentSolutions));
	for (i=0; i<numOfAnts; i++)
  {
     currentSolutions[i] =  calloc(numOfParams+1, sizeof(* currentSolutions[i]));
  }

	accuWidth = numOfParams;

	antPaths = /*(int **)*/ malloc(numOfParams*sizeof(*antPaths));
	for (i=0; i<numOfParams; i++)
  {
     antPaths[i] = /*(int *)*/ calloc(numOfAnts, sizeof(* antPaths[i]));
  }


	fLow = (TValue *)malloc(numOfParams*sizeof(TValue));
	fHigh = (TValue *)malloc(numOfParams*sizeof(TValue));
	fStep = (TValue *)malloc(numOfParams*sizeof(TValue));
	fStart = (TValue *)malloc(numOfParams*sizeof(TValue));
	bestSolution = (TValue *)malloc((numOfParams+1)*sizeof(TValue));
	currentSolution = (TValue *)malloc((numOfParams+1)*sizeof(TValue));
	graphWidth = (int *)malloc(numOfParams*sizeof(int));
	CauchyXPosition = (TValue *)malloc(numOfParams*sizeof(TValue));
	iterBestSolution = (TValue *)malloc((numOfParams+1)*sizeof(TValue));
	tempBestSolution = (TValue *)malloc((numOfParams+1)*sizeof(TValue));
	scores = (TValue *)malloc((numOfAnts)*sizeof(TValue));
	history = (TValue *)malloc(((maxTime / traceFrequnecy) +100)*sizeof(TValue));
	currMeans = (TValue *)malloc(numOfParams*sizeof(TValue));
	LowCauchy = (TValue *)malloc(numOfParams*sizeof(TValue));
	HighCauchy = (TValue *)malloc(numOfParams*sizeof(TValue));
	
}


void free_memory()
{
  int i;
	for (i=0; i<numOfParams; i++)
	{
		free(graph[i]);
		free(antPaths[i]);
	}

	free (antPaths);
	free (graph);
	for (i=0; i<numOfAnts; i++)
	{
		free(currentSolutions[i]);
	}
	free (currentSolutions);

	free (scores);
	
	free (fLow);
	free (fHigh);
	free (fStep);
	free (fStart);
	free (bestSolution);
	free (currentSolution);
	free (graphWidth);
	free (CauchyXPosition);
	free (iterBestSolution);
	free (tempBestSolution);
	free (history);
	free (currMeans);
	free (LowCauchy);
	free (HighCauchy);

    //
    // turn the Python environment off, if used
    //
    if (Py_IsInitialized ( ))
    {
        printf ("Free 'pEvaluation'\n");
        // free memory taken by the 'Evaluation' object
        // FIXME SEGFAULT
        //Py_DECREF (pEvaluation);
        // free memory taken by the loaded module
        printf ("Free 'pModule'\n");
        // FIXME SEGFAULT
        //Py_DECREF (pModule);
        // close the interpreter
        printf ("Shutting the Python interpreter down ...\n");
        Py_Finalize ( );
    }
}
