class BasicObject
end
module Kernel
end
class Object < BasicObject
  include Kernel
end
class Data < Object
end
class Addrinfo < Data
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
class File < IO
end
module File::Constants
end
class IO < Object
  include File::Constants
  include Enumerable
end
class BasicSocket < IO
end
class Binding < Object
end
module Bundler
end
class Bundler::BundlerError < StandardError
end
class Bundler::APIResponseMismatchError < Bundler::BundlerError
end
class Bundler::CurrentRuby < Object
end
class Bundler::CyclicDependencyError < Bundler::BundlerError
end
module Bundler::GemHelpers
end
class Bundler::Definition < Object
  include Bundler::GemHelpers
end
class Bundler::DepProxy < Object
end
module Gem
end
class Gem::Dependency < Object
end
class Bundler::Dependency < Gem::Dependency
end
class Bundler::DeprecatedError < Bundler::BundlerError
end
module Bundler::RubyDsl
end
class Bundler::Dsl < Object
  include Bundler::RubyDsl
end
class Bundler::GemfileError < Bundler::BundlerError
end
class Bundler::Dsl::DSLError < Bundler::GemfileError
end
module Bundler::MatchPlatform
end
class Gem::BasicSpecification < Object
end
class Gem::Specification < Gem::BasicSpecification
  include Bundler::MatchPlatform
  include Bundler::GemHelpers
end
class Bundler::EndpointSpecification < Gem::Specification
end
class Bundler::EnvironmentPreserver < Object
end
class Struct < Object
  include Enumerable
end
class Bundler::GemHelpers::PlatformMatch < Struct
end
class Bundler::GemNotFound < Bundler::BundlerError
end
class Bundler::GemRequireError < Bundler::BundlerError
end
class Bundler::GemVersionPromoter < Object
end
class Bundler::GemfileEvalError < Bundler::GemfileError
end
class Bundler::GemfileLockNotFound < Bundler::BundlerError
end
class Bundler::GemfileNotFound < Bundler::BundlerError
end
class Bundler::GemspecError < Bundler::BundlerError
end
class Bundler::GenericSystemCallError < Bundler::BundlerError
end
class Bundler::GitError < Bundler::BundlerError
end
class Bundler::HTTPError < Bundler::BundlerError
end
class Bundler::Index < Object
  include Enumerable
end
class Bundler::InstallError < Bundler::BundlerError
end
class Bundler::InstallHookError < Bundler::BundlerError
end
class Bundler::InvalidOption < Bundler::BundlerError
end
class Bundler::LazySpecification < Object
  include Bundler::MatchPlatform
  include Bundler::GemHelpers
end
module Comparable
end
class Bundler::LazySpecification::Identifier < Struct
  include Comparable
end
class Bundler::LockfileError < Bundler::BundlerError
end
class Bundler::LockfileParser < Object
end
class Bundler::MarshalError < StandardError
end
class Bundler::PermissionError < Bundler::BundlerError
end
class Bundler::NoSpaceOnDeviceError < Bundler::PermissionError
end
class Bundler::OperationNotSupportedError < Bundler::PermissionError
end
class Bundler::PathError < Bundler::BundlerError
end
module Bundler::Plugin
end
class Bundler::Plugin::API < Object
end
class Bundler::PluginError < Bundler::BundlerError
end
class Bundler::Plugin::MalformattedPlugin < Bundler::PluginError
end
class Bundler::Plugin::UndefinedCommandError < Bundler::PluginError
end
class Bundler::Plugin::UnknownSourceError < Bundler::PluginError
end
class Bundler::ProductionError < Bundler::BundlerError
end
class Bundler::RemoteSpecification < Object
  include Comparable
  include Bundler::MatchPlatform
  include Bundler::GemHelpers
end
class Bundler::RubyVersion < Object
end
class Bundler::RubyVersionMismatch < Bundler::BundlerError
end
class Bundler::RubygemsIntegration < Object
end
class Bundler::RubygemsIntegration::Modern < Bundler::RubygemsIntegration
end
class Bundler::RubygemsIntegration::AlmostModern < Bundler::RubygemsIntegration::Modern
end
class Bundler::RubygemsIntegration::Legacy < Bundler::RubygemsIntegration
end
class Bundler::RubygemsIntegration::Ancient < Bundler::RubygemsIntegration::Legacy
end
class Bundler::RubygemsIntegration::Future < Bundler::RubygemsIntegration
end
class Bundler::RubygemsIntegration::MoreFuture < Bundler::RubygemsIntegration::Future
end
class Bundler::RubygemsIntegration::MoreModern < Bundler::RubygemsIntegration::Modern
end
class Bundler::RubygemsIntegration::Transitional < Bundler::RubygemsIntegration::Legacy
end
module Bundler::SharedHelpers
end
class Bundler::Runtime < Object
  include Bundler::SharedHelpers
end
class Bundler::SecurityError < Bundler::BundlerError
end
class Bundler::Settings < Object
end
class Bundler::Source < Object
end
class Bundler::Source::Path < Bundler::Source
end
class Bundler::Source::Gemspec < Bundler::Source::Path
end
class Bundler::Source::Git < Bundler::Source::Path
end
class Bundler::Source::Git::GitCommandError < Bundler::GitError
end
class Bundler::Source::Git::GitNotAllowedError < Bundler::GitError
end
class Bundler::Source::Git::GitNotInstalledError < Bundler::GitError
end
class Bundler::Source::Git::GitProxy < Object
end
class Bundler::Source::Git::MissingGitRevisionError < Bundler::GitError
end
class Bundler::Source::Rubygems < Bundler::Source
end
class Bundler::SourceList < Object
end
module TSort
end
class Bundler::SpecSet < Object
  include TSort
  include Enumerable
end
class Bundler::StubSpecification < Bundler::RemoteSpecification
end
class Bundler::SudoNotPermittedError < Bundler::BundlerError
end
class Bundler::TemporaryResourceError < Bundler::PermissionError
end
class Bundler::ThreadCreationError < Bundler::BundlerError
end
module Bundler::UI
end
class Gem::StreamUI < Object
end
class Gem::SilentUI < Gem::StreamUI
end
class Bundler::UI::RGProxy < Gem::SilentUI
end
class Bundler::UI::Silent < Object
end
module Bundler::URICredentialsFilter
end
class Bundler::VersionConflict < Bundler::BundlerError
end
class Bundler::VirtualProtocolError < Bundler::BundlerError
end
module Bundler::YAMLSerializer
end
class Bundler::YamlSyntaxError < Bundler::BundlerError
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
class Numeric < Object
  include Comparable
end
class Complex < Numeric
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
module Digest
end
module Digest::Instance
end
class Digest::Class < Object
  include Digest::Instance
end
class Digest::Base < Digest::Class
end
class Digest::SHA1 < Digest::Base
end
class Dir < Object
  include Enumerable
end
module Dir::Tmpname
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
class Errno::EAFNOSUPPORT < SystemCallError
end
class Errno::EAGAIN < SystemCallError
end
class Errno::EALREADY < SystemCallError
end
class Errno::EAUTH < SystemCallError
end
class Errno::EBADF < SystemCallError
end
class Errno::EBADMSG < SystemCallError
end
class Errno::EBADRPC < SystemCallError
end
class Errno::EBUSY < SystemCallError
end
class Errno::ECANCELED < SystemCallError
end
class Errno::ECHILD < SystemCallError
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
class Errno::EDQUOT < SystemCallError
end
class Errno::EEXIST < SystemCallError
end
class Errno::EFAULT < SystemCallError
end
class Errno::EFBIG < SystemCallError
end
class Errno::EFTYPE < SystemCallError
end
class Errno::EHOSTDOWN < SystemCallError
end
class Errno::EHOSTUNREACH < SystemCallError
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
class Errno::ELOOP < SystemCallError
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
class Errno::ENEEDAUTH < SystemCallError
end
class Errno::ENETDOWN < SystemCallError
end
class Errno::ENETRESET < SystemCallError
end
class Errno::ENETUNREACH < SystemCallError
end
class Errno::ENFILE < SystemCallError
end
class Errno::ENOATTR < SystemCallError
end
class Errno::ENOBUFS < SystemCallError
end
class Errno::ENODATA < SystemCallError
end
class Errno::ENODEV < SystemCallError
end
class Errno::ENOENT < SystemCallError
end
class Errno::ENOEXEC < SystemCallError
end
class Errno::ENOLCK < SystemCallError
end
class Errno::ENOLINK < SystemCallError
end
class Errno::ENOMEM < SystemCallError
end
class Errno::ENOMSG < SystemCallError
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
class Errno::ENOTRECOVERABLE < SystemCallError
end
class Errno::ENOTSOCK < SystemCallError
end
class Errno::ENOTSUP < SystemCallError
end
class Errno::ENOTTY < SystemCallError
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
class Errno::EPROCLIM < SystemCallError
end
class Errno::EPROCUNAVAIL < SystemCallError
end
class Errno::EPROGMISMATCH < SystemCallError
end
class Errno::EPROGUNAVAIL < SystemCallError
end
class Errno::EPROTO < SystemCallError
end
class Errno::EPROTONOSUPPORT < SystemCallError
end
class Errno::EPROTOTYPE < SystemCallError
end
class Errno::ERANGE < SystemCallError
end
class Errno::EREMOTE < SystemCallError
end
class Errno::EROFS < SystemCallError
end
class Errno::ERPCMISMATCH < SystemCallError
end
class Errno::ESHUTDOWN < SystemCallError
end
class Errno::ESOCKTNOSUPPORT < SystemCallError
end
class Errno::ESPIPE < SystemCallError
end
class Errno::ESRCH < SystemCallError
end
class Errno::ESTALE < SystemCallError
end
class Errno::ETIME < SystemCallError
end
class Errno::ETIMEDOUT < SystemCallError
end
class Errno::ETOOMANYREFS < SystemCallError
end
class Errno::ETXTBSY < SystemCallError
end
class Errno::EUSERS < SystemCallError
end
class Errno::EXDEV < SystemCallError
end
class Errno::NOERROR < SystemCallError
end
module Etc
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
class File::Stat < Object
  include Comparable
end
module FileTest
end
module FileUtils::StreamUtils_
end
module FileUtils
end
module FileUtils::LowMethods
end
module FileUtils::DryRun
end
class FileUtils::Entry_ < Object
  include FileUtils::StreamUtils_
end
module FileUtils::NoWrite
end
module FileUtils::Verbose
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
class RuntimeError < StandardError
end
class Gem::Exception < RuntimeError
end
class Gem::CommandLineError < Gem::Exception
end
module Gem::DefaultUserInteraction
end
module Gem::UserInteraction
end
class Gem::ConfigFile < Object
  include Gem::UserInteraction
  include Gem::DefaultUserInteraction
end
class ScriptError < Exception
end
class LoadError < ScriptError
end
class Gem::LoadError < LoadError
end
class Gem::ConflictError < Gem::LoadError
end
class Gem::ConsoleUI < Gem::StreamUI
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
module Gem::Ext
end
class Gem::Ext::Builder < Object
  include Gem::UserInteraction
  include Gem::DefaultUserInteraction
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
class Gem::Source < Object
  include Comparable
end
class Gem::Source::Git < Gem::Source
end
class Gem::Source::Installed < Gem::Source
end
class Gem::Source::Local < Gem::Source
end
class Gem::Source::Lock < Gem::Source
end
class Gem::Source::SpecificFile < Gem::Source
end
class Gem::Source::Vendor < Gem::Source::Installed
end
class Gem::SourceFetchProblem < Gem::ErrorReason
end
class Gem::SpecificGemNotFoundException < Gem::GemNotFoundException
end
class Gem::StreamUI::SilentDownloadReporter < Object
end
class Gem::StreamUI::SilentProgressReporter < Object
end
class Gem::StreamUI::SimpleProgressReporter < Object
  include Gem::DefaultUserInteraction
end
class Gem::StreamUI::VerboseDownloadReporter < Object
end
class Gem::StreamUI::VerboseProgressReporter < Object
  include Gem::DefaultUserInteraction
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
module Gem::Util
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
class IPSocket < BasicSocket
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
module Open3
end
module Opus::Dev
end
module Opus::Dev::BundleManager
end
class Pathname < Object
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
module Shellwords
end
module Signal
end
module SingleForwardable
end
class Socket < BasicSocket
end
class Socket::AncillaryData < Object
end
module Socket::Constants
end
class Socket::Ifaddr < Data
end
class Socket::Option < Object
end
class Socket::UDPSource < Object
end
class SocketError < StandardError
end
class SortedSet < Set
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
class TCPSocket < IPSocket
end
class TCPServer < TCPSocket
end
class TSort::Cyclic < StandardError
end
class Tempfile::Remover < Object
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
class UDPSocket < IPSocket
end
class UNIXSocket < BasicSocket
end
class UNIXServer < UNIXSocket
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
class Abbrev
  standard_method(
    {
      words: Opus::Types.any(Opus::Types.array_of(String)),
    },
    returns: Opus::Types.any(Opus::Types.hash_of(keys: String, values: String))
  )
  def self.abbrev(words); end
end

class Array
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end
end

class Array
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Array
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def length(); end
end

class Base64
  standard_method(
    {
      str: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.decode64(str); end
end

class Base64
  standard_method(
    {
      bin: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.encode64(bin); end
end

class Base64
  standard_method(
    {
      str: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.strict_decode64(str); end
end

class Base64
  standard_method(
    {
      bin: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.strict_encode64(bin); end
end

class Base64
  standard_method(
    {
      str: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.urlsafe_decode64(str); end
end

class Base64
  standard_method(
    {
      bin: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.urlsafe_encode64(bin); end
end

class BasicObject
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(other); end
end

class BasicObject
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(other); end
end

class BasicObject
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def !(); end
end

class BasicObject
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def !=(other); end
end

class BasicObject
  standard_method(
    {
      _: Opus::Types.any(String),
      filename: Opus::Types.any(String, NilClass),
      lineno: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def instance_eval(_, filename, lineno); end
end

class Benchmark
  standard_method(
    {
      width: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Benchmark::Tms))
  )
  def self.bmbm(width); end
end

class Benchmark
  standard_method(
    {
      label: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(Benchmark::Tms)
  )
  def self.measure(label); end
end

class Benchmark
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.realtime(); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def %(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def +(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def -(_); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def -@(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def +@(); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def *(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def **(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def /(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Object)
  )
  def <=>(_); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def abs(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def abs2(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ceil(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def conj(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def conjugate(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def denominator(); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer)
  )
  def div(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]))
  )
  def divmod(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def fdiv(_); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def finite?(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def floor(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imag(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imaginary(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(NilClass, Integer)
  )
  def infinite?(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def magnitude(); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def modulo(_); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def nan?(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def numerator(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase(); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def quo(_); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def real(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass)
  )
  def real?(); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, BigDecimal)
  )
  def round(_); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_i(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_int(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c(); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, Rational)
  )
  def truncate(_); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.coerce([Integer, Integer]))
  )
  def precs(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.coerce([Integer, String, Integer, Integer]))
  )
  def split(); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def remainder(_); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def fix(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def frac(); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def power(_); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Object)
  )
  def nonzero?(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def exponent(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def sign(); end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def _dump(); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def sqrt(_); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      _1: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def add(_, _1); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      _1: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def sub(_, _1); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      _1: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def mult(_, _1); end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Opus::Types.coerce([BigDecimal, BigDecimal]))
  )
  def coerce(_); end
end

class BigMath
  standard_method(
    {
      _: Opus::Types.any(Integer),
      _1: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def self.exp(_, _1); end
end

class BigMath
  standard_method(
    {
      _: Opus::Types.any(Integer),
      _1: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def self.log(_, _1); end
end

class BigMath
  standard_method(
    {
      prec: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def E(prec); end
end

class BigMath
  standard_method(
    {
      prec: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def PI(prec); end
end

class BigMath
  standard_method(
    {
      x: Opus::Types.any(Integer),
      prec: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def atan(x, prec); end
end

class BigMath
  standard_method(
    {
      x: Opus::Types.any(Integer),
      prec: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def cos(x, prec); end
end

class BigMath
  standard_method(
    {
      x: Opus::Types.any(Integer),
      prec: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def sin(x, prec); end
end

class BigMath
  standard_method(
    {
      x: Opus::Types.any(Integer),
      prec: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def sqrt(x, prec); end
end

class Class
  standard_method(
    {},
    returns: Opus::Types.any(BasicObject)
  )
  def allocate(); end
end

class Class
  standard_method(
    {
      _: Opus::Types.any(Class),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def inherited(_); end
end

class Class
  standard_method(
    {},
    returns: Opus::Types.any(Class, NilClass)
  )
  def superclass(); end
end

class Class
  standard_method(
    {
      _: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def instance_methods(_); end
end

class Class
  standard_method(
    {},
    returns: Opus::Types.any(Class)
  )
  def class(); end
end

class Class
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def name(); end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex)
  )
  def *(_); end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex)
  )
  def **(_); end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex)
  )
  def +(_); end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex)
  )
  def -(_); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def -@(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def +@(); end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex)
  )
  def /(_); end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def abs(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def abs2(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def angle(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def arg(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def conj(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def conjugate(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def denominator(); end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(_); end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex)
  )
  def fdiv(_); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def imag(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def imaginary(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def magnitude(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def numerator(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def phase(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]))
  )
  def polar(); end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex, BigDecimal)
  )
  def quo(_); end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational)
  )
  def rationalize(_); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def real(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(FalseClass)
  )
  def real?(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]))
  )
  def rect(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]))
  )
  def rectangular(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_i(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Complex, Complex]))
  )
  def coerce(_); end
end

class Coverage
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def self.start(); end
end

class Date
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def strftime(_); end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.chroot(_); end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.delete(_); end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(Encoding, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def self.entries(_, _1); end
end

class Dir
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.exist?(file); end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def self.getwd(); end
end

class Dir
  standard_method(
    {
      pattern: Opus::Types.any(String, Opus::Types.array_of(String)),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.glob(pattern, flags); end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.home(_); end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.mkdir(_, _1); end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def self.pwd(); end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.rmdir(_); end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.unlink(_); end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def close(); end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def fileno(); end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def path(); end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def pos(); end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def pos=(_); end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def read(); end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def tell(); end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def to_path(); end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.hash_of(keys: String, values: String))
  )
  def self.aliases(); end
end

class Encoding
  standard_method(
    {
      obj1: Opus::Types.any(BasicObject),
      obj2: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Encoding, NilClass)
  )
  def self.compatible?(obj1, obj2); end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(Encoding)
  )
  def self.default_external(); end
end

class Encoding
  standard_method(
    {
      _: Opus::Types.any(String, Encoding),
    },
    returns: Opus::Types.any(String, Encoding)
  )
  def self.default_external=(_); end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(Encoding)
  )
  def self.default_internal(); end
end

class Encoding
  standard_method(
    {
      _: Opus::Types.any(String, Encoding),
    },
    returns: Opus::Types.any(String, NilClass, Encoding)
  )
  def self.default_internal=(_); end
end

class Encoding
  standard_method(
    {
      _: Opus::Types.any(String, Encoding),
    },
    returns: Opus::Types.any(Encoding)
  )
  def self.find(_); end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Encoding))
  )
  def self.list(); end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def self.name_list(); end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ascii_compatible?(); end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def dummy?(); end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def name(); end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def names(); end
end

class Encoding
  standard_method(
    {
      name: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Encoding)
  )
  def replicate(name); end
end

class Enumerable
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def all?(); end
end

class Enumerable
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def any?(); end
end

class Enumerable
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Integer)
  )
  def count(_); end
end

class Enumerable
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(_); end
end

class Enumerable
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def none?(); end
end

class Enumerable
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def one?(); end
end

class Enumerator
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Enumerator
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, NilClass)
  )
  def size(); end
end

class Exception
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end
end

class Exception
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def backtrace(); end
end

class Exception
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Thread::Backtrace::Location))
  )
  def backtrace_locations(); end
end

class Exception
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def cause(); end
end

class Exception
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(Exception)
  )
  def exception(_); end
end

class Exception
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Exception
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def message(); end
end

class Exception
  standard_method(
    {
      _: Opus::Types.any(String, Opus::Types.array_of(String)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def set_backtrace(_); end
end

class Exception
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
      dir: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.absolute_path(file, dir); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
      suffix: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.basename(file, suffix); end
end

class File
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(Integer),
      _2: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(String)
  )
  def self.binread(_, _1, _2); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.blockdev?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.chardev?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.directory?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.dirname(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.executable?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.executable_real?(file); end
end

class File
  standard_method(
    {
      path: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.extname(path); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.file?(file); end
end

class File
  standard_method(
    {
      pattern: Opus::Types.any(String),
      path: Opus::Types.any(String),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.fnmatch(pattern, path, flags); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.ftype(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.grpowned?(file); end
end

class File
  standard_method(
    {
      file_1: Opus::Types.any(String, IO),
      file_2: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.identical?(file_1, file_2); end
end

class File
  standard_method(
    {
      old: Opus::Types.any(String),
      new: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.link(old, new); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(File::Stat)
  )
  def self.lstat(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.owned?(file); end
end

class File
  standard_method(
    {
      path: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.path(path); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.pipe?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.readable?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.readable_real?(file); end
end

class File
  standard_method(
    {
      link: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.readlink(link); end
end

class File
  standard_method(
    {
      pathname: Opus::Types.any(String),
      dir: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.realpath(pathname, dir); end
end

class File
  standard_method(
    {
      old: Opus::Types.any(String),
      new: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.rename(old, new); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.setgid?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.setuid?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.size(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.size?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.socket?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Opus::Types.coerce([String, String]))
  )
  def self.split(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.sticky?(file); end
end

class File
  standard_method(
    {
      old: Opus::Types.any(String),
      new: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.symlink(old, new); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.symlink?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.truncate(file, _); end
end

class File
  standard_method(
    {
      _: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.umask(_); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.world_readable?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.world_writable?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.writable?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.writable_real?(file); end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.zero?(file); end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def atime(); end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def birthtime(); end
end

class File
  standard_method(
    {
      mode: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def chmod(mode); end
end

class File
  standard_method(
    {
      owner: Opus::Types.any(Integer),
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def chown(owner, group); end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def ctime(); end
end

class File
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, TrueClass, FalseClass)
  )
  def flock(_); end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(File::Stat)
  )
  def lstat(); end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def mtime(); end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def path(); end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size(); end
end

class File
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def truncate(_); end
end

class File::Stat
  standard_method(
    {
      other: Opus::Types.any(File::Stat),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(other); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def atime(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def birthtime(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def blksize(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def blockdev?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def blocks(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def chardev?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def ctime(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def dev(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def dev_major(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def dev_minor(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def directory?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def executable?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def executable_real?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def file?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def ftype(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def gid(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def grpowned?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ino(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def mode(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def mtime(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def nlink(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def owned?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def rdev(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def rdev_major(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def rdev_minor(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def readable?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def readable_real?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def setgid?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def setuid?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def socket?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sticky?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def symlink?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def uid(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def world_readable?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def world_writable?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def writable?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def writable_real?(); end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end
end

class FileUtils
  standard_method(
    {
      src: Opus::Types.any(String, Pathname),
      dest: Opus::Types.any(String, Pathname),
      preserve: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.any(Symbol), values: Opus::Types.any(TrueClass, FalseClass)), NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def self.cp_r(src, dest, preserve); end
end

class FileUtils
  standard_method(
    {
      list: Opus::Types.any(String, Pathname),
      mode: Opus::Types.any(Opus::Types.hash_of(keys: Opus::Types.any(Symbol), values: Opus::Types.any(TrueClass, FalseClass)), NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def self.mkdir_p(list, mode); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float, BigDecimal)
  )
  def %(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def *(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, Integer, Rational, BigDecimal, Complex)
  )
  def **(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def +(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def -(_); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def -@(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def +@(); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def /(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Object)
  )
  def <=>(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=(_); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def abs(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def abs2(); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer)
  )
  def div(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]))
  )
  def divmod(_); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ceil(); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Float, Float]))
  )
  def coerce(_); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def denominator(); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def fdiv(_); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def finite?(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def floor(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Object)
  )
  def infinite?(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def magnitude(); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float, BigDecimal)
  )
  def modulo(_); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def nan?(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def next_float(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def numerator(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def prev_float(); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def quo(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational)
  )
  def rationalize(_); end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(_); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_i(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_int(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def truncate(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def conj(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def conjugate(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imag(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imaginary(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def real(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass)
  )
  def real?(); end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c(); end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def self.binary_mode(); end
end

class Gem
  standard_method(
    {
      install_dir: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.bindir(install_dir); end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(Hash)
  )
  def self.clear_default_specs(); end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def self.clear_paths(); end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def self.config_file(); end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(Gem::ConfigFile)
  )
  def self.configuration(); end
end

class Gem
  standard_method(
    {
      config: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def self.configuration=(config); end
end

class Gem
  standard_method(
    {
      gem_name: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def self.datadir(gem_name); end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_bindir(); end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_cert_path(); end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_dir(); end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_exec_format(); end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_key_path(); end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_path(); end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.default_rubygems_dirs(); end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.default_sources(); end
end

class Hash
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def compare_by_identity?(); end
end

class Hash
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end
end

class Hash
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class Hash
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Hash
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def length(); end
end

class Hash
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size(); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def %(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def &(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def *(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def **(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def +(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def -(_); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def -@(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def +@(); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def /(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def <<(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Object)
  )
  def <=>(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def >>(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Rational, Float, BigDecimal),
    },
    returns: Opus::Types.any(Integer)
  )
  def [](_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def ^(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def |(_); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ~(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def abs(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def bit_length(); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer)
  )
  def div(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]))
  )
  def divmod(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def fdiv(_); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def magnitude(); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def modulo(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def quo(_); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def abs2(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg(); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ceil(); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Encoding),
    },
    returns: Opus::Types.any(String)
  )
  def chr(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]))
  )
  def coerce(_); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def conj(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def conjugate(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def denominator(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def even?(); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def gcd(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Integer, Integer]))
  )
  def gcdlcm(_); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def floor(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imag(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imaginary(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass)
  )
  def integer?(); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def lcm(_); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def next(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def numerator(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def odd?(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ord(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def pred(); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational)
  )
  def rationalize(_); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def real(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass)
  )
  def real?(); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def remainder(_); end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(_); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def succ(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_i(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_int(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def truncate(); end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(Symbol),
      offset: Opus::Types.any(Integer, NilClass),
      len: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(NilClass)
  )
  def advise(_, offset, len); end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def autoclose=(_); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def autoclose?(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def binmode?(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def close(); end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def close_on_exec=(_); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def close_on_exec?(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def close_read(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def close_write(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def closed?(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eof(); end
end

class IO
  standard_method(
    {
      integer_cmd: Opus::Types.any(Integer),
      arg: Opus::Types.any(String, Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def fcntl(integer_cmd, arg); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def fdatasync(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def fileno(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def fsync(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def getbyte(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def getc(); end
end

class IO
  standard_method(
    {
      sep: Opus::Types.any(String, NilClass),
      limit: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def gets(sep, limit); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Encoding)
  )
  def internal_encoding(); end
end

class IO
  standard_method(
    {
      integer_cmd: Opus::Types.any(Integer),
      arg: Opus::Types.any(String, Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def ioctl(integer_cmd, arg); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def isatty(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def lineno(); end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def lineno=(_); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def pid(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def pos(); end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def pos=(_); end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(Numeric, String),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def putc(_); end
end

class IO
  standard_method(
    {
      length: Opus::Types.any(Integer, NilClass),
      outbuf: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def read(length, outbuf); end
end

class IO
  standard_method(
    {
      len: Opus::Types.any(Integer),
      buf: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def read_nonblock(len, buf); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def readbyte(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def readchar(); end
end

class IO
  standard_method(
    {
      sep: Opus::Types.any(String, NilClass),
      limit: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def readline(sep, limit); end
end

class IO
  standard_method(
    {
      sep: Opus::Types.any(String, NilClass),
      limit: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def readlines(sep, limit); end
end

class IO
  standard_method(
    {
      maxlen: Opus::Types.any(Integer),
      outbuf: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def readpartial(maxlen, outbuf); end
end

class IO
  standard_method(
    {
      other_IO: Opus::Types.any(IO),
      path: Opus::Types.any(String),
      mode_str: Opus::Types.any(String),
    },
    returns: Opus::Types.any(IO)
  )
  def reopen(other_IO, path, mode_str); end
end

class IO
  standard_method(
    {
      amount: Opus::Types.any(Integer),
      whence: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def seek(amount, whence); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(File::Stat)
  )
  def stat(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sync(); end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sync=(_); end
end

class IO
  standard_method(
    {
      maxlen: Opus::Types.any(Integer),
      outbuf: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def sysread(maxlen, outbuf); end
end

class IO
  standard_method(
    {
      amount: Opus::Types.any(Integer),
      whence: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def sysseek(amount, whence); end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def syswrite(_); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def tell(); end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def tty?(); end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(String, Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def ungetbyte(_); end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(NilClass)
  )
  def ungetc(_); end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def write(_); end
end

class IO
  standard_method(
    {
      name: Opus::Types.any(String),
      length: Opus::Types.any(Integer, NilClass),
      offset: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.binread(name, length, offset); end
end

class IO
  standard_method(
    {
      src: Opus::Types.any(String, IO),
      dst: Opus::Types.any(String, IO),
      copy_length: Opus::Types.any(Integer, NilClass),
      src_offset: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.copy_stream(src, dst, copy_length, src_offset); end
end

class IO
  standard_method(
    {
      read_array: Opus::Types.any(Opus::Types.array_of(IO)),
      write_array: Opus::Types.any(Opus::Types.array_of(IO), NilClass),
      error_array: Opus::Types.any(Opus::Types.array_of(IO), NilClass),
      timeout: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(IO), NilClass)
  )
  def self.select(read_array, write_array, error_array, timeout); end
end

class IO
  standard_method(
    {
      path: Opus::Types.any(String),
      mode: Opus::Types.any(String, NilClass),
      perm: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.sysopen(path, mode, perm); end
end

class Kernel
  standard_method(
    {
      x: Opus::Types.any(Numeric, String),
      y: Opus::Types.any(Numeric),
    },
    returns: Opus::Types.any(Complex)
  )
  def self.Complex(x, y); end
end

class Kernel
  standard_method(
    {
      x: Opus::Types.any(Numeric),
    },
    returns: Opus::Types.any(Float)
  )
  def self.Float(x); end
end

class Kernel
  standard_method(
    {
      arg: Opus::Types.any(Numeric, String),
      base: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.Integer(arg, base); end
end

class Kernel
  standard_method(
    {
      x: Opus::Types.any(Numeric, String),
      y: Opus::Types.any(Numeric),
    },
    returns: Opus::Types.any(Rational)
  )
  def self.Rational(x, y); end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(Symbol, NilClass)
  )
  def self.__callee__(); end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.__dir__(); end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(Symbol, NilClass)
  )
  def self.__method__(); end
end

class Kernel
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.`(_); end
end

class Kernel
  standard_method(
    {
      msg: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.abort(msg); end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(Proc)
  )
  def self.at_exit(); end
end

class Kernel
  standard_method(
    {
      module: Opus::Types.any(String, Symbol),
      filename: Opus::Types.any(String),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.autoload(module, filename); end
end

class Kernel
  standard_method(
    {
      name: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def self.autoload?(name); end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(Binding)
  )
  def self.binding(); end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.block_given?(); end
end

class Kernel
  standard_method(
    {
      start: Opus::Types.any(Integer, NilClass),
      length: Opus::Types.any(Integer, NilClass),
      _: Opus::Types.any(Range),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.caller(start, length, _); end
end

class Kernel
  standard_method(
    {
      start: Opus::Types.any(Integer, NilClass),
      length: Opus::Types.any(Integer, NilClass),
      _: Opus::Types.any(Range),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.caller_locations(start, length, _); end
end

class Kernel
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(Binding, NilClass),
      filename: Opus::Types.any(String, NilClass),
      lineno: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def self.eval(_, _1, filename, lineno); end
end

class Kernel
  standard_method(
    {
      status: Opus::Types.any(Integer, TrueClass, FalseClass),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.exit(status); end
end

class Kernel
  standard_method(
    {
      status: Opus::Types.any(Integer, TrueClass, FalseClass),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.exit!(status); end
end

class Kernel
  standard_method(
    {
      _: Opus::Types.any(String, Class),
      _1: Opus::Types.any(Opus::Types.array_of(String), String),
      _2: Opus::Types.any(Opus::Types.array_of(String)),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.fail(_, _1, _2); end
end

class Kernel
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.gets(_, _1); end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def self.global_variables(); end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.iterator?(); end
end

class Kernel
  standard_method(
    {
      filename: Opus::Types.any(String),
      _: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.load(filename, _); end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def self.local_variables(); end
end

class Kernel
  standard_method(
    {
      name: Opus::Types.any(String),
      rest: Opus::Types.any(String, Integer, NilClass),
      block: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(IO, NilClass)
  )
  def self.open(name, rest, block); end
end

class Kernel
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.putc(_); end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def self.raise(); end
end

class Kernel
  standard_method(
    {
      max: Opus::Types.any(Integer, Range),
    },
    returns: Opus::Types.any(Numeric)
  )
  def self.rand(max); end
end

class Kernel
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.readline(_, _1); end
end

class Kernel
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def self.readlines(_, _1); end
end

class Kernel
  standard_method(
    {
      name: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.require(name); end
end

class Kernel
  standard_method(
    {
      name: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.require_relative(name); end
end

class Kernel
  standard_method(
    {
      read: Opus::Types.any(Opus::Types.array_of(IO)),
      write: Opus::Types.any(Opus::Types.array_of(IO), NilClass),
      error: Opus::Types.any(Opus::Types.array_of(IO), NilClass),
      timeout: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def self.select(read, write, error, timeout); end
end

class Kernel
  standard_method(
    {
      duration: Opus::Types.any(Numeric),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.sleep(duration); end
end

class Kernel
  standard_method(
    {
      number: Opus::Types.any(Numeric),
    },
    returns: Opus::Types.any(Numeric)
  )
  def self.srand(number); end
end

class Kernel
  standard_method(
    {
      cmd: Opus::Types.any(String),
      file1: Opus::Types.any(String),
      file2: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass, Time)
  )
  def self.test(cmd, file1, file2); end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(Proc)
  )
  def proc(); end
end

class Kernel
  standard_method(
    {
      _: Opus::Types.any(String, Class, Exception),
      _1: Opus::Types.any(String, NilClass),
      _2: Opus::Types.any(Opus::Types.array_of(String), NilClass),
    },
    returns: Opus::Types.any(NilClass)
  )
  def raise(_, _1, _2); end
end

class Marshal
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(Proc, NilClass),
    },
    returns: Opus::Types.any(Object)
  )
  def self.load(_, _1); end
end

class MatchData
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end
end

class MatchData
  standard_method(
    {
      n: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def begin(n); end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def captures(); end
end

class MatchData
  standard_method(
    {
      n: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def end(n); end
end

class MatchData
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(other); end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash(); end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def length(); end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def names(); end
end

class MatchData
  standard_method(
    {
      n: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Integer))
  )
  def offset(n); end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def post_match(); end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def pre_match(); end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(Regexp)
  )
  def regexp(); end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size(); end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def string(); end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def to_a(); end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.acos(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.acosh(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.asin(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.asinh(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.atan(x); end
end

class Math
  standard_method(
    {
      y: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.atan2(y, x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.atanh(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.cbrt(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.cos(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.cosh(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.erf(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.erfc(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.exp(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]))
  )
  def self.frexp(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.gamma(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      y: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.hypot(x, y); end
end

class Math
  standard_method(
    {
      fraction: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      exponent: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.ldexp(fraction, exponent); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float)
  )
  def self.lgamma(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      base: Opus::Types.any(Integer, Float, Rational, BigDecimal, NilClass),
    },
    returns: Opus::Types.any(Float)
  )
  def self.log(x, base); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.log10(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.log2(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.sin(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.sinh(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.sqrt(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.tan(x); end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.tanh(x); end
end

class Module
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Integer))
  )
  def self.constants(); end
end

class Module
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Module))
  )
  def self.nesting(); end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass, NilClass)
  )
  def <(other); end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass, NilClass)
  )
  def <=(other); end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(other); end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(other); end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(other); end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass, NilClass)
  )
  def >(other); end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass, NilClass)
  )
  def >=(other); end
end

class Module
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Module))
  )
  def ancestors(); end
end

class Module
  standard_method(
    {
      module: Opus::Types.any(Symbol),
      filename: Opus::Types.any(String),
    },
    returns: Opus::Types.any(NilClass)
  )
  def autoload(module, filename); end
end

class Module
  standard_method(
    {
      name: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def autoload?(name); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(String),
      filename: Opus::Types.any(String, NilClass),
      lineno: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def class_eval(_, filename, lineno); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def class_variable_defined?(_); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def class_variable_get(_); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      _1: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def class_variable_set(_, _1); end
end

class Module
  standard_method(
    {
      inherit: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def class_variables(inherit); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      inherit: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def const_defined?(_, inherit); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      inherit: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def const_get(_, inherit); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def const_missing(_); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      _1: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def const_set(_, _1); end
end

class Module
  standard_method(
    {
      inherit: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def constants(inherit); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(_); end
end

class Module
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Module))
  )
  def included_modules(); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(UnboundMethod)
  )
  def instance_method(_); end
end

class Module
  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def instance_methods(include_super); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def method_defined?(_); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(String),
      filename: Opus::Types.any(String, NilClass),
      lineno: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def module_eval(_, filename, lineno); end
end

class Module
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def name(); end
end

class Module
  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def private_instance_methods(include_super); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def private_method_defined?(_); end
end

class Module
  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def protected_instance_methods(include_super); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def protected_method_defined?(_); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(UnboundMethod)
  )
  def public_instance_method(_); end
end

class Module
  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def public_instance_methods(include_super); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def public_method_defined?(_); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def remove_class_variable(_); end
end

class Module
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def singleton_class?(); end
end

class Module
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol),
      _1: Opus::Types.any(Method),
    },
    returns: Opus::Types.any(Symbol)
  )
  def define_method(_, _1); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def extend_object(_); end
end

class Module
  standard_method(
    {
      othermod: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def extended(othermod); end
end

class Module
  standard_method(
    {
      othermod: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def included(othermod); end
end

class Module
  standard_method(
    {
      meth: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def method_added(meth); end
end

class Module
  standard_method(
    {
      method_name: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def method_removed(method_name); end
end

class Module
  standard_method(
    {
      othermod: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def prepended(othermod); end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def remove_const(_); end
end

class NilClass
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(FalseClass)
  )
  def &(obj); end
end

class NilClass
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ^(obj); end
end

class NilClass
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def |(obj); end
end

class NilClass
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def rationalize(); end
end

class NilClass
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.coerce([]))
  )
  def to_a(); end
end

class NilClass
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c(); end
end

class NilClass
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f(); end
end

class NilClass
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
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
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def -@(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def +@(); end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Object)
  )
  def <=>(_); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def abs(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def abs2(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ceil(); end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)]))
  )
  def coerce(_); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def conj(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def conjugate(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def denominator(); end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer)
  )
  def div(_); end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)]))
  )
  def divmod(_); end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def fdiv(_); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def floor(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def i(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def imag(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def imaginary(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def integer?(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def magnitude(); end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def modulo(_); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def numerator(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)]))
  )
  def polar(); end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def quo(_); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def real(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def real?(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)]))
  )
  def rect(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)]))
  )
  def rectangular(); end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def remainder(_); end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(_); end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(TypeError)
  )
  def singleton_method_added(_); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_int(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def truncate(); end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end
end

class Object
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def !~(other); end
end

class Object
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(other); end
end

class Object
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(other); end
end

class Object
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(NilClass)
  )
  def =~(other); end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(Class)
  )
  def class(); end
end

class Object
  standard_method(
    {
      port: Opus::Types.any(IO),
    },
    returns: Opus::Types.any(NilClass)
  )
  def display(port); end
end

class Object
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(other); end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def frozen?(); end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash(); end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Class),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def instance_of?(_); end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def instance_variable_defined?(_); end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def instance_variable_get(_); end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      _1: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def instance_variable_set(_, _1); end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def instance_variables(); end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Class, Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def is_a?(_); end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Class),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def kind_of?(_); end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(Method)
  )
  def method(_); end
end

class Object
  standard_method(
    {
      regular: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def methods(regular); end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def nil?(); end
end

class Object
  standard_method(
    {
      all: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def private_methods(all); end
end

class Object
  standard_method(
    {
      all: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def protected_methods(all); end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(Method)
  )
  def public_method(_); end
end

class Object
  standard_method(
    {
      all: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def public_methods(all); end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def remove_instance_variable(_); end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(Class)
  )
  def singleton_class(); end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(Method)
  )
  def singleton_method(_); end
end

class Object
  standard_method(
    {
      all: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def singleton_methods(all); end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def tainted?(); end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def untrusted?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Pathname)
  )
  def self.getwd(); end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String),
      p2: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Pathname))
  )
  def self.glob(p1, p2); end
end

class Pathname
  standard_method(
    {
      other: Opus::Types.any(String, Pathname),
    },
    returns: Opus::Types.any(Pathname)
  )
  def +(other); end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(p1); end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(p1); end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(p1); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def absolute?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(BasicObject)
  )
  def ascend(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def atime(); end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(Pathname)
  )
  def basename(p1); end
end

class Pathname
  standard_method(
    {
      length: Opus::Types.any(Integer, NilClass),
      offset: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def binread(length, offset); end
end

class Pathname
  standard_method(
    {
      _: Opus::Types.any(String),
      offset: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def binwrite(_, offset); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def birthtime(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def blockdev?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def chardev?(); end
end

class Pathname
  standard_method(
    {
      with_directory: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Pathname))
  )
  def children(with_directory); end
end

class Pathname
  standard_method(
    {
      mode: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def chmod(mode); end
end

class Pathname
  standard_method(
    {
      owner: Opus::Types.any(Integer),
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def chown(owner, group); end
end

class Pathname
  standard_method(
    {
      consider_symlink: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def cleanpath(consider_symlink); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def ctime(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(BasicObject)
  )
  def delete(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(BasicObject)
  )
  def descend(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def directory?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Pathname)
  )
  def dirname(); end
end

class Pathname
  standard_method(
    {
      with_directory: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def each_child(with_directory); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(BasicObject)
  )
  def each_entry(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Pathname))
  )
  def entries(); end
end

class Pathname
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def executable?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def executable_real?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def exist?(); end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String, Pathname, NilClass),
    },
    returns: Opus::Types.any(Pathname)
  )
  def expand_path(p1); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def extname(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def file?(); end
end

class Pathname
  standard_method(
    {
      pattern: Opus::Types.any(String),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def fnmatch(pattern, flags); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def ftype(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def grpowned?(); end
end

class Pathname
  standard_method(
    {
      mode: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def lchmod(mode); end
end

class Pathname
  standard_method(
    {
      owner: Opus::Types.any(Integer),
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def lchown(owner, group); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(File::Stat)
  )
  def lstat(); end
end

class Pathname
  standard_method(
    {
      old: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def make_link(old); end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def mkdir(p1); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(BasicObject)
  )
  def mkpath(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def mountpoint?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def mtime(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def owned?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Pathname)
  )
  def parent(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def pipe?(); end
end

class Pathname
  standard_method(
    {
      length: Opus::Types.any(Integer, NilClass),
      offset: Opus::Types.any(Integer, NilClass),
      open_args: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def read(length, offset, open_args); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def readable?(); end
end

class Pathname
  standard_method(
    {
      sep: Opus::Types.any(String, NilClass),
      limit: Opus::Types.any(Integer, NilClass),
      open_args: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def readlines(sep, limit, open_args); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def readlink(); end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def realdirpath(p1); end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def realpath(p1); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def relative?(); end
end

class Pathname
  standard_method(
    {
      base_directory: Opus::Types.any(String, Pathname),
    },
    returns: Opus::Types.any(Pathname)
  )
  def relative_path_from(base_directory); end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def rename(p1); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def rmdir(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def rmtree(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def root?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def setgid?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def setuid?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def size?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def socket?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.coerce([Pathname, Pathname]))
  )
  def split(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(File::Stat)
  )
  def stat(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sticky?(); end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Pathname)
  )
  def sub_ext(p1); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def symlink?(); end
end

class Pathname
  standard_method(
    {
      mode: Opus::Types.any(Integer, NilClass),
      perm: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def sysopen(mode, perm); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_path(); end
end

class Pathname
  standard_method(
    {
      length: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def truncate(length); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def unlink(); end
end

class Pathname
  standard_method(
    {
      atime: Opus::Types.any(Time),
      mtime: Opus::Types.any(Time),
    },
    returns: Opus::Types.any(Integer)
  )
  def utime(atime, mtime); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def world_readable?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def world_writable?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def writable?(); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def writable_real?(); end
end

class Pathname
  standard_method(
    {
      _: Opus::Types.any(String),
      offset: Opus::Types.any(Integer, NilClass),
      open_args: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def write(_, offset, open_args); end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end
end

class Proc
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def arity(); end
end

class Proc
  standard_method(
    {},
    returns: Opus::Types.any(Binding)
  )
  def binding(); end
end

class Proc
  standard_method(
    {
      arity: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Proc)
  )
  def curry(arity); end
end

class Proc
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash(); end
end

class Proc
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def lambda(); end
end

class Proc
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.coerce([String, Integer]))
  )
  def source_location(); end
end

class Proc
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class Process
  standard_method(
    {
      msg: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def self.abort(msg); end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def self.argv0(); end
end

class Process
  standard_method(
    {
      clock_id: Opus::Types.any(Symbol, Integer),
      unit: Opus::Types.any(Symbol, NilClass),
    },
    returns: Opus::Types.any(Float, Integer)
  )
  def self.clock_getres(clock_id, unit); end
end

class Process
  standard_method(
    {
      clock_id: Opus::Types.any(Symbol, Integer),
      unit: Opus::Types.any(Symbol, NilClass),
    },
    returns: Opus::Types.any(Float, Integer)
  )
  def self.clock_gettime(clock_id, unit); end
end

class Process
  standard_method(
    {
      nochdir: Opus::Types.any(BasicObject, NilClass),
      noclose: Opus::Types.any(BasicObject, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.daemon(nochdir, noclose); end
end

class Process
  standard_method(
    {
      pid: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Thread)
  )
  def self.detach(pid); end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.egid(); end
end

class Process
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.egid=(_); end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.euid(); end
end

class Process
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.euid=(_); end
end

class Process
  standard_method(
    {
      status: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def self.exit(status); end
end

class Process
  standard_method(
    {
      status: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def self.exit!(status); end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.fork(); end
end

class Process
  standard_method(
    {
      pid: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.getpgid(pid); end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.getpgrp(); end
end

class Process
  standard_method(
    {
      kind: Opus::Types.any(Integer),
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.getpriority(kind, _); end
end

class Process
  standard_method(
    {
      resource: Opus::Types.any(Symbol, String, Integer),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Integer, Integer]))
  )
  def self.getrlimit(resource); end
end

class Process
  standard_method(
    {
      pid: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.getsid(pid); end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.gid(); end
end

class Process
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.gid=(_); end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Integer))
  )
  def self.groups(); end
end

class Process
  standard_method(
    {
      _: Opus::Types.any(Opus::Types.array_of(Integer)),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Integer))
  )
  def self.groups=(_); end
end

class Process
  standard_method(
    {
      username: Opus::Types.any(String),
      gid: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Opus::Types.array_of(Integer))
  )
  def self.initgroups(username, gid); end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.maxgroups(); end
end

class Process
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.maxgroups=(_); end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.pid(); end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.ppid(); end
end

class Process
  standard_method(
    {
      kind: Opus::Types.any(Integer),
      _: Opus::Types.any(Integer),
      priority: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.setpriority(kind, _, priority); end
end

class Process
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.setproctitle(_); end
end

class Process
  standard_method(
    {
      resource: Opus::Types.any(Symbol, String, Integer),
      cur_limit: Opus::Types.any(Integer),
      max_limit: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setrlimit(resource, cur_limit, max_limit); end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.setsid(); end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Process::Tms)
  )
  def self.times(); end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.uid(); end
end

class Process
  standard_method(
    {
      user: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.uid=(user); end
end

class Process
  standard_method(
    {
      pid: Opus::Types.any(Integer, NilClass),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.wait(pid, flags); end
end

class Process
  standard_method(
    {
      pid: Opus::Types.any(Integer, NilClass),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Integer, Integer]))
  )
  def self.wait2(pid, flags); end
end

class Process
  standard_method(
    {
      pid: Opus::Types.any(Integer, NilClass),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.waitpid(pid, flags); end
end

class Process
  standard_method(
    {
      pid: Opus::Types.any(Integer, NilClass),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Integer, Integer]))
  )
  def self.waitpid2(pid, flags); end
end

class Process::GID
  standard_method(
    {
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.change_privilege(group); end
end

class Process::GID
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.eid(); end
end

class Process::GID
  standard_method(
    {
      name: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.from_name(name); end
end

class Process::GID
  standard_method(
    {
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.grant_privilege(group); end
end

class Process::GID
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.re_exchange(); end
end

class Process::GID
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.re_exchangeable?(); end
end

class Process::GID
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.rid(); end
end

class Process::GID
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.sid_available?(); end
end

class Process::UID
  standard_method(
    {
      user: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.change_privilege(user); end
end

class Process::UID
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.eid(); end
end

class Process::UID
  standard_method(
    {
      name: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.from_name(name); end
end

class Process::UID
  standard_method(
    {
      user: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.grant_privilege(user); end
end

class Process::UID
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.re_exchange(); end
end

class Process::UID
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.re_exchangeable?(); end
end

class Process::UID
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.rid(); end
end

class Process::UID
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.sid_available?(); end
end

class Process::Status
  standard_method(
    {
      num: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def &(num); end
end

class Process::Status
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(other); end
end

class Process::Status
  standard_method(
    {
      num: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def >>(num); end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def coredump?(); end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def exited?(); end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def exitstatus(); end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def pid(); end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def signaled?(); end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def stopped?(); end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def stopsig(); end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def success?(); end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def termsig(); end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_i(); end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class Process::Sys
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.geteuid(); end
end

class Process::Sys
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.getgid(); end
end

class Process::Sys
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.getuid(); end
end

class Process::Sys
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.issetugid(); end
end

class Process::Sys
  standard_method(
    {
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setegid(group); end
end

class Process::Sys
  standard_method(
    {
      user: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.seteuid(user); end
end

class Process::Sys
  standard_method(
    {
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setgid(group); end
end

class Process::Sys
  standard_method(
    {
      rid: Opus::Types.any(Integer),
      eid: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setregid(rid, eid); end
end

class Process::Sys
  standard_method(
    {
      rid: Opus::Types.any(Integer),
      eid: Opus::Types.any(Integer),
      sid: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setresgid(rid, eid, sid); end
end

class Process::Sys
  standard_method(
    {
      rid: Opus::Types.any(Integer),
      eid: Opus::Types.any(Integer),
      sid: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setresuid(rid, eid, sid); end
end

class Process::Sys
  standard_method(
    {
      rid: Opus::Types.any(Integer),
      eid: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setreuid(rid, eid); end
end

class Process::Sys
  standard_method(
    {
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setrgid(group); end
end

class Process::Sys
  standard_method(
    {
      user: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setruid(user); end
end

class Process::Sys
  standard_method(
    {
      user: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setuid(user); end
end

class Process::Waiter
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def pid(); end
end

class Random
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end
end

class Random
  standard_method(
    {
      size: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(String)
  )
  def bytes(size); end
end

class Random
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def seed(); end
end

class Random
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.new_seed(); end
end

class Random
  standard_method(
    {
      max: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Numeric)
  )
  def self.rand(max); end
end

class Random
  standard_method(
    {
      number: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Numeric)
  )
  def self.srand(number); end
end

class Range
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(obj); end
end

class Range
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(obj); end
end

class Range
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def cover?(obj); end
end

class Range
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def exclude_end?(); end
end

class Range
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash(); end
end

class Range
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(obj); end
end

class Range
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Range
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def size(); end
end

class Range
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal)
  )
  def %(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def *(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def +(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def -(_); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def -@(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def +@(); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def **(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def /(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Object)
  )
  def <=>(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def abs(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def abs2(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg(); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer)
  )
  def div(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal)
  )
  def modulo(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def ceil(_); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def denominator(); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal), Opus::Types.any(Integer, Float, Rational, BigDecimal)]))
  )
  def divmod(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float)
  )
  def fdiv(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def floor(_); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def numerator(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase(); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def quo(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational)
  )
  def rationalize(_); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(_); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_i(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, Rational)
  )
  def truncate(_); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def conj(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def conjugate(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imag(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imaginary(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def real(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass)
  )
  def real?(); end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c(); end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, Complex),
    },
    returns: Opus::Types.any(Opus::Types.coerce([Rational, Rational]), Opus::Types.coerce([Float, Float]), Opus::Types.coerce([Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex), Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)]))
  )
  def coerce(_); end
end

class Regexp
  standard_method(
    {
      _: Opus::Types.any(String, Symbol),
    },
    returns: Opus::Types.any(String)
  )
  def self.escape(_); end
end

class Regexp
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(MatchData, String)
  )
  def self.last_match(_); end
end

class Regexp
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Regexp, NilClass)
  )
  def self.try_convert(obj); end
end

class Regexp
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(other); end
end

class Regexp
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(other); end
end

class Regexp
  standard_method(
    {
      str: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def =~(str); end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def casefold?(); end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(Encoding)
  )
  def encoding(); end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def fixed_encoding?(); end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash(); end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Regexp
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(MatchData, NilClass)
  )
  def match(_, _1); end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.hash_of(keys: String, values: Opus::Types.array_of(Integer)))
  )
  def named_captures(); end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def names(); end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def options(); end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def source(); end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def ~(); end
end

class Set
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end
end

class Set
  standard_method(
    {},
    returns: Opus::Types.any(Set)
  )
  def flatten(); end
end

class Set
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(String)
  )
  def %(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(String)
  )
  def *(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def +(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(String)
  )
  def <<(_); end
end

class String
  standard_method(
    {
      other: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(other); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def =~(_); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ascii_only?(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def b(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Array)
  )
  def bytes(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def bytesize(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def capitalize(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def capitalize!(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(NilClass, Integer)
  )
  def casecmp(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer),
      _1: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def center(_, _1); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Array)
  )
  def chars(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def chomp(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def chomp!(_); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def chop(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def chop!(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def chr(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def clear(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Integer))
  )
  def codepoints(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer, Object),
    },
    returns: Opus::Types.any(String)
  )
  def concat(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def crypt(_); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def downcase(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def downcase!(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def dump(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_byte(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_char(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_codepoint(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_line(_); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Encoding)
  )
  def encoding(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, Encoding),
    },
    returns: Opus::Types.any(String)
  )
  def force_encoding(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def getbyte(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(String, Hash),
    },
    returns: Opus::Types.any(String, Enumerator)
  )
  def gsub(_, _1); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String, NilClass, Enumerator)
  )
  def gsub!(_, _1); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hex(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def index(_, _1); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def replace(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def insert(_, _1); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def intern(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def length(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def lines(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer),
      _1: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def ljust(_, _1); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def lstrip(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def lstrip!(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(MatchData)
  )
  def match(_, _1); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def next(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def next!(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def oct(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ord(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def partition(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def prepend(_); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def reverse(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, Regexp),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def rindex(_, _1); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer),
      _1: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def rjust(_, _1); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, Regexp),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def rpartition(_); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def rstrip(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def rstrip!(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def scrub(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def scrub!(_); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String, NilClass, Integer),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def split(_, _1); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def squeeze(_); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def squeeze!(_); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def strip(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def strip!(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(String, Hash),
    },
    returns: Opus::Types.any(String)
  )
  def sub(_, _1); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def sub!(_, _1); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def succ(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def sum(_); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def swapcase(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def swapcase!(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def to_i(_); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_str(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def to_sym(); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def tr(_, _1); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def tr!(_, _1); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def tr_s(_, _1); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def tr_s!(_, _1); end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def unpack(_); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def upcase(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def upcase!(); end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def valid_encoding?(); end
end

class StringScanner
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(StringScanner)
  )
  def self.new(_, _1); end
end

class StringScanner
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eos?(); end
end

class StringScanner
  standard_method(
    {
      _: Opus::Types.any(Regexp),
    },
    returns: Opus::Types.any(String)
  )
  def scan(_); end
end

class StringScanner
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def getch(); end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.array_of(Symbol))
  )
  def self.all_symbols(); end
end

class Symbol
  standard_method(
    {
      other: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(other); end
end

class Symbol
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==(obj); end
end

class Symbol
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def =~(obj); end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def capitalize(); end
end

class Symbol
  standard_method(
    {
      other: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def casecmp(other); end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def downcase(); end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Encoding)
  )
  def encoding(); end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def id2name(); end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def length(); end
end

class Symbol
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def match(obj); end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def succ(); end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def swapcase(); end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Proc)
  )
  def to_proc(); end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def upcase(); end
end

class Time
  standard_method(
    {
      _: Opus::Types.any(Time),
      seconds_with_frac: Opus::Types.any(Numeric),
      seconds: Opus::Types.any(Numeric),
      microseconds_with_frac: Opus::Types.any(Numeric),
    },
    returns: Opus::Types.any(Time)
  )
  def self.at(_, seconds_with_frac, seconds, microseconds_with_frac); end
end

class Time
  standard_method(
    {
      year: Opus::Types.any(Integer),
      month: Opus::Types.any(Integer, String, NilClass),
      day: Opus::Types.any(Integer, NilClass),
      hour: Opus::Types.any(Integer, NilClass),
      min: Opus::Types.any(Integer, NilClass),
      sec: Opus::Types.any(Numeric, NilClass),
      usec_with_frac: Opus::Types.any(Numeric, NilClass),
    },
    returns: Opus::Types.any(Time)
  )
  def self.gm(year, month, day, hour, min, sec, usec_with_frac); end
end

class Time
  standard_method(
    {
      year: Opus::Types.any(Integer),
      month: Opus::Types.any(Integer, String, NilClass),
      day: Opus::Types.any(Integer, NilClass),
      hour: Opus::Types.any(Integer, NilClass),
      min: Opus::Types.any(Integer, NilClass),
      sec: Opus::Types.any(Numeric, NilClass),
      usec_with_frac: Opus::Types.any(Numeric, NilClass),
    },
    returns: Opus::Types.any(Time)
  )
  def self.local(year, month, day, hour, min, sec, usec_with_frac); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def self.now(); end
end

class Time
  standard_method(
    {
      year: Opus::Types.any(Integer),
      month: Opus::Types.any(Integer, String, NilClass),
      day: Opus::Types.any(Integer, NilClass),
      hour: Opus::Types.any(Integer, NilClass),
      min: Opus::Types.any(Integer, NilClass),
      sec: Opus::Types.any(Numeric, NilClass),
      usec_with_frac: Opus::Types.any(Numeric, NilClass),
    },
    returns: Opus::Types.any(Time)
  )
  def self.utc(year, month, day, hour, min, sec, usec_with_frac); end
end

class Time
  standard_method(
    {
      _: Opus::Types.any(Numeric),
    },
    returns: Opus::Types.any(Time)
  )
  def +(_); end
end

class Time
  standard_method(
    {
      _: Opus::Types.any(Time, Numeric),
    },
    returns: Opus::Types.any(Float, Time)
  )
  def -(_); end
end

class Time
  standard_method(
    {
      other: Opus::Types.any(Time),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>(other); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def asctime(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def ctime(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def day(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def dst?(); end
end

class Time
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(_); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def friday?(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def getgm(); end
end

class Time
  standard_method(
    {
      utc_offset: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Time)
  )
  def getlocal(utc_offset); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def getutc(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def gmt?(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def gmt_offset(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hour(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def isdst(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def mday(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def min(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def mon(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def monday?(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def nsec(); end
end

class Time
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Time)
  )
  def round(_); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def sec(); end
end

class Time
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def strftime(_); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Numeric)
  )
  def subsec(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def succ(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sunday?(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def thursday?(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.coerce([Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer, Opus::Types.any(TrueClass, FalseClass), String]))
  )
  def to_a(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Numeric)
  )
  def to_i(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def tuesday?(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Numeric)
  )
  def tv_nsec(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Numeric)
  )
  def tv_sec(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Numeric)
  )
  def tv_usec(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Numeric)
  )
  def usec(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def utc?(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def utc_offset(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def wday(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def wednesday?(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def yday(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def year(); end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def zone(); end
end

class YAML
  standard_method(
    {
      filename: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String))
  )
  def self.load_file(filename); end
end

