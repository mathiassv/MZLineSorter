#pragma once

#include <vector>
#include <memory>
#include <algorithm>

#include "MZLineInfo.h"
#include "MZLineCompare.h"
#include "MZLineSorter.h" // options
#include "MZParseTime.h"

namespace MZLineSorter
{
  template<typename T>
  class LinePreprocessor
  {
  public:
    LinePreprocessor(const MZLineSortingOptions& options)
      : m_options(options)
    {

    }

    void Prepare(LineInfo& line) const
    {
      const T* pLine = reinterpret_cast<const T*>(line.pLine);
      const T* pLineEnd = reinterpret_cast<const T*>(line.pLine+line.lenght);
//      size_t charLen = line.lenght / sizeof(T);

      const T* pLineOffset = pLine;
      if (m_options.LineOffset > 0)
        pLineOffset = pLine + m_options.LineOffset;

      if (m_options.options & Option_SkipLeadingSpaces)
        pLineOffset = SkipWhitespace(pLineOffset, pLineEnd);

      line.nStartOffset = static_cast<DWORD>(pLineOffset - pLine);

      if (m_options.options & Option_SortByEntireLine)
      {
        line.nCompareLen = 0;
        return;
      }
      else if (m_options.options & Option_SortBySubstring)
      {
        if (m_options.SubStringLen > 0)
        {
          if (pLineOffset + m_options.SubStringLen < pLineEnd)
            line.nCompareLen = m_options.SubStringLen;
        }
        else if (m_options.options & Option_SubStr_EndAtWhitespace)
        {
          const T* subEnd = FindEndOfWord(pLineOffset, pLineEnd);
          if (subEnd)
            line.nCompareLen = static_cast<DWORD>(subEnd - pLineOffset); 
        }
        else if (m_options.options & Option_SubStr_EndAtChar)
        {
          // not happy with this template solution. But need someway to get the correct char based on what type this class is in
          T t = 0;
          const T* subEnd = FindCharPos(pLineOffset, pLineEnd, m_options.GetMatchChar(t));
          if (subEnd)
            line.nCompareLen = static_cast<DWORD>(subEnd - pLineOffset); 
        }
      }
      else if (m_options.options & Option_SortByLineItem)
      {
        T t = 0;
        const T* startItem = FindLineItem(pLineOffset, pLineEnd, m_options.LineItem, m_options.GetMatchChar(t));
        if (startItem)
        {
          line.nStartOffset = static_cast<DWORD>(startItem - pLine);
          const T* subEnd = FindCharPos(startItem, pLineEnd, m_options.GetMatchChar(t));
          if (subEnd)
            line.nCompareLen = static_cast<DWORD>(subEnd - startItem); 
        }
      }

      // Sort As NUM
      if (m_options.options & Option_SortAsNum)
      {
        const T* pPos = pLine + line.nStartOffset;
        DWORD nLen = min(static_cast<DWORD>(pLineEnd - pPos), m_options.SubStringLen);
        line.nValue = ParseNum(pPos, nLen);
      }
      else if (m_options.options & Option_SortAsDate)
      {
        // makesure there is a date match
        if (m_options.szMatch[0] != '\0')
        {
          tm t;
          ZeroMemory(&t, sizeof(tm));
          strptime(pLine + line.nStartOffset, m_options.szMatch, &t);
          if (t.tm_year > 1900)
            t.tm_year -= 1900;

          line.nTime = _mktime64(&t);
        }
      }

    }
    protected:
    _int64 ParseNum(const T* szPos, DWORD nLen) const
    {
      if (nLen == 0)
      {
        T *pend;
        return toi64(szPos, &pend);
      }

      T tmp[64];
      tcpy_s(tmp,_countof(tmp), szPos, nLen);
      tmp[nLen] = '\0';
      T *pend;
      return toi64(tmp, &pend);
    }

    void tcpy_s(char* s, size_t count, const char* pos, DWORD nLen) const
    {
      strncpy_s(s, count, pos, nLen);
    }

    void tcpy_s(wchar_t* s, size_t count, const wchar_t* pos, DWORD nLen) const
    {
      wcsncpy_s(s, count, pos, nLen);
    }

    __int64 toi64(const char* pos, char** pend) const
    {
      return _strtoi64(pos, pend, 10);
    }

    __int64 toi64(const wchar_t* pos, wchar_t** pend) const
    {
      return _wcstoi64(pos, pend, 10);
    }

    const T* SkipWhitespace(const T* pos, const T* pend) const
    {
      while (pos < pend)
      {
        if (*pos == ' ')
          ++pos;
        else if (*pos == '\t')
          ++pos;
        else
          return pos;
      }
      return pos;
    }

    const T* FindEndOfWord(const T* pos, const T* pend) const
    {
      while (pos < pend)
      {
         if (*pos == ' ')
           return pos;
         else if (*pos == '\t')
           return pos;
         else
           ++pos;
      }
      return pos;
    }

    const T* FindCharPos(const T* pos, const T* pend, const T ch) const
    {
      while (pos < pend)
      {
         if (*pos == ch)
           return pos;
         else
           ++pos;
      }
      return pos;
    }

    // 1 based.
    const T* FindLineItem(const T* pos, const T* pend, DWORD nItem, const T ch) const
    {
      DWORD nCurItems = 1;
      while (pos < pend)
      {
        // found item.
        if (nCurItems == nItem)
          return pos;

        if (*pos == ch)
          ++nCurItems;

        ++pos;
      }
      return pos;
    }

  protected:
    const MZLineSortingOptions& m_options;
    
  };

  template<typename T>
  class LinePostProcessor
  {
  protected:
    MZLineSortingOptions m_options;
  public:
    LinePostProcessor(const MZLineSortingOptions& options)
      : m_options(options)
    {

    }
    void Process(LineInfo& line) const
    {

    }

  };
}
