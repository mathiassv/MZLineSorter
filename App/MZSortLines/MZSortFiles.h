#pragma once

#include <vector>

#include "../../Source/MZLineSorter.h"

class MZSortFiles
{
public:

  void Sort(const std::vector<STLString>& vFiles, const STLString& outFile, MZLineSorter::MZLineSortingOptions& sortOptions, bool bVerbose);
  bool SortFile(const STLString& input, const STLString& outFile, MZLineSorter::MZLineSortingOptions& sortOptions, bool bVerbose);
private:
  void OutputLine(const TCHAR* szMessage);
  void OutputLineFmt(const TCHAR* szMessage, ...);
  void PrintOptions(MZLineSorter::MZLineSortingOptions& sortOptions);
};
