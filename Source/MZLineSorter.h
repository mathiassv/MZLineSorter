
// Frontend for MZLinesSort
#pragma once
#include <atlconv.h>
#include <vector>

namespace MZLineSorter
{

  typedef DWORD SortOption;
  const SortOption Option_SortByEntireLine       = 0x00000001L;
  const SortOption Option_SortBySubstring        = 0x00000002L;
  const SortOption Option_SortByLineItem         = 0x00000004L;

  const SortOption Option_SortAsText             = 0x00000010L;
  const SortOption Option_SortAsNum              = 0x00000100L;
  const SortOption Option_SortAsDate             = 0x00000200L;

  const SortOption Option_Reverse                = 0x00001000L; // ( sort Z-A, 9-0), default is ascending 
  const SortOption Option_IgnoreCase             = 0x00002000L; 
  const SortOption Option_LanguageAware          = 0000004000L; // Use locale aware sorting (IgnoreCase is not used. locale aware always ignore cases)
  const SortOption Option_NaturalNumSorting      = 0x00008000L;

  const SortOption Option_RemoveDuplicates       = 0x00020000L;
  const SortOption Option_RemoveBlank            = 0x00040000L;
  
  const SortOption Option_ForceXPFallback        = 0x00080000L; // Used by UnitTest to text XP code path

  const SortOption Option_SkipLeadingSpaces      = 0x00100000L;
  const SortOption Option_SubStr_EndAtWhitespace = 0x00200000L;
  const SortOption Option_SubStr_EndAtChar       = 0x00400000L;

  const SortOption Option_Save_NewName           = 0x01000000L;
  const SortOption Option_Save_KeepOriginalAsBak = 0x02000000L;
  const SortOption Option_Save_DeleteOriginal    = 0x04000000L;
  const SortOption Option_Save_Overwrite         = 0x08000000L; // overwrite NewName if file already exists

  const SortOption Option_Silent                 = 0x10000000L; // Silent - No Dialogs


  class MZLineSortingOptions
  {
  public:
    DWORD StartLine;
    DWORD EndLine;

    DWORD LineOffset;   // 0-Based
    DWORD SubStringLen; // Substring to match has fixed len
    
    wchar_t charMatchW; // ItemSeparator (LineItemMatch) or EndChar (Substring match)
    char    charMatchA;

    char    szMatch[128];

    SortOption options;
    
    DWORD LineItem;

    char    GetMatchChar(char) const    { return charMatchA; }
    wchar_t GetMatchChar(wchar_t) const { return charMatchW; }

    void SetMatchChar(char c)
    {
      USES_CONVERSION;
      char tmp[8] = { 0 };
      tmp[0] = c;
      charMatchA = c;
      charMatchW = A2CW(tmp)[0];
    }

    void SetMatchChar(wchar_t c)
    {
      USES_CONVERSION;
      wchar_t tmp[8] = { 0 };
      tmp[0] = c;
      charMatchA = W2CA(tmp)[0];
      charMatchW = c;
    }

    void clear()
    {
      ZeroMemory(this, sizeof(MZLineSortingOptions));
    }

    DWORD GetSortAs()
    {
      if (options & MZLineSorter::Option_SortAsText)
        return MZLineSorter::Option_SortAsText;
      else if (options & MZLineSorter::Option_SortAsDate)
        return MZLineSorter::Option_SortAsDate;
      else if (options & MZLineSorter::Option_SortAsNum)
        return MZLineSorter::Option_SortAsNum;

      return MZLineSorter::Option_SortAsText;
    }

    DWORD GetSortBy()
    {
      if (options & MZLineSorter::Option_SortByEntireLine)
        return MZLineSorter::Option_SortByEntireLine;
      else if (options & MZLineSorter::Option_SortBySubstring)
        return MZLineSorter::Option_SortBySubstring;
      else if (options & MZLineSorter::Option_SortByLineItem)
        return MZLineSorter::Option_SortByLineItem;

      return MZLineSorter::Option_SortByEntireLine;
    }
    DWORD GetSortOrder()
    {
      if (options & MZLineSorter::Option_Reverse)
        return MZLineSorter::Option_Reverse;

      return 0;
    }
    DWORD GetSaveAs()
    {
      if (options & MZLineSorter::Option_Save_NewName)
        return MZLineSorter::Option_Save_NewName;
      else if (options & MZLineSorter::Option_Save_KeepOriginalAsBak)
        return MZLineSorter::Option_Save_KeepOriginalAsBak;
      else if (options & MZLineSorter::Option_Save_DeleteOriginal)
        return MZLineSorter::Option_Save_DeleteOriginal;

      return MZLineSorter::Option_Save_NewName;
    }
  };

  class LineSorter
  {
  public:
    DWORD SortLines(const STLString& filename, const STLString& targetFilename, const MZLineSortingOptions& options);

    // Used from UI to show sample data. Uses this so we use the same line parser
    // however output string are ALWAYS WSTRING since the UI need thats
    std::vector<std::wstring> GetSampleLines(const STLString& filename, DWORD maxlines = 50);
    std::wstring GetSampleText(const STLString& filename, DWORD maxlines = 50);

    // Will modify buffer
    DWORD SortLinesInBuffert(char* szText, DWORD dwLen, const MZLineSortingOptions& options);
    DWORD SortLinesInBuffert(wchar_t* szText, DWORD dwLen, const MZLineSortingOptions& options);

    std::string ErrorMessage() { return m_errorMessage; }
    DWORD ErrorCode() { return m_errorCode; }
  protected:
    
    std::string m_errorMessage;
    DWORD m_errorCode = 0;
  };

  class LineWriter
  {

  };

}

