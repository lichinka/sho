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
 
#ifndef _utils_h_included_
#define _utils_h_included_

#include "ini.h"
#include "types.h"

/**
 * These two functions are not used.
 * They include function calls that are Borland C++ dependant.
 *
Boolean_T AnsiToUnicode (LPSTR  pszAnsiString, LPWSTR pszwUniBuff, DWORD dwUniBuffSize);
Boolean_T UnicodeToAnsi (LPWSTR pszwUniString, LPSTR  pszAnsiBuff, DWORD dwAnsiBuffSize);
 *
 */

Boolean_T file_exists(const char * filename);
char * ParseLine1(char * line, char * val, char c);

Boolean_T IsLower(const TValue aScore, const TValue aBestScore);

double CalcCauchy(const double Scale, const double Mean,  const double x);
double CalcPPFCauchy(const double Scale, const double Mean, const double Low, const double High);

char * FloatAsStr(const TValue aFloat, const TValue optimum, const Boolean_T aErr, const Boolean_T aSci, char * res);

int writeIni(char * fileName, char * Section, char * Name, char * Line);
int readIni(char * fileName, char * Section, char * Name, enum CfgTypes CfgString, void * Line);

int getCurrThread();

char *commaprint(unsigned long n);
long long utime();
long long mtime();
int duTime(const int udelay);

#endif /* _utils_h_included_ */
