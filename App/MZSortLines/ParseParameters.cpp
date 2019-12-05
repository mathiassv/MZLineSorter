#include <stdafx.h>
#include "ParseParameters.h"
#include "optionparser.h"


struct Arg : public option::Arg
{
  static option::ArgStatus Required(const option::Option& option, bool msg)
  {
    if (option.arg != 0)
      return option::ARG_OK;

    return option::ARG_ILLEGAL;
  }

};

 enum  optionIndex { UNKNOWN, HELP, SORTAS, SORTBY, LINEOFFSET, DATEMATCH, REVERSE, ICASE, LANGUAGESORT, NUM, REMOVEBLANK, 
                     SKIPLEADSPACE, SUBSTRLEN, ENDATCHAR, ENDATWS, ITEMIDX, ITEMSEP, SILENT, KEEPTOP, KEEPBOTTOM , OUTPUT, BACKUP, OVERWRITE, VERBOSE};


 const option::Descriptor usage[] =
 {
  {UNKNOWN, 0,_T("") , _T("")    ,option::Arg::None, _T("USAGE: MCLineSort.exe [options] file\n\n")
                                             _T("Options:") },
  {HELP        ,  0,_T("?"), _T("help")        ,option::Arg::None, _T("  --help  \tPrint usage and exit.") },
  {SORTAS      ,  0,_T(""), _T("sort")         ,Arg::Required, _T("  --sort <str>\tText|Date|Num - Sort text as (Default is Text)") },
  {SORTBY      ,  0,_T(""), _T("sortby")       ,Arg::Required, _T("  --sortby <str>\tLine|SubString|Item - Sort by (Default is Line)") },
  {LINEOFFSET  ,  0,_T(""), _T("linepos")      ,Arg::Required, _T("  --linepos <num>\tLine position to start at (Default is 0)") },
  {SUBSTRLEN   ,  0,_T(""), _T("sublen")       ,Arg::Required, _T("  --sublen <num>\tEnd substring after fixed number of character") },
  {ENDATCHAR   ,  0,_T(""), _T("subendatchar") ,Arg::Required, _T("  --subendatchar <char>\tEnd substring matching at character ") },
  {ENDATWS     ,  0,_T(""), _T("subendatws")   ,Arg::Required, _T("  --subendatws\tEnd substring matching at whitespace") },
  {ITEMIDX     ,  0,_T(""), _T("item")         ,Arg::Required, _T("  --itemsep <char>\tEnd substring matching at character ") },
  {ITEMSEP     ,  0,_T(""), _T("itemsep")      ,Arg::Required, _T("  --item <num>\tEnd substring matching at whitespace") },

  {DATEMATCH   ,  0,_T(""), _T("date")         ,Arg::Required, _T("  --date <str> \tDate Matching filter (Eg --date \"%%Y-%%m-%%d\")") },
  {REMOVEBLANK ,  0,_T(""), _T("removeblank")  ,option::Arg::None, _T("  --removeblank\tRemove blank line") },
  {ICASE       ,  0,_T(""), _T("icase")        ,option::Arg::None, _T("  --icase\tIgnore Case") },
  {SKIPLEADSPACE, 0,_T(""), _T("skipspace")    ,option::Arg::None, _T("  --skipspace\tSkip Leading spaces") },
  {LANGUAGESORT,  0,_T(""), _T("lang")         ,option::Arg::None, _T("  --lang\tLanguage aware sorting (Slower)") },
  {NUM         ,  0,_T(""), _T("num")          ,option::Arg::None, _T("  --num\tNatrual Number Sorting") },
  {KEEPTOP     ,  0,_T(""), _T("keeptop")      ,Arg::Required, _T("  --keeptop <num>\tKeep line at the top") },
  {KEEPBOTTOM  ,  0,_T(""), _T("keepbottom")   ,Arg::Required, _T("  --keepbottom <num>\tKeep line at the bottom") },
  {REVERSE     ,  0,_T("r"), _T("reverse")     ,option::Arg::None, _T("  -r, --reverse\tReverse sorting") },
  {SILENT      ,  0,_T("s"), _T("silent")      ,option::Arg::None, _T("  -s, --silent\tSilent") },
  {OUTPUT      ,  0,_T(""), _T("out")          ,Arg::Required, _T("  --out\tOutput file path. (If not set the original file is overwritten)") },
  {BACKUP      ,  0,_T(""), _T("bak")          ,option::Arg::None, _T("  --bak\tKeep original file as *.bak when overwriting") },
  {OVERWRITE   ,  0,_T("o"), _T("overwrite")   ,option::Arg::None, _T("  -o, --overwrite\tOverwrite without asking.") },
  {VERBOSE     ,  0,_T("v"), _T("verbose")     ,option::Arg::None, _T("  -v\tVerbose output") },
  

  {UNKNOWN, 0,_T(""), _T("")  ,option::Arg::None, _T("\nExamples:\n")
                                             _T("  MCLineSort.exe --icase --reverse --lang --out \"C:\\MyFile.txt\" \"C:\\infile.txt\"\n")
                                             _T("  MCLineSort.exe --sortby date --date \"%%m-%%y-%%s\" -r file.txt\n") },
  {0,0,0,0,0,0}
 };

    void PrintOut(const _TCHAR* str, int size)
    {
      TCHAR buff[1024];
      _tcsncpy_s(buff, _countof(buff), str, size);
      buff[size] = '\0';
      _tprintf(buff);
    }

    bool FileExists(const std::wstring& file)
    {
      DWORD attributes = GetFileAttributes(file.c_str());
      if (attributes == INVALID_FILE_ATTRIBUTES)
        return false;

      return true;
    }

bool ParseCommanLineParameters(CommandLineParam& param, int argc, _TCHAR* argv [])
{
  param.bVerbose = false;
  param.sortOptions.clear();

  argc -= (argc > 0); argv += (argc > 0); // skip program name argv[0] if present

  option::Stats  stats(usage, argc, argv);
  option::Option* options = (option::Option*)calloc(stats.options_max, sizeof(option::Option));
  option::Option* buffer = (option::Option*)calloc(stats.buffer_max, sizeof(option::Option));
  option::Parser parse(usage, argc, argv, options, buffer);

  if (parse.error())
    return false;

  if (options[HELP] || argc == 0)
  {
    option::printUsage(&PrintOut, usage);
    return false;
  }


  for (int i = 0; i < parse.optionsCount(); ++i)
  {
    option::Option& opt = buffer[i];
    switch (opt.index())
    {
      case HELP:
          break;

      case SORTAS:
        if (opt.arg)
        {
          if (_tcsicmp(opt.arg, L"Num") == 0)
            param.sortOptions.options |= MZLineSorter::Option_SortAsNum;

          if (_tcsicmp(opt.arg, L"Text") == 0)
            param.sortOptions.options |= MZLineSorter::Option_SortAsText;

          if (_tcsicmp(opt.arg, L"Date") == 0)
            param.sortOptions.options |= MZLineSorter::Option_SortAsDate;
        }
        break;
      case SORTBY:
        if (opt.arg)
        {
          if (_tcsicmp(opt.arg, L"Line") == 0)
            param.sortOptions.options |= MZLineSorter::Option_SortByEntireLine;

          if (_tcsicmp(opt.arg, L"Sub") == 0)
            param.sortOptions.options |= MZLineSorter::Option_SortBySubstring;

          if (_tcsicmp(opt.arg, L"Item") == 0)
            param.sortOptions.options |= MZLineSorter::Option_SortByLineItem;
        }
        break;
      case DATEMATCH:
        if (opt.arg)
        {
          USES_CONVERSION;
          strncpy_s(param.sortOptions.szMatch, _countof(param.sortOptions.szMatch), T2CA(opt.arg), _countof(param.sortOptions.szMatch));
        }
        break;
      case REVERSE:
        param.sortOptions.options |= MZLineSorter::Option_Reverse;
        break;
      case ICASE:
        param.sortOptions.options |= MZLineSorter::Option_IgnoreCase;
        break;
      case LANGUAGESORT:
        param.sortOptions.options |= MZLineSorter::Option_LanguageAware;
        break;
      case NUM:
        param.sortOptions.options |= MZLineSorter::Option_NaturalNumSorting;
        break;
      case REMOVEBLANK:
        param.sortOptions.options |= MZLineSorter::Option_RemoveBlank;
        break;
      case SKIPLEADSPACE:
        param.sortOptions.options |= MZLineSorter::Option_SkipLeadingSpaces;
        break;
      case SUBSTRLEN:
      {
        if (opt.arg)
        {
          param.sortOptions.SubStringLen = std::stoi(opt.arg);
        }
      }
      break;
      case ENDATCHAR:
        if (opt.arg)
        {
          param.sortOptions.SetMatchChar(opt.arg[0]);
          param.sortOptions.options |= MZLineSorter::Option_SubStr_EndAtChar;
        }
        break;
        case ENDATWS:
        {
          param.sortOptions.options |= MZLineSorter::Option_SubStr_EndAtWhitespace;
        }
        break;
      case ITEMIDX:
      {
        if (opt.arg)
        {
          param.sortOptions.LineItem = std::stoi(opt.arg);
        }
      }
      case ITEMSEP:
      {
        if (opt.arg)
        {
          param.sortOptions.SetMatchChar(opt.arg[0]);
        }
        break;

      }break;
      case SILENT:
      {
        param.sortOptions.options |= MZLineSorter::Option_Silent;
      }
        break;
      case KEEPTOP:
        if (opt.arg)
        {
          param.sortOptions.StartLine = std::stoi(opt.arg);
        }
        break;
      case KEEPBOTTOM:
        if (opt.arg)
        {
          param.sortOptions.EndLine = std::stoi(opt.arg);
        }
        break;
      case LINEOFFSET:
      {
        if (opt.arg)
        {
          param.sortOptions.LineOffset = std::stoi(opt.arg);
        }
      }
      break;
      case OUTPUT:
      {
        if (opt.arg)
          param.outputFile = opt.arg;

        break;
      }
      case BACKUP:
      {
        param.sortOptions.options |= MZLineSorter::Option_Save_KeepOriginalAsBak;
        break;
      }
      case OVERWRITE:
      {
        //param.sortOptions.options |= MCLineSorter::Option_Save_DeleteOriginal;
        param.sortOptions.options |= MZLineSorter::Option_Save_Overwrite;
        break;
      }
      case VERBOSE:
      {
        param.bVerbose = true;
        break;
      }
      default:
      {
        _tprintf(_T("Invalid parameter : %s\n"), opt.name);
      }

    }
  }

  for (int i = 0; i < parse.nonOptionsCount(); ++i)
  {
     std::wstring file = parse.nonOption(i);

     if (FileExists(file) == false)
     {
       _tprintf(L"File not found - : \"%s\"\n", file.c_str());
       continue;
     }
     // check if file
     param.vFiles.push_back(std::move(file));
  }

  return true;
}