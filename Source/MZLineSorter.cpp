#include <stdafx.h>

#include <atlconv.h>
#include <Windows.h>

#include "MZLineSorter.h"
#include "MZLineSorterExceptions.h"
#include "MZLineCompare.h"
#include "MZSortableLinesData.h"

#include "../../MZDataReader/Source/MZDataIdentifier.h"
#include "../../MZDataReader/Source/MZLineReader.h"
#include "../../MZDataReader/Source/MZDataReader.h"
#include "../../MZDataReader/Source/MZDataWriter.h"

#include "../../MZMisc/Source/AutoHandle.h"

using namespace MZLineSorter;
using namespace MZDR;

typedef LineReaderT<char, SortableLinesData> LineReaderA;
typedef LineReaderT<wchar_t, SortableLinesData> LineReaderW;

namespace
{
  void RemoveFileAttributePreventingDelete(const STLString& file)
  {
    DWORD dwAttributes = ::GetFileAttributes(file.c_str());
    DWORD dwChangedAttribute = dwAttributes;
    if (dwAttributes & FILE_ATTRIBUTE_READONLY)
      dwChangedAttribute &= ~FILE_ATTRIBUTE_READONLY;
    if (dwAttributes & FILE_ATTRIBUTE_HIDDEN)
      dwChangedAttribute &= ~FILE_ATTRIBUTE_HIDDEN;
    if (dwAttributes & FILE_ATTRIBUTE_SYSTEM)
      dwChangedAttribute &= ~FILE_ATTRIBUTE_SYSTEM;

    if (dwChangedAttribute != dwAttributes)
      SetFileAttributes(file.c_str(), dwChangedAttribute);
  }

  template<typename T>
  std::shared_ptr<SortableLinesData> SortLinesFromReader(DataReader* pReader, const MZLineSortingOptions& options)
  {
    LineParser lineParser;
    LineReaderT<T, SortableLinesData> reader;
    auto vLineData = reader.ReadLinesFromDataReader(pReader, &lineParser);

    // Preprocess lines.
    LinePreprocessor<T> preprocessor(options);

    vLineData->Preprocessor(preprocessor);

    vLineData->Sort<T>(options);

    if (options.options & MZLineSorter::Option_RemoveBlank)
      vLineData->RemoveEmptyLines(options);

//    LinePostProcessor<T> postprocessor(options);
//    vLineData->PostProcessor(postprocessor);

    return vLineData;
  }

  template<typename T>
  void SortLinesInFiles(const STLString& filename, const STLString& targetFilename, const MZLineSortingOptions& options)
  {
    LineReaderT<T, SortableLinesData> reader;
    FileDataReader dataReader(filename);

    std::shared_ptr<SortableLinesData> pLineData = SortLinesFromReader<T>(&dataReader, options);
    dataReader.Close(); // Must close file so file can be overwritten / renamed when writing new sorted file

    bool bOverwrite = false;
    if (options.options & MZLineSorter::Option_Save_NewName)
    {
      if ((options.options & MZLineSorter::Option_Save_Overwrite) == 0)
      {
        DWORD attributes = ::GetFileAttributes(targetFilename.c_str());
        if (attributes != INVALID_FILE_ATTRIBUTES)
        {
          throw MZLineSorter::MCLineSorterException(ERROR_FILE_EXISTS, "Target file already exists");
        }
      }
      else
      {
        bOverwrite = true;
      }
    }
    else if (options.options & MZLineSorter::Option_Save_Overwrite)
    {
      bOverwrite = true; // Overwrite existing file
    }

    std::wstring fileToDelete;
    if (options.options & MZLineSorter::Option_Save_KeepOriginalAsBak || options.options & MZLineSorter::Option_Save_DeleteOriginal)
    {
      TCHAR szNewNameOfBackup[_MAX_PATH];
      MZDR::MCExtra::BackupFileEx(targetFilename.c_str(), szNewNameOfBackup, _countof(szNewNameOfBackup));
      if (options.options & MZLineSorter::Option_Save_DeleteOriginal)
        fileToDelete = szNewNameOfBackup;
    }

    T c = 0;
    auto newLine = GetNewLine(c, pLineData->GetNewLineStyle(c));
    const BYTE* pNewLine = reinterpret_cast<const BYTE*>(newLine.c_str());
    DWORD dwNewLineLen = static_cast<DWORD>(newLine.length() * sizeof(T));

    LineDataWriter::WriteLinesToFile(targetFilename, pLineData, bOverwrite, pNewLine, dwNewLineLen );

    if (options.options & MZLineSorter::Option_Save_DeleteOriginal)
    {
      if (fileToDelete.empty() == false)
      {
        RemoveFileAttributePreventingDelete(fileToDelete);
        ::DeleteFile(fileToDelete.c_str());
      }
    }
  }

  std::string GetNewLineA(MZDR::NewLine newLineStyle)
  {
    std::string newLine;

    if (newLineStyle == MZDR::CR)
      newLine = "\r";
    if (newLineStyle == MZDR::LF)
      newLine = "\n";
    if (newLineStyle == MZDR::CRLF)
      newLine = "\r\n";

    return newLine;
  }

  std::wstring GetNewLineW(MZDR::NewLine newLineStyle)
  {
    std::wstring newLine;

    if (newLineStyle == MZDR::CR)
      newLine = L"\r";
    if (newLineStyle == MZDR::LF)
      newLine = L"\n";
    if (newLineStyle == MZDR::CRLF)
      newLine = L"\r\n";

    return newLine;
  }


  std::string GetNewLine(char, MZDR::NewLine newLineStyle)
  {
    return GetNewLineA(newLineStyle);
  }

  std::wstring GetNewLine(wchar_t, MZDR::NewLine newLineStyle)
  {
    return GetNewLineW(newLineStyle);
  }
  void CopyLinesToBuffer(const std::vector<LineInfo>& vLines, BYTE* buff, DWORD dwBuffLen, const BYTE* newLine, DWORD newLineLen)
  {

    BYTE* pos = buff;
//    BYTE* pEnd = buff + dwBuffLen;
    size_t dwBuffSizeLeft = dwBuffLen;
    for (auto&& line : vLines)
    {
      size_t len = line.lenght + line.nBytesForNewLine;
      if (len > dwBuffSizeLeft)
        len = dwBuffSizeLeft;

      CopyMemory(pos, line.pLine, len);
      pos += len;
      dwBuffSizeLeft -= len;

      if (line.nBytesForNewLine == 0 && newLineLen < dwBuffSizeLeft)
      {
        CopyMemory(pos, newLine, newLineLen);
        pos += newLineLen;
        dwBuffSizeLeft -= newLineLen;
      }
    }

    if (dwBuffSizeLeft)
    {
      ZeroMemory(pos, dwBuffSizeLeft);
    }
    
  }

  std::wstring StdStringA2W(const std::string& s)
  {
    USES_CONVERSION;
    std::wstring w = A2W(s.c_str());
    return w;
  }
 }

std::wstring LineSorter::GetSampleText(const STLString& filename, DWORD maxlines)
{
  std::wstring text;
  text.reserve(60 * maxlines);
  auto vLines = GetSampleLines(filename, maxlines);

  for (auto&& item : vLines)
  {
    if (!text.empty())
      text += _T("\r\n");
    text += item;
  }
  return text;
}

std::vector<std::wstring> LineSorter::GetSampleLines(const STLString& filename, DWORD maxlines)
{
  std::vector<std::wstring> vSmapleLines;
  vSmapleLines.reserve(maxlines);

  try
  {
    auto format = DataIdentifier::GetContentFormat(filename);
    if (format == ContentAscii)
    {
      LineReaderA reader;
      FileDataReader dataReader(filename);
      LineParser lineParser;
      auto pLineData = reader.ReadLinesFromDataReader(&dataReader, &lineParser);

      std::string text;
      const auto& vLines = pLineData->GetLines();
      for (auto&& item : vLines)
      {
        text = item.GetLine<std::string, char>();
        vSmapleLines.push_back(std::move(StdStringA2W(text)));
      }
    }

  }
  catch (MZDR::MZDataReaderException& /*e*/)
  {
    return vSmapleLines;
  }
  catch (MCLineSorterException& /*e*/)
  {
    // Add Line with error info
    return vSmapleLines;
  }
  catch (std::bad_alloc& /*ba*/)
  {
    // Add Line with error info
//    const char* sz = ba.what();

    return vSmapleLines;
  }

  return vSmapleLines;
}

// Do not throw exception past this function
DWORD LineSorter::SortLines(const STLString& filename, const STLString& targetFilename, const MZLineSortingOptions& options)
{
  try
  {
    ContentFormat format = DataIdentifier::GetContentFormat(filename);

    if (format == ContentAscii)
    {
      SortLinesInFiles<char>(filename, targetFilename, options);
    }
    else if (format == ContentUnicode)
    {
      SortLinesInFiles<wchar_t>(filename, targetFilename, options);
    }
    else
    {
      throw MZLineSorter::MCLineSorterException(ERROR_UNSUPPORTED_TYPE, "Text format not supported");
    }

    return ERROR_SUCCESS;
  }
  catch (MZDR::MZDataReaderException& e)
  {
    m_errorCode = e.errorCode;
    m_errorMessage = e.what();
    return e.errorCode;
  }
  catch (MCLineSorterException& e)
  {
    m_errorCode = e.errorCode;
    m_errorMessage = e.what();
    return e.errorCode;
  }
  catch (std::bad_alloc &ba)
  {
    m_errorCode = ERROR_OUTOFMEMORY;
    m_errorMessage = ba.what();
    return m_errorCode;
  }

}

DWORD LineSorter::SortLinesInBuffert(char* szText, DWORD dwLen, const MZLineSortingOptions& options)
{
  try
  {
    MemoryDataReader dataReader(reinterpret_cast<BYTE*>(szText), dwLen, false);

    auto pLinesData = SortLinesFromReader<char>(&dataReader, options);

    auto& vLines = pLinesData->GetLines();


    // If a line does not have a line ending one needs to be added (As long as it is not the last line)
    std::string newLine = GetNewLineA(pLinesData->GetNewLineStyle(char()));

    CopyLinesToBuffer(vLines, reinterpret_cast<BYTE*>(szText), dwLen, reinterpret_cast<const BYTE*>(newLine.c_str()), static_cast<DWORD>(newLine.length()));
  }
  catch (MZDR::MZDataReaderException& e)
  {
    m_errorCode = e.errorCode;
    m_errorMessage = e.what();
    return e.errorCode;
  }
  catch (MCLineSorterException& e)
  {
    return e.errorCode;
  }
  catch (std::bad_alloc& /*ba*/)
  {
    //const char* sz = ba.what();

    return 1;
  }

  return 0;
}

DWORD LineSorter::SortLinesInBuffert(wchar_t* szText, DWORD dwLen, const MZLineSortingOptions& options)
{
  try
  {
    MemoryDataReader dataReader(reinterpret_cast<BYTE*>(szText), dwLen*sizeof(wchar_t), false);

    auto pLinesData = SortLinesFromReader<wchar_t>(&dataReader, options);

    auto& vLines = pLinesData->GetLines();

    // If a line does not have a line ending one needs to be added (As long as it is not the last line)
    std::wstring newLine = GetNewLineW(pLinesData->GetNewLineStyle(wchar_t()));

    CopyLinesToBuffer(vLines, reinterpret_cast<BYTE*>(szText), dwLen*sizeof(wchar_t), reinterpret_cast<const BYTE*>(newLine.c_str()), static_cast<DWORD>(newLine.length()*sizeof(wchar_t)));

    
  }
  catch (MZDR::MZDataReaderException& e)
  {
    m_errorCode = e.errorCode;
    m_errorMessage = e.what();
    return e.errorCode;
  }
  catch (MCLineSorterException& e)
  {
    return e.errorCode;
  }
  catch (std::bad_alloc& /*ba*/)
  {
    //const char* sz = ba.what();

    return 1;
  }

  return 0;
}
