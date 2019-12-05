#include "stdafx.h"
#include "MZSortFiles.h"

void MZSortFiles::Sort(const std::vector<STLString>& vFiles, const STLString& outFile, MZLineSorter::MZLineSortingOptions& sortOptions, bool bVerbose)
{
  if (bVerbose)
  {
    PrintOptions(sortOptions);
  }

  if (vFiles.size() == 1)
  {
    if (outFile.empty() == false)
      SortFile(vFiles.at(0), outFile, sortOptions, bVerbose);
    else
      SortFile(vFiles.at(0), vFiles.at(0), sortOptions, bVerbose);
  }
  else
  {
      for (auto it = std::begin(vFiles); it != std::end(vFiles); ++it)
      {
        // TODO
      }
  }
}

bool MZSortFiles::SortFile(const STLString& input, const STLString& outFile, MZLineSorter::MZLineSortingOptions& sortOptions, bool bVerbose)
{
  USES_CONVERSION;
  MZLineSorter::LineSorter lineSorter;

  if (outFile.empty())
    return false;

  if (lineSorter.SortLines(input, outFile, sortOptions) != ERROR_SUCCESS)
  {

    auto message = lineSorter.ErrorMessage();
    DWORD dwErrorCode = lineSorter.ErrorCode();
    if (dwErrorCode == ERROR_FILE_EXISTS)
    {
      OutputLineFmt(_T("Sorting failed. File already exists : \"%s\" (%S)"), outFile.c_str(), message.c_str());
      return false;
    }
    else
    {
      STLString str = A2CT(lineSorter.ErrorMessage().c_str());
      OutputLineFmt(_T("Sorting failed. %s"), str.c_str());
      return false;
    }
  }
  return true;

}

void MZSortFiles::OutputLineFmt(const TCHAR* szMessageFormat, ... )
{
  _TCHAR szMessage[2048];

  va_list vaList;
  va_start(vaList, szMessageFormat);
  if( _vsntprintf_s( szMessage, _countof(szMessage), szMessageFormat, vaList ) < 0 )
  {
    va_end( vaList );
    return;
  }
  va_end( vaList );

  OutputLine(szMessage);
}

void MZSortFiles::OutputLine(const TCHAR* szMessage)
{
  _tprintf(szMessage);
  _tprintf(_T("\n"));
}

void MZSortFiles::PrintOptions(MZLineSorter::MZLineSortingOptions& sortOptions)
{
  _tprintf(_T("Options\n"));

  _tprintf(_T("\tKeep line at the top    : %d\n"), sortOptions.StartLine);
  _tprintf(_T("\tKeep line at the bottom : %d\n"), sortOptions.EndLine);
  if (sortOptions.options & MZLineSorter::Option_SkipLeadingSpaces)
    _tprintf(_T("\tSkip Leading Spaces\n"));

  if (sortOptions.options & MZLineSorter::Option_SortByEntireLine)
    _tprintf(_T("\tSortBy : Line\n"));
  if (sortOptions.options & MZLineSorter::Option_SortBySubstring)
    _tprintf(_T("\tSortBy : Substring\n"));
  if (sortOptions.options & MZLineSorter::Option_SortByLineItem)
    _tprintf(_T("\tSortBy : Line Item\n"));

  if (sortOptions.options & MZLineSorter::Option_SortAsText)
    _tprintf(_T("\tSortAs : Text\n"));
  if (sortOptions.options & MZLineSorter::Option_SortAsNum)
    _tprintf(_T("\tSortAs : Number\n"));
  if (sortOptions.options & MZLineSorter::Option_SortAsDate)
  {
    _tprintf(_T("\tSortAs : Date (Date Format : %S)\n"), sortOptions.szMatch);
  }

  if (sortOptions.options & MZLineSorter::Option_Reverse)
    _tprintf(_T("\tReverse sorting\n"));
  
  if (sortOptions.options & MZLineSorter::Option_IgnoreCase)
    _tprintf(_T("\tIgnore case\n"));
  
  if (sortOptions.options & MZLineSorter::Option_LanguageAware)
    _tprintf(_T("\tLanguage aware sorting rules\n"));
  
  if (sortOptions.options & MZLineSorter::Option_NaturalNumSorting)
    _tprintf(_T("\tNatural Number sorting\n"));
  
  if (sortOptions.options & MZLineSorter::Option_RemoveBlank)
    _tprintf(_T("\tRemove Blank\n"));


}
