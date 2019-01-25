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
  Errno = T.let(nil, Integer)
end

class Errno::EACCES < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EADDRINUSE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EADDRNOTAVAIL < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EADV < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EAFNOSUPPORT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EAGAIN < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EWOULDBLOCK < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EALREADY < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EBADE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EBADF < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EBADFD < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EBADMSG < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EBADR < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EBADRQC < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EBADSLT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EBFONT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EBUSY < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ECANCELED < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ECHILD < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ECHRNG < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ECOMM < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ECONNABORTED < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ECONNREFUSED < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ECONNRESET < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EDEADLK < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EDESTADDRREQ < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EDOM < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EDOTDOT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EDQUOT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EEXIST < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EFAULT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EFBIG < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EHOSTDOWN < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EHOSTUNREACH < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EHWPOISON < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EIDRM < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EILSEQ < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EINPROGRESS < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EINTR < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EINVAL < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EIO < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EISCONN < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EISDIR < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EISNAM < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EKEYEXPIRED < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EKEYREJECTED < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EKEYREVOKED < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EL2HLT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EL2NSYNC < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EL3HLT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EL3RST < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ELIBACC < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ELIBBAD < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ELIBEXEC < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ELIBMAX < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ELIBSCN < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ELNRNG < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ELOOP < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EMEDIUMTYPE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EMFILE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EMLINK < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EMSGSIZE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EMULTIHOP < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENAMETOOLONG < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENAVAIL < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENETDOWN < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENETRESET < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENETUNREACH < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENFILE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOANO < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOBUFS < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOCSI < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENODATA < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENODEV < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOENT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOEXEC < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOKEY < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOLCK < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOLINK < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOMEDIUM < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOMEM < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOMSG < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENONET < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOPKG < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOPROTOOPT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOSPC < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOSR < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOSTR < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOSYS < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOTBLK < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOTCONN < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOTDIR < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOTEMPTY < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOTNAM < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOTRECOVERABLE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOTSOCK < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOTTY < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENOTUNIQ < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ENXIO < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EOPNOTSUPP < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EOVERFLOW < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EOWNERDEAD < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EPERM < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EPFNOSUPPORT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EPIPE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EPROTO < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EPROTONOSUPPORT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EPROTOTYPE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ERANGE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EREMCHG < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EREMOTE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EREMOTEIO < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ERESTART < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ERFKILL < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EROFS < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ESHUTDOWN < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ESOCKTNOSUPPORT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ESPIPE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ESRCH < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ESRMNT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ESTALE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ESTRPIPE < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ETIME < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ETIMEDOUT < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ETOOMANYREFS < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::ETXTBSY < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EUCLEAN < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EUNATCH < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EUSERS < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EXDEV < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::EXFULL < SystemCallError
  Errno = T.let(nil, Integer)
end

class Errno::NOERROR < SystemCallError
  Errno = T.let(nil, Integer)
end

class Etc::Group < Struct
end

class Etc::Passwd < Struct
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
  Errno = T.let(nil, Integer)
end

class IO::EAGAINWaitWritable < Errno::EAGAIN
  include IO::WaitWritable
  Errno = T.let(nil, Integer)
end

class IO::EINPROGRESSWaitReadable < Errno::EINPROGRESS
  include IO::WaitReadable
  Errno = T.let(nil, Integer)
end

class IO::EINPROGRESSWaitWritable < Errno::EINPROGRESS
  include IO::WaitWritable
  Errno = T.let(nil, Integer)
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

class ObjectSpace::WeakMap < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)
end

class Process::Tms < Struct
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

class Thread::Backtrace < Object
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

class UncaughtThrowError < ArgumentError
end

module Warning
end

class ZeroDivisionError < StandardError
end
