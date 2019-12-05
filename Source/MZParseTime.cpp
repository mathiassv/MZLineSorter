  /*
 * Copyright (c) 1999 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of KTH nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY KTH AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KTH OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */


// MODIFED - Added namespace and stdafx include.

#include <stdafx.h>

#ifdef WIN32

namespace MZLineSorter
{
  int strncasecmp(const char *s1, const char *s2, size_t n)
  {
    while (n > 0 && toupper((unsigned char) *s1) == toupper((unsigned char) *s2))
    {
      if (*s1 == '\0')
        return 0;
      s1++;
      s2++;
      n--;
    }
    if (n == 0)
      return 0;
    return toupper((unsigned char) *s1) - toupper((unsigned char) *s2);
  }

  int strncasecmp(const wchar_t *s1, const wchar_t *s2, size_t n)
  {
    while (n > 0 && towupper(*s1) == towupper(*s2))
    {
      if (*s1 == '\0')
        return 0;
      s1++;
      s2++;
      n--;
    }
    if (n == 0)
      return 0;
    return towupper(*s1) - towupper(*s2);
  }

  static const char *abb_weekdays [] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    NULL
  };

  static const wchar_t *abb_weekdaysW [] = {
    L"Sun",
    L"Mon",
    L"Tue",
    L"Wed",
    L"Thu",
    L"Fri",
    L"Sat",
    NULL
  };

  static const char *full_weekdays [] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    NULL
  };
  
  static const wchar_t *full_weekdaysW [] = {
    L"Sunday",
    L"Monday",
    L"Tuesday",
    L"Wednesday",
    L"Thursday",
    L"Friday",
    L"Saturday",
    NULL
  };
  static const char *abb_month [] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec",
    NULL
  };
    static const wchar_t *abb_monthW [] = {
    L"Jan",
    L"Feb",
    L"Mar",
    L"Apr",
    L"May",
    L"Jun",
    L"Jul",
    L"Aug",
    L"Sep",
    L"Oct",
    L"Nov",
    L"Dec",
    NULL
  };
  static const char *full_month [] = {
    "January",
    "February",
    "Mars",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
    NULL,
  };
    static const wchar_t *full_monthW [] = {
    L"January",
    L"February",
    L"Mars",
    L"April",
    L"May",
    L"June",
    L"July",
    L"August",
    L"September",
    L"October",
    L"November",
    L"December",
    NULL,
  };
  static const char *ampm [] = {
    "am",
    "pm",
    NULL
  };
    static const wchar_t *ampmW [] = {
    L"am",
    L"pm",
    NULL
  };
  /*
   * Try to match `*buf' to one of the strings in `strs'.  Return the
   * index of the matching string (or -1 if none).  Also advance buf.
   */

  static int match_string(const char **buf, const char **strs)
  {
    int i = 0;

    for (i = 0; strs[i] != NULL; ++i)
    {
      int len = (int) strlen(strs[i]);

      if (strncasecmp(*buf, strs[i], len) == 0)
      {
        *buf += len;
        return i;
      }
    }
    return -1;
  }

  static int match_string(const wchar_t **buf, const wchar_t **strs)
  {
    int i = 0;

    for (i = 0; strs[i] != NULL; ++i)
    {
      int len = (int) wcslen(strs[i]);

      if (strncasecmp(*buf, strs[i], len) == 0)
      {
        *buf += len;
        return i;
      }
    }
    return -1;
  }
  /*
   * tm_year is relative this year */

  const int tm_year_base = 1900;

  /*
   * Return TRUE iff `year' was a leap year.
   */

  static int
    is_leap_year(int year)
  {
    return (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0);
  }

  /*
   * Return the weekday [0,6] (0 = Sunday) of the first day of `year'
   */

  static int
    first_day(int year)
  {
    int ret = 4;

    for (; year > 1970; --year)
      ret = (ret + 365 + is_leap_year(year) ? 1 : 0) % 7;
    return ret;
  }

  /*
   * Set `timeptr' given `wnum' (week number [0, 53])
   */

  static void
    set_week_number_sun(struct tm *timeptr, int wnum)
  {
    int fday = first_day(timeptr->tm_year + tm_year_base);

    timeptr->tm_yday = wnum * 7 + timeptr->tm_wday - fday;
    if (timeptr->tm_yday < 0)
    {
      timeptr->tm_wday = fday;
      timeptr->tm_yday = 0;
    }
  }

  /*
   * Set `timeptr' given `wnum' (week number [0, 53])
   */

  static void
    set_week_number_mon(struct tm *timeptr, int wnum)
  {
    int fday = (first_day(timeptr->tm_year + tm_year_base) + 6) % 7;

    timeptr->tm_yday = wnum * 7 + (timeptr->tm_wday + 6) % 7 - fday;
    if (timeptr->tm_yday < 0)
    {
      timeptr->tm_wday = (fday + 1) % 7;
      timeptr->tm_yday = 0;
    }
  }

  /*
   * Set `timeptr' given `wnum' (week number [0, 53])
   */

  static void set_week_number_mon4(struct tm *timeptr, int wnum)
  {
    int fday = (first_day(timeptr->tm_year + tm_year_base) + 6) % 7;
    int offset = 0;

    if (fday < 4)
      offset += 7;

    timeptr->tm_yday = offset + (wnum - 1) * 7 + timeptr->tm_wday - fday;
    if (timeptr->tm_yday < 0)
    {
      timeptr->tm_wday = fday;
      timeptr->tm_yday = 0;
    }
  }

  /*
   *
   */
  wchar_t* strptime(const wchar_t *buf, const char *format, struct tm *timeptr)
  {
    wchar_t c;

    for (; (c = *format) != '\0'; ++format)
    {
      wchar_t *s;
      int ret;

      if (iswspace(c))
      {
        while (iswspace(*buf))
          ++buf;
      }
      else if (c == '%' && format[1] != '\0')
      {
        c = *++format;
        if (c == 'E' || c == 'O')
          c = *++format;
        switch (c)
        {
          case 'A':
            ret = match_string(&buf, full_weekdaysW);
            if (ret < 0)
              return NULL;
            timeptr->tm_wday = ret;
            break;
          case 'a':
            ret = match_string(&buf, abb_weekdaysW);
            if (ret < 0)
              return NULL;
            timeptr->tm_wday = ret;
            break;
          case 'B':
            ret = match_string(&buf, full_monthW);
            if (ret < 0)
              return NULL;
            timeptr->tm_mon = ret;
            break;
          case 'b':
          case 'h':
            ret = match_string(&buf, abb_monthW);
            if (ret < 0)
              return NULL;
            timeptr->tm_mon = ret;
            break;
          case 'C':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_year = (ret * 100) - tm_year_base;
            buf = s;
            break;
          case 'c':
            abort();
          case 'D':   /* %m/%d/%y */
            s = strptime(buf, "%m/%d/%y", timeptr);
            if (s == NULL)
              return NULL;
            buf = s;
            break;
          case 'd':
          case 'e':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_mday = ret;
            buf = s;
            break;
          case 'H':
          case 'k':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_hour = ret;
            buf = s;
            break;
          case 'I':
          case 'l':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            if (ret == 12)
              timeptr->tm_hour = 0;
            else
              timeptr->tm_hour = ret;
            buf = s;
            break;
          case 'j':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_yday = ret - 1;
            buf = s;
            break;
          case 'm':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_mon = ret - 1;
            buf = s;
            break;
          case 'M':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_min = ret;
            buf = s;
            break;
          case 'n':
            if (*buf == '\n')
              ++buf;
            else
              return NULL;
            break;
          case 'p':
            ret = match_string(&buf, ampmW);
            if (ret < 0)
              return NULL;
            if (timeptr->tm_hour == 0)
            {
              if (ret == 1)
                timeptr->tm_hour = 12;
            }
            else
            {
              if (ret == 1) // MSN -2014-11-20 - Only do +12 if we got PM
                timeptr->tm_hour += 12;
            }
            break;
          case 'r':   /* %I:%M:%S %p */
            s = strptime(buf, "%I:%M:%S %p", timeptr);
            if (s == NULL)
              return NULL;
            buf = s;
            break;
          case 'R':   /* %H:%M */
            s = strptime(buf, "%H:%M", timeptr);
            if (s == NULL)
              return NULL;
            buf = s;
            break;
          case 'S':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_sec = ret;
            buf = s;
            break;
          case 't':
            if (*buf == '\t')
              ++buf;
            else
              return NULL;
            break;
          case 'T':   /* %H:%M:%S */
          case 'X':
            s = strptime(buf, "%H:%M:%S", timeptr);
            if (s == NULL)
              return NULL;
            buf = s;
            break;
          case 'u':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_wday = ret - 1;
            buf = s;
            break;
          case 'w':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_wday = ret;
            buf = s;
            break;
          case 'U':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            set_week_number_sun(timeptr, ret);
            buf = s;
            break;
          case 'V':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            set_week_number_mon4(timeptr, ret);
            buf = s;
            break;
          case 'W':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            set_week_number_mon(timeptr, ret);
            buf = s;
            break;
          case 'x':
            s = strptime(buf, "%Y:%m:%d", timeptr);
            if (s == NULL)
              return NULL;
            buf = s;
            break;
          case 'y':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            if (ret < 70)
              timeptr->tm_year = 100 + ret;
            else
              timeptr->tm_year = ret;
            buf = s;
            break;
          case 'Y':
            ret = wcstol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_year = ret - tm_year_base;
            buf = s;
            break;
          case 'Z':
            abort();
          case '\0':
            --format;
            /* FALLTHROUGH */
          case '%':
            if (*buf == '%')
              ++buf;
            else
              return NULL;
            break;
          default:
            if (*buf == '%' || *++buf == c)
              ++buf;
            else
              return NULL;
            break;
        }
      }
      else
      {
        if (*buf == c)
          ++buf;
        else
          return NULL;
      }
    }
    return (wchar_t *) buf;
  }

  char* strptime(const char *buf, const char *format, struct tm *timeptr)
  {
    char c;

    for (; (c = *format) != '\0'; ++format)
    {
      char *s;
      int ret;

      if (isspace(c))
      {
        while (isspace(*buf))
          ++buf;
      }
      else if (c == '%' && format[1] != '\0')
      {
        c = *++format;
        if (c == 'E' || c == 'O')
          c = *++format;
        switch (c)
        {
          case 'A':
            ret = match_string(&buf, full_weekdays);
            if (ret < 0)
              return NULL;
            timeptr->tm_wday = ret;
            break;
          case 'a':
            ret = match_string(&buf, abb_weekdays);
            if (ret < 0)
              return NULL;
            timeptr->tm_wday = ret;
            break;
          case 'B':
            ret = match_string(&buf, full_month);
            if (ret < 0)
              return NULL;
            timeptr->tm_mon = ret;
            break;
          case 'b':
          case 'h':
            ret = match_string(&buf, abb_month);
            if (ret < 0)
              return NULL;
            timeptr->tm_mon = ret;
            break;
          case 'C':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_year = (ret * 100) - tm_year_base;
            buf = s;
            break;
          case 'c':
            abort();
          case 'D':   /* %m/%d/%y */
            s = strptime(buf, "%m/%d/%y", timeptr);
            if (s == NULL)
              return NULL;
            buf = s;
            break;
          case 'd':
          case 'e':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_mday = ret;
            buf = s;
            break;
          case 'H':
          case 'k':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_hour = ret;
            buf = s;
            break;
          case 'I':
          case 'l':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            if (ret == 12)
              timeptr->tm_hour = 0;
            else
              timeptr->tm_hour = ret;
            buf = s;
            break;
          case 'j':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_yday = ret - 1;
            buf = s;
            break;
          case 'm':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_mon = ret - 1;
            buf = s;
            break;
          case 'M':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_min = ret;
            buf = s;
            break;
          case 'n':
            if (*buf == '\n')
              ++buf;
            else
              return NULL;
            break;
          case 'p':
            ret = match_string(&buf, ampm);
            if (ret < 0)
              return NULL;
            if (timeptr->tm_hour == 0)
            {
              if (ret == 1)
                timeptr->tm_hour = 12;
            }
            else
            {
              if (ret == 1) // MSN -2014-11-20 - Only do +12 if we got PM
                timeptr->tm_hour += 12;
            }
            break;
          case 'r':   /* %I:%M:%S %p */
            s = strptime(buf, "%I:%M:%S %p", timeptr);
            if (s == NULL)
              return NULL;
            buf = s;
            break;
          case 'R':   /* %H:%M */
            s = strptime(buf, "%H:%M", timeptr);
            if (s == NULL)
              return NULL;
            buf = s;
            break;
          case 'S':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_sec = ret;
            buf = s;
            break;
          case 't':
            if (*buf == '\t')
              ++buf;
            else
              return NULL;
            break;
          case 'T':   /* %H:%M:%S */
          case 'X':
            s = strptime(buf, "%H:%M:%S", timeptr);
            if (s == NULL)
              return NULL;
            buf = s;
            break;
          case 'u':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_wday = ret - 1;
            buf = s;
            break;
          case 'w':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_wday = ret;
            buf = s;
            break;
          case 'U':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            set_week_number_sun(timeptr, ret);
            buf = s;
            break;
          case 'V':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            set_week_number_mon4(timeptr, ret);
            buf = s;
            break;
          case 'W':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            set_week_number_mon(timeptr, ret);
            buf = s;
            break;
          case 'x':
            s = strptime(buf, "%Y:%m:%d", timeptr);
            if (s == NULL)
              return NULL;
            buf = s;
            break;
          case 'y':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            if (ret < 70)
              timeptr->tm_year = 100 + ret;
            else
              timeptr->tm_year = ret;
            buf = s;
            break;
          case 'Y':
            ret = strtol(buf, &s, 10);
            if (s == buf)
              return NULL;
            timeptr->tm_year = ret - tm_year_base;
            buf = s;
            break;
          case 'Z':
            abort();
          case '\0':
            --format;
            /* FALLTHROUGH */
          case '%':
            if (*buf == '%')
              ++buf;
            else
              return NULL;
            break;
          default:
            if (*buf == '%' || *++buf == c)
              ++buf;
            else
              return NULL;
            break;
        }
      }
      else
      {
        if (*buf == c)
          ++buf;
        else
          return NULL;
      }
    }
    return (char *) buf;
  }

#endif /* WIN32 */



}