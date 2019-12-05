#pragma once

#include <algorithm>
#include <locale>
#include <memory>
#include "MZLineInfo.h"
#include "MZLineSorter.h"
#include <winnls.h>

typedef INT (WINAPI *pCompareStringEx)(LPCWSTR, DWORD, LPCWSTR, INT, LPCWSTR, INT, LPNLSVERSIONINFO, LPVOID, LPARAM);
typedef int(WINAPI* pCompareStringOrdinal)(LPCWCH lpString1, int cchCount1, LPCWCH lpString2, int cchCount2, BOOL bIgnoreCase);


#ifndef SORT_DIGITSASNUMBERS
  #define SORT_DIGITSASNUMBERS      0x00000008  // use digits as numbers sort method
#endif

#ifndef LOCALE_NAME_USER_DEFAULT
#define LOCALE_NAME_USER_DEFAULT            NULL
#endif

#define MINALLOCSIZE 128

namespace MZLineSorter
{
  class LineCompareLineCache
  {
  protected:
    size_t m_tmpLHSLen = 0;
    size_t m_tmpRHSLen = 0;

    // Something weird happens if I use unique_ptr
    std::unique_ptr<wchar_t []> m_szTmpLHS;
    std::unique_ptr<wchar_t []> m_szTmpRHS;

  public:
    ~LineCompareLineCache()
    {
    }

    const wchar_t* GetWideLHS(const char* strA, DWORD len)
    {
      if (len > m_tmpLHSLen)
      {
        m_tmpLHSLen = max(len + 2, MINALLOCSIZE);
        m_szTmpLHS = std::make_unique<wchar_t []>(m_tmpLHSLen);
      }

      *(m_szTmpLHS.get()) = '\0';
      MultiByteToWideChar(CP_ACP, 0, strA, -1, m_szTmpLHS.get(), len);
      return m_szTmpLHS.get();
    }

    const wchar_t* GetWideRHS(const char* strA, DWORD len)
    {
      if (len > m_tmpRHSLen)
      {
        m_tmpRHSLen = max(len + 2, MINALLOCSIZE);
        m_szTmpRHS = std::make_unique<wchar_t []>(m_tmpRHSLen);
      }

      *(m_szTmpRHS.get()) = '\0';
      MultiByteToWideChar(CP_ACP, 0, strA, -1, m_szTmpRHS.get(), len);
      return m_szTmpRHS.get();
    }
  };

  template< class T >
  struct TypeIsChar {  static const bool value = false; };
  template<>
  struct TypeIsChar < char > { static const bool value = true;  };

  template< class T >
  struct TypeIsWChar {  static const bool value = false; };
  template<>
  struct TypeIsWChar < wchar_t> { static const bool value = true;  };


  template<class T>
  class LineCompare_TextBase
  {
  protected:
  public:
    LineCompare_TextBase()
    {

    }
     bool CompareItem( const LineInfo& item1, const LineInfo& item2 ) 
     { 
      const T* lhs = reinterpret_cast<const T*>(item1.pLine + item1.nStartOffset);
      const T* rhs = reinterpret_cast<const T*>(item2.pLine + item2.nStartOffset);

      DWORD lhsBytesLen = item1.lenght - item1.nStartOffset;
      DWORD rhsBytesLen = item2.lenght - item2.nStartOffset;
      if (item1.nCompareLen)
        lhsBytesLen = item1.nCompareLen;
      if (item2.nCompareLen)
        rhsBytesLen = item2.nCompareLen;

      return Compare(lhs, lhsBytesLen / sizeof(T), rhs, rhsBytesLen / sizeof(T));
     }
  protected:
    virtual bool Compare(const T* /*lhs*/, DWORD /*lhsLen*/, const T* /*rhs*/, DWORD /*rhsLen*/) { return false; }
  };

  class LineCompare_OrdinalA : public LineCompare_TextBase<char>
  {
  protected:
    bool m_bIgnoreCase = false;
  public:
    LineCompare_OrdinalA(bool bIgnoreCase)
      : m_bIgnoreCase(bIgnoreCase)
    {

    }
    bool Compare(const char* lhs, DWORD lhsLen, const char* rhs, DWORD rhsLen) override
    { 
     if (lhsLen == 0 && rhsLen == 0)
        return false;

      if ((lhsLen == 0 && rhsLen != 0) || (lhsLen != 0 && rhsLen == 0))
        return lhsLen < rhsLen;

      int res = 0;
      if (m_bIgnoreCase)
        res = _strnicmp(lhs, rhs, min(lhsLen, rhsLen));
      else
        res = strncmp(lhs, rhs, min(lhsLen, rhsLen));

      if (res == 0)
        return false;

      return res < 0;
    }
  };

  class LineCompare_OrdinalW : public LineCompare_TextBase < wchar_t >
  {
  protected:
    bool m_bIgnoreCase = false;
    HMODULE m_hModuleKernel32 = 0;
     pCompareStringOrdinal m_pCompareStringOrdinal = nullptr;

  public:
    LineCompare_OrdinalW(bool bIgnoreCase)
      : m_bIgnoreCase(bIgnoreCase)
    {
      m_hModuleKernel32 = ::LoadLibrary( _T("kernel32.dll") );
      if (m_hModuleKernel32)
      {
        m_pCompareStringOrdinal = (pCompareStringOrdinal) GetProcAddress(m_hModuleKernel32, "CompareStringOrdinal");
      }

    }
    ~LineCompare_OrdinalW()
    {
      if (m_hModuleKernel32)
      {
        m_pCompareStringOrdinal = nullptr;
        ::FreeLibrary(m_hModuleKernel32);
        m_hModuleKernel32 = 0;
      }
    }

    bool Compare(const wchar_t* lhs, DWORD lhsLen, const wchar_t* rhs, DWORD rhsLen) override
    { 
     if (lhsLen == 0 && rhsLen == 0)
        return false;

      if ((lhsLen == 0 && rhsLen != 0) || (lhsLen != 0 && rhsLen == 0))
        return lhsLen < rhsLen;

      
        int res = 0;
        if (m_pCompareStringOrdinal)
        {
          res = (m_pCompareStringOrdinal(lhs, lhsLen, rhs, rhsLen, m_bIgnoreCase)-2);
        }
        else
        {
          if (m_bIgnoreCase)
            res = _wcsnicmp(lhs, rhs, min(lhsLen, rhsLen));
          else
            res = wcsncmp(lhs, rhs, min(lhsLen, rhsLen));
        }

        if (res == 0)
          return false;

        return res < 0;
    }
  };

  template<class T>
  class LineCompare_LanguageAware_StringCompareEx : public LineCompare_TextBase < T >
  {
  protected:
    bool m_bIgnoreCase = false;
    DWORD m_compareStringFlags = 0;
    HMODULE m_hModuleKernel32 = 0;
    pCompareStringEx m_pCompareStringEx = nullptr;

    std::wstring m_locale; // "se-SV"
    const wchar_t* szLocaleName = LOCALE_NAME_USER_DEFAULT;
    LCID m_lcid = LOCALE_USER_DEFAULT;
  public:
    void SetLocale(bool bUserDefault, const std::wstring& localeName, LCID lcid)
    {
      if (bUserDefault)
      {
        szLocaleName = LOCALE_NAME_USER_DEFAULT;
        m_locale = L"";
        m_lcid = LOCALE_USER_DEFAULT;
      }
      else
      {
        m_locale = localeName;
        szLocaleName = m_locale.c_str();
        m_lcid = lcid;
      }
    }

    // if bXPMode is true then it will not use CompareStringEx (used by tests)
    LineCompare_LanguageAware_StringCompareEx(bool bNatrualNumSorting, bool bXPMode = false)
      : m_compareStringFlags(0)
    {

      if (bXPMode == false)
      {
        m_hModuleKernel32 = ::LoadLibrary( _T("kernel32.dll") );
        if (m_hModuleKernel32)
        {
          m_pCompareStringEx = (pCompareStringEx) GetProcAddress(m_hModuleKernel32, "CompareStringEx");
        }
      }

      m_compareStringFlags |= LINGUISTIC_IGNORECASE;
      m_compareStringFlags |= NORM_LINGUISTIC_CASING;// use linguistic rules for casing
      //m_compareStringFlags |= SORT_STRINGSORT;

      // only support if using m_pCompareStringEx
      if (bNatrualNumSorting)
      {
        // StrCmpLogicalW on XP ??
        m_compareStringFlags |= SORT_DIGITSASNUMBERS;
      }

    }
    ~LineCompare_LanguageAware_StringCompareEx()
    {
      if (m_hModuleKernel32)
      {
        m_pCompareStringEx = nullptr;
        ::FreeLibrary(m_hModuleKernel32);
        m_hModuleKernel32 = 0;
      }
    }

    bool Compare(const wchar_t* lhs, DWORD lhsLen, const wchar_t* rhs, DWORD rhsLen) //override // Not always override see LineCompare_WFallback
    { 
      if (lhsLen == 0 && rhsLen == 0)
        return false;

      if ((lhsLen == 0 && rhsLen != 0) || (lhsLen != 0 && rhsLen == 0))
        return lhsLen < rhsLen;

      int res = 0;
      if (m_pCompareStringEx)
        res = (m_pCompareStringEx(szLocaleName, m_compareStringFlags, lhs, lhsLen, rhs, rhsLen, nullptr, 0, 0) - 2);
      else
        res = CompareStringW(m_lcid, m_compareStringFlags, lhs, lhsLen, rhs, rhsLen) - 2;

      if (res == 0)
        return false;

      return res < 0;
    }
  };

  // compare using wchar even if strings are char
  class LineCompare_LanguageAware_WFallback : public LineCompare_LanguageAware_StringCompareEx < char >
  {
  protected:
    LineCompareLineCache m_wideCompareStrings;
  public:
    LineCompare_LanguageAware_WFallback(bool bNatrualNumSorting)
      : LineCompare_LanguageAware_StringCompareEx(bNatrualNumSorting)
    {
    }

    bool Compare(const char* lhs, DWORD lhsLen, const char* rhs, DWORD rhsLen) override
    {
      const wchar_t* szLHS = m_wideCompareStrings.GetWideLHS(lhs, lhsLen);
      const wchar_t* szRHS = m_wideCompareStrings.GetWideRHS(rhs, rhsLen);

      return LineCompare_LanguageAware_StringCompareEx<char>::Compare(szLHS, lhsLen, szRHS, rhsLen);
      
    }
  };

  class LineCompare_LanguageAware_StringCompareA : public LineCompare_TextBase < char >
  {
  protected:
    bool m_bIgnoreCase = false;
    DWORD m_compareStringFlags = 0;

    LCID m_lcid = LOCALE_USER_DEFAULT;

  public:
    void SetLocale(LCID lcid)
    {
      m_lcid = lcid;
    }

    LineCompare_LanguageAware_StringCompareA()
      : m_compareStringFlags(0)
    {

      m_compareStringFlags |= LINGUISTIC_IGNORECASE;
      m_compareStringFlags |= NORM_LINGUISTIC_CASING;// use linguistic rules for casing
      //m_compareStringFlags |= SORT_STRINGSORT;

    }

    bool Compare(const char* lhs, DWORD lhsLen, const char* rhs, DWORD rhsLen) 
    { 
      if (lhsLen == 0 && rhsLen == 0)
        return false;

      if ((lhsLen == 0 && rhsLen != 0) || (lhsLen != 0 && rhsLen == 0))
        return lhsLen < rhsLen;

      int res = (CompareStringA(m_lcid, m_compareStringFlags, lhs, lhsLen, rhs, rhsLen) - 2);

      if (res == 0)
        return false;
      return res < 0;
    }
  };

}
