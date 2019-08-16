# typed: __STDLIB_INTERNAL

# [Time](Time) is an abstraction of dates and times.
# [Time](Time) is stored internally as the number of
# seconds with fraction since the *Epoch* , January 1, 1970 00:00 UTC.
# Also see the library module Date. The [Time](Time)
# class treats GMT (Greenwich Mean [Time](Time) ) and
# UTC (Coordinated Universal [Time](Time) ) as
# equivalent. GMT is the older way of referring to these baseline times
# but persists in the names of calls on POSIX systems.
#
# All times may have fraction. Be aware of this fact when comparing times
# with each other – times that are apparently equal when displayed may be
# different when compared.
#
# Since Ruby 1.9.2, [Time](Time) implementation uses a
# signed 63 bit integer, Bignum or
# [Rational](https://ruby-doc.org/core-2.6.3/Rational.html) . The integer
# is a number of nanoseconds since the *Epoch* which can represent
# 1823-11-12 to 2116-02-20. When Bignum or
# [Rational](https://ruby-doc.org/core-2.6.3/Rational.html) is used
# (before 1823, after 2116, under nanosecond),
# [Time](Time) works slower as when integer is used.
#
#
# All of these examples were done using the EST timezone which is GMT-5.
#
#
# You can create a new instance of [Time](Time) with
# [::new](Time#method-c-new) . This will use the
# current system time. [::now](Time#method-c-now) is
# an alias for this. You can also pass parts of the time to
# [::new](Time#method-c-new) such as year, month,
# minute, etc. When you want to construct a time this way you must pass at
# least a year. If you pass the year with nothing else time will default
# to January 1 of that year at 00:00:00 with the current system timezone.
# Here are some examples:
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
# You can also use [::gm](Time#method-c-gm) ,
# [::local](Time#method-c-local) and
# [::utc](Time#method-c-utc) to infer GMT, local and
# UTC timezones instead of using the current system setting.
#
# You can also create a new time using
# [::at](Time#method-c-at) which takes the number of
# seconds (or fraction of seconds) since the [Unix
# Epoch](http://en.wikipedia.org/wiki/Unix_time) .
#
# ```ruby
# Time.at(628232400) #=> 1989-11-28 00:00:00 -0500
# ```
#
#
# Once you have an instance of [Time](Time) there is a
# multitude of things you can do with it. Below are some examples. For all
# of the following examples, we will work on the assumption that you have
# done the following:
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
# What’s the day a year later?
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
#
# A timezone argument must have `local_to_utc` and `utc_to_local` methods,
# and may have `name` and `abbr` methods.
#
# The `local_to_utc` method should convert a Time-like object from the
# timezone to UTC, and `utc_to_local` is the opposite. The result also
# should be a [Time](Time) or Time-like object (not
# necessary to be the same class). The
# [zone](Time#method-i-zone) of the result is just
# ignored. Time-like argument to these methods is similar to a
# [Time](Time) object in UTC without sub-second; it
# has attribute readers for the parts, e.g.
# [year](Time#method-i-year) ,
# [month](Time#method-i-month) , and so on, and epoch
# time readers, [to\_i](Time#method-i-to_i) . The
# sub-second attributes are fixed as 0, and
# [utc\_offset](Time#method-i-utc_offset) ,
# [zone](Time#method-i-zone) ,
# [isdst](Time#method-i-isdst) , and their aliases are
# same as a [Time](Time) object in UTC. Also to\_time,
# \#+, and \#- methods are defined.
#
# The `name` method is used for marshaling. If this method is not defined
# on a timezone object, [Time](Time) objects using
# that timezone object can not be dumped by
# [Marshal](https://ruby-doc.org/core-2.6.3/Marshal.html) .
#
# The `abbr` method is used by ‘%Z’ in
# [strftime](Time#method-i-strftime) .
#
#
# At loading marshaled data, a timezone name will be converted to a
# timezone object by `find_timezone` class method, if the method is
# defined.
#
# Similary, that class method will be called when a timezone argument does
# not have the necessary methods mentioned above.
class Time < Object
  include Comparable

  RFC2822_DAY_NAME = T.let(T.unsafe(nil), Array)
  RFC2822_MONTH_NAME = T.let(T.unsafe(nil), Array)

  # Creates a new [Time](Time.downloaded.ruby_doc) object with the value
  # given by `time`, the given number of `seconds_with_frac`, or `seconds`
  # and `microseconds_with_frac` since the Epoch. `seconds_with_frac` and
  # `microseconds_with_frac` can be an
  # [Integer](https://ruby-doc.org/core-2.6.3/Integer.html) ,
  # [Float](https://ruby-doc.org/core-2.6.3/Float.html) ,
  # [Rational](https://ruby-doc.org/core-2.6.3/Rational.html) , or other
  # [Numeric](https://ruby-doc.org/core-2.6.3/Numeric.html) . non-portable
  # feature allows the offset to be negative on some systems.
  #
  # If `in` argument is given, the result is in that timezone or UTC offset,
  # or if a numeric argument is given, the result is in local time.
  #
  # ```ruby
  # Time.at(0)                                #=> 1969-12-31 18:00:00 -0600
  # Time.at(Time.at(0))                       #=> 1969-12-31 18:00:00 -0600
  # Time.at(946702800)                        #=> 1999-12-31 23:00:00 -0600
  # Time.at(-284061600)                       #=> 1960-12-31 00:00:00 -0600
  # Time.at(946684800.2).usec                 #=> 200000
  # Time.at(946684800, 123456.789).nsec       #=> 123456789
  # Time.at(946684800, 123456789, :nsec).nsec #=> 123456789
  # ```
  sig do
    params(
        seconds: T.any(Time, Numeric)
    )
    .returns(Time)
  end
  sig do
    params(
        seconds: Numeric,
        microseconds_with_frac: Numeric,
    )
    .returns(Time)
  end
  def self.at(seconds, microseconds_with_frac=T.unsafe(nil)); end

  # Creates a [Time](Time.downloaded.ruby_doc) object based on given values,
  # interpreted as UTC (GMT). The year must be specified. Other values
  # default to the minimum value for that field (and may be `nil` or
  # omitted). Months may be specified by numbers from 1 to 12, or by the
  # three-letter English month names. Hours are specified on a 24-hour clock
  # (0..23). Raises an
  # [ArgumentError](https://ruby-doc.org/core-2.6.3/ArgumentError.html) if
  # any values are out of range. Will also accept ten arguments in the order
  # output by [\#to\_a](Time.downloaded.ruby_doc#method-i-to_a) .
  #
  # `sec_with_frac` and `usec_with_frac` can have a fractional part.
  #
  # ```ruby
  # Time.utc(2000,"jan",1,20,15,1)  #=> 2000-01-01 20:15:01 UTC
  # Time.gm(2000,"jan",1,20,15,1)   #=> 2000-01-01 20:15:01 UTC
  # ```
  sig do
    params(
        year: Integer,
        month: T.any(Integer, String),
        day: Integer,
        hour: Integer,
        min: Integer,
        sec: Numeric,
        usec_with_frac: Numeric,
    )
    .returns(Time)
  end
  def self.gm(year, month=T.unsafe(nil), day=T.unsafe(nil), hour=T.unsafe(nil), min=T.unsafe(nil), sec=T.unsafe(nil), usec_with_frac=T.unsafe(nil)); end

  # Same as [::gm](Time.downloaded.ruby_doc#method-c-gm) , but interprets
  # the values in the local time zone.
  #
  # ```ruby
  # Time.local(2000,"jan",1,20,15,1)   #=> 2000-01-01 20:15:01 -0600
  # ```
  sig do
    params(
        year: Integer,
        month: T.any(Integer, String),
        day: Integer,
        hour: Integer,
        min: Integer,
        sec: Numeric,
        usec_with_frac: Numeric,
    )
    .returns(Time)
  end
  def self.local(year, month=T.unsafe(nil), day=T.unsafe(nil), hour=T.unsafe(nil), min=T.unsafe(nil), sec=T.unsafe(nil), usec_with_frac=T.unsafe(nil)); end

  # Creates a new [Time](Time.downloaded.ruby_doc) object for the current
  # time. This is same as [::new](Time.downloaded.ruby_doc#method-c-new)
  # without arguments.
  #
  # ```ruby
  # Time.now            #=> 2009-06-24 12:39:54 +0900
  # ```
  sig {returns(Time)}
  def self.now(); end

  # Creates a [Time](Time.downloaded.ruby_doc) object based on given values,
  # interpreted as UTC (GMT). The year must be specified. Other values
  # default to the minimum value for that field (and may be `nil` or
  # omitted). Months may be specified by numbers from 1 to 12, or by the
  # three-letter English month names. Hours are specified on a 24-hour clock
  # (0..23). Raises an
  # [ArgumentError](https://ruby-doc.org/core-2.6.3/ArgumentError.html) if
  # any values are out of range. Will also accept ten arguments in the order
  # output by [\#to\_a](Time.downloaded.ruby_doc#method-i-to_a) .
  #
  # `sec_with_frac` and `usec_with_frac` can have a fractional part.
  #
  # ```ruby
  # Time.utc(2000,"jan",1,20,15,1)  #=> 2000-01-01 20:15:01 UTC
  # Time.gm(2000,"jan",1,20,15,1)   #=> 2000-01-01 20:15:01 UTC
  # ```
  sig do
    params(
        year: Integer,
        month: T.any(Integer, String),
        day: Integer,
        hour: Integer,
        min: Integer,
        sec: Numeric,
        usec_with_frac: Numeric,
    )
    .returns(Time)
  end
  def self.utc(year, month=T.unsafe(nil), day=T.unsafe(nil), hour=T.unsafe(nil), min=T.unsafe(nil), sec=T.unsafe(nil), usec_with_frac=T.unsafe(nil)); end

  # Addition — Adds some number of seconds (possibly fractional) to *time*
  # and returns that value as a new [Time](Time.downloaded.ruby_doc) object.
  #
  # ```ruby
  # t = Time.now         #=> 2007-11-19 08:22:21 -0600
  # t + (60 * 60 * 24)   #=> 2007-11-20 08:22:21 -0600
  # ```
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Time)
  end
  def +(arg0); end

  sig do
    params(
        arg0: Time,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Time)
  end
  def -(arg0); end

  sig do
    params(
        arg0: Time,
    )
    .returns(T::Boolean)
  end
  def <(arg0); end

  sig do
    params(
        arg0: Time,
    )
    .returns(T::Boolean)
  end
  def <=(arg0); end

  # Comparison—Compares `time` with `other_time` .
  #
  # \-1, 0, +1 or nil depending on whether `time` is less than, equal to, or
  # greater than `other_time` .
  #
  # `nil` is returned if the two values are incomparable.
  #
  # ```ruby
  # t = Time.now       #=> 2007-11-19 08:12:12 -0600
  # t2 = t + 2592000   #=> 2007-12-19 08:12:12 -0600
  # t <=> t2           #=> -1
  # t2 <=> t           #=> 1
  #
  # t = Time.now       #=> 2007-11-19 08:13:38 -0600
  # t2 = t + 0.1       #=> 2007-11-19 08:13:38 -0600
  # t.nsec             #=> 98222999
  # t2.nsec            #=> 198222999
  # t <=> t2           #=> -1
  # t2 <=> t           #=> 1
  # t <=> t            #=> 0
  # ```
  sig do
    params(
        other: Time,
    )
    .returns(T.nilable(Integer))
  end
  def <=>(other); end

  sig do
    params(
        arg0: Time,
    )
    .returns(T::Boolean)
  end
  def >(arg0); end

  sig do
    params(
        arg0: Time,
    )
    .returns(T::Boolean)
  end
  def >=(arg0); end

  # Returns a canonical string representation of *time* .
  #
  # ```ruby
  # Time.now.asctime   #=> "Wed Apr  9 08:56:03 2003"
  # Time.now.ctime     #=> "Wed Apr  9 08:56:03 2003"
  # ```
  sig {returns(String)}
  def asctime(); end

  # Returns a canonical string representation of *time* .
  #
  # ```ruby
  # Time.now.asctime   #=> "Wed Apr  9 08:56:03 2003"
  # Time.now.ctime     #=> "Wed Apr  9 08:56:03 2003"
  # ```
  sig {returns(String)}
  def ctime(); end

  # Returns the day of the month (1..n) for *time* .
  #
  # ```ruby
  # t = Time.now   #=> 2007-11-19 08:27:03 -0600
  # t.day          #=> 19
  # t.mday         #=> 19
  # ```
  sig {returns(Integer)}
  def day(); end

  # Returns `true` if *time* occurs during Daylight Saving
  # [Time](Time.downloaded.ruby_doc) in its time zone.
  #
  # ```ruby
  # # CST6CDT:
  #   Time.local(2000, 1, 1).zone    #=> "CST"
  #   Time.local(2000, 1, 1).isdst   #=> false
  #   Time.local(2000, 1, 1).dst?    #=> false
  #   Time.local(2000, 7, 1).zone    #=> "CDT"
  #   Time.local(2000, 7, 1).isdst   #=> true
  #   Time.local(2000, 7, 1).dst?    #=> true
  #
  # # Asia/Tokyo:
  #   Time.local(2000, 1, 1).zone    #=> "JST"
  #   Time.local(2000, 1, 1).isdst   #=> false
  #   Time.local(2000, 1, 1).dst?    #=> false
  #   Time.local(2000, 7, 1).zone    #=> "JST"
  #   Time.local(2000, 7, 1).isdst   #=> false
  #   Time.local(2000, 7, 1).dst?    #=> false
  # ```
  sig {returns(T::Boolean)}
  def dst?(); end

  # Returns `true` if *time* and `other_time` are both
  # [Time](Time.downloaded.ruby_doc) objects with the same seconds and
  # fractional seconds.
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def eql?(arg0); end

  # Returns `true` if *time* represents Friday.
  #
  # ```ruby
  # t = Time.local(1987, 12, 18)     #=> 1987-12-18 00:00:00 -0600
  # t.friday?                        #=> true
  # ```
  sig {returns(T::Boolean)}
  def friday?(); end

  # Returns a new [Time](Time.downloaded.ruby_doc) object representing
  # *time* in UTC.
  #
  # ```ruby
  # t = Time.local(2000,1,1,20,15,1)   #=> 2000-01-01 20:15:01 -0600
  # t.gmt?                             #=> false
  # y = t.getgm                        #=> 2000-01-02 02:15:01 UTC
  # y.gmt?                             #=> true
  # t == y                             #=> true
  # ```
  sig {returns(Time)}
  def getgm(); end

  # Returns a new [Time](Time.downloaded.ruby_doc) object representing
  # *time* in local time (using the local time zone in effect for this
  # process).
  #
  # If `utc_offset` is given, it is used instead of the local time.
  # `utc_offset` can be given as a human-readable string (eg. `"+09:00"` )
  # or as a number of seconds (eg. `32400` ).
  #
  # ```ruby
  # t = Time.utc(2000,1,1,20,15,1)  #=> 2000-01-01 20:15:01 UTC
  # t.utc?                          #=> true
  #
  # l = t.getlocal                  #=> 2000-01-01 14:15:01 -0600
  # l.utc?                          #=> false
  # t == l                          #=> true
  #
  # j = t.getlocal("+09:00")        #=> 2000-01-02 05:15:01 +0900
  # j.utc?                          #=> false
  # t == j                          #=> true
  #
  # k = t.getlocal(9*60*60)         #=> 2000-01-02 05:15:01 +0900
  # k.utc?                          #=> false
  # t == k                          #=> true
  # ```
  sig do
    params(
        utc_offset: Integer,
    )
    .returns(Time)
  end
  def getlocal(utc_offset=T.unsafe(nil)); end

  # Returns a new [Time](Time.downloaded.ruby_doc) object representing
  # *time* in UTC.
  #
  # ```ruby
  # t = Time.local(2000,1,1,20,15,1)   #=> 2000-01-01 20:15:01 -0600
  # t.gmt?                             #=> false
  # y = t.getgm                        #=> 2000-01-02 02:15:01 UTC
  # y.gmt?                             #=> true
  # t == y                             #=> true
  # ```
  sig {returns(Time)}
  def getutc(); end

  # Returns `true` if *time* represents a time in UTC (GMT).
  #
  # ```ruby
  # t = Time.now                        #=> 2007-11-19 08:15:23 -0600
  # t.utc?                              #=> false
  # t = Time.gm(2000,"jan",1,20,15,1)   #=> 2000-01-01 20:15:01 UTC
  # t.utc?                              #=> true
  #
  # t = Time.now                        #=> 2007-11-19 08:16:03 -0600
  # t.gmt?                              #=> false
  # t = Time.gm(2000,1,1,20,15,1)       #=> 2000-01-01 20:15:01 UTC
  # t.gmt?                              #=> true
  # ```
  sig {returns(T::Boolean)}
  def gmt?(); end

  # Returns the offset in seconds between the timezone of *time* and UTC.
  #
  # ```ruby
  # t = Time.gm(2000,1,1,20,15,1)   #=> 2000-01-01 20:15:01 UTC
  # t.gmt_offset                    #=> 0
  # l = t.getlocal                  #=> 2000-01-01 14:15:01 -0600
  # l.gmt_offset                    #=> -21600
  # ```
  sig {returns(Integer)}
  def gmt_offset(); end

  # Converts *time* to UTC (GMT), modifying the receiver.
  #
  # ```ruby
  # t = Time.now   #=> 2007-11-19 08:18:31 -0600
  # t.gmt?         #=> false
  # t.gmtime       #=> 2007-11-19 14:18:31 UTC
  # t.gmt?         #=> true
  #
  # t = Time.now   #=> 2007-11-19 08:18:51 -0600
  # t.utc?         #=> false
  # t.utc          #=> 2007-11-19 14:18:51 UTC
  # t.utc?         #=> true
  # ```
  sig {returns(Time)}
  def gmtime(); end

  # Returns a hash code for this [Time](Time.downloaded.ruby_doc) object.
  #
  # See also Object\#hash.
  sig {returns(Integer)}
  def hash(); end

  # Returns the hour of the day (0..23) for *time* .
  #
  # ```ruby
  # t = Time.now   #=> 2007-11-19 08:26:20 -0600
  # t.hour         #=> 8
  # ```
  sig {returns(Integer)}
  def hour(); end

  sig do
    params(
        year: T.any(Integer, String),
        month: T.any(Integer, String),
        day: T.any(Integer, String),
        hour: T.any(Integer, String),
        min: T.any(Integer, String),
        sec: T.any(Numeric, String),
        usec_with_frac: T.any(Numeric, String),
    )
    .void
  end
  def initialize(year=T.unsafe(nil), month=T.unsafe(nil), day=T.unsafe(nil), hour=T.unsafe(nil), min=T.unsafe(nil), sec=T.unsafe(nil), usec_with_frac=T.unsafe(nil)); end

  # Returns a string representing *time* . Equivalent to calling
  # [strftime](Time.downloaded.ruby_doc#method-i-strftime) with the
  # appropriate format string.
  #
  # ```ruby
  # t = Time.now
  # t.to_s                              #=> "2012-11-10 18:16:12 +0100"
  # t.strftime "%Y-%m-%d %H:%M:%S %z"   #=> "2012-11-10 18:16:12 +0100"
  #
  # t.utc.to_s                          #=> "2012-11-10 17:16:12 UTC"
  # t.strftime "%Y-%m-%d %H:%M:%S UTC"  #=> "2012-11-10 17:16:12 UTC"
  # ```
  sig {returns(String)}
  def inspect(); end

  # Returns `true` if *time* occurs during Daylight Saving
  # [Time](Time.downloaded.ruby_doc) in its time zone.
  #
  # ```ruby
  # # CST6CDT:
  #   Time.local(2000, 1, 1).zone    #=> "CST"
  #   Time.local(2000, 1, 1).isdst   #=> false
  #   Time.local(2000, 1, 1).dst?    #=> false
  #   Time.local(2000, 7, 1).zone    #=> "CDT"
  #   Time.local(2000, 7, 1).isdst   #=> true
  #   Time.local(2000, 7, 1).dst?    #=> true
  #
  # # Asia/Tokyo:
  #   Time.local(2000, 1, 1).zone    #=> "JST"
  #   Time.local(2000, 1, 1).isdst   #=> false
  #   Time.local(2000, 1, 1).dst?    #=> false
  #   Time.local(2000, 7, 1).zone    #=> "JST"
  #   Time.local(2000, 7, 1).isdst   #=> false
  #   Time.local(2000, 7, 1).dst?    #=> false
  # ```
  sig {returns(T::Boolean)}
  def isdst(); end

  # Converts *time* to local time (using the local time zone in effect at
  # the creation time of *time* ) modifying the receiver.
  #
  # If `utc_offset` is given, it is used instead of the local time.
  #
  # ```ruby
  # t = Time.utc(2000, "jan", 1, 20, 15, 1) #=> 2000-01-01 20:15:01 UTC
  # t.utc?                                  #=> true
  #
  # t.localtime                             #=> 2000-01-01 14:15:01 -0600
  # t.utc?                                  #=> false
  #
  # t.localtime("+09:00")                   #=> 2000-01-02 05:15:01 +0900
  # t.utc?                                  #=> false
  # ```
  #
  # If `utc_offset` is not given and *time* is local time, just returns the
  # receiver.
  sig do
    params(
        utc_offset: String,
    )
    .returns(Time)
  end
  def localtime(utc_offset=T.unsafe(nil)); end

  # Returns the day of the month (1..n) for *time* .
  #
  # ```ruby
  # t = Time.now   #=> 2007-11-19 08:27:03 -0600
  # t.day          #=> 19
  # t.mday         #=> 19
  # ```
  sig {returns(Integer)}
  def mday(); end

  # Returns the minute of the hour (0..59) for *time* .
  #
  # ```ruby
  # t = Time.now   #=> 2007-11-19 08:25:51 -0600
  # t.min          #=> 25
  # ```
  sig {returns(Integer)}
  def min(); end

  # Returns the month of the year (1..12) for *time* .
  #
  # ```ruby
  # t = Time.now   #=> 2007-11-19 08:27:30 -0600
  # t.mon          #=> 11
  # t.month        #=> 11
  # ```
  sig {returns(Integer)}
  def mon(); end

  # Returns `true` if *time* represents Monday.
  #
  # ```ruby
  # t = Time.local(2003, 8, 4)       #=> 2003-08-04 00:00:00 -0500
  # t.monday?                        #=> true
  # ```
  sig {returns(T::Boolean)}
  def monday?(); end

  # Returns the number of nanoseconds for *time* .
  #
  # ```ruby
  # t = Time.now        #=> 2007-11-17 15:18:03 +0900
  # "%10.9f" % t.to_f   #=> "1195280283.536151409"
  # t.nsec              #=> 536151406
  # ```
  #
  # The lowest digits of [to\_f](Time.downloaded.ruby_doc#method-i-to_f) and
  # [nsec](Time.downloaded.ruby_doc#method-i-nsec) are different because
  # IEEE 754 double is not accurate enough to represent the exact number of
  # nanoseconds since the Epoch.
  #
  # The more accurate value is returned by
  # [nsec](Time.downloaded.ruby_doc#method-i-nsec) .
  sig {returns(Integer)}
  def nsec(); end

  # Rounds sub seconds to a given precision in decimal digits (0 digits by
  # default). It returns a new [Time](Time.downloaded.ruby_doc) object.
  # `ndigits` should be zero or a positive integer.
  #
  # ```ruby
  # require 'time'
  #
  # t = Time.utc(2010,3,30, 5,43,"25.123456789".to_r)
  # t.iso8601(10)           #=> "2010-03-30T05:43:25.1234567890Z"
  # t.round.iso8601(10)     #=> "2010-03-30T05:43:25.0000000000Z"
  # t.round(0).iso8601(10)  #=> "2010-03-30T05:43:25.0000000000Z"
  # t.round(1).iso8601(10)  #=> "2010-03-30T05:43:25.1000000000Z"
  # t.round(2).iso8601(10)  #=> "2010-03-30T05:43:25.1200000000Z"
  # t.round(3).iso8601(10)  #=> "2010-03-30T05:43:25.1230000000Z"
  # t.round(4).iso8601(10)  #=> "2010-03-30T05:43:25.1235000000Z"
  # t.round(5).iso8601(10)  #=> "2010-03-30T05:43:25.1234600000Z"
  # t.round(6).iso8601(10)  #=> "2010-03-30T05:43:25.1234570000Z"
  # t.round(7).iso8601(10)  #=> "2010-03-30T05:43:25.1234568000Z"
  # t.round(8).iso8601(10)  #=> "2010-03-30T05:43:25.1234567900Z"
  # t.round(9).iso8601(10)  #=> "2010-03-30T05:43:25.1234567890Z"
  # t.round(10).iso8601(10) #=> "2010-03-30T05:43:25.1234567890Z"
  #
  # t = Time.utc(1999,12,31, 23,59,59)
  # (t + 0.4).round.iso8601(3)    #=> "1999-12-31T23:59:59.000Z"
  # (t + 0.49).round.iso8601(3)   #=> "1999-12-31T23:59:59.000Z"
  # (t + 0.5).round.iso8601(3)    #=> "2000-01-01T00:00:00.000Z"
  # (t + 1.4).round.iso8601(3)    #=> "2000-01-01T00:00:00.000Z"
  # (t + 1.49).round.iso8601(3)   #=> "2000-01-01T00:00:00.000Z"
  # (t + 1.5).round.iso8601(3)    #=> "2000-01-01T00:00:01.000Z"
  #
  # t = Time.utc(1999,12,31, 23,59,59)
  # (t + 0.123456789).round(4).iso8601(6)  #=> "1999-12-31T23:59:59.123500Z"
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Time)
  end
  def round(arg0); end

  # Returns `true` if *time* represents Saturday.
  #
  # ```ruby
  # t = Time.local(2006, 6, 10)      #=> 2006-06-10 00:00:00 -0500
  # t.saturday?                      #=> true
  # ```
  sig {returns(T::Boolean)}
  def saturday?(); end

  # Returns the second of the minute (0..60) for *time* .
  #
  # **Note:** Seconds range from zero to 60 to allow the system to inject
  # leap seconds. See
  # [en.wikipedia.org/wiki/Leap\_second](http://en.wikipedia.org/wiki/Leap_second)
  # for further details.
  #
  # ```ruby
  # t = Time.now   #=> 2007-11-19 08:25:02 -0600
  # t.sec          #=> 2
  # ```
  sig {returns(Integer)}
  def sec(); end

  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def strftime(arg0); end

  # Returns the fraction for *time* .
  #
  # The return value can be a rational number.
  #
  # ```ruby
  # t = Time.now        #=> 2009-03-26 22:33:12 +0900
  # "%10.9f" % t.to_f   #=> "1238074392.940563917"
  # t.subsec            #=> (94056401/100000000)
  # ```
  #
  # The lowest digits of [to\_f](Time.downloaded.ruby_doc#method-i-to_f) and
  # [subsec](Time.downloaded.ruby_doc#method-i-subsec) are different because
  # IEEE 754 double is not accurate enough to represent the rational number.
  #
  # The more accurate value is returned by
  # [subsec](Time.downloaded.ruby_doc#method-i-subsec) .
  sig {returns(Numeric)}
  def subsec(); end

  # Returns a new [Time](Time.downloaded.ruby_doc) object, one second later
  # than *time* . [\#succ](Time.downloaded.ruby_doc#method-i-succ) is
  # obsolete since 1.9.2 for time is not a discrete value.
  #
  # ```ruby
  # t = Time.now       #=> 2007-11-19 08:23:57 -0600
  # t.succ             #=> 2007-11-19 08:23:58 -0600
  # ```
  #
  # Use instead `time + 1`
  #
  # ```ruby
  # t + 1              #=> 2007-11-19 08:23:58 -0600
  # ```
  sig {returns(Time)}
  def succ(); end

  # Returns `true` if *time* represents Sunday.
  #
  # ```ruby
  # t = Time.local(1990, 4, 1)       #=> 1990-04-01 00:00:00 -0600
  # t.sunday?                        #=> true
  # ```
  sig {returns(T::Boolean)}
  def sunday?(); end

  # Returns `true` if *time* represents Thursday.
  #
  # ```ruby
  # t = Time.local(1995, 12, 21)     #=> 1995-12-21 00:00:00 -0600
  # t.thursday?                      #=> true
  # ```
  sig {returns(T::Boolean)}
  def thursday?(); end

  # Returns a ten-element *array* of values for *time* :
  #
  # ```ruby
  # [sec, min, hour, day, month, year, wday, yday, isdst, zone]
  # ```
  #
  # See the individual methods for an explanation of the valid ranges of
  # each value. The ten elements can be passed directly to
  # [::utc](Time.downloaded.ruby_doc#method-c-utc) or
  # [::local](Time.downloaded.ruby_doc#method-c-local) to create a new
  # [Time](Time.downloaded.ruby_doc) object.
  #
  # ```ruby
  # t = Time.now     #=> 2007-11-19 08:36:01 -0600
  # now = t.to_a     #=> [1, 36, 8, 19, 11, 2007, 1, 323, false, "CST"]
  # ```
  sig {returns([Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer, T::Boolean, String])}
  def to_a(); end

  # Returns the value of *time* as a floating point number of seconds since
  # the Epoch.
  #
  # ```ruby
  # t = Time.now
  # "%10.5f" % t.to_f   #=> "1270968744.77658"
  # t.to_i              #=> 1270968744
  # ```
  #
  # Note that IEEE 754 double is not accurate enough to represent the exact
  # number of nanoseconds since the Epoch.
  sig {returns(Float)}
  def to_f(); end

  # Returns the value of *time* as an integer number of seconds since the
  # Epoch.
  #
  # ```ruby
  # t = Time.now
  # "%10.5f" % t.to_f   #=> "1270968656.89607"
  # t.to_i              #=> 1270968656
  # ```
  sig {returns(Integer)}
  def to_i(); end

  # Returns the value of *time* as a rational number of seconds since the
  # Epoch.
  #
  # ```ruby
  # t = Time.now
  # t.to_r            #=> (1270968792716287611/1000000000)
  # ```
  #
  # This methods is intended to be used to get an accurate value
  # representing the nanoseconds since the Epoch. You can use this method to
  # convert *time* to another Epoch.
  sig {returns(Rational)}
  def to_r(); end

  # Returns a string representing *time* . Equivalent to calling
  # [strftime](Time.downloaded.ruby_doc#method-i-strftime) with the
  # appropriate format string.
  #
  # ```ruby
  # t = Time.now
  # t.to_s                              #=> "2012-11-10 18:16:12 +0100"
  # t.strftime "%Y-%m-%d %H:%M:%S %z"   #=> "2012-11-10 18:16:12 +0100"
  #
  # t.utc.to_s                          #=> "2012-11-10 17:16:12 UTC"
  # t.strftime "%Y-%m-%d %H:%M:%S UTC"  #=> "2012-11-10 17:16:12 UTC"
  # ```
  sig {returns(String)}
  def to_s(); end

  # Returns `true` if *time* represents Tuesday.
  #
  # ```ruby
  # t = Time.local(1991, 2, 19)      #=> 1991-02-19 00:00:00 -0600
  # t.tuesday?                       #=> true
  # ```
  sig {returns(T::Boolean)}
  def tuesday?(); end

  # Returns the number of nanoseconds for *time* .
  #
  # ```ruby
  # t = Time.now        #=> 2007-11-17 15:18:03 +0900
  # "%10.9f" % t.to_f   #=> "1195280283.536151409"
  # t.nsec              #=> 536151406
  # ```
  #
  # The lowest digits of [to\_f](Time.downloaded.ruby_doc#method-i-to_f) and
  # [nsec](Time.downloaded.ruby_doc#method-i-nsec) are different because
  # IEEE 754 double is not accurate enough to represent the exact number of
  # nanoseconds since the Epoch.
  #
  # The more accurate value is returned by
  # [nsec](Time.downloaded.ruby_doc#method-i-nsec) .
  sig {returns(Numeric)}
  def tv_nsec(); end

  # Returns the value of *time* as an integer number of seconds since the
  # Epoch.
  #
  # ```ruby
  # t = Time.now
  # "%10.5f" % t.to_f   #=> "1270968656.89607"
  # t.to_i              #=> 1270968656
  # ```
  sig {returns(Numeric)}
  def tv_sec(); end

  # Returns the number of microseconds for *time* .
  #
  # ```ruby
  # t = Time.now        #=> 2007-11-19 08:03:26 -0600
  # "%10.6f" % t.to_f   #=> "1195481006.775195"
  # t.usec              #=> 775195
  # ```
  sig {returns(Numeric)}
  def tv_usec(); end

  # Returns the number of microseconds for *time* .
  #
  # ```ruby
  # t = Time.now        #=> 2007-11-19 08:03:26 -0600
  # "%10.6f" % t.to_f   #=> "1195481006.775195"
  # t.usec              #=> 775195
  # ```
  sig {returns(Numeric)}
  def usec(); end

  # Converts *time* to UTC (GMT), modifying the receiver.
  #
  # ```ruby
  # t = Time.now   #=> 2007-11-19 08:18:31 -0600
  # t.gmt?         #=> false
  # t.gmtime       #=> 2007-11-19 14:18:31 UTC
  # t.gmt?         #=> true
  #
  # t = Time.now   #=> 2007-11-19 08:18:51 -0600
  # t.utc?         #=> false
  # t.utc          #=> 2007-11-19 14:18:51 UTC
  # t.utc?         #=> true
  # ```
  sig {returns(Time)}
  def utc(); end

  # Returns `true` if *time* represents a time in UTC (GMT).
  #
  # ```ruby
  # t = Time.now                        #=> 2007-11-19 08:15:23 -0600
  # t.utc?                              #=> false
  # t = Time.gm(2000,"jan",1,20,15,1)   #=> 2000-01-01 20:15:01 UTC
  # t.utc?                              #=> true
  #
  # t = Time.now                        #=> 2007-11-19 08:16:03 -0600
  # t.gmt?                              #=> false
  # t = Time.gm(2000,1,1,20,15,1)       #=> 2000-01-01 20:15:01 UTC
  # t.gmt?                              #=> true
  # ```
  sig {returns(T::Boolean)}
  def utc?(); end

  # Returns the offset in seconds between the timezone of *time* and UTC.
  #
  # ```ruby
  # t = Time.gm(2000,1,1,20,15,1)   #=> 2000-01-01 20:15:01 UTC
  # t.gmt_offset                    #=> 0
  # l = t.getlocal                  #=> 2000-01-01 14:15:01 -0600
  # l.gmt_offset                    #=> -21600
  # ```
  sig {returns(Integer)}
  def utc_offset(); end

  # Returns an integer representing the day of the week, 0..6, with Sunday
  # == 0.
  #
  # ```ruby
  # t = Time.now   #=> 2007-11-20 02:35:35 -0600
  # t.wday         #=> 2
  # t.sunday?      #=> false
  # t.monday?      #=> false
  # t.tuesday?     #=> true
  # t.wednesday?   #=> false
  # t.thursday?    #=> false
  # t.friday?      #=> false
  # t.saturday?    #=> false
  # ```
  sig {returns(Integer)}
  def wday(); end

  # Returns `true` if *time* represents Wednesday.
  #
  # ```ruby
  # t = Time.local(1993, 2, 24)      #=> 1993-02-24 00:00:00 -0600
  # t.wednesday?                     #=> true
  # ```
  sig {returns(T::Boolean)}
  def wednesday?(); end

  # Returns an integer representing the day of the year, 1..366.
  #
  # ```ruby
  # t = Time.now   #=> 2007-11-19 08:32:31 -0600
  # t.yday         #=> 323
  # ```
  sig {returns(Integer)}
  def yday(); end

  # Returns the year for *time* (including the century).
  #
  # ```ruby
  # t = Time.now   #=> 2007-11-19 08:27:51 -0600
  # t.year         #=> 2007
  # ```
  sig {returns(Integer)}
  def year(); end

  # Returns the name of the time zone used for *time* . As of Ruby 1.8,
  # returns “UTC” rather than “GMT” for UTC times.
  #
  # ```ruby
  # t = Time.gm(2000, "jan", 1, 20, 15, 1)
  # t.zone   #=> "UTC"
  # t = Time.local(2000, "jan", 1, 20, 15, 1)
  # t.zone   #=> "CST"
  # ```
  sig {returns(String)}
  def zone(); end

  # Same as [::gm](Time.downloaded.ruby_doc#method-c-gm) , but interprets
  # the values in the local time zone.
  #
  # ```ruby
  # Time.local(2000,"jan",1,20,15,1)   #=> 2000-01-01 20:15:01 -0600
  # ```
  sig do
    params(
        year: Integer,
        month: T.any(Integer, String),
        day: Integer,
        hour: Integer,
        min: Integer,
        sec: Numeric,
        usec_with_frac: Numeric,
    )
    .returns(Time)
  end
  def self.mktime(year, month=T.unsafe(nil), day=T.unsafe(nil), hour=T.unsafe(nil), min=T.unsafe(nil), sec=T.unsafe(nil), usec_with_frac=T.unsafe(nil)); end

  # Returns the offset in seconds between the timezone of *time* and UTC.
  #
  # ```ruby
  # t = Time.gm(2000,1,1,20,15,1)   #=> 2000-01-01 20:15:01 UTC
  # t.gmt_offset                    #=> 0
  # l = t.getlocal                  #=> 2000-01-01 14:15:01 -0600
  # l.gmt_offset                    #=> -21600
  # ```
  sig {returns(Integer)}
  def gmtoff(); end

  # Returns the month of the year (1..12) for *time* .
  #
  # ```ruby
  # t = Time.now   #=> 2007-11-19 08:27:30 -0600
  # t.mon          #=> 11
  # t.month        #=> 11
  # ```
  sig {returns(Integer)}
  def month(); end
end
