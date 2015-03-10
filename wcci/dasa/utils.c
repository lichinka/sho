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

#define _CRT_SECURE_NO_WARNINGS

#include "dasa.h"
#include <math.h>

FILE *fr;            /* declare the file pointer */


#define STRING_LENGTH 80
#define MAX_PROCESSES 1000
#define DEFAULT_DELAY_BETWEEN_CHECKS 10
#define NANOSECONDS 1E9
#define MICROSECONDS 1E6
#define JIFFIES 100

#define STDOUT 1
#define STDERR 2

typedef char String[STRING_LENGTH];

int GlobalInterrupted;
int GlobalSignalReceived;


/**
 * These two functions are not used.
 * They include function calls that are Borland C++ dependant.
 *
Boolean_T UnicodeToAnsi(
	LPWSTR pszwUniString, 
	LPSTR  pszAnsiBuff,
	DWORD  dwAnsiBuffSize
	)
{
	int iRet = 0;
    iRet = WideCharToMultiByte(
		CP_ACP,
		0,
		pszwUniString,
		-1,
		pszAnsiBuff,
		dwAnsiBuffSize,
		NULL,
		NULL
		);
	if (0 != iRet)
        return True_;
    else
        return False_;
}


Boolean_T AnsiToUnicode(
    LPSTR  pszAnsiString, 
    LPWSTR pszwUniBuff, 
    DWORD dwUniBuffSize
    )
{

	int iRet = 0;
    iRet = MultiByteToWideChar(
		CP_ACP,
		0,
		pszAnsiString,
		-1,
		pszwUniBuff,
		dwUniBuffSize
		);
	if (0 != iRet)
        return True_;
    else
        return False_;
}
*
*/


Boolean_T file_exists(const char * filename)
{
  FILE * file;  
	if (NULL != (file = fopen(filename, "r")))
    {
        fclose(file);
        return True_;
    }
    return False_;
};


char * ParseLine1(char * line, char * val, char c)
{
  int len = 0;
  line = StripLeadingSpaces(line);
  	if (line) {
		strcpy(val, line);
		while (*line)
		{
			if ((c == *line))
			{
				val[len] = 0;
				line = StripLeadingSpaces(line);
				return line;
			} else  {
					line++;
					len++;
			}
		}
	}
	val[len] = 0;
	return line;
};

Boolean_T IsLower(const TValue aScore, const TValue aBestScore)
{
  if (bAcceptEquals)
    return !(aScore>aBestScore);
  else
    return (aScore<aBestScore);
};

double CalcCauchy(const double Scale, const double Mean, const double x)
{
	double res = 1.0/(Scale*M_PI*(1.0+sqrv((x-Mean)/Scale)));
	
	return res;
};

double CalcPPFCauchy(const double Scale, const double Mean, const double Low, const double High)
{

  double rd = random01*(High-Low)+Low;
  return Mean + (tan((rd)))*Scale;

};


char * FloatAsStr(const TValue aFloat, const TValue optimum, const Boolean_T aErr, const Boolean_T aSci, char * res)
{
  TValue val;

  if (aErr) {
    val = aFloat-optimum;
  } else {
    val = aFloat;
  }
#ifdef longType
  if (aSci) {
    sprintf(res, "%Le", val);
  } else {
    sprintf(res, "%Lf", val); //FormatFloat(floatFormatAccuracy, val);
  }
#else
  if (aSci) {
    sprintf(res, "%e", val);
  } else {
    sprintf(res, "%f", val); //FormatFloat(floatFormatAccuracy, val);
  }

#endif
  return res;
}

int writeIni(char * fileName, char * Section, char * Name, char * Line)
{
  return UpdateCfg(fileName, Section, Name, Line);
}

int readIni(char * fileName, char * Section, char * Name, enum CfgTypes CfgString, void * Line)
{
  
      struct      CfgStruct this_var;

      this_var.Name    = Name;
      this_var.DataPtr = Line;
      this_var.VarType = CfgString;
      return ReadCfg(fileName, Section, &this_var);
         

} 

#include <locale.h>

char *commaprint(unsigned long n)
{
  static int comma = '\0';
  static char retbuf[30];
  char *p = &retbuf[sizeof(retbuf)-1];
  int i = 0;

  if(comma == '\0') {
    struct lconv *lcp = localeconv();
    if(lcp != NULL) {
      if(lcp->thousands_sep != NULL && *lcp->thousands_sep != '\0')
      	comma = *lcp->thousands_sep;
      else comma = ',';
    }
  }

  *p = '\0';

  do {
    if(i%3 == 0 && i != 0)
      *--p = comma;
    *--p = '0' + n % 10;
    n /= 10;
    i++;
  } while(n != 0);

  return p;
}

#ifndef __GETTIMEOFDAY_C
#define __GETTIMEOFDAY_C

#if defined(_MSC_VER) || defined(_WINDOWS_)
   #include <time.h>
   #if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
         struct timeval 
         {
            long tv_sec;
            long tv_usec;
         };
   #endif 
#else
   #include <sys/time.h>
#endif 
#if defined(_MSC_VER) || defined(_WINDOWS_)
   int gettimeofday(struct timeval* tv, struct timezone *tz) 
   {
      union {
         long long ns100;
         FILETIME ft;
      } now;
     
      GetSystemTimeAsFileTime (&now.ft);
      tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
      tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);
     return (0);
   }
#endif
#endif /* __GETTIMEOFDAY_C */



long long mtime(void)
{
    struct timeval tv;
		gettimeofday(&tv,NULL);
    return (long long)(tv.tv_sec*1000 + (tv.tv_usec / 1000));
}

long long utime(void)
{
        struct timeval tv;
        gettimeofday(&tv,NULL);
        return (long long)(tv.tv_sec*1000000 + tv.tv_usec);
}

int duTime(const int udelay)
{
        int64_t startTime, endTime;
        startTime = utime();
        endTime = utime();
        while ((endTime - startTime) < udelay)
                endTime = utime();
        return 1;
}

