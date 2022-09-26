# typed: __STDLIB_INTERNAL

# ## Description
#
# The [`Logger`](https://docs.ruby-lang.org/en/2.7.0/Logger.html) class provides
# a simple but sophisticated logging utility that you can use to output
# messages.
#
# The messages have associated levels, such as `INFO` or `ERROR` that indicate
# their importance. You can then give the
# [`Logger`](https://docs.ruby-lang.org/en/2.7.0/Logger.html) a level, and only
# messages at that level or higher will be printed.
#
# The levels are:
#
# `UNKNOWN`
# :   An unknown message that should always be logged.
# `FATAL`
# :   An unhandleable error that results in a program crash.
# `ERROR`
# :   A handleable error condition.
# `WARN`
# :   A warning.
# `INFO`
# :   Generic (useful) information about system operation.
# `DEBUG`
# :   Low-level information for developers.
#
#
# For instance, in a production system, you may have your
# [`Logger`](https://docs.ruby-lang.org/en/2.7.0/Logger.html) set to `INFO` or
# even `WARN`. When you are developing the system, however, you probably want to
# know about the program's internal state, and would set the
# [`Logger`](https://docs.ruby-lang.org/en/2.7.0/Logger.html) to `DEBUG`.
#
# **Note**: [`Logger`](https://docs.ruby-lang.org/en/2.7.0/Logger.html) does not
# escape or sanitize any messages passed to it. Developers should be aware of
# when potentially malicious data (user-input) is passed to
# [`Logger`](https://docs.ruby-lang.org/en/2.7.0/Logger.html), and manually
# escape the untrusted data:
#
# ```ruby
# logger.info("User-input: #{input.dump}")
# logger.info("User-input: %p" % input)
# ```
#
# You can use
# [`formatter=`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#attribute-i-formatter)
# for escaping all data.
#
# ```ruby
# original_formatter = Logger::Formatter.new
# logger.formatter = proc { |severity, datetime, progname, msg|
#   original_formatter.call(severity, datetime, progname, msg.dump)
# }
# logger.info(input)
# ```
#
# ### Example
#
# This creates a [`Logger`](https://docs.ruby-lang.org/en/2.7.0/Logger.html)
# that outputs to the standard output stream, with a level of `WARN`:
#
# ```ruby
# require 'logger'
#
# logger = Logger.new(STDOUT)
# logger.level = Logger::WARN
#
# logger.debug("Created logger")
# logger.info("Program started")
# logger.warn("Nothing to do!")
#
# path = "a_non_existent_file"
#
# begin
#   File.foreach(path) do |line|
#     unless line =~ /^(\w+) = (.*)$/
#       logger.error("Line in wrong format: #{line.chomp}")
#     end
#   end
# rescue => err
#   logger.fatal("Caught exception; exiting")
#   logger.fatal(err)
# end
# ```
#
# Because the Logger's level is set to `WARN`, only the warning, error, and
# fatal messages are recorded. The debug and info messages are silently
# discarded.
#
# ### Features
#
# There are several interesting features that
# [`Logger`](https://docs.ruby-lang.org/en/2.7.0/Logger.html) provides, like
# auto-rolling of log files, setting the format of log messages, and specifying
# a program name in conjunction with the message. The next section shows you how
# to achieve these things.
#
# ## HOWTOs
#
# ### How to create a logger
#
# The options below give you various choices, in more or less increasing
# complexity.
#
# 1.  Create a logger which logs messages to STDERR/STDOUT.
#
# ```ruby
# logger = Logger.new(STDERR)
# logger = Logger.new(STDOUT)
# ```
#
# 2.  Create a logger for the file which has the specified name.
#
# ```ruby
# logger = Logger.new('logfile.log')
# ```
#
# 3.  Create a logger for the specified file.
#
# ```ruby
# file = File.open('foo.log', File::WRONLY | File::APPEND)
# # To create new logfile, add File::CREAT like:
# # file = File.open('foo.log', File::WRONLY | File::APPEND | File::CREAT)
# logger = Logger.new(file)
# ```
#
# 4.  Create a logger which ages the logfile once it reaches a certain size.
#     Leave 10 "old" log files where each file is about 1,024,000 bytes.
#
# ```ruby
# logger = Logger.new('foo.log', 10, 1024000)
# ```
#
# 5.  Create a logger which ages the logfile daily/weekly/monthly.
#
# ```ruby
# logger = Logger.new('foo.log', 'daily')
# logger = Logger.new('foo.log', 'weekly')
# logger = Logger.new('foo.log', 'monthly')
# ```
#
#
# ### How to log a message
#
# Notice the different methods (`fatal`, `error`, `info`) being used to log
# messages of various levels?  Other methods in this family are `warn` and
# `debug`. `add` is used below to log a message of an arbitrary (perhaps
# dynamic) level.
#
# 1.  Message in a block.
#
# ```ruby
# logger.fatal { "Argument 'foo' not given." }
# ```
#
# 2.  Message as a string.
#
# ```ruby
# logger.error "Argument #{@foo} mismatch."
# ```
#
# 3.  With progname.
#
# ```ruby
# logger.info('initialize') { "Initializing..." }
# ```
#
# 4.  With severity.
#
# ```ruby
# logger.add(Logger::FATAL) { 'Fatal error!' }
# ```
#
#
# The block form allows you to create potentially complex log messages, but to
# delay their evaluation until and unless the message is logged. For example, if
# we have the following:
#
# ```ruby
# logger.debug { "This is a " + potentially + " expensive operation" }
# ```
#
# If the logger's level is `INFO` or higher, no debug messages will be logged,
# and the entire block will not even be evaluated. Compare to this:
#
# ```ruby
# logger.debug("This is a " + potentially + " expensive operation")
# ```
#
# Here, the string concatenation is done every time, even if the log level is
# not set to show the debug message.
#
# ### How to close a logger
#
# ```ruby
# logger.close
# ```
#
# ### Setting severity threshold
#
# 1.  Original interface.
#
# ```ruby
# logger.sev_threshold = Logger::WARN
# ```
#
# 2.  Log4r (somewhat) compatible interface.
#
# ```ruby
# logger.level = Logger::INFO
#
# # DEBUG < INFO < WARN < ERROR < FATAL < UNKNOWN
# ```
#
# 3.  [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html) or
#     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) (case
#     insensitive)
#
# ```ruby
# logger.level = :info
# logger.level = 'INFO'
#
# # :debug < :info < :warn < :error < :fatal < :unknown
# ```
#
# 4.  Constructor
#
# ```ruby
# Logger.new(logdev, level: Logger::INFO)
# Logger.new(logdev, level: :info)
# Logger.new(logdev, level: 'INFO')
# ```
#
#
# ## Format
#
# Log messages are rendered in the output stream in a certain format by default.
# The default format and a sample are shown below:
#
# Log format:
#
# ```
# SeverityID, [DateTime #pid] SeverityLabel -- ProgName: message
# ```
#
# Log sample:
#
# ```
# I, [1999-03-03T02:34:24.895701 #19074]  INFO -- Main: info.
# ```
#
# You may change the date and time format via
# [`datetime_format=`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-datetime_format-3D).
#
# ```ruby
# logger.datetime_format = '%Y-%m-%d %H:%M:%S'
#       # e.g. "2004-01-03 00:54:26"
# ```
#
# or via the constructor.
#
# ```ruby
# Logger.new(logdev, datetime_format: '%Y-%m-%d %H:%M:%S')
# ```
#
# Or, you may change the overall format via the
# [`formatter=`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#attribute-i-formatter)
# method.
#
# ```ruby
# logger.formatter = proc do |severity, datetime, progname, msg|
#   "#{datetime}: #{msg}\n"
# end
# # e.g. "2005-09-22 08:51:08 +0900: hello world"
# ```
#
# or via the constructor.
#
# ```ruby
# Logger.new(logdev, formatter: proc {|severity, datetime, progname, msg|
#   "#{datetime}: #{msg}\n"
# })
# ```
class Logger
  include Logger::Severity

  VERSION = T.let(T.unsafe(nil), String)
  ProgName = T.let(T.unsafe(nil), String)

  class Error < RuntimeError; end
  class ShiftingError < Error; end

  # Sorbet doesn't detect that the include also defines the CONSTANTs, so redefine:
  DEBUG = T.let(T.unsafe(nil), Integer)
  INFO = T.let(T.unsafe(nil), Integer)
  WARN = T.let(T.unsafe(nil), Integer)
  ERROR = T.let(T.unsafe(nil), Integer)
  FATAL = T.let(T.unsafe(nil), Integer)
  UNKNOWN = T.let(T.unsafe(nil), Integer)

  # Logging severity threshold (e.g. `Logger::INFO`).
  sig { returns(Integer) }
  def level; end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) logging severity
  # threshold.
  #
  # `severity`
  # :   The Severity of the log message.
  #
  #
  # Also aliased as:
  # [`sev_threshold=`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-sev_threshold-3D)
  sig { params(severity: T.untyped).void }
  def level=(severity); end

  # Program name to include in log messages.
  sig { returns(T.nilable(String)) }
  def progname; end

  # Program name to include in log messages.
  sig { params(progname: T.nilable(String)).void }
  def progname=(progname); end

  # Returns the date format being used. See
  # [`datetime_format=`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-datetime_format-3D)
  sig { returns(T.nilable(String)) }
  def datetime_format; end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) date-time format.
  #
  # `datetime_format`
  # :   A string suitable for passing to `strftime`.
  sig { params(datetime_format: T.nilable(String)).void }
  def datetime_format=(datetime_format); end

  # Logging formatter, as a `Proc` that will take four arguments and return the
  # formatted message. The arguments are:
  #
  # `severity`
  # :   The Severity of the log message.
  # `time`
  # :   A [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) instance
  #     representing when the message was logged.
  # `progname`
  # :   The
  #     [`progname`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#attribute-i-progname)
  #     configured, or passed to the logger method.
  # `msg`
  # :   The *Object* the user passed to the log message; not necessarily a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  #
  #
  # The block should return an
  # [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) that can be
  # written to the logging device via `write`. The default formatter is used
  # when no formatter is set.
  sig { returns(T.untyped) }
  def formatter; end

  # Logging formatter, as a `Proc` that will take four arguments and return the
  # formatted message. The arguments are:
  #
  # `severity`
  # :   The Severity of the log message.
  # `time`
  # :   A [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html) instance
  #     representing when the message was logged.
  # `progname`
  # :   The
  #     [`progname`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#attribute-i-progname)
  #     configured, or passed to the logger method.
  # `msg`
  # :   The *Object* the user passed to the log message; not necessarily a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  #
  #
  # The block should return an
  # [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) that can be
  # written to the logging device via `write`. The default formatter is used
  # when no formatter is set.
  sig { params(formatter: T.untyped).void }
  def formatter=(formatter); end

  # Logging severity threshold (e.g. `Logger::INFO`).
  sig { returns(Integer) }
  def sev_threshold; end

  # Alias for:
  # [`level=`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-level-3D)
  sig { params(severity: T.untyped).void }
  def sev_threshold=(severity); end

  # Returns `true` iff the current severity level allows for the printing of
  # `DEBUG` messages.
  sig { returns(T::Boolean) }
  def debug?; end

  # Returns `true` iff the current severity level allows for the printing of
  # `INFO` messages.
  sig { returns(T::Boolean) }
  def info?; end

  # Returns `true` iff the current severity level allows for the printing of
  # `WARN` messages.
  sig { returns(T::Boolean) }
  def warn?; end

  # Returns `true` iff the current severity level allows for the printing of
  # `ERROR` messages.
  sig { returns(T::Boolean) }
  def error?; end

  # Returns `true` iff the current severity level allows for the printing of
  # `FATAL` messages.
  sig { returns(T::Boolean) }
  def fatal?; end

  sig do
    params(
      logdev: T.any(String, IO, StringIO, NilClass),
      shift_age: Integer,
      shift_size: Integer,
      level: Integer,
      progname: T.nilable(String),
      formatter: T.untyped,
      datetime_format: T.nilable(String),
      shift_period_suffix: T.nilable(String),
      binmode: T.untyped,
    ).void
  end
  def initialize(
    logdev, shift_age = 0, shift_size = 1048576, level: DEBUG,
    progname: nil, formatter: nil, datetime_format: nil,
    shift_period_suffix: '%Y%m%d', binmode: false
  )
    @level = T.let(T.unsafe(nil), T.nilable(Integer))
    @progname = T.let(T.unsafe(nil), T.nilable(String))
    @datetime_format = T.let(T.unsafe(nil), T.nilable(String))
    @formatter = T.let(T.unsafe(nil), T.untyped)
  end

  # ### Args
  #
  # `logdev`
  # :   The log device. This is a filename
  #     ([`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)) or
  #     [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object (typically
  #     `STDOUT`, `STDERR`, or an open file). reopen the same filename if it is
  #     `nil`, do nothing for
  #     [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html). Default is `nil`.
  #
  #
  # ### Description
  #
  # Reopen a log device.
  sig { params(logdev: T.any(NilClass, String, IO, StringIO)).returns(T.self_type) }
  def reopen(logdev = nil); end

  # ### Args
  #
  # `severity`
  # :   Severity. Constants are defined in
  #     [`Logger`](https://docs.ruby-lang.org/en/2.7.0/Logger.html) namespace:
  #     `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`, or `UNKNOWN`.
  # `message`
  # :   The log message. A
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or
  #     [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html).
  # `progname`
  # :   Program name string. Can be omitted. Treated as a message if no
  #     `message` and `block` are given.
  # `block`
  # :   Can be omitted. Called to get a message string if `message` is nil.
  #
  #
  # ### Return
  #
  # When the given severity is not high enough (for this particular logger), log
  # no message, and return `true`.
  #
  # ### Description
  #
  # Log a message if the given severity is high enough. This is the generic
  # logging method. Users will be more inclined to use
  # [`debug`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-debug),
  # [`info`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-info),
  # [`warn`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-warn),
  # [`error`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-error),
  # and
  # [`fatal`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-fatal).
  #
  # **Message format**: `message` can be any object, but it has to be converted
  # to a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) in order to
  # log it. Generally, `inspect` is used if the given object is not a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). A special case
  # is an `Exception` object, which will be printed in detail, including
  # message, class, and backtrace. See msg2str for the implementation if
  # required.
  #
  # ### Bugs
  #
  # *   Logfile is not locked.
  # *   Append open does not need to lock file.
  # *   If the OS supports multi I/O, records possibly may be mixed.
  #
  #
  # Also aliased as:
  # [`log`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-log)
  sig do
    params(
      severity: T.nilable(Integer),
      message: T.untyped,
      progname: T.untyped,
      blk: T.nilable(T.proc.returns(T.untyped))
    ).returns(TrueClass)
  end
  def add(severity, message = nil, progname = nil, &blk); end

  # Alias for:
  # [`add`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-add)
  sig do
    params(
      severity: T.nilable(Integer),
      message: T.untyped,
      progname: T.untyped,
      blk: T.nilable(T.proc.returns(T.untyped))
    ).returns(TrueClass)
  end
  def log(severity, message = nil, progname = nil, &blk); end

  # Dump given message to the log device without any formatting. If no log
  # device exists, return `nil`.
  sig { params(msg: String).void }
  def <<(msg); end

  # Log a `DEBUG` message.
  #
  # See [`info`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-info)
  # for more information.
  sig { params(progname: T.untyped, block: T.nilable(T.proc.returns(T.untyped))).void }
  def debug(progname = nil, &block); end

  # Log an `INFO` message.
  #
  # `message`
  # :   The message to log; does not need to be a
  #     [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  # `progname`
  # :   In the block form, this is the
  #     [`progname`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#attribute-i-progname)
  #     to use in the log message. The default can be set with
  #     [`progname=`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#attribute-i-progname).
  # `block`
  # :   Evaluates to the message to log. This is not evaluated unless the
  #     logger's level is sufficient to log the message. This allows you to
  #     create potentially expensive logging messages that are only called when
  #     the logger is configured to show them.
  #
  #
  # ### Examples
  #
  # ```ruby
  # logger.info("MainApp") { "Received connection from #{ip}" }
  # # ...
  # logger.info "Waiting for input from user"
  # # ...
  # logger.info { "User typed #{input}" }
  # ```
  #
  # You'll probably stick to the second form above, unless you want to provide a
  # program name (which you can do with
  # [`progname=`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#attribute-i-progname)
  # as well).
  #
  # ### Return
  #
  # See [`add`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-add).
  sig { params(progname: T.untyped, block: T.nilable(T.proc.returns(T.untyped))).void }
  def info(progname = nil, &block); end

  # Log a `WARN` message.
  #
  # See [`info`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-info)
  # for more information.
  sig { params(progname: T.untyped, block: T.nilable(T.proc.returns(T.untyped))).void }
  def warn(progname = nil, &block); end

  # Log an `ERROR` message.
  #
  # See [`info`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-info)
  # for more information.
  sig { params(progname: T.untyped, block: T.nilable(T.proc.returns(T.untyped))).void }
  def error(progname = nil, &block); end

  # Log a `FATAL` message.
  #
  # See [`info`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-info)
  # for more information.
  sig { params(progname: T.untyped, block: T.nilable(T.proc.returns(T.untyped))).void }
  def fatal(progname = nil, &block); end

  # Log an `UNKNOWN` message. This will be printed no matter what the logger's
  # level is.
  #
  # See [`info`](https://docs.ruby-lang.org/en/2.7.0/Logger.html#method-i-info)
  # for more information.
  sig { params(progname: T.untyped, block: T.nilable(T.proc.returns(T.untyped))).void }
  def unknown(progname = nil, &block); end

  # Close the logging device.
  sig { void }
  def close; end
end

class Logger::Error < RuntimeError
end

class Logger::ShiftingError < Logger::Error
end

# Logging severity.
module Logger::Severity
  # Low-level information, mostly for developers.
  DEBUG = T.let(T.unsafe(nil), Integer)
  # Generic (useful) information about system operation.
  INFO = T.let(T.unsafe(nil), Integer)
  # A warning.
  WARN = T.let(T.unsafe(nil), Integer)
  # A handleable error condition.
  ERROR = T.let(T.unsafe(nil), Integer)
  # An unhandleable error that results in a program crash.
  FATAL = T.let(T.unsafe(nil), Integer)
  # An unknown message that should always be logged.
  UNKNOWN = T.let(T.unsafe(nil), Integer)
end

# Default formatter for log messages.
class Logger::Formatter
  def call(severity, time, progname, msg); end
  def datetime_format; end
  def datetime_format=(arg0); end
  def format_datetime(time); end
  def initialize; end
  def msg2str(msg); end
end

module Logger::Period
  def self.next_rotate_time(now, shift_age); end
  def self.previous_period_end(now, shift_age); end
  def next_rotate_time(now, shift_age); end
  def previous_period_end(now, shift_age); end
end

# Device used for logging messages.
class Logger::LogDevice
  include Logger::Period

  def add_log_header(file); end
  def check_shift_log; end
  def close; end
  def create_logfile(filename); end
  def dev; end
  def filename; end
  def initialize(log = nil, shift_age: nil, shift_size: nil, shift_period_suffix: nil, binmode: false); end
  def lock_shift_log; end
  def open_logfile(filename); end
  def reopen(log = nil); end
  def set_dev(log); end
  def shift_log_age; end
  def shift_log_period(period_end); end
  def write(message); end
end
