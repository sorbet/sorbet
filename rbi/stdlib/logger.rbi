# typed: true

class Logger
  include(Logger::Severity)

  class Error < ::RuntimeError

  end

  class LogDevice
    include(MonitorMixin)
    include(Logger::Period)

    def dev()
    end

    def filename()
    end

    def write(message)
    end

    def reopen(log = _)
    end

    def close()
    end
  end

  ProgName = T.let(T.unsafe(nil), String)

  class ShiftingError < ::Logger::Error

  end

  module Severity
    DEBUG = T.let(T.unsafe(nil), Integer)

    INFO = T.let(T.unsafe(nil), Integer)

    WARN = T.let(T.unsafe(nil), Integer)

    ERROR = T.let(T.unsafe(nil), Integer)

    FATAL = T.let(T.unsafe(nil), Integer)

    UNKNOWN = T.let(T.unsafe(nil), Integer)
  end

  SEV_LABEL = T.let(T.unsafe(nil), Array)

  class Formatter
    Format = T.let(T.unsafe(nil), String)

    def call(severity, time, progname, msg)
    end

    def datetime_format=(_)
    end

    def datetime_format()
    end
  end

  VERSION = T.let(T.unsafe(nil), String)

  module Period
    SiD = T.let(T.unsafe(nil), Integer)
  end

  def debug(progname = _, &block)
  end

  def <<(msg)
  end

  def level=(severity)
  end

  def warn(progname = _, &block)
  end

  def progname()
  end

  def datetime_format=(datetime_format)
  end

  def datetime_format()
  end

  def sev_threshold()
  end

  def sev_threshold=(severity)
  end

  def debug?()
  end

  def info?()
  end

  def warn?()
  end

  def error?()
  end

  def fatal?()
  end

  def close()
  end

  def progname=(_)
  end

  def unknown(progname = _, &block)
  end

  def level()
  end

  def formatter()
  end

  def fatal(progname = _, &block)
  end

  def formatter=(_)
  end

  def add(severity, message = _, progname = _)
  end

  def reopen(logdev = _)
  end

  def log(severity, message = _, progname = _)
  end

  def error(progname = _, &block)
  end

  def info(progname = _, &block)
  end
end
