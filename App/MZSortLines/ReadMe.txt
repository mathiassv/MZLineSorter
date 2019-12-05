

USAGE: MZLineSort.exe [options] file

Options:
  --help                  Print usage and exit.
  --sort <str>            Text|Date|Num - Sort text as (Default is Text)
  --sortby <str>          Line|SubString|Item - Sort by (Default is Line)
  --linepos <nul>         Line position to start at (Default is 0)
  --sublen <num>          End substring after fixed number of character
  --subendatchar <char>   End substring matching at character
  --subendatws            End substring matching at whitespace
  --itemsep <char>        End substring matching at character
  --item <num>            End substring matching at whitespace
  --date <str>            Date Matching filter (Eg --date "%Y-%m-%d")
  --removeblank           Remove blank line
  --icase                 Ignore Case
  --skipspace             Skip Leading spaces
  --lang                  Language aware sorting (Slower)
  --num                   Natrual Number Sorting
  --keeptop <num>         Keep line at the top
  --keepbottom <num>      Keep line at the bottom
  -r, --reverse           Reverse sorting
  -s, --silent            Silent
  --out                   Output file path. (If not set the original file is
                          overwritten)
  --bak                   Keep original file as *.bak when overwriting
  -o, --overwrite         Overwrite without asking.
  -v                      Verbose output

Examples:
  MZLineSort.exe --icase --reverse --lang --out "C:\MyFile.txt" "C:\infile.txt"
  MZLineSort.exe --sortby date --date "%m-%y-%s" -r file.txt

================================================================================================================

Examples

> MZLineSort.exe C:\MyFile.txt
 Overwrite (y/N) ? : 

> MZLineSort.exe -o C:\MyFile.txt

> MZLineSort.exe - C:\MyFile.txt

=================================================================================================================
Copyright 2015-2019 - Mathias Svensson

