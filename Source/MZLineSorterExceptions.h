#pragma once
namespace MZLineSorter
{
  struct MCLineSorterException : std::exception
  {
    MCLineSorterException(const DWORD dwError, const char* szText)
      : std::exception(szText)
    {
      errorCode = dwError;
    }
    //char const* what() const throw();
    DWORD errorCode = 0;
  };

}