#include "stdafx.h"
#include "CppUnitTest.h"

#include "../../MZDataReader/Source/MZDataReader.h"
#include "../../MZDataReader/Source/MZLineParser.h"
#include "../../MZDataReader/Source/MZLineReader.h"

#include "../Source/MZSortableLinesData.h"
#include "../Source/MZLineSorter.h"

using namespace MZDR;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
/*
namespace Microsoft
{ 
    namespace VisualStudio
    { 
        namespace CppUnitTestFramework
        {
            template<> static std::wstring ToString<int64_t>(const int64_t &t)
            {
              return std::to_wstring(t);
            }
        }
    }
}
*/

namespace MZLineSorter
{
  typedef LineReaderT<char, SortableLinesData> LineReaderA;
  typedef LineReaderT<wchar_t, SortableLinesData> LineReaderW;

    namespace
    {
      __time64_t GetTimeT(int year, int mon, int day, int hour, int min, int sec)
      {
        tm t;
        ZeroMemory(&t, sizeof(tm));
        t.tm_year = year-1900;
        t.tm_mon = mon-1;
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min = min;
        t.tm_sec = sec;
        return _mktime64(&t);
      }

      class AutoRevertLocale
      {
      private:
        std::locale m_OldLocale;

      public:
        AutoRevertLocale(const char* szLocale)
        {
          m_OldLocale = std::locale::global(std::locale(szLocale));
        }

        ~AutoRevertLocale()
        {
          m_OldLocale = std::locale::global(m_OldLocale);
        }

      };
    }


  TEST_CLASS(LineParserTests)
  {
  public:

    std::string GenerateText_ABCLine(DWORD nLines)
    {
      std::string text;
      text.reserve(60 * nLines);

      std::string line;
      for (DWORD n = 0; n < nLines; n++)
      {
        line = "Line : ";
        line += std::to_string(n);
        line += "- abcdefghijklmnopqrstuvwxyzÂ‰ˆ ABCDEFGHIJKLMNOPQRSTUVWXYZ≈ƒ÷ 01234567890\r\n";
        text += line;
      }

      return text;
    }
    
    std::wstring GenerateText_ABCLineW(DWORD nLines)
    {
      std::wstring text;
      text.reserve(60 * nLines);

      std::wstring line;
      for (DWORD n = 0; n < nLines; n++)
      {
        line = L"Line : ";
        line += std::to_wstring(n);
        line += L"- abcdefghijklmnopqrstuvwxyzÂ‰ˆ ABCDEFGHIJKLMNOPQRSTUVWXYZ≈ƒ÷ 01234567890\r\n";
        text += line;
      }

      return text;
    }
    const char* GetResultLine(char* szResult, DWORD nMaxLen, ParseLineResult& result)
    {
      auto len = min(result.length / sizeof(char), nMaxLen);
      strncpy_s(szResult, nMaxLen, reinterpret_cast<const char*>(result.pLine),len);
      szResult[len] = '\0';
      return szResult;
    }

    const wchar_t* GetResultLine(wchar_t* szResult, DWORD nMaxLen, ParseLineResult& result)
    {
      auto len = min(result.length / sizeof(wchar_t), nMaxLen);
      wcsncpy_s(szResult, nMaxLen, reinterpret_cast<const wchar_t*>(result.pLine),len);
      szResult[len] = '\0';
      return szResult;
    }

    std::shared_ptr<SortableLinesData> GetLines(const std::string& text)
    {
      const BYTE* pData = reinterpret_cast<const BYTE*>(text.c_str());
      size_t nBytes = text.length() * sizeof(char);

      MemoryDataReader memoryReader(pData, nBytes, false);

      LineParser parser;
      LineReaderA reader;
      return reader.ReadLinesFromDataReader(&memoryReader, &parser);
    }

    
    std::shared_ptr<SortableLinesData> GetLines(const std::wstring& text)
    {
      const BYTE* pData = reinterpret_cast<const BYTE*>(text.c_str());
      size_t nBytes = text.length() * sizeof(wchar_t);

      MemoryDataReader memoryReader(pData, nBytes, false);

      LineParser parser;
      LineReaderW reader;
      return reader.ReadLinesFromDataReader(&memoryReader, &parser);
    }

    MZLineSortingOptions GetDefaultOptions()
    {
      MZLineSortingOptions options;
      ZeroMemory(&options, sizeof(MZLineSortingOptions));

      return options;
    }
    ////////////////////////////////////////////////////////


    TEST_METHOD(LineParserA_GetLine_LF)
    {
      char szResultLine[1024];
      const char szText [] = "Line1\nLine2\nLine3\nLine4";

      LineParser parser; 
      auto result = parser.ParseLine(szText, szText+strlen(szText));

      GetResultLine(szResultLine, _countof(szResultLine), result);

      Assert::AreEqual("Line1", szResultLine, "Not expected line");
    }

    TEST_METHOD(LineParserW_GetLine_LF)
    {
      wchar_t szResultLine[1024];
      const wchar_t szText [] = L"Line1\nLine2\nLine3\nLine4";

      LineParser parser; 
      auto result = parser.ParseLine(szText, szText+wcslen(szText));

      GetResultLine(szResultLine, _countof(szResultLine), result);

      Assert::AreEqual(L"Line1", szResultLine, "Not expected line");
    }

    TEST_METHOD(LineParserA_GetLine_CRLF)
    {
      char szResultLine[1024];

      const char szText [] = "Line1\r\nLine2\r\nLine3\r\nLine4";

      LineParser parser; 
      
      GetResultLine(szResultLine, _countof(szResultLine), parser.ParseLine(szText, szText+strlen(szText)));

      Assert::AreEqual("Line1", szResultLine, "Not expected line");
    }

    TEST_METHOD(LineParserW_GetLine_CRLF)
    {
      wchar_t szResultLine[1024];

      const wchar_t szText [] = L"Line1\r\nLine2\r\nLine3\r\nLine4";

      LineParser parser; 
      
      GetResultLine(szResultLine, _countof(szResultLine), parser.ParseLine(szText, szText+wcslen(szText)));

      Assert::AreEqual(L"Line1", szResultLine, "Not expected line");
    }


    TEST_METHOD(LineParserA_GetSingleLine)
    {
      char szResultLine[1024];
      const char szText [] = "Line1";

      LineParser parser; 
      auto result = parser.ParseLine(szText, szText+strlen(szText));

      GetResultLine(szResultLine, _countof(szResultLine), result);

      Assert::AreEqual("Line1", szResultLine, "Not expected line");
    }

    TEST_METHOD(LineParserA_GetLine_EmptyLine)
    {
      char szResultLine[1024];

      const char szText [] = "\r\n\r\nLine2";

      LineParser parser; 
      
      GetResultLine(szResultLine, _countof(szResultLine), parser.ParseLine(szText, szText+strlen(szText)));

      Assert::AreEqual("", szResultLine, "Not expected line");
    }

    TEST_METHOD(LineParserW_GetLine_EmptyLine)
    {
      wchar_t szResultLine[1024];

      const wchar_t szText [] = L"\r\n\r\nLine2";

      LineParser parser; 
      
      GetResultLine(szResultLine, _countof(szResultLine), parser.ParseLine(szText, szText+wcslen(szText)));

      Assert::AreEqual(L"", szResultLine, "Not expected line");
    }

    TEST_METHOD(LineReaderA_ParseSimple)
    {
      std::string textBuffer = "Line1\nLine2\nLine3";
      
      const BYTE* pData = reinterpret_cast<const BYTE*>(textBuffer.c_str());
      size_t nBytes = textBuffer.length() * sizeof(char);

      LineParser parser;
      LineReaderA reader;
      auto pLines = reader.ReadLinesFromBuffert(pData, nBytes, &parser);

      size_t expected = 3;
      Assert::AreEqual(expected, pLines->NumLines(), L"Number of lines did not match");
    }

    TEST_METHOD(LineReaderW_ParseSimple)
    {
      std::wstring textBuffer = L"Line1\nLine2\nLine3";
      
      const BYTE* pData = reinterpret_cast<const BYTE*>(textBuffer.c_str());
      size_t nBytes = textBuffer.length() * sizeof(wchar_t);

      LineParser parser;
      LineReaderW reader;
      auto pLines = reader.ReadLinesFromBuffert(pData, nBytes, &parser);

      size_t expected = 3;
      Assert::AreEqual(expected, pLines->NumLines(), L"Number of lines did not match");
    }

    TEST_METHOD(LineReaderA_ParseSimple_EmptyRowAtEnd)
    {
      std::string textBuffer = "Line1\nLine2\nLine3\n\n";
      
      const BYTE* pData = reinterpret_cast<const BYTE*>(textBuffer.c_str());
      size_t nBytes = textBuffer.length() * sizeof(char);

      LineParser parser;
      LineReaderA reader;
      auto pLines = reader.ReadLinesFromBuffert(pData, nBytes, &parser);

      size_t expected = 4;
      Assert::AreEqual(expected, pLines->NumLines(), L"Number of lines did not match");
    }

    TEST_METHOD(LineReaderA_ParseSimple_EmptyRowInMiddle)
    {
      std::string textBuffer = "Line1\nLine2\n\nLine3";
      
      const BYTE* pData = reinterpret_cast<const BYTE*>(textBuffer.c_str());
      size_t nBytes = textBuffer.length() * sizeof(char);

      LineParser parser;
      LineReaderA reader;
      auto pLines = reader.ReadLinesFromBuffert(pData, nBytes, &parser);

      size_t expected = 4;
      Assert::AreEqual(expected, pLines->NumLines(), L"Number of lines did not match");
    }

    TEST_METHOD(LineReaderA_ParseTextBuffer_LargeText)
    {
      std::string textBuffer = GenerateText_ABCLine(100);
      
      const BYTE* pData = reinterpret_cast<const BYTE*>(textBuffer.c_str());
      size_t nBytes = textBuffer.length() * sizeof(char);

      LineParser parser;
      LineReaderA reader;
      auto pLines = reader.ReadLinesFromBuffert(pData, nBytes, &parser);

      size_t expected = 100;
      Assert::AreEqual(expected, pLines->NumLines(), L"Number of lines did not match");
    }

    TEST_METHOD(LineReaderW_ParseTextBuffer_LargeText)
    {
      std::wstring textBuffer = GenerateText_ABCLineW(100);
      
      const BYTE* pData = reinterpret_cast<const BYTE*>(textBuffer.c_str());
      size_t nBytes = textBuffer.length() * sizeof(wchar_t);

      LineParser parser;
      LineReaderW reader;
      auto pLines = reader.ReadLinesFromBuffert(pData, nBytes, &parser);

      size_t expected = 100;
      Assert::AreEqual(expected, pLines->NumLines(), L"Number of lines did not match");
    }

    TEST_METHOD(LineReaderA_ReadLinesUsingMemoryDataReader)
    {
      std::string textBuffer = GenerateText_ABCLine(500);
      const BYTE* pData = reinterpret_cast<const BYTE*>(textBuffer.c_str());
      size_t nBytes = textBuffer.length() * sizeof(char);


      MemoryDataReader memoryReader(pData, nBytes, false);

      LineParser parser;
      LineReaderA reader;
      auto pLines = reader.ReadLinesFromDataReader(&memoryReader, &parser);

      size_t expected = 500;
      Assert::AreEqual(expected, pLines->NumLines(), L"Number of lines did not match");
    }

    TEST_METHOD(LinePreprocessorA_LineOffset)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.LineOffset = 2;
      
      auto pLines = GetLines("1 CCC\n2 AAA\n3 JJ\n4 DDD\n5 BB");

      LinePreprocessor<char> preprocessor(options);
      pLines->Preprocessor(preprocessor);

      Assert::AreEqual(static_cast<size_t>(5), pLines->NumLines(), L"Number of lines did not match");

      const auto& vLines = pLines->GetLines();

      Assert::AreEqual(2UL, vLines.at(0).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(2UL, vLines.at(1).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(2UL, vLines.at(2).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(2UL, vLines.at(3).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(2UL, vLines.at(4).nStartOffset, L"Line offset wrong");
    }

    TEST_METHOD(LinePreprocessorA_SkipWhitespace)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SkipLeadingSpaces;
      
      auto pLines = GetLines(" CCC\n  AAA\n JJ\n   DDD\n  BB");

      LinePreprocessor<char> preprocessor(options);
      pLines->Preprocessor(preprocessor);

      Assert::AreEqual(static_cast<size_t>(5), pLines->NumLines(), L"Number of lines did not match");

      const auto& vLines = pLines->GetLines();

      Assert::AreEqual(1UL, vLines.at(0).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(2UL, vLines.at(1).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(1UL, vLines.at(2).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(3UL, vLines.at(3).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(2UL, vLines.at(4).nStartOffset, L"Line offset wrong");
    }

    TEST_METHOD(LinePreprocessorA_SubStrWithEndAtSpace)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring | Option_SubStr_EndAtWhitespace;
      options.LineOffset = 2;

      auto pLines = GetLines("1 ABC DEFG\n3 CDEF ASS\n4 ABCDE FGH\n2 CDEFG ASE\n5 ABC AB");

      LinePreprocessor<char> preprocessor(options);
      pLines->Preprocessor(preprocessor);

      Assert::AreEqual(static_cast<size_t>(5), pLines->NumLines(), L"Number of lines did not match");

      const auto& vLines = pLines->GetLines();

      Assert::AreEqual(3UL, vLines.at(0).nCompareLen, L"Compare lenght is wrong");
      Assert::AreEqual(4UL, vLines.at(1).nCompareLen, L"Compare lenght is wrong");
      Assert::AreEqual(5UL, vLines.at(2).nCompareLen, L"Compare lenght is wrong");
      Assert::AreEqual(5UL, vLines.at(3).nCompareLen, L"Compare lenght is wrong");
      Assert::AreEqual(3UL, vLines.at(4).nCompareLen, L"Compare lenght is wrong");
    }

    TEST_METHOD(LinePreprocessorW_SubStrWithEndAtSpace)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring | Option_SubStr_EndAtWhitespace;
      options.LineOffset = 2;

      auto pLines = GetLines(L"1 ABC DEFG\n3 CDEF ASS\n4 ABCDE FGH\n2 CDEFG ASE\n5 ABC AB");

      LinePreprocessor<wchar_t> preprocessor(options);
      pLines->Preprocessor(preprocessor);

      Assert::AreEqual(static_cast<size_t>(5), pLines->NumLines(), L"Number of lines did not match");

      const auto& vLines = pLines->GetLines();

      Assert::AreEqual(3UL, vLines.at(0).nCompareLen, L"Compare lenght is wrong");
      Assert::AreEqual(4UL, vLines.at(1).nCompareLen, L"Compare lenght is wrong");
      Assert::AreEqual(5UL, vLines.at(2).nCompareLen, L"Compare lenght is wrong");
      Assert::AreEqual(5UL, vLines.at(3).nCompareLen, L"Compare lenght is wrong");
      Assert::AreEqual(3UL, vLines.at(4).nCompareLen, L"Compare lenght is wrong");
    }

    TEST_METHOD(LinePreprocessorA_SubEndCompareAtComma)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring | Option_SubStr_EndAtChar;
      options.LineOffset = 2;
      options.charMatchA = ',';

      auto pLines = GetLines("1 ABC,DEFG\n3 CDEF,ASS\n4 ABCDE,FGH\n2 CDEFG,ASE\n5 ABC,AB");

      LinePreprocessor<char> preprocessor(options);
      pLines->Preprocessor(preprocessor);

      Assert::AreEqual(static_cast<size_t>(5), pLines->NumLines(), L"Number of lines did not match");

      const auto& vLines = pLines->GetLines();

      Assert::AreEqual(3UL, vLines.at(0).nCompareLen, L"Compare lenght is wrong");
      Assert::AreEqual(4UL, vLines.at(1).nCompareLen, L"Compare lenght is wrong");
      Assert::AreEqual(5UL, vLines.at(2).nCompareLen, L"Compare lenght is wrong");
      Assert::AreEqual(5UL, vLines.at(3).nCompareLen, L"Compare lenght is wrong");
      Assert::AreEqual(3UL, vLines.at(4).nCompareLen, L"Compare lenght is wrong");
    }

    TEST_METHOD(LinePreprocessorA_LineItem)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByLineItem | Option_SkipLeadingSpaces;
      options.charMatchA = ',';
      options.LineItem = 4;

      auto pLines = GetLines(" Item,Item,Item,CBC,Item,Item\n" \
                             " ItemA,ItemB,Item,ABC,Item line3,Item\n" \
                             " Item,Item 2,Item,BCDD,Item,Item\n" \
                             " Item On Line 4,Item,Item,BCDE,Item,Item\n");

      LinePreprocessor<char> preprocessor(options);
      pLines->Preprocessor(preprocessor);

      Assert::AreEqual(static_cast<size_t>(4), pLines->NumLines(), L"Number of lines did not match");

      const auto& vLines = pLines->GetLines();

      Assert::AreEqual(16UL, vLines.at(0).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(3UL, vLines.at(0).nCompareLen, L"Compare lenght is wrong");

      Assert::AreEqual(18UL, vLines.at(1).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(3UL, vLines.at(1).nCompareLen, L"Compare lenght is wrong");

      Assert::AreEqual(18UL, vLines.at(2).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(4UL, vLines.at(2).nCompareLen, L"Compare lenght is wrong");

      Assert::AreEqual(26UL, vLines.at(3).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(4UL, vLines.at(3).nCompareLen, L"Compare lenght is wrong");
    }

    TEST_METHOD(LinePreprocessorW_LineItem)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByLineItem | Option_SkipLeadingSpaces;
      options.charMatchA = ',';
      options.charMatchW = ',';
      options.LineItem = 4;

      auto pLines = GetLines(L" Item,Item,Item,CBC,Item,Item\n" \
                             L" ItemA,ItemB,Item,ABC,Item line3,Item\n" \
                             L" Item,Item 2,Item,BCDD,Item,Item\n" \
                             L" Item On Line 4,Item,Item,BCDE,Item,Item\n");

      LinePreprocessor<wchar_t> preprocessor(options);
      pLines->Preprocessor(preprocessor);

      Assert::AreEqual(static_cast<size_t>(4), pLines->NumLines(), L"Number of lines did not match");

      const auto& vLines = pLines->GetLines();

      Assert::AreEqual(16UL, vLines.at(0).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(3UL, vLines.at(0).nCompareLen, L"Compare lenght is wrong");

      Assert::AreEqual(18UL, vLines.at(1).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(3UL, vLines.at(1).nCompareLen, L"Compare lenght is wrong");

      Assert::AreEqual(18UL, vLines.at(2).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(4UL, vLines.at(2).nCompareLen, L"Compare lenght is wrong");

      Assert::AreEqual(26UL, vLines.at(3).nStartOffset, L"Line offset wrong");
      Assert::AreEqual(4UL, vLines.at(3).nCompareLen, L"Compare lenght is wrong");
    }

    TEST_METHOD(LinePreprocessorA_MatchDate)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring | Option_SkipLeadingSpaces | Option_SortAsDate;
      options.LineOffset = 5;
      strcpy_s(options.szMatch, sizeof(options.szMatch), "%b %d %H:%M:%S %Y");

      auto pLines = GetLines(" AAAA Nov 06 03:44:48 2014 MY DATE : 1415241888\n"
                             " BBBB Dec 01 08:59:25 2014 MY DATE : 1417420765\n"
                             " CCCC Oct 02 08:04:05 2014 MY DATE : 1412233445\n"
                             " DDDD Nov 13 03:59:15 2014 MY DATE : 1415847555\n");

      LinePreprocessor<char> preprocessor(options);
      pLines->Preprocessor(preprocessor);

      __time64_t expected1 = GetTimeT(2014, 11, 6, 3, 44, 48);
      __time64_t expected2 = GetTimeT(2014, 12, 1, 8, 59, 25);
      __time64_t expected3 = GetTimeT(2014, 10, 2, 8, 04, 05);
      __time64_t expected4 = GetTimeT(2014, 11,13, 3, 59, 15);

      const auto& vLines = pLines->GetLines();
      
      Assert::AreEqual(static_cast<size_t>(4), pLines->NumLines(), L"Number of lines did not match");
      
      
      Assert::AreEqual(expected1, vLines.at(0).nTime, L"Parsed time wrong");
      Assert::AreEqual(expected2, vLines.at(1).nTime, L"Parsed time wrong");
      Assert::AreEqual(expected3, vLines.at(2).nTime, L"Parsed time wrong");
      Assert::AreEqual(expected4, vLines.at(3).nTime, L"Parsed time wrong");
    }
    TEST_METHOD(LinePreprocessorW_MatchDate)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring | Option_SkipLeadingSpaces | Option_SortAsDate;
      options.LineOffset = 5;
      strcpy_s(options.szMatch, sizeof(options.szMatch), "%b %d %H:%M:%S %Y");

      auto pLines = GetLines(L" AAAA Nov 06 03:44:48 2014 MY DATE : 1415241888\n"
                             L" BBBB Dec 01 08:59:25 2014 MY DATE : 1417420765\n"
                             L" CCCC Oct 02 08:04:05 2014 MY DATE : 1412233445\n"
                             L" DDDD Nov 13 03:59:15 2014 MY DATE : 1415847555\n");

      LinePreprocessor<wchar_t> preprocessor(options);
      pLines->Preprocessor(preprocessor);

      __time64_t expected1 = GetTimeT(2014, 11, 6, 3, 44, 48);
      __time64_t expected2 = GetTimeT(2014, 12, 1, 8, 59, 25);
      __time64_t expected3 = GetTimeT(2014, 10, 2, 8, 04, 05);
      __time64_t expected4 = GetTimeT(2014, 11,13, 3, 59, 15);

      const auto& vLines = pLines->GetLines();
      
      Assert::AreEqual(static_cast<size_t>(4), pLines->NumLines(), L"Number of lines did not match");
      
      
      Assert::AreEqual(expected1, vLines.at(0).nTime, L"Parsed time wrong");
      Assert::AreEqual(expected2, vLines.at(1).nTime, L"Parsed time wrong");
      Assert::AreEqual(expected3, vLines.at(2).nTime, L"Parsed time wrong");
      Assert::AreEqual(expected4, vLines.at(3).nTime, L"Parsed time wrong");
    }
    TEST_METHOD(LinePreprocessorA_MatchDate_USDateFormat)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring | Option_SkipLeadingSpaces | Option_SortAsDate;
      options.LineOffset = 5;
      strcpy_s(options.szMatch, sizeof(options.szMatch), "%d/%m/%Y %I:%M:%S %p");

      auto pLines = GetLines(" AAAA 06/11/2014 03:44:48 am MY DATE : 1415241888\n"
                             " BBBB 1/12/2014 08:59:25 pm MY DATE : 1417420765\n"
                             " CCCC 2/10/2014 08:04:05 am MY DATE : 1412233445\n"
                             " DDDD 13/11/2014 3:59:15 am MY DATE : 1415847555\n");

      LinePreprocessor<char> preprocessor(options);
      pLines->Preprocessor(preprocessor);

      __time64_t expected1 = GetTimeT(2014, 11, 6, 3, 44, 48);
      __time64_t expected2 = GetTimeT(2014, 12, 1, 20, 59, 25);
      __time64_t expected3 = GetTimeT(2014, 10, 2, 8, 04, 05);
      __time64_t expected4 = GetTimeT(2014, 11,13, 3, 59, 15);

      const auto& vLines = pLines->GetLines();
      
      Assert::AreEqual(static_cast<size_t>(4), pLines->NumLines(), L"Number of lines did not match");
      
      
      Assert::AreEqual(expected1, vLines.at(0).nTime, L"Parsed time wrong");
      Assert::AreEqual(expected2, vLines.at(1).nTime, L"Parsed time wrong");
      Assert::AreEqual(expected3, vLines.at(2).nTime, L"Parsed time wrong");
      Assert::AreEqual(expected4, vLines.at(3).nTime, L"Parsed time wrong");
    }

    TEST_METHOD(LinePreprocessorA_MatchNum)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring | Option_SkipLeadingSpaces | Option_SortAsNum;
      options.LineOffset = 5;
      strcpy_s(options.szMatch, sizeof(options.szMatch), "%d/%m/%Y %I:%M:%S %p");

      auto pLines = GetLines(" AAAA 0215448 MY NUM\n"
                             " BBBB 544874589 MY NUM\n"
                             " CCCC 12345678901 MY NUM\n"
                             " DDDD 000021455 MY NUM\n");

      LinePreprocessor<char> preprocessor(options);
      pLines->Preprocessor(preprocessor);

      const auto& vLines = pLines->GetLines();
      
      Assert::AreEqual(static_cast<size_t>(4), pLines->NumLines(), L"Number of lines did not match");
      
      
      Assert::AreEqual(215448i64, vLines.at(0).nValue, L"Parsed num wrong");
      Assert::AreEqual(544874589i64, vLines.at(1).nValue, L"Parsed num wrong");
      Assert::AreEqual(12345678901i64, vLines.at(2).nValue, L"Parsed num wrong");
      Assert::AreEqual(21455i64, vLines.at(3).nValue, L"Parsed num wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_EntireLine_LangAware)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_LanguageAware;

      char szText [] = { "CBC\n" \
        "BCD\n" \
        "AAC\n" \
        "BAC" };

      char szTextExpected [] = { "AAC\n" \
        "BAC\n" \
        "BCD\n" \
        "CBC" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingW_MemBuffer_EntireLine)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine;

      wchar_t szText [] = { L"BCD\n" \
        L"bcd\n" \
        L"DDD\n" \
        L"ccc\n" \
        L"ddd\n" \
        L"AAC\n" \
        L"aac" };

      // Uppercase is before lowercase
      wchar_t szTextExpected [] = { L"AAC\n" \
        L"BCD\n" \
        L"DDD\n" \
        L"aac\n" \
        L"bcd\n" \
        L"ccc\n" \
        L"ddd" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingW_MemBuffer_EntireLine_CRLF)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine;

      wchar_t szText [] = { L"BCD\r\n" \
        L"bcd\r\n" \
        L"DDD\r\n" \
        L"ccc\r\n" \
        L"ddd\r\n" \
        L"AAC\r\n" \
        L"aac" };

      // Uppercase is before lowercase
      wchar_t szTextExpected [] = { L"AAC\r\n" \
        L"BCD\r\n" \
        L"DDD\r\n" \
        L"aac\r\n" \
        L"bcd\r\n" \
        L"ccc\r\n" \
        L"ddd" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingW_MemBuffer_EntireLine_ErrorCase)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine;

      wchar_t szText [] = { L"   c,vxzn;oinm;ocijner8mdoir  \n" \
        L"                           abbbbbbbbbbbbb   \n" \
        L"aksjioncjdfbieruf,dfk\n" \
        L"ssssssssssss\n" \
        L"ertoitnco875cxn7xn537xn" 
        };

      // Uppercase is before lowercase
      wchar_t szTextExpected [] = { L"                           abbbbbbbbbbbbb   \n" \
        L"   c,vxzn;oinm;ocijner8mdoir  \n" \
        L"aksjioncjdfbieruf,dfk\n" \
        L"ertoitnco875cxn7xn537xn\n" \
        L"ssssssssssss" 
         };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_EntireLine_ErrorCase)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine;

      char szText [] = { "   c,vxzn;oinm;ocijner8mdoir  \n" \
        "                           abbbbbbbbbbbbb   \n" \
        "aksjioncjdfbieruf,dfk\n" \
        "ssssssssssss\n" \
        "ertoitnco875cxn7xn537xn" 
        };

      // Uppercase is before lowercase
      char szTextExpected [] = { "                           abbbbbbbbbbbbb   \n" \
        "   c,vxzn;oinm;ocijner8mdoir  \n" \
        "aksjioncjdfbieruf,dfk\n" \
        "ertoitnco875cxn7xn537xn\n" \
        "ssssssssssss" 
         };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_EntireLine_ErrorCase_CRLF)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine;

      char szText [] = { "   c,vxzn;oinm;ocijner8mdoir  \r\n" \
        "                           abbbbbbbbbbbbb   \r\n" \
        "aksjioncjdfbieruf,dfk\r\n" \
        "ssssssssssss\r\n" \
        "ertoitnco875cxn7xn537xn" 
        };

      // Uppercase is before lowercase
      char szTextExpected [] = { "                           abbbbbbbbbbbbb   \r\n" \
        "   c,vxzn;oinm;ocijner8mdoir  \r\n" \
        "aksjioncjdfbieruf,dfk\r\n" \
        "ertoitnco875cxn7xn537xn\r\n" \
        "ssssssssssss" 
         };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingW_MemBuffer_EntireLine_XP)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_ForceXPFallback;

      wchar_t szText [] = { L"CBC\n" \
        L"BCD\n" \
        L"AAC\n" \
        L"BAC" };

      wchar_t szTextExpected [] = { L"AAC\n" \
        L"BAC\n" \
        L"BCD\n" \
        L"CBC" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingW_MemBuffer_EntireLine_LangAware_Numbers)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_NaturalNumSorting | Option_LanguageAware;

      wchar_t szText [] = { L"050CBC\n" \
        L"30BCD\n" \
        L"001AAC\n" \
        L"002BAC" };

      wchar_t szTextExpected [] = { L"001AAC\n" \
        L"002BAC\n" \
        L"30BCD\n" \
        L"050CBC" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_EntireLineWithNumbers)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_NaturalNumSorting | Option_LanguageAware;
      
      char szText [] = { "050CBC\n" \
        "30BCD\n" \
        "001AAC\n" \
        "002BAC" };

      char szTextExpected [] = { "001AAC\n" \
        "002BAC\n" \
        "30BCD\n" \
        "050CBC" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingW_MemBuffer_EntireLine_IgnoreCase)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_IgnoreCase;

     wchar_t szText [] = { L"AdE\n" \
        L"ADf\n" \
        L"aEf\n" \
        L"abb\n" \
        L"aDd\n" \
        L"abe\n" \
        L"ABC" };

      // Upper/Lower case is together, But Lowercase is always before uppercase.
      wchar_t szTextExpected [] = { L"abb\n" \
        L"ABC\n" \
        L"abe\n" \
        L"aDd\n" \
        L"AdE\n" \
        L"ADf\n" \
        L"aEf" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingW_MemBuffer_EntireLine_IgnoreCase2)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_IgnoreCase;

      wchar_t szText [] = { L"BCD\n" \
        L"bcd\n" \
        L"DDD\n" \
        L"ccc\n" \
        L"ddd\n" \
        L"AAC\n" \
        L"aac" };

      wchar_t szTextExpected [] = { L"AAC\n" \
        L"aac\n" \
        L"BCD\n" \
        L"bcd\n" \
        L"ccc\n" \
        L"DDD\n" \
        L"ddd" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingW_MemBuffer_EntireLine_CaseSensitive_LangAware)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_LanguageAware;

     wchar_t szText [] = { L"AdE\n" \
        L"ADf\n" \
        L"aEf\n" \
        L"abb\n" \
        L"aDd\n" \
        L"abe\n" \
        L"ABC" };

      wchar_t szTextExpected [] = { L"abb\n" \
        L"ABC\n" \
        L"abe\n" \
        L"aDd\n" \
        L"AdE\n" \
        L"ADf\n" \
        L"aEf" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }


    TEST_METHOD(LineSortingA_MemBuffer_EntireLine)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine;

      char szText [] = { "BCD\n" \
        "bcd\n" \
        "DDD\n" \
        "ccc\n" \
        "ddd\n" \
        "AAC\n" \
        "aac" };

      // Uppercase is before lowercase
      char szTextExpected [] = { "AAC\n" \
        "BCD\n" \
        "DDD\n" \
        "aac\n" \
        "bcd\n" \
        "ccc\n" \
        "ddd" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_EntireLine_IgnoreCase)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_IgnoreCase;

      char szText [] = { "BCD\n" \
        "bcd\n" \
        "DDD\n" \
        "ccc\n" \
        "ddd\n" \
        "AAC\n" \
        "aac" };

      char szTextExpected [] = { "AAC\n" \
        "aac\n" \
        "BCD\n" \
        "bcd\n" \
        "ccc\n" \
        "DDD\n" \
        "ddd" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    
    TEST_METHOD(LineSortingW_MemBuffer_EntireLine_LangAware)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_LanguageAware;

      wchar_t szText [] = { L"BCD\n" \
        L"bcd\n" \
        L"DDD\n" \
        L"ccc\n" \
        L"ddd\n" \
        L"AAC\n" \
        L"aac" };

      // LINGUISTIC_IGNORECASE is not used then sorting is changed so lowercase is above uppercase.
      wchar_t szTextExpected [] = { L"AAC\n" \
        L"aac\n" \
        L"BCD\n" \
        L"bcd\n" \
        L"ccc\n" \
        L"DDD\n" \
        L"ddd" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingW_MemBuffer_EntireLine_LangAware_XP)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_LanguageAware | Option_ForceXPFallback;

      wchar_t szText [] = { L"BCD\n" \
        L"bcd\n" \
        L"DDD\n" \
        L"ccc\n" \
        L"ddd\n" \
        L"AAC\n" \
        L"aac" };

       // LINGUISTIC_IGNORECASE is not used then sorting is changed so lowercase is above uppercase.
      wchar_t szTextExpected [] = { L"AAC\n" \
        L"aac\n" \
        L"BCD\n" \
        L"bcd\n" \
        L"ccc\n" \
        L"DDD\n" \
        L"ddd" };


      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingW_MemBuffer_EntireLine_LangAware_SE)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_LanguageAware;

      AutoRevertLocale locale("sv-SE");

      wchar_t szText [] = { L"≈≈≈\n" \
        L"BBB\n" \
        L"÷÷÷\n" \
        L"AAA\n" \
        L"ƒƒƒ" };

      wchar_t szTextExpected [] = { L"AAA\n" \
        L"BBB\n" \
        L"≈≈≈\n" \
        L"ƒƒƒ\n" \
        L"÷÷÷" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }
    /*
    TEST_METHOD(LineSortingW_MemBuffer_EntireLine_FR)
    {
      MCLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine;

      auto loc1 = GetThreadLocale();
      //AutoRevertLocale locale("fr");
      SetThreadLocale(MAKELCID(LANG_FRENCH, SORT_DEFAULT));
      SetThreadUILanguage(MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH));
      auto loc2 = GetThreadLocale();
      
      wchar_t szText [] = { L"cafe\n" \
        L"c‰de\n" \
        L"cabe" };

      wchar_t szTextExpected [] = { L"cabe\n" \
        L"c‰de\n" \
        L"cafe" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }
    */
    TEST_METHOD(LineSortingA_MemBuffer_EntireLine_SkipLeadingSpaces)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_SkipLeadingSpaces;

      char szText [] = { " CBC\n" \
        "BCD\n" \
        " AAC\n" \
        "  BAC" };

      char szTextExpected [] = { " AAC\n" \
        "  BAC\n" \
        "BCD\n" \
        " CBC" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_SubString_Fixed)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring;
      options.LineOffset = 5;
      options.SubStringLen = 3;

      char szText [] = { " AAA CBCDA\n" \
        " BBB BCDDD\n" \
        " CCC AACDDC\n" \
        " DDD AACDDA" };

      char szTextExpected [] = { " CCC AACDDC\n" \
        " DDD AACDDA\n" \
        " BBB BCDDD\n" \
        " AAA CBCDA" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_EntireLine_SkipFirstLine)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring;
      options.LineOffset = 1;
      options.StartLine = 1;

      char szText [] = { " CCC\n" \
        " BBB\n" \
        " DDD\n" \
        " AAA" };

      char szTextExpected [] = { " CCC\n" \
        " AAA\n" \
        " BBB\n" \
        " DDD" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_EntireLine_SkipLastLine)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring;
      options.LineOffset = 1;
      options.EndLine = 1;

      char szText [] = { " CCC\n" \
        " BBB\n" \
        " DDD\n" \
        " AAA" };

      char szTextExpected [] = { " BBB\n" \
        " CCC\n" \
        " DDD\n" \
        " AAA" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_EntireLine_SkipFirstAndLastLine)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring;
      options.LineOffset = 1;
      options.StartLine = 1;
      options.EndLine = 1;

      char szText [] = { " CCC\n" \
        " BBB\n" \
        " DDD\n" \
        " AAA\n" \
        " CCC\n" \
        " AAA" };

      char szTextExpected [] = { " CCC\n" \
        " AAA\n" \
        " BBB\n" \
        " CCC\n" \
        " DDD\n" \
        " AAA" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_SortDescending)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_Reverse;
      

      char szText [] = { "CBC\n" \
        "AAC\n" \
        "BCD\n" \
        "BAC" };

      char szTextExpected [] = { "CBC\n" \
        "BCD\n" \
        "BAC\n" \
        "AAC" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_SubString_Fixed_SortAsNum)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring|Option_SortAsNum;
      options.LineOffset = 5;
      options.SubStringLen = 3;

      char szText [] = { " AAA 333333AAA\n" \
        " BBB 999454BBB\n" \
        " CCC 023522222CCC\n" \
        " DDD 63958448DDDD" };

      char szTextExpected [] = { " CCC 023522222CCC\n" \
        " AAA 333333AAA\n" \
        " DDD 63958448DDDD\n" \
        " BBB 999454BBB" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }
   
    TEST_METHOD(LineSortingW_MemBuffer_SubString_Fixed_SortAsNum)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring|Option_SortAsNum;
      options.LineOffset = 5;
      options.SubStringLen = 3;

      wchar_t szText [] = { L" AAA 333333AAA\n" \
        L" BBB 999454BBB\n" \
        L" CCC 023522222CCC\n" \
        L" DDD 63958448DDDD" };

      wchar_t szTextExpected [] = { L" CCC 023522222CCC\n" \
        L" AAA 333333AAA\n" \
        L" DDD 63958448DDDD\n" \
        L" BBB 999454BBB" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_SortAsNum_WithLeadingSpace)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring|Option_SortAsNum|Option_SkipLeadingSpaces;
      options.LineOffset = 4;
      options.SubStringLen = 3;

      char szText [] = { " AAA 333333AAA\n" \
        " BBB 999454BBB\n" \
        " CCC 023522222CCC\n" \
        " DDD 63958448DDDD" };

      char szTextExpected [] = { " CCC 023522222CCC\n" \
        " AAA 333333AAA\n" \
        " DDD 63958448DDDD\n" \
        " BBB 999454BBB" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_SortAsNum)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortAsNum;
      options.LineOffset = 5;

      char szText [] = { " AAA 333333AAA\n" \
        " BBB 0444444BBB\n" \
        " CCC 222222CCC\n" \
        " DDD 0555555DDDD" };

      char szTextExpected [] = { " CCC 222222CCC\n" \
        " AAA 333333AAA\n" \
        " BBB 0444444BBB\n" \
        " DDD 0555555DDDD" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_SortAsNum_Reverse)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortAsNum | Option_Reverse;
      options.LineOffset = 5;

      char szText [] = { " AAA 333333AAA\n" \
        " BBB 0444444BBB\n" \
        " CCC 222222CCC\n" \
        " DDD 0555555DDDD" };

     char szTextExpected [] = { " DDD 0555555DDDD\n" \
        " BBB 0444444BBB\n" \
        " AAA 333333AAA\n" \
        " CCC 222222CCC" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingW_MemBuffer_SortAsNum)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortAsNum;
      options.LineOffset = 5;

      wchar_t szText [] = { L" AAA 333333AAA\n" \
        L" BBB 0444444BBB\n" \
        L" CCC 222222CCC\n" \
        L" DDD 0555555DDDD" };

      wchar_t szTextExpected [] = { L" CCC 222222CCC\n" \
        L" AAA 333333AAA\n" \
        L" BBB 0444444BBB\n" \
        L" DDD 0555555DDDD" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_SortAsNum_Decending)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortAsNum | Option_Reverse;
      options.LineOffset = 5;

      char szText [] = 
      { " AAA 333333AAA\n" \
        " BBB 444444BBB\n" \
        " CCC 222222CCC\n" \
        " DDD 555555DDD" };

      char szTextExpected [] = 
      { " DDD 555555DDD\n" \
        " BBB 444444BBB\n" \
        " AAA 333333AAA\n" \
        " CCC 222222CCC" };

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_SortAsDate)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring | Option_SkipLeadingSpaces | Option_SortAsDate;
      options.LineOffset = 5;
      strcpy_s(options.szMatch, sizeof(options.szMatch), "%b %d %H:%M:%S %Y");

      char szText [] = { " AAAA Nov 06 03:44:48 2014 MY DATE : 1415241888\n"
        " BBBB Dec 01 08:59:25 2014 MY DATE : 1417420765\n"
        " CCCC Oct 02 08:04:05 2014 MY DATE : 1412233445\n"
        " DDDD Nov 13 03:59:15 2014 MY DATE : 1415847555\n" };

      char szTextExpected [] = { " CCCC Oct 02 08:04:05 2014 MY DATE : 1412233445\n" \
        " AAAA Nov 06 03:44:48 2014 MY DATE : 1415241888\n" \
        " DDDD Nov 13 03:59:15 2014 MY DATE : 1415847555\n" \
        " BBBB Dec 01 08:59:25 2014 MY DATE : 1417420765\n" };

       LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }


    TEST_METHOD(LineSortingW_MemBuffer_SortAsDate)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring | Option_SkipLeadingSpaces | Option_SortAsDate;
      options.LineOffset = 5;
      strcpy_s(options.szMatch, sizeof(options.szMatch), "%b %d %H:%M:%S %Y");

      wchar_t szText [] = { L" AAAA Nov 06 03:44:48 2014 MY DATE : 1415241888\n"
        L" BBBB Dec 01 08:59:25 2014 MY DATE : 1417420765\n"
        L" CCCC Oct 02 08:04:05 2014 MY DATE : 1412233445\n"
        L" DDDD Nov 13 03:59:15 2014 MY DATE : 1415847555\n" };

      wchar_t szTextExpected [] = { L" CCCC Oct 02 08:04:05 2014 MY DATE : 1412233445\n" \
        L" AAAA Nov 06 03:44:48 2014 MY DATE : 1415241888\n" \
        L" DDDD Nov 13 03:59:15 2014 MY DATE : 1415847555\n" \
        L" BBBB Dec 01 08:59:25 2014 MY DATE : 1417420765\n" };

       LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_SortAsDate_Reverse)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortBySubstring | Option_SkipLeadingSpaces | Option_SortAsDate | Option_Reverse;
      options.LineOffset = 5;
      strcpy_s(options.szMatch, sizeof(options.szMatch), "%b %d %H:%M:%S %Y");

      char szText [] = { " AAAA Nov 06 03:44:48 2014 MY DATE : 1415241888\n"
        " BBBB Dec 01 08:59:25 2014 MY DATE : 1417420765\n"
        " CCCC Oct 02 08:04:05 2014 MY DATE : 1412233445\n"
        " DDDD Nov 13 03:59:15 2014 MY DATE : 1415847555\n" };

      char szTextExpected [] = { " BBBB Dec 01 08:59:25 2014 MY DATE : 1417420765\n" \
        " DDDD Nov 13 03:59:15 2014 MY DATE : 1415847555\n" \
        " AAAA Nov 06 03:44:48 2014 MY DATE : 1415241888\n" \
        " CCCC Oct 02 08:04:05 2014 MY DATE : 1412233445\n" };

       LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingA_MemBuffer_EntireLine_RemoveEmptyLines)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_RemoveBlank;

      char szText [] = { "BCD\n" \
        "bcd\n" \
        "\n" \
        "DDD\n" \
        "ccc\n" \
        "\n" \
        "ddd\n" \
        "AAC\n" \
        "aac" };

      char szTextExpected [] = { "AAC\n" \
        "BCD\n" \
        "DDD\n" \
        "aac\n" \
        "bcd\n" \
        "ccc\n" \
        "ddd\n" }; //  <<== SINCE the buff now is smaller. Then the last line will get a \n

      // Note.. Might solve the issue with extra \n at the end. If the buffsize is shrinked with the size of the removed rows.

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }

    TEST_METHOD(LineSortingW_MemBuffer_EntireLine_RemoveEmptyLines)
    {
      MZLineSortingOptions options = GetDefaultOptions();
      options.options = Option_SortByEntireLine | Option_RemoveBlank;

      wchar_t szText [] = { L"BCD\n" \
        L"bcd\n" \
        L"\n" \
        L"DDD\n" \
        L"ccc\n" \
        L"\n" \
        L"ddd\n" \
        L"AAC\n" \
        L"aac" };

      // Uppercase is before lowercase
      wchar_t szTextExpected [] = { L"AAC\n" \
        L"BCD\n" \
        L"DDD\n" \
        L"aac\n" \
        L"bcd\n" \
        L"ccc\n" \
        L"ddd\n" }; //  <<== SINCE the buff now is smaller. Then the last line will get a \n

      // Note.. Might solve the issue with extra \n at the end. If the buffsize is shrinked with the size of the removed rows.

      LineSorter sorter;
      // -1 so we skip the \0 byte at the end
      sorter.SortLinesInBuffert(szText, _countof(szText)-1, options);

      Assert::AreEqual(szTextExpected, szText, L"Sorted Result wrong");
    }


  };
}
