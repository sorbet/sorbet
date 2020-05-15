# typed: __STDLIB_INTERNAL

# The syslog package provides a Ruby interface to the POSIX system logging
# facility.
#
# [`Syslog`](https://docs.ruby-lang.org/en/2.6.0/Syslog.html) messages are
# typically passed to a central logging daemon. The daemon may filter them;
# route them into different files (usually found under /var/log); place them in
# SQL databases; forward them to centralized logging servers via TCP or UDP; or
# even alert the system administrator via email, pager or text message.
#
# Unlike application-level logging via
# [`Logger`](https://docs.ruby-lang.org/en/2.6.0/Logger.html) or Log4r, syslog
# is designed to allow secure tamper-proof logging.
#
# The syslog protocol is standardized in RFC 5424.
module Syslog
  include(::Syslog::Constants)
  include(::Syslog::Macros)
  include(::Syslog::Level)
  include(::Syslog::Facility)
  include(::Syslog::Option)
  extend(::Syslog::Macros)

  # Closes the syslog facility. Raises a runtime exception if it is not open.
  def self.close; end

  # Returns the facility number used in the last call to open()
  def self.facility; end

  # Returns the identity string used in the last call to open()
  def self.ident; end

  # Returns an inspect() string summarizing the object state.
  def self.inspect; end

  # Returns self, for backward compatibility.
  def self.instance; end

  # Log a message with the specified priority. Example:
  #
  # ```ruby
  # Syslog.log(Syslog::LOG_CRIT, "Out of disk space")
  # Syslog.log(Syslog::LOG_CRIT, "User %s logged in", ENV['USER'])
  # ```
  #
  # The priority levels, in descending order, are:
  #
  # LOG\_EMERG
  # :   System is unusable
  # LOG\_ALERT
  # :   Action needs to be taken immediately
  # LOG\_CRIT
  # :   A critical condition has occurred
  # LOG\_ERR
  # :   An error occurred
  # LOG\_WARNING
  # :   [`Warning`](https://docs.ruby-lang.org/en/2.6.0/Warning.html) of a
  #     possible problem
  # LOG\_NOTICE
  # :   A normal but significant condition occurred
  # LOG\_INFO
  # :   Informational message
  # LOG\_DEBUG
  # :   Debugging information
  #
  #
  # Each priority level also has a shortcut method that logs with it's named
  # priority. As an example, the two following statements would produce the same
  # result:
  #
  # ```ruby
  # Syslog.log(Syslog::LOG_ALERT, "Out of memory")
  # Syslog.alert("Out of memory")
  # ```
  #
  # Format strings are as for printf/sprintf, except that in addition %m is
  # replaced with the error message string that would be returned by
  # strerror(errno).
  def self.log(*_); end

  # Returns the log priority mask in effect. The mask is not reset by opening or
  # closing syslog.
  def self.mask; end

  # Sets the log priority mask. A method LOG\_UPTO is defined to make it easier
  # to set mask values. Example:
  #
  # ```ruby
  # Syslog.mask = Syslog::LOG_UPTO(Syslog::LOG_ERR)
  # ```
  #
  # Alternatively, specific priorities can be selected and added together using
  # binary OR. Example:
  #
  # ```ruby
  # Syslog.mask = Syslog::LOG_MASK(Syslog::LOG_ERR) | Syslog::LOG_MASK(Syslog::LOG_CRIT)
  # ```
  #
  # The priority mask persists through calls to open() and close().
  def self.mask=(_); end

  # Open the syslog facility. Raises a runtime exception if it is already open.
  #
  # Can be called with or without a code block. If called with a block, the
  # [`Syslog`](https://docs.ruby-lang.org/en/2.6.0/Syslog.html) object created
  # is passed to the block.
  #
  # If the syslog is already open, raises a
  # [`RuntimeError`](https://docs.ruby-lang.org/en/2.6.0/RuntimeError.html).
  #
  # `ident` is a [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html)
  # which identifies the calling program.
  #
  # `options` is the logical OR of any of the following:
  #
  # LOG\_CONS
  # :   If there is an error while sending to the system logger, write directly
  #     to the console instead.
  #
  # LOG\_NDELAY
  # :   Open the connection now, rather than waiting for the first message to be
  #     written.
  #
  # LOG\_NOWAIT
  # :   Don't wait for any child processes created while logging messages. (Has
  #     no effect on Linux.)
  #
  # LOG\_ODELAY
  # :   Opposite of LOG\_NDELAY; wait until a message is sent before opening the
  #     connection. (This is the default.)
  #
  # LOG\_PERROR
  # :   Print the message to stderr as well as sending it to syslog. (Not in
  #     POSIX.1-2001.)
  #
  # LOG\_PID
  # :   Include the current process ID with each message.
  #
  #
  # `facility` describes the type of program opening the syslog, and is the
  # logical OR of any of the following which are defined for the host OS:
  #
  # LOG\_AUTH
  # :   Security or authorization. Deprecated, use LOG\_AUTHPRIV instead.
  #
  # LOG\_AUTHPRIV
  # :   Security or authorization messages which should be kept private.
  #
  # LOG\_CONSOLE
  # :   System console message.
  #
  # LOG\_CRON
  # :   System task scheduler (cron or at).
  #
  # LOG\_DAEMON
  # :   A system daemon which has no facility value of its own.
  #
  # LOG\_FTP
  # :   An FTP server.
  #
  # LOG\_KERN
  # :   A kernel message (not sendable by user processes, so not of much use to
  #     Ruby, but listed here for completeness).
  #
  # LOG\_LPR
  # :   Line printer subsystem.
  #
  # LOG\_MAIL
  # :   Mail delivery or transport subsystem.
  #
  # LOG\_NEWS
  # :   Usenet news system.
  #
  # LOG\_NTP
  # :   Network [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) Protocol
  #     server.
  #
  # LOG\_SECURITY
  # :   General security message.
  #
  # LOG\_SYSLOG
  # :   Messages generated internally by syslog.
  #
  # LOG\_USER
  # :   Generic user-level message.
  #
  # LOG\_UUCP
  # :   UUCP subsystem.
  #
  # LOG\_LOCAL0 to LOG\_LOCAL7
  # :   Locally-defined facilities.
  #
  #
  # Example:
  #
  # ```ruby
  # Syslog.open("webrick", Syslog::LOG_PID,
  #             Syslog::LOG_DAEMON | Syslog::LOG_LOCAL3)
  # ```
  def self.open(*_); end

  # Closes and then reopens the syslog.
  #
  # Arguments are the same as for open().
  def self.open!(*_); end

  # Returns true if the syslog is open.
  def self.opened?; end

  # Returns the options bitmask used in the last call to open()
  def self.options; end

  # Closes and then reopens the syslog.
  #
  # Arguments are the same as for open().
  def self.reopen(*_); end
end

module Syslog::Constants
  include(::Syslog::Level)
  include(::Syslog::Facility)
  include(::Syslog::Option)

  def self.included(_); end
end

module Syslog::Facility
end

module Syslog::Level
end

# [`Syslog::Logger`](https://docs.ruby-lang.org/en/2.6.0/Syslog/Logger.html) is
# a [`Logger`](https://docs.ruby-lang.org/en/2.6.0/Syslog/Logger.html)
# work-alike that logs via syslog instead of to a file. You can use
# [`Syslog::Logger`](https://docs.ruby-lang.org/en/2.6.0/Syslog/Logger.html) to
# aggregate logs between multiple machines.
#
# By default,
# [`Syslog::Logger`](https://docs.ruby-lang.org/en/2.6.0/Syslog/Logger.html)
# uses the program name 'ruby', but this can be changed via the first argument
# to
# [`Syslog::Logger.new`](https://docs.ruby-lang.org/en/2.6.0/Logger.html#method-c-new).
#
# NOTE! You can only set the
# [`Syslog::Logger`](https://docs.ruby-lang.org/en/2.6.0/Syslog/Logger.html)
# program name when you initialize
# [`Syslog::Logger`](https://docs.ruby-lang.org/en/2.6.0/Syslog/Logger.html) for
# the first time. This is a limitation of the way
# [`Syslog::Logger`](https://docs.ruby-lang.org/en/2.6.0/Syslog/Logger.html)
# uses syslog (and in some ways, a limitation of the way syslog(3) works).
# Attempts to change Syslog::Logger's program name after the first
# initialization will be ignored.
#
# ### Example
#
# The following will log to syslogd on your local machine:
#
# ```ruby
# require 'syslog/logger'
#
# log = Syslog::Logger.new 'my_program'
# log.info 'this line will be logged via syslog(3)'
# ```
#
# Also the facility may be set to specify the facility level which will be used:
#
# ```ruby
# log.info 'this line will be logged using Syslog default facility level'
#
# log_local1 = Syslog::Logger.new 'my_program', Syslog::LOG_LOCAL1
# log_local1.info 'this line will be logged using local1 facility level'
# ```
#
# You may need to perform some syslog.conf setup first. For a BSD machine add
# the following lines to /etc/syslog.conf:
#
# ```
# !my_program
# *.*                                             /var/log/my_program.log
# ```
#
# Then touch /var/log/my\_program.log and signal syslogd with a HUP (killall
# -HUP syslogd, on FreeBSD).
#
# If you wish to have logs automatically roll over and archive, see the
# newsyslog.conf(5) and newsyslog(8) man pages.
class Syslog::Logger
  # Maps [`Logger`](https://docs.ruby-lang.org/en/2.6.0/Syslog/Logger.html)
  # warning types to syslog(3) warning types.
  #
  # Messages from Ruby applications are not considered as critical as messages
  # from other system daemons using syslog(3), so most messages are reduced by
  # one level. For example, a fatal message for Ruby's
  # [`Logger`](https://docs.ruby-lang.org/en/2.6.0/Syslog/Logger.html) is
  # considered an error for syslog(3).
  LEVEL_MAP = T.let(T.unsafe(nil), T::Hash[Integer, Integer])

  # The version of
  # [`Syslog::Logger`](https://docs.ruby-lang.org/en/2.6.0/Syslog/Logger.html)
  # you are using.
  VERSION = T.let(T.unsafe(nil), String)

  # Fills in variables for
  # [`Logger`](https://docs.ruby-lang.org/en/2.6.0/Logger.html) compatibility.
  # If this is the first instance of
  # [`Syslog::Logger`](https://docs.ruby-lang.org/en/2.6.0/Syslog/Logger.html),
  # `program_name` may be set to change the logged program name. The `facility`
  # may be set to specify the facility level which will be used.
  #
  # Due to the way syslog works, only one program name may be chosen.
  def self.new(program_name = _, facility = _); end

  # Almost duplicates
  # [`Logger#add`](https://docs.ruby-lang.org/en/2.6.0/Logger.html#method-i-add).
  # `progname` is ignored.
  def add(severity, message = _, progname = _, &block); end

  # Logs a `message` at the debug (syslog debug) log level, or logs the message
  # returned from the block.
  def debug(message = _, &block); end

  # Logs a `message` at the error (syslog warning) log level, or logs the
  # message returned from the block.
  def error(message = _, &block); end

  # The facility argument is used to specify what type of program is logging the
  # message.
  def facility; end

  # The facility argument is used to specify what type of program is logging the
  # message.
  def facility=(_); end

  # Logs a `message` at the fatal (syslog err) log level, or logs the message
  # returned from the block.
  def fatal(message = _, &block); end

  # Logging formatter, as a `Proc` that will take four arguments and return the
  # formatted message. The arguments are:
  #
  # `severity`
  # :   The Severity of the log message.
  # `time`
  # :   A [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) instance
  #     representing when the message was logged.
  # `progname`
  # :   The progname configured, or passed to the logger method.
  # `msg`
  # :   The *Object* the user passed to the log message; not necessarily a
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html).
  #
  #
  # The block should return an
  # [`Object`](https://docs.ruby-lang.org/en/2.6.0/Object.html) that can be
  # written to the logging device via `write`. The default formatter is used
  # when no formatter is set.
  def formatter; end

  # Logging formatter, as a `Proc` that will take four arguments and return the
  # formatted message. The arguments are:
  #
  # `severity`
  # :   The Severity of the log message.
  # `time`
  # :   A [`Time`](https://docs.ruby-lang.org/en/2.6.0/Time.html) instance
  #     representing when the message was logged.
  # `progname`
  # :   The progname configured, or passed to the logger method.
  # `msg`
  # :   The *Object* the user passed to the log message; not necessarily a
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html).
  #
  #
  # The block should return an
  # [`Object`](https://docs.ruby-lang.org/en/2.6.0/Object.html) that can be
  # written to the logging device via `write`. The default formatter is used
  # when no formatter is set.
  def formatter=(_); end

  # Logs a `message` at the info (syslog info) log level, or logs the message
  # returned from the block.
  def info(message = _, &block); end

  # Log level for
  # [`Logger`](https://docs.ruby-lang.org/en/2.6.0/Syslog/Logger.html)
  # compatibility.
  def level; end

  # Log level for
  # [`Logger`](https://docs.ruby-lang.org/en/2.6.0/Syslog/Logger.html)
  # compatibility.
  def level=(_); end

  # Logs a `message` at the unknown (syslog alert) log level, or logs the
  # message returned from the block.
  def unknown(message = _, &block); end

  # Logs a `message` at the warn (syslog notice) log level, or logs the message
  # returned from the block.
  def warn(message = _, &block); end

  # Builds a methods for level `meth`.
  def self.make_methods(meth); end

  # Returns the internal
  # [`Syslog`](https://docs.ruby-lang.org/en/2.6.0/Syslog.html) object that is
  # initialized when the first instance is created.
  def self.syslog; end

  # Specifies the internal
  # [`Syslog`](https://docs.ruby-lang.org/en/2.6.0/Syslog.html) object to be
  # used.
  def self.syslog=(syslog); end
end

# Default formatter for log messages.
# Default formatter for log messages.
class Syslog::Logger::Formatter
  def call(severity, time, progname, msg); end
end

module Syslog::Macros
  # Generates a mask bit for a priority level. See mask=
  def LOG_MASK(_); end

  # Generates a mask value for priority levels at or below the level specified.
  # See mask=
  def LOG_UPTO(_); end

  def self.included(_); end
end

module Syslog::Option
end
