module Abbrev
end
class BasicObject
end
module Kernel
end
class Object < BasicObject
  include Kernel
end
class Exception < Object
end
class StandardError < Exception
end
class ArgumentError < StandardError
end
module Enumerable
end
class Array < Object
  include Enumerable
end
module Benchmark
end
class Benchmark::Job < Object
end
class Benchmark::Report < Object
end
class Benchmark::Tms < Object
end
module Comparable
end
class Numeric < Object
  include Comparable
end
class BigDecimal < Numeric
end
module BigMath
end
class Binding < Object
end
class CSV < Object
  include Enumerable
end
class Struct < Object
  include Enumerable
end
class CSV::FieldInfo < Struct
end
class RuntimeError < StandardError
end
class CSV::MalformedCSVError < RuntimeError
end
class CSV::Row < Object
  include Enumerable
end
class CSV::Table < Object
  include Enumerable
end
class Module < Object
end
class Class < Module
end
class IndexError < StandardError
end
class StopIteration < IndexError
end
class ClosedQueueError < StopIteration
end
class Complex < Numeric
end
module Coverage
end
class Data < Object
end
class Date < Object
  include Comparable
end
class Date::Infinity < Numeric
end
class DateTime < Date
end
module DidYouMean
end
class DidYouMean::ClassNameChecker < Object
end
module DidYouMean::Correctable
end
class DidYouMean::Formatter < Object
end
module DidYouMean::Jaro
end
module DidYouMean::JaroWinkler
end
module DidYouMean::Levenshtein
end
class DidYouMean::MethodNameChecker < Object
end
module DidYouMean::NameErrorCheckers
end
class DidYouMean::NullChecker < Object
end
class DidYouMean::SpellChecker < Object
end
class DidYouMean::VariableNameChecker < Object
end
class Dir < Object
  include Enumerable
end
class IOError < StandardError
end
class EOFError < IOError
end
class Encoding < Object
end
class EncodingError < StandardError
end
class Encoding::CompatibilityError < EncodingError
end
class Encoding::Converter < Data
end
class Encoding::ConverterNotFoundError < EncodingError
end
class Encoding::InvalidByteSequenceError < EncodingError
end
class Encoding::UndefinedConversionError < EncodingError
end
class Enumerator < Object
  include Enumerable
end
class Enumerator::Generator < Object
  include Enumerable
end
class Enumerator::Lazy < Enumerator
end
class Enumerator::Yielder < Object
end
module Errno
end
class SystemCallError < StandardError
end
class Errno::E2BIG < SystemCallError
end
class Errno::EACCES < SystemCallError
end
class Errno::EADDRINUSE < SystemCallError
end
class Errno::EADDRNOTAVAIL < SystemCallError
end
class Errno::EADV < SystemCallError
end
class Errno::EAFNOSUPPORT < SystemCallError
end
class Errno::EAGAIN < SystemCallError
end
class Errno::EALREADY < SystemCallError
end
class Errno::EBADE < SystemCallError
end
class Errno::EBADF < SystemCallError
end
class Errno::EBADFD < SystemCallError
end
class Errno::EBADMSG < SystemCallError
end
class Errno::EBADR < SystemCallError
end
class Errno::EBADRQC < SystemCallError
end
class Errno::EBADSLT < SystemCallError
end
class Errno::EBFONT < SystemCallError
end
class Errno::EBUSY < SystemCallError
end
class Errno::ECANCELED < SystemCallError
end
class Errno::ECHILD < SystemCallError
end
class Errno::ECHRNG < SystemCallError
end
class Errno::ECOMM < SystemCallError
end
class Errno::ECONNABORTED < SystemCallError
end
class Errno::ECONNREFUSED < SystemCallError
end
class Errno::ECONNRESET < SystemCallError
end
class Errno::EDEADLK < SystemCallError
end
class Errno::EDESTADDRREQ < SystemCallError
end
class Errno::EDOM < SystemCallError
end
class Errno::EDOTDOT < SystemCallError
end
class Errno::EDQUOT < SystemCallError
end
class Errno::EEXIST < SystemCallError
end
class Errno::EFAULT < SystemCallError
end
class Errno::EFBIG < SystemCallError
end
class Errno::EHOSTDOWN < SystemCallError
end
class Errno::EHOSTUNREACH < SystemCallError
end
class Errno::EHWPOISON < SystemCallError
end
class Errno::EIDRM < SystemCallError
end
class Errno::EILSEQ < SystemCallError
end
class Errno::EINPROGRESS < SystemCallError
end
class Errno::EINTR < SystemCallError
end
class Errno::EINVAL < SystemCallError
end
class Errno::EIO < SystemCallError
end
class Errno::EISCONN < SystemCallError
end
class Errno::EISDIR < SystemCallError
end
class Errno::EISNAM < SystemCallError
end
class Errno::EKEYEXPIRED < SystemCallError
end
class Errno::EKEYREJECTED < SystemCallError
end
class Errno::EKEYREVOKED < SystemCallError
end
class Errno::EL2HLT < SystemCallError
end
class Errno::EL2NSYNC < SystemCallError
end
class Errno::EL3HLT < SystemCallError
end
class Errno::EL3RST < SystemCallError
end
class Errno::ELIBACC < SystemCallError
end
class Errno::ELIBBAD < SystemCallError
end
class Errno::ELIBEXEC < SystemCallError
end
class Errno::ELIBMAX < SystemCallError
end
class Errno::ELIBSCN < SystemCallError
end
class Errno::ELNRNG < SystemCallError
end
class Errno::ELOOP < SystemCallError
end
class Errno::EMEDIUMTYPE < SystemCallError
end
class Errno::EMFILE < SystemCallError
end
class Errno::EMLINK < SystemCallError
end
class Errno::EMSGSIZE < SystemCallError
end
class Errno::EMULTIHOP < SystemCallError
end
class Errno::ENAMETOOLONG < SystemCallError
end
class Errno::ENAVAIL < SystemCallError
end
class Errno::ENETDOWN < SystemCallError
end
class Errno::ENETRESET < SystemCallError
end
class Errno::ENETUNREACH < SystemCallError
end
class Errno::ENFILE < SystemCallError
end
class Errno::ENOANO < SystemCallError
end
class Errno::ENOBUFS < SystemCallError
end
class Errno::ENOCSI < SystemCallError
end
class Errno::ENODATA < SystemCallError
end
class Errno::ENODEV < SystemCallError
end
class Errno::ENOENT < SystemCallError
end
class Errno::ENOEXEC < SystemCallError
end
class Errno::ENOKEY < SystemCallError
end
class Errno::ENOLCK < SystemCallError
end
class Errno::ENOLINK < SystemCallError
end
class Errno::ENOMEDIUM < SystemCallError
end
class Errno::ENOMEM < SystemCallError
end
class Errno::ENOMSG < SystemCallError
end
class Errno::ENONET < SystemCallError
end
class Errno::ENOPKG < SystemCallError
end
class Errno::ENOPROTOOPT < SystemCallError
end
class Errno::ENOSPC < SystemCallError
end
class Errno::ENOSR < SystemCallError
end
class Errno::ENOSTR < SystemCallError
end
class Errno::ENOSYS < SystemCallError
end
class Errno::ENOTBLK < SystemCallError
end
class Errno::ENOTCONN < SystemCallError
end
class Errno::ENOTDIR < SystemCallError
end
class Errno::ENOTEMPTY < SystemCallError
end
class Errno::ENOTNAM < SystemCallError
end
class Errno::ENOTRECOVERABLE < SystemCallError
end
class Errno::ENOTSOCK < SystemCallError
end
class Errno::ENOTTY < SystemCallError
end
class Errno::ENOTUNIQ < SystemCallError
end
class Errno::ENXIO < SystemCallError
end
class Errno::EOPNOTSUPP < SystemCallError
end
class Errno::EOVERFLOW < SystemCallError
end
class Errno::EOWNERDEAD < SystemCallError
end
class Errno::EPERM < SystemCallError
end
class Errno::EPFNOSUPPORT < SystemCallError
end
class Errno::EPIPE < SystemCallError
end
class Errno::EPROTO < SystemCallError
end
class Errno::EPROTONOSUPPORT < SystemCallError
end
class Errno::EPROTOTYPE < SystemCallError
end
class Errno::ERANGE < SystemCallError
end
class Errno::EREMCHG < SystemCallError
end
class Errno::EREMOTE < SystemCallError
end
class Errno::EREMOTEIO < SystemCallError
end
class Errno::ERESTART < SystemCallError
end
class Errno::ERFKILL < SystemCallError
end
class Errno::EROFS < SystemCallError
end
class Errno::ESHUTDOWN < SystemCallError
end
class Errno::ESOCKTNOSUPPORT < SystemCallError
end
class Errno::ESPIPE < SystemCallError
end
class Errno::ESRCH < SystemCallError
end
class Errno::ESRMNT < SystemCallError
end
class Errno::ESTALE < SystemCallError
end
class Errno::ESTRPIPE < SystemCallError
end
class Errno::ETIME < SystemCallError
end
class Errno::ETIMEDOUT < SystemCallError
end
class Errno::ETOOMANYREFS < SystemCallError
end
class Errno::ETXTBSY < SystemCallError
end
class Errno::EUCLEAN < SystemCallError
end
class Errno::EUNATCH < SystemCallError
end
class Errno::EUSERS < SystemCallError
end
class Errno::EXDEV < SystemCallError
end
class Errno::EXFULL < SystemCallError
end
class Errno::NOERROR < SystemCallError
end
class FalseClass < Object
end
class Fiber < Object
end
class FiberError < StandardError
end
module File::Constants
end
class IO < Object
  include File::Constants
  include Enumerable
end
class File < IO
end
class File::Stat < Object
  include Comparable
end
module FileTest
end
class Float < Numeric
end
class RangeError < StandardError
end
class FloatDomainError < RangeError
end
module Forwardable
end
module GC
end
module GC::Profiler
end
module Gem
end
class Gem::BasicSpecification < Object
end
class Gem::Exception < RuntimeError
end
class Gem::CommandLineError < Gem::Exception
end
class ScriptError < Exception
end
class LoadError < ScriptError
end
class Gem::LoadError < LoadError
end
class Gem::ConflictError < Gem::LoadError
end
class Gem::Dependency < Object
end
class Gem::DependencyError < Gem::Exception
end
class Gem::DependencyRemovalException < Gem::Exception
end
class Gem::DependencyResolutionError < Gem::DependencyError
end
module Gem::Deprecate
end
class Gem::DocumentError < Gem::Exception
end
class Gem::EndOfYAMLException < Gem::Exception
end
class Gem::ErrorReason < Object
end
class Gem::FilePermissionError < Gem::Exception
end
class Gem::FormatException < Gem::Exception
end
class Gem::GemNotFoundException < Gem::Exception
end
class Gem::GemNotInHomeException < Gem::Exception
end
class Gem::ImpossibleDependenciesError < Gem::Exception
end
class Gem::InstallError < Gem::Exception
end
class Gem::InvalidSpecificationException < Gem::Exception
end
class Gem::List < Object
  include Enumerable
end
class Gem::MissingSpecError < Gem::LoadError
end
class Gem::MissingSpecVersionError < Gem::MissingSpecError
end
class Gem::OperationNotSupportedError < Gem::Exception
end
class Gem::PathSupport < Object
end
class Gem::Platform < Object
end
class Gem::PlatformMismatch < Gem::ErrorReason
end
class Gem::RemoteError < Gem::Exception
end
class Gem::RemoteInstallationCancelled < Gem::Exception
end
class Gem::RemoteInstallationSkipped < Gem::Exception
end
class Gem::RemoteSourceException < Gem::Exception
end
class Gem::Requirement < Object
end
class Gem::Requirement::BadRequirementError < ArgumentError
end
class Gem::RubyVersionMismatch < Gem::Exception
end
class Gem::SourceFetchProblem < Gem::ErrorReason
end
class Gem::SpecificGemNotFoundException < Gem::GemNotFoundException
end
class Gem::Specification < Gem::BasicSpecification
end
class Gem::StubSpecification < Gem::BasicSpecification
end
class Gem::StubSpecification::StubLine < Object
end
class SystemExit < Exception
end
class Gem::SystemExitException < SystemExit
end
class Gem::UnsatisfiableDependencyError < Gem::DependencyError
end
class Gem::VerificationError < Gem::Exception
end
class Gem::Version < Object
  include Comparable
end
class Hash < Object
  include Enumerable
end
module IO::WaitReadable
end
class IO::EAGAINWaitReadable < Errno::EAGAIN
  include IO::WaitReadable
end
module IO::WaitWritable
end
class IO::EAGAINWaitWritable < Errno::EAGAIN
  include IO::WaitWritable
end
class IO::EINPROGRESSWaitReadable < Errno::EINPROGRESS
  include IO::WaitReadable
end
class IO::EINPROGRESSWaitWritable < Errno::EINPROGRESS
  include IO::WaitWritable
end
class Integer < Numeric
end
class SignalException < Exception
end
class Interrupt < SignalException
end
class KeyError < IndexError
end
class LocalJumpError < StandardError
end
module Marshal
end
class MatchData < Object
end
module Math
end
class Math::DomainError < StandardError
end
class Method < Object
end
module MonitorMixin
end
class Monitor < Object
  include MonitorMixin
end
class MonitorMixin::ConditionVariable < Object
end
class MonitorMixin::ConditionVariable::Timeout < Exception
end
class NameError < StandardError
end
class NilClass < Object
end
class NoMemoryError < Exception
end
class NoMethodError < NameError
end
class NotImplementedError < ScriptError
end
module ObjectSpace
end
class ObjectSpace::WeakMap < Object
  include Enumerable
end
class Proc < Object
end
module Process
end
module Process::GID
end
class Process::Status < Object
end
module Process::Sys
end
class Process::Tms < Struct
end
module Process::UID
end
class Thread < Object
end
class Process::Waiter < Thread
end
module Random::Formatter
end
class Random < Object
  include Random::Formatter
end
class Range < Object
  include Enumerable
end
class Rational < Numeric
end
module RbConfig
end
class Regexp < Object
end
class RegexpError < StandardError
end
class RubyVM < Object
end
class RubyVM::InstructionSequence < Object
end
class SecurityError < Exception
end
module Signal
end
module SingleForwardable
end
class String < Object
  include Comparable
end
class StringIO < Data
  include IO::generic_writable
  include IO::generic_readable
  include Enumerable
end
class Symbol < Object
  include Comparable
end
class SyntaxError < ScriptError
end
class SystemStackError < Exception
end
class Thread::Backtrace < Object
end
class Thread::Backtrace::Location < Object
end
class Thread::ConditionVariable < Object
end
class Thread::Mutex < Object
end
class Thread::Queue < Object
end
class Thread::SizedQueue < Thread::Queue
end
class ThreadError < StandardError
end
class ThreadGroup < Object
end
class Time < Object
  include Comparable
end
class TracePoint < Object
end
class TrueClass < Object
end
class TypeError < StandardError
end
module URI::RFC2396_REGEXP
end
module URI
end
class URI::Error < StandardError
end
class URI::BadURIError < URI::Error
end
module URI::Escape
end
class URI::Generic < Object
  include URI
  include URI::RFC2396_REGEXP
end
class URI::FTP < URI::Generic
end
class URI::HTTP < URI::Generic
end
class URI::HTTPS < URI::HTTP
end
class URI::InvalidComponentError < URI::Error
end
class URI::InvalidURIError < URI::Error
end
class URI::LDAP < URI::Generic
end
class URI::LDAPS < URI::LDAP
end
class URI::MailTo < URI::Generic
end
class URI::RFC2396_Parser < Object
  include URI::RFC2396_REGEXP
end
module URI::RFC2396_REGEXP::PATTERN
end
class URI::RFC3986_Parser < Object
end
module URI::Util
end
class UnboundMethod < Object
end
class UncaughtThrowError < ArgumentError
end
module Warning
end
class ZeroDivisionError < StandardError
end
module Abbrev
  standard_method(
    {
      words: Opus::Types.array_of(String),
    },
    returns: Opus::Types.hash_of(keys: String, values: String)
  )
  def self.abbrev(words); end
end

class Array
  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def <<(_); end

  standard_method(
    {
      _: Opus::Types.any(Range, Integer, Float),
      _1: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Opus::Types.untyped)
  )
  def [](_, _1=_); end

  standard_method(
    {
      _: Opus::Types.array_of(BasicObject),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def &(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, String),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), String)
  )
  def *(_); end

  standard_method(
    {
      _: Opus::Types.any(Enumerable, Opus::Types.array_of(BasicObject)),
    },
    returns: Opus::Types.array_of(BasicObject)
  )
  def +(_); end

  standard_method(
    {
      _: Opus::Types.array_of(BasicObject),
    },
    returns: Opus::Types.array_of(BasicObject)
  )
  def -(_); end

  standard_method(
    {
      _: Opus::Types.any(Range, Integer),
      _1: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Opus::Types.untyped)
  )
  def slice(_, _1=_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Range),
      _1: Opus::Types.any(BasicObject, Integer),
      _2: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def []=(_, _1, _2=_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def assoc(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.untyped
  )
  def at(_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def clear(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def map(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def map!(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def collect(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def combination(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def push(*_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def compact(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def compact!(); end

  standard_method(
    {
      _: Opus::Types.array_of(BasicObject),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def concat(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Integer
  )
  def count(_=_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def cycle(_=_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(Opus::Types.untyped, BasicObject)
  )
  def delete(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def delete_at(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def delete_if(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def drop(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def drop_while(); end

  standard_method(
    {},
    returns: Opus::Types.any(Enumerator, Opus::Types.array_of(Opus::Types.untyped))
  )
  def each(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def each_index(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end

  standard_method(
    {
      _: Integer,
      _1: BasicObject,
    },
    returns: Opus::Types.any(Opus::Types.untyped, BasicObject)
  )
  def fetch(_, _1=_); end

  standard_method(
    {
      _: Opus::Types.any(BasicObject, Integer, Range),
      _1: Opus::Types.any(Integer, Range),
      _2: Integer,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def fill(_=_, _1=_, _2=_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def flatten(); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(Integer, Enumerator)
  )
  def index(_=_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def first(_=_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(_); end

  standard_method(
    {
      _: Integer,
      _1: BasicObject,
    },
    returns: Object
  )
  def initialize(_=_, _1=_); end

  standard_method(
    {
      _: Integer,
      _1: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def insert(_, *_1); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def join(_=_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def keep_if(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def last(_=_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def member?(_); end

  standard_method(
    {},
    returns: Integer
  )
  def length(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Enumerator, Opus::Types.array_of(Opus::Types.untyped))
  )
  def permutation(_=_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Opus::Types.untyped)
  )
  def pop(_=_); end

  standard_method(
    {
      _: Opus::Types.array_of(BasicObject),
    },
    returns: Opus::Types.array_of(Opus::Types.array_of(BasicObject))
  )
  def product(*_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def rassoc(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def reject(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def reject!(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def repeated_combination(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def repeated_permutation(_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def reverse(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def reverse!(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def reverse_each(); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Integer, Enumerator)
  )
  def rindex(_=_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def rotate(_=_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def rotate!(_=_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def sample(_=_); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def select(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def select!(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def shift(_=_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def shuffle(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def shuffle!(); end

  standard_method(
    {
      _: Opus::Types.any(Range, Integer, Float),
      _1: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Opus::Types.untyped)
  )
  def slice!(_, _1=_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def sort(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def sort!(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def sort_by!(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def take(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def take_while(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def to_a(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def to_ary(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def transpose(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def uniq(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def uniq!(); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def unshift(*_); end

  standard_method(
    {
      _: Opus::Types.any(Range, Integer),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def values_at(*_); end

  standard_method(
    {
      _: Opus::Types.array_of(BasicObject),
    },
    returns: Opus::Types.array_of(Opus::Types.array_of(BasicObject))
  )
  def zip(*_); end

  standard_method(
    {
      _: Opus::Types.array_of(BasicObject),
    },
    returns: Opus::Types.array_of(BasicObject)
  )
  def |(_); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end

  standard_method(
    {
      _: Opus::Types.any(Range, Integer, Float),
      _1: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Opus::Types.untyped)
  )
  def slice(_, _1=_); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end
end

module Base64
  standard_method(
    {
      str: String,
    },
    returns: String
  )
  def self.decode64(str); end

  standard_method(
    {
      bin: String,
    },
    returns: String
  )
  def self.encode64(bin); end

  standard_method(
    {
      str: String,
    },
    returns: String
  )
  def self.strict_decode64(str); end

  standard_method(
    {
      bin: String,
    },
    returns: String
  )
  def self.strict_encode64(bin); end

  standard_method(
    {
      str: String,
    },
    returns: String
  )
  def self.urlsafe_decode64(str); end

  standard_method(
    {
      bin: String,
    },
    returns: String
  )
  def self.urlsafe_encode64(bin); end
end

class BasicObject
  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(other); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(other); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def !(); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def !=(other); end

  standard_method(
    {
      _: String,
      filename: String,
      lineno: Integer,
    },
    returns: Opus::Types.untyped
  )
  def instance_eval(_=_, filename=_, lineno=_); end

  standard_method(
    {
      args: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def instance_exec(*args); end

  standard_method(
    {
      _: Symbol,
      _1: BasicObject,
    },
    returns: BasicObject
  )
  def __send__(_, *_1); end

  standard_method(
    {},
    returns: Integer
  )
  def __id__(); end
end

module Benchmark
  standard_method(
    {
      caption: String,
      label_width: Integer,
      format: String,
      labels: String,
    },
    returns: Opus::Types.array_of(Benchmark::Tms)
  )
  def self.benchmark(caption, label_width=_, format=_, *labels); end

  standard_method(
    {
      label_width: Integer,
      labels: String,
    },
    returns: Opus::Types.array_of(Benchmark::Tms)
  )
  def self.bm(label_width=_, *labels); end

  standard_method(
    {
      width: Integer,
    },
    returns: Opus::Types.array_of(Benchmark::Tms)
  )
  def self.bmbm(width=_); end

  standard_method(
    {
      label: String,
    },
    returns: Benchmark::Tms
  )
  def self.measure(label=_); end

  standard_method(
    {},
    returns: Integer
  )
  def self.realtime(); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: BigDecimal
  )
  def %(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def +(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def -(_); end

  standard_method(
    {},
    returns: BigDecimal
  )
  def -@(); end

  standard_method(
    {},
    returns: BigDecimal
  )
  def +@(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def *(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: BigDecimal
  )
  def **(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def /(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Object
  )
  def <=>(_); end

  standard_method(
    {},
    returns: BigDecimal
  )
  def abs(); end

  standard_method(
    {},
    returns: BigDecimal
  )
  def abs2(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg(); end

  standard_method(
    {},
    returns: Integer
  )
  def ceil(); end

  standard_method(
    {},
    returns: BigDecimal
  )
  def conj(); end

  standard_method(
    {},
    returns: BigDecimal
  )
  def conjugate(); end

  standard_method(
    {},
    returns: Integer
  )
  def denominator(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Integer
  )
  def div(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def divmod(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def fdiv(_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def finite?(); end

  standard_method(
    {},
    returns: Integer
  )
  def floor(); end

  standard_method(
    {},
    returns: Integer
  )
  def hash(); end

  standard_method(
    {},
    returns: Integer
  )
  def imag(); end

  standard_method(
    {},
    returns: Integer
  )
  def imaginary(); end

  standard_method(
    {},
    returns: Opus::Types.any(NilClass, Integer)
  )
  def infinite?(); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: BigDecimal
  )
  def magnitude(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: BigDecimal
  )
  def modulo(_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def nan?(); end

  standard_method(
    {},
    returns: Integer
  )
  def numerator(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def quo(_); end

  standard_method(
    {},
    returns: BigDecimal
  )
  def real(); end

  standard_method(
    {},
    returns: TrueClass
  )
  def real?(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Integer, BigDecimal)
  )
  def round(_=_); end

  standard_method(
    {},
    returns: Float
  )
  def to_f(); end

  standard_method(
    {},
    returns: Integer
  )
  def to_i(); end

  standard_method(
    {},
    returns: Integer
  )
  def to_int(); end

  standard_method(
    {},
    returns: Rational
  )
  def to_r(); end

  standard_method(
    {},
    returns: Complex
  )
  def to_c(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Integer, Rational)
  )
  def truncate(_=_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end

  standard_method(
    {},
    returns: [Integer, Integer]
  )
  def precs(); end

  standard_method(
    {},
    returns: [Integer, String, Integer, Integer]
  )
  def split(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: BigDecimal
  )
  def remainder(_); end

  standard_method(
    {},
    returns: BigDecimal
  )
  def fix(); end

  standard_method(
    {},
    returns: BigDecimal
  )
  def frac(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: BigDecimal
  )
  def power(_); end

  standard_method(
    {},
    returns: Object
  )
  def nonzero?(); end

  standard_method(
    {},
    returns: Integer
  )
  def exponent(); end

  standard_method(
    {},
    returns: Integer
  )
  def sign(); end

  standard_method(
    {},
    returns: String
  )
  def _dump(); end

  standard_method(
    {
      _: Integer,
    },
    returns: BigDecimal
  )
  def sqrt(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      _1: Integer,
    },
    returns: BigDecimal
  )
  def add(_, _1); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      _1: Integer,
    },
    returns: BigDecimal
  )
  def sub(_, _1); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      _1: Integer,
    },
    returns: BigDecimal
  )
  def mult(_, _1); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: [BigDecimal, BigDecimal]
  )
  def coerce(_); end
end

module BigMath
  standard_method(
    {
      _: Integer,
      _1: Integer,
    },
    returns: BigDecimal
  )
  def self.exp(_, _1); end

  standard_method(
    {
      _: Integer,
      _1: Integer,
    },
    returns: BigDecimal
  )
  def self.log(_, _1); end

  standard_method(
    {
      prec: Integer,
    },
    returns: BigDecimal
  )
  def E(prec); end

  standard_method(
    {
      prec: Integer,
    },
    returns: BigDecimal
  )
  def PI(prec); end

  standard_method(
    {
      x: Integer,
      prec: Integer,
    },
    returns: BigDecimal
  )
  def atan(x, prec); end

  standard_method(
    {
      x: Integer,
      prec: Integer,
    },
    returns: BigDecimal
  )
  def cos(x, prec); end

  standard_method(
    {
      x: Integer,
      prec: Integer,
    },
    returns: BigDecimal
  )
  def sin(x, prec); end

  standard_method(
    {
      x: Integer,
      prec: Integer,
    },
    returns: BigDecimal
  )
  def sqrt(x, prec); end
end

class CSV
  standard_method(
    {
      path: Opus::Types.any(String, File),
      options: Opus::Types.hash_of(keys: Symbol, values: BasicObject),
    },
    returns: NilClass
  )
  def self.foreach(path, options=_); end
end

class Class
  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def allocate(); end

  standard_method(
    {
      _: Class,
    },
    returns: Opus::Types.untyped
  )
  def inherited(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Class, NilClass)
  )
  def superclass(); end

  standard_method(
    {
      _: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def instance_methods(_=_); end

  standard_method(
    {},
    returns: Class
  )
  def class(); end

  standard_method(
    {},
    returns: String
  )
  def name(); end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Complex
  )
  def *(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Complex
  )
  def **(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Complex
  )
  def +(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Complex
  )
  def -(_); end

  standard_method(
    {},
    returns: Complex
  )
  def -@(); end

  standard_method(
    {},
    returns: Complex
  )
  def +@(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Complex
  )
  def /(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def abs(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def abs2(); end

  standard_method(
    {},
    returns: Float
  )
  def angle(); end

  standard_method(
    {},
    returns: Float
  )
  def arg(); end

  standard_method(
    {},
    returns: Complex
  )
  def conj(); end

  standard_method(
    {},
    returns: Complex
  )
  def conjugate(); end

  standard_method(
    {},
    returns: Integer
  )
  def denominator(); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Complex
  )
  def fdiv(_); end

  standard_method(
    {},
    returns: Integer
  )
  def hash(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def imag(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def imaginary(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def magnitude(); end

  standard_method(
    {},
    returns: Complex
  )
  def numerator(); end

  standard_method(
    {},
    returns: Float
  )
  def phase(); end

  standard_method(
    {},
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def polar(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex, BigDecimal)
  )
  def quo(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Rational
  )
  def rationalize(_=_); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def real(); end

  standard_method(
    {},
    returns: FalseClass
  )
  def real?(); end

  standard_method(
    {},
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def rect(); end

  standard_method(
    {},
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def rectangular(); end

  standard_method(
    {},
    returns: Complex
  )
  def to_c(); end

  standard_method(
    {},
    returns: Float
  )
  def to_f(); end

  standard_method(
    {},
    returns: Integer
  )
  def to_i(); end

  standard_method(
    {},
    returns: Rational
  )
  def to_r(); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: [Complex, Complex]
  )
  def coerce(_); end
end

module Coverage
  standard_method(
    {},
    returns: NilClass
  )
  def self.start(); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: String, values: Opus::Types.array_of(Opus::Types.any(Integer, NilClass)))
  )
  def self.result(); end
end

class Date
  standard_method(
    {
      _: Integer,
      _1: Integer,
      _2: Integer,
      _3: Integer,
    },
    returns: Object
  )
  def initialize(_=_, _1=_, _2=_, _3=_); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def strftime(_); end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String, Pathname),
    },
    returns: Opus::Types.any(Integer, Opus::Types.untyped)
  )
  def self.chdir(_=_); end

  standard_method(
    {
      _: String,
    },
    returns: Integer
  )
  def self.chroot(_); end

  standard_method(
    {
      _: String,
    },
    returns: Integer
  )
  def self.delete(_); end

  standard_method(
    {
      _: String,
      _1: Encoding,
    },
    returns: Opus::Types.array_of(String)
  )
  def self.entries(_, _1=_); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.exist?(file); end

  standard_method(
    {
      dir: String,
      _: Encoding,
    },
    returns: Opus::Types.any(NilClass, Enumerator)
  )
  def self.foreach(dir, _=_); end

  standard_method(
    {},
    returns: String
  )
  def self.getwd(); end

  standard_method(
    {
      pattern: Opus::Types.any(String, Opus::Types.array_of(String)),
      flags: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.glob(pattern, flags=_); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def self.home(_=_); end

  standard_method(
    {
      _: String,
      _1: Integer,
    },
    returns: Integer
  )
  def self.mkdir(_, _1=_); end

  standard_method(
    {
      _: String,
      _1: Encoding,
    },
    returns: Opus::Types.any(Dir, Opus::Types.untyped)
  )
  def self.open(_, _1=_); end

  standard_method(
    {},
    returns: String
  )
  def self.pwd(); end

  standard_method(
    {
      _: String,
    },
    returns: Integer
  )
  def self.rmdir(_); end

  standard_method(
    {
      _: String,
    },
    returns: Integer
  )
  def self.unlink(_); end

  standard_method(
    {},
    returns: NilClass
  )
  def close(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each(); end

  standard_method(
    {},
    returns: Integer
  )
  def fileno(); end

  standard_method(
    {
      _: String,
      _1: Encoding,
    },
    returns: Object
  )
  def initialize(_, _1=_); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def path(); end

  standard_method(
    {},
    returns: Integer
  )
  def pos(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def pos=(_); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def read(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def rewind(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.untyped
  )
  def seek(_); end

  standard_method(
    {},
    returns: Integer
  )
  def tell(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def to_path(); end

  standard_method(
    {
      pattern: Opus::Types.any(String, Opus::Types.array_of(String)),
      flags: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.[](pattern, flags=_); end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: String, values: String)
  )
  def self.aliases(); end

  standard_method(
    {
      obj1: BasicObject,
      obj2: BasicObject,
    },
    returns: Opus::Types.any(Encoding, NilClass)
  )
  def self.compatible?(obj1, obj2); end

  standard_method(
    {},
    returns: Encoding
  )
  def self.default_external(); end

  standard_method(
    {
      _: Opus::Types.any(String, Encoding),
    },
    returns: Opus::Types.any(String, Encoding)
  )
  def self.default_external=(_); end

  standard_method(
    {},
    returns: Encoding
  )
  def self.default_internal(); end

  standard_method(
    {
      _: Opus::Types.any(String, Encoding),
    },
    returns: Opus::Types.any(String, NilClass, Encoding)
  )
  def self.default_internal=(_); end

  standard_method(
    {
      _: Opus::Types.any(String, Encoding),
    },
    returns: Encoding
  )
  def self.find(_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Encoding)
  )
  def self.list(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(String)
  )
  def self.name_list(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ascii_compatible?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def dummy?(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: String
  )
  def name(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(String)
  )
  def names(); end

  standard_method(
    {
      name: String,
    },
    returns: Encoding
  )
  def replicate(name); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end
end

module Enumerable
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def all?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def any?(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def collect(); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Integer
  )
  def count(_=_); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.any(NilClass, Enumerator)
  )
  def cycle(n=_); end

  standard_method(
    {
      ifnone: Proc,
    },
    returns: Opus::Types.any(BasicObject, NilClass, Enumerator)
  )
  def detect(ifnone=_); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def drop(n); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def drop_while(); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.any(NilClass, Enumerator)
  )
  def each_cons(n); end

  standard_method(
    {},
    returns: Enumerable
  )
  def each_with_index(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def entries(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def find_all(); end

  standard_method(
    {
      value: BasicObject,
    },
    returns: Opus::Types.any(Integer, NilClass, Enumerator)
  )
  def find_index(value=_); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.any(BasicObject, NilClass, Opus::Types.array_of(BasicObject))
  )
  def first(n=_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def grep(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.array_of(Opus::Types.untyped)), Enumerator)
  )
  def group_by(); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(_); end

  standard_method(
    {
      initial: Opus::Types.any(BasicObject, Symbol),
      _: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def inject(initial=_, _=_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def max(_=_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Enumerator, Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def max_by(_=_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def min(_=_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Enumerator, Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def min_by(_=_); end

  standard_method(
    {},
    returns: [BasicObject, BasicObject]
  )
  def minmax(); end

  standard_method(
    {},
    returns: Opus::Types.any([BasicObject, BasicObject], Enumerator)
  )
  def minmax_by(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def none?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def one?(); end

  standard_method(
    {},
    returns: Opus::Types.any([Opus::Types.array_of(BasicObject), Opus::Types.array_of(BasicObject)], Enumerator)
  )
  def partition(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def reject(); end

  standard_method(
    {},
    returns: Enumerator
  )
  def reverse_each(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def sort(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def sort_by(); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(BasicObject), NilClass)
  )
  def take(n); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def take_while(); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def to_h(); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.any(NilClass, Enumerator)
  )
  def each_slice(n); end

  standard_method(
    {
      ifnone: Proc,
    },
    returns: Opus::Types.any(BasicObject, NilClass, Enumerator)
  )
  def find(ifnone=_); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def map(); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def member?(_); end

  standard_method(
    {
      initial: Opus::Types.any(BasicObject, Symbol),
      _: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def reduce(initial=_, _=_); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def select(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def to_a(); end
end

class Enumerator
  standard_method(
    {
      _: Opus::Types.any(Integer, Proc),
    },
    returns: Object
  )
  def initialize(_=_); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def each(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_with_index(); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_with_object(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: NilClass
  )
  def feed(_); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def next(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def next_values(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def peek(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def peek_values(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def rewind(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, NilClass)
  )
  def size(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def with_index(); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def with_object(_); end
end

class Exception
  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(String)
  )
  def backtrace(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Thread::Backtrace::Location)
  )
  def backtrace_locations(); end

  standard_method(
    {},
    returns: NilClass
  )
  def cause(); end

  standard_method(
    {
      _: String,
    },
    returns: Exception
  )
  def exception(_=_); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: String
  )
  def message(); end

  standard_method(
    {
      _: Opus::Types.any(String, Opus::Types.array_of(String)),
    },
    returns: Opus::Types.array_of(String)
  )
  def set_backtrace(_); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end
end

class File
  standard_method(
    {
      file: String,
      dir: String,
    },
    returns: String
  )
  def self.absolute_path(file, dir=_); end

  standard_method(
    {
      file: Opus::Types.any(BasicObject, Pathname, IO),
    },
    returns: Time
  )
  def self.atime(file); end

  standard_method(
    {
      file: String,
      suffix: String,
    },
    returns: String
  )
  def self.basename(file, suffix=_); end

  standard_method(
    {
      _: String,
      _1: Integer,
      _2: Integer,
    },
    returns: String
  )
  def self.binread(_, _1=_, _2=_); end

  standard_method(
    {
      file: Opus::Types.any(BasicObject, Pathname, IO),
    },
    returns: Time
  )
  def self.birthtime(file); end

  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.blockdev?(file); end

  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.chardev?(file); end

  standard_method(
    {
      mode: Integer,
      files: String,
    },
    returns: Integer
  )
  def self.chmod(mode, *files); end

  standard_method(
    {
      owner: Integer,
      group: Integer,
      files: String,
    },
    returns: Integer
  )
  def self.chown(owner, group, *files); end

  standard_method(
    {
      file: Opus::Types.any(BasicObject, Pathname, IO),
    },
    returns: Time
  )
  def self.ctime(file); end

  standard_method(
    {
      files: String,
    },
    returns: Integer
  )
  def self.delete(*files); end

  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.directory?(file); end

  standard_method(
    {
      file: String,
    },
    returns: String
  )
  def self.dirname(file); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.executable?(file); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.executable_real?(file); end

  standard_method(
    {
      file: Opus::Types.any(BasicObject, Pathname, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.exist?(file); end

  standard_method(
    {
      file: Opus::Types.any(BasicObject, Pathname),
      dir: Opus::Types.any(BasicObject, Pathname),
    },
    returns: String
  )
  def self.expand_path(file, dir=_); end

  standard_method(
    {
      path: String,
    },
    returns: String
  )
  def self.extname(path); end

  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.file?(file); end

  standard_method(
    {
      pattern: String,
      path: String,
      flags: Integer,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.fnmatch(pattern, path, flags=_); end

  standard_method(
    {
      file: String,
    },
    returns: String
  )
  def self.ftype(file); end

  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.grpowned?(file); end

  standard_method(
    {
      file_1: Opus::Types.any(String, IO),
      file_2: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.identical?(file_1, file_2); end

  standard_method(
    {
      _: Opus::Types.any(BasicObject, Pathname, File, Opus::Types.array_of(String)),
    },
    returns: String
  )
  def self.join(*_); end

  standard_method(
    {
      mode: Integer,
      files: String,
    },
    returns: Integer
  )
  def self.lchmod(mode, *files); end

  standard_method(
    {
      owner: Integer,
      group: Integer,
      files: String,
    },
    returns: Integer
  )
  def self.lchown(owner, group, *files); end

  standard_method(
    {
      old: String,
      new: String,
    },
    returns: Integer
  )
  def self.link(old, new); end

  standard_method(
    {
      file: String,
    },
    returns: File::Stat
  )
  def self.lstat(file); end

  standard_method(
    {
      file: Opus::Types.any(BasicObject, Pathname, IO),
    },
    returns: Time
  )
  def self.mtime(file); end

  standard_method(
    {
      file: Opus::Types.any(String, BasicObject, Pathname),
      perm: String,
      opt: Integer,
      mode: String,
      external_encoding: String,
      internal_encoding: String,
      encoding: String,
      textmode: BasicObject,
      binmode: BasicObject,
      autoclose: BasicObject,
    },
    returns: Opus::Types.any(File, Opus::Types.untyped)
  )
  def self.open(file=_, perm=_, opt=_, mode: _, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.owned?(file); end

  standard_method(
    {
      path: String,
    },
    returns: String
  )
  def self.path(path); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.pipe?(file); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.readable?(file); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.readable_real?(file); end

  standard_method(
    {
      link: String,
    },
    returns: String
  )
  def self.readlink(link); end

  standard_method(
    {
      pathname: String,
      dir: String,
    },
    returns: String
  )
  def self.realdirpath(pathname, dir=_); end

  standard_method(
    {
      pathname: String,
      dir: String,
    },
    returns: String
  )
  def self.realpath(pathname, dir=_); end

  standard_method(
    {
      old: String,
      new: String,
    },
    returns: Integer
  )
  def self.rename(old, new); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.setgid?(file); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.setuid?(file); end

  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Integer
  )
  def self.size(file); end

  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.size?(file); end

  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.socket?(file); end

  standard_method(
    {
      file: String,
    },
    returns: [String, String]
  )
  def self.split(file); end

  standard_method(
    {
      file: Opus::Types.any(BasicObject, Pathname),
    },
    returns: File::Stat
  )
  def self.stat(file); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.sticky?(file); end

  standard_method(
    {
      old: String,
      new: String,
    },
    returns: Integer
  )
  def self.symlink(old, new); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.symlink?(file); end

  standard_method(
    {
      file: String,
      _: Integer,
    },
    returns: Integer
  )
  def self.truncate(file, _); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def self.umask(_=_); end

  standard_method(
    {
      atime: Time,
      mtime: Time,
      files: String,
    },
    returns: Integer
  )
  def self.utime(atime, mtime, *files); end

  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.world_readable?(file); end

  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.world_writable?(file); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.writable?(file); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.writable_real?(file); end

  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.zero?(file); end

  standard_method(
    {},
    returns: Time
  )
  def atime(); end

  standard_method(
    {},
    returns: Time
  )
  def birthtime(); end

  standard_method(
    {
      mode: Integer,
    },
    returns: Integer
  )
  def chmod(mode); end

  standard_method(
    {
      owner: Integer,
      group: Integer,
    },
    returns: Integer
  )
  def chown(owner, group); end

  standard_method(
    {},
    returns: Time
  )
  def ctime(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Integer, TrueClass, FalseClass)
  )
  def flock(_); end

  standard_method(
    {
      file: String,
      mode: String,
      perm: String,
      opt: Integer,
    },
    returns: Object
  )
  def initialize(file, mode=_, perm=_, opt=_); end

  standard_method(
    {},
    returns: File::Stat
  )
  def lstat(); end

  standard_method(
    {},
    returns: Time
  )
  def mtime(); end

  standard_method(
    {},
    returns: String
  )
  def path(); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def truncate(_); end

  standard_method(
    {
      pattern: String,
      path: String,
      flags: Integer,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.fnmatch?(pattern, path, flags=_); end

  standard_method(
    {
      files: String,
    },
    returns: Integer
  )
  def self.unlink(*files); end

  standard_method(
    {},
    returns: String
  )
  def to_path(); end
end

class File::Stat
  standard_method(
    {
      other: File::Stat,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(other); end

  standard_method(
    {},
    returns: Time
  )
  def atime(); end

  standard_method(
    {},
    returns: Time
  )
  def birthtime(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def blksize(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def blockdev?(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def blocks(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def chardev?(); end

  standard_method(
    {},
    returns: Time
  )
  def ctime(); end

  standard_method(
    {},
    returns: Integer
  )
  def dev(); end

  standard_method(
    {},
    returns: Integer
  )
  def dev_major(); end

  standard_method(
    {},
    returns: Integer
  )
  def dev_minor(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def directory?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def executable?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def executable_real?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def file?(); end

  standard_method(
    {},
    returns: String
  )
  def ftype(); end

  standard_method(
    {},
    returns: Integer
  )
  def gid(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def grpowned?(); end

  standard_method(
    {},
    returns: Integer
  )
  def ino(); end

  standard_method(
    {
      file: String,
    },
    returns: Object
  )
  def initialize(file); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Integer
  )
  def mode(); end

  standard_method(
    {},
    returns: Time
  )
  def mtime(); end

  standard_method(
    {},
    returns: Integer
  )
  def nlink(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def owned?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def pipe?(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def rdev(); end

  standard_method(
    {},
    returns: Integer
  )
  def rdev_major(); end

  standard_method(
    {},
    returns: Integer
  )
  def rdev_minor(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def readable?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def readable_real?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def setgid?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def setuid?(); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def socket?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sticky?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def symlink?(); end

  standard_method(
    {},
    returns: Integer
  )
  def uid(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def world_readable?(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def world_writable?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def writable?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def writable_real?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end
end

module FileUtils
  standard_method(
    {
      src: Opus::Types.any(String, Pathname),
      dest: Opus::Types.any(String, Pathname),
      preserve: Opus::Types.hash_of(keys: Symbol, values: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.array_of(String)
  )
  def self.cp_r(src, dest, preserve=_); end

  standard_method(
    {
      list: Opus::Types.any(String, Pathname),
      mode: Opus::Types.hash_of(keys: Symbol, values: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.array_of(String)
  )
  def self.mkdir_p(list, mode=_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float, BigDecimal)
  )
  def %(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def *(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, Integer, Rational, BigDecimal, Complex)
  )
  def **(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def +(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def -(_); end

  standard_method(
    {},
    returns: Float
  )
  def -@(); end

  standard_method(
    {},
    returns: Float
  )
  def +@(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def /(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Object
  )
  def <=>(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=(_); end

  standard_method(
    {},
    returns: Float
  )
  def abs(); end

  standard_method(
    {},
    returns: Float
  )
  def abs2(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Integer
  )
  def div(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def divmod(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg(); end

  standard_method(
    {},
    returns: Integer
  )
  def ceil(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: [Float, Float]
  )
  def coerce(_); end

  standard_method(
    {},
    returns: Integer
  )
  def denominator(); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def fdiv(_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def finite?(); end

  standard_method(
    {},
    returns: Integer
  )
  def floor(); end

  standard_method(
    {},
    returns: Integer
  )
  def hash(); end

  standard_method(
    {},
    returns: Object
  )
  def infinite?(); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Float
  )
  def magnitude(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float, BigDecimal)
  )
  def modulo(_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def nan?(); end

  standard_method(
    {},
    returns: Float
  )
  def next_float(); end

  standard_method(
    {},
    returns: Integer
  )
  def numerator(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase(); end

  standard_method(
    {},
    returns: Float
  )
  def prev_float(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def quo(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Rational
  )
  def rationalize(_=_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(_=_); end

  standard_method(
    {},
    returns: Float
  )
  def to_f(); end

  standard_method(
    {},
    returns: Integer
  )
  def to_i(); end

  standard_method(
    {},
    returns: Integer
  )
  def to_int(); end

  standard_method(
    {},
    returns: Rational
  )
  def to_r(); end

  standard_method(
    {},
    returns: Integer
  )
  def truncate(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end

  standard_method(
    {},
    returns: Float
  )
  def conj(); end

  standard_method(
    {},
    returns: Float
  )
  def conjugate(); end

  standard_method(
    {},
    returns: Integer
  )
  def imag(); end

  standard_method(
    {},
    returns: Integer
  )
  def imaginary(); end

  standard_method(
    {},
    returns: Float
  )
  def real(); end

  standard_method(
    {},
    returns: TrueClass
  )
  def real?(); end

  standard_method(
    {},
    returns: Complex
  )
  def to_c(); end
end

module Gem
  standard_method(
    {
      name: String,
      args: String,
      requirements: Gem::Requirement,
    },
    returns: String
  )
  def self.bin_path(name, args=_, *requirements); end

  standard_method(
    {},
    returns: String
  )
  def self.binary_mode(); end

  standard_method(
    {
      install_dir: String,
    },
    returns: String
  )
  def self.bindir(install_dir=_); end

  standard_method(
    {},
    returns: Hash
  )
  def self.clear_default_specs(); end

  standard_method(
    {},
    returns: NilClass
  )
  def self.clear_paths(); end

  standard_method(
    {},
    returns: String
  )
  def self.config_file(); end

  standard_method(
    {},
    returns: Gem::ConfigFile
  )
  def self.configuration(); end

  standard_method(
    {
      config: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def self.configuration=(config); end

  standard_method(
    {
      gem_name: String,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def self.datadir(gem_name); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_bindir(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_cert_path(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_dir(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_exec_format(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_key_path(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_path(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.default_rubygems_dirs(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.default_sources(); end
end

class Hash
  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def [](_); end

  standard_method(
    {
      _: BasicObject,
      _1: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def []=(_, _1); end

  standard_method(
    {
      _: BasicObject,
      _1: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def store(_, _1); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.array_of(BasicObject)
  )
  def assoc(_); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def clear(); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def compare_by_identity(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def compare_by_identity?(); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def default(_=_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def default=(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(Opus::Types.untyped, BasicObject)
  )
  def delete(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped), Enumerator)
  )
  def delete_if(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped), Enumerator)
  )
  def each(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped), Enumerator)
  )
  def each_pair(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped), Enumerator)
  )
  def each_key(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped), Enumerator)
  )
  def each_value(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end

  standard_method(
    {
      _: BasicObject,
      _1: BasicObject,
    },
    returns: Opus::Types.any(Opus::Types.untyped, BasicObject)
  )
  def fetch(_, _1=_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def member?(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def has_key?(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def key?(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def has_value?(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def value?(_); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def invert(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped), Enumerator)
  )
  def keep_if(); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def key(_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def keys(); end

  standard_method(
    {},
    returns: Integer
  )
  def length(); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end

  standard_method(
    {
      _: Opus::Types.hash_of(keys: BasicObject, values: BasicObject),
    },
    returns: Opus::Types.hash_of(keys: BasicObject, values: BasicObject)
  )
  def merge(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.array_of(BasicObject)
  )
  def rassoc(_); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def rehash(); end

  standard_method(
    {},
    returns: Opus::Types.any(Enumerator, Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped))
  )
  def reject(); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def reject!(); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def select(); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def select!(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(BasicObject)
  )
  def shift(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.array_of(BasicObject))
  )
  def to_a(); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def to_hash(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def values(); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def values_at(*_); end
end

class IO
  standard_method(
    {
      fd: Integer,
      mode: Integer,
      opt: Integer,
    },
    returns: Object
  )
  def initialize(fd, mode=_, opt=_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def <<(_); end

  standard_method(
    {
      _: Symbol,
      offset: Integer,
      len: Integer,
    },
    returns: NilClass
  )
  def advise(_, offset=_, len=_); end

  standard_method(
    {
      _: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def autoclose=(_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def autoclose?(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def binmode(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def binmode?(); end

  standard_method(
    {},
    returns: NilClass
  )
  def close(); end

  standard_method(
    {
      _: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def close_on_exec=(_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def close_on_exec?(); end

  standard_method(
    {},
    returns: NilClass
  )
  def close_read(); end

  standard_method(
    {},
    returns: NilClass
  )
  def close_write(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def closed?(); end

  standard_method(
    {
      sep: String,
      limit: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each(sep=_, limit=_); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_byte(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_char(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_codepoint(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eof(); end

  standard_method(
    {
      integer_cmd: Integer,
      arg: Opus::Types.any(String, Integer),
    },
    returns: Integer
  )
  def fcntl(integer_cmd, arg); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def fdatasync(); end

  standard_method(
    {},
    returns: Integer
  )
  def fileno(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def flush(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def fsync(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def getbyte(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def getc(); end

  standard_method(
    {
      sep: String,
      limit: Integer,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def gets(sep=_, limit=_); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Encoding
  )
  def internal_encoding(); end

  standard_method(
    {
      integer_cmd: Integer,
      arg: Opus::Types.any(String, Integer),
    },
    returns: Integer
  )
  def ioctl(integer_cmd, arg); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def isatty(); end

  standard_method(
    {},
    returns: Integer
  )
  def lineno(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def lineno=(_); end

  standard_method(
    {},
    returns: Integer
  )
  def pid(); end

  standard_method(
    {},
    returns: Integer
  )
  def pos(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def pos=(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: NilClass
  )
  def print(*_); end

  standard_method(
    {
      format_string: String,
      _: BasicObject,
    },
    returns: NilClass
  )
  def printf(format_string, *_); end

  standard_method(
    {
      _: Opus::Types.any(Numeric, String),
    },
    returns: Opus::Types.untyped
  )
  def putc(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: NilClass
  )
  def puts(*_); end

  standard_method(
    {
      length: Integer,
      outbuf: String,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def read(length=_, outbuf=_); end

  standard_method(
    {
      len: Integer,
      buf: String,
    },
    returns: String
  )
  def read_nonblock(len, buf=_); end

  standard_method(
    {},
    returns: Integer
  )
  def readbyte(); end

  standard_method(
    {},
    returns: String
  )
  def readchar(); end

  standard_method(
    {
      sep: String,
      limit: Integer,
    },
    returns: String
  )
  def readline(sep=_, limit=_); end

  standard_method(
    {
      sep: String,
      limit: Integer,
    },
    returns: Opus::Types.array_of(String)
  )
  def readlines(sep=_, limit=_); end

  standard_method(
    {
      maxlen: Integer,
      outbuf: String,
    },
    returns: String
  )
  def readpartial(maxlen, outbuf=_); end

  standard_method(
    {
      other_IO: Opus::Types.any(IO, String),
      mode_str: String,
    },
    returns: IO
  )
  def reopen(other_IO, mode_str=_); end

  standard_method(
    {},
    returns: Integer
  )
  def rewind(); end

  standard_method(
    {
      amount: Integer,
      whence: Integer,
    },
    returns: Integer
  )
  def seek(amount, whence=_); end

  standard_method(
    {
      ext_or_ext_int_enc: Opus::Types.any(String, Encoding),
      int_enc: Opus::Types.any(String, Encoding),
    },
    returns: Opus::Types.untyped
  )
  def set_encoding(ext_or_ext_int_enc=_, int_enc=_); end

  standard_method(
    {},
    returns: File::Stat
  )
  def stat(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sync(); end

  standard_method(
    {
      _: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sync=(_); end

  standard_method(
    {
      maxlen: Integer,
      outbuf: String,
    },
    returns: String
  )
  def sysread(maxlen, outbuf); end

  standard_method(
    {
      amount: Integer,
      whence: Integer,
    },
    returns: Integer
  )
  def sysseek(amount, whence=_); end

  standard_method(
    {
      _: String,
    },
    returns: Integer
  )
  def syswrite(_); end

  standard_method(
    {},
    returns: Integer
  )
  def tell(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def to_io(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def tty?(); end

  standard_method(
    {
      _: Opus::Types.any(String, Integer),
    },
    returns: NilClass
  )
  def ungetbyte(_); end

  standard_method(
    {
      _: String,
    },
    returns: NilClass
  )
  def ungetc(_); end

  standard_method(
    {
      _: String,
    },
    returns: Integer
  )
  def write(_); end

  standard_method(
    {
      name: String,
      length: Integer,
      offset: Integer,
    },
    returns: String
  )
  def self.binread(name, length=_, offset=_); end

  standard_method(
    {
      name: String,
      _: String,
      offset: Integer,
      external_encoding: String,
      internal_encoding: String,
      encoding: String,
      textmode: BasicObject,
      binmode: BasicObject,
      autoclose: BasicObject,
      mode: String,
    },
    returns: Integer
  )
  def self.binwrite(name, _, offset=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  standard_method(
    {
      src: Opus::Types.any(String, IO),
      dst: Opus::Types.any(String, IO),
      copy_length: Integer,
      src_offset: Integer,
    },
    returns: Integer
  )
  def self.copy_stream(src, dst, copy_length=_, src_offset=_); end

  standard_method(
    {
      name: String,
      sep: String,
      limit: Integer,
      external_encoding: String,
      internal_encoding: String,
      encoding: String,
      textmode: BasicObject,
      binmode: BasicObject,
      autoclose: BasicObject,
      mode: String,
    },
    returns: Opus::Types.any(NilClass, Enumerator)
  )
  def self.foreach(name, sep=_, limit=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  standard_method(
    {
      fd: Integer,
      mode: String,
      external_encoding: String,
      internal_encoding: String,
      encoding: String,
      textmode: BasicObject,
      binmode: BasicObject,
      autoclose: BasicObject,
      mode: String,
    },
    returns: Opus::Types.any(IO, Opus::Types.untyped)
  )
  def self.open(fd, mode=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  standard_method(
    {
      ext_or_ext_int_enc: String,
      int_enc: String,
      external_encoding: String,
      internal_encoding: String,
      encoding: String,
      textmode: BasicObject,
      binmode: BasicObject,
      autoclose: BasicObject,
      mode: String,
    },
    returns: Opus::Types.any([IO, IO], Opus::Types.untyped)
  )
  def self.pipe(ext_or_ext_int_enc=_, int_enc=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  standard_method(
    {
      name: String,
      length: Integer,
      offset: Integer,
      external_encoding: String,
      internal_encoding: String,
      encoding: String,
      textmode: BasicObject,
      binmode: BasicObject,
      autoclose: BasicObject,
      mode: String,
    },
    returns: String
  )
  def self.read(name, length=_, offset=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  standard_method(
    {
      name: String,
      sep: String,
      limit: Integer,
      external_encoding: String,
      internal_encoding: String,
      encoding: String,
      textmode: BasicObject,
      binmode: BasicObject,
      autoclose: BasicObject,
      mode: String,
    },
    returns: Opus::Types.array_of(String)
  )
  def self.readlines(name, sep=_, limit=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  standard_method(
    {
      read_array: Opus::Types.array_of(IO),
      write_array: Opus::Types.array_of(IO),
      error_array: Opus::Types.array_of(IO),
      timeout: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(IO), NilClass)
  )
  def self.select(read_array, write_array=_, error_array=_, timeout=_); end

  standard_method(
    {
      path: String,
      mode: String,
      perm: String,
    },
    returns: Integer
  )
  def self.sysopen(path, mode=_, perm=_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(IO, NilClass)
  )
  def self.try_convert(_); end

  standard_method(
    {
      name: String,
      _: String,
      offset: Integer,
      external_encoding: String,
      internal_encoding: String,
      encoding: String,
      textmode: BasicObject,
      binmode: BasicObject,
      autoclose: BasicObject,
      mode: String,
    },
    returns: Integer
  )
  def self.write(name, _, offset=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  standard_method(
    {
      fd: Integer,
      mode: Integer,
      opt: Integer,
    },
    returns: Opus::Types.untyped
  )
  def self.for_fd(fd, mode=_, opt=_); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def bytes(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def chars(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def codepoints(); end

  standard_method(
    {
      sep: String,
      limit: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_line(sep=_, limit=_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eof?(); end

  standard_method(
    {
      sep: String,
      limit: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def lines(sep=_, limit=_); end

  standard_method(
    {},
    returns: Integer
  )
  def to_i(); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def %(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def &(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def *(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def **(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def +(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def -(_); end

  standard_method(
    {},
    returns: Integer
  )
  def -@(); end

  standard_method(
    {},
    returns: Integer
  )
  def +@(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def /(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def <<(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Object
  )
  def <=>(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def >>(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Rational, Float, BigDecimal),
    },
    returns: Integer
  )
  def [](_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def ^(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def |(_); end

  standard_method(
    {},
    returns: Integer
  )
  def ~(); end

  standard_method(
    {},
    returns: Integer
  )
  def abs(); end

  standard_method(
    {},
    returns: Integer
  )
  def bit_length(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Integer
  )
  def div(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def divmod(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def fdiv(_); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Integer
  )
  def magnitude(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def modulo(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def quo(_); end

  standard_method(
    {},
    returns: Integer
  )
  def abs2(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg(); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end

  standard_method(
    {},
    returns: Integer
  )
  def hash(); end

  standard_method(
    {},
    returns: Integer
  )
  def ceil(); end

  standard_method(
    {
      _: Encoding,
    },
    returns: String
  )
  def chr(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def coerce(_); end

  standard_method(
    {},
    returns: Integer
  )
  def conj(); end

  standard_method(
    {},
    returns: Integer
  )
  def conjugate(); end

  standard_method(
    {},
    returns: Integer
  )
  def denominator(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Integer, Enumerator)
  )
  def downto(_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def even?(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def gcd(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: [Integer, Integer]
  )
  def gcdlcm(_); end

  standard_method(
    {},
    returns: Integer
  )
  def floor(); end

  standard_method(
    {},
    returns: Integer
  )
  def imag(); end

  standard_method(
    {},
    returns: Integer
  )
  def imaginary(); end

  standard_method(
    {},
    returns: TrueClass
  )
  def integer?(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def lcm(_); end

  standard_method(
    {},
    returns: Integer
  )
  def next(); end

  standard_method(
    {},
    returns: Integer
  )
  def numerator(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def odd?(); end

  standard_method(
    {},
    returns: Integer
  )
  def ord(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase(); end

  standard_method(
    {},
    returns: Integer
  )
  def pred(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Rational
  )
  def rationalize(_=_); end

  standard_method(
    {},
    returns: Integer
  )
  def real(); end

  standard_method(
    {},
    returns: TrueClass
  )
  def real?(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def remainder(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(_=_); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end

  standard_method(
    {},
    returns: Integer
  )
  def succ(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Enumerator)
  )
  def times(); end

  standard_method(
    {},
    returns: Complex
  )
  def to_c(); end

  standard_method(
    {},
    returns: Float
  )
  def to_f(); end

  standard_method(
    {},
    returns: Integer
  )
  def to_i(); end

  standard_method(
    {},
    returns: Integer
  )
  def to_int(); end

  standard_method(
    {},
    returns: Rational
  )
  def to_r(); end

  standard_method(
    {},
    returns: Integer
  )
  def truncate(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Integer, Enumerator)
  )
  def upto(_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end
end

module Kernel
  standard_method(
    {
      x: Opus::Types.any(Numeric, String),
      y: Numeric,
    },
    returns: Complex
  )
  def self.Complex(x, y=_); end

  standard_method(
    {
      x: Numeric,
    },
    returns: Float
  )
  def self.Float(x); end

  standard_method(
    {
      x: NilClass,
    },
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def self.Hash(x); end

  standard_method(
    {
      arg: Opus::Types.any(Numeric, String),
      base: Integer,
    },
    returns: Integer
  )
  def self.Integer(arg, base=_); end

  standard_method(
    {
      x: Opus::Types.any(Numeric, String),
      y: Numeric,
    },
    returns: Rational
  )
  def self.Rational(x, y=_); end

  standard_method(
    {},
    returns: Opus::Types.any(Symbol, NilClass)
  )
  def self.__callee__(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.__dir__(); end

  standard_method(
    {},
    returns: Opus::Types.any(Symbol, NilClass)
  )
  def self.__method__(); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def self.`(_); end

  standard_method(
    {
      msg: String,
    },
    returns: NilClass
  )
  def self.abort(msg=_); end

  standard_method(
    {},
    returns: Proc
  )
  def self.at_exit(); end

  standard_method(
    {
      _module: Opus::Types.any(String, Symbol),
      filename: String,
    },
    returns: NilClass
  )
  def self.autoload(_module, filename); end

  standard_method(
    {
      name: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def self.autoload?(name); end

  standard_method(
    {},
    returns: Binding
  )
  def self.binding(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.block_given?(); end

  standard_method(
    {
      start: Opus::Types.any(Integer, Range),
      length: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.caller(start=_, length=_); end

  standard_method(
    {
      start: Opus::Types.any(Integer, Range),
      length: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.caller_locations(start=_, length=_); end

  standard_method(
    {
      _: String,
      _1: Binding,
      filename: String,
      lineno: Integer,
    },
    returns: Opus::Types.untyped
  )
  def self.eval(_, _1=_, filename=_, lineno=_); end

  standard_method(
    {
      status: Opus::Types.any(Integer, TrueClass, FalseClass),
    },
    returns: NilClass
  )
  def self.exit(status=_); end

  standard_method(
    {
      status: Opus::Types.any(Integer, TrueClass, FalseClass),
    },
    returns: NilClass
  )
  def self.exit!(status); end

  standard_method(
    {
      _: Opus::Types.any(String, Class),
      _1: Opus::Types.any(Opus::Types.array_of(String), String),
      _2: Opus::Types.array_of(String),
    },
    returns: NilClass
  )
  def self.fail(_=_, _1=_, _2=_); end

  standard_method(
    {
      format: String,
      args: BasicObject,
    },
    returns: String
  )
  def self.format(format, *args); end

  standard_method(
    {
      _: String,
      _1: Integer,
    },
    returns: String
  )
  def self.gets(_=_, _1=_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Symbol)
  )
  def self.global_variables(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.iterator?(); end

  standard_method(
    {
      filename: String,
      _: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.load(filename, _=_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Symbol)
  )
  def self.local_variables(); end

  standard_method(
    {
      name: String,
      rest: Opus::Types.any(String, Integer),
      block: String,
    },
    returns: Opus::Types.any(IO, NilClass)
  )
  def self.open(name, rest=_, block=_); end

  standard_method(
    {
      _: IO,
      _1: String,
      _2: BasicObject,
    },
    returns: NilClass
  )
  def self.printf(_=_, _1=_, *_2); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def self.putc(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: NilClass
  )
  def self.puts(*_); end

  standard_method(
    {},
    returns: NilClass
  )
  def self.raise(); end

  standard_method(
    {
      max: Opus::Types.any(Integer, Range),
    },
    returns: Numeric
  )
  def self.rand(max); end

  standard_method(
    {
      _: String,
      _1: Integer,
    },
    returns: String
  )
  def self.readline(_=_, _1=_); end

  standard_method(
    {
      _: String,
      _1: Integer,
    },
    returns: Opus::Types.array_of(String)
  )
  def self.readlines(_=_, _1=_); end

  standard_method(
    {
      name: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.require(name); end

  standard_method(
    {
      name: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.require_relative(name); end

  standard_method(
    {
      read: Opus::Types.array_of(IO),
      write: Opus::Types.array_of(IO),
      error: Opus::Types.array_of(IO),
      timeout: Integer,
    },
    returns: Opus::Types.array_of(String)
  )
  def self.select(read, write=_, error=_, timeout=_); end

  standard_method(
    {
      duration: Numeric,
    },
    returns: Integer
  )
  def self.sleep(duration); end

  standard_method(
    {
      number: Numeric,
    },
    returns: Numeric
  )
  def self.srand(number); end

  standard_method(
    {
      num: Integer,
      args: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def self.syscall(num, *args); end

  standard_method(
    {
      cmd: String,
      file1: String,
      file2: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass, Time)
  )
  def self.test(cmd, file1, file2=_); end

  standard_method(
    {
      msg: String,
    },
    returns: NilClass
  )
  def self.warn(*msg); end

  standard_method(
    {},
    returns: Proc
  )
  def proc(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def clone(); end

  standard_method(
    {
      _: Opus::Types.any(String, Class, Exception),
      _1: String,
      _2: Opus::Types.array_of(String),
    },
    returns: NilClass
  )
  def raise(_=_, _1=_, _2=_); end

  standard_method(
    {
      _: Opus::Types.any(String, Symbol),
      _1: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def send(_, *_1); end

  standard_method(
    {
      format: String,
      args: BasicObject,
    },
    returns: String
  )
  def self.sprintf(format, *args); end
end

module Marshal
  standard_method(
    {
      _: String,
      _1: Proc,
    },
    returns: Object
  )
  def self.load(_, _1=_); end

  standard_method(
    {
      _: Object,
      _1: Opus::Types.any(IO, Integer),
      _2: Integer,
    },
    returns: Object
  )
  def self.dump(_, _1=_, _2=_); end
end

class MatchData
  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end

  standard_method(
    {
      i: Opus::Types.any(Integer, Range, String, Symbol),
      length: Integer,
    },
    returns: Opus::Types.any(String, NilClass, Opus::Types.array_of(String))
  )
  def [](i, length=_); end

  standard_method(
    {
      n: Integer,
    },
    returns: Integer
  )
  def begin(n); end

  standard_method(
    {},
    returns: Opus::Types.array_of(String)
  )
  def captures(); end

  standard_method(
    {
      n: Integer,
    },
    returns: Integer
  )
  def end(n); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(other); end

  standard_method(
    {},
    returns: Integer
  )
  def hash(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Integer
  )
  def length(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(String)
  )
  def names(); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.array_of(Integer)
  )
  def offset(n); end

  standard_method(
    {},
    returns: String
  )
  def post_match(); end

  standard_method(
    {},
    returns: String
  )
  def pre_match(); end

  standard_method(
    {},
    returns: Regexp
  )
  def regexp(); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end

  standard_method(
    {},
    returns: String
  )
  def string(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(String)
  )
  def to_a(); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {
      indexes: Integer,
    },
    returns: Opus::Types.array_of(String)
  )
  def values_at(*indexes); end
end

module Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.acos(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.acosh(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.asin(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.asinh(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.atan(x); end

  standard_method(
    {
      y: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.atan2(y, x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.atanh(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.cbrt(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.cos(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.cosh(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.erf(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.erfc(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.exp(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def self.frexp(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.gamma(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      y: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.hypot(x, y); end

  standard_method(
    {
      fraction: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      exponent: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.ldexp(fraction, exponent); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float)
  )
  def self.lgamma(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      base: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.log(x, base=_); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.log10(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.log2(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.sin(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.sinh(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.sqrt(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.tan(x); end

  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Float
  )
  def self.tanh(x); end
end

class Module
  standard_method(
    {},
    returns: Opus::Types.array_of(Integer)
  )
  def self.constants(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Module)
  )
  def self.nesting(); end

  standard_method(
    {},
    returns: Object
  )
  def initialize(); end

  standard_method(
    {
      other: Module,
    },
    returns: Opus::Types.any(TrueClass, FalseClass, NilClass)
  )
  def <(other); end

  standard_method(
    {
      other: Module,
    },
    returns: Opus::Types.any(TrueClass, FalseClass, NilClass)
  )
  def <=(other); end

  standard_method(
    {
      other: Module,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(other); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(other); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(other); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(other); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(other); end

  standard_method(
    {
      other: Module,
    },
    returns: Opus::Types.any(TrueClass, FalseClass, NilClass)
  )
  def >(other); end

  standard_method(
    {
      other: Module,
    },
    returns: Opus::Types.any(TrueClass, FalseClass, NilClass)
  )
  def >=(other); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Module)
  )
  def ancestors(); end

  standard_method(
    {
      _module: Symbol,
      filename: String,
    },
    returns: NilClass
  )
  def autoload(_module, filename); end

  standard_method(
    {
      name: Symbol,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def autoload?(name); end

  standard_method(
    {
      _: String,
      filename: String,
      lineno: Integer,
    },
    returns: Opus::Types.untyped
  )
  def class_eval(_, filename=_, lineno=_); end

  standard_method(
    {
      args: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def class_exec(*args); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def class_variable_defined?(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def class_variable_get(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      _1: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def class_variable_set(_, _1); end

  standard_method(
    {
      inherit: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def class_variables(inherit=_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      inherit: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def const_defined?(_, inherit=_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      inherit: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.untyped
  )
  def const_get(_, inherit=_); end

  standard_method(
    {
      _: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def const_missing(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      _1: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def const_set(_, _1); end

  standard_method(
    {
      inherit: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def constants(inherit=_); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def freeze(); end

  standard_method(
    {
      _: Module,
    },
    returns: Opus::Types.untyped
  )
  def include(*_); end

  standard_method(
    {
      _: Module,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Module)
  )
  def included_modules(); end

  standard_method(
    {
      _: Symbol,
    },
    returns: UnboundMethod
  )
  def instance_method(_); end

  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def instance_methods(include_super=_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def method_defined?(_); end

  standard_method(
    {
      _: String,
      filename: String,
      lineno: Integer,
    },
    returns: Opus::Types.untyped
  )
  def module_eval(_, filename=_, lineno=_); end

  standard_method(
    {
      args: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def module_exec(*args); end

  standard_method(
    {},
    returns: String
  )
  def name(); end

  standard_method(
    {
      _: Module,
    },
    returns: Opus::Types.untyped
  )
  def prepend(*_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def private_class_method(*_); end

  standard_method(
    {
      _: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def private_constant(*_); end

  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def private_instance_methods(include_super=_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def private_method_defined?(_); end

  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def protected_instance_methods(include_super=_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def protected_method_defined?(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def public_class_method(*_); end

  standard_method(
    {
      _: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def public_constant(*_); end

  standard_method(
    {
      _: Symbol,
    },
    returns: UnboundMethod
  )
  def public_instance_method(_); end

  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def public_instance_methods(include_super=_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def public_method_defined?(_); end

  standard_method(
    {
      _: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def remove_class_variable(_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def singleton_class?(); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {
      new_name: Symbol,
      old_name: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def alias_method(new_name, old_name); end

  standard_method(
    {
      _: Module,
    },
    returns: Opus::Types.untyped
  )
  def append_features(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: NilClass
  )
  def attr_accessor(*_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: NilClass
  )
  def attr_reader(*_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: NilClass
  )
  def attr_writer(*_); end

  standard_method(
    {
      _: Symbol,
      _1: Method,
    },
    returns: Symbol
  )
  def define_method(_, _1=_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def extend_object(_); end

  standard_method(
    {
      othermod: Module,
    },
    returns: Opus::Types.untyped
  )
  def extended(othermod); end

  standard_method(
    {
      othermod: Module,
    },
    returns: Opus::Types.untyped
  )
  def included(othermod); end

  standard_method(
    {
      meth: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def method_added(meth); end

  standard_method(
    {
      method_name: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def method_removed(method_name); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def module_function(*_); end

  standard_method(
    {
      _: Module,
    },
    returns: Opus::Types.untyped
  )
  def prepend_features(_); end

  standard_method(
    {
      othermod: Module,
    },
    returns: Opus::Types.untyped
  )
  def prepended(othermod); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def private(*_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def protected(*_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def public(*_); end

  standard_method(
    {
      _: Class,
    },
    returns: Opus::Types.untyped
  )
  def refine(_); end

  standard_method(
    {
      _: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def remove_const(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def remove_method(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def undef_method(_); end

  standard_method(
    {
      _: Module,
    },
    returns: Opus::Types.untyped
  )
  def using(_); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: NilClass
  )
  def attr(*_); end
end

class NilClass
  standard_method(
    {
      obj: BasicObject,
    },
    returns: FalseClass
  )
  def &(obj); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ^(obj); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def |(obj); end

  standard_method(
    {},
    returns: Rational
  )
  def rationalize(); end

  standard_method(
    {},
    returns: []
  )
  def to_a(); end

  standard_method(
    {},
    returns: Complex
  )
  def to_c(); end

  standard_method(
    {},
    returns: Float
  )
  def to_f(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def to_h(); end

  standard_method(
    {},
    returns: Rational
  )
  def to_r(); end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def %(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def -@(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def +@(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Object
  )
  def <=>(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def abs(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def abs2(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg(); end

  standard_method(
    {},
    returns: Integer
  )
  def ceil(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)]
  )
  def coerce(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def conj(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def conjugate(); end

  standard_method(
    {},
    returns: Integer
  )
  def denominator(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Integer
  )
  def div(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)]
  )
  def divmod(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def fdiv(_); end

  standard_method(
    {},
    returns: Integer
  )
  def floor(); end

  standard_method(
    {},
    returns: Complex
  )
  def i(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def imag(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def imaginary(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def integer?(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def magnitude(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def modulo(_); end

  standard_method(
    {},
    returns: Opus::Types.any(BasicObject, NilClass)
  )
  def nonzero?(); end

  standard_method(
    {},
    returns: Integer
  )
  def numerator(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase(); end

  standard_method(
    {},
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)]
  )
  def polar(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def quo(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def real(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def real?(); end

  standard_method(
    {},
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)]
  )
  def rect(); end

  standard_method(
    {},
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)]
  )
  def rectangular(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def remainder(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(_); end

  standard_method(
    {
      _: Symbol,
    },
    returns: TypeError
  )
  def singleton_method_added(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
      _1: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex, Enumerator)
  )
  def step(_, _1=_); end

  standard_method(
    {},
    returns: Complex
  )
  def to_c(); end

  standard_method(
    {},
    returns: Integer
  )
  def to_int(); end

  standard_method(
    {},
    returns: Integer
  )
  def truncate(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end
end

class Object
  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def !~(other); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(other); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(other); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: NilClass
  )
  def =~(other); end

  standard_method(
    {},
    returns: Class
  )
  def class(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def clone(); end

  standard_method(
    {
      port: IO,
    },
    returns: NilClass
  )
  def display(port); end

  standard_method(
    {},
    returns: BasicObject
  )
  def dup(); end

  standard_method(
    {
      method: Symbol,
      args: BasicObject,
    },
    returns: Enumerator
  )
  def enum_for(method=_, *args); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(other); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def freeze(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def frozen?(); end

  standard_method(
    {},
    returns: Integer
  )
  def hash(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {
      _: Class,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def instance_of?(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def instance_variable_defined?(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def instance_variable_get(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      _1: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def instance_variable_set(_, _1); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Symbol)
  )
  def instance_variables(); end

  standard_method(
    {
      _: Opus::Types.any(Class, Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def is_a?(_); end

  standard_method(
    {
      _: Class,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def kind_of?(_); end

  standard_method(
    {
      _: Symbol,
    },
    returns: Method
  )
  def method(_); end

  standard_method(
    {
      regular: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def methods(regular=_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def nil?(); end

  standard_method(
    {
      all: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def private_methods(all=_); end

  standard_method(
    {
      all: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def protected_methods(all=_); end

  standard_method(
    {
      _: Symbol,
    },
    returns: Method
  )
  def public_method(_); end

  standard_method(
    {
      all: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def public_methods(all=_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      args: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def public_send(_, *args); end

  standard_method(
    {
      _: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def remove_instance_variable(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      args: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def send(_, *args); end

  standard_method(
    {},
    returns: Class
  )
  def singleton_class(); end

  standard_method(
    {
      _: Symbol,
    },
    returns: Method
  )
  def singleton_method(_); end

  standard_method(
    {
      all: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def singleton_methods(all=_); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def taint(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def tainted?(); end

  standard_method(
    {
      method: Symbol,
      args: BasicObject,
    },
    returns: Enumerator
  )
  def to_enum(method=_, *args); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def trust(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def untaint(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def untrust(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def untrusted?(); end
end

class Pathname
  standard_method(
    {},
    returns: Pathname
  )
  def self.getwd(); end

  standard_method(
    {
      p1: String,
      p2: String,
    },
    returns: Opus::Types.array_of(Pathname)
  )
  def self.glob(p1, p2=_); end

  standard_method(
    {
      other: Opus::Types.any(String, Pathname),
    },
    returns: Pathname
  )
  def +(other); end

  standard_method(
    {
      p1: BasicObject,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(p1); end

  standard_method(
    {
      p1: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(p1); end

  standard_method(
    {
      p1: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(p1); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def absolute?(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def ascend(); end

  standard_method(
    {},
    returns: Time
  )
  def atime(); end

  standard_method(
    {
      p1: String,
    },
    returns: Pathname
  )
  def basename(p1=_); end

  standard_method(
    {
      length: Integer,
      offset: Integer,
    },
    returns: String
  )
  def binread(length=_, offset=_); end

  standard_method(
    {
      _: String,
      offset: Integer,
    },
    returns: Integer
  )
  def binwrite(_, offset=_); end

  standard_method(
    {},
    returns: Time
  )
  def birthtime(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def blockdev?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def chardev?(); end

  standard_method(
    {
      with_directory: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Pathname)
  )
  def children(with_directory); end

  standard_method(
    {
      mode: Integer,
    },
    returns: Integer
  )
  def chmod(mode); end

  standard_method(
    {
      owner: Integer,
      group: Integer,
    },
    returns: Integer
  )
  def chown(owner, group); end

  standard_method(
    {
      consider_symlink: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.untyped
  )
  def cleanpath(consider_symlink=_); end

  standard_method(
    {},
    returns: Time
  )
  def ctime(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def delete(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def descend(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def directory?(); end

  standard_method(
    {},
    returns: Pathname
  )
  def dirname(); end

  standard_method(
    {
      with_directory: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.untyped
  )
  def each_child(with_directory); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def each_entry(); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_filename(); end

  standard_method(
    {
      sep: String,
      limit: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_line(sep=_, limit=_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Pathname)
  )
  def entries(); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def executable?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def executable_real?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def exist?(); end

  standard_method(
    {
      p1: Opus::Types.any(String, Pathname),
    },
    returns: Pathname
  )
  def expand_path(p1=_); end

  standard_method(
    {},
    returns: String
  )
  def extname(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def file?(); end

  standard_method(
    {
      ignore_error: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def find(ignore_error); end

  standard_method(
    {
      pattern: String,
      flags: Integer,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def fnmatch(pattern, flags=_); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def freeze(); end

  standard_method(
    {},
    returns: String
  )
  def ftype(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def grpowned?(); end

  standard_method(
    {
      args: Opus::Types.any(String, Pathname),
    },
    returns: Pathname
  )
  def join(*args); end

  standard_method(
    {
      mode: Integer,
    },
    returns: Integer
  )
  def lchmod(mode); end

  standard_method(
    {
      owner: Integer,
      group: Integer,
    },
    returns: Integer
  )
  def lchown(owner, group); end

  standard_method(
    {},
    returns: File::Stat
  )
  def lstat(); end

  standard_method(
    {
      old: String,
    },
    returns: Integer
  )
  def make_link(old); end

  standard_method(
    {
      old: String,
    },
    returns: Opus::Types.any(Integer, TrueClass, FalseClass)
  )
  def symlink?(old=_); end

  standard_method(
    {
      p1: String,
    },
    returns: Integer
  )
  def mkdir(p1); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def mkpath(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def mountpoint?(); end

  standard_method(
    {},
    returns: Time
  )
  def mtime(); end

  standard_method(
    {
      mode: String,
      perm: String,
      opt: Integer,
    },
    returns: Opus::Types.any(File, Opus::Types.untyped)
  )
  def open(mode=_, perm=_, opt=_); end

  standard_method(
    {
      _: Encoding,
    },
    returns: Opus::Types.any(Dir, Opus::Types.untyped)
  )
  def opendir(_=_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def owned?(); end

  standard_method(
    {},
    returns: Pathname
  )
  def parent(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def pipe?(); end

  standard_method(
    {
      length: Integer,
      offset: Integer,
      open_args: Integer,
    },
    returns: String
  )
  def read(length=_, offset=_, open_args=_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def readable?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def readable_real?(); end

  standard_method(
    {
      sep: String,
      limit: Integer,
      open_args: Integer,
    },
    returns: Opus::Types.array_of(String)
  )
  def readlines(sep=_, limit=_, open_args=_); end

  standard_method(
    {},
    returns: String
  )
  def readlink(); end

  standard_method(
    {
      p1: String,
    },
    returns: String
  )
  def realdirpath(p1=_); end

  standard_method(
    {
      p1: String,
    },
    returns: String
  )
  def realpath(p1=_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def relative?(); end

  standard_method(
    {
      base_directory: Opus::Types.any(String, Pathname),
    },
    returns: Pathname
  )
  def relative_path_from(base_directory); end

  standard_method(
    {
      p1: String,
    },
    returns: Integer
  )
  def rename(p1); end

  standard_method(
    {},
    returns: Integer
  )
  def rmdir(); end

  standard_method(
    {},
    returns: Integer
  )
  def rmtree(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def root?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def setgid?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def setuid?(); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def size?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def socket?(); end

  standard_method(
    {},
    returns: [Pathname, Pathname]
  )
  def split(); end

  standard_method(
    {},
    returns: File::Stat
  )
  def stat(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sticky?(); end

  standard_method(
    {
      args: String,
    },
    returns: Pathname
  )
  def sub(*args); end

  standard_method(
    {
      p1: String,
    },
    returns: Pathname
  )
  def sub_ext(p1); end

  standard_method(
    {
      mode: Integer,
      perm: Integer,
    },
    returns: Integer
  )
  def sysopen(mode=_, perm=_); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def taint(); end

  standard_method(
    {},
    returns: String
  )
  def to_path(); end

  standard_method(
    {
      length: Integer,
    },
    returns: Integer
  )
  def truncate(length); end

  standard_method(
    {},
    returns: Integer
  )
  def unlink(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def untaint(); end

  standard_method(
    {
      atime: Time,
      mtime: Time,
    },
    returns: Integer
  )
  def utime(atime, mtime); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def world_readable?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def world_writable?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def writable?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def writable_real?(); end

  standard_method(
    {
      _: String,
      offset: Integer,
      open_args: Integer,
    },
    returns: Integer
  )
  def write(_, offset=_, open_args=_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end

  standard_method(
    {},
    returns: Pathname
  )
  def self.pwd(); end

  standard_method(
    {
      other: Opus::Types.any(String, Pathname),
    },
    returns: Pathname
  )
  def /(other); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end
end

class Proc
  standard_method(
    {},
    returns: Integer
  )
  def arity(); end

  standard_method(
    {},
    returns: Binding
  )
  def binding(); end

  standard_method(
    {
      arity: Integer,
    },
    returns: Proc
  )
  def curry(arity=_); end

  standard_method(
    {},
    returns: Integer
  )
  def hash(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def lambda(); end

  standard_method(
    {},
    returns: Opus::Types.array_of([Symbol, Symbol])
  )
  def parameters(); end

  standard_method(
    {},
    returns: [String, Integer]
  )
  def source_location(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def to_proc(); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end
end

module Process
  standard_method(
    {
      msg: String,
    },
    returns: Opus::Types.untyped
  )
  def self.abort(msg=_); end

  standard_method(
    {},
    returns: String
  )
  def self.argv0(); end

  standard_method(
    {
      clock_id: Opus::Types.any(Symbol, Integer),
      unit: Symbol,
    },
    returns: Opus::Types.any(Float, Integer)
  )
  def self.clock_getres(clock_id, unit=_); end

  standard_method(
    {
      clock_id: Opus::Types.any(Symbol, Integer),
      unit: Symbol,
    },
    returns: Opus::Types.any(Float, Integer)
  )
  def self.clock_gettime(clock_id, unit=_); end

  standard_method(
    {
      nochdir: BasicObject,
      noclose: BasicObject,
    },
    returns: Integer
  )
  def self.daemon(nochdir=_, noclose=_); end

  standard_method(
    {
      pid: Integer,
    },
    returns: Thread
  )
  def self.detach(pid); end

  standard_method(
    {},
    returns: Integer
  )
  def self.egid(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def self.egid=(_); end

  standard_method(
    {},
    returns: Integer
  )
  def self.euid(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def self.euid=(_); end

  standard_method(
    {
      status: Integer,
    },
    returns: Opus::Types.untyped
  )
  def self.exit(status=_); end

  standard_method(
    {
      status: Integer,
    },
    returns: Opus::Types.untyped
  )
  def self.exit!(status=_); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.fork(); end

  standard_method(
    {
      pid: Integer,
    },
    returns: Integer
  )
  def self.getpgid(pid); end

  standard_method(
    {},
    returns: Integer
  )
  def self.getpgrp(); end

  standard_method(
    {
      kind: Integer,
      _: Integer,
    },
    returns: Integer
  )
  def self.getpriority(kind, _); end

  standard_method(
    {
      resource: Opus::Types.any(Symbol, String, Integer),
    },
    returns: [Integer, Integer]
  )
  def self.getrlimit(resource); end

  standard_method(
    {
      pid: Integer,
    },
    returns: Integer
  )
  def self.getsid(pid=_); end

  standard_method(
    {},
    returns: Integer
  )
  def self.gid(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def self.gid=(_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Integer)
  )
  def self.groups(); end

  standard_method(
    {
      _: Opus::Types.array_of(Integer),
    },
    returns: Opus::Types.array_of(Integer)
  )
  def self.groups=(_); end

  standard_method(
    {
      username: String,
      gid: Integer,
    },
    returns: Opus::Types.array_of(Integer)
  )
  def self.initgroups(username, gid); end

  standard_method(
    {
      signal: Opus::Types.any(Integer, Symbol, String),
      pids: Integer,
    },
    returns: Integer
  )
  def self.kill(signal, *pids); end

  standard_method(
    {},
    returns: Integer
  )
  def self.maxgroups(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def self.maxgroups=(_); end

  standard_method(
    {},
    returns: Integer
  )
  def self.pid(); end

  standard_method(
    {},
    returns: Integer
  )
  def self.ppid(); end

  standard_method(
    {
      kind: Integer,
      _: Integer,
      priority: Integer,
    },
    returns: Integer
  )
  def self.setpriority(kind, _, priority); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def self.setproctitle(_); end

  standard_method(
    {
      resource: Opus::Types.any(Symbol, String, Integer),
      cur_limit: Integer,
      max_limit: Integer,
    },
    returns: NilClass
  )
  def self.setrlimit(resource, cur_limit, max_limit=_); end

  standard_method(
    {
      pid: Integer,
      _: Integer,
    },
    returns: Integer
  )
  def self.setpgid(pid, _); end

  standard_method(
    {},
    returns: Integer
  )
  def self.setsid(); end

  standard_method(
    {},
    returns: Process::Tms
  )
  def self.times(); end

  standard_method(
    {},
    returns: Integer
  )
  def self.uid(); end

  standard_method(
    {
      user: Integer,
    },
    returns: Integer
  )
  def self.uid=(user); end

  standard_method(
    {
      pid: Integer,
      flags: Integer,
    },
    returns: Integer
  )
  def self.wait(pid=_, flags=_); end

  standard_method(
    {
      pid: Integer,
      flags: Integer,
    },
    returns: [Integer, Integer]
  )
  def self.wait2(pid=_, flags=_); end

  standard_method(
    {},
    returns: Opus::Types.array_of([Integer, Integer])
  )
  def self.waitall(); end

  standard_method(
    {
      pid: Integer,
      flags: Integer,
    },
    returns: Integer
  )
  def self.waitpid(pid=_, flags=_); end

  standard_method(
    {
      pid: Integer,
      flags: Integer,
    },
    returns: [Integer, Integer]
  )
  def self.waitpid2(pid=_, flags=_); end
end

module Process::GID
  standard_method(
    {
      group: Integer,
    },
    returns: Integer
  )
  def self.change_privilege(group); end

  standard_method(
    {},
    returns: Integer
  )
  def self.eid(); end

  standard_method(
    {
      name: String,
    },
    returns: Integer
  )
  def self.from_name(name); end

  standard_method(
    {
      group: Integer,
    },
    returns: Integer
  )
  def self.grant_privilege(group); end

  standard_method(
    {},
    returns: Integer
  )
  def self.re_exchange(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.re_exchangeable?(); end

  standard_method(
    {},
    returns: Integer
  )
  def self.rid(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.sid_available?(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Opus::Types.untyped)
  )
  def self.switch(); end

  standard_method(
    {
      group: Integer,
    },
    returns: Integer
  )
  def self.eid=(group); end
end

class Process::Status
  standard_method(
    {
      num: Integer,
    },
    returns: Integer
  )
  def &(num); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(other); end

  standard_method(
    {
      num: Integer,
    },
    returns: Integer
  )
  def >>(num); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def coredump?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def exited?(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def exitstatus(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Integer
  )
  def pid(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def signaled?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def stopped?(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def stopsig(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def success?(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def termsig(); end

  standard_method(
    {},
    returns: Integer
  )
  def to_i(); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end
end

module Process::Sys
  standard_method(
    {},
    returns: Integer
  )
  def self.geteuid(); end

  standard_method(
    {},
    returns: Integer
  )
  def self.getgid(); end

  standard_method(
    {},
    returns: Integer
  )
  def self.getuid(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.issetugid(); end

  standard_method(
    {
      group: Integer,
    },
    returns: NilClass
  )
  def self.setegid(group); end

  standard_method(
    {
      user: Integer,
    },
    returns: NilClass
  )
  def self.seteuid(user); end

  standard_method(
    {
      group: Integer,
    },
    returns: NilClass
  )
  def self.setgid(group); end

  standard_method(
    {
      rid: Integer,
      eid: Integer,
    },
    returns: NilClass
  )
  def self.setregid(rid, eid); end

  standard_method(
    {
      rid: Integer,
      eid: Integer,
      sid: Integer,
    },
    returns: NilClass
  )
  def self.setresgid(rid, eid, sid); end

  standard_method(
    {
      rid: Integer,
      eid: Integer,
      sid: Integer,
    },
    returns: NilClass
  )
  def self.setresuid(rid, eid, sid); end

  standard_method(
    {
      rid: Integer,
      eid: Integer,
    },
    returns: NilClass
  )
  def self.setreuid(rid, eid); end

  standard_method(
    {
      group: Integer,
    },
    returns: NilClass
  )
  def self.setrgid(group); end

  standard_method(
    {
      user: Integer,
    },
    returns: NilClass
  )
  def self.setruid(user); end

  standard_method(
    {
      user: Integer,
    },
    returns: NilClass
  )
  def self.setuid(user); end
end

module Process::UID
  standard_method(
    {
      user: Integer,
    },
    returns: Integer
  )
  def self.change_privilege(user); end

  standard_method(
    {},
    returns: Integer
  )
  def self.eid(); end

  standard_method(
    {
      name: String,
    },
    returns: Integer
  )
  def self.from_name(name); end

  standard_method(
    {
      user: Integer,
    },
    returns: Integer
  )
  def self.grant_privilege(user); end

  standard_method(
    {},
    returns: Integer
  )
  def self.re_exchange(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.re_exchangeable?(); end

  standard_method(
    {},
    returns: Integer
  )
  def self.rid(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.sid_available?(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Opus::Types.untyped)
  )
  def self.switch(); end

  standard_method(
    {
      user: Integer,
    },
    returns: Integer
  )
  def self.eid=(user); end
end

class Process::Waiter
  standard_method(
    {},
    returns: Integer
  )
  def pid(); end
end

class Random
  standard_method(
    {
      seed: Integer,
    },
    returns: Object
  )
  def initialize(seed=_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end

  standard_method(
    {
      size: Integer,
    },
    returns: String
  )
  def bytes(size); end

  standard_method(
    {
      max: Opus::Types.any(Integer, Range, Float),
    },
    returns: Opus::Types.any(Integer, Float)
  )
  def rand(max=_); end

  standard_method(
    {},
    returns: Integer
  )
  def seed(); end

  standard_method(
    {},
    returns: Integer
  )
  def self.new_seed(); end

  standard_method(
    {
      max: Integer,
    },
    returns: Numeric
  )
  def self.rand(max=_); end

  standard_method(
    {
      number: Integer,
    },
    returns: Numeric
  )
  def self.srand(number=_); end
end

class Range
  standard_method(
    {
      _begin: Integer,
      _end: Integer,
      exclude_end: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Object
  )
  def initialize(_begin, _end, exclude_end=_); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(obj); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(obj); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def begin(); end

  standard_method(
    {},
    returns: Opus::Types.any(BasicObject, NilClass)
  )
  def bsearch(); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def cover?(obj); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def end(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def exclude_end?(); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def first(n=_); end

  standard_method(
    {},
    returns: Integer
  )
  def hash(); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(obj); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def last(n=_); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def max(n=_); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def min(n=_); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def size(); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def step(n=_); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(obj); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def member?(obj); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal)
  )
  def %(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def *(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def +(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def -(_); end

  standard_method(
    {},
    returns: Rational
  )
  def -@(); end

  standard_method(
    {},
    returns: Rational
  )
  def +@(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def **(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def /(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Object
  )
  def <=>(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end

  standard_method(
    {},
    returns: Rational
  )
  def abs(); end

  standard_method(
    {},
    returns: Rational
  )
  def abs2(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Integer
  )
  def div(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal)
  )
  def modulo(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def ceil(_=_); end

  standard_method(
    {},
    returns: Integer
  )
  def denominator(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def divmod(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Float
  )
  def fdiv(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def floor(_=_); end

  standard_method(
    {},
    returns: Integer
  )
  def hash(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Integer
  )
  def numerator(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def quo(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Rational
  )
  def rationalize(_=_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(_=_); end

  standard_method(
    {},
    returns: Float
  )
  def to_f(); end

  standard_method(
    {},
    returns: Integer
  )
  def to_i(); end

  standard_method(
    {},
    returns: Rational
  )
  def to_r(); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Integer, Rational)
  )
  def truncate(_=_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end

  standard_method(
    {},
    returns: Rational
  )
  def conj(); end

  standard_method(
    {},
    returns: Rational
  )
  def conjugate(); end

  standard_method(
    {},
    returns: Integer
  )
  def imag(); end

  standard_method(
    {},
    returns: Integer
  )
  def imaginary(); end

  standard_method(
    {},
    returns: Rational
  )
  def real(); end

  standard_method(
    {},
    returns: TrueClass
  )
  def real?(); end

  standard_method(
    {},
    returns: Complex
  )
  def to_c(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, Complex),
    },
    returns: Opus::Types.any([Rational, Rational], [Float, Float], [Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)])
  )
  def coerce(_); end
end

class Regexp
  standard_method(
    {
      _: Opus::Types.any(String, Symbol),
    },
    returns: String
  )
  def self.escape(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(MatchData, String)
  )
  def self.last_match(_=_); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(Regexp, NilClass)
  )
  def self.try_convert(obj); end

  standard_method(
    {
      _: Opus::Types.any(String, Regexp),
      options: BasicObject,
      kcode: String,
    },
    returns: Object
  )
  def initialize(_, options=_, kcode=_); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(other); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(other); end

  standard_method(
    {
      str: String,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def =~(str); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def casefold?(); end

  standard_method(
    {},
    returns: Encoding
  )
  def encoding(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def fixed_encoding?(); end

  standard_method(
    {},
    returns: Integer
  )
  def hash(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {
      _: String,
      _1: Integer,
    },
    returns: Opus::Types.any(MatchData, NilClass)
  )
  def match(_, _1=_); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: String, values: Opus::Types.array_of(Integer))
  )
  def named_captures(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(String)
  )
  def names(); end

  standard_method(
    {},
    returns: Integer
  )
  def options(); end

  standard_method(
    {},
    returns: String
  )
  def source(); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def ~(); end

  standard_method(
    {
      _: Opus::Types.any(String, Regexp),
      options: BasicObject,
      kcode: String,
    },
    returns: Opus::Types.untyped
  )
  def self.compile(_, options=_, kcode=_); end

  standard_method(
    {
      _: Opus::Types.any(String, Symbol),
    },
    returns: String
  )
  def self.quote(_); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(other); end
end

class Set
  standard_method(
    {
      ary: BasicObject,
    },
    returns: Set
  )
  def self.[](*ary); end

  standard_method(
    {
      enum: Enumerable,
    },
    returns: Object
  )
  def initialize(enum=_); end

  standard_method(
    {
      enum: Enumerable,
    },
    returns: Set
  )
  def +(enum); end

  standard_method(
    {
      enum: Enumerable,
    },
    returns: Set
  )
  def ^(enum); end

  standard_method(
    {
      o: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def add(o); end

  standard_method(
    {
      o: BasicObject,
    },
    returns: Opus::Types.any(BasicObject, NilClass)
  )
  def add?(o); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Set)
  )
  def classify(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def clear(); end

  standard_method(
    {
      o: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def delete(o); end

  standard_method(
    {
      o: BasicObject,
    },
    returns: Opus::Types.any(BasicObject, NilClass)
  )
  def delete?(o); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def delete_if(); end

  standard_method(
    {
      enum: Enumerable,
    },
    returns: Set
  )
  def difference(enum); end

  standard_method(
    {
      set: Set,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def disjoint?(set); end

  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end

  standard_method(
    {},
    returns: Opus::Types.any(BasicObject, NilClass)
  )
  def flatten!(); end

  standard_method(
    {},
    returns: Set
  )
  def flatten(); end

  standard_method(
    {
      set: Set,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def intersect?(set); end

  standard_method(
    {
      enum: Enumerable,
    },
    returns: Set
  )
  def intersection(enum); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def keep_if(); end

  standard_method(
    {},
    returns: Set
  )
  def map!(); end

  standard_method(
    {
      o: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def member?(o); end

  standard_method(
    {
      enum: Enumerable,
    },
    returns: Opus::Types.untyped
  )
  def merge(enum); end

  standard_method(
    {
      set: Set,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def proper_subset?(set); end

  standard_method(
    {
      set: Set,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def proper_superset?(set); end

  standard_method(
    {},
    returns: Opus::Types.any(BasicObject, NilClass)
  )
  def reject!(); end

  standard_method(
    {
      enum: Enumerable,
    },
    returns: Set
  )
  def replace(enum); end

  standard_method(
    {},
    returns: Opus::Types.any(BasicObject, NilClass)
  )
  def select!(); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end

  standard_method(
    {
      set: Set,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def subset?(set); end

  standard_method(
    {
      enum: Enumerable,
    },
    returns: Opus::Types.untyped
  )
  def subtract(enum); end

  standard_method(
    {
      set: Set,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def superset?(set); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def to_a(); end

  standard_method(
    {
      enum: Enumerable,
    },
    returns: Set
  )
  def &(enum); end

  standard_method(
    {
      enum: Enumerable,
    },
    returns: Set
  )
  def -(enum); end

  standard_method(
    {
      set: Set,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <(set); end

  standard_method(
    {
      o: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def <<(o); end

  standard_method(
    {
      set: Set,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=(set); end

  standard_method(
    {
      set: Set,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >(set); end

  standard_method(
    {
      set: Set,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=(set); end

  standard_method(
    {},
    returns: Set
  )
  def collect!(); end

  standard_method(
    {
      o: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(o); end

  standard_method(
    {},
    returns: Integer
  )
  def length(); end

  standard_method(
    {
      enum: Enumerable,
    },
    returns: Set
  )
  def |(enum); end

  standard_method(
    {
      enum: Enumerable,
    },
    returns: Set
  )
  def union(enum); end
end

class String
  standard_method(
    {
      str: String,
    },
    returns: Object
  )
  def initialize(str=_); end

  standard_method(
    {
      _: Object,
    },
    returns: String
  )
  def %(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: String
  )
  def *(_); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def +(_); end

  standard_method(
    {
      _: Object,
    },
    returns: String
  )
  def <<(_); end

  standard_method(
    {
      other: String,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(other); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(_); end

  standard_method(
    {
      _: Object,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def =~(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Range, Regexp, String),
      _1: Opus::Types.any(Integer, String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def [](_, _1=_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ascii_only?(); end

  standard_method(
    {},
    returns: String
  )
  def b(); end

  standard_method(
    {},
    returns: Array
  )
  def bytes(); end

  standard_method(
    {},
    returns: Integer
  )
  def bytesize(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Range),
      _1: Integer,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def byteslice(_, _1=_); end

  standard_method(
    {},
    returns: String
  )
  def capitalize(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def capitalize!(); end

  standard_method(
    {
      _: String,
    },
    returns: Opus::Types.any(NilClass, Integer)
  )
  def casecmp(_); end

  standard_method(
    {
      _: Integer,
      _1: String,
    },
    returns: String
  )
  def center(_, _1=_); end

  standard_method(
    {},
    returns: Array
  )
  def chars(); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def chomp(_=_); end

  standard_method(
    {
      _: String,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def chomp!(_=_); end

  standard_method(
    {},
    returns: String
  )
  def chop(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def chop!(); end

  standard_method(
    {},
    returns: String
  )
  def chr(); end

  standard_method(
    {},
    returns: String
  )
  def clear(); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Integer)
  )
  def codepoints(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Object),
    },
    returns: String
  )
  def concat(_); end

  standard_method(
    {
      _: String,
      _1: String,
    },
    returns: Integer
  )
  def count(_, *_1); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def crypt(_); end

  standard_method(
    {
      _: String,
      _1: String,
    },
    returns: String
  )
  def delete(_, *_1); end

  standard_method(
    {
      _: String,
      _1: String,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def delete!(_, *_1); end

  standard_method(
    {},
    returns: String
  )
  def downcase(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def downcase!(); end

  standard_method(
    {},
    returns: String
  )
  def dump(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_byte(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_char(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_codepoint(); end

  standard_method(
    {
      _: String,
    },
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_line(_=_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end

  standard_method(
    {},
    returns: Encoding
  )
  def encoding(); end

  standard_method(
    {
      _: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def end_with?(*_); end

  standard_method(
    {
      _: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end

  standard_method(
    {
      _: Opus::Types.any(String, Encoding),
    },
    returns: String
  )
  def force_encoding(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def getbyte(_); end

  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(String, Hash),
    },
    returns: Opus::Types.any(String, Enumerator)
  )
  def gsub(_, _1=_); end

  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: String,
    },
    returns: Opus::Types.any(String, NilClass, Enumerator)
  )
  def gsub!(_, _1=_); end

  standard_method(
    {},
    returns: Integer
  )
  def hash(); end

  standard_method(
    {},
    returns: Integer
  )
  def hex(); end

  standard_method(
    {
      _: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(_); end

  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Integer,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def index(_, _1=_); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def replace(_); end

  standard_method(
    {
      _: Integer,
      _1: String,
    },
    returns: String
  )
  def insert(_, _1); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Symbol
  )
  def intern(); end

  standard_method(
    {},
    returns: Integer
  )
  def length(); end

  standard_method(
    {
      _: String,
    },
    returns: Opus::Types.array_of(String)
  )
  def lines(_=_); end

  standard_method(
    {
      _: Integer,
      _1: String,
    },
    returns: String
  )
  def ljust(_, _1=_); end

  standard_method(
    {},
    returns: String
  )
  def lstrip(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def lstrip!(); end

  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Integer,
    },
    returns: MatchData
  )
  def match(_, _1=_); end

  standard_method(
    {},
    returns: String
  )
  def next(); end

  standard_method(
    {},
    returns: String
  )
  def next!(); end

  standard_method(
    {},
    returns: Integer
  )
  def oct(); end

  standard_method(
    {},
    returns: Integer
  )
  def ord(); end

  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
    },
    returns: Opus::Types.array_of(String)
  )
  def partition(_); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def prepend(_); end

  standard_method(
    {},
    returns: String
  )
  def reverse(); end

  standard_method(
    {
      _: Opus::Types.any(String, Regexp),
      _1: Integer,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def rindex(_, _1=_); end

  standard_method(
    {
      _: Integer,
      _1: String,
    },
    returns: String
  )
  def rjust(_, _1=_); end

  standard_method(
    {
      _: Opus::Types.any(String, Regexp),
    },
    returns: Opus::Types.array_of(String)
  )
  def rpartition(_); end

  standard_method(
    {},
    returns: String
  )
  def rstrip(); end

  standard_method(
    {},
    returns: String
  )
  def rstrip!(); end

  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
    },
    returns: Opus::Types.array_of(Opus::Types.any(String, Opus::Types.array_of(String)))
  )
  def scan(_); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def scrub(_=_); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def scrub!(_=_); end

  standard_method(
    {
      _: Integer,
      _1: Integer,
    },
    returns: Integer
  )
  def setbyte(_, _1); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Range, Regexp, String),
      _1: Opus::Types.any(Integer, String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def slice!(_, _1=_); end

  standard_method(
    {
      _: Opus::Types.any(Regexp, String, Integer),
      _1: Integer,
    },
    returns: Opus::Types.array_of(String)
  )
  def split(_=_, _1=_); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def squeeze(_=_); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def squeeze!(_=_); end

  standard_method(
    {
      _: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def start_with?(*_); end

  standard_method(
    {},
    returns: String
  )
  def strip(); end

  standard_method(
    {},
    returns: String
  )
  def strip!(); end

  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(String, Hash),
    },
    returns: String
  )
  def sub(_, _1=_); end

  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: String,
    },
    returns: String
  )
  def sub!(_, _1=_); end

  standard_method(
    {},
    returns: String
  )
  def succ(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def sum(_=_); end

  standard_method(
    {},
    returns: String
  )
  def swapcase(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def swapcase!(); end

  standard_method(
    {},
    returns: Complex
  )
  def to_c(); end

  standard_method(
    {},
    returns: Float
  )
  def to_f(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def to_i(_=_); end

  standard_method(
    {},
    returns: Rational
  )
  def to_r(); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {},
    returns: String
  )
  def to_str(); end

  standard_method(
    {},
    returns: Symbol
  )
  def to_sym(); end

  standard_method(
    {
      _: String,
      _1: String,
    },
    returns: String
  )
  def tr(_, _1); end

  standard_method(
    {
      _: String,
      _1: String,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def tr!(_, _1); end

  standard_method(
    {
      _: String,
      _1: String,
    },
    returns: String
  )
  def tr_s(_, _1); end

  standard_method(
    {
      _: String,
      _1: String,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def tr_s!(_, _1); end

  standard_method(
    {
      _: String,
    },
    returns: Opus::Types.array_of(String)
  )
  def unpack(_); end

  standard_method(
    {},
    returns: String
  )
  def upcase(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def upcase!(); end

  standard_method(
    {
      _: String,
      _1: BasicObject,
    },
    returns: Opus::Types.any(Enumerator, String)
  )
  def upto(_, _1=_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def valid_encoding?(); end

  standard_method(
    {
      obj: Object,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def self.try_convert(obj); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Range, Regexp, String),
      _1: Opus::Types.any(Integer, String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def slice(_, _1=_); end
end

class StringScanner
  standard_method(
    {
      _: String,
      _1: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: StringScanner
  )
  def self.new(_, _1=_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eos?(); end

  standard_method(
    {
      _: Regexp,
    },
    returns: String
  )
  def scan(_); end

  standard_method(
    {},
    returns: String
  )
  def getch(); end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.array_of(Symbol)
  )
  def self.all_symbols(); end

  standard_method(
    {
      other: Symbol,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(other); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(obj); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def =~(obj); end

  standard_method(
    {
      idx: Opus::Types.any(Integer, Range),
      n: Integer,
    },
    returns: String
  )
  def [](idx, n=_); end

  standard_method(
    {},
    returns: Symbol
  )
  def capitalize(); end

  standard_method(
    {
      other: Symbol,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def casecmp(other); end

  standard_method(
    {},
    returns: Symbol
  )
  def downcase(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end

  standard_method(
    {},
    returns: Encoding
  )
  def encoding(); end

  standard_method(
    {},
    returns: String
  )
  def id2name(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def intern(); end

  standard_method(
    {},
    returns: Integer
  )
  def length(); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def match(obj); end

  standard_method(
    {},
    returns: Symbol
  )
  def succ(); end

  standard_method(
    {},
    returns: Symbol
  )
  def swapcase(); end

  standard_method(
    {},
    returns: Proc
  )
  def to_proc(); end

  standard_method(
    {},
    returns: Symbol
  )
  def upcase(); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end

  standard_method(
    {
      idx: Opus::Types.any(Integer, Range),
      n: Integer,
    },
    returns: String
  )
  def slice(idx, n=_); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def to_sym(); end
end

class Time
  standard_method(
    {
      _: Opus::Types.any(Time, Numeric),
      microseconds_with_frac: Numeric,
    },
    returns: Time
  )
  def self.at(_, microseconds_with_frac=_); end

  standard_method(
    {
      year: Integer,
      month: Opus::Types.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
    },
    returns: Time
  )
  def self.gm(year, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  standard_method(
    {
      year: Integer,
      month: Opus::Types.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
    },
    returns: Time
  )
  def self.local(year, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  standard_method(
    {},
    returns: Time
  )
  def self.now(); end

  standard_method(
    {
      year: Integer,
      month: Opus::Types.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
    },
    returns: Time
  )
  def self.utc(year, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  standard_method(
    {
      year: Integer,
      month: Opus::Types.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
    },
    returns: Object
  )
  def initialize(year=_, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  standard_method(
    {
      _: Numeric,
    },
    returns: Time
  )
  def +(_); end

  standard_method(
    {
      _: Opus::Types.any(Time, Numeric),
    },
    returns: Opus::Types.any(Float, Time)
  )
  def -(_); end

  standard_method(
    {
      other: Time,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(other); end

  standard_method(
    {},
    returns: String
  )
  def asctime(); end

  standard_method(
    {},
    returns: String
  )
  def ctime(); end

  standard_method(
    {},
    returns: Integer
  )
  def day(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def dst?(); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def friday?(); end

  standard_method(
    {},
    returns: Time
  )
  def getgm(); end

  standard_method(
    {
      utc_offset: Integer,
    },
    returns: Time
  )
  def getlocal(utc_offset=_); end

  standard_method(
    {},
    returns: Time
  )
  def getutc(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def gmt?(); end

  standard_method(
    {},
    returns: Integer
  )
  def gmt_offset(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def gmtime(); end

  standard_method(
    {},
    returns: Integer
  )
  def hash(); end

  standard_method(
    {},
    returns: Integer
  )
  def hour(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def isdst(); end

  standard_method(
    {
      utc_offset: String,
    },
    returns: Opus::Types.untyped
  )
  def localtime(utc_offset=_); end

  standard_method(
    {},
    returns: Integer
  )
  def mday(); end

  standard_method(
    {},
    returns: Integer
  )
  def min(); end

  standard_method(
    {},
    returns: Integer
  )
  def mon(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def monday?(); end

  standard_method(
    {},
    returns: Integer
  )
  def nsec(); end

  standard_method(
    {
      _: Integer,
    },
    returns: Time
  )
  def round(_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def saturday?(); end

  standard_method(
    {},
    returns: Integer
  )
  def sec(); end

  standard_method(
    {
      _: String,
    },
    returns: String
  )
  def strftime(_); end

  standard_method(
    {},
    returns: Numeric
  )
  def subsec(); end

  standard_method(
    {},
    returns: Time
  )
  def succ(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sunday?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def thursday?(); end

  standard_method(
    {},
    returns: [Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer, Opus::Types.any(TrueClass, FalseClass), String]
  )
  def to_a(); end

  standard_method(
    {},
    returns: Float
  )
  def to_f(); end

  standard_method(
    {},
    returns: Numeric
  )
  def to_i(); end

  standard_method(
    {},
    returns: Rational
  )
  def to_r(); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def tuesday?(); end

  standard_method(
    {},
    returns: Numeric
  )
  def tv_nsec(); end

  standard_method(
    {},
    returns: Numeric
  )
  def tv_sec(); end

  standard_method(
    {},
    returns: Numeric
  )
  def tv_usec(); end

  standard_method(
    {},
    returns: Numeric
  )
  def usec(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def utc(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def utc?(); end

  standard_method(
    {},
    returns: Integer
  )
  def utc_offset(); end

  standard_method(
    {},
    returns: Integer
  )
  def wday(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def wednesday?(); end

  standard_method(
    {},
    returns: Integer
  )
  def yday(); end

  standard_method(
    {},
    returns: Integer
  )
  def year(); end

  standard_method(
    {},
    returns: String
  )
  def zone(); end

  standard_method(
    {
      year: Integer,
      month: Opus::Types.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
    },
    returns: Time
  )
  def self.mktime(year, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  standard_method(
    {},
    returns: Integer
  )
  def gmtoff(); end

  standard_method(
    {},
    returns: Integer
  )
  def month(); end
end

module URI
  standard_method(
    {
      str: String,
      enc: Encoding,
    },
    returns: Opus::Types.array_of([String, String])
  )
  def self.decode_www_form_component(str, enc=_); end

  standard_method(
    {
      str: String,
      schemes: Array,
    },
    returns: Opus::Types.array_of(String)
  )
  def self.extract(str, schemes=_); end

  standard_method(
    {
      str: String,
    },
    returns: URI::HTTP
  )
  def self.join(*str); end

  standard_method(
    {
      uri: String,
    },
    returns: URI::HTTP
  )
  def self.parse(uri); end

  standard_method(
    {
      schemes: Array,
    },
    returns: Opus::Types.array_of(String)
  )
  def self.regexp(schemes=_); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: String, values: Class)
  )
  def self.scheme_list(); end

  standard_method(
    {
      uri: String,
    },
    returns: Opus::Types.array_of(Opus::Types.any(String, NilClass))
  )
  def self.split(uri); end

  standard_method(
    {
      arg: String,
      _: Opus::Types.any(Regexp, String),
    },
    returns: String
  )
  def self.escape(arg, *_); end

  standard_method(
    {
      arg: String,
    },
    returns: String
  )
  def self.unescape(*arg); end

  standard_method(
    {
      arg: String,
      _: Opus::Types.any(Regexp, String),
    },
    returns: String
  )
  def self.encode(arg, *_); end

  standard_method(
    {
      arg: String,
    },
    returns: String
  )
  def self.decode(*arg); end
end

module YAML
  standard_method(
    {
      filename: String,
    },
    returns: Opus::Types.array_of(String)
  )
  def self.load_file(filename); end
end
