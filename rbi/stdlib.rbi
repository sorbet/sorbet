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
module Bundler::BuildMetadata
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
class Bundler::FeatureFlag < Object
end
module Bundler::FileUtils::StreamUtils_
end
module Bundler::FileUtils
end
module Bundler::FileUtils::LowMethods
end
module Bundler::FileUtils::DryRun
end
class Bundler::FileUtils::Entry_ < Object
  include Bundler::FileUtils::StreamUtils_
end
module Bundler::FileUtils::NoWrite
end
module Bundler::FileUtils::Verbose
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
module Bundler::Molinillo
end
class Bundler::Molinillo::ResolverError < StandardError
end
class Bundler::Molinillo::CircularDependencyError < Bundler::Molinillo::ResolverError
end
module Bundler::Molinillo::Compatibility
end
module Bundler::Molinillo::Delegates
end
module Bundler::Molinillo::Delegates::ResolutionState
end
module Bundler::Molinillo::Delegates::SpecificationProvider
end
module TSort
end
class Bundler::Molinillo::DependencyGraph < Object
  include TSort
  include Enumerable
end
class Bundler::Molinillo::DependencyGraph::Action < Object
end
class Bundler::Molinillo::DependencyGraph::AddEdgeNoCircular < Bundler::Molinillo::DependencyGraph::Action
end
class Bundler::Molinillo::DependencyGraph::AddVertex < Bundler::Molinillo::DependencyGraph::Action
end
class Bundler::Molinillo::DependencyGraph::DeleteEdge < Bundler::Molinillo::DependencyGraph::Action
end
class Bundler::Molinillo::DependencyGraph::DetachVertexNamed < Bundler::Molinillo::DependencyGraph::Action
end
class Bundler::Molinillo::DependencyGraph::Edge < Struct
end
class Bundler::Molinillo::DependencyGraph::Log < Object
end
class Bundler::Molinillo::DependencyGraph::SetPayload < Bundler::Molinillo::DependencyGraph::Action
end
class Bundler::Molinillo::DependencyGraph::Tag < Bundler::Molinillo::DependencyGraph::Action
end
class Bundler::Molinillo::DependencyGraph::Vertex < Object
end
class Bundler::Molinillo::ResolutionState < Struct
end
class Bundler::Molinillo::DependencyState < Bundler::Molinillo::ResolutionState
end
class Bundler::Molinillo::NoSuchDependencyError < Bundler::Molinillo::ResolverError
end
class Bundler::Molinillo::PossibilityState < Bundler::Molinillo::ResolutionState
end
class Bundler::Molinillo::Resolver < Object
end
class Bundler::Molinillo::Resolver::Resolution < Object
  include Bundler::Molinillo::Delegates::SpecificationProvider
  include Bundler::Molinillo::Delegates::ResolutionState
end
class Bundler::Molinillo::Resolver::Resolution::Conflict < Struct
end
class Bundler::Molinillo::Resolver::Resolution::PossibilitySet < Struct
end
class Bundler::Molinillo::Resolver::Resolution::UnwindDetails < Struct
  include Comparable
end
module Bundler::Molinillo::SpecificationProvider
end
module Bundler::Molinillo::UI
end
class Bundler::Molinillo::VersionConflict < Bundler::Molinillo::ResolverError
  include Bundler::Molinillo::Delegates::SpecificationProvider
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
class Bundler::Resolver < Object
  include Bundler::Molinillo::SpecificationProvider
  include Bundler::Molinillo::UI
end
class Bundler::Resolver::SpecGroup < Object
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
class Bundler::Settings::Path < Struct
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
class Bundler::Source::Metadata < Bundler::Source
end
class Bundler::Source::Rubygems < Bundler::Source
end
class Bundler::SourceList < Object
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
module Gem::BundlerVersionFinder
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
class Gem::RuntimeRequirementNotMetError < Gem::InstallError
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
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end

  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def join(_); end

  standard_method(
    {},
    returns: Integer
  )
  def length(); end
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
      filename: Opus::Types.any(String, NilClass),
      lineno: Opus::Types.any(Integer, NilClass),
    },
    returns: BasicObject
  )
  def instance_eval(_, filename, lineno); end
end

module Benchmark
  standard_method(
    {
      width: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.array_of(Benchmark::Tms)
  )
  def self.bmbm(width); end

  standard_method(
    {
      label: Opus::Types.any(String, NilClass),
    },
    returns: Benchmark::Tms
  )
  def self.measure(label); end

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
  def round(_); end

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
  def truncate(_); end

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

class Class
  standard_method(
    {},
    returns: BasicObject
  )
  def allocate(); end

  standard_method(
    {
      _: Class,
    },
    returns: BasicObject
  )
  def inherited(_); end

  standard_method(
    {},
    returns: Opus::Types.any(Class, NilClass)
  )
  def superclass(); end

  standard_method(
    {
      _: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def instance_methods(_); end

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
  def rationalize(_); end

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
end

class Date
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
      _1: Opus::Types.any(Encoding, NilClass),
    },
    returns: Opus::Types.array_of(String)
  )
  def self.entries(_, _1); end

  standard_method(
    {
      file: String,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.exist?(file); end

  standard_method(
    {},
    returns: String
  )
  def self.getwd(); end

  standard_method(
    {
      pattern: Opus::Types.any(String, Opus::Types.array_of(String)),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.glob(pattern, flags); end

  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def self.home(_); end

  standard_method(
    {
      _: String,
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def self.mkdir(_, _1); end

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
    returns: Integer
  )
  def fileno(); end

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
    returns: Integer
  )
  def tell(); end

  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def to_path(); end
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
    {
      _: BasicObject,
    },
    returns: Integer
  )
  def count(_); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?(_); end

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
end

class Enumerator
  standard_method(
    {},
    returns: String
  )
  def inspect(); end

  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, NilClass)
  )
  def size(); end
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
      _: Opus::Types.any(String, NilClass),
    },
    returns: Exception
  )
  def exception(_); end

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
      dir: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def self.absolute_path(file, dir); end

  standard_method(
    {
      file: String,
      suffix: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def self.basename(file, suffix); end

  standard_method(
    {
      _: String,
      _1: Integer,
      _2: Integer,
    },
    returns: String
  )
  def self.binread(_, _1, _2); end

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
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.fnmatch(pattern, path, flags); end

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
      dir: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def self.realdirpath(pathname, dir); end

  standard_method(
    {
      pathname: String,
      dir: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def self.realpath(pathname, dir); end

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
      _: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def self.umask(_); end

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
      preserve: Opus::Types.any(Opus::Types.hash_of(keys: Symbol, values: Opus::Types.any(TrueClass, FalseClass)), NilClass),
    },
    returns: Opus::Types.array_of(String)
  )
  def self.cp_r(src, dest, preserve); end

  standard_method(
    {
      list: Opus::Types.any(String, Pathname),
      mode: Opus::Types.any(Opus::Types.hash_of(keys: Symbol, values: Opus::Types.any(TrueClass, FalseClass)), NilClass),
    },
    returns: Opus::Types.array_of(String)
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
  def rationalize(_); end

  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(_); end

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
    {},
    returns: String
  )
  def self.binary_mode(); end

  standard_method(
    {
      install_dir: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def self.bindir(install_dir); end

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
    returns: BasicObject
  )
  def self.configuration=(config); end

  standard_method(
    {
      args: String,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def self.datadir(args); end

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
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def compare_by_identity?(); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end

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
  def length(); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end
end

class IO
  standard_method(
    {
      _: Symbol,
      offset: Opus::Types.any(Integer, NilClass),
      len: Opus::Types.any(Integer, NilClass),
    },
    returns: NilClass
  )
  def advise(_, offset, len); end

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
      sep: Opus::Types.any(String, NilClass),
      limit: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def gets(sep, limit); end

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
      _: Opus::Types.any(Numeric, String),
    },
    returns: BasicObject
  )
  def putc(_); end

  standard_method(
    {
      length: Opus::Types.any(Integer, NilClass),
      outbuf: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def read(length, outbuf); end

  standard_method(
    {
      len: Integer,
      buf: String,
    },
    returns: String
  )
  def read_nonblock(len, buf); end

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
      sep: Opus::Types.any(String, NilClass),
      limit: Opus::Types.any(Integer, NilClass),
    },
    returns: String
  )
  def readline(sep, limit); end

  standard_method(
    {
      sep: Opus::Types.any(String, NilClass),
      limit: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.array_of(String)
  )
  def readlines(sep, limit); end

  standard_method(
    {
      maxlen: Integer,
      outbuf: String,
    },
    returns: String
  )
  def readpartial(maxlen, outbuf); end

  standard_method(
    {
      other_IO: IO,
      path: String,
      mode_str: String,
    },
    returns: IO
  )
  def reopen(other_IO, path, mode_str); end

  standard_method(
    {},
    returns: Integer
  )
  def rewind(); end

  standard_method(
    {
      amount: Integer,
      whence: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def seek(amount, whence); end

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
      whence: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def sysseek(amount, whence); end

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
      length: Opus::Types.any(Integer, NilClass),
      offset: Opus::Types.any(Integer, NilClass),
    },
    returns: String
  )
  def self.binread(name, length, offset); end

  standard_method(
    {
      src: Opus::Types.any(String, IO),
      dst: Opus::Types.any(String, IO),
      copy_length: Opus::Types.any(Integer, NilClass),
      src_offset: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def self.copy_stream(src, dst, copy_length, src_offset); end

  standard_method(
    {
      read_array: Opus::Types.array_of(IO),
      write_array: Opus::Types.any(Opus::Types.array_of(IO), NilClass),
      error_array: Opus::Types.any(Opus::Types.array_of(IO), NilClass),
      timeout: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Opus::Types.array_of(IO), NilClass)
  )
  def self.select(read_array, write_array, error_array, timeout); end

  standard_method(
    {
      path: String,
      mode: Opus::Types.any(String, NilClass),
      perm: Opus::Types.any(String, NilClass),
    },
    returns: Integer
  )
  def self.sysopen(path, mode, perm); end
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
  def rationalize(_); end

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
  def round(_); end

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
  def self.Complex(x, y); end

  standard_method(
    {
      x: Numeric,
    },
    returns: Float
  )
  def self.Float(x); end

  standard_method(
    {
      arg: Opus::Types.any(Numeric, String),
      base: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def self.Integer(arg, base); end

  standard_method(
    {
      x: Opus::Types.any(Numeric, String),
      y: Numeric,
    },
    returns: Rational
  )
  def self.Rational(x, y); end

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
      msg: Opus::Types.any(String, NilClass),
    },
    returns: NilClass
  )
  def self.abort(msg); end

  standard_method(
    {},
    returns: Proc
  )
  def self.at_exit(); end

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
      start: Opus::Types.any(Integer, NilClass),
      length: Opus::Types.any(Integer, NilClass),
      _: Range,
    },
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.caller(start, length, _); end

  standard_method(
    {
      start: Opus::Types.any(Integer, NilClass),
      length: Opus::Types.any(Integer, NilClass),
      _: Range,
    },
    returns: Opus::Types.any(Opus::Types.array_of(String), NilClass)
  )
  def self.caller_locations(start, length, _); end

  standard_method(
    {
      _: String,
      _1: Opus::Types.any(Binding, NilClass),
      filename: Opus::Types.any(String, NilClass),
      lineno: Opus::Types.any(Integer, NilClass),
    },
    returns: BasicObject
  )
  def self.eval(_, _1, filename, lineno); end

  standard_method(
    {
      status: Opus::Types.any(Integer, TrueClass, FalseClass),
    },
    returns: NilClass
  )
  def self.exit(status); end

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
  def self.fail(_, _1, _2); end

  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: String
  )
  def self.gets(_, _1); end

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
      _: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.load(filename, _); end

  standard_method(
    {},
    returns: Opus::Types.array_of(Symbol)
  )
  def self.local_variables(); end

  standard_method(
    {
      name: String,
      rest: Opus::Types.any(String, Integer, NilClass),
      block: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(IO, NilClass)
  )
  def self.open(name, rest, block); end

  standard_method(
    {
      _: Integer,
    },
    returns: Integer
  )
  def self.putc(_); end

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
      _: Opus::Types.any(String, NilClass),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: String
  )
  def self.readline(_, _1); end

  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.array_of(String)
  )
  def self.readlines(_, _1); end

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
      write: Opus::Types.any(Opus::Types.array_of(IO), NilClass),
      error: Opus::Types.any(Opus::Types.array_of(IO), NilClass),
      timeout: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.array_of(String)
  )
  def self.select(read, write, error, timeout); end

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
      cmd: String,
      file1: String,
      file2: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass, Time)
  )
  def self.test(cmd, file1, file2); end

  standard_method(
    {},
    returns: Proc
  )
  def proc(); end

  standard_method(
    {
      _: Opus::Types.any(String, Class, Exception),
      _1: Opus::Types.any(String, NilClass),
      _2: Opus::Types.any(Opus::Types.array_of(String), NilClass),
    },
    returns: NilClass
  )
  def raise(_, _1, _2); end
end

module Marshal
  standard_method(
    {
      _: String,
      _1: Opus::Types.any(Proc, NilClass),
    },
    returns: Object
  )
  def self.load(_, _1); end
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
      base: Opus::Types.any(Integer, Float, Rational, BigDecimal, NilClass),
    },
    returns: Float
  )
  def self.log(x, base); end

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
      name: Symbol,
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def autoload?(name); end

  standard_method(
    {
      _: String,
      filename: Opus::Types.any(String, NilClass),
      lineno: Opus::Types.any(Integer, NilClass),
    },
    returns: BasicObject
  )
  def class_eval(_, filename, lineno); end

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
    returns: BasicObject
  )
  def class_variable_get(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      _1: BasicObject,
    },
    returns: BasicObject
  )
  def class_variable_set(_, _1); end

  standard_method(
    {
      inherit: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def class_variables(inherit); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      inherit: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def const_defined?(_, inherit); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      inherit: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: BasicObject
  )
  def const_get(_, inherit); end

  standard_method(
    {
      _: Symbol,
    },
    returns: BasicObject
  )
  def const_missing(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      _1: BasicObject,
    },
    returns: BasicObject
  )
  def const_set(_, _1); end

  standard_method(
    {
      inherit: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def constants(inherit); end

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
      include_super: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def instance_methods(include_super); end

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
      filename: Opus::Types.any(String, NilClass),
      lineno: Opus::Types.any(Integer, NilClass),
    },
    returns: BasicObject
  )
  def module_eval(_, filename, lineno); end

  standard_method(
    {},
    returns: String
  )
  def name(); end

  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def private_instance_methods(include_super); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def private_method_defined?(_); end

  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def protected_instance_methods(include_super); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def protected_method_defined?(_); end

  standard_method(
    {
      _: Symbol,
    },
    returns: UnboundMethod
  )
  def public_instance_method(_); end

  standard_method(
    {
      include_super: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def public_instance_methods(include_super); end

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
    returns: BasicObject
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
      _: Symbol,
      _1: Method,
    },
    returns: Symbol
  )
  def define_method(_, _1); end

  standard_method(
    {
      _: BasicObject,
    },
    returns: BasicObject
  )
  def extend_object(_); end

  standard_method(
    {
      othermod: Module,
    },
    returns: BasicObject
  )
  def extended(othermod); end

  standard_method(
    {
      othermod: Module,
    },
    returns: BasicObject
  )
  def included(othermod); end

  standard_method(
    {
      meth: Symbol,
    },
    returns: BasicObject
  )
  def method_added(meth); end

  standard_method(
    {
      method_name: Symbol,
    },
    returns: BasicObject
  )
  def method_removed(method_name); end

  standard_method(
    {
      othermod: Module,
    },
    returns: BasicObject
  )
  def prepended(othermod); end

  standard_method(
    {
      _: Symbol,
    },
    returns: BasicObject
  )
  def remove_const(_); end
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
    {
      port: IO,
    },
    returns: NilClass
  )
  def display(port); end

  standard_method(
    {
      other: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?(other); end

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
    returns: BasicObject
  )
  def instance_variable_get(_); end

  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      _1: BasicObject,
    },
    returns: BasicObject
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
      regular: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def methods(regular); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def nil?(); end

  standard_method(
    {
      all: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def private_methods(all); end

  standard_method(
    {
      all: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def protected_methods(all); end

  standard_method(
    {
      _: Symbol,
    },
    returns: Method
  )
  def public_method(_); end

  standard_method(
    {
      all: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def public_methods(all); end

  standard_method(
    {
      _: Symbol,
    },
    returns: BasicObject
  )
  def remove_instance_variable(_); end

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
      all: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.array_of(Symbol)
  )
  def singleton_methods(all); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def tainted?(); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end

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
      p2: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.array_of(Pathname)
  )
  def self.glob(p1, p2); end

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
    returns: BasicObject
  )
  def ascend(); end

  standard_method(
    {},
    returns: Time
  )
  def atime(); end

  standard_method(
    {
      p1: Opus::Types.any(String, NilClass),
    },
    returns: Pathname
  )
  def basename(p1); end

  standard_method(
    {
      length: Opus::Types.any(Integer, NilClass),
      offset: Opus::Types.any(Integer, NilClass),
    },
    returns: String
  )
  def binread(length, offset); end

  standard_method(
    {
      _: String,
      offset: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def binwrite(_, offset); end

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
      consider_symlink: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: BasicObject
  )
  def cleanpath(consider_symlink); end

  standard_method(
    {},
    returns: Time
  )
  def ctime(); end

  standard_method(
    {},
    returns: BasicObject
  )
  def delete(); end

  standard_method(
    {},
    returns: BasicObject
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
    returns: BasicObject
  )
  def each_child(with_directory); end

  standard_method(
    {},
    returns: BasicObject
  )
  def each_entry(); end

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
      p1: Opus::Types.any(String, Pathname, NilClass),
    },
    returns: Pathname
  )
  def expand_path(p1); end

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
      pattern: String,
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def fnmatch(pattern, flags); end

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
  def symlink?(old); end

  standard_method(
    {
      p1: String,
    },
    returns: Integer
  )
  def mkdir(p1); end

  standard_method(
    {},
    returns: BasicObject
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
      length: Opus::Types.any(Integer, NilClass),
      offset: Opus::Types.any(Integer, NilClass),
      open_args: Opus::Types.any(Integer, NilClass),
    },
    returns: String
  )
  def read(length, offset, open_args); end

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
      sep: Opus::Types.any(String, NilClass),
      limit: Opus::Types.any(Integer, NilClass),
      open_args: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.array_of(String)
  )
  def readlines(sep, limit, open_args); end

  standard_method(
    {},
    returns: String
  )
  def readlink(); end

  standard_method(
    {
      p1: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def realdirpath(p1); end

  standard_method(
    {
      p1: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def realpath(p1); end

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
      p1: String,
    },
    returns: Pathname
  )
  def sub_ext(p1); end

  standard_method(
    {
      mode: Opus::Types.any(Integer, NilClass),
      perm: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def sysopen(mode, perm); end

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
      offset: Opus::Types.any(Integer, NilClass),
      open_args: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def write(_, offset, open_args); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?(); end
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
      arity: Opus::Types.any(Integer, NilClass),
    },
    returns: Proc
  )
  def curry(arity); end

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
    returns: [String, Integer]
  )
  def source_location(); end

  standard_method(
    {},
    returns: String
  )
  def to_s(); end
end

module Process
  standard_method(
    {
      msg: Opus::Types.any(String, NilClass),
    },
    returns: BasicObject
  )
  def self.abort(msg); end

  standard_method(
    {},
    returns: String
  )
  def self.argv0(); end

  standard_method(
    {
      clock_id: Opus::Types.any(Symbol, Integer),
      unit: Opus::Types.any(Symbol, NilClass),
    },
    returns: Opus::Types.any(Float, Integer)
  )
  def self.clock_getres(clock_id, unit); end

  standard_method(
    {
      clock_id: Opus::Types.any(Symbol, Integer),
      unit: Opus::Types.any(Symbol, NilClass),
    },
    returns: Opus::Types.any(Float, Integer)
  )
  def self.clock_gettime(clock_id, unit); end

  standard_method(
    {
      nochdir: Opus::Types.any(BasicObject, NilClass),
      noclose: Opus::Types.any(BasicObject, NilClass),
    },
    returns: Integer
  )
  def self.daemon(nochdir, noclose); end

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
      status: Opus::Types.any(Integer, NilClass),
    },
    returns: BasicObject
  )
  def self.exit(status); end

  standard_method(
    {
      status: Opus::Types.any(Integer, NilClass),
    },
    returns: BasicObject
  )
  def self.exit!(status); end

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
      pid: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def self.getsid(pid); end

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
      max_limit: Opus::Types.any(Integer, NilClass),
    },
    returns: NilClass
  )
  def self.setrlimit(resource, cur_limit, max_limit); end

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
      pid: Opus::Types.any(Integer, NilClass),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def self.wait(pid, flags); end

  standard_method(
    {
      pid: Opus::Types.any(Integer, NilClass),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: [Integer, Integer]
  )
  def self.wait2(pid, flags); end

  standard_method(
    {
      pid: Opus::Types.any(Integer, NilClass),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def self.waitpid(pid, flags); end

  standard_method(
    {
      pid: Opus::Types.any(Integer, NilClass),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: [Integer, Integer]
  )
  def self.waitpid2(pid, flags); end
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
      max: Opus::Types.any(Integer, NilClass),
    },
    returns: Numeric
  )
  def self.rand(max); end

  standard_method(
    {
      number: Opus::Types.any(Integer, NilClass),
    },
    returns: Numeric
  )
  def self.srand(number); end
end

class Range
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
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def cover?(obj); end

  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def exclude_end?(); end

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
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def size(); end

  standard_method(
    {},
    returns: String
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
  def ceil(_); end

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
  def floor(_); end

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
  def rationalize(_); end

  standard_method(
    {
      _: Integer,
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round(_); end

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
  def truncate(_); end

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
  def self.last_match(_); end

  standard_method(
    {
      obj: BasicObject,
    },
    returns: Opus::Types.any(Regexp, NilClass)
  )
  def self.try_convert(obj); end

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
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(MatchData, NilClass)
  )
  def match(_, _1); end

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
end

class Set
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?(); end

  standard_method(
    {},
    returns: Set
  )
  def flatten(); end

  standard_method(
    {},
    returns: Integer
  )
  def size(); end
end

class String
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
      _1: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def center(_, _1); end

  standard_method(
    {},
    returns: Array
  )
  def chars(); end

  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def chomp(_); end

  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def chomp!(_); end

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
    },
    returns: String
  )
  def crypt(_); end

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
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_line(_); end

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
  def gsub(_, _1); end

  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: String,
    },
    returns: Opus::Types.any(String, NilClass, Enumerator)
  )
  def gsub!(_, _1); end

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
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def index(_, _1); end

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
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.array_of(String)
  )
  def lines(_); end

  standard_method(
    {
      _: Integer,
      _1: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def ljust(_, _1); end

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
  def match(_, _1); end

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
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def rindex(_, _1); end

  standard_method(
    {
      _: Integer,
      _1: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def rjust(_, _1); end

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
      _: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def scrub(_); end

  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def scrub!(_); end

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
      _: Opus::Types.any(Regexp, String, NilClass, Integer),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.array_of(String)
  )
  def split(_, _1); end

  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def squeeze(_); end

  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: String
  )
  def squeeze!(_); end

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
  def sub(_, _1); end

  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: String,
    },
    returns: String
  )
  def sub!(_, _1); end

  standard_method(
    {},
    returns: String
  )
  def succ(); end

  standard_method(
    {
      _: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def sum(_); end

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
      _: Opus::Types.any(Integer, NilClass),
    },
    returns: Integer
  )
  def to_i(_); end

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
end

class StringScanner
  standard_method(
    {
      _: String,
      _1: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: StringScanner
  )
  def self.new(_, _1); end

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
end

class Time
  standard_method(
    {
      _: Time,
      seconds_with_frac: Numeric,
      seconds: Numeric,
      microseconds_with_frac: Numeric,
    },
    returns: Time
  )
  def self.at(_, seconds_with_frac, seconds, microseconds_with_frac); end

  standard_method(
    {
      year: Integer,
      month: Opus::Types.any(Integer, String, NilClass),
      day: Opus::Types.any(Integer, NilClass),
      hour: Opus::Types.any(Integer, NilClass),
      min: Opus::Types.any(Integer, NilClass),
      sec: Opus::Types.any(Numeric, NilClass),
      usec_with_frac: Opus::Types.any(Numeric, NilClass),
    },
    returns: Time
  )
  def self.gm(year, month, day, hour, min, sec, usec_with_frac); end

  standard_method(
    {
      year: Integer,
      month: Opus::Types.any(Integer, String, NilClass),
      day: Opus::Types.any(Integer, NilClass),
      hour: Opus::Types.any(Integer, NilClass),
      min: Opus::Types.any(Integer, NilClass),
      sec: Opus::Types.any(Numeric, NilClass),
      usec_with_frac: Opus::Types.any(Numeric, NilClass),
    },
    returns: Time
  )
  def self.local(year, month, day, hour, min, sec, usec_with_frac); end

  standard_method(
    {},
    returns: Time
  )
  def self.now(); end

  standard_method(
    {
      year: Integer,
      month: Opus::Types.any(Integer, String, NilClass),
      day: Opus::Types.any(Integer, NilClass),
      hour: Opus::Types.any(Integer, NilClass),
      min: Opus::Types.any(Integer, NilClass),
      sec: Opus::Types.any(Numeric, NilClass),
      usec_with_frac: Opus::Types.any(Numeric, NilClass),
    },
    returns: Time
  )
  def self.utc(year, month, day, hour, min, sec, usec_with_frac); end

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
      utc_offset: Opus::Types.any(Integer, NilClass),
    },
    returns: Time
  )
  def getlocal(utc_offset); end

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
end

module URI
  standard_method(
    {
      str: String,
      schemes: Opus::Types.any(Array, NilClass),
    },
    returns: Opus::Types.array_of(String)
  )
  def self.extract(str, schemes); end

  standard_method(
    {
      uri: String,
    },
    returns: URI::HTTP
  )
  def self.parse(uri); end

  standard_method(
    {
      schemes: Opus::Types.any(Array, NilClass),
    },
    returns: Opus::Types.array_of(String)
  )
  def self.regexp(schemes); end

  standard_method(
    {},
    returns: Opus::Types.hash_of(keys: String, values: Class)
  )
  def self.scheme_list(); end
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
