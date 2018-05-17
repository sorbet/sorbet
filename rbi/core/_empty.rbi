# typed: true
class ArgumentError < StandardError
end

class Benchmark::Job < Object
end

class Benchmark::Report < Object
end

class Binding < Object
end

class CSV::FieldInfo < Struct
end

class CSV::MalformedCSVError < RuntimeError
end

class CSV::Row < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)
end

class CSV::Table < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)
end

class ClosedQueueError < StopIteration
end

module Comparable
end

class Data < Object
end

class Date::Infinity < Numeric
end

class Delegator < BasicObject
end

class DidYouMean::ClassNameChecker < Object
end

module DidYouMean::Correctable
end

class DidYouMean::Formatter < Object
end

module DidYouMean::Jaro
end

module DidYouMean::Levenshtein
end

module DidYouMean::NameErrorCheckers
end

class DidYouMean::NullChecker < Object
end

class DidYouMean::SpellChecker < Object
end

class EOFError < IOError
end

class Encoding::CompatibilityError < EncodingError
end

class Encoding::ConverterNotFoundError < EncodingError
end

class Encoding::InvalidByteSequenceError < EncodingError
end

class Encoding::UndefinedConversionError < EncodingError
end

class EncodingError < StandardError
end

class Enumerator::Generator < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)
end

class Enumerator::Lazy < Enumerator
  extend T::Generic
  Elem = type_member(:out)
end

class Enumerator::Yielder < Object
end

module Errno
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

class Etc::Group < Struct
end

class Etc::Passwd < Struct
end

class FalseClass < Object
end

class Fiber < Object
end

class FiberError < StandardError
end

module FileTest
end

module FileUtils::LowMethods
end

module FileUtils::StreamUtils_
end

class FloatDomainError < RangeError
end

module GC::Profiler
end

class Gem::BasicSpecification < Object
end

class Gem::CommandLineError < Gem::Exception
end

class Gem::ConflictError < Gem::LoadError
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

class Gem::Exception < RuntimeError
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

  extend T::Generic
  Elem = type_member(:out)
end

class Gem::LoadError < LoadError
end

class Gem::MissingSpecError < Gem::LoadError
end

class Gem::MissingSpecVersionError < Gem::MissingSpecError
end

class Gem::OperationNotSupportedError < Gem::Exception
end

class Gem::PathSupport < Object
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

class Gem::Requirement::BadRequirementError < ArgumentError
end

class Gem::RubyVersionMismatch < Gem::Exception
end

class Gem::SourceFetchProblem < Gem::ErrorReason
end

class Gem::SpecificGemNotFoundException < Gem::GemNotFoundException
end

class Gem::SystemExitException < SystemExit
end

class Gem::UnsatisfiableDependencyError < Gem::DependencyError
end

class Gem::VerificationError < Gem::Exception
end

class IO::EAGAINWaitReadable < Errno::EAGAIN
  include IO::WaitReadable
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

module IO::WaitReadable
end

module IO::WaitWritable
end

class IOError < StandardError
end

class IndexError < StandardError
end

class Interrupt < SignalException
end

class KeyError < IndexError
end

class LoadError < ScriptError
end

class LocalJumpError < StandardError
end

class Math::DomainError < StandardError
end

class Method < Object
end

class Monitor < Object
  include MonitorMixin
end

module MonitorMixin
end

class MonitorMixin::ConditionVariable < Object
end

class MonitorMixin::ConditionVariable::Timeout < Exception
end

class NameError < StandardError
end

class NoMemoryError < Exception
end

class NoMethodError < NameError
end

class NotImplementedError < ScriptError
end

class Object < BasicObject
  include Kernel
end

module ObjectSpace
end

class ObjectSpace::WeakMap < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)
end

class Process::Tms < Struct
end

module Random::Formatter
end

class RangeError < StandardError
end

class RegexpError < StandardError
end

class RubyVM::InstructionSequence < Object
end

class RuntimeError < StandardError
end

class ScriptError < Exception
end

class SecurityError < Exception
end

class SignalException < Exception
end

class SimpleDelegator < Delegator
end

module SingleForwardable
end

class SortedSet < Set
  extend T::Generic
  Elem = type_member(:out)
end

class StandardError < Exception
end

class StopIteration < IndexError
end

class StringIO < Data
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: String)
end

class StringScanner::Error < StandardError
end

class Struct < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)
end

class SyntaxError < ScriptError
end

class SystemCallError < StandardError
end

class SystemExit < Exception
end

class SystemStackError < Exception
end

class Thread < Object
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

class TracePoint < Object
end

class TrueClass < Object
end

class TypeError < StandardError
end

class URI::BadURIError < URI::Error
end

class URI::Error < StandardError
end

module URI::Escape
end

class URI::InvalidComponentError < URI::Error
end

class URI::InvalidURIError < URI::Error
end

class URI::RFC2396_Parser < Object
  include URI::RFC2396_REGEXP
end

module URI::RFC2396_REGEXP
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
