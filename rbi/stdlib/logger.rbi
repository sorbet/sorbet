# typed: strict
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

  sig { returns(Integer) }
  def level; end

  sig { params(severity: T.untyped).void }
  def level=(severity); end

  sig { returns(T.nilable(String)) }
  def progname; end

  sig { params(progname: T.nilable(String)).void }
  def progname=(progname); end

  sig { returns(T.nilable(String)) }
  def datetime_format; end

  sig { params(datetime_format: T.nilable(String)).void }
  def datetime_format=(datetime_format); end

  FormatterProcType = T.type_alias(
    T.proc.params(
      severity: Integer,
      time: Time,
      progname: T.nilable(String),
      msg: T.untyped
    ).returns(T.untyped)
  )

  sig { returns(T.nilable(FormatterProcType)) }
  def formatter; end

  sig { params(formatter: T.nilable(FormatterProcType)).void }
  def formatter=(formatter); end

  sig { returns(Integer) }
  def sev_threshold; end

  sig { params(severity: T.untyped).void }
  def sev_threshold=(severity); end

  sig { returns(T::Boolean) }
  def debug?; end

  sig { returns(T::Boolean) }
  def info?; end

  sig { returns(T::Boolean) }
  def warn?; end

  sig { returns(T::Boolean) }
  def error?; end

  sig { returns(T::Boolean) }
  def fatal?; end

  sig do
    params(
      logdev: T.any(String, IO),
      shift_age: Integer,
      shift_size: Integer,
      level: Integer,
      progname: T.nilable(String),
      formatter: T.nilable(FormatterProcType),
      datetime_format: T.nilable(String),
      shift_period_suffix: T.nilable(String)
    ).void
  end
  def initialize(
    logdev, shift_age = 0, shift_size = 1048576, level: DEBUG,
    progname: nil, formatter: nil, datetime_format: nil,
    shift_period_suffix: '%Y%m%d'
  )
    @level = T.let(T.unsafe(nil), T.nilable(Integer))
    @progname = T.let(T.unsafe(nil), T.nilable(String))
    @datetime_format = T.let(T.unsafe(nil), T.nilable(String))
    @formatter = T.let(T.unsafe(nil), T.nilable(FormatterProcType))
  end

  sig { params(logdev: T.any(NilClass, String, IO)).returns(T.self_type) }
  def reopen(logdev = nil); end

  sig do
    params(
      severity: T.nilable(Integer),
      message: T.untyped,
      progname: T.untyped,
      blk: T.proc.returns(T.untyped)
    ).returns(TrueClass)
  end
  def add(severity, message = nil, progname = nil, &blk); end

  sig do
    params(
      severity: T.nilable(Integer),
      message: T.untyped,
      progname: T.untyped,
      blk: T.proc.returns(T.untyped)
    ).returns(TrueClass)
  end
  def log(severity, message = nil, progname = nil, &blk); end

  sig { params(msg: String).void }
  def <<(msg); end

  sig { params(progname: T.untyped, block: T.nilable(T.proc.returns(T.untyped))).void }
  def debug(progname = nil, &block); end

  sig { params(progname: T.untyped, block: T.nilable(T.proc.returns(T.untyped))).void }
  def info(progname = nil, &block); end

  sig { params(progname: T.untyped, block: T.nilable(T.proc.returns(T.untyped))).void }
  def warn(progname = nil, &block); end

  sig { params(progname: T.untyped, block: T.nilable(T.proc.returns(T.untyped))).void }
  def error(progname = nil, &block); end

  sig { params(progname: T.untyped, block: T.nilable(T.proc.returns(T.untyped))).void }
  def fatal(progname = nil, &block); end

  sig { params(progname: T.untyped, block: T.nilable(T.proc.returns(T.untyped))).void }
  def unknown(progname = nil, &block); end

  sig { void }
  def close; end
end

class Logger::Error < RuntimeError
end
class Logger::ShiftingError < Logger::Error
end
module Logger::Severity
  DEBUG = T.let(T.unsafe(nil), Integer)
  INFO = T.let(T.unsafe(nil), Integer)
  WARN = T.let(T.unsafe(nil), Integer)
  ERROR = T.let(T.unsafe(nil), Integer)
  FATAL = T.let(T.unsafe(nil), Integer)
  UNKNOWN = T.let(T.unsafe(nil), Integer)
end
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
class Logger::LogDevice
  include Logger::Period

  def add_log_header(file); end
  def check_shift_log; end
  def close; end
  def create_logfile(filename); end
  def dev; end
  def filename; end
  def initialize(log = nil, shift_age: nil, shift_size: nil, shift_period_suffix: nil); end
  def lock_shift_log; end
  def open_logfile(filename); end
  def reopen(log = nil); end
  def set_dev(log); end
  def shift_log_age; end
  def shift_log_period(period_end); end
  def write(message); end
end
