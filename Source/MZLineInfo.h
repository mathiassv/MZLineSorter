#pragma once

#include <time.h>

#include "../../MZDataReader/Source/MZLinesData.h"

namespace MZLineSorter
{
  struct LineInfo
  {
    LineInfo(const BYTE* p, DWORD l, MZDR::NewLine newLineChars, BYTE numBytesForNewLine)
      : pLine(p)
      , lenght(l)
      , nBytesForNewLine(numBytesForNewLine)
      , newLineChar(newLineChars)
      , nValue(0)
    {
      
    }

    // usage :  std::string text = item.GetLine<std::string, char>();
    template<typename R, typename C>
    R GetLine() const
    {
      C szTmp[2048];
      auto len = min(lenght / sizeof(C), _countof(szTmp));
      strncpy_s(szTmp, _countof(szTmp), reinterpret_cast<const C*>(pLine),len);
      szTmp[len] = '\0';
      return szTmp;
    }

    DWORD GetLineDataLength() const
    {
      return lenght + nBytesForNewLine;
    }
    const BYTE* GetLineData() const
    {
      return pLine;
    }

    union 
    {
      const BYTE* pLine;     // line start
      // debug
      const char* pTextA;
      const wchar_t* pTextW;
    };
    DWORD lenght;          // bytes
    BYTE nBytesForNewLine; // num of bytes for the NewLine chars. (that exists in the bufer) used when saving.
    MZDR::NewLine newLineChar;
    // Offset and length in bytes for the comparer
    DWORD nStartOffset;
    DWORD nCompareLen;
    union
    {
      _int64     nValue;
      __time64_t nTime;
      double     dValue;
    };
  };
}
