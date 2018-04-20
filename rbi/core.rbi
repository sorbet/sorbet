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
class Delegator < BasicObject
end
module DidYouMean
end
class DidYouMean::ClassNameChecker < Object
end
class SimpleDelegator < Delegator
end
class DidYouMean::ClassNameChecker::ClassName < SimpleDelegator
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
class CSV
  extend T::Generic
  Elem = type_member(fixed: T::Array[String])

  type_parameters(:U).sig(
      path: T.any(String, File),
      options: T::Hash[Symbol, T.type_parameter(:U)],
      blk: T.proc(arg0: T::Array[String]).returns(BasicObject),
  )
  .returns(NilClass)
  def self.foreach(path, options=_, &blk); end

  sig.returns(CSV)
  def clone(); end

  sig.returns(CSV)
  def dup(); end

  sig.returns(CSV)
  def freeze(); end

  sig.returns(CSV)
  def taint(); end

  sig.returns(CSV)
  def trust(); end

  sig.returns(CSV)
  def untaint(); end

  sig.returns(CSV)
  def untrust(); end
end

class Dir
  extend T::Generic
  Elem = type_member(fixed: String)

  sig(
      arg0: T.any(String, Pathname),
  )
  .returns(Integer)
  type_parameters(:U).sig(
      arg0: T.any(String, Pathname),
      blk: T.proc(arg0: String).returns(T.type_parameter(:U)),
  )
  .returns(T.type_parameter(:U))
  def self.chdir(arg0=_, &blk); end

  sig(
      arg0: String,
  )
  .returns(Integer)
  def self.chroot(arg0); end

  sig(
      arg0: String,
  )
  .returns(Integer)
  def self.delete(arg0); end

  sig(
      arg0: String,
      arg1: Encoding,
  )
  .returns(T::Array[String])
  def self.entries(arg0, arg1=_); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.exist?(file); end

  sig(
      dir: String,
      arg0: Encoding,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(NilClass)
  sig(
      dir: String,
      arg0: Encoding,
  )
  .returns(Enumerator[String])
  def self.foreach(dir, arg0=_, &blk); end

  sig.returns(String)
  def self.getwd(); end

  sig(
      pattern: T.any(String, T::Array[String]),
      flags: Integer,
  )
  .returns(T::Array[String])
  sig(
      pattern: T.any(String, T::Array[String]),
      flags: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(NilClass)
  def self.glob(pattern, flags=_, &blk); end

  sig(
      arg0: String,
  )
  .returns(String)
  def self.home(arg0=_); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(Integer)
  def self.mkdir(arg0, arg1=_); end

  sig(
      arg0: String,
      arg1: Encoding,
  )
  .returns(Dir)
  type_parameters(:U).sig(
      arg0: String,
      arg1: Encoding,
      blk: T.proc(arg0: Dir).returns(T.type_parameter(:U)),
  )
  .returns(T.type_parameter(:U))
  def self.open(arg0, arg1=_, &blk); end

  sig.returns(String)
  def self.pwd(); end

  sig(
      arg0: String,
  )
  .returns(Integer)
  def self.rmdir(arg0); end

  sig(
      arg0: String,
  )
  .returns(Integer)
  def self.unlink(arg0); end

  sig.returns(Dir)
  def clone(); end

  sig.returns(NilClass)
  def close(); end

  sig.returns(Dir)
  def dup(); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(Dir)
  sig.returns(Enumerator[String])
  def each(&blk); end

  sig.returns(Integer)
  def fileno(); end

  sig.returns(Dir)
  def freeze(); end

  sig(
      arg0: String,
      arg1: Encoding,
  )
  .returns(Object)
  def initialize(arg0, arg1=_); end

  sig.returns(String)
  def inspect(); end

  sig.returns(T.nilable(String))
  def path(); end

  sig.returns(Integer)
  def pos(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def pos=(arg0); end

  sig.returns(T.nilable(String))
  def read(); end

  sig.returns(Dir)
  def rewind(); end

  sig(
      arg0: Integer,
  )
  .returns(Dir)
  def seek(arg0); end

  sig.returns(Dir)
  def taint(); end

  sig.returns(Integer)
  def tell(); end

  sig.returns(T.nilable(String))
  def to_path(); end

  sig.returns(Dir)
  def trust(); end

  sig.returns(Dir)
  def untaint(); end

  sig.returns(Dir)
  def untrust(); end

  sig(
      pattern: T.any(String, T::Array[String]),
      flags: Integer,
  )
  .returns(T::Array[String])
  sig(
      pattern: T.any(String, T::Array[String]),
      flags: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(NilClass)
  def self.[](pattern, flags=_, &blk); end
end

class File
  extend T::Generic
  Elem = type_member(fixed: String)

  sig(
      file: String,
      dir: String,
  )
  .returns(String)
  def self.absolute_path(file, dir=_); end

  sig(
      file: BasicObject,
  )
  .returns(Time)
  def self.atime(file); end

  sig(
      file: String,
      suffix: String,
  )
  .returns(String)
  def self.basename(file, suffix=_); end

  sig(
      arg0: String,
  )
  .returns(String)
  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(String)
  sig(
      arg0: String,
      arg1: Integer,
      arg2: Integer,
  )
  .returns(String)
  def self.binread(arg0, arg1=_, arg2=_); end

  sig(
      file: BasicObject,
  )
  .returns(Time)
  def self.birthtime(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.blockdev?(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.chardev?(file); end

  sig(
      mode: Integer,
      files: String,
  )
  .returns(Integer)
  def self.chmod(mode, *files); end

  sig(
      owner: Integer,
      group: Integer,
      files: String,
  )
  .returns(Integer)
  def self.chown(owner, group, *files); end

  sig(
      file: BasicObject,
  )
  .returns(Time)
  def self.ctime(file); end

  sig(
      files: String,
  )
  .returns(Integer)
  def self.delete(*files); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.directory?(file); end

  sig(
      file: String,
  )
  .returns(String)
  def self.dirname(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.executable?(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.executable_real?(file); end

  sig(
      file: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.exist?(file); end

  sig(
      file: BasicObject,
      dir: BasicObject,
  )
  .returns(String)
  def self.expand_path(file, dir=_); end

  sig(
      path: String,
  )
  .returns(String)
  def self.extname(path); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.file?(file); end

  sig(
      pattern: String,
      path: String,
      flags: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.fnmatch(pattern, path, flags=_); end

  sig(
      file: String,
  )
  .returns(String)
  def self.ftype(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.grpowned?(file); end

  sig(
      file_1: T.any(String, IO),
      file_2: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.identical?(file_1, file_2); end

  sig(
      arg0: BasicObject,
  )
  .returns(String)
  def self.join(*arg0); end

  sig(
      mode: Integer,
      files: String,
  )
  .returns(Integer)
  def self.lchmod(mode, *files); end

  sig(
      owner: Integer,
      group: Integer,
      files: String,
  )
  .returns(Integer)
  def self.lchown(owner, group, *files); end

  sig(
      old: String,
      new: String,
  )
  .returns(Integer)
  def self.link(old, new); end

  sig(
      file: String,
  )
  .returns(File::Stat)
  def self.lstat(file); end

  sig(
      file: BasicObject,
  )
  .returns(Time)
  def self.mtime(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.owned?(file); end

  sig(
      path: String,
  )
  .returns(String)
  def self.path(path); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.pipe?(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.readable?(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.readable_real?(file); end

  sig(
      link: String,
  )
  .returns(String)
  def self.readlink(link); end

  sig(
      pathname: String,
      dir: String,
  )
  .returns(String)
  def self.realdirpath(pathname, dir=_); end

  sig(
      pathname: String,
      dir: String,
  )
  .returns(String)
  def self.realpath(pathname, dir=_); end

  sig(
      old: String,
      new: String,
  )
  .returns(Integer)
  def self.rename(old, new); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.setgid?(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.setuid?(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(Integer)
  def self.size(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.nilable(Integer))
  def self.size?(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.socket?(file); end

  sig(
      file: String,
  )
  .returns([String, String])
  def self.split(file); end

  sig(
      file: BasicObject,
  )
  .returns(File::Stat)
  def self.stat(file); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.sticky?(file); end

  sig(
      old: String,
      new: String,
  )
  .returns(Integer)
  def self.symlink(old, new); end

  sig(
      file: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.symlink?(file); end

  sig(
      file: String,
      arg0: Integer,
  )
  .returns(Integer)
  def self.truncate(file, arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.umask(arg0=_); end

  sig(
      atime: Time,
      mtime: Time,
      files: String,
  )
  .returns(Integer)
  def self.utime(atime, mtime, *files); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.nilable(Integer))
  def self.world_readable?(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.nilable(Integer))
  def self.world_writable?(file); end

  sig(
      file: String,
  )
  .returns(T.nilable(Integer))
  def self.writable?(file); end

  sig(
      file: String,
  )
  .returns(T.nilable(Integer))
  def self.writable_real?(file); end

  sig(
      file: T.any(String, IO),
  )
  .returns(T.nilable(Integer))
  def self.zero?(file); end

  sig(
      arg0: BasicObject,
  )
  .returns(File)
  def <<(arg0); end

  sig.returns(Time)
  def atime(); end

  sig.returns(File)
  def binmode(); end

  sig.returns(Time)
  def birthtime(); end

  sig(
      mode: Integer,
  )
  .returns(Integer)
  def chmod(mode); end

  sig(
      owner: Integer,
      group: Integer,
  )
  .returns(Integer)
  def chown(owner, group); end

  sig.returns(File)
  def clone(); end

  sig.returns(Time)
  def ctime(); end

  sig.returns(File)
  def dup(); end

  sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(File)
  def each(sep=_, limit=_, &blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(File)
  def each_byte(&blk); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(File)
  def each_char(&blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(File)
  def each_codepoint(&blk); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(Integer, TrueClass, FalseClass))
  def flock(arg0); end

  sig.returns(File)
  def flush(); end

  sig.returns(File)
  def freeze(); end

  sig(
      file: String,
      mode: String,
      perm: String,
      opt: Integer,
  )
  .returns(Object)
  def initialize(file, mode=_, perm=_, opt=_); end

  sig.returns(File::Stat)
  def lstat(); end

  sig.returns(Time)
  def mtime(); end

  sig.returns(String)
  def path(); end

  sig(
      ext_or_ext_int_enc: T.any(String, Encoding),
  )
  .returns(File)
  sig(
      ext_or_ext_int_enc: T.any(String, Encoding),
      int_enc: T.any(String, Encoding),
  )
  .returns(File)
  def set_encoding(ext_or_ext_int_enc=_, int_enc=_); end

  sig.returns(Integer)
  def size(); end

  sig.returns(File)
  def taint(); end

  sig.returns(File)
  def to_io(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def truncate(arg0); end

  sig.returns(File)
  def trust(); end

  sig.returns(File)
  def untaint(); end

  sig.returns(File)
  def untrust(); end

  sig(
      pattern: String,
      path: String,
      flags: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.fnmatch?(pattern, path, flags=_); end

  sig(
      files: String,
  )
  .returns(Integer)
  def self.unlink(*files); end

  sig.returns(String)
  def to_path(); end
end

class IO
  extend T::Generic
  Elem = type_member(fixed: String)

  sig(
      arg0: BasicObject,
  )
  .returns(IO)
  def <<(arg0); end

  sig(
      arg0: Symbol,
      offset: Integer,
      len: Integer,
  )
  .returns(NilClass)
  def advise(arg0, offset=_, len=_); end

  sig(
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def autoclose=(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def autoclose?(); end

  sig.returns(IO)
  def binmode(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def binmode?(); end

  sig.returns(IO)
  def clone(); end

  sig.returns(NilClass)
  def close(); end

  sig(
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def close_on_exec=(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def close_on_exec?(); end

  sig.returns(NilClass)
  def close_read(); end

  sig.returns(NilClass)
  def close_write(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def closed?(); end

  sig.returns(IO)
  def dup(); end

  sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(IO)
  sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def each(sep=_, limit=_, &blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(IO)
  sig.returns(Enumerator[Integer])
  def each_byte(&blk); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(IO)
  sig.returns(Enumerator[String])
  def each_char(&blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(IO)
  sig.returns(Enumerator[Integer])
  def each_codepoint(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def eof(); end

  sig(
      integer_cmd: Integer,
      arg: T.any(String, Integer),
  )
  .returns(Integer)
  def fcntl(integer_cmd, arg); end

  sig.returns(T.nilable(Integer))
  def fdatasync(); end

  sig.returns(Integer)
  def fileno(); end

  sig.returns(IO)
  def flush(); end

  sig.returns(IO)
  def freeze(); end

  sig.returns(T.nilable(Integer))
  def fsync(); end

  sig.returns(T.nilable(Integer))
  def getbyte(); end

  sig.returns(T.nilable(String))
  def getc(); end

  sig(
      sep: String,
      limit: Integer,
  )
  .returns(T.nilable(String))
  def gets(sep=_, limit=_); end

  sig(
      fd: Integer,
      mode: Integer,
      opt: Integer,
  )
  .returns(Object)
  def initialize(fd, mode=_, opt=_); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Encoding)
  def internal_encoding(); end

  sig(
      integer_cmd: Integer,
      arg: T.any(String, Integer),
  )
  .returns(Integer)
  def ioctl(integer_cmd, arg); end

  sig.returns(T.any(TrueClass, FalseClass))
  def isatty(); end

  sig.returns(Integer)
  def lineno(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def lineno=(arg0); end

  sig.returns(Integer)
  def pid(); end

  sig.returns(Integer)
  def pos(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def pos=(arg0); end

  sig(
      arg0: BasicObject,
  )
  .returns(NilClass)
  def print(*arg0); end

  sig(
      format_string: String,
      arg0: BasicObject,
  )
  .returns(NilClass)
  def printf(format_string, *arg0); end

  sig(
      arg0: T.any(Numeric, String),
  )
  .returns(T.untyped)
  def putc(arg0); end

  sig(
      arg0: BasicObject,
  )
  .returns(NilClass)
  def puts(*arg0); end

  sig(
      length: Integer,
      outbuf: String,
  )
  .returns(T.nilable(String))
  def read(length=_, outbuf=_); end

  sig(
      len: Integer,
  )
  .returns(String)
  sig(
      len: Integer,
      buf: String,
  )
  .returns(String)
  def read_nonblock(len, buf=_); end

  sig.returns(Integer)
  def readbyte(); end

  sig.returns(String)
  def readchar(); end

  sig(
      sep: String,
      limit: Integer,
  )
  .returns(String)
  def readline(sep=_, limit=_); end

  sig(
      sep: String,
      limit: Integer,
  )
  .returns(T::Array[String])
  def readlines(sep=_, limit=_); end

  sig(
      maxlen: Integer,
  )
  .returns(String)
  sig(
      maxlen: Integer,
      outbuf: String,
  )
  .returns(String)
  def readpartial(maxlen, outbuf=_); end

  sig(
      other_IO_or_path: IO,
  )
  .returns(IO)
  sig(
      other_IO_or_path: String,
      mode_str: String,
  )
  .returns(IO)
  def reopen(other_IO_or_path, mode_str=_); end

  sig.returns(Integer)
  def rewind(); end

  sig(
      amount: Integer,
      whence: Integer,
  )
  .returns(Integer)
  def seek(amount, whence=_); end

  sig(
      ext_or_ext_int_enc: T.any(String, Encoding),
  )
  .returns(IO)
  sig(
      ext_or_ext_int_enc: T.any(String, Encoding),
      int_enc: T.any(String, Encoding),
  )
  .returns(IO)
  def set_encoding(ext_or_ext_int_enc=_, int_enc=_); end

  sig.returns(File::Stat)
  def stat(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def sync(); end

  sig(
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def sync=(arg0); end

  sig(
      maxlen: Integer,
      outbuf: String,
  )
  .returns(String)
  def sysread(maxlen, outbuf); end

  sig(
      amount: Integer,
      whence: Integer,
  )
  .returns(Integer)
  def sysseek(amount, whence=_); end

  sig(
      arg0: String,
  )
  .returns(Integer)
  def syswrite(arg0); end

  sig.returns(IO)
  def taint(); end

  sig.returns(Integer)
  def tell(); end

  sig.returns(IO)
  def to_io(); end

  sig.returns(IO)
  def trust(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def tty?(); end

  sig(
      arg0: T.any(String, Integer),
  )
  .returns(NilClass)
  def ungetbyte(arg0); end

  sig(
      arg0: String,
  )
  .returns(NilClass)
  def ungetc(arg0); end

  sig.returns(IO)
  def untaint(); end

  sig.returns(IO)
  def untrust(); end

  sig(
      arg0: String,
  )
  .returns(Integer)
  def write(arg0); end

  sig(
      name: String,
      length: Integer,
      offset: Integer,
  )
  .returns(String)
  def self.binread(name, length=_, offset=_); end

  sig(
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
  )
  .returns(Integer)
  def self.binwrite(name, arg0, offset=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  sig(
      src: T.any(String, IO),
      dst: T.any(String, IO),
      copy_length: Integer,
      src_offset: Integer,
  )
  .returns(Integer)
  def self.copy_stream(src, dst, copy_length=_, src_offset=_); end

  sig(
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
  )
  .returns(String)
  def self.read(name, length=_, offset=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  sig(
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
  )
  .returns(T::Array[String])
  def self.readlines(name, sep=_, limit=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  sig(
      read_array: T::Array[IO],
      write_array: T::Array[IO],
      error_array: T::Array[IO],
      timeout: Integer,
  )
  .returns(T.nilable(T::Array[IO]))
  def self.select(read_array, write_array=_, error_array=_, timeout=_); end

  sig(
      path: String,
      mode: String,
      perm: String,
  )
  .returns(Integer)
  def self.sysopen(path, mode=_, perm=_); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.nilable(IO))
  def self.try_convert(arg0); end

  sig(
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
  )
  .returns(Integer)
  def self.write(name, arg0, offset=_, external_encoding: _, internal_encoding: _, encoding: _, textmode: _, binmode: _, autoclose: _, mode: _); end

  sig(
      fd: Integer,
      mode: Integer,
      opt: Integer,
  )
  .returns(IO)
  def self.for_fd(fd, mode=_, opt=_); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(IO)
  sig.returns(Enumerator[Integer])
  def bytes(&blk); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(IO)
  sig.returns(Enumerator[String])
  def chars(&blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(IO)
  sig.returns(Enumerator[Integer])
  def codepoints(&blk); end

  sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(IO)
  sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def each_line(sep=_, limit=_, &blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def eof?(); end

  sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(IO)
  sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def lines(sep=_, limit=_, &blk); end

  sig.returns(Integer)
  def to_i(); end
end

class StringIO
  extend T::Generic
  Elem = type_member(fixed: String)

  sig.returns(StringIO)
  def clone(); end

  sig.returns(StringIO)
  def dup(); end

  sig.returns(StringIO)
  def freeze(); end

  sig.returns(StringIO)
  def taint(); end

  sig.returns(StringIO)
  def trust(); end

  sig.returns(StringIO)
  def untaint(); end

  sig.returns(StringIO)
  def untrust(); end
end

class Struct
  extend T::Generic
  Elem = type_member(fixed: T.untyped)

  sig.returns(Struct)
  def clone(); end

  sig.returns(Struct)
  def dup(); end

  sig.returns(Struct)
  def freeze(); end

  sig.returns(Struct)
  def taint(); end

  sig.returns(Struct)
  def trust(); end

  sig.returns(Struct)
  def untaint(); end

  sig.returns(Struct)
  def untrust(); end
end

module Abbrev
  sig(
      words: T::Array[String],
  )
  .returns(T::Hash[String, String])
  def self.abbrev(words); end
end

class ArgumentError
  sig.returns(ArgumentError)
  def clone(); end

  sig.returns(ArgumentError)
  def dup(); end

  sig.returns(ArgumentError)
  def freeze(); end

  sig.returns(ArgumentError)
  def taint(); end

  sig.returns(ArgumentError)
  def trust(); end

  sig.returns(ArgumentError)
  def untaint(); end

  sig.returns(ArgumentError)
  def untrust(); end
end

class Array
  extend T::Generic
  Elem = type_member

  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
  )
  .returns(T::Array[T.type_parameter(:U)])
  def self.[](*arg0); end

  sig(
      arg0: T::Array[Elem],
  )
  .returns(T::Array[Elem])
  def &(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  sig(
      arg0: String,
  )
  .returns(String)
  def *(arg0); end

  sig(
      arg0: Enumerable[Elem],
  )
  .returns(T::Array[Elem])
  sig(
      arg0: T::Array[Elem],
  )
  .returns(T::Array[Elem])
  def +(arg0); end

  sig(
      arg0: T::Array[Elem],
  )
  .returns(T::Array[Elem])
  def -(arg0); end

  sig(
      arg0: Elem,
  )
  .returns(T::Array[Elem])
  def <<(arg0); end

  sig(
      arg0: Range[Integer],
  )
  .returns(T::Array[Elem])
  sig(
      arg0: T.any(Integer, Float),
  )
  .returns(Elem)
  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T::Array[Elem])
  def [](arg0, arg1=_); end

  sig(
      arg0: Integer,
      arg1: Elem,
  )
  .returns(Elem)
  sig(
      arg0: Integer,
      arg1: Integer,
      arg2: Elem,
  )
  .returns(Elem)
  sig(
      arg0: Range[Integer],
      arg1: Elem,
  )
  .returns(Elem)
  def []=(arg0, arg1, arg2=_); end

  sig(
      arg0: Elem,
  )
  .returns(T::Array[Elem])
  def assoc(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Elem)
  def at(arg0); end

  sig.returns(T::Array[Elem])
  def clear(); end

  sig.returns(T::Array[Elem])
  def clone(); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Array[T.type_parameter(:U)])
  sig.returns(Enumerator[Elem])
  def collect(&blk); end

  sig(
      arg0: Integer,
      blk: T.proc(arg0: T::Array[Elem]).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig(
      arg0: Integer,
  )
  .returns(Enumerator[Elem])
  def combination(arg0, &blk); end

  sig.returns(T::Array[Elem])
  def compact(); end

  sig.returns(T::Array[Elem])
  def compact!(); end

  sig(
      arg0: T::Array[Elem],
  )
  .returns(T::Array[Elem])
  def concat(arg0); end

  sig.returns(Integer)
  sig(
      arg0: Elem,
  )
  .returns(Integer)
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Integer)
  def count(arg0=_, &blk); end

  sig(
      arg0: Integer,
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.untyped)
  sig(
      arg0: Integer,
  )
  .returns(Enumerator[Elem])
  def cycle(arg0=_, &blk); end

  sig(
      arg0: Elem,
  )
  .returns(Elem)
  sig(
      blk: T.proc().returns(Elem),
  )
  .returns(Elem)
  def delete(arg0=_, &blk); end

  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  def delete_at(arg0); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def delete_if(&blk); end

  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  def drop(arg0); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def drop_while(&blk); end

  sig.returns(T::Array[Elem])
  def dup(); end

  sig.returns(Enumerator[Elem])
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  def each(&blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def each_index(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def empty?(); end

  sig(
      arg0: Integer,
  )
  .returns(Elem)
  sig(
      arg0: Integer,
      arg1: Elem,
  )
  .returns(Elem)
  sig(
      arg0: Integer,
      blk: T.proc(arg0: Integer).returns(Elem),
  )
  .returns(Elem)
  def fetch(arg0, arg1=_, &blk); end

  sig(
      arg0: Elem,
  )
  .returns(T::Array[Elem])
  sig(
      arg0: Elem,
      arg1: Integer,
      arg2: Integer,
  )
  .returns(T::Array[Elem])
  sig(
      arg0: Elem,
      arg1: Range[Integer],
  )
  .returns(T::Array[Elem])
  sig(
      blk: T.proc(arg0: Integer).returns(Elem),
  )
  .returns(T::Array[Elem])
  sig(
      arg0: Integer,
      arg1: Integer,
      blk: T.proc(arg0: Integer).returns(Elem),
  )
  .returns(T::Array[Elem])
  sig(
      arg0: Range[Integer],
      blk: T.proc(arg0: Integer).returns(Elem),
  )
  .returns(T::Array[Elem])
  def fill(arg0=_, arg1=_, arg2=_, &blk); end

  sig.returns(Elem)
  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  def first(arg0=_); end

  sig.returns(T::Array[T.untyped])
  def flatten(); end

  sig.returns(T::Array[Elem])
  def freeze(); end

  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(arg0); end

  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
  )
  .returns(Integer)
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Integer)
  sig.returns(Enumerator[Elem])
  def index(arg0=_, &blk); end

  sig.returns(Object)
  sig(
      arg0: Integer,
  )
  .returns(Object)
  sig(
      arg0: Integer,
      arg1: Elem,
  )
  .returns(Object)
  def initialize(arg0=_, arg1=_); end

  sig(
      arg0: Integer,
      arg1: Elem,
  )
  .returns(T::Array[Elem])
  def insert(arg0, *arg1); end

  sig.returns(String)
  def inspect(); end

  sig(
      arg0: String,
  )
  .returns(String)
  def join(arg0=_); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  def keep_if(&blk); end

  sig.returns(Elem)
  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  def last(arg0=_); end

  sig.returns(Integer)
  def length(); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Array[T.type_parameter(:U)])
  sig.returns(Enumerator[Elem])
  def map(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Array[T.type_parameter(:U)])
  sig.returns(Enumerator[Elem])
  def map!(&blk); end

  sig(
      arg0: Elem,
  )
  .returns(T.any(TrueClass, FalseClass))
  def member?(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Enumerator[Elem])
  sig(
      arg0: Integer,
      blk: T.proc(arg0: T::Array[Elem]).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  def permutation(arg0=_, &blk); end

  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  sig.returns(Elem)
  def pop(arg0=_); end

  type_parameters(:U).sig(
      arg0: T::Array[T.type_parameter(:U)],
  )
  .returns(T::Array[T::Array[T.any(Elem, T.type_parameter(:U))]])
  def product(*arg0); end

  sig(
      arg0: Elem,
  )
  .returns(T::Array[Elem])
  def push(*arg0); end

  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
  )
  .returns(Elem)
  def rassoc(arg0); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def reject(&blk); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def reject!(&blk); end

  sig(
      arg0: Integer,
      blk: T.proc(arg0: T::Array[Elem]).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig(
      arg0: Integer,
  )
  .returns(Enumerator[Elem])
  def repeated_combination(arg0, &blk); end

  sig(
      arg0: Integer,
      blk: T.proc(arg0: T::Array[Elem]).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig(
      arg0: Integer,
  )
  .returns(Enumerator[Elem])
  def repeated_permutation(arg0, &blk); end

  sig.returns(T::Array[Elem])
  def reverse(); end

  sig.returns(T::Array[Elem])
  def reverse!(); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def reverse_each(&blk); end

  sig(
      arg0: Elem,
  )
  .returns(Elem)
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Integer)
  sig.returns(Enumerator[Elem])
  def rindex(arg0=_, &blk); end

  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  def rotate(arg0=_); end

  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  def rotate!(arg0=_); end

  sig.returns(Elem)
  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  def sample(arg0=_); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def select(&blk); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def select!(&blk); end

  sig.returns(Elem)
  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  def shift(arg0=_); end

  sig.returns(T::Array[Elem])
  def shuffle(); end

  sig.returns(T::Array[Elem])
  def shuffle!(); end

  sig(
      arg0: Range[Integer],
  )
  .returns(T::Array[Elem])
  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T::Array[Elem])
  sig(
      arg0: T.any(Integer, Float),
  )
  .returns(Elem)
  def slice!(arg0, arg1=_); end

  sig.returns(T::Array[Elem])
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def sort(&blk); end

  sig.returns(T::Array[Elem])
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def sort!(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def sort_by!(&blk); end

  sig.returns(T::Array[Elem])
  def taint(); end

  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  def take(arg0); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def take_while(&blk); end

  sig.returns(T::Array[Elem])
  def to_a(); end

  sig.returns(T::Array[Elem])
  def to_ary(); end

  sig.returns(T::Array[Elem])
  def transpose(); end

  sig.returns(T::Array[Elem])
  def trust(); end

  sig.returns(T::Array[Elem])
  def uniq(); end

  sig.returns(T::Array[Elem])
  def uniq!(); end

  sig(
      arg0: Elem,
  )
  .returns(T::Array[Elem])
  def unshift(*arg0); end

  sig.returns(T::Array[Elem])
  def untaint(); end

  sig.returns(T::Array[Elem])
  def untrust(); end

  sig(
      arg0: T.any(Range[Integer], Integer),
  )
  .returns(T::Array[Elem])
  def values_at(*arg0); end

  type_parameters(:U).sig(
      arg0: T::Array[T.type_parameter(:U)],
  )
  .returns(T::Array[T::Array[T.any(Elem, T.type_parameter(:U))]])
  def zip(*arg0); end

  sig(
      arg0: T::Array[Elem],
  )
  .returns(T::Array[Elem])
  def |(arg0); end

  sig.returns(Integer)
  def size(); end

  sig(
      arg0: Range[Integer],
  )
  .returns(T::Array[Elem])
  sig(
      arg0: T.any(Integer, Float),
  )
  .returns(Elem)
  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T::Array[Elem])
  def slice(arg0, arg1=_); end

  sig.returns(String)
  def to_s(); end
end

module Base64
  sig(
      str: String,
  )
  .returns(String)
  def self.decode64(str); end

  sig(
      bin: String,
  )
  .returns(String)
  def self.encode64(bin); end

  sig(
      str: String,
  )
  .returns(String)
  def self.strict_decode64(str); end

  sig(
      bin: String,
  )
  .returns(String)
  def self.strict_encode64(bin); end

  sig(
      str: String,
  )
  .returns(String)
  def self.urlsafe_decode64(str); end

  sig(
      bin: String,
  )
  .returns(String)
  def self.urlsafe_encode64(bin); end
end

class BasicObject
  sig.returns(T.any(TrueClass, FalseClass))
  def !(); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def !=(other); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(other); end

  sig(
      arg0: Symbol,
      arg1: BasicObject,
  )
  .returns(T.untyped)
  def __send__(arg0, *arg1); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(other); end

  sig(
      arg0: String,
      filename: String,
      lineno: Integer,
  )
  .returns(T.untyped)
  sig(
      blk: T.proc().returns(BasicObject),
  )
  .returns(T.untyped)
  def instance_eval(arg0=_, filename=_, lineno=_, &blk); end

  sig(
      args: BasicObject,
      blk: BasicObject,
  )
  .returns(T.untyped)
  def instance_exec(*args, &blk); end

  sig.returns(Integer)
  def __id__(); end
end

module Benchmark
  sig(
      caption: String,
      label_width: Integer,
      format: String,
      labels: String,
  )
  .returns(T::Array[Benchmark::Tms])
  def self.benchmark(caption, label_width=_, format=_, *labels); end

  sig(
      label_width: Integer,
      labels: String,
      blk: T.proc(arg0: Process).returns(NilClass),
  )
  .returns(T::Array[Benchmark::Tms])
  def self.bm(label_width=_, *labels, &blk); end

  sig(
      width: Integer,
      blk: T.proc(arg0: Process).returns(NilClass),
  )
  .returns(T::Array[Benchmark::Tms])
  def self.bmbm(width=_, &blk); end

  sig(
      label: String,
  )
  .returns(Benchmark::Tms)
  def self.measure(label=_); end

  sig(
      blk: BasicObject,
  )
  .returns(Integer)
  def self.realtime(&blk); end
end

class BigDecimal
  sig(
      arg0: Numeric,
  )
  .returns(BigDecimal)
  def %(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def *(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def **(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def +(arg0); end

  sig.returns(BigDecimal)
  def +@(); end

  sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def -(arg0); end

  sig.returns(BigDecimal)
  def -@(); end

  sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def /(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Integer)
  sig(
      arg0: Rational,
  )
  .returns(Integer)
  sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def <=>(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(arg0); end

  sig.returns(String)
  def _dump(); end

  sig.returns(BigDecimal)
  def abs(); end

  sig.returns(BigDecimal)
  def abs2(); end

  sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
      arg1: Integer,
  )
  .returns(BigDecimal)
  def add(arg0, arg1); end

  sig.returns(Numeric)
  def angle(); end

  sig.returns(Numeric)
  def arg(); end

  sig.returns(Integer)
  def ceil(); end

  sig.returns(BigDecimal)
  def clone(); end

  sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns([BigDecimal, BigDecimal])
  def coerce(arg0); end

  sig.returns(BigDecimal)
  def conj(); end

  sig.returns(BigDecimal)
  def conjugate(); end

  sig.returns(Integer)
  def denominator(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Integer)
  sig(
      arg0: Rational,
  )
  .returns(Integer)
  sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def div(arg0); end

  sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def divmod(arg0); end

  sig.returns(BigDecimal)
  def dup(); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(arg0); end

  sig.returns(Integer)
  def exponent(); end

  sig(
      arg0: Integer,
  )
  .returns(Float)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Float)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def fdiv(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def finite?(); end

  sig.returns(BigDecimal)
  def fix(); end

  sig.returns(Integer)
  def floor(); end

  sig.returns(BigDecimal)
  def frac(); end

  sig.returns(BigDecimal)
  def freeze(); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(Integer)
  def imag(); end

  sig.returns(Integer)
  def imaginary(); end

  sig.returns(T.nilable(Integer))
  def infinite?(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(BigDecimal)
  def magnitude(); end

  sig(
      arg0: Numeric,
  )
  .returns(BigDecimal)
  def modulo(arg0); end

  sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
      arg1: Integer,
  )
  .returns(BigDecimal)
  def mult(arg0, arg1); end

  sig.returns(T.any(TrueClass, FalseClass))
  def nan?(); end

  sig.returns(Object)
  def nonzero?(); end

  sig.returns(Integer)
  def numerator(); end

  sig.returns(Numeric)
  def phase(); end

  sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def power(arg0); end

  sig.returns([Integer, Integer])
  def precs(); end

  sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def quo(arg0); end

  sig.returns(BigDecimal)
  def real(); end

  sig.returns(TrueClass)
  def real?(); end

  sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(BigDecimal)
  def remainder(arg0); end

  sig.returns(Integer)
  sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  def round(arg0=_); end

  sig.returns(Integer)
  def sign(); end

  sig.returns([Integer, String, Integer, Integer])
  def split(); end

  sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  def sqrt(arg0); end

  sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
      arg1: Integer,
  )
  .returns(BigDecimal)
  def sub(arg0, arg1); end

  sig.returns(BigDecimal)
  def taint(); end

  sig.returns(Complex)
  def to_c(); end

  sig.returns(Float)
  def to_f(); end

  sig.returns(Integer)
  def to_i(); end

  sig.returns(Integer)
  def to_int(); end

  sig.returns(Rational)
  def to_r(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Integer)
  sig(
      arg0: Integer,
  )
  .returns(Rational)
  def truncate(arg0=_); end

  sig.returns(BigDecimal)
  def trust(); end

  sig.returns(BigDecimal)
  def untaint(); end

  sig.returns(BigDecimal)
  def untrust(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end

module BigMath
  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(BigDecimal)
  def self.exp(arg0, arg1); end

  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(BigDecimal)
  def self.log(arg0, arg1); end

  sig(
      prec: Integer,
  )
  .returns(BigDecimal)
  def E(prec); end

  sig(
      prec: Integer,
  )
  .returns(BigDecimal)
  def PI(prec); end

  sig(
      x: Integer,
      prec: Integer,
  )
  .returns(BigDecimal)
  def atan(x, prec); end

  sig(
      x: Integer,
      prec: Integer,
  )
  .returns(BigDecimal)
  def cos(x, prec); end

  sig(
      x: Integer,
      prec: Integer,
  )
  .returns(BigDecimal)
  def sin(x, prec); end

  sig(
      x: Integer,
      prec: Integer,
  )
  .returns(BigDecimal)
  def sqrt(x, prec); end
end

class Binding
  sig.returns(Binding)
  def clone(); end

  sig.returns(Binding)
  def dup(); end

  sig.returns(Binding)
  def freeze(); end

  sig.returns(Binding)
  def taint(); end

  sig.returns(Binding)
  def trust(); end

  sig.returns(Binding)
  def untaint(); end

  sig.returns(Binding)
  def untrust(); end
end

class Class
  sig.returns(T.untyped)
  def allocate(); end

  sig.returns(Class)
  def clone(); end

  sig.returns(Class)
  def dup(); end

  sig.returns(Class)
  def freeze(); end

  sig(
      arg0: Module,
  )
  .returns(Class)
  def include(*arg0); end

  sig(
      arg0: Class,
  )
  .returns(T.untyped)
  def inherited(arg0); end

  sig(
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def instance_methods(arg0=_); end

  sig.returns(String)
  def name(); end

  sig(
      arg0: Module,
  )
  .returns(Class)
  def prepend(*arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(Class)
  def private_class_method(*arg0); end

  sig(
      arg0: Symbol,
  )
  .returns(Class)
  def private_constant(*arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(Class)
  def public_class_method(*arg0); end

  sig(
      arg0: Symbol,
  )
  .returns(Class)
  def public_constant(*arg0); end

  sig.returns(T.nilable(Class))
  sig.returns(Class)
  def superclass(); end

  sig.returns(Class)
  def taint(); end

  sig.returns(Class)
  def trust(); end

  sig.returns(Class)
  def untaint(); end

  sig.returns(Class)
  def untrust(); end
end

class ClosedQueueError
  sig.returns(ClosedQueueError)
  def clone(); end

  sig.returns(ClosedQueueError)
  def dup(); end

  sig.returns(ClosedQueueError)
  def freeze(); end

  sig.returns(ClosedQueueError)
  def taint(); end

  sig.returns(ClosedQueueError)
  def trust(); end

  sig.returns(ClosedQueueError)
  def untaint(); end

  sig.returns(ClosedQueueError)
  def untrust(); end
end

class Complex
  sig(
      arg0: Integer,
  )
  .returns(Complex)
  sig(
      arg0: Float,
  )
  .returns(Complex)
  sig(
      arg0: Rational,
  )
  .returns(Complex)
  sig(
      arg0: BigDecimal,
  )
  .returns(Complex)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def *(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Complex)
  sig(
      arg0: Float,
  )
  .returns(Complex)
  sig(
      arg0: Rational,
  )
  .returns(Complex)
  sig(
      arg0: BigDecimal,
  )
  .returns(Complex)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def **(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Complex)
  sig(
      arg0: Float,
  )
  .returns(Complex)
  sig(
      arg0: Rational,
  )
  .returns(Complex)
  sig(
      arg0: BigDecimal,
  )
  .returns(Complex)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def +(arg0); end

  sig.returns(Complex)
  def +@(); end

  sig(
      arg0: Integer,
  )
  .returns(Complex)
  sig(
      arg0: Float,
  )
  .returns(Complex)
  sig(
      arg0: Rational,
  )
  .returns(Complex)
  sig(
      arg0: BigDecimal,
  )
  .returns(Complex)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def -(arg0); end

  sig.returns(Complex)
  def -@(); end

  sig(
      arg0: Integer,
  )
  .returns(Complex)
  sig(
      arg0: Float,
  )
  .returns(Complex)
  sig(
      arg0: Rational,
  )
  .returns(Complex)
  sig(
      arg0: BigDecimal,
  )
  .returns(Complex)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def /(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig.returns(Numeric)
  def abs(); end

  sig.returns(Numeric)
  def abs2(); end

  sig.returns(Float)
  def angle(); end

  sig.returns(Float)
  def arg(); end

  sig.returns(Complex)
  def clone(); end

  sig(
      arg0: Numeric,
  )
  .returns([Complex, Complex])
  def coerce(arg0); end

  sig.returns(Complex)
  def conj(); end

  sig.returns(Complex)
  def conjugate(); end

  sig.returns(Integer)
  def denominator(); end

  sig.returns(Complex)
  def dup(); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(Complex)
  def fdiv(arg0); end

  sig.returns(Complex)
  def freeze(); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(T.any(Integer, Float, Rational, BigDecimal))
  def imag(); end

  sig.returns(T.any(Integer, Float, Rational, BigDecimal))
  def imaginary(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(T.any(Integer, Float, Rational, BigDecimal))
  def magnitude(); end

  sig.returns(Complex)
  def numerator(); end

  sig.returns(Float)
  def phase(); end

  sig.returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def polar(); end

  sig(
      arg0: Integer,
  )
  .returns(Complex)
  sig(
      arg0: Float,
  )
  .returns(Complex)
  sig(
      arg0: Rational,
  )
  .returns(Complex)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def quo(arg0); end

  sig.returns(Rational)
  sig(
      arg0: Numeric,
  )
  .returns(Rational)
  def rationalize(arg0=_); end

  sig.returns(T.any(Integer, Float, Rational, BigDecimal))
  def real(); end

  sig.returns(FalseClass)
  def real?(); end

  sig.returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def rect(); end

  sig.returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def rectangular(); end

  sig.returns(Complex)
  def taint(); end

  sig.returns(Complex)
  def to_c(); end

  sig.returns(Float)
  def to_f(); end

  sig.returns(Integer)
  def to_i(); end

  sig.returns(Rational)
  def to_r(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Complex)
  def trust(); end

  sig.returns(Complex)
  def untaint(); end

  sig.returns(Complex)
  def untrust(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end

module Coverage
  sig.returns(T::Hash[String, T::Array[T.nilable(Integer)]])
  def self.result(); end

  sig.returns(NilClass)
  def self.start(); end
end

class Data
  sig.returns(Data)
  def clone(); end

  sig.returns(Data)
  def dup(); end

  sig.returns(Data)
  def freeze(); end

  sig.returns(Data)
  def taint(); end

  sig.returns(Data)
  def trust(); end

  sig.returns(Data)
  def untaint(); end

  sig.returns(Data)
  def untrust(); end
end

class Date
  sig.returns(Date)
  def clone(); end

  sig.returns(Date)
  def dup(); end

  sig.returns(Date)
  def freeze(); end

  sig.returns(Date)
  def taint(); end

  sig.returns(Date)
  def trust(); end

  sig.returns(Date)
  def untaint(); end

  sig.returns(Date)
  def untrust(); end
end

class DateTime
  sig.returns(DateTime)
  def clone(); end

  sig.returns(DateTime)
  def dup(); end

  sig.returns(DateTime)
  def freeze(); end

  sig.returns(DateTime)
  def taint(); end

  sig.returns(DateTime)
  def trust(); end

  sig.returns(DateTime)
  def untaint(); end

  sig.returns(DateTime)
  def untrust(); end
end

class EOFError
  sig.returns(EOFError)
  def clone(); end

  sig.returns(EOFError)
  def dup(); end

  sig.returns(EOFError)
  def freeze(); end

  sig.returns(EOFError)
  def taint(); end

  sig.returns(EOFError)
  def trust(); end

  sig.returns(EOFError)
  def untaint(); end

  sig.returns(EOFError)
  def untrust(); end
end

class Encoding
  sig.returns(T::Hash[String, String])
  def self.aliases(); end

  sig(
      obj1: BasicObject,
      obj2: BasicObject,
  )
  .returns(T.nilable(Encoding))
  def self.compatible?(obj1, obj2); end

  sig.returns(Encoding)
  def self.default_external(); end

  sig(
      arg0: String,
  )
  .returns(String)
  sig(
      arg0: Encoding,
  )
  .returns(Encoding)
  def self.default_external=(arg0); end

  sig.returns(Encoding)
  def self.default_internal(); end

  sig(
      arg0: String,
  )
  .returns(T.nilable(String))
  sig(
      arg0: Encoding,
  )
  .returns(T.nilable(Encoding))
  def self.default_internal=(arg0); end

  sig(
      arg0: T.any(String, Encoding),
  )
  .returns(Encoding)
  def self.find(arg0); end

  sig.returns(T::Array[Encoding])
  def self.list(); end

  sig.returns(T::Array[String])
  def self.name_list(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def ascii_compatible?(); end

  sig.returns(Encoding)
  def clone(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def dummy?(); end

  sig.returns(Encoding)
  def dup(); end

  sig.returns(Encoding)
  def freeze(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(String)
  def name(); end

  sig.returns(T::Array[String])
  def names(); end

  sig(
      name: String,
  )
  .returns(Encoding)
  def replicate(name); end

  sig.returns(Encoding)
  def taint(); end

  sig.returns(Encoding)
  def trust(); end

  sig.returns(Encoding)
  def untaint(); end

  sig.returns(Encoding)
  def untrust(); end

  sig.returns(String)
  def to_s(); end
end

class EncodingError
  sig.returns(EncodingError)
  def clone(); end

  sig.returns(EncodingError)
  def dup(); end

  sig.returns(EncodingError)
  def freeze(); end

  sig.returns(EncodingError)
  def taint(); end

  sig.returns(EncodingError)
  def trust(); end

  sig.returns(EncodingError)
  def untaint(); end

  sig.returns(EncodingError)
  def untrust(); end
end

module Enumerable
  extend T::Generic
  Elem = type_member

  sig.returns(T.any(TrueClass, FalseClass))
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.any(TrueClass, FalseClass))
  def all?(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.any(TrueClass, FalseClass))
  def any?(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Array[T.type_parameter(:U)])
  sig.returns(Enumerator[Elem])
  def collect(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(Enumerator[T.type_parameter(:U)]),
  )
  .returns(T::Array[T.type_parameter(:U)])
  def collect_concat(&blk); end

  sig.returns(Integer)
  sig(
      arg0: BasicObject,
  )
  .returns(Integer)
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Integer)
  def count(arg0=_, &blk); end

  sig(
      n: Integer,
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(NilClass)
  sig(
      n: Integer,
  )
  .returns(Enumerator[Elem])
  def cycle(n=_, &blk); end

  sig(
      ifnone: Proc,
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(Elem))
  sig(
      ifnone: Proc,
  )
  .returns(Enumerator[Elem])
  def detect(ifnone=_, &blk); end

  sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  def drop(n); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def drop_while(&blk); end

  sig(
      n: Integer,
      blk: T.proc(arg0: T::Array[Elem]).returns(BasicObject),
  )
  .returns(NilClass)
  sig(
      n: Integer,
  )
  .returns(Enumerator[Elem])
  def each_cons(n, &blk); end

  sig(
      blk: T.proc(arg0: Elem, arg1: Integer).returns(BasicObject),
  )
  .returns(Enumerable[Elem])
  sig.returns(Enumerable[Elem])
  def each_with_index(&blk); end

  sig.returns(T::Array[Elem])
  def entries(); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def find_all(&blk); end

  sig(
      value: BasicObject,
  )
  .returns(T.nilable(Integer))
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(Integer))
  sig.returns(Enumerator[Elem])
  def find_index(value=_, &blk); end

  sig.returns(T.nilable(Elem))
  sig(
      n: Integer,
  )
  .returns(T.nilable(T::Array[Elem]))
  def first(n=_); end

  sig(
      arg0: BasicObject,
  )
  .returns(T::Array[Elem])
  type_parameters(:U).sig(
      arg0: BasicObject,
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Array[T.type_parameter(:U)])
  def grep(arg0, &blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Hash[T.type_parameter(:U), T::Array[Elem]])
  sig.returns(Enumerator[Elem])
  def group_by(&blk); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(arg0); end

  type_parameters(:Any).sig(
      initial: T.type_parameter(:Any),
      arg0: Symbol,
  )
  .returns(T.untyped)
  sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  sig(
      initial: Elem,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Elem),
  )
  .returns(Elem)
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Elem),
  )
  .returns(Elem)
  def inject(initial=_, arg0=_, &blk); end

  sig.returns(Elem)
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(Elem)
  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  sig(
      arg0: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def max(arg0=_, &blk); end

  sig.returns(Enumerator[Elem])
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(Elem)
  sig(
      arg0: Integer,
  )
  .returns(Enumerator[Elem])
  sig(
      arg0: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def max_by(arg0=_, &blk); end

  sig.returns(Elem)
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(Elem)
  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  sig(
      arg0: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def min(arg0=_, &blk); end

  sig.returns(Enumerator[Elem])
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(Elem)
  sig(
      arg0: Integer,
  )
  .returns(Enumerator[Elem])
  sig(
      arg0: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def min_by(arg0=_, &blk); end

  sig.returns([Elem, Elem])
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns([Elem, Elem])
  def minmax(&blk); end

  sig.returns([Elem, Elem])
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(Enumerator[Elem])
  def minmax_by(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.any(TrueClass, FalseClass))
  def none?(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.any(TrueClass, FalseClass))
  def one?(&blk); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns([T::Array[Elem], T::Array[Elem]])
  sig.returns(Enumerator[Elem])
  def partition(&blk); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def reject(&blk); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Enumerator[Elem])
  sig.returns(Enumerator[Elem])
  def reverse_each(&blk); end

  sig.returns(T::Array[Elem])
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def sort(&blk); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def sort_by(&blk); end

  sig(
      n: Integer,
  )
  .returns(T.nilable(T::Array[Elem]))
  def take(n); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def take_while(&blk); end

  sig.returns(T::Hash[Elem, Elem])
  def to_h(); end

  sig(
      n: Integer,
      blk: T.proc(arg0: T::Array[Elem]).returns(BasicObject),
  )
  .returns(NilClass)
  sig(
      n: Integer,
  )
  .returns(Enumerator[Elem])
  def each_slice(n, &blk); end

  sig(
      ifnone: Proc,
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(Elem))
  sig(
      ifnone: Proc,
  )
  .returns(Enumerator[Elem])
  def find(ifnone=_, &blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(Enumerator[T.type_parameter(:U)]),
  )
  .returns(T::Array[T.type_parameter(:U)])
  def flat_map(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Array[T.type_parameter(:U)])
  sig.returns(Enumerator[Elem])
  def map(&blk); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def member?(arg0); end

  type_parameters(:Any).sig(
      initial: T.type_parameter(:Any),
      arg0: Symbol,
  )
  .returns(T.untyped)
  sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  sig(
      initial: Elem,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Elem),
  )
  .returns(Elem)
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Elem),
  )
  .returns(Elem)
  def reduce(initial=_, arg0=_, &blk); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def select(&blk); end

  sig.returns(T::Array[Elem])
  def to_a(); end
end

class Enumerator
  extend T::Generic
  Elem = type_member

  sig.returns(Enumerator[Elem])
  def clone(); end

  sig.returns(Enumerator[Elem])
  def dup(); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.untyped)
  sig.returns(Enumerator[Elem])
  def each(&blk); end

  sig(
      blk: T.proc(arg0: Elem, arg1: Integer).returns(BasicObject),
  )
  .returns(T.untyped)
  sig.returns(Enumerator[[Elem, Integer]])
  def each_with_index(&blk); end

  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
      blk: T.proc(arg0: Elem, arg1: T.type_parameter(:U)).returns(BasicObject),
  )
  .returns(T.untyped)
  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
  )
  .returns(Enumerator[[Elem, T.type_parameter(:U)]])
  def each_with_object(arg0, &blk); end

  sig(
      arg0: Elem,
  )
  .returns(NilClass)
  def feed(arg0); end

  sig.returns(Enumerator[Elem])
  def freeze(); end

  type_parameters(:U).sig(
      arg0: Integer,
      blk: T.proc(arg0: T::Array[T.type_parameter(:U)]).returns(BasicObject),
  )
  .returns(Object)
  type_parameters(:U).sig(
      arg0: Proc,
      blk: T.proc(arg0: T::Array[T.type_parameter(:U)]).returns(BasicObject),
  )
  .returns(Object)
  def initialize(arg0=_, &blk); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Elem)
  def next(); end

  sig.returns(T::Array[Elem])
  def next_values(); end

  sig.returns(Elem)
  def peek(); end

  sig.returns(T::Array[Elem])
  def peek_values(); end

  sig.returns(Enumerator[Elem])
  def rewind(); end

  sig.returns(T.nilable(T.any(Integer, Float)))
  def size(); end

  sig.returns(Enumerator[Elem])
  def taint(); end

  sig.returns(Enumerator[Elem])
  def trust(); end

  sig.returns(Enumerator[Elem])
  def untaint(); end

  sig.returns(Enumerator[Elem])
  def untrust(); end

  sig(
      blk: T.proc(arg0: Elem, arg1: Integer).returns(BasicObject),
  )
  .returns(T.untyped)
  sig.returns(Enumerator[[Elem, Integer]])
  def with_index(&blk); end

  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
      blk: T.proc(arg0: Elem, arg1: T.type_parameter(:U)).returns(BasicObject),
  )
  .returns(T.untyped)
  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
  )
  .returns(Enumerator[[Elem, T.type_parameter(:U)]])
  def with_object(arg0, &blk); end
end

class Exception
  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig.returns(T::Array[String])
  def backtrace(); end

  sig.returns(T::Array[Thread::Backtrace::Location])
  def backtrace_locations(); end

  sig.returns(NilClass)
  def cause(); end

  sig.returns(Exception)
  def clone(); end

  sig.returns(Exception)
  def dup(); end

  sig(
      arg0: String,
  )
  .returns(Exception)
  def exception(arg0=_); end

  sig.returns(Exception)
  def freeze(); end

  sig(
      arg0: String,
  )
  .returns(Object)
  def initialize(arg0=_); end

  sig.returns(String)
  def inspect(); end

  sig.returns(String)
  def message(); end

  sig(
      arg0: T.any(String, T::Array[String]),
  )
  .returns(T::Array[String])
  def set_backtrace(arg0); end

  sig.returns(Exception)
  def taint(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Exception)
  def trust(); end

  sig.returns(Exception)
  def untaint(); end

  sig.returns(Exception)
  def untrust(); end
end

class FalseClass
  sig.returns(FalseClass)
  def clone(); end

  sig.returns(FalseClass)
  def dup(); end

  sig.returns(FalseClass)
  def freeze(); end

  sig.returns(FalseClass)
  def taint(); end

  sig.returns(FalseClass)
  def trust(); end

  sig.returns(FalseClass)
  def untaint(); end

  sig.returns(FalseClass)
  def untrust(); end
end

class Fiber
  sig.returns(Fiber)
  def clone(); end

  sig.returns(Fiber)
  def dup(); end

  sig.returns(Fiber)
  def freeze(); end

  sig.returns(Fiber)
  def taint(); end

  sig.returns(Fiber)
  def trust(); end

  sig.returns(Fiber)
  def untaint(); end

  sig.returns(Fiber)
  def untrust(); end
end

class FiberError
  sig.returns(FiberError)
  def clone(); end

  sig.returns(FiberError)
  def dup(); end

  sig.returns(FiberError)
  def freeze(); end

  sig.returns(FiberError)
  def taint(); end

  sig.returns(FiberError)
  def trust(); end

  sig.returns(FiberError)
  def untaint(); end

  sig.returns(FiberError)
  def untrust(); end
end

class File::Stat
  sig(
      other: File::Stat,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  sig.returns(Time)
  def atime(); end

  sig.returns(Time)
  def birthtime(); end

  sig.returns(T.nilable(Integer))
  def blksize(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def blockdev?(); end

  sig.returns(T.nilable(Integer))
  def blocks(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def chardev?(); end

  sig.returns(Time)
  def ctime(); end

  sig.returns(Integer)
  def dev(); end

  sig.returns(Integer)
  def dev_major(); end

  sig.returns(Integer)
  def dev_minor(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def directory?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def executable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def executable_real?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def file?(); end

  sig.returns(String)
  def ftype(); end

  sig.returns(Integer)
  def gid(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def grpowned?(); end

  sig(
      file: String,
  )
  .returns(Object)
  def initialize(file); end

  sig.returns(Integer)
  def ino(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Integer)
  def mode(); end

  sig.returns(Time)
  def mtime(); end

  sig.returns(Integer)
  def nlink(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def owned?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def pipe?(); end

  sig.returns(T.nilable(Integer))
  def rdev(); end

  sig.returns(Integer)
  def rdev_major(); end

  sig.returns(Integer)
  def rdev_minor(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def readable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def readable_real?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def setgid?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def setuid?(); end

  sig.returns(Integer)
  def size(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def socket?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def sticky?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def symlink?(); end

  sig.returns(Integer)
  def uid(); end

  sig.returns(T.nilable(Integer))
  def world_readable?(); end

  sig.returns(T.nilable(Integer))
  def world_writable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def writable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def writable_real?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end

module FileUtils
  sig(
      src: T.any(String, Pathname),
      dest: T.any(String, Pathname),
      preserve: T::Hash[Symbol, T.any(TrueClass, FalseClass)],
  )
  .returns(T::Array[String])
  def self.cp_r(src, dest, preserve=_); end

  sig(
      list: T.any(String, Pathname),
      mode: T::Hash[Symbol, T.any(TrueClass, FalseClass)],
  )
  .returns(T::Array[String])
  def self.mkdir_p(list, mode=_); end
end

class Float
  sig(
      arg0: Integer,
  )
  .returns(Float)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Float)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def %(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Float)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Float)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def *(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Float)
  sig(
      arg0: Float,
  )
  .returns(Numeric)
  sig(
      arg0: Rational,
  )
  .returns(Numeric)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def **(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Float)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Float)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def +(arg0); end

  sig.returns(Float)
  def +@(); end

  sig(
      arg0: Integer,
  )
  .returns(Float)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Float)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def -(arg0); end

  sig.returns(Float)
  def -@(); end

  sig(
      arg0: Integer,
  )
  .returns(Float)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Float)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def /(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Integer)
  sig(
      arg0: Rational,
  )
  .returns(Integer)
  sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def <=>(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(arg0); end

  sig.returns(Float)
  def abs(); end

  sig.returns(Float)
  def abs2(); end

  sig.returns(Numeric)
  def angle(); end

  sig.returns(Numeric)
  def arg(); end

  sig.returns(Integer)
  def ceil(); end

  sig.returns(Float)
  def clone(); end

  sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns([Float, Float])
  sig(
      arg0: Numeric,
  )
  .returns([Float, Float])
  def coerce(arg0); end

  sig.returns(Float)
  def conj(); end

  sig.returns(Float)
  def conjugate(); end

  sig.returns(Integer)
  def denominator(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Integer)
  sig(
      arg0: Rational,
  )
  .returns(Integer)
  sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def div(arg0); end

  sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def divmod(arg0); end

  sig.returns(Float)
  def dup(); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Float)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Float)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def fdiv(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def finite?(); end

  sig.returns(Integer)
  def floor(); end

  sig.returns(Float)
  def freeze(); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(Integer)
  def imag(); end

  sig.returns(Integer)
  def imaginary(); end

  sig.returns(Object)
  def infinite?(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Float)
  def magnitude(); end

  sig(
      arg0: Integer,
  )
  .returns(Float)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Float)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def modulo(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def nan?(); end

  sig.returns(Float)
  def next_float(); end

  sig.returns(Integer)
  def numerator(); end

  sig.returns(Numeric)
  def phase(); end

  sig.returns(Float)
  def prev_float(); end

  sig(
      arg0: Integer,
  )
  .returns(Float)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Float)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def quo(arg0); end

  sig.returns(Rational)
  sig(
      arg0: Numeric,
  )
  .returns(Rational)
  def rationalize(arg0=_); end

  sig.returns(Float)
  def real(); end

  sig.returns(TrueClass)
  def real?(); end

  sig.returns(Integer)
  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def round(arg0=_); end

  sig.returns(Float)
  def taint(); end

  sig.returns(Complex)
  def to_c(); end

  sig.returns(Float)
  def to_f(); end

  sig.returns(Integer)
  def to_i(); end

  sig.returns(Integer)
  def to_int(); end

  sig.returns(Rational)
  def to_r(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Integer)
  def truncate(); end

  sig.returns(Float)
  def trust(); end

  sig.returns(Float)
  def untaint(); end

  sig.returns(Float)
  def untrust(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end

class FloatDomainError
  sig.returns(FloatDomainError)
  def clone(); end

  sig.returns(FloatDomainError)
  def dup(); end

  sig.returns(FloatDomainError)
  def freeze(); end

  sig.returns(FloatDomainError)
  def taint(); end

  sig.returns(FloatDomainError)
  def trust(); end

  sig.returns(FloatDomainError)
  def untaint(); end

  sig.returns(FloatDomainError)
  def untrust(); end
end

module GC
  sig.returns(Integer)
  def self.count(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.disable(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.enable(); end

  sig(
      full_mark: T.any(TrueClass, FalseClass),
      immediate_sweep: T.any(TrueClass, FalseClass),
  )
  .returns(NilClass)
  def self.start(full_mark: _, immediate_sweep: _); end

  sig(
      arg0: T.any(Hash, Symbol),
  )
  .returns(T::Hash[Symbol, Integer])
  def self.stat(arg0=_); end

  sig.returns(T.any(Integer, TrueClass, FalseClass))
  def self.stress(); end
end

module Gem
  sig(
      name: String,
      args: String,
      requirements: Gem::Requirement,
  )
  .returns(String)
  def self.bin_path(name, args=_, *requirements); end

  sig.returns(String)
  def self.binary_mode(); end

  sig(
      install_dir: String,
  )
  .returns(String)
  def self.bindir(install_dir=_); end

  sig.returns(Hash)
  def self.clear_default_specs(); end

  sig.returns(NilClass)
  def self.clear_paths(); end

  sig.returns(String)
  def self.config_file(); end

  sig.returns(Gem::ConfigFile)
  def self.configuration(); end

  sig(
      config: BasicObject,
  )
  .returns(T.untyped)
  def self.configuration=(config); end

  sig(
      gem_name: String,
  )
  .returns(T.nilable(String))
  def self.datadir(gem_name); end

  sig.returns(T.nilable(String))
  def self.default_bindir(); end

  sig.returns(T.nilable(String))
  def self.default_cert_path(); end

  sig.returns(T.nilable(String))
  def self.default_dir(); end

  sig.returns(T.nilable(String))
  def self.default_exec_format(); end

  sig.returns(T.nilable(String))
  def self.default_key_path(); end

  sig.returns(T.nilable(String))
  def self.default_path(); end

  sig.returns(T.nilable(T::Array[String]))
  def self.default_rubygems_dirs(); end

  sig.returns(T.nilable(T::Array[String]))
  def self.default_sources(); end
end

class Hash
  extend T::Generic
  K = type_member
  V = type_member
  Elem = type_member

  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
  )
  .returns(T::Hash[T.type_parameter(:U), T.type_parameter(:U)])
  def self.[](*arg0); end

  sig(
      arg0: K,
  )
  .returns(T.nilable(V))
  def [](arg0); end

  sig(
      arg0: K,
      arg1: V,
  )
  .returns(V)
  def []=(arg0, arg1); end

  sig(
      arg0: K,
  )
  .returns(T::Array[T.any(K, V)])
  def assoc(arg0); end

  sig.returns(T::Hash[K, V])
  def clear(); end

  sig.returns(T::Hash[K, V])
  def clone(); end

  sig.returns(T::Hash[K, V])
  def compare_by_identity(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def compare_by_identity?(); end

  sig(
      arg0: K,
  )
  .returns(V)
  sig(
      arg0: K,
      blk: T.proc(arg0: K).returns(V),
  )
  .returns(V)
  def default(arg0=_, &blk); end

  sig(
      arg0: V,
  )
  .returns(V)
  def default=(arg0); end

  sig(
      arg0: K,
  )
  .returns(T.nilable(V))
  type_parameters(:U).sig(
      arg0: K,
      blk: T.proc(arg0: K).returns(T.type_parameter(:U)),
  )
  .returns(T.any(T.type_parameter(:U), V))
  def delete(arg0, &blk); end

  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  sig.returns(Enumerator[[K, V]])
  def delete_if(&blk); end

  sig.returns(T::Hash[K, V])
  def dup(); end

  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  sig.returns(Enumerator[[K, V]])
  def each(&blk); end

  sig(
      blk: T.proc(arg0: K).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  sig.returns(Enumerator[[K, V]])
  def each_key(&blk); end

  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  sig.returns(Enumerator[[K, V]])
  def each_pair(&blk); end

  sig(
      blk: T.proc(arg0: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  sig.returns(Enumerator[[K, V]])
  def each_value(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def empty?(); end

  sig(
      arg0: K,
  )
  .returns(V)
  sig(
      arg0: K,
      arg1: V,
  )
  .returns(V)
  sig(
      arg0: K,
      blk: T.proc(arg0: K).returns(V),
  )
  .returns(V)
  def fetch(arg0, arg1=_, &blk); end

  sig.returns(T::Hash[K, V])
  def freeze(); end

  type_parameters(:T).sig(
      arg0: T.type_parameter(:T),
  )
  .returns(T.any(TrueClass, FalseClass))
  def has_key?(arg0); end

  type_parameters(:T).sig(
      arg0: T.type_parameter(:T),
  )
  .returns(T.any(TrueClass, FalseClass))
  def has_value?(arg0); end

  sig.returns(String)
  def inspect(); end

  sig.returns(T::Hash[V, K])
  def invert(); end

  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  sig.returns(Enumerator[[K, V]])
  def keep_if(&blk); end

  type_parameters(:T).sig(
      arg0: T.type_parameter(:T),
  )
  .returns(K)
  def key(arg0); end

  type_parameters(:T).sig(
      arg0: T.type_parameter(:T),
  )
  .returns(T.any(TrueClass, FalseClass))
  def key?(arg0); end

  sig.returns(T::Array[K])
  def keys(); end

  sig.returns(Integer)
  def length(); end

  type_parameters(:T).sig(
      arg0: T.type_parameter(:T),
  )
  .returns(T.any(TrueClass, FalseClass))
  def member?(arg0); end

  type_parameters(:A ,:B).sig(
      arg0: T::Hash[T.type_parameter(:A), T.type_parameter(:B)],
  )
  .returns(T::Hash[T.any(T.type_parameter(:A), K), T.any(T.type_parameter(:B), V)])
  type_parameters(:A ,:B).sig(
      arg0: T::Hash[T.type_parameter(:A), T.type_parameter(:B)],
      blk: T.proc(arg0: K, arg1: V, arg2: T.type_parameter(:B)).returns(T.any(V, T.type_parameter(:B))),
  )
  .returns(T::Hash[T.any(T.type_parameter(:A), K), T.any(T.type_parameter(:B), V)])
  def merge(arg0, &blk); end

  sig(
      arg0: K,
  )
  .returns(T::Array[T.any(K, V)])
  def rassoc(arg0); end

  sig.returns(T::Hash[K, V])
  def rehash(); end

  sig.returns(Enumerator[[K, V]])
  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  def reject(&blk); end

  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  def reject!(&blk); end

  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  def select(&blk); end

  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  def select!(&blk); end

  sig.returns(T::Array[T.any(K, V)])
  def shift(); end

  sig.returns(Integer)
  def size(); end

  sig(
      arg0: K,
      arg1: V,
  )
  .returns(V)
  def store(arg0, arg1); end

  sig.returns(T::Hash[K, V])
  def taint(); end

  sig.returns(T::Array[T::Array[T.any(K, V)]])
  def to_a(); end

  sig.returns(T::Hash[K, V])
  def to_hash(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(T::Hash[K, V])
  def trust(); end

  sig.returns(T::Hash[K, V])
  def untaint(); end

  sig.returns(T::Hash[K, V])
  def untrust(); end

  type_parameters(:T).sig(
      arg0: T.type_parameter(:T),
  )
  .returns(T.any(TrueClass, FalseClass))
  def value?(arg0); end

  sig.returns(T::Array[V])
  def values(); end

  sig(
      arg0: K,
  )
  .returns(T::Array[V])
  def values_at(*arg0); end
end

class IOError
  sig.returns(IOError)
  def clone(); end

  sig.returns(IOError)
  def dup(); end

  sig.returns(IOError)
  def freeze(); end

  sig.returns(IOError)
  def taint(); end

  sig.returns(IOError)
  def trust(); end

  sig.returns(IOError)
  def untaint(); end

  sig.returns(IOError)
  def untrust(); end
end

class IndexError
  sig.returns(IndexError)
  def clone(); end

  sig.returns(IndexError)
  def dup(); end

  sig.returns(IndexError)
  def freeze(); end

  sig.returns(IndexError)
  def taint(); end

  sig.returns(IndexError)
  def trust(); end

  sig.returns(IndexError)
  def untaint(); end

  sig.returns(IndexError)
  def untrust(); end
end

class Integer
  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def %(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def &(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def *(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Numeric)
  sig(
      arg0: Float,
  )
  .returns(Numeric)
  sig(
      arg0: Rational,
  )
  .returns(Numeric)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def **(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def +(arg0); end

  sig.returns(Integer)
  def +@(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def -(arg0); end

  sig.returns(Integer)
  def -@(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def /(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def <<(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Integer)
  sig(
      arg0: Rational,
  )
  .returns(Integer)
  sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def <=>(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def >>(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Rational,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Integer)
  sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def [](arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def ^(arg0); end

  sig.returns(Integer)
  def abs(); end

  sig.returns(Integer)
  def abs2(); end

  sig.returns(Numeric)
  def angle(); end

  sig.returns(Numeric)
  def arg(); end

  sig.returns(Integer)
  def bit_length(); end

  sig.returns(Integer)
  def ceil(); end

  sig(
      arg0: Encoding,
  )
  .returns(String)
  def chr(arg0); end

  sig.returns(Integer)
  def clone(); end

  sig(
      arg0: Numeric,
  )
  .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def coerce(arg0); end

  sig.returns(Integer)
  def conj(); end

  sig.returns(Integer)
  def conjugate(); end

  sig.returns(Integer)
  def denominator(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Integer)
  sig(
      arg0: Rational,
  )
  .returns(Integer)
  sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def div(arg0); end

  sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def divmod(arg0); end

  sig(
      limit: Integer,
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(Integer)
  sig(
      limit: Integer,
  )
  .returns(Enumerator[Integer])
  def downto(limit, &blk); end

  sig.returns(Integer)
  def dup(); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def even?(); end

  sig(
      arg0: Integer,
  )
  .returns(Float)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Float)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def fdiv(arg0); end

  sig.returns(Integer)
  def floor(); end

  sig.returns(Integer)
  def freeze(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def gcd(arg0); end

  sig(
      arg0: Integer,
  )
  .returns([Integer, Integer])
  def gcdlcm(arg0); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(Integer)
  def imag(); end

  sig.returns(Integer)
  def imaginary(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(TrueClass)
  def integer?(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def lcm(arg0); end

  sig.returns(Integer)
  def magnitude(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def modulo(arg0); end

  sig.returns(Integer)
  def next(); end

  sig.returns(Integer)
  def numerator(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def odd?(); end

  sig.returns(Integer)
  def ord(); end

  sig.returns(Numeric)
  def phase(); end

  sig.returns(Integer)
  def pred(); end

  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def quo(arg0); end

  sig.returns(Rational)
  sig(
      arg0: Numeric,
  )
  .returns(Rational)
  def rationalize(arg0=_); end

  sig.returns(Integer)
  def real(); end

  sig.returns(TrueClass)
  def real?(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def remainder(arg0); end

  sig.returns(Integer)
  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def round(arg0=_); end

  sig.returns(Integer)
  def size(); end

  sig.returns(Integer)
  def succ(); end

  sig.returns(Integer)
  def taint(); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(Integer)
  sig.returns(Enumerator[Integer])
  def times(&blk); end

  sig.returns(Complex)
  def to_c(); end

  sig.returns(Float)
  def to_f(); end

  sig.returns(Integer)
  def to_i(); end

  sig.returns(Integer)
  def to_int(); end

  sig.returns(Rational)
  def to_r(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Integer)
  def truncate(); end

  sig.returns(Integer)
  def trust(); end

  sig.returns(Integer)
  def untaint(); end

  sig.returns(Integer)
  def untrust(); end

  sig(
      arg0: Integer,
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(Integer)
  sig(
      arg0: Integer,
  )
  .returns(Enumerator[Integer])
  def upto(arg0, &blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def |(arg0); end

  sig.returns(Integer)
  def ~(); end
end

class Interrupt
  sig.returns(Interrupt)
  def clone(); end

  sig.returns(Interrupt)
  def dup(); end

  sig.returns(Interrupt)
  def freeze(); end

  sig.returns(Interrupt)
  def taint(); end

  sig.returns(Interrupt)
  def trust(); end

  sig.returns(Interrupt)
  def untaint(); end

  sig.returns(Interrupt)
  def untrust(); end
end

module Kernel
  type_parameters(:T).sig(
      x: Object,
  )
  .returns(T::Array[T.type_parameter(:T)])
  def self.Array(x); end

  sig(
      x: Numeric,
      y: Numeric,
  )
  .returns(Complex)
  sig(
      x: String,
  )
  .returns(Complex)
  def self.Complex(x, y=_); end

  sig(
      x: Numeric,
  )
  .returns(Float)
  def self.Float(x); end

  type_parameters(:K ,:V).sig(
      x: Object,
  )
  .returns(T::Hash[T.type_parameter(:K), T.type_parameter(:V)])
  def self.Hash(x); end

  sig(
      arg: T.any(Numeric, String),
      base: Integer,
  )
  .returns(Integer)
  def self.Integer(arg, base=_); end

  sig(
      x: Numeric,
      y: Numeric,
  )
  .returns(Rational)
  sig(
      x: Object,
  )
  .returns(Rational)
  def self.Rational(x, y=_); end

  sig(
      x: Object,
  )
  .returns(String)
  def self.String(x); end

  sig.returns(T.nilable(Symbol))
  def self.__callee__(); end

  sig.returns(T.nilable(String))
  def self.__dir__(); end

  sig.returns(T.nilable(Symbol))
  def self.__method__(); end

  sig(
      arg0: String,
  )
  .returns(String)
  def self.`(arg0); end

  sig(
      msg: String,
  )
  .returns(T.noreturn)
  def self.abort(msg=_); end

  sig(
      blk: T.proc().returns(BasicObject),
  )
  .returns(Proc)
  def self.at_exit(&blk); end

  sig(
      _module: T.any(String, Symbol),
      filename: String,
  )
  .returns(NilClass)
  def self.autoload(_module, filename); end

  sig(
      name: T.any(Symbol, String),
  )
  .returns(T.nilable(String))
  def self.autoload?(name); end

  sig.returns(Binding)
  def self.binding(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.block_given?(); end

  sig(
      start_or_range: Integer,
      length: Integer,
  )
  .returns(T.nilable(T::Array[String]))
  sig(
      start_or_range: Range[Integer],
  )
  .returns(T.nilable(T::Array[String]))
  def self.caller(start_or_range=_, length=_); end

  sig(
      start_or_range: Integer,
      length: Integer,
  )
  .returns(T.nilable(T::Array[String]))
  sig(
      start_or_range: Range[Integer],
  )
  .returns(T.nilable(T::Array[String]))
  def self.caller_locations(start_or_range=_, length=_); end

  sig(
      arg0: String,
      arg1: Binding,
      filename: String,
      lineno: Integer,
  )
  .returns(T.untyped)
  def self.eval(arg0, arg1=_, filename=_, lineno=_); end

  sig.returns(T.noreturn)
  sig(
      status: T.any(Integer, TrueClass, FalseClass),
  )
  .returns(T.noreturn)
  def self.exit(status=_); end

  sig(
      status: T.any(Integer, TrueClass, FalseClass),
  )
  .returns(T.noreturn)
  def self.exit!(status); end

  sig.returns(T.noreturn)
  sig(
      arg0: String,
  )
  .returns(T.noreturn)
  sig(
      arg0: Class,
      arg1: T::Array[String],
  )
  .returns(T.noreturn)
  sig(
      arg0: Class,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  def self.fail(arg0=_, arg1=_, arg2=_); end

  sig(
      format: String,
      args: BasicObject,
  )
  .returns(String)
  def self.format(format, *args); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(String)
  def self.gets(arg0=_, arg1=_); end

  sig.returns(T::Array[Symbol])
  def self.global_variables(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.iterator?(); end

  sig(
      filename: String,
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.load(filename, arg0=_); end

  sig.returns(T::Array[Symbol])
  def self.local_variables(); end

  sig(
      name: String,
      rest: T.any(String, Integer),
      block: String,
  )
  .returns(T.nilable(IO))
  def self.open(name, rest=_, block=_); end

  sig(
      arg0: IO,
      arg1: String,
      arg2: BasicObject,
  )
  .returns(NilClass)
  def self.printf(arg0=_, arg1=_, *arg2); end

  sig(
      blk: BasicObject,
  )
  .returns(Proc)
  def self.proc(&blk); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.putc(arg0); end

  sig(
      arg0: BasicObject,
  )
  .returns(NilClass)
  def self.puts(*arg0); end

  sig.returns(T.noreturn)
  sig(
      arg0: String,
  )
  .returns(T.noreturn)
  sig(
      arg0: Class,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  sig(
      arg0: Exception,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  def self.raise(arg0=_, arg1=_, arg2=_); end

  sig(
      max: T.any(Integer, Range[Integer]),
  )
  .returns(Numeric)
  def self.rand(max); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(String)
  def self.readline(arg0=_, arg1=_); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(T::Array[String])
  def self.readlines(arg0=_, arg1=_); end

  sig(
      name: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.require(name); end

  sig(
      name: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def self.require_relative(name); end

  sig(
      read: T::Array[IO],
      write: T::Array[IO],
      error: T::Array[IO],
      timeout: Integer,
  )
  .returns(T::Array[String])
  def self.select(read, write=_, error=_, timeout=_); end

  sig(
      duration: Numeric,
  )
  .returns(Integer)
  def self.sleep(duration); end

  sig(
      number: Numeric,
  )
  .returns(Numeric)
  def self.srand(number); end

  sig(
      num: Integer,
      args: BasicObject,
  )
  .returns(T.untyped)
  def self.syscall(num, *args); end

  sig(
      cmd: String,
      file1: String,
      file2: String,
  )
  .returns(T.any(TrueClass, FalseClass, Time))
  def self.test(cmd, file1, file2=_); end

  sig(
      msg: String,
  )
  .returns(NilClass)
  def self.warn(*msg); end

  sig.returns(Kernel)
  def clone(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def nil?(); end

  sig(
      arg0: T.any(String, Symbol),
      arg1: BasicObject,
  )
  .returns(T.untyped)
  sig(
      arg0: T.any(String, Symbol),
      arg1: BasicObject,
      blk: BasicObject,
  )
  .returns(T.untyped)
  def send(arg0, *arg1, &blk); end

  sig.returns(String)
  def to_s(); end

  type_parameters(:T).sig(
      x: Object,
  )
  .returns(T::Array[T.type_parameter(:T)])
  def Array(x); end

  sig(
      x: Numeric,
      y: Numeric,
  )
  .returns(Complex)
  sig(
      x: String,
  )
  .returns(Complex)
  def Complex(x, y=_); end

  sig(
      x: Numeric,
  )
  .returns(Float)
  def Float(x); end

  type_parameters(:K ,:V).sig(
      x: Object,
  )
  .returns(T::Hash[T.type_parameter(:K), T.type_parameter(:V)])
  def Hash(x); end

  sig(
      arg: T.any(Numeric, String),
      base: Integer,
  )
  .returns(Integer)
  def Integer(arg, base=_); end

  sig(
      x: Numeric,
      y: Numeric,
  )
  .returns(Rational)
  sig(
      x: Object,
  )
  .returns(Rational)
  def Rational(x, y=_); end

  sig(
      x: Object,
  )
  .returns(String)
  def String(x); end

  sig.returns(T.nilable(Symbol))
  def __callee__(); end

  sig.returns(T.nilable(String))
  def __dir__(); end

  sig.returns(T.nilable(Symbol))
  def __method__(); end

  sig(
      arg0: String,
  )
  .returns(String)
  def `(arg0); end

  sig(
      msg: String,
  )
  .returns(T.noreturn)
  def abort(msg=_); end

  sig(
      blk: T.proc().returns(BasicObject),
  )
  .returns(Proc)
  def at_exit(&blk); end

  sig(
      _module: T.any(String, Symbol),
      filename: String,
  )
  .returns(NilClass)
  def autoload(_module, filename); end

  sig(
      name: T.any(Symbol, String),
  )
  .returns(T.nilable(String))
  def autoload?(name); end

  sig.returns(Binding)
  def binding(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def block_given?(); end

  sig.returns(T.noreturn)
  sig(
      status: T.any(Integer, TrueClass, FalseClass),
  )
  .returns(T.noreturn)
  def exit(status=_); end

  sig(
      status: T.any(Integer, TrueClass, FalseClass),
  )
  .returns(T.noreturn)
  def exit!(status); end

  sig.returns(T.noreturn)
  sig(
      arg0: String,
  )
  .returns(T.noreturn)
  sig(
      arg0: Class,
      arg1: T::Array[String],
  )
  .returns(T.noreturn)
  sig(
      arg0: Class,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  def fail(arg0=_, arg1=_, arg2=_); end

  sig(
      format: String,
      args: BasicObject,
  )
  .returns(String)
  def format(format, *args); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(String)
  def gets(arg0=_, arg1=_); end

  sig.returns(T::Array[Symbol])
  def global_variables(); end

  sig(
      filename: String,
      arg0: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def load(filename, arg0=_); end

  sig(
      name: String,
      rest: T.any(String, Integer),
      block: String,
  )
  .returns(T.nilable(IO))
  def open(name, rest=_, block=_); end

  sig(
      arg0: IO,
      arg1: String,
      arg2: BasicObject,
  )
  .returns(NilClass)
  def printf(arg0=_, arg1=_, *arg2); end

  sig(
      blk: BasicObject,
  )
  .returns(Proc)
  def proc(&blk); end

  sig(
      blk: BasicObject,
  )
  .returns(Proc)
  def lambda(&blk); end

  sig(
      blk: BasicObject,
  )
  .returns(Proc)
  def self.lambda(&blk); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def putc(arg0); end

  sig(
      arg0: BasicObject,
  )
  .returns(NilClass)
  def puts(*arg0); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(String)
  def readline(arg0=_, arg1=_); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(T::Array[String])
  def readlines(arg0=_, arg1=_); end

  sig(
      path: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def require(path); end

  sig(
      feature: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def require_relative(feature); end

  sig(
      read: T::Array[IO],
      write: T::Array[IO],
      error: T::Array[IO],
      timeout: Integer,
  )
  .returns(T::Array[String])
  def select(read, write=_, error=_, timeout=_); end

  sig(
      duration: Numeric,
  )
  .returns(Integer)
  def sleep(duration); end

  sig(
      format: String,
      args: BasicObject,
  )
  .returns(String)
  def self.sprintf(format, *args); end

  sig(
      format: String,
      args: BasicObject,
  )
  .returns(String)
  def sprintf(format, *args); end

  sig(
      num: Integer,
      args: BasicObject,
  )
  .returns(T.untyped)
  def syscall(num, *args); end

  sig(
      cmd: String,
      file1: String,
      file2: String,
  )
  .returns(T.any(TrueClass, FalseClass, Time))
  def test(cmd, file1, file2=_); end

  sig(
      msg: String,
  )
  .returns(NilClass)
  def warn(*msg); end

  sig.returns(T.noreturn)
  sig(
      arg0: String,
  )
  .returns(T.noreturn)
  sig(
      arg0: Class,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  sig(
      arg0: Exception,
      arg1: String,
      arg2: T::Array[String],
  )
  .returns(T.noreturn)
  def raise(arg0=_, arg1=_, arg2=_); end
end

class KeyError
  sig.returns(KeyError)
  def clone(); end

  sig.returns(KeyError)
  def dup(); end

  sig.returns(KeyError)
  def freeze(); end

  sig.returns(KeyError)
  def taint(); end

  sig.returns(KeyError)
  def trust(); end

  sig.returns(KeyError)
  def untaint(); end

  sig.returns(KeyError)
  def untrust(); end
end

class LoadError
  sig.returns(LoadError)
  def clone(); end

  sig.returns(LoadError)
  def dup(); end

  sig.returns(LoadError)
  def freeze(); end

  sig.returns(LoadError)
  def taint(); end

  sig.returns(LoadError)
  def trust(); end

  sig.returns(LoadError)
  def untaint(); end

  sig.returns(LoadError)
  def untrust(); end
end

class LocalJumpError
  sig.returns(LocalJumpError)
  def clone(); end

  sig.returns(LocalJumpError)
  def dup(); end

  sig.returns(LocalJumpError)
  def freeze(); end

  sig.returns(LocalJumpError)
  def taint(); end

  sig.returns(LocalJumpError)
  def trust(); end

  sig.returns(LocalJumpError)
  def untaint(); end

  sig.returns(LocalJumpError)
  def untrust(); end
end

module Marshal
  sig(
      arg0: Object,
      arg1: IO,
      arg2: Integer,
  )
  .returns(Object)
  sig(
      arg0: Object,
      arg1: Integer,
  )
  .returns(Object)
  def self.dump(arg0, arg1=_, arg2=_); end

  sig(
      arg0: String,
      arg1: Proc,
  )
  .returns(Object)
  def self.load(arg0, arg1=_); end
end

class MatchData
  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig(
      i_or_start_or_range_or_name: Integer,
  )
  .returns(T.nilable(String))
  sig(
      i_or_start_or_range_or_name: Integer,
      length: Integer,
  )
  .returns(T::Array[String])
  sig(
      i_or_start_or_range_or_name: Range[Integer],
  )
  .returns(T::Array[String])
  sig(
      i_or_start_or_range_or_name: T.any(String, Symbol),
  )
  .returns(T.nilable(String))
  def [](i_or_start_or_range_or_name, length=_); end

  sig(
      n: Integer,
  )
  .returns(Integer)
  def begin(n); end

  sig.returns(T::Array[String])
  def captures(); end

  sig.returns(MatchData)
  def clone(); end

  sig.returns(MatchData)
  def dup(); end

  sig(
      n: Integer,
  )
  .returns(Integer)
  def end(n); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(other); end

  sig.returns(MatchData)
  def freeze(); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Integer)
  def length(); end

  sig.returns(T::Array[String])
  def names(); end

  sig(
      n: Integer,
  )
  .returns(T::Array[Integer])
  def offset(n); end

  sig.returns(String)
  def post_match(); end

  sig.returns(String)
  def pre_match(); end

  sig.returns(Regexp)
  def regexp(); end

  sig.returns(Integer)
  def size(); end

  sig.returns(String)
  def string(); end

  sig.returns(MatchData)
  def taint(); end

  sig.returns(T::Array[String])
  def to_a(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(MatchData)
  def trust(); end

  sig.returns(MatchData)
  def untaint(); end

  sig.returns(MatchData)
  def untrust(); end

  sig(
      indexes: Integer,
  )
  .returns(T::Array[String])
  def values_at(*indexes); end
end

module Math
  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.acos(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.acosh(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.asin(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.asinh(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.atan(x); end

  sig(
      y: T.any(Integer, Float, Rational, BigDecimal),
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.atan2(y, x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.atanh(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.cbrt(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.cos(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.cosh(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.erf(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.erfc(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.exp(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def self.frexp(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.gamma(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
      y: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.hypot(x, y); end

  sig(
      fraction: T.any(Integer, Float, Rational, BigDecimal),
      exponent: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.ldexp(fraction, exponent); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(T.any(Integer, Float))
  def self.lgamma(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
      base: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.log(x, base=_); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.log10(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.log2(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.sin(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.sinh(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.sqrt(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.tan(x); end

  sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.tanh(x); end
end

class Method
  sig.returns(Method)
  def clone(); end

  sig.returns(Method)
  def dup(); end

  sig.returns(Method)
  def freeze(); end

  sig.returns(Method)
  def taint(); end

  sig.returns(Method)
  def trust(); end

  sig.returns(Method)
  def untaint(); end

  sig.returns(Method)
  def untrust(); end
end

class Module
  sig.returns(T::Array[Integer])
  def self.constants(); end

  sig.returns(T::Array[Module])
  def self.nesting(); end

  sig(
      other: Module,
  )
  .returns(T.nilable(T.any(TrueClass, FalseClass)))
  def <(other); end

  sig(
      other: Module,
  )
  .returns(T.nilable(T.any(TrueClass, FalseClass)))
  def <=(other); end

  sig(
      other: Module,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(other); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(other); end

  sig(
      other: Module,
  )
  .returns(T.nilable(T.any(TrueClass, FalseClass)))
  def >(other); end

  sig(
      other: Module,
  )
  .returns(T.nilable(T.any(TrueClass, FalseClass)))
  def >=(other); end

  sig(
      new_name: Symbol,
      old_name: Symbol,
  )
  .returns(Module)
  def alias_method(new_name, old_name); end

  sig.returns(T::Array[Module])
  def ancestors(); end

  sig(
      arg0: Module,
  )
  .returns(Module)
  def append_features(arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(NilClass)
  def attr_accessor(*arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(NilClass)
  def attr_reader(*arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(NilClass)
  def attr_writer(*arg0); end

  sig(
      _module: Symbol,
      filename: String,
  )
  .returns(NilClass)
  def autoload(_module, filename); end

  sig(
      name: Symbol,
  )
  .returns(T.nilable(String))
  def autoload?(name); end

  sig(
      arg0: String,
      filename: String,
      lineno: Integer,
  )
  .returns(T.untyped)
  def class_eval(arg0, filename=_, lineno=_); end

  sig(
      args: BasicObject,
      blk: BasicObject,
  )
  .returns(T.untyped)
  def class_exec(*args, &blk); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.any(TrueClass, FalseClass))
  def class_variable_defined?(arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.untyped)
  def class_variable_get(arg0); end

  sig(
      arg0: T.any(Symbol, String),
      arg1: BasicObject,
  )
  .returns(T.untyped)
  def class_variable_set(arg0, arg1); end

  sig(
      inherit: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def class_variables(inherit=_); end

  sig.returns(Module)
  def clone(); end

  sig(
      arg0: T.any(Symbol, String),
      inherit: T.any(TrueClass, FalseClass),
  )
  .returns(T.any(TrueClass, FalseClass))
  def const_defined?(arg0, inherit=_); end

  sig(
      arg0: T.any(Symbol, String),
      inherit: T.any(TrueClass, FalseClass),
  )
  .returns(T.untyped)
  def const_get(arg0, inherit=_); end

  sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  def const_missing(arg0); end

  sig(
      arg0: T.any(Symbol, String),
      arg1: BasicObject,
  )
  .returns(T.untyped)
  def const_set(arg0, arg1); end

  sig(
      inherit: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def constants(inherit=_); end

  sig(
      arg0: Symbol,
      arg1: Method,
  )
  .returns(Symbol)
  sig(
      arg0: Symbol,
      blk: BasicObject,
  )
  .returns(Symbol)
  def define_method(arg0, arg1=_, &blk); end

  sig.returns(Module)
  def dup(); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(other); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(other); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.untyped)
  def extend_object(arg0); end

  sig(
      othermod: Module,
  )
  .returns(T.untyped)
  def extended(othermod); end

  sig.returns(Module)
  def freeze(); end

  sig(
      arg0: Module,
  )
  .returns(Module)
  def include(*arg0); end

  sig(
      arg0: Module,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(arg0); end

  sig(
      othermod: Module,
  )
  .returns(T.untyped)
  def included(othermod); end

  sig.returns(T::Array[Module])
  def included_modules(); end

  sig.returns(Object)
  sig(
      blk: T.proc(arg0: Module).returns(BasicObject),
  )
  .returns(Object)
  def initialize(&blk); end

  sig(
      arg0: Symbol,
  )
  .returns(UnboundMethod)
  def instance_method(arg0); end

  sig(
      include_super: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def instance_methods(include_super=_); end

  sig(
      meth: Symbol,
  )
  .returns(T.untyped)
  def method_added(meth); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.any(TrueClass, FalseClass))
  def method_defined?(arg0); end

  sig(
      method_name: Symbol,
  )
  .returns(T.untyped)
  def method_removed(method_name); end

  sig(
      arg0: String,
      filename: String,
      lineno: Integer,
  )
  .returns(T.untyped)
  def module_eval(arg0, filename=_, lineno=_); end

  sig(
      args: BasicObject,
      blk: BasicObject,
  )
  .returns(T.untyped)
  def module_exec(*args, &blk); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(Module)
  def module_function(*arg0); end

  sig.returns(String)
  def name(); end

  sig(
      arg0: Module,
  )
  .returns(Module)
  def prepend(*arg0); end

  sig(
      arg0: Module,
  )
  .returns(Module)
  def prepend_features(arg0); end

  sig(
      othermod: Module,
  )
  .returns(T.untyped)
  def prepended(othermod); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(Module)
  def private(*arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(Module)
  def private_class_method(*arg0); end

  sig(
      arg0: Symbol,
  )
  .returns(Module)
  def private_constant(*arg0); end

  sig(
      include_super: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def private_instance_methods(include_super=_); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.any(TrueClass, FalseClass))
  def private_method_defined?(arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(Module)
  def protected(*arg0); end

  sig(
      include_super: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def protected_instance_methods(include_super=_); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.any(TrueClass, FalseClass))
  def protected_method_defined?(arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(Module)
  def public(*arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(Module)
  def public_class_method(*arg0); end

  sig(
      arg0: Symbol,
  )
  .returns(Module)
  def public_constant(*arg0); end

  sig(
      arg0: Symbol,
  )
  .returns(UnboundMethod)
  def public_instance_method(arg0); end

  sig(
      include_super: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def public_instance_methods(include_super=_); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.any(TrueClass, FalseClass))
  def public_method_defined?(arg0); end

  sig(
      arg0: Class,
      blk: T.proc(arg0: T.untyped).returns(BasicObject),
  )
  .returns(Module)
  def refine(arg0, &blk); end

  sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  def remove_class_variable(arg0); end

  sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  def remove_const(arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(Module)
  def remove_method(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def singleton_class?(); end

  sig.returns(Module)
  def taint(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Module)
  def trust(); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(Module)
  def undef_method(arg0); end

  sig.returns(Module)
  def untaint(); end

  sig.returns(Module)
  def untrust(); end

  sig(
      arg0: Module,
  )
  .returns(Module)
  def using(arg0); end

  sig.returns(String)
  def inspect(); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(NilClass)
  def attr(*arg0); end
end

class Monitor
  sig.returns(Monitor)
  def clone(); end

  sig.returns(Monitor)
  def dup(); end

  sig.returns(Monitor)
  def freeze(); end

  sig.returns(Monitor)
  def taint(); end

  sig.returns(Monitor)
  def trust(); end

  sig.returns(Monitor)
  def untaint(); end

  sig.returns(Monitor)
  def untrust(); end
end

class NameError
  sig.returns(NameError)
  def clone(); end

  sig.returns(NameError)
  def dup(); end

  sig.returns(NameError)
  def freeze(); end

  sig.returns(NameError)
  def taint(); end

  sig.returns(NameError)
  def trust(); end

  sig.returns(NameError)
  def untaint(); end

  sig.returns(NameError)
  def untrust(); end
end

class NilClass
  sig(
      obj: BasicObject,
  )
  .returns(FalseClass)
  def &(obj); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ^(obj); end

  sig.returns(NilClass)
  def clone(); end

  sig.returns(NilClass)
  def dup(); end

  sig.returns(NilClass)
  def freeze(); end

  sig.returns(Rational)
  def rationalize(); end

  sig.returns(NilClass)
  def taint(); end

  sig.returns([])
  def to_a(); end

  sig.returns(Complex)
  def to_c(); end

  sig.returns(Float)
  def to_f(); end

  sig.returns(T.untyped)
  def to_h(); end

  sig.returns(Rational)
  def to_r(); end

  sig.returns(NilClass)
  def trust(); end

  sig.returns(NilClass)
  def untaint(); end

  sig.returns(NilClass)
  def untrust(); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def |(obj); end
end

class NoMemoryError
  sig.returns(NoMemoryError)
  def clone(); end

  sig.returns(NoMemoryError)
  def dup(); end

  sig.returns(NoMemoryError)
  def freeze(); end

  sig.returns(NoMemoryError)
  def taint(); end

  sig.returns(NoMemoryError)
  def trust(); end

  sig.returns(NoMemoryError)
  def untaint(); end

  sig.returns(NoMemoryError)
  def untrust(); end
end

class NoMethodError
  sig.returns(NoMethodError)
  def clone(); end

  sig.returns(NoMethodError)
  def dup(); end

  sig.returns(NoMethodError)
  def freeze(); end

  sig.returns(NoMethodError)
  def taint(); end

  sig.returns(NoMethodError)
  def trust(); end

  sig.returns(NoMethodError)
  def untaint(); end

  sig.returns(NoMethodError)
  def untrust(); end
end

class NotImplementedError
  sig.returns(NotImplementedError)
  def clone(); end

  sig.returns(NotImplementedError)
  def dup(); end

  sig.returns(NotImplementedError)
  def freeze(); end

  sig.returns(NotImplementedError)
  def taint(); end

  sig.returns(NotImplementedError)
  def trust(); end

  sig.returns(NotImplementedError)
  def untaint(); end

  sig.returns(NotImplementedError)
  def untrust(); end
end

class Numeric
  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def %(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def +(arg0); end

  sig.returns(Numeric)
  def +@(); end

  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def -(arg0); end

  sig.returns(Numeric)
  def -@(); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(Integer)
  def <=>(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(arg0); end

  sig.returns(Numeric)
  def abs(); end

  sig.returns(Numeric)
  def abs2(); end

  sig.returns(Numeric)
  def angle(); end

  sig.returns(Numeric)
  def arg(); end

  sig.returns(Integer)
  def ceil(); end

  sig.returns(Numeric)
  def clone(); end

  sig(
      arg0: Numeric,
  )
  .returns([Numeric, Numeric])
  def coerce(arg0); end

  sig.returns(Numeric)
  def conj(); end

  sig.returns(Numeric)
  def conjugate(); end

  sig.returns(Integer)
  def denominator(); end

  sig(
      arg0: Numeric,
  )
  .returns(Integer)
  def div(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns([Numeric, Numeric])
  def divmod(arg0); end

  sig.returns(Numeric)
  def dup(); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def fdiv(arg0); end

  sig.returns(Integer)
  def floor(); end

  sig.returns(Numeric)
  def freeze(); end

  sig.returns(Complex)
  def i(); end

  sig.returns(Numeric)
  def imag(); end

  sig.returns(Numeric)
  def imaginary(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def integer?(); end

  sig.returns(Numeric)
  def magnitude(); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(Integer, Float, Rational, BigDecimal))
  def modulo(arg0); end

  type_parameters(:Self).sig.returns(T.nilable(T.type_parameter(:Self)))
  def nonzero?(); end

  sig.returns(Integer)
  def numerator(); end

  sig.returns(Numeric)
  def phase(); end

  sig.returns([Numeric, Numeric])
  def polar(); end

  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def quo(arg0); end

  sig.returns(Numeric)
  def real(); end

  sig.returns(Numeric)
  def real?(); end

  sig.returns([Numeric, Numeric])
  def rect(); end

  sig.returns([Numeric, Numeric])
  def rectangular(); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(Integer, Float, Rational, BigDecimal))
  def remainder(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def round(arg0); end

  sig(
      arg0: Symbol,
  )
  .returns(TypeError)
  def singleton_method_added(arg0); end

  sig(
      arg0: Numeric,
      blk: T.proc(arg0: Numeric).returns(BasicObject),
  )
  .returns(Numeric)
  sig(
      arg0: Numeric,
  )
  .returns(Enumerator[Numeric])
  sig(
      arg0: Numeric,
      arg1: Numeric,
      blk: T.proc(arg0: Numeric).returns(BasicObject),
  )
  .returns(Numeric)
  sig(
      arg0: Numeric,
      arg1: Numeric,
  )
  .returns(Enumerator[Numeric])
  def step(arg0, arg1=_, &blk); end

  sig.returns(Numeric)
  def taint(); end

  sig.returns(Complex)
  def to_c(); end

  sig.returns(Integer)
  def to_int(); end

  sig.returns(Integer)
  def truncate(); end

  sig.returns(Numeric)
  def trust(); end

  sig.returns(Numeric)
  def untaint(); end

  sig.returns(Numeric)
  def untrust(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end

class Object
  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def !~(other); end

  sig(
      other: BasicObject,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(other); end

  sig(
      other: BasicObject,
  )
  .returns(NilClass)
  def =~(other); end

  sig.returns(Object)
  def clone(); end

  sig(
      port: IO,
  )
  .returns(NilClass)
  def display(port); end

  sig.returns(Object)
  def dup(); end

  sig(
      method: Symbol,
      args: BasicObject,
  )
  .returns(Enumerator[T.untyped])
  sig(
      method: Symbol,
      args: BasicObject,
      blk: BasicObject,
  )
  .returns(Enumerator[T.untyped])
  def enum_for(method=_, *args, &blk); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(other); end

  sig(
      mod: Module,
  )
  .returns(NilClass)
  def extend(mod); end

  sig.returns(Object)
  def freeze(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def frozen?(); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(String)
  def inspect(); end

  sig(
      arg0: Class,
  )
  .returns(T.any(TrueClass, FalseClass))
  def instance_of?(arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.any(TrueClass, FalseClass))
  def instance_variable_defined?(arg0); end

  sig(
      arg0: T.any(Symbol, String),
  )
  .returns(T.untyped)
  def instance_variable_get(arg0); end

  sig(
      arg0: T.any(Symbol, String),
      arg1: BasicObject,
  )
  .returns(T.untyped)
  def instance_variable_set(arg0, arg1); end

  sig.returns(T::Array[Symbol])
  def instance_variables(); end

  sig(
      arg0: T.any(Class, Module),
  )
  .returns(T.any(TrueClass, FalseClass))
  def is_a?(arg0); end

  sig(
      arg0: Class,
  )
  .returns(T.any(TrueClass, FalseClass))
  def kind_of?(arg0); end

  sig(
      arg0: Symbol,
  )
  .returns(Method)
  def method(arg0); end

  sig(
      regular: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def methods(regular=_); end

  sig(
      all: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def private_methods(all=_); end

  sig(
      all: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def protected_methods(all=_); end

  sig(
      arg0: Symbol,
  )
  .returns(Method)
  def public_method(arg0); end

  sig(
      all: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def public_methods(all=_); end

  sig(
      arg0: T.any(Symbol, String),
      args: BasicObject,
  )
  .returns(T.untyped)
  def public_send(arg0, *args); end

  sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  def remove_instance_variable(arg0); end

  sig(
      arg0: T.any(Symbol, String),
      args: BasicObject,
  )
  .returns(T.untyped)
  def send(arg0, *args); end

  sig.returns(Class)
  def singleton_class(); end

  sig(
      arg0: Symbol,
  )
  .returns(Method)
  def singleton_method(arg0); end

  sig(
      all: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Symbol])
  def singleton_methods(all=_); end

  sig.returns(Object)
  def taint(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def tainted?(); end

  sig(
      method: Symbol,
      args: BasicObject,
  )
  .returns(Enumerator[T.untyped])
  sig(
      method: Symbol,
      args: BasicObject,
      blk: BasicObject,
  )
  .returns(Enumerator[T.untyped])
  def to_enum(method=_, *args, &blk); end

  sig.returns(Object)
  def trust(); end

  sig.returns(Object)
  def untaint(); end

  sig.returns(Object)
  def untrust(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def untrusted?(); end
end

class Pathname
  sig.returns(Pathname)
  def self.getwd(); end

  sig(
      p1: String,
      p2: String,
  )
  .returns(T::Array[Pathname])
  def self.glob(p1, p2=_); end

  sig(
      other: T.any(String, Pathname),
  )
  .returns(Pathname)
  def +(other); end

  sig(
      p1: BasicObject,
  )
  .returns(T.nilable(Integer))
  def <=>(p1); end

  sig(
      p1: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(p1); end

  sig(
      p1: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(p1); end

  sig.returns(T.any(TrueClass, FalseClass))
  def absolute?(); end

  sig(
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  def ascend(&blk); end

  sig.returns(Time)
  def atime(); end

  sig(
      p1: String,
  )
  .returns(Pathname)
  def basename(p1=_); end

  sig(
      length: Integer,
      offset: Integer,
  )
  .returns(String)
  def binread(length=_, offset=_); end

  sig(
      arg0: String,
      offset: Integer,
  )
  .returns(Integer)
  def binwrite(arg0, offset=_); end

  sig.returns(Time)
  def birthtime(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def blockdev?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def chardev?(); end

  sig(
      with_directory: T.any(TrueClass, FalseClass),
  )
  .returns(T::Array[Pathname])
  def children(with_directory); end

  sig(
      mode: Integer,
  )
  .returns(Integer)
  def chmod(mode); end

  sig(
      owner: Integer,
      group: Integer,
  )
  .returns(Integer)
  def chown(owner, group); end

  sig(
      consider_symlink: T.any(TrueClass, FalseClass),
  )
  .returns(T.untyped)
  def cleanpath(consider_symlink=_); end

  sig.returns(Time)
  def ctime(); end

  sig.returns(T.untyped)
  def delete(); end

  sig(
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  def descend(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def directory?(); end

  sig.returns(Pathname)
  def dirname(); end

  sig(
      with_directory: T.any(TrueClass, FalseClass),
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  def each_child(with_directory, &blk); end

  sig(
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  def each_entry(&blk); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.untyped)
  sig.returns(Enumerator[String])
  def each_filename(&blk); end

  sig(
      sep: String,
      limit: Integer,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.untyped)
  sig(
      sep: String,
      limit: Integer,
  )
  .returns(Enumerator[String])
  def each_line(sep=_, limit=_, &blk); end

  sig.returns(T::Array[Pathname])
  def entries(); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def executable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def executable_real?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def exist?(); end

  sig(
      p1: T.any(String, Pathname),
  )
  .returns(Pathname)
  def expand_path(p1=_); end

  sig.returns(String)
  def extname(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def file?(); end

  sig(
      ignore_error: T.any(TrueClass, FalseClass),
      blk: T.proc(arg0: Pathname).returns(BasicObject),
  )
  .returns(T.untyped)
  sig(
      ignore_error: T.any(TrueClass, FalseClass),
  )
  .returns(Enumerator[Pathname])
  def find(ignore_error, &blk); end

  sig(
      pattern: String,
      flags: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  def fnmatch(pattern, flags=_); end

  sig.returns(Pathname)
  def freeze(); end

  sig.returns(String)
  def ftype(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def grpowned?(); end

  sig(
      args: T.any(String, Pathname),
  )
  .returns(Pathname)
  def join(*args); end

  sig(
      mode: Integer,
  )
  .returns(Integer)
  def lchmod(mode); end

  sig(
      owner: Integer,
      group: Integer,
  )
  .returns(Integer)
  def lchown(owner, group); end

  sig.returns(File::Stat)
  def lstat(); end

  sig(
      old: String,
  )
  .returns(Integer)
  def make_link(old); end

  sig(
      p1: String,
  )
  .returns(Integer)
  def mkdir(p1); end

  sig.returns(T.untyped)
  def mkpath(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def mountpoint?(); end

  sig.returns(Time)
  def mtime(); end

  sig(
      mode: String,
      perm: String,
      opt: Integer,
  )
  .returns(File)
  type_parameters(:T).sig(
      mode: String,
      perm: String,
      opt: Integer,
      blk: T.proc(arg0: File).returns(T.type_parameter(:T)),
  )
  .returns(T.type_parameter(:T))
  def open(mode=_, perm=_, opt=_, &blk); end

  sig(
      arg0: Encoding,
  )
  .returns(Dir)
  type_parameters(:U).sig(
      arg0: Encoding,
      blk: T.proc(arg0: Dir).returns(T.type_parameter(:U)),
  )
  .returns(T.type_parameter(:U))
  def opendir(arg0=_, &blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def owned?(); end

  sig.returns(Pathname)
  def parent(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def pipe?(); end

  sig(
      length: Integer,
      offset: Integer,
      open_args: Integer,
  )
  .returns(String)
  def read(length=_, offset=_, open_args=_); end

  sig.returns(T.any(TrueClass, FalseClass))
  def readable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def readable_real?(); end

  sig(
      sep: String,
      limit: Integer,
      open_args: Integer,
  )
  .returns(T::Array[String])
  def readlines(sep=_, limit=_, open_args=_); end

  sig.returns(String)
  def readlink(); end

  sig(
      p1: String,
  )
  .returns(String)
  def realdirpath(p1=_); end

  sig(
      p1: String,
  )
  .returns(String)
  def realpath(p1=_); end

  sig.returns(T.any(TrueClass, FalseClass))
  def relative?(); end

  sig(
      base_directory: T.any(String, Pathname),
  )
  .returns(Pathname)
  def relative_path_from(base_directory); end

  sig(
      p1: String,
  )
  .returns(Integer)
  def rename(p1); end

  sig.returns(Integer)
  def rmdir(); end

  sig.returns(Integer)
  def rmtree(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def root?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def setgid?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def setuid?(); end

  sig.returns(Integer)
  def size(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def size?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def socket?(); end

  sig.returns([Pathname, Pathname])
  def split(); end

  sig.returns(File::Stat)
  def stat(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def sticky?(); end

  sig(
      args: String,
  )
  .returns(Pathname)
  def sub(*args); end

  sig(
      p1: String,
  )
  .returns(Pathname)
  def sub_ext(p1); end

  sig(
      old: String,
  )
  .returns(Integer)
  sig.returns(T.any(TrueClass, FalseClass))
  def symlink?(old=_); end

  sig(
      mode: Integer,
      perm: Integer,
  )
  .returns(Integer)
  def sysopen(mode=_, perm=_); end

  sig.returns(Pathname)
  def taint(); end

  sig.returns(String)
  def to_path(); end

  sig(
      length: Integer,
  )
  .returns(Integer)
  def truncate(length); end

  sig.returns(Integer)
  def unlink(); end

  sig.returns(Pathname)
  def untaint(); end

  sig(
      atime: Time,
      mtime: Time,
  )
  .returns(Integer)
  def utime(atime, mtime); end

  sig.returns(T.any(TrueClass, FalseClass))
  def world_readable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def world_writable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def writable?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def writable_real?(); end

  sig(
      arg0: String,
      offset: Integer,
      open_args: Integer,
  )
  .returns(Integer)
  def write(arg0, offset=_, open_args=_); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end

  sig.returns(Pathname)
  def self.pwd(); end

  sig(
      other: T.any(String, Pathname),
  )
  .returns(Pathname)
  def /(other); end

  sig.returns(String)
  def to_s(); end
end

class Proc
  sig.returns(Integer)
  def arity(); end

  sig.returns(Binding)
  def binding(); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.untyped)
  def call(*arg0); end

  sig.returns(Proc)
  def clone(); end

  sig(
      arity: Integer,
  )
  .returns(Proc)
  def curry(arity=_); end

  sig.returns(Proc)
  def dup(); end

  sig.returns(Proc)
  def freeze(); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def lambda(); end

  sig.returns(T::Array[[Symbol, Symbol]])
  def parameters(); end

  sig.returns([String, Integer])
  def source_location(); end

  sig.returns(Proc)
  def taint(); end

  sig.returns(Proc)
  def to_proc(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Proc)
  def trust(); end

  sig.returns(Proc)
  def untaint(); end

  sig.returns(Proc)
  def untrust(); end

  sig.returns(String)
  def inspect(); end
end

module Process
  sig(
      msg: String,
  )
  .returns(T.untyped)
  def self.abort(msg=_); end

  sig.returns(String)
  def self.argv0(); end

  sig(
      clock_id: T.any(Symbol, Integer),
      unit: Symbol,
  )
  .returns(T.any(Float, Integer))
  def self.clock_getres(clock_id, unit=_); end

  sig(
      clock_id: T.any(Symbol, Integer),
      unit: Symbol,
  )
  .returns(T.any(Float, Integer))
  def self.clock_gettime(clock_id, unit=_); end

  sig(
      nochdir: BasicObject,
      noclose: BasicObject,
  )
  .returns(Integer)
  def self.daemon(nochdir=_, noclose=_); end

  sig(
      pid: Integer,
  )
  .returns(Thread)
  def self.detach(pid); end

  sig.returns(Integer)
  def self.egid(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.egid=(arg0); end

  sig.returns(Integer)
  def self.euid(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.euid=(arg0); end

  sig(
      status: Integer,
  )
  .returns(T.untyped)
  def self.exit(status=_); end

  sig(
      status: Integer,
  )
  .returns(T.untyped)
  def self.exit!(status=_); end

  sig.returns(T.nilable(Integer))
  sig(
      blk: T.proc().returns(BasicObject),
  )
  .returns(T.nilable(Integer))
  def self.fork(&blk); end

  sig(
      pid: Integer,
  )
  .returns(Integer)
  def self.getpgid(pid); end

  sig.returns(Integer)
  def self.getpgrp(); end

  sig(
      kind: Integer,
      arg0: Integer,
  )
  .returns(Integer)
  def self.getpriority(kind, arg0); end

  sig(
      resource: T.any(Symbol, String, Integer),
  )
  .returns([Integer, Integer])
  def self.getrlimit(resource); end

  sig(
      pid: Integer,
  )
  .returns(Integer)
  def self.getsid(pid=_); end

  sig.returns(Integer)
  def self.gid(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.gid=(arg0); end

  sig.returns(T::Array[Integer])
  def self.groups(); end

  sig(
      arg0: T::Array[Integer],
  )
  .returns(T::Array[Integer])
  def self.groups=(arg0); end

  sig(
      username: String,
      gid: Integer,
  )
  .returns(T::Array[Integer])
  def self.initgroups(username, gid); end

  sig(
      signal: T.any(Integer, Symbol, String),
      pids: Integer,
  )
  .returns(Integer)
  def self.kill(signal, *pids); end

  sig.returns(Integer)
  def self.maxgroups(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def self.maxgroups=(arg0); end

  sig.returns(Integer)
  def self.pid(); end

  sig.returns(Integer)
  def self.ppid(); end

  sig(
      pid: Integer,
      arg0: Integer,
  )
  .returns(Integer)
  def self.setpgid(pid, arg0); end

  sig(
      kind: Integer,
      arg0: Integer,
      priority: Integer,
  )
  .returns(Integer)
  def self.setpriority(kind, arg0, priority); end

  sig(
      arg0: String,
  )
  .returns(String)
  def self.setproctitle(arg0); end

  sig(
      resource: T.any(Symbol, String, Integer),
      cur_limit: Integer,
      max_limit: Integer,
  )
  .returns(NilClass)
  def self.setrlimit(resource, cur_limit, max_limit=_); end

  sig.returns(Integer)
  def self.setsid(); end

  sig.returns(Process::Tms)
  def self.times(); end

  sig.returns(Integer)
  def self.uid(); end

  sig(
      user: Integer,
  )
  .returns(Integer)
  def self.uid=(user); end

  sig(
      pid: Integer,
      flags: Integer,
  )
  .returns(Integer)
  def self.wait(pid=_, flags=_); end

  sig(
      pid: Integer,
      flags: Integer,
  )
  .returns([Integer, Integer])
  def self.wait2(pid=_, flags=_); end

  sig.returns(T::Array[[Integer, Integer]])
  def self.waitall(); end

  sig(
      pid: Integer,
      flags: Integer,
  )
  .returns(Integer)
  def self.waitpid(pid=_, flags=_); end

  sig(
      pid: Integer,
      flags: Integer,
  )
  .returns([Integer, Integer])
  def self.waitpid2(pid=_, flags=_); end
end

module Process::GID
  sig(
      group: Integer,
  )
  .returns(Integer)
  def self.change_privilege(group); end

  sig.returns(Integer)
  def self.eid(); end

  sig(
      name: String,
  )
  .returns(Integer)
  def self.from_name(name); end

  sig(
      group: Integer,
  )
  .returns(Integer)
  def self.grant_privilege(group); end

  sig.returns(Integer)
  def self.re_exchange(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.re_exchangeable?(); end

  sig.returns(Integer)
  def self.rid(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.sid_available?(); end

  sig.returns(Integer)
  type_parameters(:T).sig(
      blk: T.proc().returns(T.type_parameter(:T)),
  )
  .returns(T.type_parameter(:T))
  def self.switch(&blk); end

  sig(
      group: Integer,
  )
  .returns(Integer)
  def self.eid=(group); end
end

class Process::Status
  sig(
      num: Integer,
  )
  .returns(Integer)
  def &(num); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(other); end

  sig(
      num: Integer,
  )
  .returns(Integer)
  def >>(num); end

  sig.returns(T.any(TrueClass, FalseClass))
  def coredump?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def exited?(); end

  sig.returns(T.nilable(Integer))
  def exitstatus(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Integer)
  def pid(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def signaled?(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def stopped?(); end

  sig.returns(T.nilable(Integer))
  def stopsig(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def success?(); end

  sig.returns(T.nilable(Integer))
  def termsig(); end

  sig.returns(Integer)
  def to_i(); end

  sig.returns(String)
  def to_s(); end
end

module Process::Sys
  sig.returns(Integer)
  def self.geteuid(); end

  sig.returns(Integer)
  def self.getgid(); end

  sig.returns(Integer)
  def self.getuid(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.issetugid(); end

  sig(
      group: Integer,
  )
  .returns(NilClass)
  def self.setegid(group); end

  sig(
      user: Integer,
  )
  .returns(NilClass)
  def self.seteuid(user); end

  sig(
      group: Integer,
  )
  .returns(NilClass)
  def self.setgid(group); end

  sig(
      rid: Integer,
      eid: Integer,
  )
  .returns(NilClass)
  def self.setregid(rid, eid); end

  sig(
      rid: Integer,
      eid: Integer,
      sid: Integer,
  )
  .returns(NilClass)
  def self.setresgid(rid, eid, sid); end

  sig(
      rid: Integer,
      eid: Integer,
      sid: Integer,
  )
  .returns(NilClass)
  def self.setresuid(rid, eid, sid); end

  sig(
      rid: Integer,
      eid: Integer,
  )
  .returns(NilClass)
  def self.setreuid(rid, eid); end

  sig(
      group: Integer,
  )
  .returns(NilClass)
  def self.setrgid(group); end

  sig(
      user: Integer,
  )
  .returns(NilClass)
  def self.setruid(user); end

  sig(
      user: Integer,
  )
  .returns(NilClass)
  def self.setuid(user); end
end

module Process::UID
  sig(
      user: Integer,
  )
  .returns(Integer)
  def self.change_privilege(user); end

  sig.returns(Integer)
  def self.eid(); end

  sig(
      name: String,
  )
  .returns(Integer)
  def self.from_name(name); end

  sig(
      user: Integer,
  )
  .returns(Integer)
  def self.grant_privilege(user); end

  sig.returns(Integer)
  def self.re_exchange(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.re_exchangeable?(); end

  sig.returns(Integer)
  def self.rid(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def self.sid_available?(); end

  sig.returns(Integer)
  type_parameters(:T).sig(
      blk: T.proc().returns(T.type_parameter(:T)),
  )
  .returns(T.type_parameter(:T))
  def self.switch(&blk); end

  sig(
      user: Integer,
  )
  .returns(Integer)
  def self.eid=(user); end
end

class Process::Waiter
  sig.returns(Integer)
  def pid(); end
end

class Random
  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig(
      size: Integer,
  )
  .returns(String)
  def bytes(size); end

  sig.returns(Random)
  def clone(); end

  sig.returns(Random)
  def dup(); end

  sig.returns(Random)
  def freeze(); end

  sig(
      seed: Integer,
  )
  .returns(Object)
  def initialize(seed=_); end

  sig(
      max: T.any(Integer, Range[Integer]),
  )
  .returns(Integer)
  sig(
      max: T.any(Float, Range[Float]),
  )
  .returns(Float)
  def rand(max=_); end

  sig.returns(Integer)
  def seed(); end

  sig.returns(Random)
  def taint(); end

  sig.returns(Random)
  def trust(); end

  sig.returns(Random)
  def untaint(); end

  sig.returns(Random)
  def untrust(); end

  sig.returns(Integer)
  def self.new_seed(); end

  sig(
      max: Integer,
  )
  .returns(Numeric)
  def self.rand(max=_); end

  sig(
      number: Integer,
  )
  .returns(Numeric)
  def self.srand(number=_); end
end

class Range
  extend T::Generic
  Elem = type_member

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(obj); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(obj); end

  sig.returns(Elem)
  def begin(); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.any(TrueClass, FalseClass)),
  )
  .returns(T.nilable(T.type_parameter(:U)))
  def bsearch(&blk); end

  sig.returns(Range[Elem])
  def clone(); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def cover?(obj); end

  sig.returns(Range[Elem])
  def dup(); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Range[Elem])
  sig.returns(Enumerator[Elem])
  def each(&blk); end

  sig.returns(Elem)
  def end(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def exclude_end?(); end

  sig.returns(Elem)
  sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  def first(n=_); end

  sig.returns(Range[Elem])
  def freeze(); end

  sig.returns(Integer)
  def hash(); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(obj); end

  sig(
      _begin: Integer,
      _end: Integer,
      exclude_end: T.any(TrueClass, FalseClass),
  )
  .returns(Object)
  def initialize(_begin, _end, exclude_end=_); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Elem)
  sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  def last(n=_); end

  sig.returns(Elem)
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(Elem)
  sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  sig(
      n: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def max(n=_, &blk); end

  sig.returns(Elem)
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(Elem)
  sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  sig(
      n: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def min(n=_, &blk); end

  sig.returns(T.nilable(Integer))
  def size(); end

  sig(
      n: Integer,
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Range[Elem])
  sig(
      n: Integer,
  )
  .returns(Enumerator[Elem])
  def step(n=_, &blk); end

  sig.returns(Range[Elem])
  def taint(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Range[Elem])
  def trust(); end

  sig.returns(Range[Elem])
  def untaint(); end

  sig.returns(Range[Elem])
  def untrust(); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(obj); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def member?(obj); end
end

class RangeError
  sig.returns(RangeError)
  def clone(); end

  sig.returns(RangeError)
  def dup(); end

  sig.returns(RangeError)
  def freeze(); end

  sig.returns(RangeError)
  def taint(); end

  sig.returns(RangeError)
  def trust(); end

  sig.returns(RangeError)
  def untaint(); end

  sig.returns(RangeError)
  def untrust(); end
end

class Rational
  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def %(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def *(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Numeric)
  sig(
      arg0: Float,
  )
  .returns(Numeric)
  sig(
      arg0: Rational,
  )
  .returns(Numeric)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def **(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def +(arg0); end

  sig.returns(Rational)
  def +@(); end

  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def -(arg0); end

  sig.returns(Rational)
  def -@(); end

  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def /(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Integer)
  sig(
      arg0: Rational,
  )
  .returns(Integer)
  sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def <=>(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(arg0); end

  sig.returns(Rational)
  def abs(); end

  sig.returns(Rational)
  def abs2(); end

  sig.returns(Numeric)
  def angle(); end

  sig.returns(Numeric)
  def arg(); end

  sig.returns(Integer)
  sig(
      arg0: Integer,
  )
  .returns(Numeric)
  def ceil(arg0=_); end

  sig.returns(Rational)
  def clone(); end

  sig(
      arg0: Integer,
  )
  .returns([Rational, Rational])
  sig(
      arg0: Float,
  )
  .returns([Float, Float])
  sig(
      arg0: Rational,
  )
  .returns([Rational, Rational])
  sig(
      arg0: Complex,
  )
  .returns([Numeric, Numeric])
  def coerce(arg0); end

  sig.returns(Rational)
  def conj(); end

  sig.returns(Rational)
  def conjugate(); end

  sig.returns(Integer)
  def denominator(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Integer)
  sig(
      arg0: Rational,
  )
  .returns(Integer)
  sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def div(arg0); end

  sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def divmod(arg0); end

  sig.returns(Rational)
  def dup(); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Float)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Float)
  sig(
      arg0: BigDecimal,
  )
  .returns(Float)
  sig(
      arg0: Complex,
  )
  .returns(Float)
  def fdiv(arg0); end

  sig.returns(Integer)
  sig(
      arg0: Integer,
  )
  .returns(Numeric)
  def floor(arg0=_); end

  sig.returns(Rational)
  def freeze(); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(Integer)
  def imag(); end

  sig.returns(Integer)
  def imaginary(); end

  sig.returns(String)
  def inspect(); end

  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def modulo(arg0); end

  sig.returns(Integer)
  def numerator(); end

  sig.returns(Numeric)
  def phase(); end

  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def quo(arg0); end

  sig.returns(Rational)
  sig(
      arg0: Numeric,
  )
  .returns(Rational)
  def rationalize(arg0=_); end

  sig.returns(Rational)
  def real(); end

  sig.returns(TrueClass)
  def real?(); end

  sig.returns(Integer)
  sig(
      arg0: Integer,
  )
  .returns(Numeric)
  def round(arg0=_); end

  sig.returns(Rational)
  def taint(); end

  sig.returns(Complex)
  def to_c(); end

  sig.returns(Float)
  def to_f(); end

  sig.returns(Integer)
  def to_i(); end

  sig.returns(Rational)
  def to_r(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Integer)
  sig(
      arg0: Integer,
  )
  .returns(Rational)
  def truncate(arg0=_); end

  sig.returns(Rational)
  def trust(); end

  sig.returns(Rational)
  def untaint(); end

  sig.returns(Rational)
  def untrust(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end

class Regexp
  sig(
      arg0: T.any(String, Symbol),
  )
  .returns(String)
  def self.escape(arg0); end

  sig.returns(MatchData)
  sig(
      arg0: Integer,
  )
  .returns(String)
  def self.last_match(arg0=_); end

  sig(
      obj: BasicObject,
  )
  .returns(T.nilable(Regexp))
  def self.try_convert(obj); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(other); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(other); end

  sig(
      str: String,
  )
  .returns(T.nilable(Integer))
  def =~(str); end

  sig.returns(T.any(TrueClass, FalseClass))
  def casefold?(); end

  sig.returns(Regexp)
  def clone(); end

  sig.returns(Regexp)
  def dup(); end

  sig.returns(Encoding)
  def encoding(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def fixed_encoding?(); end

  sig.returns(Regexp)
  def freeze(); end

  sig.returns(Integer)
  def hash(); end

  sig(
      arg0: String,
      options: BasicObject,
      kcode: String,
  )
  .returns(Object)
  sig(
      arg0: Regexp,
  )
  .returns(Object)
  def initialize(arg0, options=_, kcode=_); end

  sig.returns(String)
  def inspect(); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(T.nilable(MatchData))
  def match(arg0, arg1=_); end

  sig.returns(T::Hash[String, T::Array[Integer]])
  def named_captures(); end

  sig.returns(T::Array[String])
  def names(); end

  sig.returns(Integer)
  def options(); end

  sig.returns(String)
  def source(); end

  sig.returns(Regexp)
  def taint(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Regexp)
  def trust(); end

  sig.returns(Regexp)
  def untaint(); end

  sig.returns(Regexp)
  def untrust(); end

  sig.returns(T.nilable(Integer))
  def ~(); end

  sig(
      arg0: String,
      options: BasicObject,
      kcode: String,
  )
  .returns(Regexp)
  sig(
      arg0: Regexp,
  )
  .returns(Regexp)
  def self.compile(arg0, options=_, kcode=_); end

  sig(
      arg0: T.any(String, Symbol),
  )
  .returns(String)
  def self.quote(arg0); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(other); end
end

class RegexpError
  sig.returns(RegexpError)
  def clone(); end

  sig.returns(RegexpError)
  def dup(); end

  sig.returns(RegexpError)
  def freeze(); end

  sig.returns(RegexpError)
  def taint(); end

  sig.returns(RegexpError)
  def trust(); end

  sig.returns(RegexpError)
  def untaint(); end

  sig.returns(RegexpError)
  def untrust(); end
end

class RubyVM
  sig.returns(RubyVM)
  def clone(); end

  sig.returns(RubyVM)
  def dup(); end

  sig.returns(RubyVM)
  def freeze(); end

  sig.returns(RubyVM)
  def taint(); end

  sig.returns(RubyVM)
  def trust(); end

  sig.returns(RubyVM)
  def untaint(); end

  sig.returns(RubyVM)
  def untrust(); end
end

class RuntimeError
  sig.returns(RuntimeError)
  def clone(); end

  sig.returns(RuntimeError)
  def dup(); end

  sig.returns(RuntimeError)
  def freeze(); end

  sig.returns(RuntimeError)
  def taint(); end

  sig.returns(RuntimeError)
  def trust(); end

  sig.returns(RuntimeError)
  def untaint(); end

  sig.returns(RuntimeError)
  def untrust(); end
end

class ScriptError
  sig.returns(ScriptError)
  def clone(); end

  sig.returns(ScriptError)
  def dup(); end

  sig.returns(ScriptError)
  def freeze(); end

  sig.returns(ScriptError)
  def taint(); end

  sig.returns(ScriptError)
  def trust(); end

  sig.returns(ScriptError)
  def untaint(); end

  sig.returns(ScriptError)
  def untrust(); end
end

class SecurityError
  sig.returns(SecurityError)
  def clone(); end

  sig.returns(SecurityError)
  def dup(); end

  sig.returns(SecurityError)
  def freeze(); end

  sig.returns(SecurityError)
  def taint(); end

  sig.returns(SecurityError)
  def trust(); end

  sig.returns(SecurityError)
  def untaint(); end

  sig.returns(SecurityError)
  def untrust(); end
end

class Set
  extend T::Generic
  Elem = type_member

  type_parameters(:U).sig(
      ary: T.type_parameter(:U),
  )
  .returns(Set[T.type_parameter(:U)])
  def self.[](*ary); end

  sig(
      enum: Enumerable[Elem],
  )
  .returns(Set[Elem])
  def +(enum); end

  sig(
      enum: Enumerable[Elem],
  )
  .returns(Set[Elem])
  def ^(enum); end

  sig(
      o: Elem,
  )
  .returns(Set[Elem])
  def add(o); end

  type_parameters(:Self).sig(
      o: Elem,
  )
  .returns(T.nilable(T.type_parameter(:Self)))
  def add?(o); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: T.type_parameter(:U)).returns(Elem),
  )
  .returns(T::Hash[T.type_parameter(:U), Set[Elem]])
  def classify(&blk); end

  sig.returns(Set[Elem])
  def clear(); end

  sig.returns(Set[Elem])
  def clone(); end

  sig(
      o: Elem,
  )
  .returns(Set[Elem])
  def delete(o); end

  type_parameters(:Self).sig(
      o: Elem,
  )
  .returns(T.nilable(T.type_parameter(:Self)))
  def delete?(o); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Set[Elem])
  def delete_if(&blk); end

  sig(
      enum: Enumerable[Elem],
  )
  .returns(Set[Elem])
  def difference(enum); end

  sig(
      set: Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def disjoint?(set); end

  sig.returns(Set[Elem])
  def dup(); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Set[Elem])
  sig.returns(Enumerator[Elem])
  def each(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def empty?(); end

  sig.returns(Set)
  def flatten(); end

  type_parameters(:Self).sig.returns(T.nilable(T.type_parameter(:Self)))
  def flatten!(); end

  sig.returns(Set[Elem])
  def freeze(); end

  type_parameters(:U).sig(
      enum: Enumerable[T.type_parameter(:U)],
  )
  .returns(Object)
  def initialize(enum=_); end

  sig(
      set: Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def intersect?(set); end

  sig(
      enum: Enumerable[Elem],
  )
  .returns(Set[Elem])
  def intersection(enum); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Set[Elem])
  def keep_if(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(Set[T.type_parameter(:U)])
  def map!(&blk); end

  sig(
      o: Elem,
  )
  .returns(T.any(TrueClass, FalseClass))
  def member?(o); end

  sig(
      enum: Enumerable[Elem],
  )
  .returns(Set[Elem])
  def merge(enum); end

  sig(
      set: Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def proper_subset?(set); end

  sig(
      set: Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def proper_superset?(set); end

  type_parameters(:Self).sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(T.type_parameter(:Self)))
  def reject!(&blk); end

  type_parameters(:U).sig(
      enum: Enumerable[T.type_parameter(:U)],
  )
  .returns(Set[T.type_parameter(:U)])
  def replace(enum); end

  type_parameters(:Self).sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(T.type_parameter(:Self)))
  def select!(&blk); end

  sig.returns(Integer)
  def size(); end

  sig(
      set: Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def subset?(set); end

  sig(
      enum: Enumerable[Elem],
  )
  .returns(Set[Elem])
  def subtract(enum); end

  sig(
      set: Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def superset?(set); end

  sig.returns(Set[Elem])
  def taint(); end

  sig.returns(T::Array[Elem])
  def to_a(); end

  sig.returns(Set[Elem])
  def trust(); end

  sig.returns(Set[Elem])
  def untaint(); end

  sig.returns(Set[Elem])
  def untrust(); end

  sig(
      enum: Enumerable[Elem],
  )
  .returns(Set[Elem])
  def &(enum); end

  sig(
      enum: Enumerable[Elem],
  )
  .returns(Set[Elem])
  def -(enum); end

  sig(
      set: Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(set); end

  sig(
      o: Elem,
  )
  .returns(Set[Elem])
  def <<(o); end

  sig(
      set: Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(set); end

  sig(
      set: Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(set); end

  sig(
      set: Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(set); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(Set[T.type_parameter(:U)])
  def collect!(&blk); end

  sig(
      o: Elem,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(o); end

  sig.returns(Integer)
  def length(); end

  sig(
      enum: Enumerable[Elem],
  )
  .returns(Set[Elem])
  def |(enum); end

  sig(
      enum: Enumerable[Elem],
  )
  .returns(Set[Elem])
  def union(enum); end
end

module Signal
  sig.returns(T::Hash[String, Integer])
  def self.list(); end

  sig(
      arg0: Integer,
  )
  .returns(T.nilable(String))
  def self.signame(arg0); end

  sig(
      arg0: T.any(Integer, String),
  )
  .returns(T.any(String, Proc))
  sig(
      arg0: T.any(Integer, String),
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(T.any(String, Proc))
  def self.trap(arg0, &blk); end
end

class SignalException
  sig.returns(SignalException)
  def clone(); end

  sig.returns(SignalException)
  def dup(); end

  sig.returns(SignalException)
  def freeze(); end

  sig.returns(SignalException)
  def taint(); end

  sig.returns(SignalException)
  def trust(); end

  sig.returns(SignalException)
  def untaint(); end

  sig.returns(SignalException)
  def untrust(); end
end

class SortedSet
  extend T::Generic
  Elem = type_member

  sig(
      o: Elem,
  )
  .returns(SortedSet[Elem])
  def add(o); end

  sig.returns(SortedSet[Elem])
  def clear(); end

  sig.returns(SortedSet[Elem])
  def clone(); end

  sig(
      o: Elem,
  )
  .returns(SortedSet[Elem])
  def delete(o); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(SortedSet[Elem])
  def delete_if(&blk); end

  sig.returns(SortedSet[Elem])
  def dup(); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(SortedSet[Elem])
  def each(&blk); end

  sig.returns(SortedSet[Elem])
  def freeze(); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(SortedSet[Elem])
  def keep_if(&blk); end

  sig(
      enum: Enumerable[Elem],
  )
  .returns(SortedSet[Elem])
  def merge(enum); end

  sig(
      enum: Enumerable[Elem],
  )
  .returns(SortedSet[Elem])
  def subtract(enum); end

  sig.returns(SortedSet[Elem])
  def taint(); end

  sig.returns(SortedSet[Elem])
  def trust(); end

  sig.returns(SortedSet[Elem])
  def untaint(); end

  sig.returns(SortedSet[Elem])
  def untrust(); end
end

class StandardError
  sig.returns(StandardError)
  def clone(); end

  sig.returns(StandardError)
  def dup(); end

  sig.returns(StandardError)
  def freeze(); end

  sig.returns(StandardError)
  def taint(); end

  sig.returns(StandardError)
  def trust(); end

  sig.returns(StandardError)
  def untaint(); end

  sig.returns(StandardError)
  def untrust(); end
end

class StopIteration
  sig.returns(StopIteration)
  def clone(); end

  sig.returns(StopIteration)
  def dup(); end

  sig.returns(StopIteration)
  def freeze(); end

  sig.returns(StopIteration)
  def taint(); end

  sig.returns(StopIteration)
  def trust(); end

  sig.returns(StopIteration)
  def untaint(); end

  sig.returns(StopIteration)
  def untrust(); end
end

class String
  sig(
      arg0: Object,
  )
  .returns(String)
  def %(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(String)
  def *(arg0); end

  sig(
      arg0: String,
  )
  .returns(String)
  def +(arg0); end

  sig(
      arg0: Object,
  )
  .returns(String)
  def <<(arg0); end

  sig(
      other: String,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.nilable(Integer))
  def =~(arg0); end

  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: T.any(Range[Integer], Regexp),
  )
  .returns(T.nilable(String))
  sig(
      arg0: Regexp,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: Regexp,
      arg1: String,
  )
  .returns(T.nilable(String))
  sig(
      arg0: String,
  )
  .returns(T.nilable(String))
  def [](arg0, arg1=_); end

  sig.returns(T.any(TrueClass, FalseClass))
  def ascii_only?(); end

  sig.returns(String)
  def b(); end

  sig.returns(Array)
  def bytes(); end

  sig.returns(Integer)
  def bytesize(); end

  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: Range[Integer],
  )
  .returns(T.nilable(String))
  def byteslice(arg0, arg1=_); end

  sig.returns(String)
  def capitalize(); end

  sig.returns(T.nilable(String))
  def capitalize!(); end

  sig(
      arg0: String,
  )
  .returns(T.nilable(Integer))
  def casecmp(arg0); end

  sig(
      arg0: Integer,
      arg1: String,
  )
  .returns(String)
  def center(arg0, arg1=_); end

  sig.returns(Array)
  def chars(); end

  sig(
      arg0: String,
  )
  .returns(String)
  def chomp(arg0=_); end

  sig(
      arg0: String,
  )
  .returns(T.nilable(String))
  def chomp!(arg0=_); end

  sig.returns(String)
  def chop(); end

  sig.returns(T.nilable(String))
  def chop!(); end

  sig.returns(String)
  def chr(); end

  sig.returns(String)
  def clear(); end

  sig.returns(String)
  def clone(); end

  sig.returns(T::Array[Integer])
  sig(
      blk: BasicObject,
  )
  .returns(T::Array[Integer])
  def codepoints(&blk); end

  sig(
      arg0: T.any(Integer, Object),
  )
  .returns(String)
  def concat(arg0); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(Integer)
  def count(arg0, *arg1); end

  sig(
      arg0: String,
  )
  .returns(String)
  def crypt(arg0); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(String)
  def delete(arg0, *arg1); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(T.nilable(String))
  def delete!(arg0, *arg1); end

  sig.returns(String)
  def downcase(); end

  sig.returns(T.nilable(String))
  def downcase!(); end

  sig.returns(String)
  def dump(); end

  sig.returns(String)
  def dup(); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(String)
  sig.returns(Enumerator[Integer])
  def each_byte(&blk); end

  sig(
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  sig.returns(Enumerator[String])
  def each_char(&blk); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(String)
  sig.returns(Enumerator[Integer])
  def each_codepoint(&blk); end

  sig(
      arg0: String,
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  sig(
      arg0: String,
  )
  .returns(Enumerator[String])
  def each_line(arg0=_, &blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def empty?(); end

  sig.returns(Encoding)
  def encoding(); end

  sig(
      arg0: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def end_with?(*arg0); end

  sig(
      arg0: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  sig(
      arg0: T.any(String, Encoding),
  )
  .returns(String)
  def force_encoding(arg0); end

  sig.returns(String)
  def freeze(); end

  sig(
      arg0: Integer,
  )
  .returns(T.nilable(Integer))
  def getbyte(arg0); end

  sig(
      arg0: T.any(Regexp, String),
      arg1: String,
  )
  .returns(String)
  sig(
      arg0: T.any(Regexp, String),
      arg1: Hash,
  )
  .returns(String)
  sig(
      arg0: T.any(Regexp, String),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  sig(
      arg0: T.any(Regexp, String),
  )
  .returns(Enumerator[String])
  sig(
      arg0: T.any(Regexp, String),
  )
  .returns(String)
  def gsub(arg0, arg1=_, &blk); end

  sig(
      arg0: T.any(Regexp, String),
      arg1: String,
  )
  .returns(T.nilable(String))
  sig(
      arg0: T.any(Regexp, String),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(T.nilable(String))
  sig(
      arg0: T.any(Regexp, String),
  )
  .returns(Enumerator[String])
  def gsub!(arg0, arg1=_, &blk); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(Integer)
  def hex(); end

  sig(
      arg0: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(arg0); end

  sig(
      arg0: T.any(Regexp, String),
      arg1: Integer,
  )
  .returns(T.nilable(Integer))
  def index(arg0, arg1=_); end

  sig(
      str: String,
  )
  .returns(Object)
  def initialize(str=_); end

  sig(
      arg0: Integer,
      arg1: String,
  )
  .returns(String)
  def insert(arg0, arg1); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Symbol)
  def intern(); end

  sig.returns(Integer)
  def length(); end

  sig(
      arg0: String,
  )
  .returns(T::Array[String])
  def lines(arg0=_); end

  sig(
      arg0: Integer,
      arg1: String,
  )
  .returns(String)
  def ljust(arg0, arg1=_); end

  sig.returns(String)
  def lstrip(); end

  sig.returns(T.nilable(String))
  def lstrip!(); end

  sig(
      arg0: T.any(Regexp, String),
  )
  .returns(MatchData)
  sig(
      arg0: T.any(Regexp, String),
      arg1: Integer,
  )
  .returns(MatchData)
  def match(arg0, arg1=_); end

  sig.returns(String)
  def next(); end

  sig.returns(String)
  def next!(); end

  sig.returns(Integer)
  def oct(); end

  sig.returns(Integer)
  def ord(); end

  sig(
      arg0: T.any(Regexp, String),
  )
  .returns(T::Array[String])
  def partition(arg0); end

  sig(
      arg0: String,
  )
  .returns(String)
  def prepend(arg0); end

  sig(
      arg0: String,
  )
  .returns(String)
  def replace(arg0); end

  sig.returns(String)
  def reverse(); end

  sig(
      arg0: T.any(String, Regexp),
      arg1: Integer,
  )
  .returns(T.nilable(Integer))
  def rindex(arg0, arg1=_); end

  sig(
      arg0: Integer,
      arg1: String,
  )
  .returns(String)
  def rjust(arg0, arg1=_); end

  sig(
      arg0: T.any(String, Regexp),
  )
  .returns(T::Array[String])
  def rpartition(arg0); end

  sig.returns(String)
  def rstrip(); end

  sig.returns(String)
  def rstrip!(); end

  sig(
      arg0: T.any(Regexp, String),
  )
  .returns(T::Array[T.any(String, T::Array[String])])
  sig(
      arg0: T.any(Regexp, String),
      blk: BasicObject,
  )
  .returns(T::Array[T.any(String, T::Array[String])])
  def scan(arg0, &blk); end

  sig(
      arg0: String,
  )
  .returns(String)
  sig(
      arg0: String,
      blk: T.proc(arg0: T.untyped).returns(BasicObject),
  )
  .returns(String)
  def scrub(arg0=_, &blk); end

  sig(
      arg0: String,
  )
  .returns(String)
  sig(
      arg0: String,
      blk: T.proc(arg0: T.untyped).returns(BasicObject),
  )
  .returns(String)
  def scrub!(arg0=_, &blk); end

  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(Integer)
  def setbyte(arg0, arg1); end

  sig.returns(Integer)
  def size(); end

  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: T.any(Range[Integer], Regexp),
  )
  .returns(T.nilable(String))
  sig(
      arg0: Regexp,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: Regexp,
      arg1: String,
  )
  .returns(T.nilable(String))
  sig(
      arg0: String,
  )
  .returns(T.nilable(String))
  def slice!(arg0, arg1=_); end

  sig(
      arg0: T.any(Regexp, String),
      arg1: Integer,
  )
  .returns(T::Array[String])
  sig(
      arg0: Integer,
  )
  .returns(T::Array[String])
  def split(arg0=_, arg1=_); end

  sig(
      arg0: String,
  )
  .returns(String)
  def squeeze(arg0=_); end

  sig(
      arg0: String,
  )
  .returns(String)
  def squeeze!(arg0=_); end

  sig(
      arg0: String,
  )
  .returns(T.any(TrueClass, FalseClass))
  def start_with?(*arg0); end

  sig.returns(String)
  def strip(); end

  sig.returns(String)
  def strip!(); end

  sig(
      arg0: T.any(Regexp, String),
      arg1: T.any(String, Hash),
  )
  .returns(String)
  sig(
      arg0: T.any(Regexp, String),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  def sub(arg0, arg1=_, &blk); end

  sig(
      arg0: T.any(Regexp, String),
      arg1: String,
  )
  .returns(String)
  sig(
      arg0: T.any(Regexp, String),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  def sub!(arg0, arg1=_, &blk); end

  sig.returns(String)
  def succ(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def sum(arg0=_); end

  sig.returns(String)
  def swapcase(); end

  sig.returns(T.nilable(String))
  def swapcase!(); end

  sig.returns(String)
  def taint(); end

  sig.returns(Complex)
  def to_c(); end

  sig.returns(Float)
  def to_f(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def to_i(arg0=_); end

  sig.returns(Rational)
  def to_r(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(String)
  def to_str(); end

  sig.returns(Symbol)
  def to_sym(); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(String)
  def tr(arg0, arg1); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(T.nilable(String))
  def tr!(arg0, arg1); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(String)
  def tr_s(arg0, arg1); end

  sig(
      arg0: String,
      arg1: String,
  )
  .returns(T.nilable(String))
  def tr_s!(arg0, arg1); end

  sig.returns(String)
  def trust(); end

  sig(
      arg0: String,
  )
  .returns(T::Array[String])
  def unpack(arg0); end

  sig.returns(String)
  def untaint(); end

  sig.returns(String)
  def untrust(); end

  sig.returns(String)
  def upcase(); end

  sig.returns(T.nilable(String))
  def upcase!(); end

  type_parameters(:Bool).sig(
      arg0: String,
      arg1: T.type_parameter(:Bool),
  )
  .returns(Enumerator[String])
  type_parameters(:Bool).sig(
      arg0: String,
      arg1: T.type_parameter(:Bool),
      blk: T.proc(arg0: String).returns(BasicObject),
  )
  .returns(String)
  def upto(arg0, arg1=_, &blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def valid_encoding?(); end

  sig(
      obj: Object,
  )
  .returns(T.nilable(String))
  def self.try_convert(obj); end

  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: T.any(Range[Integer], Regexp),
  )
  .returns(T.nilable(String))
  sig(
      arg0: Regexp,
      arg1: Integer,
  )
  .returns(T.nilable(String))
  sig(
      arg0: Regexp,
      arg1: String,
  )
  .returns(T.nilable(String))
  sig(
      arg0: String,
  )
  .returns(T.nilable(String))
  def slice(arg0, arg1=_); end
end

class StringScanner
  sig(
      arg0: String,
      arg1: T.any(TrueClass, FalseClass),
  )
  .returns(StringScanner)
  def self.new(arg0, arg1=_); end

  sig.returns(T.any(TrueClass, FalseClass))
  def eos?(); end

  sig.returns(String)
  def getch(); end

  sig(
      arg0: Regexp,
  )
  .returns(String)
  def scan(arg0); end
end

class Symbol
  sig.returns(T::Array[Symbol])
  def self.all_symbols(); end

  sig(
      other: Symbol,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(obj); end

  sig(
      obj: BasicObject,
  )
  .returns(T.nilable(Integer))
  def =~(obj); end

  sig(
      idx_or_range: Integer,
  )
  .returns(String)
  sig(
      idx_or_range: Integer,
      n: Integer,
  )
  .returns(String)
  sig(
      idx_or_range: Range[Integer],
  )
  .returns(String)
  def [](idx_or_range, n=_); end

  sig.returns(Symbol)
  def capitalize(); end

  sig(
      other: Symbol,
  )
  .returns(T.nilable(Integer))
  def casecmp(other); end

  sig.returns(Symbol)
  def clone(); end

  sig.returns(Symbol)
  def downcase(); end

  sig.returns(Symbol)
  def dup(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def empty?(); end

  sig.returns(Encoding)
  def encoding(); end

  sig.returns(Symbol)
  def freeze(); end

  sig.returns(String)
  def id2name(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Symbol)
  def intern(); end

  sig.returns(Integer)
  def length(); end

  sig(
      obj: BasicObject,
  )
  .returns(T.nilable(Integer))
  def match(obj); end

  sig.returns(Symbol)
  def succ(); end

  sig.returns(Symbol)
  def swapcase(); end

  sig.returns(Symbol)
  def taint(); end

  sig.returns(Proc)
  def to_proc(); end

  sig.returns(Symbol)
  def trust(); end

  sig.returns(Symbol)
  def untaint(); end

  sig.returns(Symbol)
  def untrust(); end

  sig.returns(Symbol)
  def upcase(); end

  sig.returns(Integer)
  def size(); end

  sig(
      idx_or_range: Integer,
  )
  .returns(String)
  sig(
      idx_or_range: Integer,
      n: Integer,
  )
  .returns(String)
  sig(
      idx_or_range: Range[Integer],
  )
  .returns(String)
  def slice(idx_or_range, n=_); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Symbol)
  def to_sym(); end
end

class SyntaxError
  sig.returns(SyntaxError)
  def clone(); end

  sig.returns(SyntaxError)
  def dup(); end

  sig.returns(SyntaxError)
  def freeze(); end

  sig.returns(SyntaxError)
  def taint(); end

  sig.returns(SyntaxError)
  def trust(); end

  sig.returns(SyntaxError)
  def untaint(); end

  sig.returns(SyntaxError)
  def untrust(); end
end

class SystemCallError
  sig.returns(SystemCallError)
  def clone(); end

  sig.returns(SystemCallError)
  def dup(); end

  sig.returns(SystemCallError)
  def freeze(); end

  sig.returns(SystemCallError)
  def taint(); end

  sig.returns(SystemCallError)
  def trust(); end

  sig.returns(SystemCallError)
  def untaint(); end

  sig.returns(SystemCallError)
  def untrust(); end
end

class SystemExit
  sig.returns(SystemExit)
  def clone(); end

  sig.returns(SystemExit)
  def dup(); end

  sig.returns(SystemExit)
  def freeze(); end

  sig.returns(SystemExit)
  def taint(); end

  sig.returns(SystemExit)
  def trust(); end

  sig.returns(SystemExit)
  def untaint(); end

  sig.returns(SystemExit)
  def untrust(); end
end

class SystemStackError
  sig.returns(SystemStackError)
  def clone(); end

  sig.returns(SystemStackError)
  def dup(); end

  sig.returns(SystemStackError)
  def freeze(); end

  sig.returns(SystemStackError)
  def taint(); end

  sig.returns(SystemStackError)
  def trust(); end

  sig.returns(SystemStackError)
  def untaint(); end

  sig.returns(SystemStackError)
  def untrust(); end
end

class Thread
  sig.returns(Thread)
  def clone(); end

  sig.returns(Thread)
  def dup(); end

  sig.returns(Thread)
  def freeze(); end

  sig.returns(Thread)
  def taint(); end

  sig.returns(Thread)
  def trust(); end

  sig.returns(Thread)
  def untaint(); end

  sig.returns(Thread)
  def untrust(); end
end

class ThreadError
  sig.returns(ThreadError)
  def clone(); end

  sig.returns(ThreadError)
  def dup(); end

  sig.returns(ThreadError)
  def freeze(); end

  sig.returns(ThreadError)
  def taint(); end

  sig.returns(ThreadError)
  def trust(); end

  sig.returns(ThreadError)
  def untaint(); end

  sig.returns(ThreadError)
  def untrust(); end
end

class ThreadGroup
  sig.returns(ThreadGroup)
  def clone(); end

  sig.returns(ThreadGroup)
  def dup(); end

  sig.returns(ThreadGroup)
  def freeze(); end

  sig.returns(ThreadGroup)
  def taint(); end

  sig.returns(ThreadGroup)
  def trust(); end

  sig.returns(ThreadGroup)
  def untaint(); end

  sig.returns(ThreadGroup)
  def untrust(); end
end

class Time
  sig(
      seconds: Time,
  )
  .returns(Time)
  sig(
      seconds: Numeric,
  )
  .returns(Time)
  sig(
      seconds: Numeric,
      microseconds_with_frac: Numeric,
  )
  .returns(Time)
  def self.at(seconds, microseconds_with_frac=_); end

  sig(
      year: Integer,
      month: T.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
  )
  .returns(Time)
  def self.gm(year, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  sig(
      year: Integer,
      month: T.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
  )
  .returns(Time)
  def self.local(year, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  sig.returns(Time)
  def self.now(); end

  sig(
      year: Integer,
      month: T.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
  )
  .returns(Time)
  def self.utc(year, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  sig(
      arg0: Numeric,
  )
  .returns(Time)
  def +(arg0); end

  sig(
      arg0: Time,
  )
  .returns(Float)
  sig(
      arg0: Numeric,
  )
  .returns(Time)
  def -(arg0); end

  sig(
      arg0: Time,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(arg0); end

  sig(
      arg0: Time,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(arg0); end

  sig(
      other: Time,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  sig(
      arg0: Time,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(arg0); end

  sig(
      arg0: Time,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(arg0); end

  sig.returns(String)
  def asctime(); end

  sig.returns(Time)
  def clone(); end

  sig.returns(String)
  def ctime(); end

  sig.returns(Integer)
  def day(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def dst?(); end

  sig.returns(Time)
  def dup(); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  sig.returns(Time)
  def freeze(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def friday?(); end

  sig.returns(Time)
  def getgm(); end

  sig(
      utc_offset: Integer,
  )
  .returns(Time)
  def getlocal(utc_offset=_); end

  sig.returns(Time)
  def getutc(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def gmt?(); end

  sig.returns(Integer)
  def gmt_offset(); end

  sig.returns(Time)
  def gmtime(); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(Integer)
  def hour(); end

  sig(
      year: Integer,
      month: T.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
  )
  .returns(Object)
  def initialize(year=_, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  sig.returns(String)
  def inspect(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def isdst(); end

  sig(
      utc_offset: String,
  )
  .returns(Time)
  def localtime(utc_offset=_); end

  sig.returns(Integer)
  def mday(); end

  sig.returns(Integer)
  def min(); end

  sig.returns(Integer)
  def mon(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def monday?(); end

  sig.returns(Integer)
  def nsec(); end

  sig(
      arg0: Integer,
  )
  .returns(Time)
  def round(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def saturday?(); end

  sig.returns(Integer)
  def sec(); end

  sig(
      arg0: String,
  )
  .returns(String)
  def strftime(arg0); end

  sig.returns(Numeric)
  def subsec(); end

  sig.returns(Time)
  def succ(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def sunday?(); end

  sig.returns(Time)
  def taint(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def thursday?(); end

  sig.returns([Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer, T.any(TrueClass, FalseClass), String])
  def to_a(); end

  sig.returns(Float)
  def to_f(); end

  sig.returns(Numeric)
  def to_i(); end

  sig.returns(Rational)
  def to_r(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Time)
  def trust(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def tuesday?(); end

  sig.returns(Numeric)
  def tv_nsec(); end

  sig.returns(Numeric)
  def tv_sec(); end

  sig.returns(Numeric)
  def tv_usec(); end

  sig.returns(Time)
  def untaint(); end

  sig.returns(Time)
  def untrust(); end

  sig.returns(Numeric)
  def usec(); end

  sig.returns(Time)
  def utc(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def utc?(); end

  sig.returns(Integer)
  def utc_offset(); end

  sig.returns(Integer)
  def wday(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def wednesday?(); end

  sig.returns(Integer)
  def yday(); end

  sig.returns(Integer)
  def year(); end

  sig.returns(String)
  def zone(); end

  sig(
      year: Integer,
      month: T.any(Integer, String),
      day: Integer,
      hour: Integer,
      min: Integer,
      sec: Numeric,
      usec_with_frac: Numeric,
  )
  .returns(Time)
  def self.mktime(year, month=_, day=_, hour=_, min=_, sec=_, usec_with_frac=_); end

  sig.returns(Integer)
  def gmtoff(); end

  sig.returns(Integer)
  def month(); end
end

class TracePoint
  sig.returns(TracePoint)
  def clone(); end

  sig.returns(TracePoint)
  def dup(); end

  sig.returns(TracePoint)
  def freeze(); end

  sig.returns(TracePoint)
  def taint(); end

  sig.returns(TracePoint)
  def trust(); end

  sig.returns(TracePoint)
  def untaint(); end

  sig.returns(TracePoint)
  def untrust(); end
end

class TrueClass
  sig.returns(TrueClass)
  def clone(); end

  sig.returns(TrueClass)
  def dup(); end

  sig.returns(TrueClass)
  def freeze(); end

  sig.returns(TrueClass)
  def taint(); end

  sig.returns(TrueClass)
  def trust(); end

  sig.returns(TrueClass)
  def untaint(); end

  sig.returns(TrueClass)
  def untrust(); end
end

class TypeError
  sig.returns(TypeError)
  def clone(); end

  sig.returns(TypeError)
  def dup(); end

  sig.returns(TypeError)
  def freeze(); end

  sig.returns(TypeError)
  def taint(); end

  sig.returns(TypeError)
  def trust(); end

  sig.returns(TypeError)
  def untaint(); end

  sig.returns(TypeError)
  def untrust(); end
end

module URI
  sig(
      str: String,
      enc: Encoding,
  )
  .returns(T::Array[[String, String]])
  def self.decode_www_form_component(str, enc=_); end

  sig(
      arg: String,
      arg0: Regexp,
  )
  .returns(String)
  sig(
      arg: String,
      arg0: String,
  )
  .returns(String)
  def self.escape(arg, *arg0); end

  sig(
      str: String,
      schemes: Array,
      blk: BasicObject,
  )
  .returns(T::Array[String])
  def self.extract(str, schemes=_, &blk); end

  sig(
      str: String,
  )
  .returns(URI::HTTP)
  def self.join(*str); end

  sig(
      uri: String,
  )
  .returns(URI::HTTP)
  def self.parse(uri); end

  sig(
      schemes: Array,
  )
  .returns(T::Array[String])
  def self.regexp(schemes=_); end

  sig.returns(T::Hash[String, Class])
  def self.scheme_list(); end

  sig(
      uri: String,
  )
  .returns(T::Array[T.nilable(String)])
  def self.split(uri); end

  sig(
      arg: String,
  )
  .returns(String)
  def self.unescape(*arg); end

  sig(
      arg: String,
      arg0: Regexp,
  )
  .returns(String)
  sig(
      arg: String,
      arg0: String,
  )
  .returns(String)
  def self.encode(arg, *arg0); end

  sig(
      arg: String,
  )
  .returns(String)
  def self.decode(*arg); end
end

class UnboundMethod
  sig.returns(UnboundMethod)
  def clone(); end

  sig.returns(UnboundMethod)
  def dup(); end

  sig.returns(UnboundMethod)
  def freeze(); end

  sig.returns(UnboundMethod)
  def taint(); end

  sig.returns(UnboundMethod)
  def trust(); end

  sig.returns(UnboundMethod)
  def untaint(); end

  sig.returns(UnboundMethod)
  def untrust(); end
end

class UncaughtThrowError
  sig.returns(UncaughtThrowError)
  def clone(); end

  sig.returns(UncaughtThrowError)
  def dup(); end

  sig.returns(UncaughtThrowError)
  def freeze(); end

  sig.returns(UncaughtThrowError)
  def taint(); end

  sig.returns(UncaughtThrowError)
  def trust(); end

  sig.returns(UncaughtThrowError)
  def untaint(); end

  sig.returns(UncaughtThrowError)
  def untrust(); end
end

class ZeroDivisionError
  sig.returns(ZeroDivisionError)
  def clone(); end

  sig.returns(ZeroDivisionError)
  def dup(); end

  sig.returns(ZeroDivisionError)
  def freeze(); end

  sig.returns(ZeroDivisionError)
  def taint(); end

  sig.returns(ZeroDivisionError)
  def trust(); end

  sig.returns(ZeroDivisionError)
  def untaint(); end

  sig.returns(ZeroDivisionError)
  def untrust(); end
end
::ARGF = T.let(T.unsafe(nil), Object)
::ARGV = T.let(T.unsafe(nil), Array)
::CROSS_COMPILING = T.let(T.unsafe(nil), NilClass)
::FALSE = T.let(T.unsafe(nil), FalseClass)
::NIL = T.let(T.unsafe(nil), NilClass)
::RUBYGEMS_ACTIVATION_MONITOR = T.let(T.unsafe(nil), Monitor)
::RUBY_COPYRIGHT = T.let(T.unsafe(nil), String)
::RUBY_DESCRIPTION = T.let(T.unsafe(nil), String)
::RUBY_ENGINE = T.let(T.unsafe(nil), String)
::RUBY_ENGINE_VERSION = T.let(T.unsafe(nil), String)
::RUBY_PATCHLEVEL = T.let(T.unsafe(nil), Integer)
::RUBY_PLATFORM = T.let(T.unsafe(nil), String)
::RUBY_RELEASE_DATE = T.let(T.unsafe(nil), String)
::RUBY_REVISION = T.let(T.unsafe(nil), Integer)
::RUBY_VERSION = T.let(T.unsafe(nil), String)
::STDERR = T.let(T.unsafe(nil), IO)
::STDIN = T.let(T.unsafe(nil), IO)
::STDOUT = T.let(T.unsafe(nil), IO)
::TOPLEVEL_BINDING = T.let(T.unsafe(nil), Binding)
::TRUE = T.let(T.unsafe(nil), TrueClass)
Benchmark::BENCHMARK_VERSION = T.let(T.unsafe(nil), String)
Benchmark::CAPTION = T.let(T.unsafe(nil), String)
Benchmark::FORMAT = T.let(T.unsafe(nil), String)
Benchmark::Tms::CAPTION = T.let(T.unsafe(nil), String)
Benchmark::Tms::FORMAT = T.let(T.unsafe(nil), String)
BigDecimal::BASE = T.let(T.unsafe(nil), Integer)
BigDecimal::EXCEPTION_ALL = T.let(T.unsafe(nil), Integer)
BigDecimal::EXCEPTION_INFINITY = T.let(T.unsafe(nil), Integer)
BigDecimal::EXCEPTION_OVERFLOW = T.let(T.unsafe(nil), Integer)
BigDecimal::EXCEPTION_UNDERFLOW = T.let(T.unsafe(nil), Integer)
BigDecimal::EXCEPTION_ZERODIVIDE = T.let(T.unsafe(nil), Integer)
BigDecimal::INFINITY = T.let(T.unsafe(nil), BigDecimal)
BigDecimal::NAN = T.let(T.unsafe(nil), BigDecimal)
BigDecimal::ROUND_CEILING = T.let(T.unsafe(nil), Integer)
BigDecimal::ROUND_DOWN = T.let(T.unsafe(nil), Integer)
BigDecimal::ROUND_FLOOR = T.let(T.unsafe(nil), Integer)
BigDecimal::ROUND_HALF_DOWN = T.let(T.unsafe(nil), Integer)
BigDecimal::ROUND_HALF_EVEN = T.let(T.unsafe(nil), Integer)
BigDecimal::ROUND_HALF_UP = T.let(T.unsafe(nil), Integer)
BigDecimal::ROUND_MODE = T.let(T.unsafe(nil), Integer)
BigDecimal::ROUND_UP = T.let(T.unsafe(nil), Integer)
BigDecimal::SIGN_NEGATIVE_FINITE = T.let(T.unsafe(nil), Integer)
BigDecimal::SIGN_NEGATIVE_INFINITE = T.let(T.unsafe(nil), Integer)
BigDecimal::SIGN_NEGATIVE_ZERO = T.let(T.unsafe(nil), Integer)
BigDecimal::SIGN_POSITIVE_FINITE = T.let(T.unsafe(nil), Integer)
BigDecimal::SIGN_POSITIVE_INFINITE = T.let(T.unsafe(nil), Integer)
BigDecimal::SIGN_POSITIVE_ZERO = T.let(T.unsafe(nil), Integer)
CSV::DEFAULT_OPTIONS = T.let(T.unsafe(nil), Hash)
CSV::VERSION = T.let(T.unsafe(nil), String)
Complex::I = T.let(T.unsafe(nil), Complex)
Date::ABBR_DAYNAMES = T.let(T.unsafe(nil), Array)
Date::ABBR_MONTHNAMES = T.let(T.unsafe(nil), Array)
Date::DAYNAMES = T.let(T.unsafe(nil), Array)
Date::ENGLAND = T.let(T.unsafe(nil), Integer)
Date::GREGORIAN = T.let(T.unsafe(nil), Float)
Date::ITALY = T.let(T.unsafe(nil), Integer)
Date::JULIAN = T.let(T.unsafe(nil), Float)
Date::MONTHNAMES = T.let(T.unsafe(nil), Array)
DateTime::ABBR_DAYNAMES = T.let(T.unsafe(nil), Array)
DateTime::ABBR_MONTHNAMES = T.let(T.unsafe(nil), Array)
DateTime::DAYNAMES = T.let(T.unsafe(nil), Array)
DateTime::ENGLAND = T.let(T.unsafe(nil), Integer)
DateTime::GREGORIAN = T.let(T.unsafe(nil), Float)
DateTime::ITALY = T.let(T.unsafe(nil), Integer)
DateTime::JULIAN = T.let(T.unsafe(nil), Float)
DateTime::MONTHNAMES = T.let(T.unsafe(nil), Array)
DidYouMean::ClassNameChecker::ClassName::RUBYGEMS_ACTIVATION_MONITOR = T.let(T.unsafe(nil), Monitor)
DidYouMean::IGNORED_CALLERS = T.let(T.unsafe(nil), Array)
DidYouMean::JaroWinkler::THRESHOLD = T.let(T.unsafe(nil), Float)
DidYouMean::JaroWinkler::WEIGHT = T.let(T.unsafe(nil), Float)
DidYouMean::MethodNameChecker::NAMES_TO_EXCLUDE = T.let(T.unsafe(nil), Hash)
DidYouMean::SPELL_CHECKERS = T.let(T.unsafe(nil), Hash)
DidYouMean::VERSION = T.let(T.unsafe(nil), String)
DidYouMean::VariableNameChecker::NAMES_TO_EXCLUDE = T.let(T.unsafe(nil), Hash)
Encoding::ANSI_X3_4_1968 = T.let(T.unsafe(nil), Encoding)
Encoding::ASCII = T.let(T.unsafe(nil), Encoding)
Encoding::ASCII_8BIT = T.let(T.unsafe(nil), Encoding)
Encoding::BIG5 = T.let(T.unsafe(nil), Encoding)
Encoding::BIG5_HKSCS = T.let(T.unsafe(nil), Encoding)
Encoding::BIG5_HKSCS_2008 = T.let(T.unsafe(nil), Encoding)
Encoding::BIG5_UAO = T.let(T.unsafe(nil), Encoding)
Encoding::BINARY = T.let(T.unsafe(nil), Encoding)
Encoding::CP1250 = T.let(T.unsafe(nil), Encoding)
Encoding::CP1251 = T.let(T.unsafe(nil), Encoding)
Encoding::CP1252 = T.let(T.unsafe(nil), Encoding)
Encoding::CP1253 = T.let(T.unsafe(nil), Encoding)
Encoding::CP1254 = T.let(T.unsafe(nil), Encoding)
Encoding::CP1255 = T.let(T.unsafe(nil), Encoding)
Encoding::CP1256 = T.let(T.unsafe(nil), Encoding)
Encoding::CP1257 = T.let(T.unsafe(nil), Encoding)
Encoding::CP1258 = T.let(T.unsafe(nil), Encoding)
Encoding::CP437 = T.let(T.unsafe(nil), Encoding)
Encoding::CP50220 = T.let(T.unsafe(nil), Encoding)
Encoding::CP50221 = T.let(T.unsafe(nil), Encoding)
Encoding::CP51932 = T.let(T.unsafe(nil), Encoding)
Encoding::CP65000 = T.let(T.unsafe(nil), Encoding)
Encoding::CP65001 = T.let(T.unsafe(nil), Encoding)
Encoding::CP737 = T.let(T.unsafe(nil), Encoding)
Encoding::CP775 = T.let(T.unsafe(nil), Encoding)
Encoding::CP850 = T.let(T.unsafe(nil), Encoding)
Encoding::CP852 = T.let(T.unsafe(nil), Encoding)
Encoding::CP855 = T.let(T.unsafe(nil), Encoding)
Encoding::CP857 = T.let(T.unsafe(nil), Encoding)
Encoding::CP860 = T.let(T.unsafe(nil), Encoding)
Encoding::CP861 = T.let(T.unsafe(nil), Encoding)
Encoding::CP862 = T.let(T.unsafe(nil), Encoding)
Encoding::CP863 = T.let(T.unsafe(nil), Encoding)
Encoding::CP864 = T.let(T.unsafe(nil), Encoding)
Encoding::CP865 = T.let(T.unsafe(nil), Encoding)
Encoding::CP866 = T.let(T.unsafe(nil), Encoding)
Encoding::CP869 = T.let(T.unsafe(nil), Encoding)
Encoding::CP874 = T.let(T.unsafe(nil), Encoding)
Encoding::CP878 = T.let(T.unsafe(nil), Encoding)
Encoding::CP932 = T.let(T.unsafe(nil), Encoding)
Encoding::CP936 = T.let(T.unsafe(nil), Encoding)
Encoding::CP949 = T.let(T.unsafe(nil), Encoding)
Encoding::CP950 = T.let(T.unsafe(nil), Encoding)
Encoding::CP951 = T.let(T.unsafe(nil), Encoding)
Encoding::CSWINDOWS31J = T.let(T.unsafe(nil), Encoding)
Encoding::Converter::AFTER_OUTPUT = T.let(T.unsafe(nil), Integer)
Encoding::Converter::CRLF_NEWLINE_DECORATOR = T.let(T.unsafe(nil), Integer)
Encoding::Converter::CR_NEWLINE_DECORATOR = T.let(T.unsafe(nil), Integer)
Encoding::Converter::INVALID_MASK = T.let(T.unsafe(nil), Integer)
Encoding::Converter::INVALID_REPLACE = T.let(T.unsafe(nil), Integer)
Encoding::Converter::PARTIAL_INPUT = T.let(T.unsafe(nil), Integer)
Encoding::Converter::UNDEF_HEX_CHARREF = T.let(T.unsafe(nil), Integer)
Encoding::Converter::UNDEF_MASK = T.let(T.unsafe(nil), Integer)
Encoding::Converter::UNDEF_REPLACE = T.let(T.unsafe(nil), Integer)
Encoding::Converter::UNIVERSAL_NEWLINE_DECORATOR = T.let(T.unsafe(nil), Integer)
Encoding::Converter::XML_ATTR_CONTENT_DECORATOR = T.let(T.unsafe(nil), Integer)
Encoding::Converter::XML_ATTR_QUOTE_DECORATOR = T.let(T.unsafe(nil), Integer)
Encoding::Converter::XML_TEXT_DECORATOR = T.let(T.unsafe(nil), Integer)
Encoding::EBCDIC_CP_US = T.let(T.unsafe(nil), Encoding)
Encoding::EMACS_MULE = T.let(T.unsafe(nil), Encoding)
Encoding::EUCCN = T.let(T.unsafe(nil), Encoding)
Encoding::EUCJP = T.let(T.unsafe(nil), Encoding)
Encoding::EUCJP_MS = T.let(T.unsafe(nil), Encoding)
Encoding::EUCKR = T.let(T.unsafe(nil), Encoding)
Encoding::EUCTW = T.let(T.unsafe(nil), Encoding)
Encoding::EUC_CN = T.let(T.unsafe(nil), Encoding)
Encoding::EUC_JISX0213 = T.let(T.unsafe(nil), Encoding)
Encoding::EUC_JIS_2004 = T.let(T.unsafe(nil), Encoding)
Encoding::EUC_JP = T.let(T.unsafe(nil), Encoding)
Encoding::EUC_JP_MS = T.let(T.unsafe(nil), Encoding)
Encoding::EUC_KR = T.let(T.unsafe(nil), Encoding)
Encoding::EUC_TW = T.let(T.unsafe(nil), Encoding)
Encoding::GB12345 = T.let(T.unsafe(nil), Encoding)
Encoding::GB18030 = T.let(T.unsafe(nil), Encoding)
Encoding::GB1988 = T.let(T.unsafe(nil), Encoding)
Encoding::GB2312 = T.let(T.unsafe(nil), Encoding)
Encoding::GBK = T.let(T.unsafe(nil), Encoding)
Encoding::IBM037 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM437 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM737 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM775 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM850 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM852 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM855 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM857 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM860 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM861 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM862 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM863 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM864 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM865 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM866 = T.let(T.unsafe(nil), Encoding)
Encoding::IBM869 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO2022_JP = T.let(T.unsafe(nil), Encoding)
Encoding::ISO2022_JP2 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_1 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_10 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_11 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_13 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_14 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_15 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_16 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_2 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_3 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_4 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_5 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_6 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_7 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_8 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO8859_9 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_2022_JP = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_2022_JP_2 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_2022_JP_KDDI = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_1 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_10 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_11 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_13 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_14 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_15 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_16 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_2 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_3 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_4 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_5 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_6 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_7 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_8 = T.let(T.unsafe(nil), Encoding)
Encoding::ISO_8859_9 = T.let(T.unsafe(nil), Encoding)
Encoding::KOI8_R = T.let(T.unsafe(nil), Encoding)
Encoding::KOI8_U = T.let(T.unsafe(nil), Encoding)
Encoding::MACCENTEURO = T.let(T.unsafe(nil), Encoding)
Encoding::MACCROATIAN = T.let(T.unsafe(nil), Encoding)
Encoding::MACCYRILLIC = T.let(T.unsafe(nil), Encoding)
Encoding::MACGREEK = T.let(T.unsafe(nil), Encoding)
Encoding::MACICELAND = T.let(T.unsafe(nil), Encoding)
Encoding::MACJAPAN = T.let(T.unsafe(nil), Encoding)
Encoding::MACJAPANESE = T.let(T.unsafe(nil), Encoding)
Encoding::MACROMAN = T.let(T.unsafe(nil), Encoding)
Encoding::MACROMANIA = T.let(T.unsafe(nil), Encoding)
Encoding::MACTHAI = T.let(T.unsafe(nil), Encoding)
Encoding::MACTURKISH = T.let(T.unsafe(nil), Encoding)
Encoding::MACUKRAINE = T.let(T.unsafe(nil), Encoding)
Encoding::PCK = T.let(T.unsafe(nil), Encoding)
Encoding::SHIFT_JIS = T.let(T.unsafe(nil), Encoding)
Encoding::SJIS = T.let(T.unsafe(nil), Encoding)
Encoding::SJIS_DOCOMO = T.let(T.unsafe(nil), Encoding)
Encoding::SJIS_KDDI = T.let(T.unsafe(nil), Encoding)
Encoding::SJIS_SOFTBANK = T.let(T.unsafe(nil), Encoding)
Encoding::STATELESS_ISO_2022_JP = T.let(T.unsafe(nil), Encoding)
Encoding::STATELESS_ISO_2022_JP_KDDI = T.let(T.unsafe(nil), Encoding)
Encoding::TIS_620 = T.let(T.unsafe(nil), Encoding)
Encoding::UCS_2BE = T.let(T.unsafe(nil), Encoding)
Encoding::UCS_4BE = T.let(T.unsafe(nil), Encoding)
Encoding::UCS_4LE = T.let(T.unsafe(nil), Encoding)
Encoding::US_ASCII = T.let(T.unsafe(nil), Encoding)
Encoding::UTF8_DOCOMO = T.let(T.unsafe(nil), Encoding)
Encoding::UTF8_KDDI = T.let(T.unsafe(nil), Encoding)
Encoding::UTF8_MAC = T.let(T.unsafe(nil), Encoding)
Encoding::UTF8_SOFTBANK = T.let(T.unsafe(nil), Encoding)
Encoding::UTF_16 = T.let(T.unsafe(nil), Encoding)
Encoding::UTF_16BE = T.let(T.unsafe(nil), Encoding)
Encoding::UTF_16LE = T.let(T.unsafe(nil), Encoding)
Encoding::UTF_32 = T.let(T.unsafe(nil), Encoding)
Encoding::UTF_32BE = T.let(T.unsafe(nil), Encoding)
Encoding::UTF_32LE = T.let(T.unsafe(nil), Encoding)
Encoding::UTF_7 = T.let(T.unsafe(nil), Encoding)
Encoding::UTF_8 = T.let(T.unsafe(nil), Encoding)
Encoding::UTF_8_HFS = T.let(T.unsafe(nil), Encoding)
Encoding::UTF_8_MAC = T.let(T.unsafe(nil), Encoding)
Encoding::WINDOWS_1250 = T.let(T.unsafe(nil), Encoding)
Encoding::WINDOWS_1251 = T.let(T.unsafe(nil), Encoding)
Encoding::WINDOWS_1252 = T.let(T.unsafe(nil), Encoding)
Encoding::WINDOWS_1253 = T.let(T.unsafe(nil), Encoding)
Encoding::WINDOWS_1254 = T.let(T.unsafe(nil), Encoding)
Encoding::WINDOWS_1255 = T.let(T.unsafe(nil), Encoding)
Encoding::WINDOWS_1256 = T.let(T.unsafe(nil), Encoding)
Encoding::WINDOWS_1257 = T.let(T.unsafe(nil), Encoding)
Encoding::WINDOWS_1258 = T.let(T.unsafe(nil), Encoding)
Encoding::WINDOWS_31J = T.let(T.unsafe(nil), Encoding)
Encoding::WINDOWS_874 = T.let(T.unsafe(nil), Encoding)
File::ALT_SEPARATOR = T.let(T.unsafe(nil), NilClass)
File::APPEND = T.let(T.unsafe(nil), Integer)
File::BINARY = T.let(T.unsafe(nil), Integer)
File::CREAT = T.let(T.unsafe(nil), Integer)
File::Constants::APPEND = T.let(T.unsafe(nil), Integer)
File::Constants::BINARY = T.let(T.unsafe(nil), Integer)
File::Constants::CREAT = T.let(T.unsafe(nil), Integer)
File::Constants::DIRECT = T.let(T.unsafe(nil), Integer)
File::Constants::DSYNC = T.let(T.unsafe(nil), Integer)
File::Constants::EXCL = T.let(T.unsafe(nil), Integer)
File::Constants::FNM_CASEFOLD = T.let(T.unsafe(nil), Integer)
File::Constants::FNM_DOTMATCH = T.let(T.unsafe(nil), Integer)
File::Constants::FNM_EXTGLOB = T.let(T.unsafe(nil), Integer)
File::Constants::FNM_NOESCAPE = T.let(T.unsafe(nil), Integer)
File::Constants::FNM_PATHNAME = T.let(T.unsafe(nil), Integer)
File::Constants::FNM_SHORTNAME = T.let(T.unsafe(nil), Integer)
File::Constants::FNM_SYSCASE = T.let(T.unsafe(nil), Integer)
File::Constants::LOCK_EX = T.let(T.unsafe(nil), Integer)
File::Constants::LOCK_NB = T.let(T.unsafe(nil), Integer)
File::Constants::LOCK_SH = T.let(T.unsafe(nil), Integer)
File::Constants::LOCK_UN = T.let(T.unsafe(nil), Integer)
File::Constants::NOATIME = T.let(T.unsafe(nil), Integer)
File::Constants::NOCTTY = T.let(T.unsafe(nil), Integer)
File::Constants::NOFOLLOW = T.let(T.unsafe(nil), Integer)
File::Constants::NONBLOCK = T.let(T.unsafe(nil), Integer)
File::Constants::NULL = T.let(T.unsafe(nil), String)
File::Constants::RDONLY = T.let(T.unsafe(nil), Integer)
File::Constants::RDWR = T.let(T.unsafe(nil), Integer)
File::Constants::RSYNC = T.let(T.unsafe(nil), Integer)
File::Constants::SHARE_DELETE = T.let(T.unsafe(nil), Integer)
File::Constants::SYNC = T.let(T.unsafe(nil), Integer)
File::Constants::TMPFILE = T.let(T.unsafe(nil), Integer)
File::Constants::TRUNC = T.let(T.unsafe(nil), Integer)
File::Constants::WRONLY = T.let(T.unsafe(nil), Integer)
File::DIRECT = T.let(T.unsafe(nil), Integer)
File::DSYNC = T.let(T.unsafe(nil), Integer)
File::EXCL = T.let(T.unsafe(nil), Integer)
File::FNM_CASEFOLD = T.let(T.unsafe(nil), Integer)
File::FNM_DOTMATCH = T.let(T.unsafe(nil), Integer)
File::FNM_EXTGLOB = T.let(T.unsafe(nil), Integer)
File::FNM_NOESCAPE = T.let(T.unsafe(nil), Integer)
File::FNM_PATHNAME = T.let(T.unsafe(nil), Integer)
File::FNM_SHORTNAME = T.let(T.unsafe(nil), Integer)
File::FNM_SYSCASE = T.let(T.unsafe(nil), Integer)
File::LOCK_EX = T.let(T.unsafe(nil), Integer)
File::LOCK_NB = T.let(T.unsafe(nil), Integer)
File::LOCK_SH = T.let(T.unsafe(nil), Integer)
File::LOCK_UN = T.let(T.unsafe(nil), Integer)
File::NOATIME = T.let(T.unsafe(nil), Integer)
File::NOCTTY = T.let(T.unsafe(nil), Integer)
File::NOFOLLOW = T.let(T.unsafe(nil), Integer)
File::NONBLOCK = T.let(T.unsafe(nil), Integer)
File::NULL = T.let(T.unsafe(nil), String)
File::PATH_SEPARATOR = T.let(T.unsafe(nil), String)
File::RDONLY = T.let(T.unsafe(nil), Integer)
File::RDWR = T.let(T.unsafe(nil), Integer)
File::RSYNC = T.let(T.unsafe(nil), Integer)
File::SEEK_CUR = T.let(T.unsafe(nil), Integer)
File::SEEK_DATA = T.let(T.unsafe(nil), Integer)
File::SEEK_END = T.let(T.unsafe(nil), Integer)
File::SEEK_HOLE = T.let(T.unsafe(nil), Integer)
File::SEEK_SET = T.let(T.unsafe(nil), Integer)
File::SEPARATOR = T.let(T.unsafe(nil), String)
File::SHARE_DELETE = T.let(T.unsafe(nil), Integer)
File::SYNC = T.let(T.unsafe(nil), Integer)
File::TMPFILE = T.let(T.unsafe(nil), Integer)
File::TRUNC = T.let(T.unsafe(nil), Integer)
File::WRONLY = T.let(T.unsafe(nil), Integer)
Float::DIG = T.let(T.unsafe(nil), Integer)
Float::EPSILON = T.let(T.unsafe(nil), Float)
Float::INFINITY = T.let(T.unsafe(nil), Float)
Float::MANT_DIG = T.let(T.unsafe(nil), Integer)
Float::MAX = T.let(T.unsafe(nil), Float)
Float::MAX_10_EXP = T.let(T.unsafe(nil), Integer)
Float::MAX_EXP = T.let(T.unsafe(nil), Integer)
Float::MIN = T.let(T.unsafe(nil), Float)
Float::MIN_10_EXP = T.let(T.unsafe(nil), Integer)
Float::MIN_EXP = T.let(T.unsafe(nil), Integer)
Float::NAN = T.let(T.unsafe(nil), Float)
Float::RADIX = T.let(T.unsafe(nil), Integer)
Float::ROUNDS = T.let(T.unsafe(nil), Integer)
Forwardable::FILTER_EXCEPTION = T.let(T.unsafe(nil), String)
Forwardable::FORWARDABLE_VERSION = T.let(T.unsafe(nil), String)
GC::INTERNAL_CONSTANTS = T.let(T.unsafe(nil), Hash)
GC::OPTS = T.let(T.unsafe(nil), Array)
Gem::DEFAULT_HOST = T.let(T.unsafe(nil), String)
Gem::Dependency::TYPES = T.let(T.unsafe(nil), Array)
Gem::GEM_DEP_FILES = T.let(T.unsafe(nil), Array)
Gem::GEM_PRELUDE_SUCKAGE = T.let(T.unsafe(nil), NilClass)
Gem::LOADED_SPECS_MUTEX = T.let(T.unsafe(nil), Thread::Mutex)
Gem::MARSHAL_SPEC_DIR = T.let(T.unsafe(nil), String)
Gem::Platform::CURRENT = T.let(T.unsafe(nil), String)
Gem::Platform::RUBY = T.let(T.unsafe(nil), String)
Gem::READ_BINARY_ERRORS = T.let(T.unsafe(nil), Array)
Gem::REPOSITORY_DEFAULT_GEM_SUBDIRECTORIES = T.let(T.unsafe(nil), Array)
Gem::REPOSITORY_SUBDIRECTORIES = T.let(T.unsafe(nil), Array)
Gem::RUBYGEMS_DIR = T.let(T.unsafe(nil), String)
Gem::Requirement::OPS = T.let(T.unsafe(nil), Hash)
Gem::Requirement::PATTERN = T.let(T.unsafe(nil), Regexp)
Gem::Requirement::PATTERN_RAW = T.let(T.unsafe(nil), String)
Gem::Requirement::SOURCE_SET_REQUIREMENT = T.let(T.unsafe(nil), Object)
Gem::Specification::CURRENT_SPECIFICATION_VERSION = T.let(T.unsafe(nil), Integer)
Gem::Specification::EMPTY = T.let(T.unsafe(nil), Array)
Gem::Specification::MARSHAL_FIELDS = T.let(T.unsafe(nil), Hash)
Gem::Specification::NONEXISTENT_SPECIFICATION_VERSION = T.let(T.unsafe(nil), Integer)
Gem::Specification::NOT_FOUND = T.let(T.unsafe(nil), Object)
Gem::Specification::SPECIFICATION_VERSION_HISTORY = T.let(T.unsafe(nil), Hash)
Gem::Specification::TODAY = T.let(T.unsafe(nil), Time)
Gem::Specification::VALID_NAME_PATTERN = T.let(T.unsafe(nil), Regexp)
Gem::StubSpecification::OPEN_MODE = T.let(T.unsafe(nil), String)
Gem::StubSpecification::PREFIX = T.let(T.unsafe(nil), String)
Gem::StubSpecification::StubLine::NO_EXTENSIONS = T.let(T.unsafe(nil), Array)
Gem::StubSpecification::StubLine::REQUIRE_PATHS = T.let(T.unsafe(nil), Hash)
Gem::StubSpecification::StubLine::REQUIRE_PATH_LIST = T.let(T.unsafe(nil), Hash)
Gem::VERSION = T.let(T.unsafe(nil), String)
Gem::Version::ANCHORED_VERSION_PATTERN = T.let(T.unsafe(nil), Regexp)
Gem::Version::VERSION_PATTERN = T.let(T.unsafe(nil), String)
Gem::WIN_PATTERNS = T.let(T.unsafe(nil), Array)
Gem::WRITE_BINARY_ERRORS = T.let(T.unsafe(nil), Array)
IO::APPEND = T.let(T.unsafe(nil), Integer)
IO::BINARY = T.let(T.unsafe(nil), Integer)
IO::CREAT = T.let(T.unsafe(nil), Integer)
IO::DIRECT = T.let(T.unsafe(nil), Integer)
IO::DSYNC = T.let(T.unsafe(nil), Integer)
IO::EXCL = T.let(T.unsafe(nil), Integer)
IO::FNM_CASEFOLD = T.let(T.unsafe(nil), Integer)
IO::FNM_DOTMATCH = T.let(T.unsafe(nil), Integer)
IO::FNM_EXTGLOB = T.let(T.unsafe(nil), Integer)
IO::FNM_NOESCAPE = T.let(T.unsafe(nil), Integer)
IO::FNM_PATHNAME = T.let(T.unsafe(nil), Integer)
IO::FNM_SHORTNAME = T.let(T.unsafe(nil), Integer)
IO::FNM_SYSCASE = T.let(T.unsafe(nil), Integer)
IO::LOCK_EX = T.let(T.unsafe(nil), Integer)
IO::LOCK_NB = T.let(T.unsafe(nil), Integer)
IO::LOCK_SH = T.let(T.unsafe(nil), Integer)
IO::LOCK_UN = T.let(T.unsafe(nil), Integer)
IO::NOATIME = T.let(T.unsafe(nil), Integer)
IO::NOCTTY = T.let(T.unsafe(nil), Integer)
IO::NOFOLLOW = T.let(T.unsafe(nil), Integer)
IO::NONBLOCK = T.let(T.unsafe(nil), Integer)
IO::NULL = T.let(T.unsafe(nil), String)
IO::RDONLY = T.let(T.unsafe(nil), Integer)
IO::RDWR = T.let(T.unsafe(nil), Integer)
IO::RSYNC = T.let(T.unsafe(nil), Integer)
IO::SEEK_CUR = T.let(T.unsafe(nil), Integer)
IO::SEEK_DATA = T.let(T.unsafe(nil), Integer)
IO::SEEK_END = T.let(T.unsafe(nil), Integer)
IO::SEEK_HOLE = T.let(T.unsafe(nil), Integer)
IO::SEEK_SET = T.let(T.unsafe(nil), Integer)
IO::SHARE_DELETE = T.let(T.unsafe(nil), Integer)
IO::SYNC = T.let(T.unsafe(nil), Integer)
IO::TMPFILE = T.let(T.unsafe(nil), Integer)
IO::TRUNC = T.let(T.unsafe(nil), Integer)
IO::WRONLY = T.let(T.unsafe(nil), Integer)
Kernel::RUBYGEMS_ACTIVATION_MONITOR = T.let(T.unsafe(nil), Monitor)
Marshal::MAJOR_VERSION = T.let(T.unsafe(nil), Integer)
Marshal::MINOR_VERSION = T.let(T.unsafe(nil), Integer)
Math::E = T.let(T.unsafe(nil), Float)
Math::PI = T.let(T.unsafe(nil), Float)
Module::ARGF = T.let(T.unsafe(nil), Object)
Module::ARGV = T.let(T.unsafe(nil), Array)
Module::CROSS_COMPILING = T.let(T.unsafe(nil), NilClass)
Module::FALSE = T.let(T.unsafe(nil), FalseClass)
Module::NIL = T.let(T.unsafe(nil), NilClass)
Module::RUBY_COPYRIGHT = T.let(T.unsafe(nil), String)
Module::RUBY_DESCRIPTION = T.let(T.unsafe(nil), String)
Module::RUBY_ENGINE = T.let(T.unsafe(nil), String)
Module::RUBY_ENGINE_VERSION = T.let(T.unsafe(nil), String)
Module::RUBY_PATCHLEVEL = T.let(T.unsafe(nil), Integer)
Module::RUBY_PLATFORM = T.let(T.unsafe(nil), String)
Module::RUBY_RELEASE_DATE = T.let(T.unsafe(nil), String)
Module::RUBY_REVISION = T.let(T.unsafe(nil), Integer)
Module::RUBY_VERSION = T.let(T.unsafe(nil), String)
Module::STDERR = T.let(T.unsafe(nil), IO)
Module::STDIN = T.let(T.unsafe(nil), IO)
Module::STDOUT = T.let(T.unsafe(nil), IO)
Module::TOPLEVEL_BINDING = T.let(T.unsafe(nil), Binding)
Module::TRUE = T.let(T.unsafe(nil), TrueClass)
Process::CLOCK_BOOTTIME = T.let(T.unsafe(nil), Integer)
Process::CLOCK_BOOTTIME_ALARM = T.let(T.unsafe(nil), Integer)
Process::CLOCK_MONOTONIC = T.let(T.unsafe(nil), Integer)
Process::CLOCK_MONOTONIC_COARSE = T.let(T.unsafe(nil), Integer)
Process::CLOCK_MONOTONIC_RAW = T.let(T.unsafe(nil), Integer)
Process::CLOCK_PROCESS_CPUTIME_ID = T.let(T.unsafe(nil), Integer)
Process::CLOCK_REALTIME = T.let(T.unsafe(nil), Integer)
Process::CLOCK_REALTIME_ALARM = T.let(T.unsafe(nil), Integer)
Process::CLOCK_REALTIME_COARSE = T.let(T.unsafe(nil), Integer)
Process::CLOCK_THREAD_CPUTIME_ID = T.let(T.unsafe(nil), Integer)
Process::PRIO_PGRP = T.let(T.unsafe(nil), Integer)
Process::PRIO_PROCESS = T.let(T.unsafe(nil), Integer)
Process::PRIO_USER = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_AS = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_CORE = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_CPU = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_DATA = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_FSIZE = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_MEMLOCK = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_MSGQUEUE = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_NICE = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_NOFILE = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_NPROC = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_RSS = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_RTPRIO = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_RTTIME = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_SIGPENDING = T.let(T.unsafe(nil), Integer)
Process::RLIMIT_STACK = T.let(T.unsafe(nil), Integer)
Process::RLIM_INFINITY = T.let(T.unsafe(nil), Integer)
Process::RLIM_SAVED_CUR = T.let(T.unsafe(nil), Integer)
Process::RLIM_SAVED_MAX = T.let(T.unsafe(nil), Integer)
Process::WNOHANG = T.let(T.unsafe(nil), Integer)
Process::WUNTRACED = T.let(T.unsafe(nil), Integer)
Random::DEFAULT = T.let(T.unsafe(nil), Random)
RbConfig::CONFIG = T.let(T.unsafe(nil), Hash)
RbConfig::DESTDIR = T.let(T.unsafe(nil), String)
RbConfig::MAKEFILE_CONFIG = T.let(T.unsafe(nil), Hash)
RbConfig::TOPDIR = T.let(T.unsafe(nil), String)
Regexp::EXTENDED = T.let(T.unsafe(nil), Integer)
Regexp::FIXEDENCODING = T.let(T.unsafe(nil), Integer)
Regexp::IGNORECASE = T.let(T.unsafe(nil), Integer)
Regexp::MULTILINE = T.let(T.unsafe(nil), Integer)
Regexp::NOENCODING = T.let(T.unsafe(nil), Integer)
RubyVM::DEFAULT_PARAMS = T.let(T.unsafe(nil), Hash)
RubyVM::INSTRUCTION_NAMES = T.let(T.unsafe(nil), Array)
RubyVM::OPTS = T.let(T.unsafe(nil), Array)
Time::RFC2822_DAY_NAME = T.let(T.unsafe(nil), Array)
Time::RFC2822_MONTH_NAME = T.let(T.unsafe(nil), Array)
URI::ABS_PATH = T.let(T.unsafe(nil), Regexp)
URI::ABS_URI = T.let(T.unsafe(nil), Regexp)
URI::ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
URI::ESCAPED = T.let(T.unsafe(nil), Regexp)
URI::FRAGMENT = T.let(T.unsafe(nil), Regexp)
URI::FTP::ABS_PATH = T.let(T.unsafe(nil), Regexp)
URI::FTP::ABS_URI = T.let(T.unsafe(nil), Regexp)
URI::FTP::ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::FTP::COMPONENT = T.let(T.unsafe(nil), Array)
URI::FTP::DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
URI::FTP::DEFAULT_PORT = T.let(T.unsafe(nil), Integer)
URI::FTP::ESCAPED = T.let(T.unsafe(nil), Regexp)
URI::FTP::FRAGMENT = T.let(T.unsafe(nil), Regexp)
URI::FTP::HOST = T.let(T.unsafe(nil), Regexp)
URI::FTP::HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
URI::FTP::OPAQUE = T.let(T.unsafe(nil), Regexp)
URI::FTP::PORT = T.let(T.unsafe(nil), Regexp)
URI::FTP::QUERY = T.let(T.unsafe(nil), Regexp)
URI::FTP::REGISTRY = T.let(T.unsafe(nil), Regexp)
URI::FTP::REL_PATH = T.let(T.unsafe(nil), Regexp)
URI::FTP::REL_URI = T.let(T.unsafe(nil), Regexp)
URI::FTP::REL_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::FTP::RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
URI::FTP::SCHEME = T.let(T.unsafe(nil), Regexp)
URI::FTP::TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::FTP::TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::FTP::TYPECODE = T.let(T.unsafe(nil), Array)
URI::FTP::TYPECODE_PREFIX = T.let(T.unsafe(nil), String)
URI::FTP::UNSAFE = T.let(T.unsafe(nil), Regexp)
URI::FTP::URI_REF = T.let(T.unsafe(nil), Regexp)
URI::FTP::USERINFO = T.let(T.unsafe(nil), Regexp)
URI::FTP::USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
URI::FTP::VERSION = T.let(T.unsafe(nil), String)
URI::FTP::VERSION_CODE = T.let(T.unsafe(nil), String)
URI::FTP::WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
URI::Generic::ABS_PATH = T.let(T.unsafe(nil), Regexp)
URI::Generic::ABS_URI = T.let(T.unsafe(nil), Regexp)
URI::Generic::ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::Generic::COMPONENT = T.let(T.unsafe(nil), Array)
URI::Generic::DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
URI::Generic::DEFAULT_PORT = T.let(T.unsafe(nil), NilClass)
URI::Generic::ESCAPED = T.let(T.unsafe(nil), Regexp)
URI::Generic::FRAGMENT = T.let(T.unsafe(nil), Regexp)
URI::Generic::HOST = T.let(T.unsafe(nil), Regexp)
URI::Generic::HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
URI::Generic::OPAQUE = T.let(T.unsafe(nil), Regexp)
URI::Generic::PORT = T.let(T.unsafe(nil), Regexp)
URI::Generic::QUERY = T.let(T.unsafe(nil), Regexp)
URI::Generic::REGISTRY = T.let(T.unsafe(nil), Regexp)
URI::Generic::REL_PATH = T.let(T.unsafe(nil), Regexp)
URI::Generic::REL_URI = T.let(T.unsafe(nil), Regexp)
URI::Generic::REL_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::Generic::RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
URI::Generic::SCHEME = T.let(T.unsafe(nil), Regexp)
URI::Generic::TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::Generic::TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::Generic::UNSAFE = T.let(T.unsafe(nil), Regexp)
URI::Generic::URI_REF = T.let(T.unsafe(nil), Regexp)
URI::Generic::USERINFO = T.let(T.unsafe(nil), Regexp)
URI::Generic::USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
URI::Generic::VERSION = T.let(T.unsafe(nil), String)
URI::Generic::VERSION_CODE = T.let(T.unsafe(nil), String)
URI::Generic::WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
URI::HOST = T.let(T.unsafe(nil), Regexp)
URI::HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
URI::HTTP::ABS_PATH = T.let(T.unsafe(nil), Regexp)
URI::HTTP::ABS_URI = T.let(T.unsafe(nil), Regexp)
URI::HTTP::ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::HTTP::COMPONENT = T.let(T.unsafe(nil), Array)
URI::HTTP::DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
URI::HTTP::DEFAULT_PORT = T.let(T.unsafe(nil), Integer)
URI::HTTP::ESCAPED = T.let(T.unsafe(nil), Regexp)
URI::HTTP::FRAGMENT = T.let(T.unsafe(nil), Regexp)
URI::HTTP::HOST = T.let(T.unsafe(nil), Regexp)
URI::HTTP::HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
URI::HTTP::OPAQUE = T.let(T.unsafe(nil), Regexp)
URI::HTTP::PORT = T.let(T.unsafe(nil), Regexp)
URI::HTTP::QUERY = T.let(T.unsafe(nil), Regexp)
URI::HTTP::REGISTRY = T.let(T.unsafe(nil), Regexp)
URI::HTTP::REL_PATH = T.let(T.unsafe(nil), Regexp)
URI::HTTP::REL_URI = T.let(T.unsafe(nil), Regexp)
URI::HTTP::REL_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::HTTP::RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
URI::HTTP::SCHEME = T.let(T.unsafe(nil), Regexp)
URI::HTTP::TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::HTTP::TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::HTTP::UNSAFE = T.let(T.unsafe(nil), Regexp)
URI::HTTP::URI_REF = T.let(T.unsafe(nil), Regexp)
URI::HTTP::USERINFO = T.let(T.unsafe(nil), Regexp)
URI::HTTP::USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
URI::HTTP::VERSION = T.let(T.unsafe(nil), String)
URI::HTTP::VERSION_CODE = T.let(T.unsafe(nil), String)
URI::HTTP::WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
URI::HTTPS::ABS_PATH = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::ABS_URI = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::COMPONENT = T.let(T.unsafe(nil), Array)
URI::HTTPS::DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
URI::HTTPS::DEFAULT_PORT = T.let(T.unsafe(nil), Integer)
URI::HTTPS::ESCAPED = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::FRAGMENT = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::HOST = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
URI::HTTPS::OPAQUE = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::PORT = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::QUERY = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::REGISTRY = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::REL_PATH = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::REL_URI = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::REL_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
URI::HTTPS::SCHEME = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::HTTPS::TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::HTTPS::UNSAFE = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::URI_REF = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::USERINFO = T.let(T.unsafe(nil), Regexp)
URI::HTTPS::USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
URI::HTTPS::VERSION = T.let(T.unsafe(nil), String)
URI::HTTPS::VERSION_CODE = T.let(T.unsafe(nil), String)
URI::HTTPS::WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
URI::LDAP::ABS_PATH = T.let(T.unsafe(nil), Regexp)
URI::LDAP::ABS_URI = T.let(T.unsafe(nil), Regexp)
URI::LDAP::ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::LDAP::COMPONENT = T.let(T.unsafe(nil), Array)
URI::LDAP::DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
URI::LDAP::DEFAULT_PORT = T.let(T.unsafe(nil), Integer)
URI::LDAP::ESCAPED = T.let(T.unsafe(nil), Regexp)
URI::LDAP::FRAGMENT = T.let(T.unsafe(nil), Regexp)
URI::LDAP::HOST = T.let(T.unsafe(nil), Regexp)
URI::LDAP::HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
URI::LDAP::OPAQUE = T.let(T.unsafe(nil), Regexp)
URI::LDAP::PORT = T.let(T.unsafe(nil), Regexp)
URI::LDAP::QUERY = T.let(T.unsafe(nil), Regexp)
URI::LDAP::REGISTRY = T.let(T.unsafe(nil), Regexp)
URI::LDAP::REL_PATH = T.let(T.unsafe(nil), Regexp)
URI::LDAP::REL_URI = T.let(T.unsafe(nil), Regexp)
URI::LDAP::REL_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::LDAP::RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
URI::LDAP::SCHEME = T.let(T.unsafe(nil), Regexp)
URI::LDAP::SCOPE = T.let(T.unsafe(nil), Array)
URI::LDAP::SCOPE_BASE = T.let(T.unsafe(nil), String)
URI::LDAP::SCOPE_ONE = T.let(T.unsafe(nil), String)
URI::LDAP::SCOPE_SUB = T.let(T.unsafe(nil), String)
URI::LDAP::TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::LDAP::TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::LDAP::UNSAFE = T.let(T.unsafe(nil), Regexp)
URI::LDAP::URI_REF = T.let(T.unsafe(nil), Regexp)
URI::LDAP::USERINFO = T.let(T.unsafe(nil), Regexp)
URI::LDAP::USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
URI::LDAP::VERSION = T.let(T.unsafe(nil), String)
URI::LDAP::VERSION_CODE = T.let(T.unsafe(nil), String)
URI::LDAP::WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
URI::LDAPS::ABS_PATH = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::ABS_URI = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::COMPONENT = T.let(T.unsafe(nil), Array)
URI::LDAPS::DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
URI::LDAPS::DEFAULT_PORT = T.let(T.unsafe(nil), Integer)
URI::LDAPS::ESCAPED = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::FRAGMENT = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::HOST = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
URI::LDAPS::OPAQUE = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::PORT = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::QUERY = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::REGISTRY = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::REL_PATH = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::REL_URI = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::REL_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
URI::LDAPS::SCHEME = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::SCOPE = T.let(T.unsafe(nil), Array)
URI::LDAPS::SCOPE_BASE = T.let(T.unsafe(nil), String)
URI::LDAPS::SCOPE_ONE = T.let(T.unsafe(nil), String)
URI::LDAPS::SCOPE_SUB = T.let(T.unsafe(nil), String)
URI::LDAPS::TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::LDAPS::TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::LDAPS::UNSAFE = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::URI_REF = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::USERINFO = T.let(T.unsafe(nil), Regexp)
URI::LDAPS::USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
URI::LDAPS::VERSION = T.let(T.unsafe(nil), String)
URI::LDAPS::VERSION_CODE = T.let(T.unsafe(nil), String)
URI::LDAPS::WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
URI::MailTo::ABS_PATH = T.let(T.unsafe(nil), Regexp)
URI::MailTo::ABS_URI = T.let(T.unsafe(nil), Regexp)
URI::MailTo::ABS_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::MailTo::COMPONENT = T.let(T.unsafe(nil), Array)
URI::MailTo::DEFAULT_PARSER = T.let(T.unsafe(nil), URI::RFC2396_Parser)
URI::MailTo::DEFAULT_PORT = T.let(T.unsafe(nil), NilClass)
URI::MailTo::EMAIL_REGEXP = T.let(T.unsafe(nil), Regexp)
URI::MailTo::ESCAPED = T.let(T.unsafe(nil), Regexp)
URI::MailTo::FRAGMENT = T.let(T.unsafe(nil), Regexp)
URI::MailTo::HEADER_REGEXP = T.let(T.unsafe(nil), Regexp)
URI::MailTo::HOST = T.let(T.unsafe(nil), Regexp)
URI::MailTo::HTML5ASCIIINCOMPAT = T.let(T.unsafe(nil), String)
URI::MailTo::OPAQUE = T.let(T.unsafe(nil), Regexp)
URI::MailTo::PORT = T.let(T.unsafe(nil), Regexp)
URI::MailTo::QUERY = T.let(T.unsafe(nil), Regexp)
URI::MailTo::REGISTRY = T.let(T.unsafe(nil), Regexp)
URI::MailTo::REL_PATH = T.let(T.unsafe(nil), Regexp)
URI::MailTo::REL_URI = T.let(T.unsafe(nil), Regexp)
URI::MailTo::REL_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::MailTo::RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
URI::MailTo::SCHEME = T.let(T.unsafe(nil), Regexp)
URI::MailTo::TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::MailTo::TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::MailTo::UNSAFE = T.let(T.unsafe(nil), Regexp)
URI::MailTo::URI_REF = T.let(T.unsafe(nil), Regexp)
URI::MailTo::USERINFO = T.let(T.unsafe(nil), Regexp)
URI::MailTo::USE_REGISTRY = T.let(T.unsafe(nil), FalseClass)
URI::MailTo::VERSION = T.let(T.unsafe(nil), String)
URI::MailTo::VERSION_CODE = T.let(T.unsafe(nil), String)
URI::MailTo::WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
URI::OPAQUE = T.let(T.unsafe(nil), Regexp)
URI::PORT = T.let(T.unsafe(nil), Regexp)
URI::QUERY = T.let(T.unsafe(nil), Regexp)
URI::REGISTRY = T.let(T.unsafe(nil), Regexp)
URI::REL_PATH = T.let(T.unsafe(nil), Regexp)
URI::REL_URI = T.let(T.unsafe(nil), Regexp)
URI::REL_URI_REF = T.let(T.unsafe(nil), Regexp)
URI::RFC2396_REGEXP::PATTERN::ABS_PATH = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::ABS_URI = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::ALNUM = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::ALPHA = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::DOMLABEL = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::ESCAPED = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::FRAGMENT = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::HEX = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::HIER_PART = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::HOST = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::HOSTNAME = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::HOSTPORT = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::IPV4ADDR = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::IPV6ADDR = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::IPV6REF = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::NET_PATH = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::OPAQUE_PART = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::PATH_SEGMENTS = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::PORT = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::QUERY = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::REG_NAME = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::REL_PATH = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::REL_SEGMENT = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::REL_URI = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::RESERVED = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::SCHEME = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::TOPLABEL = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::UNRESERVED = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::URIC = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::URIC_NO_SLASH = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::URI_REF = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::USERINFO = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::X_ABS_URI = T.let(T.unsafe(nil), String)
URI::RFC2396_REGEXP::PATTERN::X_REL_URI = T.let(T.unsafe(nil), String)
URI::RFC3986_PARSER = T.let(T.unsafe(nil), URI::RFC3986_Parser)
URI::RFC3986_Parser::RFC3986_URI = T.let(T.unsafe(nil), Regexp)
URI::SCHEME = T.let(T.unsafe(nil), Regexp)
URI::TBLDECWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::TBLENCWWWCOMP_ = T.let(T.unsafe(nil), Hash)
URI::UNSAFE = T.let(T.unsafe(nil), Regexp)
URI::URI_REF = T.let(T.unsafe(nil), Regexp)
URI::USERINFO = T.let(T.unsafe(nil), Regexp)
URI::VERSION = T.let(T.unsafe(nil), String)
URI::VERSION_CODE = T.let(T.unsafe(nil), String)
URI::WEB_ENCODINGS_ = T.let(T.unsafe(nil), Hash)
