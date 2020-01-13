# typed: __STDLIB_INTERNAL

# [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) is an abstraction of
# dates and times. [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) is
# stored internally as the number of seconds with fraction since the *Epoch*,
# January 1, 1970 00:00 UTC. Also see the library module
# [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html). The
# [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) class treats GMT
# (Greenwich Mean [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html)) and
# UTC (Coordinated Universal
# [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html)) as equivalent. GMT is
# the older way of referring to these baseline times but persists in the names
# of calls on POSIX systems.
#
# All times may have fraction. Be aware of this fact when comparing times with
# each other -- times that are apparently equal when displayed may be different
# when compared.
#
# Since Ruby 1.9.2, [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html)
# implementation uses a signed 63 bit integer, Bignum or
# [`Rational`](https://docs.ruby-lang.org/en/2.6.0/Rational.html). The integer
# is a number of nanoseconds since the *Epoch* which can represent 1823-11-12 to
# 2116-02-20. When Bignum or
# [`Rational`](https://docs.ruby-lang.org/en/2.6.0/Rational.html) is used
# (before 1823, after 2116, under nanosecond),
# [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) works slower as when
# integer is used.
#
# # Examples
#
# All of these examples were done using the EST timezone which is GMT-5.
#
# ## Creating a new [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) instance
#
# You can create a new instance of
# [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) with
# [`Time::new`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-c-new).
# This will use the current system time.
# [`Time::now`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-c-now) is
# an alias for this. You can also pass parts of the time to
# [`Time::new`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-c-new) such
# as year, month, minute, etc. When you want to construct a time this way you
# must pass at least a year. If you pass the year with nothing else time will
# default to January 1 of that year at 00:00:00 with the current system
# timezone. Here are some examples:
#
# ```ruby
# Time.new(2002)         #=> 2002-01-01 00:00:00 -0500
# Time.new(2002, 10)     #=> 2002-10-01 00:00:00 -0500
# Time.new(2002, 10, 31) #=> 2002-10-31 00:00:00 -0500
# ```
#
# You can pass a UTC offset:
#
# ```ruby
# Time.new(2002, 10, 31, 2, 2, 2, "+02:00") #=> 2002-10-31 02:02:02 +0200
# ```
#
# Or a timezone object:
#
# ```ruby
# tz = timezone("Europe/Athens") # Eastern European Time, UTC+2
# Time.new(2002, 10, 31, 2, 2, 2, tz) #=> 2002-10-31 02:02:02 +0200
# ```
#
# You can also use
# [`Time::gm`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-c-gm),
# [`Time::local`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-c-local)
# and [`Time::utc`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-c-utc)
# to infer GMT, local and UTC timezones instead of using the current system
# setting.
#
# You can also create a new time using
# [`Time::at`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-c-at) which
# takes the number of seconds (or fraction of seconds) since the [Unix
# Epoch](http://en.wikipedia.org/wiki/Unix_time).
#
# ```ruby
# Time.at(628232400) #=> 1989-11-28 00:00:00 -0500
# ```
#
# ## Working with an instance of [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html)
#
# Once you have an instance of
# [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) there is a multitude
# of things you can do with it. Below are some examples. For all of the
# following examples, we will work on the assumption that you have done the
# following:
#
# ```ruby
# t = Time.new(1993, 02, 24, 12, 0, 0, "+09:00")
# ```
#
# Was that a monday?
#
# ```ruby
# t.monday? #=> false
# ```
#
# What year was that again?
#
# ```ruby
# t.year #=> 1993
# ```
#
# Was it daylight savings at the time?
#
# ```ruby
# t.dst? #=> false
# ```
#
# What's the day a year later?
#
# ```ruby
# t + (60*60*24*365) #=> 1994-02-24 12:00:00 +0900
# ```
#
# How many seconds was that since the Unix Epoch?
#
# ```ruby
# t.to_i #=> 730522800
# ```
#
# You can also do standard functions like compare two times.
#
# ```ruby
# t1 = Time.new(2010)
# t2 = Time.new(2011)
#
# t1 == t2 #=> false
# t1 == t1 #=> true
# t1 <  t2 #=> true
# t1 >  t2 #=> false
#
# Time.new(2010,10,31).between?(t1, t2) #=> true
# ```
#
# ## Timezone argument
#
# A timezone argument must have `local_to_utc` and `utc_to_local` methods, and
# may have `name` and `abbr` methods.
#
# The `local_to_utc` method should convert a Time-like object from the timezone
# to UTC, and `utc_to_local` is the opposite. The result also should be a
# [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) or Time-like object
# (not necessary to be the same class). The
# [`zone`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-zone) of the
# result is just ignored. Time-like argument to these methods is similar to a
# [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) object in UTC without
# sub-second; it has attribute readers for the parts, e.g.
# [`year`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-year),
# [`month`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-month), and
# so on, and epoch time readers,
# [`to_i`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-to_i). The
# sub-second attributes are fixed as 0, and
# [`utc_offset`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-utc_offset),
# [`zone`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-zone),
# [`isdst`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-isdst), and
# their aliases are same as a
# [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) object in UTC. Also
# [`to_time`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-to_time),
# [`+`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-2B), and
# [`-`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-2D) methods are
# defined.
#
# The `name` method is used for marshaling. If this method is not defined on a
# timezone object, [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html)
# objects using that timezone object can not be dumped by
# [`Marshal`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html).
#
# The `abbr` method is used by '%Z' in
# [`strftime`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-strftime).
#
# ### Auto conversion to Timezone
#
# At loading marshaled data, a timezone name will be converted to a timezone
# object by `find_timezone` class method, if the method is defined.
#
# Similary, that class method will be called when a timezone argument does not
# have the necessary methods mentioned above.
class Time
  # Parses `date` as an HTTP-date defined by RFC 2616 and converts it to a
  # [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) object.
  #
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html) is
  # raised if `date` is not compliant with RFC 2616 or if the
  # [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) class cannot
  # represent specified date.
  #
  # See
  # [`httpdate`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-httpdate)
  # for more information on this format.
  #
  # ```ruby
  # require 'time'
  #
  # Time.httpdate("Thu, 06 Oct 2011 02:26:12 GMT")
  # #=> 2011-10-06 02:26:12 UTC
  # ```
  #
  # You must require 'time' to use this method.
  sig do
    params(
        date: String,
    )
    .returns(Time)
  end
  def self.httpdate(date); end

  # Alias for:
  # [`xmlschema`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-c-xmlschema)
  sig do
    params(
        date: String,
    )
    .returns(Time)
  end
  def self.iso8601(date); end

  # Takes a string representation of a
  # [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) and attempts to
  # parse it using a heuristic.
  #
  # ```ruby
  # require 'time'
  #
  # Time.parse("2010-10-31") #=> 2010-10-31 00:00:00 -0500
  # ```
  #
  # Any missing pieces of the date are inferred based on the current date.
  #
  # ```ruby
  # require 'time'
  #
  # # assuming the current date is "2011-10-31"
  # Time.parse("12:00") #=> 2011-10-31 12:00:00 -0500
  # ```
  #
  # We can change the date used to infer our missing elements by passing a
  # second object that responds to
  # [`mon`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-mon),
  # [`day`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-day) and
  # [`year`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-year), such
  # as [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html),
  # [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) or
  # [`DateTime`](https://docs.ruby-lang.org/en/2.6.0/DateTime.html). We can also
  # use our own object.
  #
  # ```ruby
  # require 'time'
  #
  # class MyDate
  #   attr_reader :mon, :day, :year
  #
  #   def initialize(mon, day, year)
  #     @mon, @day, @year = mon, day, year
  #   end
  # end
  #
  # d  = Date.parse("2010-10-28")
  # t  = Time.parse("2010-10-29")
  # dt = DateTime.parse("2010-10-30")
  # md = MyDate.new(10,31,2010)
  #
  # Time.parse("12:00", d)  #=> 2010-10-28 12:00:00 -0500
  # Time.parse("12:00", t)  #=> 2010-10-29 12:00:00 -0500
  # Time.parse("12:00", dt) #=> 2010-10-30 12:00:00 -0500
  # Time.parse("12:00", md) #=> 2010-10-31 12:00:00 -0500
  # ```
  #
  # If a block is given, the year described in `date` is converted by the block.
  # This is specifically designed for handling two digit years. For example, if
  # you wanted to treat all two digit years prior to 70 as the year 2000+ you
  # could write this:
  #
  # ```ruby
  # require 'time'
  #
  # Time.parse("01-10-31") {|year| year + (year < 70 ? 2000 : 1900)}
  # #=> 2001-10-31 00:00:00 -0500
  # Time.parse("70-10-31") {|year| year + (year < 70 ? 2000 : 1900)}
  # #=> 1970-10-31 00:00:00 -0500
  # ```
  #
  # If the upper components of the given time are broken or missing, they are
  # supplied with those of `now`. For the lower components, the minimum values
  # (1 or 0) are assumed if broken or missing. For example:
  #
  # ```ruby
  # require 'time'
  #
  # # Suppose it is "Thu Nov 29 14:33:20 2001" now and
  # # your time zone is EST which is GMT-5.
  # now = Time.parse("Thu Nov 29 14:33:20 2001")
  # Time.parse("16:30", now)     #=> 2001-11-29 16:30:00 -0500
  # Time.parse("7/23", now)      #=> 2001-07-23 00:00:00 -0500
  # Time.parse("Aug 31", now)    #=> 2001-08-31 00:00:00 -0500
  # Time.parse("Aug 2000", now)  #=> 2000-08-01 00:00:00 -0500
  # ```
  #
  # Since there are numerous conflicts among locally defined time zone
  # abbreviations all over the world, this method is not intended to understand
  # all of them. For example, the abbreviation "CST" is used variously as:
  #
  # ```
  # -06:00 in America/Chicago,
  # -05:00 in America/Havana,
  # +08:00 in Asia/Harbin,
  # +09:30 in Australia/Darwin,
  # +10:30 in Australia/Adelaide,
  # etc.
  # ```
  #
  # Based on this fact, this method only understands the time zone abbreviations
  # described in RFC 822 and the system time zone, in the order named. (i.e. a
  # definition in RFC 822 overrides the system time zone definition.)  The
  # system time zone is taken from `Time.local(year, 1, 1).zone` and
  # `Time.local(year, 7, 1).zone`. If the extracted time zone abbreviation does
  # not match any of them, it is ignored and the given time is regarded as a
  # local time.
  #
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html) is
  # raised if [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html).\_parse
  # cannot extract information from `date` or if the
  # [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) class cannot
  # represent specified date.
  #
  # This method can be used as a fail-safe for other parsing methods as:
  #
  # ```ruby
  # Time.rfc2822(date) rescue Time.parse(date)
  # Time.httpdate(date) rescue Time.parse(date)
  # Time.xmlschema(date) rescue Time.parse(date)
  # ```
  #
  # A failure of
  # [`Time.parse`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-c-parse)
  # should be checked, though.
  #
  # You must require 'time' to use this method.
  sig do
    params(
        date: String,
        now: Time,
    )
    .returns(Time)
  end
  def self.parse(date, now=T.unsafe(nil)); end

  # Parses `date` as date-time defined by RFC 2822 and converts it to a
  # [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) object. The format
  # is identical to the date format defined by RFC 822 and updated by RFC 1123.
  #
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html) is
  # raised if `date` is not compliant with RFC 2822 or if the
  # [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) class cannot
  # represent specified date.
  #
  # See
  # [`rfc2822`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-rfc2822)
  # for more information on this format.
  #
  # ```ruby
  # require 'time'
  #
  # Time.rfc2822("Wed, 05 Oct 2011 22:26:12 -0400")
  # #=> 2010-10-05 22:26:12 -0400
  # ```
  #
  # You must require 'time' to use this method.
  #
  # Also aliased as:
  # [`rfc822`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-c-rfc822)
  sig do
    params(
        date: String,
    )
    .returns(Time)
  end
  def self.rfc2822(date); end

  # Alias for:
  # [`rfc2822`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-c-rfc2822)
  sig do
    params(
        date: String,
    )
    .returns(Time)
  end
  def self.rfc822(date); end

  # Works similar to `parse` except that instead of using a heuristic to detect
  # the format of the input string, you provide a second argument that describes
  # the format of the string.
  #
  # If a block is given, the year described in `date` is converted by the block.
  # For example:
  #
  # ```
  # Time.strptime(...) {|y| y < 100 ? (y >= 69 ? y + 1900 : y + 2000) : y}
  # ```
  #
  # Below is a list of the formatting options:
  #
  # %a
  # :   The abbreviated weekday name ("Sun")
  # %A
  # :   The  full  weekday  name ("Sunday")
  # %b
  # :   The abbreviated month name ("Jan")
  # %B
  # :   The  full  month  name ("January")
  # %c
  # :   The preferred local date and time representation
  # %C
  # :   Century (20 in 2009)
  # %d
  # :   Day of the month (01..31)
  # %D
  # :   [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html) (%m/%d/%y)
  # %e
  # :   Day of the month, blank-padded ( 1..31)
  # %F
  # :   Equivalent to %Y-%m-%d (the ISO 8601 date format)
  # %h
  # :   Equivalent to %b
  # %H
  # :   Hour of the day, 24-hour clock (00..23)
  # %I
  # :   Hour of the day, 12-hour clock (01..12)
  # %j
  # :   Day of the year (001..366)
  # %k
  # :   hour, 24-hour clock, blank-padded ( 0..23)
  # %l
  # :   hour, 12-hour clock, blank-padded ( 0..12)
  # %L
  # :   Millisecond of the second (000..999)
  # %m
  # :   Month of the year (01..12)
  # %M
  # :   Minute of the hour (00..59)
  # %n
  # :   Newline (n)
  # %N
  # :   Fractional seconds digits
  # %p
  # :   Meridian indicator ("AM" or "PM")
  # %P
  # :   Meridian indicator ("am" or "pm")
  # %Q
  # :   Number of milliseconds since 1970-01-01 00:00:00 UTC.
  # %r
  # :   time, 12-hour (same as %I:%M:%S %p)
  # %R
  # :   time, 24-hour (%H:%M)
  # %s
  # :   Number of seconds since 1970-01-01 00:00:00 UTC.
  # %S
  # :   Second of the minute (00..60)
  # %t
  # :   Tab character (t)
  # %T
  # :   time, 24-hour (%H:%M:%S)
  # %u
  # :   Day of the week as a decimal, Monday being 1. (1..7)
  # %U
  # :   Week number of the current year, starting with the first Sunday as the
  #     first day of the first week (00..53)
  # %v
  # :   VMS date (%e-%b-%Y)
  # %V
  # :   Week number of year according to ISO 8601 (01..53)
  # %W
  # :   Week  number  of the current year, starting with the first Monday as the
  #     first day of the first week (00..53)
  # %w
  # :   Day of the week (Sunday is 0, 0..6)
  # %x
  # :   Preferred representation for the date alone, no time
  # %X
  # :   Preferred representation for the time alone, no date
  # %y
  # :   Year without a century (00..99)
  # %Y
  # :   Year which may include century, if provided
  # %z
  # :   [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) zone as  hour
  #     offset from UTC (e.g. +0900)
  # %Z
  # :   [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) zone name
  # %%
  # :   Literal "%" character
  # %+
  # :   date(1) (%a %b %e %H:%M:%S %Z %Y)
  #
  #
  # ```ruby
  # require 'time'
  #
  # Time.strptime("2000-10-31", "%Y-%m-%d") #=> 2000-10-31 00:00:00 -0500
  # ```
  #
  # You must require 'time' to use this method.
  sig do
    params(
        date: String,
        format: String,
        now: Time,
    )
    .returns(Time)
  end
  def self.strptime(date, format, now=T.unsafe(nil)); end

  # Parses `date` as a dateTime defined by the
  # [`XML`](https://docs.ruby-lang.org/en/2.6.0/XML.html) Schema and converts it
  # to a [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) object. The
  # format is a restricted version of the format defined by ISO 8601.
  #
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html) is
  # raised if `date` is not compliant with the format or if the
  # [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) class cannot
  # represent specified date.
  #
  # See
  # [`xmlschema`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-xmlschema)
  # for more information on this format.
  #
  # ```ruby
  # require 'time'
  #
  # Time.xmlschema("2011-10-05T22:26:12-04:00")
  # #=> 2011-10-05 22:26:12-04:00
  # ```
  #
  # You must require 'time' to use this method.
  #
  # Also aliased as:
  # [`iso8601`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-c-iso8601)
  sig do
    params(
        date: String,
    )
    .returns(Time)
  end
  def self.xmlschema(date); end

  # Return the number of seconds the specified time zone differs from UTC.
  #
  # [`Numeric`](https://docs.ruby-lang.org/en/2.6.0/Numeric.html) time zones
  # that include minutes, such as `-10:00` or `+1330` will work, as will simpler
  # hour-only time zones like `-10` or `+13`.
  #
  # Textual time zones listed in ZoneOffset are also supported.
  #
  # If the time zone does not match any of the above, `zone_offset` will check
  # if the local time zone (both with and without potential Daylight Saving
  # [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) changes being in
  # effect) matches `zone`. Specifying a value for `year` will change the year
  # used to find the local time zone.
  #
  # If `zone_offset` is unable to determine the offset, nil will be returned.
  #
  # ```ruby
  # require 'time'
  #
  # Time.zone_offset("EST") #=> -18000
  # ```
  #
  # You must require 'time' to use this method.
  sig do
    params(
        zone: String,
    )
    .returns(Time)
  end
  def self.zone_offset(zone); end

  # Returns a string which represents the time as RFC 1123 date of HTTP-date
  # defined by RFC 2616:
  #
  # ```
  # day-of-week, DD month-name CCYY hh:mm:ss GMT
  # ```
  #
  # Note that the result is always UTC (GMT).
  #
  # ```ruby
  # require 'time'
  #
  # t = Time.now
  # t.httpdate # => "Thu, 06 Oct 2011 02:26:12 GMT"
  # ```
  #
  # You must require 'time' to use this method.
  sig {returns(String)}
  def httpdate(); end

  # Alias for:
  # [`xmlschema`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-xmlschema)
  sig do
    params(
        fraction_digits: T.any(Integer, String),
    )
    .returns(String)
  end
  def iso8601(fraction_digits=0); end

  # Returns a string which represents the time as date-time defined by RFC 2822:
  #
  # ```
  # day-of-week, DD month-name CCYY hh:mm:ss zone
  # ```
  #
  # where zone is [+-]hhmm.
  #
  # If `self` is a UTC time, -0000 is used as zone.
  #
  # ```ruby
  # require 'time'
  #
  # t = Time.now
  # t.rfc2822  # => "Wed, 05 Oct 2011 22:26:12 -0400"
  # ```
  #
  # You must require 'time' to use this method.
  #
  # Also aliased as:
  # [`rfc822`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-rfc822)
  sig {returns(String)}
  def rfc2822(); end

  # Alias for:
  # [`rfc2822`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-rfc2822)
  sig {returns(String)}
  def rfc822(); end

  # Returns a string which represents the time as a dateTime defined by
  # [`XML`](https://docs.ruby-lang.org/en/2.6.0/XML.html) Schema:
  #
  # ```
  # CCYY-MM-DDThh:mm:ssTZD
  # CCYY-MM-DDThh:mm:ss.sssTZD
  # ```
  #
  # where TZD is Z or [+-]hh:mm.
  #
  # If self is a UTC time, Z is used as TZD. [+-]hh:mm is used otherwise.
  #
  # `fractional_digits` specifies a number of digits to use for fractional
  # seconds. Its default value is 0.
  #
  # ```ruby
  # require 'time'
  #
  # t = Time.now
  # t.iso8601  # => "2011-10-05T22:26:12-04:00"
  # ```
  #
  # You must require 'time' to use this method.
  #
  # Also aliased as:
  # [`iso8601`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-iso8601)
  sig do
    params(
        fraction_digits: T.any(Integer, String),
    )
    .returns(String)
  end
  def xmlschema(fraction_digits=0); end

  # Returns self.
  sig {returns(Time)}
  def to_time(); end

  # Returns a [`Date`](https://docs.ruby-lang.org/en/2.6.0/Date.html) object
  # which denotes self.
  sig {returns(Date)}
  def to_date(); end

  # Returns a [`DateTime`](https://docs.ruby-lang.org/en/2.6.0/DateTime.html)
  # object which denotes self.
  sig {returns(DateTime)}
  def to_datetime(); end
end
