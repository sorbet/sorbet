# typed: __STDLIB_INTERNAL

# date and datetime class - Tadayoshi Funaba 1998-2011
#
# 'date' provides two classes:
# [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html) and
# [`DateTime`](https://docs.ruby-lang.org/en/2.6.0/DateTime.html).
#
# ## Terms and Definitions
#
# Some terms and definitions are based on ISO 8601 and JIS X 0301.
#
# ### Calendar [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html)
#
# The calendar date is a particular day of a calendar year, identified by its
# ordinal number within a calendar month within that year.
#
# In those classes, this is so-called "civil".
#
# ### Ordinal [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html)
#
# The ordinal date is a particular day of a calendar year identified by its
# ordinal number within the year.
#
# In those classes, this is so-called "ordinal".
#
# ### Week [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html)
#
# The week date is a date identified by calendar week and day numbers.
#
# The calendar week is a seven day period within a calendar year, starting on a
# Monday and identified by its ordinal number within the year; the first
# calendar week of the year is the one that includes the first Thursday of that
# year. In the Gregorian calendar, this is equivalent to the week which includes
# January 4.
#
# In those classes, this is so-called "commercial".
#
# ### Julian Day Number
#
# The Julian day number is in elapsed days since noon (Greenwich Mean
# [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html)) on January 1, 4713
# BCE (in the Julian calendar).
#
# In this document, the astronomical Julian day number is the same as the
# original Julian day number. And the chronological Julian day number is a
# variation of the Julian day number. Its days begin at midnight on local time.
#
# In this document, when the term "Julian day number" simply appears, it just
# refers to "chronological Julian day number", not the original.
#
# In those classes, those are so-called "ajd" and "jd".
#
# ### Modified Julian Day Number
#
# The modified Julian day number is in elapsed days since midnight (Coordinated
# Universal [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html)) on November
# 17, 1858 CE (in the Gregorian calendar).
#
# In this document, the astronomical modified Julian day number is the same as
# the original modified Julian day number. And the chronological modified Julian
# day number is a variation of the modified Julian day number. Its days begin at
# midnight on local time.
#
# In this document, when the term "modified Julian day number" simply appears,
# it just refers to "chronological modified Julian day number", not the
# original.
#
# In those classes, those are so-called "amjd" and "mjd".
#
# ## [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html)
#
# A subclass of [`Object`](https://docs.ruby-lang.org/en/2.6.0/Object.html) that
# includes the
# [`Comparable`](https://docs.ruby-lang.org/en/2.6.0/Comparable.html) module and
# easily handles date.
#
# A [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html) object is created
# with
# [`Date::new`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-new),
# [`Date::jd`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-jd),
# [`Date::ordinal`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-ordinal),
# [`Date::commercial`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-commercial),
# [`Date::parse`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-parse),
# [`Date::strptime`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-strptime),
# [`Date::today`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-today),
# [`Time#to_date`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-to_date),
# etc.
#
# ```ruby
# require 'date'
#
# Date.new(2001,2,3)
#  #=> #<Date: 2001-02-03 ...>
# Date.jd(2451944)
#  #=> #<Date: 2001-02-03 ...>
# Date.ordinal(2001,34)
#  #=> #<Date: 2001-02-03 ...>
# Date.commercial(2001,5,6)
#  #=> #<Date: 2001-02-03 ...>
# Date.parse('2001-02-03')
#  #=> #<Date: 2001-02-03 ...>
# Date.strptime('03-02-2001', '%d-%m-%Y')
#  #=> #<Date: 2001-02-03 ...>
# Time.new(2001,2,3).to_date
#  #=> #<Date: 2001-02-03 ...>
# ```
#
# All date objects are immutable; hence cannot modify themselves.
#
# The concept of a date object can be represented as a tuple of the day count,
# the offset and the day of calendar reform.
#
# The day count denotes the absolute position of a temporal dimension. The
# offset is relative adjustment, which determines decoded local time with the
# day count. The day of calendar reform denotes the start day of the new style.
# The old style of the West is the Julian calendar which was adopted by Caesar.
# The new style is the Gregorian calendar, which is the current civil calendar
# of many countries.
#
# The day count is virtually the astronomical Julian day number. The offset in
# this class is usually zero, and cannot be specified directly.
#
# A [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html) object can be
# created with an optional argument, the day of calendar reform as a Julian day
# number, which should be 2298874 to 2426355 or negative/positive infinity. The
# default value is `Date::ITALY` (2299161=1582-10-15). See also sample/cal.rb.
#
# ```
# $ ruby sample/cal.rb -c it 10 1582
#     October 1582
#  S  M Tu  W Th  F  S
#     1  2  3  4 15 16
# 17 18 19 20 21 22 23
# 24 25 26 27 28 29 30
# 31
#
# $ ruby sample/cal.rb -c gb  9 1752
#    September 1752
#  S  M Tu  W Th  F  S
#        1  2 14 15 16
# 17 18 19 20 21 22 23
# 24 25 26 27 28 29 30
# ```
#
# A [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html) object has various
# methods. See each reference.
#
# ```ruby
# d = Date.parse('3rd Feb 2001')
#                              #=> #<Date: 2001-02-03 ...>
# d.year                       #=> 2001
# d.mon                        #=> 2
# d.mday                       #=> 3
# d.wday                       #=> 6
# d += 1                       #=> #<Date: 2001-02-04 ...>
# d.strftime('%a %d %b %Y')    #=> "Sun 04 Feb 2001"
# ```
class Date
  include Comparable

  # An array of strings of abbreviated day names in
  # [`English`](https://docs.ruby-lang.org/en/2.6.0/English.html). The first is
  # "Sun".
  ABBR_DAYNAMES = T.let(T.unsafe(nil), T::Array[String])
  # An array of strings of abbreviated month names in
  # [`English`](https://docs.ruby-lang.org/en/2.6.0/English.html). The first
  # element is nil.
  ABBR_MONTHNAMES = T.let(T.unsafe(nil), T::Array[T.nilable(String)])
  # An array of strings of the full names of days of the week in
  # [`English`](https://docs.ruby-lang.org/en/2.6.0/English.html). The first is
  # "Sunday".
  DAYNAMES = T.let(T.unsafe(nil), T::Array[String])
  # The Julian day number of the day of calendar reform for England and her
  # colonies.
  ENGLAND = T.let(T.unsafe(nil), Integer)
  # The Julian day number of the day of calendar reform for the proleptic
  # Gregorian calendar.
  GREGORIAN = T.let(T.unsafe(nil), Float)
  # The Julian day number of the day of calendar reform for Italy and some
  # catholic countries.
  ITALY = T.let(T.unsafe(nil), Integer)
  # The Julian day number of the day of calendar reform for the proleptic Julian
  # calendar.
  JULIAN = T.let(T.unsafe(nil), Float)
  # An array of strings of full month names in
  # [`English`](https://docs.ruby-lang.org/en/2.6.0/English.html). The first
  # element is nil.
  MONTHNAMES = T.let(T.unsafe(nil), T::Array[T.nilable(String)])

  sig do
    params(
      year: Integer,
      month: Integer,
      mday: Integer,
      start: T.any(Integer, Float),
    )
    .void
  end
  def initialize(year=-4712, month=1, mday=1, start=Date::ITALY); end

  # This method is equivalent to step(max, 1){|date| ...}.
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def upto(arg0); end

  sig do
    params(
      locale: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def localize(locale=T.unsafe(nil), options=T.unsafe(nil)); end

  # Compares the two dates and returns -1, zero, 1 or nil. The other should be a
  # date object or a numeric value as an astronomical Julian day number.
  #
  # ```ruby
  # Date.new(2001,2,3) <=> Date.new(2001,2,4)   #=> -1
  # Date.new(2001,2,3) <=> Date.new(2001,2,3)   #=> 0
  # Date.new(2001,2,3) <=> Date.new(2001,2,2)   #=> 1
  # Date.new(2001,2,3) <=> Object.new           #=> nil
  # Date.new(2001,2,3) <=> Rational(4903887,2)  #=> 0
  # ```
  #
  # See also
  # [`Comparable`](https://docs.ruby-lang.org/en/2.6.0/Comparable.html).
  sig {params(arg0: T.untyped).returns(T.nilable(Integer))}
  def <=>(arg0); end

  # Returns a date object pointing `n` months before self. The argument `n`
  # should be a numeric value.
  #
  # ```ruby
  # Date.new(2001,2,3)  <<  1   #=> #<Date: 2001-01-03 ...>
  # Date.new(2001,2,3)  << -2   #=> #<Date: 2001-04-03 ...>
  # ```
  #
  # When the same day does not exist for the corresponding month, the last day
  # of the month is used instead:
  #
  # ```ruby
  # Date.new(2001,3,28) << 1   #=> #<Date: 2001-02-28 ...>
  # Date.new(2001,3,31) << 1   #=> #<Date: 2001-02-28 ...>
  # ```
  #
  # This also results in the following, possibly unexpected, behavior:
  #
  # ```ruby
  # Date.new(2001,3,31) << 2         #=> #<Date: 2001-01-31 ...>
  # Date.new(2001,3,31) << 1 << 1    #=> #<Date: 2001-01-28 ...>
  #
  # Date.new(2001,3,31) << 1 << -1   #=> #<Date: 2001-03-28 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.self_type)}
  def <<(arg0); end

  # Returns a date object pointing `n` months after self. The argument `n`
  # should be a numeric value.
  #
  # ```ruby
  # Date.new(2001,2,3)  >>  1   #=> #<Date: 2001-03-03 ...>
  # Date.new(2001,2,3)  >> -2   #=> #<Date: 2000-12-03 ...>
  # ```
  #
  # When the same day does not exist for the corresponding month, the last day
  # of the month is used instead:
  #
  # ```ruby
  # Date.new(2001,1,28) >> 1   #=> #<Date: 2001-02-28 ...>
  # Date.new(2001,1,31) >> 1   #=> #<Date: 2001-02-28 ...>
  # ```
  #
  # This also results in the following, possibly unexpected, behavior:
  #
  # ```ruby
  # Date.new(2001,1,31) >> 2         #=> #<Date: 2001-03-31 ...>
  # Date.new(2001,1,31) >> 1 >> 1    #=> #<Date: 2001-03-28 ...>
  #
  # Date.new(2001,1,31) >> 1 >> -1   #=> #<Date: 2001-01-28 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.self_type)}
  def >>(arg0); end

  # Returns true if they are the same day.
  #
  # ```ruby
  # Date.new(2001,2,3) === Date.new(2001,2,3)
  #                                   #=> true
  # Date.new(2001,2,3) === Date.new(2001,2,4)
  #                                   #=> false
  # DateTime.new(2001,2,3) === DateTime.new(2001,2,3,12)
  #                                   #=> true
  # DateTime.new(2001,2,3) === DateTime.new(2001,2,3,0,0,0,'+24:00')
  #                                   #=> true
  # DateTime.new(2001,2,3) === DateTime.new(2001,2,4,0,0,0,'+24:00')
  #                                   #=> false
  # ```
  sig {params(arg0: T.untyped).returns(T::Boolean)}
  def ===(arg0); end

  sig {params(arg0: T.untyped).returns(T::Boolean)}
  def eql?(arg0); end

  # Returns the Julian day number denoting the day of calendar reform.
  #
  # ```ruby
  # Date.new(2001,2,3).start                  #=> 2299161.0
  # Date.new(2001,2,3,Date::GREGORIAN).start  #=> -Infinity
  # ```
  sig {returns(Float)}
  def start(); end

  sig {returns(T.untyped)}
  def marshal_dump(); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def marshal_load(arg0); end

  # Returns the astronomical Julian day number. This is a fractional number,
  # which is not adjusted by the offset.
  #
  # ```ruby
  # DateTime.new(2001,2,3,4,5,6,'+7').ajd     #=> (11769328217/4800)
  # DateTime.new(2001,2,2,14,5,6,'-7').ajd    #=> (11769328217/4800)
  # ```
  sig {returns(T.untyped)}
  def ajd(); end

  # Returns a date object pointing `other` days after self. The other should be
  # a numeric value. If the other is a fractional number, assumes its precision
  # is at most nanosecond.
  #
  # ```ruby
  # Date.new(2001,2,3) + 1    #=> #<Date: 2001-02-04 ...>
  # DateTime.new(2001,2,3) + Rational(1,2)
  #                           #=> #<DateTime: 2001-02-03T12:00:00+00:00 ...>
  # DateTime.new(2001,2,3) + Rational(-1,2)
  #                           #=> #<DateTime: 2001-02-02T12:00:00+00:00 ...>
  # DateTime.jd(0,12) + DateTime.new(2001,2,3).ajd
  #                           #=> #<DateTime: 2001-02-03T00:00:00+00:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.self_type)}
  def +(arg0); end

  # Returns the value as a string for inspection.
  #
  # ```ruby
  # Date.new(2001,2,3).inspect
  #           #=> "#<Date: 2001-02-03 ((2451944j,0s,0n),+0s,2299161j)>"
  # DateTime.new(2001,2,3,4,5,6,'-7').inspect
  #           #=> "#<DateTime: 2001-02-03T04:05:06-07:00 ((2451944j,39906s,0n),-25200s,2299161j)>"
  # ```
  sig {returns(String)}
  def inspect(); end

  # Returns the difference between the two dates if the other is a date object.
  # If the other is a numeric value, returns a date object pointing `other` days
  # before self. If the other is a fractional number, assumes its precision is
  # at most nanosecond.
  #
  # ```ruby
  # Date.new(2001,2,3) - 1   #=> #<Date: 2001-02-02 ...>
  # DateTime.new(2001,2,3) - Rational(1,2)
  #                          #=> #<DateTime: 2001-02-02T12:00:00+00:00 ...>
  # Date.new(2001,2,3) - Date.new(2001)
  #                          #=> (33/1)
  # DateTime.new(2001,2,3) - DateTime.new(2001,2,2,12)
  #                          #=> (1/2)
  # ```
  sig {params(arg0: Numeric).returns(T.self_type)}
  sig {params(arg0: Date).returns(Rational)}
  sig {params(arg0: DateTime).returns(Rational)}
  def -(arg0); end

  # Returns the day of the month (1-31).
  #
  # ```ruby
  # Date.new(2001,2,3).mday           #=> 3
  # ```
  sig {returns(Integer)}
  def mday(); end

  # Returns the day of the month (1-31).
  #
  # ```ruby
  # Date.new(2001,2,3).mday           #=> 3
  # ```
  sig {returns(Integer)}
  def day(); end

  # Returns the month (1-12).
  #
  # ```ruby
  # Date.new(2001,2,3).mon            #=> 2
  # ```
  sig {returns(Integer)}
  def mon(); end

  # Returns the month (1-12).
  #
  # ```ruby
  # Date.new(2001,2,3).mon            #=> 2
  # ```
  sig {returns(Integer)}
  def month(); end

  # Returns the year.
  #
  # ```ruby
  # Date.new(2001,2,3).year           #=> 2001
  # (Date.new(1,1,1) - 1).year        #=> 0
  # ```
  sig {returns(Integer)}
  def year(); end

  # Returns the day of week (0-6, Sunday is zero).
  #
  # ```ruby
  # Date.new(2001,2,3).wday           #=> 6
  # ```
  sig {returns(Integer)}
  def wday(); end

  # Returns the day of the year (1-366).
  #
  # ```ruby
  # Date.new(2001,2,3).yday           #=> 34
  # ```
  sig {returns(Integer)}
  def yday(); end

  # Returns a string in asctime(3) format (but without "n\0" at the end). This
  # method is equivalent to strftime('%c').
  #
  # See also asctime(3) or ctime(3).
  sig {returns(String)}
  def ctime(); end

  sig {returns(T.untyped)}
  def pretty_date(); end

  # Returns a date object denoting the following day.
  sig {returns(T.self_type)}
  def succ(); end

  sig {returns(T.untyped)}
  def to_utc_time(); end

  # Returns the Julian day number. This is a whole number, which is adjusted by
  # the offset as the local time.
  #
  # ```ruby
  # DateTime.new(2001,2,3,4,5,6,'+7').jd      #=> 2451944
  # DateTime.new(2001,2,3,4,5,6,'-7').jd      #=> 2451944
  # ```
  sig {returns(Integer)}
  def jd(); end

  # Returns true if the date is Sunday.
  sig {returns(T::Boolean)}
  def sunday?(); end

  # Returns true if the date is Monday.
  sig {returns(T::Boolean)}
  def monday?(); end

  # Returns true if the date is Tuesday.
  sig {returns(T::Boolean)}
  def tuesday?(); end

  # Returns true if the date is Wednesday.
  sig {returns(T::Boolean)}
  def wednesday?(); end

  # Iterates evaluation of the given block, which takes a date object. The limit
  # should be a date object.
  #
  # ```ruby
  # Date.new(2001).step(Date.new(2001,-1,-1)).select{|d| d.sunday?}.size
  #                           #=> 52
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def step(*arg0); end

  # Returns true if the date is Friday.
  sig {returns(T::Boolean)}
  def friday?(); end

  # Returns true if the date is Saturday.
  sig {returns(T::Boolean)}
  def saturday?(); end

  sig {returns(T::Boolean)}
  def blank?(); end

  # This method is equivalent to step(min, -1){|date| ...}.
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def downto(arg0); end

  # Returns a string in asctime(3) format (but without "n\0" at the end). This
  # method is equivalent to strftime('%c').
  #
  # See also asctime(3) or ctime(3).
  sig {returns(String)}
  def asctime(); end

  # Formats date according to the directives in the given format string. The
  # directives begin with a percent (%) character. Any text not listed as a
  # directive will be passed through to the output string.
  #
  # A directive consists of a percent (%) character, zero or more flags, an
  # optional minimum field width, an optional modifier, and a conversion
  # specifier as follows.
  #
  # ```
  # %<flags><width><modifier><conversion>
  # ```
  #
  # Flags:
  #
  # ```
  # -  don't pad a numerical output.
  # _  use spaces for padding.
  # 0  use zeros for padding.
  # ^  upcase the result string.
  # #  change case.
  # ```
  #
  # The minimum field width specifies the minimum width.
  #
  # The modifiers are "E", "O", ":", "::" and ":::". "E" and "O" are ignored. No
  # effect to result currently.
  #
  # Format directives:
  #
  # ```
  # Date (Year, Month, Day):
  #   %Y - Year with century (can be negative, 4 digits at least)
  #           -0001, 0000, 1995, 2009, 14292, etc.
  #   %C - year / 100 (round down.  20 in 2009)
  #   %y - year % 100 (00..99)
  #
  #   %m - Month of the year, zero-padded (01..12)
  #           %_m  blank-padded ( 1..12)
  #           %-m  no-padded (1..12)
  #   %B - The full month name (``January'')
  #           %^B  uppercased (``JANUARY'')
  #   %b - The abbreviated month name (``Jan'')
  #           %^b  uppercased (``JAN'')
  #   %h - Equivalent to %b
  #
  #   %d - Day of the month, zero-padded (01..31)
  #           %-d  no-padded (1..31)
  #   %e - Day of the month, blank-padded ( 1..31)
  #
  #   %j - Day of the year (001..366)
  #
  # Time (Hour, Minute, Second, Subsecond):
  #   %H - Hour of the day, 24-hour clock, zero-padded (00..23)
  #   %k - Hour of the day, 24-hour clock, blank-padded ( 0..23)
  #   %I - Hour of the day, 12-hour clock, zero-padded (01..12)
  #   %l - Hour of the day, 12-hour clock, blank-padded ( 1..12)
  #   %P - Meridian indicator, lowercase (``am'' or ``pm'')
  #   %p - Meridian indicator, uppercase (``AM'' or ``PM'')
  #
  #   %M - Minute of the hour (00..59)
  #
  #   %S - Second of the minute (00..60)
  #
  #   %L - Millisecond of the second (000..999)
  #   %N - Fractional seconds digits, default is 9 digits (nanosecond)
  #           %3N  millisecond (3 digits)   %15N femtosecond (15 digits)
  #           %6N  microsecond (6 digits)   %18N attosecond  (18 digits)
  #           %9N  nanosecond  (9 digits)   %21N zeptosecond (21 digits)
  #           %12N picosecond (12 digits)   %24N yoctosecond (24 digits)
  #
  # Time zone:
  #   %z - Time zone as hour and minute offset from UTC (e.g. +0900)
  #           %:z - hour and minute offset from UTC with a colon (e.g. +09:00)
  #           %::z - hour, minute and second offset from UTC (e.g. +09:00:00)
  #           %:::z - hour, minute and second offset from UTC
  #                                             (e.g. +09, +09:30, +09:30:30)
  #   %Z - Equivalent to %:z (e.g. +09:00)
  #
  # Weekday:
  #   %A - The full weekday name (``Sunday'')
  #           %^A  uppercased (``SUNDAY'')
  #   %a - The abbreviated name (``Sun'')
  #           %^a  uppercased (``SUN'')
  #   %u - Day of the week (Monday is 1, 1..7)
  #   %w - Day of the week (Sunday is 0, 0..6)
  #
  # ISO 8601 week-based year and week number:
  # The week 1 of YYYY starts with a Monday and includes YYYY-01-04.
  # The days in the year before the first week are in the last week of
  # the previous year.
  #   %G - The week-based year
  #   %g - The last 2 digits of the week-based year (00..99)
  #   %V - Week number of the week-based year (01..53)
  #
  # Week number:
  # The week 1 of YYYY starts with a Sunday or Monday (according to %U
  # or %W).  The days in the year before the first week are in week 0.
  #   %U - Week number of the year.  The week starts with Sunday.  (00..53)
  #   %W - Week number of the year.  The week starts with Monday.  (00..53)
  #
  # Seconds since the Unix Epoch:
  #   %s - Number of seconds since 1970-01-01 00:00:00 UTC.
  #   %Q - Number of milliseconds since 1970-01-01 00:00:00 UTC.
  #
  # Literal string:
  #   %n - Newline character (\n)
  #   %t - Tab character (\t)
  #   %% - Literal ``%'' character
  #
  # Combination:
  #   %c - date and time (%a %b %e %T %Y)
  #   %D - Date (%m/%d/%y)
  #   %F - The ISO 8601 date format (%Y-%m-%d)
  #   %v - VMS date (%e-%b-%Y)
  #   %x - Same as %D
  #   %X - Same as %T
  #   %r - 12-hour time (%I:%M:%S %p)
  #   %R - 24-hour time (%H:%M)
  #   %T - 24-hour time (%H:%M:%S)
  #   %+ - date(1) (%a %b %e %H:%M:%S %Z %Y)
  # ```
  #
  # This method is similar to the strftime() function defined in ISO C and
  # POSIX. Several directives (%a, %A, %b, %B, %c, %p, %r, %x, %X, %E\*, %O\*
  # and %Z) are locale dependent in the function. However, this method is locale
  # independent. So, the result may differ even if the same format string is
  # used in other systems such as C. It is good practice to avoid %x and %X
  # because there are corresponding locale independent representations, %D and
  # %T.
  #
  # Examples:
  #
  # ```ruby
  # d = DateTime.new(2007,11,19,8,37,48,"-06:00")
  #                           #=> #<DateTime: 2007-11-19T08:37:48-0600 ...>
  # d.strftime("Printed on %m/%d/%Y")   #=> "Printed on 11/19/2007"
  # d.strftime("at %I:%M%p")            #=> "at 08:37AM"
  # ```
  #
  # Various ISO 8601 formats:
  #
  # ```
  # %Y%m%d           => 20071119                  Calendar date (basic)
  # %F               => 2007-11-19                Calendar date (extended)
  # %Y-%m            => 2007-11                   Calendar date, reduced accuracy, specific month
  # %Y               => 2007                      Calendar date, reduced accuracy, specific year
  # %C               => 20                        Calendar date, reduced accuracy, specific century
  # %Y%j             => 2007323                   Ordinal date (basic)
  # %Y-%j            => 2007-323                  Ordinal date (extended)
  # %GW%V%u          => 2007W471                  Week date (basic)
  # %G-W%V-%u        => 2007-W47-1                Week date (extended)
  # %GW%V            => 2007W47                   Week date, reduced accuracy, specific week (basic)
  # %G-W%V           => 2007-W47                  Week date, reduced accuracy, specific week (extended)
  # %H%M%S           => 083748                    Local time (basic)
  # %T               => 08:37:48                  Local time (extended)
  # %H%M             => 0837                      Local time, reduced accuracy, specific minute (basic)
  # %H:%M            => 08:37                     Local time, reduced accuracy, specific minute (extended)
  # %H               => 08                        Local time, reduced accuracy, specific hour
  # %H%M%S,%L        => 083748,000                Local time with decimal fraction, comma as decimal sign (basic)
  # %T,%L            => 08:37:48,000              Local time with decimal fraction, comma as decimal sign (extended)
  # %H%M%S.%L        => 083748.000                Local time with decimal fraction, full stop as decimal sign (basic)
  # %T.%L            => 08:37:48.000              Local time with decimal fraction, full stop as decimal sign (extended)
  # %H%M%S%z         => 083748-0600               Local time and the difference from UTC (basic)
  # %T%:z            => 08:37:48-06:00            Local time and the difference from UTC (extended)
  # %Y%m%dT%H%M%S%z  => 20071119T083748-0600      Date and time of day for calendar date (basic)
  # %FT%T%:z         => 2007-11-19T08:37:48-06:00 Date and time of day for calendar date (extended)
  # %Y%jT%H%M%S%z    => 2007323T083748-0600       Date and time of day for ordinal date (basic)
  # %Y-%jT%T%:z      => 2007-323T08:37:48-06:00   Date and time of day for ordinal date (extended)
  # %GW%V%uT%H%M%S%z => 2007W471T083748-0600      Date and time of day for week date (basic)
  # %G-W%V-%uT%T%:z  => 2007-W47-1T08:37:48-06:00 Date and time of day for week date (extended)
  # %Y%m%dT%H%M      => 20071119T0837             Calendar date and local time (basic)
  # %FT%R            => 2007-11-19T08:37          Calendar date and local time (extended)
  # %Y%jT%H%MZ       => 2007323T0837Z             Ordinal date and UTC of day (basic)
  # %Y-%jT%RZ        => 2007-323T08:37Z           Ordinal date and UTC of day (extended)
  # %GW%V%uT%H%M%z   => 2007W471T0837-0600        Week date and local time and difference from UTC (basic)
  # %G-W%V-%uT%R%:z  => 2007-W47-1T08:37-06:00    Week date and local time and difference from UTC (extended)
  # ```
  #
  # See also strftime(3) and
  # [`::strptime`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-strptime).
  sig {params(arg0: T.untyped).returns(String)}
  def strftime(*arg0); end

  # Returns true if the date is Thursday.
  sig {returns(T::Boolean)}
  def thursday?(); end

  # Returns a string in an ISO 8601 format. (This method doesn't use the
  # expanded representations.)
  #
  # ```ruby
  # Date.new(2001,2,3).to_s  #=> "2001-02-03"
  # ```
  sig {returns(String)}
  def to_s(); end

  # Returns true if the year is a leap year.
  #
  # ```ruby
  # Date.new(2000).leap?      #=> true
  # Date.new(2001).leap?      #=> false
  # ```
  sig {returns(T::Boolean)}
  def leap?(); end

  # This method is equivalent to strftime('%F').
  sig {returns(String)}
  def iso8601(); end

  # This method is equivalent to strftime('%FT%T%:z').
  sig {returns(String)}
  def rfc3339(); end

  # This method is equivalent to strftime('%F').
  sig {returns(String)}
  def xmlschema(); end

  # This method is equivalent to strftime('%a, %-d %b %Y %T %z').
  sig {returns(String)}
  def rfc2822(); end

  # This method is equivalent to strftime('%a, %-d %b %Y %T %z').
  sig {returns(String)}
  def rfc822(); end

  # This method is equivalent to strftime('%a, %d %b %Y %T GMT'). See also RFC
  # 2616.
  sig {returns(String)}
  def httpdate(); end

  # Returns a string in a JIS X 0301 format.
  #
  # ```ruby
  # Date.new(2001,2,3).jisx0301       #=> "H13.02.03"
  # ```
  sig {returns(String)}
  def jisx0301(); end

  # Returns the astronomical modified Julian day number. This is a fractional
  # number, which is not adjusted by the offset.
  #
  # ```ruby
  # DateTime.new(2001,2,3,4,5,6,'+7').amjd    #=> (249325817/4800)
  # DateTime.new(2001,2,2,14,5,6,'-7').amjd   #=> (249325817/4800)
  # ```
  sig {returns(T.untyped)}
  def amjd(); end

  # Returns the modified Julian day number. This is a whole number, which is
  # adjusted by the offset as the local time.
  #
  # ```ruby
  # DateTime.new(2001,2,3,4,5,6,'+7').mjd     #=> 51943
  # DateTime.new(2001,2,3,4,5,6,'-7').mjd     #=> 51943
  # ```
  sig {returns(T.untyped)}
  def mjd(); end

  # Returns the fractional part of the day.
  #
  # ```ruby
  # DateTime.new(2001,2,3,12).day_fraction    #=> (1/2)
  # ```
  sig {returns(T.untyped)}
  def day_fraction(); end

  # Returns the calendar week based year.
  #
  # ```ruby
  # Date.new(2001,2,3).cwyear         #=> 2001
  # Date.new(2000,1,1).cwyear         #=> 1999
  # ```
  sig {returns(Integer)}
  def cwyear(); end

  # Returns the calendar week number (1-53).
  #
  # ```ruby
  # Date.new(2001,2,3).cweek          #=> 5
  # ```
  sig {returns(Integer)}
  def cweek(); end

  # Returns the day of calendar week (1-7, Monday is 1).
  #
  # ```ruby
  # Date.new(2001,2,3).cwday          #=> 6
  # ```
  sig {returns(Integer)}
  def cwday(); end

  sig {returns(T.untyped)}
  def hash(); end

  # Returns true if the date is before the day of calendar reform.
  #
  # ```ruby
  # Date.new(1582,10,15).julian?             #=> false
  # (Date.new(1582,10,15) - 1).julian?       #=> true
  # ```
  sig {returns(T::Boolean)}
  def julian?(); end

  # Returns true if the date is on or after the day of calendar reform.
  #
  # ```ruby
  # Date.new(1582,10,15).gregorian?          #=> true
  # (Date.new(1582,10,15) - 1).gregorian?    #=> false
  # ```
  sig {returns(T::Boolean)}
  def gregorian?(); end

  # Duplicates self and resets its day of calendar reform.
  #
  # ```ruby
  # d = Date.new(1582,10,15)
  # d.new_start(Date::JULIAN)         #=> #<Date: 1582-10-05 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def new_start(*arg0); end

  # This method is equivalent to
  # [`new_start`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-i-new_start)(Date::ITALY).
  sig {returns(T.self_type)}
  def italy(); end

  # This method is equivalent to
  # [`new_start`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-i-new_start)(Date::ENGLAND).
  sig {returns(T.self_type)}
  def england(); end

  # This method is equivalent to
  # [`new_start`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-i-new_start)(Date::JULIAN).
  sig {returns(T.self_type)}
  def julian(); end

  # This method is equivalent to
  # [`new_start`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-i-new_start)(Date::GREGORIAN).
  sig {returns(T.self_type)}
  def gregorian(); end

  # This method is equivalent to d + n.
  sig {params(arg0: T.untyped).returns(T.self_type)}
  def next_day(*arg0); end

  # Returns the Lilian day number. This is a whole number, which is adjusted by
  # the offset as the local time.
  #
  # ```ruby
  # Date.new(2001,2,3).ld            #=> 152784
  # ```
  sig {returns(Integer)}
  def ld(); end

  # This method is equivalent to d >> n.
  #
  # See
  # [`Date#>>`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-i-3E-3E)
  # for examples.
  sig {params(arg0: T.untyped).returns(T.self_type)}
  def next_month(*arg0); end

  # This method is equivalent to d - n.
  sig {params(arg0: T.untyped).returns(T.self_type)}
  def prev_day(*arg0); end

  # Returns a date object denoting the following day.
  sig {returns(T.self_type)}
  def next(); end

  # This method is equivalent to d << n.
  #
  # See
  # [`Date#<<`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-i-3C-3C)
  # for examples.
  sig {params(arg0: T.untyped).returns(T.self_type)}
  def prev_month(*arg0); end

  # This method is equivalent to d >> (n \* 12).
  #
  # ```ruby
  # Date.new(2001,2,3).next_year      #=> #<Date: 2002-02-03 ...>
  # Date.new(2008,2,29).next_year     #=> #<Date: 2009-02-28 ...>
  # Date.new(2008,2,29).next_year(4)  #=> #<Date: 2012-02-29 ...>
  # ```
  #
  # See also
  # [`Date#>>`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-i-3E-3E).
  sig {params(arg0: T.untyped).returns(T.self_type)}
  def next_year(*arg0); end

  # This method is equivalent to d << (n \* 12).
  #
  # ```ruby
  # Date.new(2001,2,3).prev_year      #=> #<Date: 2000-02-03 ...>
  # Date.new(2008,2,29).prev_year     #=> #<Date: 2007-02-28 ...>
  # Date.new(2008,2,29).prev_year(4)  #=> #<Date: 2004-02-29 ...>
  # ```
  #
  # See also
  # [`Date#<<`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-i-3C-3C).
  sig {params(arg0: T.untyped).returns(T.self_type)}
  def prev_year(*arg0); end

  # Returns a [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) object
  # which denotes self.
  sig {returns(Time)}
  def to_time(); end

  # Returns self.
  sig {returns(Date)}
  def to_date(); end

  # Returns a [`DateTime`](https://docs.ruby-lang.org/en/2.6.0/DateTime.html)
  # object which denotes self.
  sig {returns(DateTime)}
  def to_datetime(); end

  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self._load(arg0); end

  # Creates a date object denoting the present day.
  #
  # ```ruby
  # Date.today   #=> #<Date: 2011-06-11 ...>
  # ```
  sig {params(arg0: T.untyped).returns(Date)}
  def self.today(*arg0); end

  # Parses the given representation of date and time, and creates a date object.
  # This method does not function as a validator.
  #
  # If the optional second argument is true and the detected year is in the
  # range "00" to "99", considers the year a 2-digit form and makes it full.
  #
  # ```ruby
  # Date.parse('2001-02-03')          #=> #<Date: 2001-02-03 ...>
  # Date.parse('20010203')            #=> #<Date: 2001-02-03 ...>
  # Date.parse('3rd Feb 2001')        #=> #<Date: 2001-02-03 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.attached_class)}
  def self.parse(*arg0); end

  # Creates a date object denoting the given chronological Julian day number.
  #
  # ```ruby
  # Date.jd(2451944)          #=> #<Date: 2001-02-03 ...>
  # Date.jd(2451945)          #=> #<Date: 2001-02-04 ...>
  # Date.jd(0)                #=> #<Date: -4712-01-01 ...>
  # ```
  #
  # See also
  # [`::new`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-new).
  sig {params(arg0: T.untyped).returns(T.attached_class)}
  def self.jd(*arg0); end

  # Just returns true. It's nonsense, but is for symmetry.
  #
  # ```ruby
  # Date.valid_jd?(2451944)           #=> true
  # ```
  #
  # See also
  # [`::jd`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-jd).
  sig {params(arg0: T.untyped).returns(T::Boolean)}
  def self.valid_jd?(*arg0); end

  # Returns true if the given ordinal date is valid, and false if not.
  #
  # ```ruby
  # Date.valid_ordinal?(2001,34)      #=> true
  # Date.valid_ordinal?(2001,366)     #=> false
  # ```
  #
  # See also [`::jd`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-jd)
  # and
  # [`::ordinal`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-ordinal).
  sig {params(arg0: T.untyped).returns(T::Boolean)}
  def self.valid_ordinal?(*arg0); end

  # Returns true if the given calendar date is valid, and false if not.
  #
  # ```ruby
  # Date.valid_date?(2001,2,3)        #=> true
  # Date.valid_date?(2001,2,29)       #=> false
  # ```
  #
  # See also [`::jd`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-jd)
  # and
  # [`::civil`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-civil).
  sig {params(arg0: T.untyped).returns(T::Boolean)}
  def self.valid_civil?(*arg0); end

  # Returns true if the given calendar date is valid, and false if not.
  #
  # ```ruby
  # Date.valid_date?(2001,2,3)        #=> true
  # Date.valid_date?(2001,2,29)       #=> false
  # ```
  #
  # See also [`::jd`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-jd)
  # and
  # [`::civil`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-civil).
  sig {params(arg0: T.untyped).returns(T::Boolean)}
  def self.valid_date?(*arg0); end

  # Returns true if the given week date is valid, and false if not.
  #
  # ```ruby
  # Date.valid_commercial?(2001,5,6)  #=> true
  # Date.valid_commercial?(2001,5,8)  #=> false
  # ```
  #
  # See also [`::jd`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-jd)
  # and
  # [`::commercial`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-commercial).
  sig {params(arg0: T.untyped).returns(T::Boolean)}
  def self.valid_commercial?(*arg0); end

  # Returns true if the given year is a leap year of the proleptic Julian
  # calendar.
  #
  # ```ruby
  # Date.julian_leap?(1900)           #=> true
  # Date.julian_leap?(1901)           #=> false
  # ```
  sig {params(arg0: T.untyped).returns(T::Boolean)}
  def self.julian_leap?(arg0); end

  # Returns true if the given year is a leap year of the proleptic Gregorian
  # calendar.
  #
  # ```ruby
  # Date.gregorian_leap?(1900)        #=> false
  # Date.gregorian_leap?(2000)        #=> true
  # ```
  sig {params(arg0: T.untyped).returns(T::Boolean)}
  def self.gregorian_leap?(arg0); end

  # Returns true if the given year is a leap year of the proleptic Gregorian
  # calendar.
  #
  # ```ruby
  # Date.gregorian_leap?(1900)        #=> false
  # Date.gregorian_leap?(2000)        #=> true
  # ```
  sig {params(arg0: T.untyped).returns(T::Boolean)}
  def self.leap?(arg0); end

  # Creates a date object denoting the given ordinal date.
  #
  # The day of year should be a negative or a positive number (as a relative day
  # from the end of year when negative). It should not be zero.
  #
  # ```ruby
  # Date.ordinal(2001)        #=> #<Date: 2001-01-01 ...>
  # Date.ordinal(2001,34)     #=> #<Date: 2001-02-03 ...>
  # Date.ordinal(2001,-1)     #=> #<Date: 2001-12-31 ...>
  # ```
  #
  # See also [`::jd`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-jd)
  # and [`::new`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-new).
  sig {params(arg0: T.untyped).returns(T.attached_class)}
  def self.ordinal(*arg0); end

  # Creates a date object denoting the given calendar date.
  #
  # In this class, BCE years are counted astronomically. Thus, the year before
  # the year 1 is the year zero, and the year preceding the year zero is the
  # year -1. The month and the day of month should be a negative or a positive
  # number (as a relative month/day from the end of year/month when negative).
  # They should not be zero.
  #
  # The last argument should be a Julian day number which denotes the day of
  # calendar reform.
  # [`Date::ITALY`](https://docs.ruby-lang.org/en/2.6.0/Date.html#ITALY)
  # (2299161=1582-10-15),
  # [`Date::ENGLAND`](https://docs.ruby-lang.org/en/2.6.0/Date.html#ENGLAND)
  # (2361222=1752-09-14),
  # [`Date::GREGORIAN`](https://docs.ruby-lang.org/en/2.6.0/Date.html#GREGORIAN)
  # (the proleptic Gregorian calendar) and
  # [`Date::JULIAN`](https://docs.ruby-lang.org/en/2.6.0/Date.html#JULIAN) (the
  # proleptic Julian calendar) can be specified as a day of calendar reform.
  #
  # ```ruby
  # Date.new(2001)            #=> #<Date: 2001-01-01 ...>
  # Date.new(2001,2,3)        #=> #<Date: 2001-02-03 ...>
  # Date.new(2001,2,-1)       #=> #<Date: 2001-02-28 ...>
  # ```
  #
  # See also
  # [`::jd`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-jd).
  sig {params(arg0: T.untyped).returns(T.attached_class)}
  def self.civil(*arg0); end

  # Creates a date object denoting the given week date.
  #
  # The week and the day of week should be a negative or a positive number (as a
  # relative week/day from the end of year/week when negative). They should not
  # be zero.
  #
  # ```ruby
  # Date.commercial(2001)     #=> #<Date: 2001-01-01 ...>
  # Date.commercial(2002)     #=> #<Date: 2001-12-31 ...>
  # Date.commercial(2001,5,6) #=> #<Date: 2001-02-03 ...>
  # ```
  #
  # See also [`::jd`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-jd)
  # and [`::new`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-c-new).
  sig {params(arg0: T.untyped).returns(T.attached_class)}
  def self.commercial(*arg0); end

  # Parses the given representation of date and time with the given template,
  # and returns a hash of parsed elements. \_strptime does not support
  # specification of flags and width unlike strftime.
  #
  # ```ruby
  # Date._strptime('2001-02-03', '%Y-%m-%d')
  #                           #=> {:year=>2001, :mon=>2, :mday=>3}
  # ```
  #
  # See also strptime(3) and
  # [`strftime`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-i-strftime).
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self._strptime(*arg0); end

  # Parses the given representation of date and time with the given template,
  # and creates a date object. strptime does not support specification of flags
  # and width unlike strftime.
  #
  # ```ruby
  # Date.strptime('2001-02-03', '%Y-%m-%d')   #=> #<Date: 2001-02-03 ...>
  # Date.strptime('03-02-2001', '%d-%m-%Y')   #=> #<Date: 2001-02-03 ...>
  # Date.strptime('2001-034', '%Y-%j')        #=> #<Date: 2001-02-03 ...>
  # Date.strptime('2001-W05-6', '%G-W%V-%u')  #=> #<Date: 2001-02-03 ...>
  # Date.strptime('2001 04 6', '%Y %U %w')    #=> #<Date: 2001-02-03 ...>
  # Date.strptime('2001 05 6', '%Y %W %u')    #=> #<Date: 2001-02-03 ...>
  # Date.strptime('sat3feb01', '%a%d%b%y')    #=> #<Date: 2001-02-03 ...>
  # ```
  #
  # See also strptime(3) and
  # [`strftime`](https://docs.ruby-lang.org/en/2.6.0/Date.html#method-i-strftime).
  sig {params(arg0: T.untyped).returns(T.attached_class)}
  def self.strptime(*arg0); end

  # Parses the given representation of date and time, and returns a hash of
  # parsed elements. This method does not function as a validator.
  #
  # If the optional second argument is true and the detected year is in the
  # range "00" to "99", considers the year a 2-digit form and makes it full.
  #
  # ```ruby
  # Date._parse('2001-02-03') #=> {:year=>2001, :mon=>2, :mday=>3}
  # ```
  sig do
    params(
      arg0: String,
      comp: T::Boolean
    )
    .returns(T::Hash[T.untyped, T.untyped])
  end
  def self._parse(arg0, comp=true); end

  # Returns a hash of parsed elements.
  sig {params(arg0: String).returns(T::Hash[T.untyped, T.untyped])}
  def self._iso8601(arg0); end

  # Creates a new [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html) object
  # by parsing from a string according to some typical ISO 8601 formats.
  #
  # ```ruby
  # Date.iso8601('2001-02-03')        #=> #<Date: 2001-02-03 ...>
  # Date.iso8601('20010203')          #=> #<Date: 2001-02-03 ...>
  # Date.iso8601('2001-W05-6')        #=> #<Date: 2001-02-03 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.attached_class)}
  def self.iso8601(*arg0); end

  # Returns a hash of parsed elements.
  sig {params(arg0: String).returns(T::Hash[T.untyped, T.untyped])}
  def self._rfc3339(arg0); end

  # Creates a new [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html) object
  # by parsing from a string according to some typical RFC 3339 formats.
  #
  # ```ruby
  # Date.rfc3339('2001-02-03T04:05:06+07:00') #=> #<Date: 2001-02-03 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.attached_class)}
  def self.rfc3339(*arg0); end

  # Returns a hash of parsed elements.
  sig {params(arg0: String).returns(T::Hash[T.untyped, T.untyped])}
  def self._xmlschema(arg0); end

  # Creates a new [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html) object
  # by parsing from a string according to some typical
  # [`XML`](https://docs.ruby-lang.org/en/2.6.0/XML.html) Schema formats.
  #
  # ```ruby
  # Date.xmlschema('2001-02-03')      #=> #<Date: 2001-02-03 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.attached_class)}
  def self.xmlschema(*arg0); end

  # Returns a hash of parsed elements.
  sig {params(arg0: String).returns(T::Hash[T.untyped, T.untyped])}
  def self._rfc2822(arg0); end

  # Returns a hash of parsed elements.
  sig {params(arg0: String).returns(T::Hash[T.untyped, T.untyped])}
  def self._rfc822(arg0); end

  # Creates a new [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html) object
  # by parsing from a string according to some typical RFC 2822 formats.
  #
  # ```ruby
  # Date.rfc2822('Sat, 3 Feb 2001 00:00:00 +0000')
  #                                           #=> #<Date: 2001-02-03 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.attached_class)}
  def self.rfc2822(*arg0); end

  # Creates a new [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html) object
  # by parsing from a string according to some typical RFC 2822 formats.
  #
  # ```ruby
  # Date.rfc2822('Sat, 3 Feb 2001 00:00:00 +0000')
  #                                           #=> #<Date: 2001-02-03 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.attached_class)}
  def self.rfc822(*arg0); end

  # Returns a hash of parsed elements.
  sig {params(arg0: String).returns(T::Hash[T.untyped, T.untyped])}
  def self._httpdate(arg0); end

  # Creates a new [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html) object
  # by parsing from a string according to some RFC 2616 format.
  #
  # ```ruby
  # Date.httpdate('Sat, 03 Feb 2001 00:00:00 GMT')
  #                                           #=> #<Date: 2001-02-03 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.attached_class)}
  def self.httpdate(*arg0); end

  # Returns a hash of parsed elements.
  sig {params(arg0: String).returns(T::Hash[T.untyped, T.untyped])}
  def self._jisx0301(arg0); end

  # Creates a new [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html) object
  # by parsing from a string according to some typical JIS X 0301 formats.
  #
  # ```ruby
  # Date.jisx0301('H13.02.03')                #=> #<Date: 2001-02-03 ...>
  # ```
  #
  # For no-era year, legacy format, Heisei is assumed.
  #
  # ```ruby
  # Date.jisx0301('13.02.03')                 #=> #<Date: 2001-02-03 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.attached_class)}
  def self.jisx0301(*arg0); end
end

class Date::Error < ArgumentError; end

class Date::Infinity < Numeric
  sig {returns(Date::Infinity)}
  def +@(); end

  sig {returns(Date::Infinity)}
  def -@(); end

  sig {params(other: T.untyped).returns(T.nilable(Integer))}
  def <=>(other); end

  sig {returns(Float)}
  def to_f(); end

  sig {params(other: T.untyped).returns(Numeric)}
  def coerce(other); end

  sig {returns(Date::Infinity)}
  def abs(); end

  sig {returns(T::Boolean)}
  def zero?(); end

  sig {returns(T::Boolean)}
  def finite?(); end

  sig {returns(T::Boolean)}
  def infinite?(); end

  sig {returns(T::Boolean)}
  def nan?(); end

  sig {returns(T::Boolean)}
  def d(); end
end
