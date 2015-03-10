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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _CRT_SECURE_NO_WARNINGS

#include "dasa.h"

//
// This variable holds the full path to the cell configuration
// file. It is only used for solving the 'SHO balancing problem'
// The variable is declared in 'sho_eval.h'.
//
extern char cellConfFile [255];



int main(int argc, char *argv[])
{
	char paramStr1[80] = "\0", paramStr2[80] = "\0", paramStr3[80] = "\0", paramStr4[80] = "\0", temp[80] = "\0";
	int i = 1;
	
	if (argc < 2) {
		printf("DASA ver. 1.0\n");
		printf("Author: Asist. Prof. Dr. Peter Korosec\n");
		printf("Email: Peter.Korosec@ijs.si\n");
		printf("WWW: http://csd.ijs.si/korosec\n");
		printf("-----------------------------------------------------\n");
		printf("Example (0): dasa //returns this help info\n");
		printf("Example (1): dasa  -f 1 [-D x] [-noOutput] //runs the first function (1..6) with parameters in f01.params and with dimension x and we have a no output option\n");
		printf("Example (2): dasa  -f 1 [-param nameOfParamsFile] [-startSol nameOfStartSolFile] [-opt nameOfOptFile]  //this way every file can be chosen\n");
		printf("Example (3): dasa  -f 1 [-priority idle|high|real]  //you can select priority (default is normal)\n");
		printf("Example (4): dasa  -f 1 [-printAll] [-history] //if all scores are being printed on Screen and write bestScores to history\n");
		printf("Example (5): dasa  -f 1 [-maxTime x] //Maximal number of evaluations is x \n");
		printf("Example (6): dasa  -f 1 [-numOfAnts x] //Number of ants is x \n");
		printf("Example (7): dasa  -f 1 [-acceptEquals] //if score is equal than currently best it is accepted as new best\n");
		printf("-----------------------------------------------------\n");
		printf("Of course all arguments in examples can be mixed!\n");
		printf("-----------------------------------------------------\n");
        printf("-cells [file name]\n");
        printf("\tA special flag used only for the SHO balancing problem (-f 101),\n");
        printf("\tindicating the full path of the cell configuration file.-\n");
		printf ("-----------------------------------------------------\n");
        printf ("After running the algorithm, the output flags meaning is as follows:\n");
        printf ("\t- t: indicates the time passed or the number of evaluations done,\n");
        printf ("\t- o: indicates the deviation of the last move,\n");
        printf ("\t- s: indicates the achieved result,\n");
        printf ("\t- e: indicates the error (or distance) from the optimum,\n");
        printf ("\t- b: indicates the best achieved result so far.\n");
		printf ("-----------------------------------------------------\n");
		printf("Press Enter to exit!\n");
		fgetc(stdin);
		return 0;
	}


	//Initialization
	numOfParams = -1;

	InitVariables(); // load the default setting from the DASA.ini file

	logging = True_;
	printAll = False_;
	strcpy(paramStr1,"");
	strcpy(paramStr2, "");
	strcpy(paramStr3, "");
	strcpy(paramStr4, "");
	bHistory = False_;
	bAcceptEquals = False_;
	
	// if executed from a comand line with specified parameters reset
	// the default values of the global structures and control parameters

	while (i<argc) { 

		//  match argument value with the possible options
		if (strcmp(argv[i], "-noOutput") == 0)      // do not make a log of the rezult of the search
			logging = False_;
		if (strcmp(argv[i], "-printAll") == 0)      // print all intremidiate rezults 
			printAll = True_;
		if (strcmp(argv[i], "-f") == 0)             // function(problem) ID
			fFunction = atoi(argv[++i]);
		if (strcmp(argv[i], "-D") == 0)             // dimension of the problem
			numOfParams = atoi(argv[++i]);
		if (strcmp(argv[i], "-maxTime") == 0)       // maximal number of evaluations
			maxTime = atoi(argv[++i]);
		if (strcmp(argv[i], "-param") == 0)         // name of the file with data about the parameter set
			strcpy(paramStr2, argv[++i]);
		if (strcmp(argv[i], "-startSol") == 0)      // name of the file with the initial solution
			strcpy(paramStr3, argv[++i]);
		if (strcmp(argv[i], "-opt") == 0)           // name of the file that contains the optimal solution value
			strcpy(paramStr4, argv[++i]);
		if (strcmp(argv[i], "-history") == 0)       // save brief reports in the 'history' folder
			bHistory = True_;
		if (strcmp(argv[i], "-acceptEquals") == 0)  // count the equally good solution as new best solutions 
			bAcceptEquals = True_;
		if (strcmp(argv[i], "-numOfAnts") == 0)     // number of ants
			numOfAnts = atoi(argv[++i]);
#if defined(_MSC_VER) || defined(_WINDOWS_)         // if WINDOWS machines
		if (strcmp(argv[i], "-priority") == 0) {    // option to set the priority of the main process
			i++;
			if (strcmp(argv[i], "idle") == 0)
				SetPriorityClass(GetCurrentProcess(), 0x40);
			if (strcmp(argv[i], "high") == 0)
				SetPriorityClass(GetCurrentProcess(), 0x80);
			if (strcmp(argv[i], "realtime") == 0)
				SetPriorityClass(GetCurrentProcess(), 0x100);
		}
#endif
		if (strcmp(argv[i], "-cells") == 0)         // full path to the cells
			strcpy (cellConfFile, argv[++i]);       // configuration file
		i++;
	}
	
	if (numOfAnts < 1)
		numOfAnts = 1; // set to at least one ant

    // if function ID is in the specified range, genrate the name of the file with data on the parameters set
	if ((fFunction > 0) && (fFunction < 2000)) {
		if (strcmp(paramStr2,"") == 0) {
			sprintf(temp, "./params/f%.2d.params", fFunction);
			if (file_exists(temp)){                     // check if exsist this file and save the name in a temporary string 
				sprintf(paramStr2, "f%.2d", fFunction); 
			  } else {                                 //if does not exist, ask the user to specify the names of the needed files
				printf("Input params filename: \n");
				scanf("%s", paramStr2);
				printf("Input startSol filename: \n");
				scanf("%s", paramStr3);
				printf("Input opt filename: \n");
				scanf("%s", paramStr4);
			  }; 
		};
	}
	  
 
	if (strcmp(paramStr2, "") == 0) {
    printf("Missing input parameters settings!\n");
		return EXIT_FAILURE;
	}

    // finaly create 'problemName' variable and additionally 'fileName' 
    // variable used for naming the files with the reports of the searsh
	sprintf(problemName,"%s", paramStr2);
    strcpy(fileName, problemName);

    // load the data for the current problem
	if (LoadParams(paramStr2, paramStr3, paramStr4) == - 1)
		return EXIT_FAILURE;

	sprintf (fileName, "%s-%s", "DASA", problemName);

	
    // start the optimization procedure
	RunOptimization();

    // print on the screen the output of the optimization 
	FloatAsStr(bestScore, optimum, True_, True_, tempString);
#ifdef longType
			printf("Ver. 0.1 run: %d time: %6d best score: %Lf  with error: %s  best found: %d num of resets: %d\n", lastAlgIteration, iTime, bestScore, tempString, bestFound, numOfResets);
#else
			printf("Ver. 0.1 run: %d time: %6d best score: %f  with error: %s  best found: %d num of resets: %d\n", lastAlgIteration, iTime, bestScore, tempString, bestFound, numOfResets);
#endif

	//increment the counter of runs
	lastAlgIteration++;

	//  and save it in the 'INI_FILE' file 
	sprintf(tempString, "%d", lastAlgIteration);
	writeIni(INI_FILE, SECTION, "LastAlgIteration", tempString);

	
	return EXIT_SUCCESS;
}
