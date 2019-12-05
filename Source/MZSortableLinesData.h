#pragma once

#include <vector>
#include <memory>
#include <algorithm>

#include "MZLineInfo.h"
#include "MZLineCompare.h"
#include "MZLineSorter.h" // options
#include "MZParseTime.h"

#include "../../MZDataReader/Source/MZLinesData.h"
#include "MZSortingLinePreprocessor.h"

namespace MZLineSorter
{

  class SortableLinesData : public MZDR::LinesData<LineInfo>
  {
  public:

    template<typename T>
    void Preprocessor(const LinePreprocessor<T>& proc)
    {
      for (auto&& item : m_vItems)
      {
        proc.Prepare(item);
      }
    }

    template<typename T>
    void PostProcessor(const LinePostProcessor<T>& proc)
    {
      for (auto&& item : m_vItems)
      {
        proc.Process(item);
      }
    }


    template<typename T>
    void Sort(const MZLineSortingOptions& options)
    {
      const bool bForceXPFallback = (options.options & MZLineSorter::Option_ForceXPFallback) > 0;
      const bool bReverse = (options.options & MZLineSorter::Option_Reverse) > 0;
      const bool bNatrualNum = (options.options & MZLineSorter::Option_NaturalNumSorting) > 0;
      const bool bIgnoreCase = (options.options & MZLineSorter::Option_IgnoreCase) > 0;
      
      if (options.options & MZLineSorter::Option_SortAsNum)
      {
        // Sort as NUM
          std::stable_sort(begin(m_vItems) + options.StartLine, end(m_vItems) - options.EndLine, [](const LineInfo& item1, const LineInfo& item2) noexcept { return item1.nValue < item2.nValue; });
      }
      else if (options.options & MZLineSorter::Option_SortAsDate)
      {
        // Sort as Date
          std::stable_sort(begin(m_vItems) + options.StartLine, end(m_vItems) - options.EndLine, [](const LineInfo& item1, const LineInfo& item2) noexcept { return item1.nTime < item2.nTime; });
      }
      else if (options.options & MZLineSorter::Option_LanguageAware)
      {
        if (TypeIsWChar<T>::value) // WCHAR
        {
          LineCompare_LanguageAware_StringCompareEx<T> cmp(bNatrualNum, bForceXPFallback);
          std::stable_sort(begin(m_vItems) + options.StartLine, end(m_vItems) - options.EndLine, [&cmp](const LineInfo& item1, const LineInfo& item2) { return cmp.CompareItem(item1, item2); });
        }
        else if (TypeIsChar<T>::value) // CHAR
        {
          if (bNatrualNum) // For numsorting we must fallback to WCHAR for char. 
          {
            LineCompare_LanguageAware_WFallback cmp(bNatrualNum);
            std::stable_sort(begin(m_vItems) + options.StartLine, end(m_vItems) - options.EndLine, [&cmp](const LineInfo& item1, const LineInfo& item2) { return cmp.CompareItem(item1, item2); });
          }
          else
          {
            LineCompare_LanguageAware_StringCompareA cmp;
            std::stable_sort(begin(m_vItems) + options.StartLine, end(m_vItems) - options.EndLine, [&cmp](const LineInfo& item1, const LineInfo& item2) { return cmp.CompareItem(item1, item2); });
          }
        }
      }
      else
      {
        // NUMSORT NOT SUPPORTED for NoneLanguageAware sort... YET

        // Sort as Simple
        if (TypeIsChar< T >::value) // CHAR
        {
          LineCompare_OrdinalA cmp(bIgnoreCase);
          std::stable_sort(begin(m_vItems) + options.StartLine, end(m_vItems) - options.EndLine, [&cmp](const LineInfo& item1, const LineInfo& item2) { return cmp.CompareItem(item1, item2); });
        }
        else if (TypeIsWChar<T>::value) // WCHAR
        {
          LineCompare_OrdinalW cmp(bIgnoreCase);
          std::stable_sort(begin(m_vItems) + options.StartLine, end(m_vItems) - options.EndLine, [&cmp](const LineInfo& item1, const LineInfo& item2) { return cmp.CompareItem(item1, item2); });
        }
      }

      // Reverse result
      if (bReverse)
      {
        std::reverse(begin(m_vItems) + options.StartLine, end(m_vItems) - options.EndLine);
      }
    }

    void RemoveEmptyLines(const MZLineSortingOptions& options)
    {
//        DWORD nRemoved = 0;

        m_vItems.erase(std::remove_if(begin(m_vItems) + options.StartLine,
                                      end(m_vItems) - options.EndLine, 
                                      [](const LineInfo& lineInfo) 
        {
          if (lineInfo.lenght == 0)
            return true;
          return false;
        }), end(m_vItems) - options.EndLine);

    }

  };

}
