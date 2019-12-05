#pragma once

#include <vector>

#include "../../Source/MZLineSorter.h" // MCLineSorter::MCLineSortingOptions

struct CommandLineParam
{
  MZLineSorter::MZLineSortingOptions sortOptions;
  STLString outputFile;
  std::vector<STLString> vFiles;
  bool bVerbose;
};

extern bool ParseCommanLineParameters(CommandLineParam& param, int argc, _TCHAR* argv []);
