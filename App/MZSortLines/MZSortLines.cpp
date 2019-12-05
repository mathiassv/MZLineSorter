/*
  MCSortLines 

  -SortAs:[Text]|Date|Num
  -SortBy:[EntireLine]|SubString|Item
  -Reverse
  -IgnoreCase
  -LanguageSort
  -NatrualNum
  -RemoveBlank
  -SkipLeadingSpace
  -EndAtChar:D
  -EndAtWhitespace
  -Silent
  -Overwrite
  -SaveReg    - Save Settings to Registry 
  -LoadReg    - Load Settings from Registry
  -stdout

*/

#include "stdafx.h"
#include <memory>
#include "MZSortFiles.h"
#include "ParseParameters.h"


int _tmain(int argc, _TCHAR* argv[])
{
  
  CommandLineParam param;
  if (!ParseCommanLineParameters(param, argc, argv))
  {
    // fail
    return 1;
  }

  if (param.vFiles.size() == 0)
  {
    _tprintf(_T("No files to sort\n"));
    return 1;
  }

  MZSortFiles sortFiles;
  sortFiles.Sort(param.vFiles, param.outputFile, param.sortOptions, param.bVerbose);

  return 0;
}

