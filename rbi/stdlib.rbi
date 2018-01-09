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
class Set < Object
  include Enumerable
end
module Signal
end
module SingleForwardable
end
class SortedSet < Set
end
class String < Object
  include Comparable
end
class StringIO < Data
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
      arg0: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def <<(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Range, Integer, Float),
      arg1: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Opus::Types.untyped)
  )
  def [](arg0, arg1=_); end

  standard_method(
    {
      arg0: Opus::Types.array_of(BasicObject),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def &(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, String),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), String)
  )
  def *(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Enumerable, Opus::Types.array_of(BasicObject)),
    },
    returns: Opus::Types.array_of(BasicObject)
  )
  def +(arg0); end

  standard_method(
    {
      arg0: Opus::Types.array_of(BasicObject),
    },
    returns: Opus::Types.array_of(BasicObject)
  )
  def -(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Range),
      arg1: Opus::Types.any(BasicObject, Integer),
      arg2: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def []=(arg0, arg1, arg2=_); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def assoc(arg0); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.untyped
  )
  def at(arg0); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def clear(); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def map(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def map!(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def collect(&blk); end

  standard_method(
    {
      arg0: Integer,
      blk: Opus::Types.proc([Opus::Types.array_of(Opus::Types.untyped)], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def combination(arg0, &blk); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def push(*arg0); end

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
      arg0: Opus::Types.array_of(BasicObject),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def concat(arg0); end

  standard_method(
    {
      arg0: BasicObject,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Integer
  )
  def count(arg0=_, &blk); end

  standard_method(
    {
      arg0: Integer,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def cycle(arg0=_, &blk); end

  standard_method(
    {
      arg0: BasicObject,
      blk: Opus::Types.proc([], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, BasicObject)
  )
  def delete(arg0, &blk); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def delete_at(arg0); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def delete_if(&blk); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def drop(arg0); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def drop_while(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Enumerator, Opus::Types.array_of(Opus::Types.untyped))
  )
  def each(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def each_index(&blk); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end

  standard_method(
    {
      arg0: Integer,
      arg1: BasicObject,
      blk: Opus::Types.proc([Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, BasicObject)
  )
  def fetch(arg0, arg1=_, &blk); end

  standard_method(
    {
      arg0: Opus::Types.any(BasicObject, Integer, Range),
      arg1: Opus::Types.any(Integer, Range),
      arg2: Integer,
      blk: Opus::Types.proc([Integer], returns: BasicObject),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def fill(arg0=_, arg1=_, arg2=_, &blk); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def flatten(); end

  standard_method(
    {
      arg0: BasicObject,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Integer, Enumerator)
  )
  def index(arg0=_, &blk); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def first(arg0=_); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(arg0); end

  standard_method(
    {
      arg0: Integer,
      arg1: BasicObject,
    },
    returns: Object
  )
  def initialize(arg0=_, arg1=_); end

  standard_method(
    {
      arg0: Integer,
      arg1: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def insert(arg0, *arg1); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {
      arg0: String,
    },
    returns: String
  )
  def join(arg0=_); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def keep_if(&blk); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def last(arg0=_); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def member?(arg0); end

  standard_method(
    {},
    returns: Integer
  )
  def length(); end

  standard_method(
    {
      arg0: Integer,
      blk: Opus::Types.proc([Opus::Types.array_of(Opus::Types.untyped)], returns: BasicObject),
    },
    returns: Opus::Types.any(Enumerator, Opus::Types.array_of(Opus::Types.untyped))
  )
  def permutation(arg0=_, &blk); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Opus::Types.untyped)
  )
  def pop(arg0=_); end

  standard_method(
    {
      arg0: Opus::Types.array_of(BasicObject),
    },
    returns: Opus::Types.array_of(Opus::Types.array_of(BasicObject))
  )
  def product(*arg0); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def rassoc(arg0); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def reject(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def reject!(&blk); end

  standard_method(
    {
      arg0: Integer,
      blk: Opus::Types.proc([Opus::Types.array_of(Opus::Types.untyped)], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def repeated_combination(arg0, &blk); end

  standard_method(
    {
      arg0: Integer,
      blk: Opus::Types.proc([Opus::Types.array_of(Opus::Types.untyped)], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def repeated_permutation(arg0, &blk); end

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
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def reverse_each(&blk); end

  standard_method(
    {
      arg0: BasicObject,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Integer, Enumerator)
  )
  def rindex(arg0=_, &blk); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def rotate(arg0=_); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def rotate!(arg0=_); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def sample(arg0=_); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def select(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def select!(&blk); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def shift(arg0=_); end

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
      arg0: Opus::Types.any(Range, Integer, Float),
      arg1: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Opus::Types.untyped)
  )
  def slice!(arg0, arg1=_); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Integer),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def sort(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Integer),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def sort!(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def sort_by!(&blk); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def take(arg0); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def take_while(&blk); end

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
      arg0: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def unshift(*arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Range, Integer),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def values_at(*arg0); end

  standard_method(
    {
      arg0: Opus::Types.array_of(BasicObject),
    },
    returns: Opus::Types.array_of(Opus::Types.array_of(BasicObject))
  )
  def zip(*arg0); end

  standard_method(
    {
      arg0: Opus::Types.array_of(BasicObject),
    },
    returns: Opus::Types.array_of(BasicObject)
  )
  def |(arg0); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end

  standard_method(
    {
      arg0: Opus::Types.any(Range, Integer, Float),
      arg1: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Opus::Types.untyped)
  )
  def slice(arg0, arg1=_); end

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
      arg0: String,
      filename: String,
      lineno: Integer,
      blk: Opus::Types.proc([], returns: BasicObject),
    },
    returns: Opus::Types.untyped
  )
  def instance_eval(arg0=_, filename=_, lineno=_, &blk); end

  standard_method(
    {
      args: BasicObject,
      blk: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def instance_exec(*args, &blk); end

  standard_method(
    {
      arg0: Symbol,
      arg1: BasicObject,
    },
    returns: BasicObject
  )
  def __send__(arg0, *arg1); end

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
      blk: Opus::Types.proc([Process], returns: NilClass),
    },
    returns: Opus::Types.array_of(Benchmark::Tms)
  )
  def self.bm(label_width=_, *labels, &blk); end

  standard_method(
    {
      width: Integer,
      blk: Opus::Types.proc([Process], returns: NilClass),
    },
    returns: Opus::Types.array_of(Benchmark::Tms)
  )
  def self.bmbm(width=_, &blk); end

  standard_method(
    {
      label: String,
    },
    returns: Benchmark::Tms
  )
  def self.measure(label=_); end

  standard_method(
    {
      blk: BasicObject,
    },
    returns: Integer
  )
  def self.realtime(&blk); end
end

class BigDecimal
  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: BigDecimal
  )
  def %(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def +(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def -(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def *(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: BigDecimal
  )
  def **(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def /(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Object
  )
  def <=>(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Integer
  )
  def div(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def divmod(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def fdiv(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: BigDecimal
  )
  def modulo(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def quo(arg0); end

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
      arg0: Integer,
    },
    returns: Opus::Types.any(Integer, BigDecimal)
  )
  def round(arg0=_); end

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
      arg0: Integer,
    },
    returns: Opus::Types.any(Integer, Rational)
  )
  def truncate(arg0=_); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: BigDecimal
  )
  def remainder(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: BigDecimal
  )
  def power(arg0); end

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
      arg0: Integer,
    },
    returns: BigDecimal
  )
  def sqrt(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      arg1: Integer,
    },
    returns: BigDecimal
  )
  def add(arg0, arg1); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      arg1: Integer,
    },
    returns: BigDecimal
  )
  def sub(arg0, arg1); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      arg1: Integer,
    },
    returns: BigDecimal
  )
  def mult(arg0, arg1); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: [BigDecimal, BigDecimal]
  )
  def coerce(arg0); end
end

module BigMath
  standard_method(
    {
      arg0: Integer,
      arg1: Integer,
    },
    returns: BigDecimal
  )
  def self.exp(arg0, arg1); end

  standard_method(
    {
      arg0: Integer,
      arg1: Integer,
    },
    returns: BigDecimal
  )
  def self.log(arg0, arg1); end

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
      blk: Opus::Types.proc([Opus::Types.array_of(String)], returns: BasicObject),
    },
    returns: NilClass
  )
  def self.foreach(path, options=_, &blk); end
end

class Class
  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def allocate(); end

  standard_method(
    {
      arg0: Class,
    },
    returns: Opus::Types.untyped
  )
  def inherited(arg0); end

  standard_method(
    {},
    returns: Opus::Types.any(Class, NilClass)
  )
  def superclass(); end

  standard_method(
    {
      arg0: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def instance_methods(arg0=_); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Complex
  )
  def *(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Complex
  )
  def **(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Complex
  )
  def +(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Complex
  )
  def -(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Complex
  )
  def /(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(arg0); end

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
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Complex
  )
  def fdiv(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex, BigDecimal)
  )
  def quo(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Rational
  )
  def rationalize(arg0=_); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: [Complex, Complex]
  )
  def coerce(arg0); end
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
      arg0: Integer,
      arg1: Integer,
      arg2: Integer,
      arg3: Integer,
    },
    returns: Object
  )
  def initialize(arg0=_, arg1=_, arg2=_, arg3=_); end

  standard_method(
    {
      arg0: String,
    },
    returns: String
  )
  def strftime(arg0); end
end

class Dir
  standard_method(
    {
      arg0: Opus::Types.any(String, Pathname),
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(Integer, Opus::Types.untyped)
  )
  def self.chdir(arg0=_, &blk); end

  standard_method(
    {
      arg0: String,
    },
    returns: Integer
  )
  def self.chroot(arg0); end

  standard_method(
    {
      arg0: String,
    },
    returns: Integer
  )
  def self.delete(arg0); end

  standard_method(
    {
      arg0: String,
      arg1: Encoding,
    },
    returns: Opus::Types.array_of(String)
  )
  def self.entries(arg0, arg1=_); end

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
      arg0: Encoding,
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(NilClass, Enumerator)
  )
  def self.foreach(dir, arg0=_, &blk); end

  standard_method(
    {},
    returns: String
  )
  def self.getwd(); end

  standard_method(
    {
      pattern: Opus::Types.any(String, Opus::Types.array_of(String)),
      flags: Integer,
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.glob(pattern, flags=_, &blk); end

  standard_method(
    {
      arg0: String,
    },
    returns: String
  )
  def self.home(arg0=_); end

  standard_method(
    {
      arg0: String,
      arg1: Integer,
    },
    returns: Integer
  )
  def self.mkdir(arg0, arg1=_); end

  standard_method(
    {
      arg0: String,
      arg1: Encoding,
      blk: Opus::Types.proc([Dir], returns: BasicObject),
    },
    returns: Opus::Types.any(Dir, Opus::Types.untyped)
  )
  def self.open(arg0, arg1=_, &blk); end

  standard_method(
    {},
    returns: String
  )
  def self.pwd(); end

  standard_method(
    {
      arg0: String,
    },
    returns: Integer
  )
  def self.rmdir(arg0); end

  standard_method(
    {
      arg0: String,
    },
    returns: Integer
  )
  def self.unlink(arg0); end

  standard_method(
    {},
    returns: NilClass
  )
  def close(); end

  standard_method(
    {
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each(&blk); end

  standard_method(
    {},
    returns: Integer
  )
  def fileno(); end

  standard_method(
    {
      arg0: String,
      arg1: Encoding,
    },
    returns: Object
  )
  def initialize(arg0, arg1=_); end

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
      arg0: Integer,
    },
    returns: Integer
  )
  def pos=(arg0); end

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
      arg0: Integer,
    },
    returns: Opus::Types.untyped
  )
  def seek(arg0); end

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
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.[](pattern, flags=_, &blk); end
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
      arg0: Opus::Types.any(String, Encoding),
    },
    returns: Opus::Types.any(String, Encoding)
  )
  def self.default_external=(arg0); end

  standard_method(
    {},
    returns: Encoding
  )
  def self.default_internal(); end

  standard_method(
    {
      arg0: Opus::Types.any(String, Encoding),
    },
    returns: Opus::Types.any(String, NilClass, Encoding)
  )
  def self.default_internal=(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(String, Encoding),
    },
    returns: Encoding
  )
  def self.find(arg0); end

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
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def all?(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def any?(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def collect(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Enumerator),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def collect_concat(&blk); end

  standard_method(
    {
      arg0: BasicObject,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Integer
  )
  def count(arg0=_, &blk); end

  standard_method(
    {
      n: Integer,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(NilClass, Enumerator)
  )
  def cycle(n=_, &blk); end

  standard_method(
    {
      ifnone: Proc,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(BasicObject, NilClass, Enumerator)
  )
  def detect(ifnone=_, &blk); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def drop(n); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def drop_while(&blk); end

  standard_method(
    {
      n: Integer,
      blk: Opus::Types.proc([Opus::Types.array_of(Opus::Types.untyped)], returns: BasicObject),
    },
    returns: Opus::Types.any(NilClass, Enumerator)
  )
  def each_cons(n, &blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Integer], returns: BasicObject),
    },
    returns: Enumerable
  )
  def each_with_index(&blk); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def entries(); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def find_all(&blk); end

  standard_method(
    {
      value: BasicObject,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Integer, NilClass, Enumerator)
  )
  def find_index(value=_, &blk); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.any(BasicObject, NilClass, Opus::Types.array_of(BasicObject))
  )
  def first(n=_); end

  standard_method(
    {
      arg0: BasicObject,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def grep(arg0, &blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.array_of(Opus::Types.untyped)), Enumerator)
  )
  def group_by(&blk); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(arg0); end

  standard_method(
    {
      initial: Opus::Types.any(BasicObject, Symbol),
      arg0: Symbol,
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.untyped
  )
  def inject(initial=_, arg0=_, &blk); end

  standard_method(
    {
      arg0: Integer,
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Integer),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def max(arg0=_, &blk); end

  standard_method(
    {
      arg0: Integer,
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Integer),
    },
    returns: Opus::Types.any(Enumerator, Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def max_by(arg0=_, &blk); end

  standard_method(
    {
      arg0: Integer,
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Integer),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def min(arg0=_, &blk); end

  standard_method(
    {
      arg0: Integer,
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Integer),
    },
    returns: Opus::Types.any(Enumerator, Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def min_by(arg0=_, &blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Integer),
    },
    returns: [BasicObject, BasicObject]
  )
  def minmax(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Integer),
    },
    returns: Opus::Types.any([BasicObject, BasicObject], Enumerator)
  )
  def minmax_by(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def none?(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def one?(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any([Opus::Types.array_of(BasicObject), Opus::Types.array_of(BasicObject)], Enumerator)
  )
  def partition(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def reject(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Enumerator
  )
  def reverse_each(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Integer),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def sort(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def sort_by(&blk); end

  standard_method(
    {
      n: Integer,
    },
    returns: Opus::Types.any(Opus::Types.array_of(BasicObject), NilClass)
  )
  def take(n); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def take_while(&blk); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def to_h(); end

  standard_method(
    {
      n: Integer,
      blk: Opus::Types.proc([Opus::Types.array_of(Opus::Types.untyped)], returns: BasicObject),
    },
    returns: Opus::Types.any(NilClass, Enumerator)
  )
  def each_slice(n, &blk); end

  standard_method(
    {
      ifnone: Proc,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(BasicObject, NilClass, Enumerator)
  )
  def find(ifnone=_, &blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Enumerator),
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def flat_map(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def map(&blk); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def member?(arg0); end

  standard_method(
    {
      initial: Opus::Types.any(BasicObject, Symbol),
      arg0: Symbol,
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.untyped
  )
  def reduce(initial=_, arg0=_, &blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Opus::Types.untyped), Enumerator)
  )
  def select(&blk); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def to_a(); end
end

class Enumerator
  standard_method(
    {
      arg0: Opus::Types.any(Integer, Proc),
      blk: Opus::Types.proc([Opus::Types.array_of(Opus::Types.untyped)], returns: BasicObject),
    },
    returns: Object
  )
  def initialize(arg0=_, &blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.untyped
  )
  def each(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_with_index(&blk); end

  standard_method(
    {
      arg0: BasicObject,
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_with_object(arg0, &blk); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: NilClass
  )
  def feed(arg0); end

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
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def with_index(&blk); end

  standard_method(
    {
      arg0: BasicObject,
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def with_object(arg0, &blk); end
end

class Exception
  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(arg0); end

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
      arg0: String,
    },
    returns: Exception
  )
  def exception(arg0=_); end

  standard_method(
    {
      arg0: String,
    },
    returns: Object
  )
  def initialize(arg0=_); end

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
      arg0: Opus::Types.any(String, Opus::Types.array_of(String)),
    },
    returns: Opus::Types.array_of(String)
  )
  def set_backtrace(arg0); end

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
      arg0: String,
      arg1: Integer,
      arg2: Integer,
    },
    returns: String
  )
  def self.binread(arg0, arg1=_, arg2=_); end

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
      arg0: Opus::Types.any(BasicObject, Pathname, File, Opus::Types.array_of(String)),
    },
    returns: String
  )
  def self.join(*arg0); end

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
      blk: Opus::Types.proc([File], returns: BasicObject),
    },
    returns: Opus::Types.any(File, Opus::Types.untyped)
  )
  def self.open(file=_, perm=_, opt=_, mode: _, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, &blk); end

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
      arg0: Integer,
    },
    returns: Integer
  )
  def self.truncate(file, arg0); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Integer
  )
  def self.umask(arg0=_); end

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
      arg0: Integer,
    },
    returns: Opus::Types.any(Integer, TrueClass, FalseClass)
  )
  def flock(arg0); end

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
      arg0: Integer,
    },
    returns: Integer
  )
  def truncate(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float, BigDecimal)
  )
  def %(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def *(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, Integer, Rational, BigDecimal, Complex)
  )
  def **(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def +(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def -(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def /(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Object
  )
  def <=>(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Integer
  )
  def div(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def divmod(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: [Float, Float]
  )
  def coerce(arg0); end

  standard_method(
    {},
    returns: Integer
  )
  def denominator(); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def fdiv(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float, BigDecimal)
  )
  def modulo(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def quo(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Rational
  )
  def rationalize(arg0=_); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(arg0=_); end

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
      arg0: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def [](arg0); end

  standard_method(
    {
      arg0: BasicObject,
      arg1: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def []=(arg0, arg1); end

  standard_method(
    {
      arg0: BasicObject,
      arg1: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def store(arg0, arg1); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.array_of(BasicObject)
  )
  def assoc(arg0); end

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
      arg0: BasicObject,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.untyped
  )
  def default(arg0=_, &blk); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def default=(arg0); end

  standard_method(
    {
      arg0: BasicObject,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, BasicObject)
  )
  def delete(arg0, &blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped), Enumerator)
  )
  def delete_if(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped), Enumerator)
  )
  def each(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped), Enumerator)
  )
  def each_pair(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped), Enumerator)
  )
  def each_key(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped), Enumerator)
  )
  def each_value(&blk); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end

  standard_method(
    {
      arg0: BasicObject,
      arg1: BasicObject,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, BasicObject)
  )
  def fetch(arg0, arg1=_, &blk); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def member?(arg0); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def has_key?(arg0); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def key?(arg0); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def has_value?(arg0); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def value?(arg0); end

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
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped), Enumerator)
  )
  def keep_if(&blk); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def key(arg0); end

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
      arg0: Opus::Types.hash_of(keys: BasicObject, values: BasicObject),
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped, Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.hash_of(keys: BasicObject, values: BasicObject)
  )
  def merge(arg0, &blk); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.array_of(BasicObject)
  )
  def rassoc(arg0); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def rehash(); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(Enumerator, Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped))
  )
  def reject(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def reject!(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def select(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def select!(&blk); end

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
      arg0: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def values_at(*arg0); end
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
      arg0: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def <<(arg0); end

  standard_method(
    {
      arg0: Symbol,
      offset: Integer,
      len: Integer,
    },
    returns: NilClass
  )
  def advise(arg0, offset=_, len=_); end

  standard_method(
    {
      arg0: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def autoclose=(arg0); end

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
      arg0: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def close_on_exec=(arg0); end

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
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each(sep=_, limit=_, &blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_byte(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_char(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_codepoint(&blk); end

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
      arg0: Integer,
    },
    returns: Integer
  )
  def lineno=(arg0); end

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
      arg0: Integer,
    },
    returns: Integer
  )
  def pos=(arg0); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: NilClass
  )
  def print(*arg0); end

  standard_method(
    {
      format_string: String,
      arg0: BasicObject,
    },
    returns: NilClass
  )
  def printf(format_string, *arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Numeric, String),
    },
    returns: Opus::Types.untyped
  )
  def putc(arg0); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: NilClass
  )
  def puts(*arg0); end

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
      arg0: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sync=(arg0); end

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
      arg0: String,
    },
    returns: Integer
  )
  def syswrite(arg0); end

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
      arg0: Opus::Types.any(String, Integer),
    },
    returns: NilClass
  )
  def ungetbyte(arg0); end

  standard_method(
    {
      arg0: String,
    },
    returns: NilClass
  )
  def ungetc(arg0); end

  standard_method(
    {
      arg0: String,
    },
    returns: Integer
  )
  def write(arg0); end

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
      arg0: String,
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
  def self.binwrite(name, arg0, offset=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

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
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(NilClass, Enumerator)
  )
  def self.foreach(name, sep=_, limit=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _, &blk); end

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
      blk: Opus::Types.proc([IO], returns: BasicObject),
    },
    returns: Opus::Types.any(IO, Opus::Types.untyped)
  )
  def self.open(fd, mode=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _, &blk); end

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
      blk: Opus::Types.proc([[IO, IO]], returns: BasicObject),
    },
    returns: Opus::Types.any([IO, IO], Opus::Types.untyped)
  )
  def self.pipe(ext_or_ext_int_enc=_, int_enc=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _, &blk); end

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
      arg0: BasicObject,
    },
    returns: Opus::Types.any(IO, NilClass)
  )
  def self.try_convert(arg0); end

  standard_method(
    {
      name: String,
      arg0: String,
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
  def self.write(name, arg0, offset=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

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
    {
      blk: Opus::Types.proc([Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def bytes(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def chars(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def codepoints(&blk); end

  standard_method(
    {
      sep: String,
      limit: Integer,
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_line(sep=_, limit=_, &blk); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eof?(); end

  standard_method(
    {
      sep: String,
      limit: Integer,
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def lines(sep=_, limit=_, &blk); end

  standard_method(
    {},
    returns: Integer
  )
  def to_i(); end
end

class Integer
  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def %(arg0); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Integer
  )
  def &(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def *(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def **(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def +(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def -(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def /(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <(arg0); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Integer
  )
  def <<(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Object
  )
  def <=>(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=(arg0); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Integer
  )
  def >>(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Rational, Float, BigDecimal),
    },
    returns: Integer
  )
  def [](arg0); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Integer
  )
  def ^(arg0); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Integer
  )
  def |(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Integer
  )
  def div(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def divmod(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def fdiv(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def modulo(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def quo(arg0); end

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
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(arg0); end

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
      arg0: Encoding,
    },
    returns: String
  )
  def chr(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def coerce(arg0); end

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
      arg0: Integer,
      blk: Opus::Types.proc([Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(Integer, Enumerator)
  )
  def downto(arg0, &blk); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def even?(); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Integer
  )
  def gcd(arg0); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: [Integer, Integer]
  )
  def gcdlcm(arg0); end

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
      arg0: Integer,
    },
    returns: Integer
  )
  def lcm(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Rational
  )
  def rationalize(arg0=_); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def remainder(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(arg0=_); end

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
    {
      blk: Opus::Types.proc([Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(Integer, Enumerator)
  )
  def times(&blk); end

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
      arg0: Integer,
      blk: Opus::Types.proc([Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(Integer, Enumerator)
  )
  def upto(arg0, &blk); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end
end

module Kernel
  standard_method(
    {
      x: Object,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def self.Array(x); end

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
      x: Object,
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
      x: Opus::Types.any(Numeric, Object),
      y: Numeric,
    },
    returns: Rational
  )
  def self.Rational(x, y=_); end

  standard_method(
    {
      x: Object,
    },
    returns: String
  )
  def self.String(x); end

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
      arg0: String,
    },
    returns: String
  )
  def self.`(arg0); end

  standard_method(
    {
      msg: String,
    },
    returns: NilClass
  )
  def self.abort(msg=_); end

  standard_method(
    {
      blk: Opus::Types.proc([], returns: BasicObject),
    },
    returns: Proc
  )
  def self.at_exit(&blk); end

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
      arg0: String,
      arg1: Binding,
      filename: String,
      lineno: Integer,
    },
    returns: Opus::Types.untyped
  )
  def self.eval(arg0, arg1=_, filename=_, lineno=_); end

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
      arg0: Opus::Types.any(String, Class),
      arg1: Opus::Types.any(Opus::Types.array_of(String), String),
      arg2: Opus::Types.array_of(String),
    },
    returns: NilClass
  )
  def self.fail(arg0=_, arg1=_, arg2=_); end

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
      arg0: String,
      arg1: Integer,
    },
    returns: String
  )
  def self.gets(arg0=_, arg1=_); end

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
      arg0: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.load(filename, arg0=_); end

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
      arg0: IO,
      arg1: String,
      arg2: BasicObject,
    },
    returns: NilClass
  )
  def self.printf(arg0=_, arg1=_, *arg2); end

  standard_method(
    {
      blk: BasicObject,
    },
    returns: Proc
  )
  def self.proc(&blk); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Integer
  )
  def self.putc(arg0); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: NilClass
  )
  def self.puts(*arg0); end

  standard_method(
    {
      max: Opus::Types.any(Integer, Range),
    },
    returns: Numeric
  )
  def self.rand(max); end

  standard_method(
    {
      arg0: String,
      arg1: Integer,
    },
    returns: String
  )
  def self.readline(arg0=_, arg1=_); end

  standard_method(
    {
      arg0: String,
      arg1: Integer,
    },
    returns: Opus::Types.array_of(String)
  )
  def self.readlines(arg0=_, arg1=_); end

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
    {
      arg0: Opus::Types.any(String, Class, Exception),
      arg1: String,
      arg2: Opus::Types.array_of(String),
    },
    returns: NilClass
  )
  def self.raise(arg0=_, arg1=_, arg2=_); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

  standard_method(
    {},
    returns: Opus::Types.untyped
  )
  def clone(); end

  standard_method(
    {
      arg0: Opus::Types.any(String, Symbol),
      arg1: BasicObject,
      blk: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def send(arg0, *arg1, &blk); end

  standard_method(
    {
      x: Object,
    },
    returns: Opus::Types.array_of(Opus::Types.untyped)
  )
  def Array(x); end

  standard_method(
    {
      x: Opus::Types.any(Numeric, String),
      y: Numeric,
    },
    returns: Complex
  )
  def Complex(x, y=_); end

  standard_method(
    {
      x: Numeric,
    },
    returns: Float
  )
  def Float(x); end

  standard_method(
    {
      x: Object,
    },
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Opus::Types.untyped)
  )
  def Hash(x); end

  standard_method(
    {
      arg: Opus::Types.any(Numeric, String),
      base: Integer,
    },
    returns: Integer
  )
  def Integer(arg, base=_); end

  standard_method(
    {
      x: Opus::Types.any(Numeric, Object),
      y: Numeric,
    },
    returns: Rational
  )
  def Rational(x, y=_); end

  standard_method(
    {
      x: Object,
    },
    returns: String
  )
  def String(x); end

  standard_method(
    {},
    returns: Opus::Types.any(Symbol, NilClass)
  )
  def __callee__(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def __dir__(); end

  standard_method(
    {},
    returns: Opus::Types.any(Symbol, NilClass)
  )
  def __method__(); end

  standard_method(
    {
      arg0: String,
    },
    returns: String
  )
  def `(arg0); end

  standard_method(
    {
      msg: String,
    },
    returns: NilClass
  )
  def abort(msg=_); end

  standard_method(
    {
      blk: Opus::Types.proc([], returns: BasicObject),
    },
    returns: Proc
  )
  def at_exit(&blk); end

  standard_method(
    {
      _module: Opus::Types.any(String, Symbol),
      filename: String,
    },
    returns: NilClass
  )
  def autoload(_module, filename); end

  standard_method(
    {
      name: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def autoload?(name); end

  standard_method(
    {},
    returns: Binding
  )
  def binding(); end

  standard_method(
    {
      status: Opus::Types.any(Integer, TrueClass, FalseClass),
    },
    returns: NilClass
  )
  def exit(status=_); end

  standard_method(
    {
      status: Opus::Types.any(Integer, TrueClass, FalseClass),
    },
    returns: NilClass
  )
  def exit!(status); end

  standard_method(
    {
      arg0: Opus::Types.any(String, Class),
      arg1: Opus::Types.any(Opus::Types.array_of(String), String),
      arg2: Opus::Types.array_of(String),
    },
    returns: NilClass
  )
  def fail(arg0=_, arg1=_, arg2=_); end

  standard_method(
    {
      format: String,
      args: BasicObject,
    },
    returns: String
  )
  def format(format, *args); end

  standard_method(
    {
      arg0: String,
      arg1: Integer,
    },
    returns: String
  )
  def gets(arg0=_, arg1=_); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Symbol)
  )
  def global_variables(); end

  standard_method(
    {
      filename: String,
      arg0: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def load(filename, arg0=_); end

  standard_method(
    {
      name: String,
      rest: Opus::Types.any(String, Integer),
      block: String,
    },
    returns: Opus::Types.any(IO, NilClass)
  )
  def open(name, rest=_, block=_); end

  standard_method(
    {
      arg0: IO,
      arg1: String,
      arg2: BasicObject,
    },
    returns: NilClass
  )
  def printf(arg0=_, arg1=_, *arg2); end

  standard_method(
    {
      blk: BasicObject,
    },
    returns: Proc
  )
  def proc(&blk); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Integer
  )
  def putc(arg0); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: NilClass
  )
  def puts(*arg0); end

  standard_method(
    {
      arg0: String,
      arg1: Integer,
    },
    returns: String
  )
  def readline(arg0=_, arg1=_); end

  standard_method(
    {
      arg0: String,
      arg1: Integer,
    },
    returns: Opus::Types.array_of(String)
  )
  def readlines(arg0=_, arg1=_); end

  standard_method(
    {
      path: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def require(path); end

  standard_method(
    {
      feature: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def require_relative(feature); end

  standard_method(
    {
      read: Opus::Types.array_of(IO),
      write: Opus::Types.array_of(IO),
      error: Opus::Types.array_of(IO),
      timeout: Integer,
    },
    returns: Opus::Types.array_of(String)
  )
  def select(read, write=_, error=_, timeout=_); end

  standard_method(
    {
      duration: Numeric,
    },
    returns: Integer
  )
  def sleep(duration); end

  standard_method(
    {
      format: String,
      args: BasicObject,
    },
    returns: String
  )
  def self.sprintf(format, *args); end

  standard_method(
    {
      format: String,
      args: BasicObject,
    },
    returns: String
  )
  def sprintf(format, *args); end

  standard_method(
    {
      num: Integer,
      args: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def syscall(num, *args); end

  standard_method(
    {
      cmd: String,
      file1: String,
      file2: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass, Time)
  )
  def test(cmd, file1, file2=_); end

  standard_method(
    {
      msg: String,
    },
    returns: NilClass
  )
  def warn(*msg); end

  standard_method(
    {
      arg0: Opus::Types.any(String, Class, Exception),
      arg1: String,
      arg2: Opus::Types.array_of(String),
    },
    returns: NilClass
  )
  def raise(arg0=_, arg1=_, arg2=_); end
end

module Marshal
  standard_method(
    {
      arg0: String,
      arg1: Proc,
    },
    returns: Object
  )
  def self.load(arg0, arg1=_); end

  standard_method(
    {
      arg0: Object,
      arg1: Opus::Types.any(IO, Integer),
      arg2: Integer,
    },
    returns: Object
  )
  def self.dump(arg0, arg1=_, arg2=_); end
end

class MatchData
  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(arg0); end

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
    {
      blk: Opus::Types.proc([Module], returns: BasicObject),
    },
    returns: Object
  )
  def initialize(&blk); end

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
      arg0: String,
      filename: String,
      lineno: Integer,
    },
    returns: Opus::Types.untyped
  )
  def class_eval(arg0, filename=_, lineno=_); end

  standard_method(
    {
      args: BasicObject,
      blk: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def class_exec(*args, &blk); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def class_variable_defined?(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def class_variable_get(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
      arg1: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def class_variable_set(arg0, arg1); end

  standard_method(
    {
      inherit: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def class_variables(inherit=_); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
      inherit: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def const_defined?(arg0, inherit=_); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
      inherit: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.untyped
  )
  def const_get(arg0, inherit=_); end

  standard_method(
    {
      arg0: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def const_missing(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
      arg1: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def const_set(arg0, arg1); end

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
      arg0: Module,
    },
    returns: Opus::Types.untyped
  )
  def include(*arg0); end

  standard_method(
    {
      arg0: Module,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(arg0); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Module)
  )
  def included_modules(); end

  standard_method(
    {
      arg0: Symbol,
    },
    returns: UnboundMethod
  )
  def instance_method(arg0); end

  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def instance_methods(include_super=_); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def method_defined?(arg0); end

  standard_method(
    {
      arg0: String,
      filename: String,
      lineno: Integer,
    },
    returns: Opus::Types.untyped
  )
  def module_eval(arg0, filename=_, lineno=_); end

  standard_method(
    {
      args: BasicObject,
      blk: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def module_exec(*args, &blk); end

  standard_method(
    {},
    returns: String
  )
  def name(); end

  standard_method(
    {
      arg0: Module,
    },
    returns: Opus::Types.untyped
  )
  def prepend(*arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def private_class_method(*arg0); end

  standard_method(
    {
      arg0: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def private_constant(*arg0); end

  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def private_instance_methods(include_super=_); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def private_method_defined?(arg0); end

  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def protected_instance_methods(include_super=_); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def protected_method_defined?(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def public_class_method(*arg0); end

  standard_method(
    {
      arg0: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def public_constant(*arg0); end

  standard_method(
    {
      arg0: Symbol,
    },
    returns: UnboundMethod
  )
  def public_instance_method(arg0); end

  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def public_instance_methods(include_super=_); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def public_method_defined?(arg0); end

  standard_method(
    {
      arg0: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def remove_class_variable(arg0); end

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
      arg0: Module,
    },
    returns: Opus::Types.untyped
  )
  def append_features(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: NilClass
  )
  def attr_accessor(*arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: NilClass
  )
  def attr_reader(*arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: NilClass
  )
  def attr_writer(*arg0); end

  standard_method(
    {
      arg0: Symbol,
      arg1: Method,
      blk: BasicObject,
    },
    returns: Symbol
  )
  def define_method(arg0, arg1=_, &blk); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def extend_object(arg0); end

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
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def module_function(*arg0); end

  standard_method(
    {
      arg0: Module,
    },
    returns: Opus::Types.untyped
  )
  def prepend_features(arg0); end

  standard_method(
    {
      othermod: Module,
    },
    returns: Opus::Types.untyped
  )
  def prepended(othermod); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def private(*arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def protected(*arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def public(*arg0); end

  standard_method(
    {
      arg0: Class,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.untyped
  )
  def refine(arg0, &blk); end

  standard_method(
    {
      arg0: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def remove_const(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def remove_method(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def undef_method(arg0); end

  standard_method(
    {
      arg0: Module,
    },
    returns: Opus::Types.untyped
  )
  def using(arg0); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: NilClass
  )
  def attr(*arg0); end
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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def %(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Object
  )
  def <=>(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)]
  )
  def coerce(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Integer
  )
  def div(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)]
  )
  def divmod(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def fdiv(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def modulo(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def quo(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def remainder(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(arg0); end

  standard_method(
    {
      arg0: Symbol,
    },
    returns: TypeError
  )
  def singleton_method_added(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
      arg1: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
      blk: Opus::Types.proc([Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)], returns: BasicObject),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex, Enumerator)
  )
  def step(arg0, arg1=_, &blk); end

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
      blk: BasicObject,
    },
    returns: Enumerator
  )
  def enum_for(method=_, *args, &blk); end

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
      arg0: Class,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def instance_of?(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def instance_variable_defined?(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.untyped
  )
  def instance_variable_get(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
      arg1: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def instance_variable_set(arg0, arg1); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Symbol)
  )
  def instance_variables(); end

  standard_method(
    {
      arg0: Opus::Types.any(Class, Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def is_a?(arg0); end

  standard_method(
    {
      arg0: Class,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def kind_of?(arg0); end

  standard_method(
    {
      arg0: Symbol,
    },
    returns: Method
  )
  def method(arg0); end

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
      arg0: Symbol,
    },
    returns: Method
  )
  def public_method(arg0); end

  standard_method(
    {
      all: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def public_methods(all=_); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
      args: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def public_send(arg0, *args); end

  standard_method(
    {
      arg0: Symbol,
    },
    returns: Opus::Types.untyped
  )
  def remove_instance_variable(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Symbol, String),
      args: BasicObject,
    },
    returns: Opus::Types.untyped
  )
  def send(arg0, *args); end

  standard_method(
    {},
    returns: Class
  )
  def singleton_class(); end

  standard_method(
    {
      arg0: Symbol,
    },
    returns: Method
  )
  def singleton_method(arg0); end

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
      blk: BasicObject,
    },
    returns: Enumerator
  )
  def to_enum(method=_, *args, &blk); end

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
    {
      blk: Opus::Types.proc([Pathname], returns: BasicObject),
    },
    returns: Opus::Types.untyped
  )
  def ascend(&blk); end

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
      arg0: String,
      offset: Integer,
    },
    returns: Integer
  )
  def binwrite(arg0, offset=_); end

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
    {
      blk: Opus::Types.proc([Pathname], returns: BasicObject),
    },
    returns: Opus::Types.untyped
  )
  def descend(&blk); end

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
      blk: Opus::Types.proc([Pathname], returns: BasicObject),
    },
    returns: Opus::Types.untyped
  )
  def each_child(with_directory, &blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Pathname], returns: BasicObject),
    },
    returns: Opus::Types.untyped
  )
  def each_entry(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_filename(&blk); end

  standard_method(
    {
      sep: String,
      limit: Integer,
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each_line(sep=_, limit=_, &blk); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Pathname)
  )
  def entries(); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(arg0); end

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
      blk: Opus::Types.proc([Pathname], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def find(ignore_error, &blk); end

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
      blk: Opus::Types.proc([File], returns: BasicObject),
    },
    returns: Opus::Types.any(File, Opus::Types.untyped)
  )
  def open(mode=_, perm=_, opt=_, &blk); end

  standard_method(
    {
      arg0: Encoding,
      blk: Opus::Types.proc([Dir], returns: BasicObject),
    },
    returns: Opus::Types.any(Dir, Opus::Types.untyped)
  )
  def opendir(arg0=_, &blk); end

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
      arg0: String,
      offset: Integer,
      open_args: Integer,
    },
    returns: Integer
  )
  def write(arg0, offset=_, open_args=_); end

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
      arg0: Integer,
    },
    returns: Integer
  )
  def self.egid=(arg0); end

  standard_method(
    {},
    returns: Integer
  )
  def self.euid(); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Integer
  )
  def self.euid=(arg0); end

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
    {
      blk: Opus::Types.proc([], returns: BasicObject),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.fork(&blk); end

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
      arg0: Integer,
    },
    returns: Integer
  )
  def self.getpriority(kind, arg0); end

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
      arg0: Integer,
    },
    returns: Integer
  )
  def self.gid=(arg0); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Integer)
  )
  def self.groups(); end

  standard_method(
    {
      arg0: Opus::Types.array_of(Integer),
    },
    returns: Opus::Types.array_of(Integer)
  )
  def self.groups=(arg0); end

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
      arg0: Integer,
    },
    returns: Integer
  )
  def self.maxgroups=(arg0); end

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
      arg0: Integer,
      priority: Integer,
    },
    returns: Integer
  )
  def self.setpriority(kind, arg0, priority); end

  standard_method(
    {
      arg0: String,
    },
    returns: String
  )
  def self.setproctitle(arg0); end

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
      arg0: Integer,
    },
    returns: Integer
  )
  def self.setpgid(pid, arg0); end

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
    {
      blk: Opus::Types.proc([], returns: BasicObject),
    },
    returns: Opus::Types.any(Integer, Opus::Types.untyped)
  )
  def self.switch(&blk); end

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
    {
      blk: Opus::Types.proc([], returns: BasicObject),
    },
    returns: Opus::Types.any(Integer, Opus::Types.untyped)
  )
  def self.switch(&blk); end

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
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(arg0); end

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
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(BasicObject, NilClass)
  )
  def bsearch(&blk); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def cover?(obj); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each(&blk); end

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
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Integer),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def max(n=_, &blk); end

  standard_method(
    {
      n: Integer,
      blk: Opus::Types.proc([Opus::Types.untyped, Opus::Types.untyped], returns: Integer),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Opus::Types.array_of(Opus::Types.untyped))
  )
  def min(n=_, &blk); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def size(); end

  standard_method(
    {
      n: Integer,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def step(n=_, &blk); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal)
  )
  def %(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def *(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def +(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def -(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def **(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def /(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Object
  )
  def <=>(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(arg0); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Integer
  )
  def div(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal)
  )
  def modulo(arg0); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def ceil(arg0=_); end

  standard_method(
    {},
    returns: Integer
  )
  def denominator(); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: [Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]
  )
  def divmod(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Float
  )
  def fdiv(arg0); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def floor(arg0=_); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def quo(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Rational
  )
  def rationalize(arg0=_); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(arg0=_); end

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
      arg0: Integer,
    },
    returns: Opus::Types.any(Integer, Rational)
  )
  def truncate(arg0=_); end

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
      arg0: Opus::Types.any(Integer, Float, Rational, Complex),
    },
    returns: Opus::Types.any([Rational, Rational], [Float, Float], [Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)])
  )
  def coerce(arg0); end
end

class Regexp
  standard_method(
    {
      arg0: Opus::Types.any(String, Symbol),
    },
    returns: String
  )
  def self.escape(arg0); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.any(MatchData, String)
  )
  def self.last_match(arg0=_); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(Regexp, NilClass)
  )
  def self.try_convert(obj); end

  standard_method(
    {
      arg0: Opus::Types.any(String, Regexp),
      options: BasicObject,
      kcode: String,
    },
    returns: Object
  )
  def initialize(arg0, options=_, kcode=_); end

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
      arg0: String,
      arg1: Integer,
    },
    returns: Opus::Types.any(MatchData, NilClass)
  )
  def match(arg0, arg1=_); end

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
      arg0: Opus::Types.any(String, Regexp),
      options: BasicObject,
      kcode: String,
    },
    returns: Opus::Types.untyped
  )
  def self.compile(arg0, options=_, kcode=_); end

  standard_method(
    {
      arg0: Opus::Types.any(String, Symbol),
    },
    returns: String
  )
  def self.quote(arg0); end

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
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.hash_of(keys: Opus::Types.untyped, values: Set)
  )
  def classify(&blk); end

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
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.untyped
  )
  def delete_if(&blk); end

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
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Opus::Types.any(Opus::Types.untyped, Enumerator)
  )
  def each(&blk); end

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
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.untyped
  )
  def keep_if(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Set
  )
  def map!(&blk); end

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
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(BasicObject, NilClass)
  )
  def reject!(&blk); end

  standard_method(
    {
      enum: Enumerable,
    },
    returns: Set
  )
  def replace(enum); end

  standard_method(
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: Opus::Types.any(TrueClass, FalseClass)),
    },
    returns: Opus::Types.any(BasicObject, NilClass)
  )
  def select!(&blk); end

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
    {
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: Set
  )
  def collect!(&blk); end

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
      arg0: Object,
    },
    returns: String
  )
  def %(arg0); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: String
  )
  def *(arg0); end

  standard_method(
    {
      arg0: String,
    },
    returns: String
  )
  def +(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: String
  )
  def <<(arg0); end

  standard_method(
    {
      other: String,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(other); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(arg0); end

  standard_method(
    {
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(arg0); end

  standard_method(
    {
      arg0: Object,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def =~(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Range, Regexp, String),
      arg1: Opus::Types.any(Integer, String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def [](arg0, arg1=_); end

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
      arg0: Opus::Types.any(Integer, Range),
      arg1: Integer,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def byteslice(arg0, arg1=_); end

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
      arg0: String,
    },
    returns: Opus::Types.any(NilClass, Integer)
  )
  def casecmp(arg0); end

  standard_method(
    {
      arg0: Integer,
      arg1: String,
    },
    returns: String
  )
  def center(arg0, arg1=_); end

  standard_method(
    {},
    returns: Array
  )
  def chars(); end

  standard_method(
    {
      arg0: String,
    },
    returns: String
  )
  def chomp(arg0=_); end

  standard_method(
    {
      arg0: String,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def chomp!(arg0=_); end

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
    {
      blk: BasicObject,
    },
    returns: Opus::Types.array_of(Integer)
  )
  def codepoints(&blk); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Object),
    },
    returns: String
  )
  def concat(arg0); end

  standard_method(
    {
      arg0: String,
      arg1: String,
    },
    returns: Integer
  )
  def count(arg0, *arg1); end

  standard_method(
    {
      arg0: String,
    },
    returns: String
  )
  def crypt(arg0); end

  standard_method(
    {
      arg0: String,
      arg1: String,
    },
    returns: String
  )
  def delete(arg0, *arg1); end

  standard_method(
    {
      arg0: String,
      arg1: String,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def delete!(arg0, *arg1); end

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
    {
      blk: Opus::Types.proc([Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_byte(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_char(&blk); end

  standard_method(
    {
      blk: Opus::Types.proc([Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_codepoint(&blk); end

  standard_method(
    {
      arg0: String,
      blk: Opus::Types.proc([Integer], returns: BasicObject),
    },
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_line(arg0=_, &blk); end

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
      arg0: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def end_with?(*arg0); end

  standard_method(
    {
      arg0: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(String, Encoding),
    },
    returns: String
  )
  def force_encoding(arg0); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def getbyte(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Regexp, String),
      arg1: Opus::Types.any(String, Hash),
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(String, Enumerator)
  )
  def gsub(arg0, arg1=_, &blk); end

  standard_method(
    {
      arg0: Opus::Types.any(Regexp, String),
      arg1: String,
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(String, NilClass, Enumerator)
  )
  def gsub!(arg0, arg1=_, &blk); end

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
      arg0: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Regexp, String),
      arg1: Integer,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def index(arg0, arg1=_); end

  standard_method(
    {
      arg0: String,
    },
    returns: String
  )
  def replace(arg0); end

  standard_method(
    {
      arg0: Integer,
      arg1: String,
    },
    returns: String
  )
  def insert(arg0, arg1); end

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
      arg0: String,
    },
    returns: Opus::Types.array_of(String)
  )
  def lines(arg0=_); end

  standard_method(
    {
      arg0: Integer,
      arg1: String,
    },
    returns: String
  )
  def ljust(arg0, arg1=_); end

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
      arg0: Opus::Types.any(Regexp, String),
      arg1: Integer,
    },
    returns: MatchData
  )
  def match(arg0, arg1=_); end

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
      arg0: Opus::Types.any(Regexp, String),
    },
    returns: Opus::Types.array_of(String)
  )
  def partition(arg0); end

  standard_method(
    {
      arg0: String,
    },
    returns: String
  )
  def prepend(arg0); end

  standard_method(
    {},
    returns: String
  )
  def reverse(); end

  standard_method(
    {
      arg0: Opus::Types.any(String, Regexp),
      arg1: Integer,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def rindex(arg0, arg1=_); end

  standard_method(
    {
      arg0: Integer,
      arg1: String,
    },
    returns: String
  )
  def rjust(arg0, arg1=_); end

  standard_method(
    {
      arg0: Opus::Types.any(String, Regexp),
    },
    returns: Opus::Types.array_of(String)
  )
  def rpartition(arg0); end

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
      arg0: Opus::Types.any(Regexp, String),
      blk: BasicObject,
    },
    returns: Opus::Types.array_of(Opus::Types.any(String, Opus::Types.array_of(String)))
  )
  def scan(arg0, &blk); end

  standard_method(
    {
      arg0: String,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: String
  )
  def scrub(arg0=_, &blk); end

  standard_method(
    {
      arg0: String,
      blk: Opus::Types.proc([Opus::Types.untyped], returns: BasicObject),
    },
    returns: String
  )
  def scrub!(arg0=_, &blk); end

  standard_method(
    {
      arg0: Integer,
      arg1: Integer,
    },
    returns: Integer
  )
  def setbyte(arg0, arg1); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end

  standard_method(
    {
      arg0: Opus::Types.any(Integer, Range, Regexp, String),
      arg1: Opus::Types.any(Integer, String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def slice!(arg0, arg1=_); end

  standard_method(
    {
      arg0: Opus::Types.any(Regexp, String, Integer),
      arg1: Integer,
    },
    returns: Opus::Types.array_of(String)
  )
  def split(arg0=_, arg1=_); end

  standard_method(
    {
      arg0: String,
    },
    returns: String
  )
  def squeeze(arg0=_); end

  standard_method(
    {
      arg0: String,
    },
    returns: String
  )
  def squeeze!(arg0=_); end

  standard_method(
    {
      arg0: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def start_with?(*arg0); end

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
      arg0: Opus::Types.any(Regexp, String),
      arg1: Opus::Types.any(String, Hash),
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: String
  )
  def sub(arg0, arg1=_, &blk); end

  standard_method(
    {
      arg0: Opus::Types.any(Regexp, String),
      arg1: String,
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: String
  )
  def sub!(arg0, arg1=_, &blk); end

  standard_method(
    {},
    returns: String
  )
  def succ(); end

  standard_method(
    {
      arg0: Integer,
    },
    returns: Integer
  )
  def sum(arg0=_); end

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
      arg0: Integer,
    },
    returns: Integer
  )
  def to_i(arg0=_); end

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
      arg0: String,
      arg1: String,
    },
    returns: String
  )
  def tr(arg0, arg1); end

  standard_method(
    {
      arg0: String,
      arg1: String,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def tr!(arg0, arg1); end

  standard_method(
    {
      arg0: String,
      arg1: String,
    },
    returns: String
  )
  def tr_s(arg0, arg1); end

  standard_method(
    {
      arg0: String,
      arg1: String,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def tr_s!(arg0, arg1); end

  standard_method(
    {
      arg0: String,
    },
    returns: Opus::Types.array_of(String)
  )
  def unpack(arg0); end

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
      arg0: String,
      arg1: BasicObject,
      blk: Opus::Types.proc([String], returns: BasicObject),
    },
    returns: Opus::Types.any(Enumerator, String)
  )
  def upto(arg0, arg1=_, &blk); end

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
      arg0: Opus::Types.any(Integer, Range, Regexp, String),
      arg1: Opus::Types.any(Integer, String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def slice(arg0, arg1=_); end
end

class StringScanner
  standard_method(
    {
      arg0: String,
      arg1: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: StringScanner
  )
  def self.new(arg0, arg1=_); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eos?(); end

  standard_method(
    {
      arg0: Regexp,
    },
    returns: String
  )
  def scan(arg0); end

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
      arg0: Opus::Types.any(Time, Numeric),
      microseconds_with_frac: Numeric,
    },
    returns: Time
  )
  def self.at(arg0, microseconds_with_frac=_); end

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
      arg0: Numeric,
    },
    returns: Time
  )
  def +(arg0); end

  standard_method(
    {
      arg0: Opus::Types.any(Time, Numeric),
    },
    returns: Opus::Types.any(Float, Time)
  )
  def -(arg0); end

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
      arg0: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(arg0); end

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
      arg0: Integer,
    },
    returns: Time
  )
  def round(arg0); end

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
      arg0: String,
    },
    returns: String
  )
  def strftime(arg0); end

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
      blk: BasicObject,
    },
    returns: Opus::Types.array_of(String)
  )
  def self.extract(str, schemes=_, &blk); end

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
      arg0: Opus::Types.any(Regexp, String),
    },
    returns: String
  )
  def self.escape(arg, *arg0); end

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
      arg0: Opus::Types.any(Regexp, String),
    },
    returns: String
  )
  def self.encode(arg, *arg0); end

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
