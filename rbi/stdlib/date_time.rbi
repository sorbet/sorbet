# typed: __STDLIB_INTERNAL

# ## [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html)
#
# A subclass of [`Date`](https://docs.ruby-lang.org/en/2.7.0/Date.html) that
# easily handles date, hour, minute, second, and offset.
#
# [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) does not
# consider any leap seconds, does not track any summer time rules.
#
# A [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) object is
# created with
# [`DateTime::new`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html#method-c-new),
# [`DateTime::jd`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html#method-c-jd),
# [`DateTime::ordinal`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html#method-c-ordinal),
# [`DateTime::commercial`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html#method-c-commercial),
# [`DateTime::parse`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html#method-c-parse),
# [`DateTime::strptime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html#method-c-strptime),
# [`DateTime::now`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html#method-c-now),
# [`Time#to_datetime`](https://docs.ruby-lang.org/en/2.7.0/Time.html#method-i-to_datetime),
# etc.
#
# ```ruby
# require 'date'
#
# DateTime.new(2001,2,3,4,5,6)
#                     #=> #<DateTime: 2001-02-03T04:05:06+00:00 ...>
# ```
#
# The last element of day, hour, minute, or second can be a fractional number.
# The fractional number's precision is assumed at most nanosecond.
#
# ```ruby
# DateTime.new(2001,2,3.5)
#                     #=> #<DateTime: 2001-02-03T12:00:00+00:00 ...>
# ```
#
# An optional argument, the offset, indicates the difference between the local
# time and UTC. For example, `Rational(3,24)` represents ahead of 3 hours of
# UTC, `Rational(-5,24)` represents behind of 5 hours of UTC. The offset should
# be -1 to +1, and its precision is assumed at most second. The default value is
# zero (equals to UTC).
#
# ```ruby
# DateTime.new(2001,2,3,4,5,6,Rational(3,24))
#                     #=> #<DateTime: 2001-02-03T04:05:06+03:00 ...>
# ```
#
# The offset also accepts string form:
#
# ```ruby
# DateTime.new(2001,2,3,4,5,6,'+03:00')
#                     #=> #<DateTime: 2001-02-03T04:05:06+03:00 ...>
# ```
#
# An optional argument, the day of calendar reform (`start`), denotes a Julian
# day number, which should be 2298874 to 2426355 or negative/positive infinity.
# The default value is `Date::ITALY` (2299161=1582-10-15).
#
# A [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) object has
# various methods. See each reference.
#
# ```ruby
# d = DateTime.parse('3rd Feb 2001 04:05:06+03:30')
#                     #=> #<DateTime: 2001-02-03T04:05:06+03:30 ...>
# d.hour              #=> 4
# d.min               #=> 5
# d.sec               #=> 6
# d.offset            #=> (7/48)
# d.zone              #=> "+03:30"
# d += Rational('1.5')
#                     #=> #<DateTime: 2001-02-04%16:05:06+03:30 ...>
# d = d.new_offset('+09:00')
#                     #=> #<DateTime: 2001-02-04%21:35:06+09:00 ...>
# d.strftime('%I:%M:%S %p')
#                     #=> "09:35:06 PM"
# d > DateTime.new(1999)
#                     #=> true
# ```
#
# ### When should you use [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) and when should you use [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html)?
#
# It's a common misconception that [William
# Shakespeare](http://en.wikipedia.org/wiki/William_Shakespeare) and [Miguel de
# Cervantes](http://en.wikipedia.org/wiki/Miguel_de_Cervantes) died on the same
# day in history - so much so that UNESCO named April 23 as [World Book Day
# because of this fact](http://en.wikipedia.org/wiki/World_Book_Day). However,
# because England hadn't yet adopted the [Gregorian Calendar
# Reform](http://en.wikipedia.org/wiki/Gregorian_calendar#Gregorian_reform) (and
# wouldn't until
# [1752](http://en.wikipedia.org/wiki/Calendar_(New_Style)_Act_1750)) their
# deaths are actually 10 days apart. Since Ruby's
# [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) class implements a
# [proleptic Gregorian
# calendar](http://en.wikipedia.org/wiki/Proleptic_Gregorian_calendar) and has
# no concept of calendar reform there's no way to express this with
# [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) objects. This is where
# [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) steps in:
#
# ```ruby
# shakespeare = DateTime.iso8601('1616-04-23', Date::ENGLAND)
#  #=> Tue, 23 Apr 1616 00:00:00 +0000
# cervantes = DateTime.iso8601('1616-04-23', Date::ITALY)
#  #=> Sat, 23 Apr 1616 00:00:00 +0000
# ```
#
# Already you can see something is weird - the days of the week are different.
# Taking this further:
#
# ```ruby
# cervantes == shakespeare
#  #=> false
# (shakespeare - cervantes).to_i
#  #=> 10
# ```
#
# This shows that in fact they died 10 days apart (in reality 11 days since
# Cervantes died a day earlier but was buried on the 23rd). We can see the
# actual date of Shakespeare's death by using the
# [`gregorian`](https://docs.ruby-lang.org/en/2.7.0/Date.html#method-i-gregorian)
# method to convert it:
#
# ```ruby
# shakespeare.gregorian
#  #=> Tue, 03 May 1616 00:00:00 +0000
# ```
#
# So there's an argument that all the celebrations that take place on the 23rd
# April in Stratford-upon-Avon are actually the wrong date since England is now
# using the Gregorian calendar. You can see why when we transition across the
# reform date boundary:
#
# ```ruby
# # start off with the anniversary of Shakespeare's birth in 1751
# shakespeare = DateTime.iso8601('1751-04-23', Date::ENGLAND)
#  #=> Tue, 23 Apr 1751 00:00:00 +0000
#
# # add 366 days since 1752 is a leap year and April 23 is after February 29
# shakespeare + 366
#  #=> Thu, 23 Apr 1752 00:00:00 +0000
#
# # add another 365 days to take us to the anniversary in 1753
# shakespeare + 366 + 365
#  #=> Fri, 04 May 1753 00:00:00 +0000
# ```
#
# As you can see, if we're accurately tracking the number of [solar
# years](http://en.wikipedia.org/wiki/Tropical_year) since Shakespeare's
# birthday then the correct anniversary date would be the 4th May and not the
# 23rd April.
#
# So when should you use
# [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) in Ruby and
# when should you use [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html)?
# Almost certainly you'll want to use
# [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) since your app is
# probably dealing with current dates and times. However, if you need to deal
# with dates and times in a historical context you'll want to use
# [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) to avoid
# making the same mistakes as UNESCO. If you also have to deal with timezones
# then best of luck - just bear in mind that you'll probably be dealing with
# [local solar times](http://en.wikipedia.org/wiki/Solar_time), since it wasn't
# until the 19th century that the introduction of the railways necessitated the
# need for [Standard
# Time](http://en.wikipedia.org/wiki/Standard_time#Great_Britain) and eventually
# timezones.
class DateTime < Date

  # Creates a DateTime object denoting the given calendar date.
  #
  # ```ruby
  # DateTime.new(2001,2,3)    #=> #<DateTime: 2001-02-03T00:00:00+00:00 ...>
  # DateTime.new(2001,2,3,4,5,6,'+7')
  #                          # #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # DateTime.new(2001,-11,-26,-20,-55,-54,'+7')
  #                          # #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # ```
  sig do
    params(
      year: Numeric,
      month: Numeric,
      mday: Numeric,
      hour: Numeric,
      minute: Numeric,
      second: Numeric,
      offset: T.any(Numeric, String),
      start: Numeric
    )
    .void
  end
  def initialize(year=-4712, month=1, mday=1, hour=0, minute=0, second=0, offset=0, start=Date::ITALY); end


  # Returns the minute (0-59).
  #
  # ```ruby
  # DateTime.new(2001,2,3,4,5,6).min          #=> 5
  # ```
  sig {returns(T.untyped)}
  def min(); end

  # Returns a string in an ISO 8601 format. (This method doesn't use the
  # expanded representations.)
  #
  # ```ruby
  # DateTime.new(2001,2,3,4,5,6,'-7').to_s
  #                          #=> "2001-02-03T04:05:06-07:00"
  # ```
  sig {returns(T.untyped)}
  def to_s(); end

  # Returns the offset.
  #
  # ```ruby
  # DateTime.parse('04pm+0730').offset        #=> (5/16)
  # ```
  sig {returns(T.untyped)}
  def offset(); end

  # Returns the timezone.
  #
  # ```ruby
  # DateTime.parse('04pm+0730').zone          #=> "+07:30"
  # ```
  sig {returns(T.untyped)}
  def zone(); end

  # Returns the second (0-59).
  #
  # ```ruby
  # DateTime.new(2001,2,3,4,5,6).sec          #=> 6
  # ```
  sig {returns(T.untyped)}
  def sec(); end

  # Returns the hour (0-23).
  #
  # ```ruby
  # DateTime.new(2001,2,3,4,5,6).hour         #=> 4
  # ```
  sig {returns(T.untyped)}
  def hour(); end

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
  # :  use colons for %z.
  # ```
  #
  # The minimum field width specifies the minimum width.
  #
  # The modifiers are "E" and "O". They are ignored.
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
  # [`::strptime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html#method-c-strptime).
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def strftime(*arg0); end

  # Returns the second (0-59).
  #
  # ```ruby
  # DateTime.new(2001,2,3,4,5,6).sec          #=> 6
  # ```
  sig {returns(T.untyped)}
  def second(); end

  # This method is equivalent to strftime('%FT%T%:z'). The optional argument `n`
  # is the number of digits for fractional seconds.
  #
  # ```ruby
  # DateTime.parse('2001-02-03T04:05:06.123456789+07:00').iso8601(9)
  #                           #=> "2001-02-03T04:05:06.123456789+07:00"
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def iso8601(*arg0); end

  # This method is equivalent to strftime('%FT%T%:z'). The optional argument `n`
  # is the number of digits for fractional seconds.
  #
  # ```ruby
  # DateTime.parse('2001-02-03T04:05:06.123456789+07:00').rfc3339(9)
  #                           #=> "2001-02-03T04:05:06.123456789+07:00"
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def rfc3339(*arg0); end

  # This method is equivalent to strftime('%FT%T%:z'). The optional argument `n`
  # is the number of digits for fractional seconds.
  #
  # ```ruby
  # DateTime.parse('2001-02-03T04:05:06.123456789+07:00').iso8601(9)
  #                           #=> "2001-02-03T04:05:06.123456789+07:00"
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def xmlschema(*arg0); end

  # Returns a string in a JIS X 0301 format. The optional argument `n` is the
  # number of digits for fractional seconds.
  #
  # ```ruby
  # DateTime.parse('2001-02-03T04:05:06.123456789+07:00').jisx0301(9)
  #                           #=> "H13.02.03T04:05:06.123456789+07:00"
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def jisx0301(*arg0); end

  # Returns the minute (0-59).
  #
  # ```ruby
  # DateTime.new(2001,2,3,4,5,6).min          #=> 5
  # ```
  sig {returns(T.untyped)}
  def minute(); end

  # Returns the fractional part of the second.
  #
  # ```ruby
  # DateTime.new(2001,2,3,4,5,6.5).sec_fraction       #=> (1/2)
  # ```
  sig {returns(T.untyped)}
  def sec_fraction(); end

  # Returns the fractional part of the second.
  #
  # ```ruby
  # DateTime.new(2001,2,3,4,5,6.5).sec_fraction       #=> (1/2)
  # ```
  sig {returns(T.untyped)}
  def second_fraction(); end

  # Duplicates self and resets its offset.
  #
  # ```ruby
  # d = DateTime.new(2001,2,3,4,5,6,'-02:00')
  #                           #=> #<DateTime: 2001-02-03T04:05:06-02:00 ...>
  # d.new_offset('+09:00')    #=> #<DateTime: 2001-02-03T15:05:06+09:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def new_offset(*arg0); end

  # Returns a [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) object
  # which denotes self.
  sig {returns(Time)}
  def to_time(); end

  # Returns a [`Date`](https://docs.ruby-lang.org/en/2.7.0/Date.html) object
  # which denotes self.
  sig {returns(Date)}
  def to_date(); end

  # Returns self.
  sig {returns(DateTime)}
  def to_datetime(); end

  sig {returns(T.untyped)}
  def blank?(); end

  sig do
    params(
      locale: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def localize(locale=T.unsafe(nil), options=T.unsafe(nil)); end

  # Creates a [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html)
  # object denoting the present time.
  #
  # ```ruby
  # DateTime.now              #=> #<DateTime: 2011-06-11T21:20:44+09:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(DateTime)}
  def self.now(*arg0); end

  # Parses the given representation of date and time, and creates a
  # [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) object. This
  # method does not function as a validator.
  #
  # If the optional second argument is true and the detected year is in the
  # range "00" to "99", makes it full.
  #
  # ```ruby
  # DateTime.parse('2001-02-03T04:05:06+07:00')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # DateTime.parse('20010203T040506+0700')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # DateTime.parse('3rd Feb 2001 04:05:06 PM')
  #                           #=> #<DateTime: 2001-02-03T16:05:06+00:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.parse(*arg0); end

  # Creates a [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html)
  # object denoting the given chronological Julian day number.
  #
  # ```ruby
  # DateTime.jd(2451944)      #=> #<DateTime: 2001-02-03T00:00:00+00:00 ...>
  # DateTime.jd(2451945)      #=> #<DateTime: 2001-02-04T00:00:00+00:00 ...>
  # DateTime.jd(Rational('0.5'))
  #                           #=> #<DateTime: -4712-01-01T12:00:00+00:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.jd(*arg0); end

  # Creates a [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html)
  # object denoting the given ordinal date.
  #
  # ```ruby
  # DateTime.ordinal(2001,34) #=> #<DateTime: 2001-02-03T00:00:00+00:00 ...>
  # DateTime.ordinal(2001,34,4,5,6,'+7')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # DateTime.ordinal(2001,-332,-20,-55,-54,'+7')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.ordinal(*arg0); end

  # Creates a [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html)
  # object denoting the given calendar date.
  #
  # ```ruby
  # DateTime.new(2001,2,3)    #=> #<DateTime: 2001-02-03T00:00:00+00:00 ...>
  # DateTime.new(2001,2,3,4,5,6,'+7')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # DateTime.new(2001,-11,-26,-20,-55,-54,'+7')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.civil(*arg0); end

  # Creates a [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html)
  # object denoting the given week date.
  #
  # ```ruby
  # DateTime.commercial(2001) #=> #<DateTime: 2001-01-01T00:00:00+00:00 ...>
  # DateTime.commercial(2002) #=> #<DateTime: 2001-12-31T00:00:00+00:00 ...>
  # DateTime.commercial(2001,5,6,4,5,6,'+7')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.commercial(*arg0); end

  # Parses the given representation of date and time with the given template,
  # and returns a hash of parsed elements. \_strptime does not support
  # specification of flags and width unlike strftime.
  #
  # See also strptime(3) and
  # [`strftime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html#method-i-strftime).
  sig do
    params(
      arg0: String,
      format: String
    )
    .returns(T::Hash[T.untyped, T.untyped])
  end
  def self._strptime(arg0, format="%F"); end

  # Parses the given representation of date and time with the given template,
  # and creates a
  # [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) object.
  # strptime does not support specification of flags and width unlike strftime.
  #
  # ```ruby
  # DateTime.strptime('2001-02-03T04:05:06+07:00', '%Y-%m-%dT%H:%M:%S%z')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # DateTime.strptime('03-02-2001 04:05:06 PM', '%d-%m-%Y %I:%M:%S %p')
  #                           #=> #<DateTime: 2001-02-03T16:05:06+00:00 ...>
  # DateTime.strptime('2001-W05-6T04:05:06+07:00', '%G-W%V-%uT%H:%M:%S%z')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # DateTime.strptime('2001 04 6 04 05 06 +7', '%Y %U %w %H %M %S %z')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # DateTime.strptime('2001 05 6 04 05 06 +7', '%Y %W %u %H %M %S %z')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # DateTime.strptime('-1', '%s')
  #                           #=> #<DateTime: 1969-12-31T23:59:59+00:00 ...>
  # DateTime.strptime('-1000', '%Q')
  #                           #=> #<DateTime: 1969-12-31T23:59:59+00:00 ...>
  # DateTime.strptime('sat3feb014pm+7', '%a%d%b%y%H%p%z')
  #                           #=> #<DateTime: 2001-02-03T16:00:00+07:00 ...>
  # ```
  #
  # See also strptime(3) and
  # [`strftime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html#method-i-strftime).
  sig {params(str: String, fmt: String, sg: Integer).returns(T.attached_class)}
  def self.strptime(str="-4712-01-01T00:00:00+00:00", fmt="%FT%T%z", sg=Date::ITALY); end

  # Creates a new
  # [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) object by
  # parsing from a string according to some typical ISO 8601 formats.
  #
  # ```ruby
  # DateTime.iso8601('2001-02-03T04:05:06+07:00')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # DateTime.iso8601('20010203T040506+0700')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # DateTime.iso8601('2001-W05-6T04:05:06+07:00')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.iso8601(*arg0); end

  # Creates a new
  # [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) object by
  # parsing from a string according to some typical RFC 3339 formats.
  #
  # ```ruby
  # DateTime.rfc3339('2001-02-03T04:05:06+07:00')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.rfc3339(*arg0); end

  # Creates a new
  # [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) object by
  # parsing from a string according to some typical
  # [`XML`](https://docs.ruby-lang.org/en/2.7.0/XML.html) Schema formats.
  #
  # ```ruby
  # DateTime.xmlschema('2001-02-03T04:05:06+07:00')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.xmlschema(*arg0); end

  # Creates a new
  # [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) object by
  # parsing from a string according to some typical RFC 2822 formats.
  #
  # ```ruby
  # DateTime.rfc2822('Sat, 3 Feb 2001 04:05:06 +0700')
  #                          #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.rfc2822(*arg0); end

  # Creates a new
  # [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) object by
  # parsing from a string according to some typical RFC 2822 formats.
  #
  # ```ruby
  # DateTime.rfc2822('Sat, 3 Feb 2001 04:05:06 +0700')
  #                          #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.rfc822(*arg0); end

  # Creates a new
  # [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) object by
  # parsing from a string according to some RFC 2616 format.
  #
  # ```ruby
  # DateTime.httpdate('Sat, 03 Feb 2001 04:05:06 GMT')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+00:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.httpdate(*arg0); end

  # Creates a new
  # [`DateTime`](https://docs.ruby-lang.org/en/2.7.0/DateTime.html) object by
  # parsing from a string according to some typical JIS X 0301 formats.
  #
  # ```ruby
  # DateTime.jisx0301('H13.02.03T04:05:06+07:00')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # ```
  #
  # For no-era year, legacy format, Heisei is assumed.
  #
  # ```ruby
  # DateTime.jisx0301('13.02.03T04:05:06+07:00')
  #                           #=> #<DateTime: 2001-02-03T04:05:06+07:00 ...>
  # ```
  sig {params(arg0: T.untyped).returns(T.untyped)}
  def self.jisx0301(*arg0); end
end
