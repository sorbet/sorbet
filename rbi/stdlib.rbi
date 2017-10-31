class BasicObject
end
module Kernel
end
class Object < BasicObject
  include Kernel
  include BasicObject
end
class Data < Object
  include Object
end
class Addrinfo < Data
  include Data
end
class Exception < Object
  include Object
end
class StandardError < Exception
  include Exception
end
class ArgumentError < StandardError
  include StandardError
end
module Enumerable
end
class Array < Object
  include Enumerable
  include Object
end
class File < IO
  include IO
end
module File::Constants
end
class IO < Object
  include File::Constants
  include Enumerable
  include Object
end
class BasicSocket < IO
  include IO
end
class Binding < Object
  include Object
end
module Bundler
end
class Bundler::BundlerError < StandardError
  include StandardError
end
class Bundler::APIResponseMismatchError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::CurrentRuby < Object
  include Object
end
class Bundler::CyclicDependencyError < Bundler::BundlerError
  include Bundler::BundlerError
end
module Bundler::GemHelpers
end
class Bundler::Definition < Object
  include Bundler::GemHelpers
  include Object
end
class Bundler::DepProxy < Object
  include Object
end
module Gem
end
class Gem::Dependency < Object
  include Object
end
class Bundler::Dependency < Gem::Dependency
  include Gem::Dependency
end
class Bundler::DeprecatedError < Bundler::BundlerError
  include Bundler::BundlerError
end
module Bundler::RubyDsl
end
class Bundler::Dsl < Object
  include Bundler::RubyDsl
  include Object
end
class Bundler::GemfileError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::Dsl::DSLError < Bundler::GemfileError
  include Bundler::GemfileError
end
module Bundler::MatchPlatform
end
class Gem::BasicSpecification < Object
  include Object
end
class Gem::Specification < Gem::BasicSpecification
  include Bundler::MatchPlatform
  include Bundler::GemHelpers
  include Gem::BasicSpecification
end
class Bundler::EndpointSpecification < Gem::Specification
  include Gem::Specification
end
class Bundler::EnvironmentPreserver < Object
  include Object
end
class Struct < Object
  include Enumerable
  include Object
end
class Bundler::GemHelpers::PlatformMatch < Struct
  include Struct
end
class Bundler::GemNotFound < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::GemRequireError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::GemVersionPromoter < Object
  include Object
end
class Bundler::GemfileEvalError < Bundler::GemfileError
  include Bundler::GemfileError
end
class Bundler::GemfileLockNotFound < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::GemfileNotFound < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::GemspecError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::GenericSystemCallError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::GitError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::HTTPError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::Index < Object
  include Enumerable
  include Object
end
class Bundler::InstallError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::InstallHookError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::InvalidOption < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::LazySpecification < Object
  include Bundler::MatchPlatform
  include Bundler::GemHelpers
  include Object
end
module Comparable
end
class Bundler::LazySpecification::Identifier < Struct
  include Comparable
  include Struct
end
class Bundler::LockfileError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::LockfileParser < Object
  include Object
end
class Bundler::MarshalError < StandardError
  include StandardError
end
class Bundler::PermissionError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::NoSpaceOnDeviceError < Bundler::PermissionError
  include Bundler::PermissionError
end
class Bundler::OperationNotSupportedError < Bundler::PermissionError
  include Bundler::PermissionError
end
class Bundler::PathError < Bundler::BundlerError
  include Bundler::BundlerError
end
module Bundler::Plugin
end
class Bundler::Plugin::API < Object
  include Object
end
class Bundler::PluginError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::Plugin::MalformattedPlugin < Bundler::PluginError
  include Bundler::PluginError
end
class Bundler::Plugin::UndefinedCommandError < Bundler::PluginError
  include Bundler::PluginError
end
class Bundler::Plugin::UnknownSourceError < Bundler::PluginError
  include Bundler::PluginError
end
class Bundler::ProductionError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::RemoteSpecification < Object
  include Comparable
  include Bundler::MatchPlatform
  include Bundler::GemHelpers
  include Object
end
class Bundler::RubyVersion < Object
  include Object
end
class Bundler::RubyVersionMismatch < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::RubygemsIntegration < Object
  include Object
end
class Bundler::RubygemsIntegration::Modern < Bundler::RubygemsIntegration
  include Bundler::RubygemsIntegration
end
class Bundler::RubygemsIntegration::AlmostModern < Bundler::RubygemsIntegration::Modern
  include Bundler::RubygemsIntegration::Modern
end
class Bundler::RubygemsIntegration::Legacy < Bundler::RubygemsIntegration
  include Bundler::RubygemsIntegration
end
class Bundler::RubygemsIntegration::Ancient < Bundler::RubygemsIntegration::Legacy
  include Bundler::RubygemsIntegration::Legacy
end
class Bundler::RubygemsIntegration::Future < Bundler::RubygemsIntegration
  include Bundler::RubygemsIntegration
end
class Bundler::RubygemsIntegration::MoreFuture < Bundler::RubygemsIntegration::Future
  include Bundler::RubygemsIntegration::Future
end
class Bundler::RubygemsIntegration::MoreModern < Bundler::RubygemsIntegration::Modern
  include Bundler::RubygemsIntegration::Modern
end
class Bundler::RubygemsIntegration::Transitional < Bundler::RubygemsIntegration::Legacy
  include Bundler::RubygemsIntegration::Legacy
end
module Bundler::SharedHelpers
end
class Bundler::Runtime < Object
  include Bundler::SharedHelpers
  include Object
end
class Bundler::SecurityError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::Settings < Object
  include Object
end
class Bundler::Source < Object
  include Object
end
class Bundler::Source::Path < Bundler::Source
  include Bundler::Source
end
class Bundler::Source::Gemspec < Bundler::Source::Path
  include Bundler::Source::Path
end
class Bundler::Source::Git < Bundler::Source::Path
  include Bundler::Source::Path
end
class Bundler::Source::Git::GitCommandError < Bundler::GitError
  include Bundler::GitError
end
class Bundler::Source::Git::GitNotAllowedError < Bundler::GitError
  include Bundler::GitError
end
class Bundler::Source::Git::GitNotInstalledError < Bundler::GitError
  include Bundler::GitError
end
class Bundler::Source::Git::GitProxy < Object
  include Object
end
class Bundler::Source::Git::MissingGitRevisionError < Bundler::GitError
  include Bundler::GitError
end
class Bundler::Source::Rubygems < Bundler::Source
  include Bundler::Source
end
class Bundler::SourceList < Object
  include Object
end
module TSort
end
class Bundler::SpecSet < Object
  include TSort
  include Enumerable
  include Object
end
class Bundler::StubSpecification < Bundler::RemoteSpecification
  include Bundler::RemoteSpecification
end
class Bundler::SudoNotPermittedError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::TemporaryResourceError < Bundler::PermissionError
  include Bundler::PermissionError
end
class Bundler::ThreadCreationError < Bundler::BundlerError
  include Bundler::BundlerError
end
module Bundler::UI
end
class Gem::StreamUI < Object
  include Object
end
class Gem::SilentUI < Gem::StreamUI
  include Gem::StreamUI
end
class Bundler::UI::RGProxy < Gem::SilentUI
  include Gem::SilentUI
end
class Bundler::UI::Silent < Object
  include Object
end
module Bundler::URICredentialsFilter
end
class Bundler::VersionConflict < Bundler::BundlerError
  include Bundler::BundlerError
end
class Bundler::VirtualProtocolError < Bundler::BundlerError
  include Bundler::BundlerError
end
module Bundler::YAMLSerializer
end
class Bundler::YamlSyntaxError < Bundler::BundlerError
  include Bundler::BundlerError
end
class Module < Object
  include Object
end
class Class < Module
  include Module
end
class IndexError < StandardError
  include StandardError
end
class StopIteration < IndexError
  include IndexError
end
class ClosedQueueError < StopIteration
  include StopIteration
end
class Numeric < Object
  include Comparable
  include Object
end
class Complex < Numeric
  include Numeric
end
module DidYouMean
end
class DidYouMean::ClassNameChecker < Object
  include Object
end
module DidYouMean::Correctable
end
class DidYouMean::Formatter < Object
  include Object
end
module DidYouMean::Jaro
end
module DidYouMean::JaroWinkler
end
module DidYouMean::Levenshtein
end
class DidYouMean::MethodNameChecker < Object
  include Object
end
module DidYouMean::NameErrorCheckers
end
class DidYouMean::NullChecker < Object
  include Object
end
class DidYouMean::SpellChecker < Object
  include Object
end
class DidYouMean::VariableNameChecker < Object
  include Object
end
module Digest
end
module Digest::Instance
end
class Digest::Class < Object
  include Digest::Instance
  include Object
end
class Digest::Base < Digest::Class
  include Digest::Class
end
class Digest::SHA1 < Digest::Base
  include Digest::Base
end
class Dir < Object
  include Enumerable
  include Object
end
module Dir::Tmpname
end
class IOError < StandardError
  include StandardError
end
class EOFError < IOError
  include IOError
end
class Encoding < Object
  include Object
end
class EncodingError < StandardError
  include StandardError
end
class Encoding::CompatibilityError < EncodingError
  include EncodingError
end
class Encoding::Converter < Data
  include Data
end
class Encoding::ConverterNotFoundError < EncodingError
  include EncodingError
end
class Encoding::InvalidByteSequenceError < EncodingError
  include EncodingError
end
class Encoding::UndefinedConversionError < EncodingError
  include EncodingError
end
class Enumerator < Object
  include Enumerable
  include Object
end
class Enumerator::Generator < Object
  include Enumerable
  include Object
end
class Enumerator::Lazy < Enumerator
  include Enumerator
end
class Enumerator::Yielder < Object
  include Object
end
module Errno
end
class SystemCallError < StandardError
  include StandardError
end
class Errno::E2BIG < SystemCallError
  include SystemCallError
end
class Errno::EACCES < SystemCallError
  include SystemCallError
end
class Errno::EADDRINUSE < SystemCallError
  include SystemCallError
end
class Errno::EADDRNOTAVAIL < SystemCallError
  include SystemCallError
end
class Errno::EAFNOSUPPORT < SystemCallError
  include SystemCallError
end
class Errno::EAGAIN < SystemCallError
  include SystemCallError
end
class Errno::EALREADY < SystemCallError
  include SystemCallError
end
class Errno::EAUTH < SystemCallError
  include SystemCallError
end
class Errno::EBADF < SystemCallError
  include SystemCallError
end
class Errno::EBADMSG < SystemCallError
  include SystemCallError
end
class Errno::EBADRPC < SystemCallError
  include SystemCallError
end
class Errno::EBUSY < SystemCallError
  include SystemCallError
end
class Errno::ECANCELED < SystemCallError
  include SystemCallError
end
class Errno::ECHILD < SystemCallError
  include SystemCallError
end
class Errno::ECONNABORTED < SystemCallError
  include SystemCallError
end
class Errno::ECONNREFUSED < SystemCallError
  include SystemCallError
end
class Errno::ECONNRESET < SystemCallError
  include SystemCallError
end
class Errno::EDEADLK < SystemCallError
  include SystemCallError
end
class Errno::EDESTADDRREQ < SystemCallError
  include SystemCallError
end
class Errno::EDOM < SystemCallError
  include SystemCallError
end
class Errno::EDQUOT < SystemCallError
  include SystemCallError
end
class Errno::EEXIST < SystemCallError
  include SystemCallError
end
class Errno::EFAULT < SystemCallError
  include SystemCallError
end
class Errno::EFBIG < SystemCallError
  include SystemCallError
end
class Errno::EFTYPE < SystemCallError
  include SystemCallError
end
class Errno::EHOSTDOWN < SystemCallError
  include SystemCallError
end
class Errno::EHOSTUNREACH < SystemCallError
  include SystemCallError
end
class Errno::EIDRM < SystemCallError
  include SystemCallError
end
class Errno::EILSEQ < SystemCallError
  include SystemCallError
end
class Errno::EINPROGRESS < SystemCallError
  include SystemCallError
end
class Errno::EINTR < SystemCallError
  include SystemCallError
end
class Errno::EINVAL < SystemCallError
  include SystemCallError
end
class Errno::EIO < SystemCallError
  include SystemCallError
end
class Errno::EISCONN < SystemCallError
  include SystemCallError
end
class Errno::EISDIR < SystemCallError
  include SystemCallError
end
class Errno::ELOOP < SystemCallError
  include SystemCallError
end
class Errno::EMFILE < SystemCallError
  include SystemCallError
end
class Errno::EMLINK < SystemCallError
  include SystemCallError
end
class Errno::EMSGSIZE < SystemCallError
  include SystemCallError
end
class Errno::EMULTIHOP < SystemCallError
  include SystemCallError
end
class Errno::ENAMETOOLONG < SystemCallError
  include SystemCallError
end
class Errno::ENEEDAUTH < SystemCallError
  include SystemCallError
end
class Errno::ENETDOWN < SystemCallError
  include SystemCallError
end
class Errno::ENETRESET < SystemCallError
  include SystemCallError
end
class Errno::ENETUNREACH < SystemCallError
  include SystemCallError
end
class Errno::ENFILE < SystemCallError
  include SystemCallError
end
class Errno::ENOATTR < SystemCallError
  include SystemCallError
end
class Errno::ENOBUFS < SystemCallError
  include SystemCallError
end
class Errno::ENODATA < SystemCallError
  include SystemCallError
end
class Errno::ENODEV < SystemCallError
  include SystemCallError
end
class Errno::ENOENT < SystemCallError
  include SystemCallError
end
class Errno::ENOEXEC < SystemCallError
  include SystemCallError
end
class Errno::ENOLCK < SystemCallError
  include SystemCallError
end
class Errno::ENOLINK < SystemCallError
  include SystemCallError
end
class Errno::ENOMEM < SystemCallError
  include SystemCallError
end
class Errno::ENOMSG < SystemCallError
  include SystemCallError
end
class Errno::ENOPROTOOPT < SystemCallError
  include SystemCallError
end
class Errno::ENOSPC < SystemCallError
  include SystemCallError
end
class Errno::ENOSR < SystemCallError
  include SystemCallError
end
class Errno::ENOSTR < SystemCallError
  include SystemCallError
end
class Errno::ENOSYS < SystemCallError
  include SystemCallError
end
class Errno::ENOTBLK < SystemCallError
  include SystemCallError
end
class Errno::ENOTCONN < SystemCallError
  include SystemCallError
end
class Errno::ENOTDIR < SystemCallError
  include SystemCallError
end
class Errno::ENOTEMPTY < SystemCallError
  include SystemCallError
end
class Errno::ENOTRECOVERABLE < SystemCallError
  include SystemCallError
end
class Errno::ENOTSOCK < SystemCallError
  include SystemCallError
end
class Errno::ENOTSUP < SystemCallError
  include SystemCallError
end
class Errno::ENOTTY < SystemCallError
  include SystemCallError
end
class Errno::ENXIO < SystemCallError
  include SystemCallError
end
class Errno::EOPNOTSUPP < SystemCallError
  include SystemCallError
end
class Errno::EOVERFLOW < SystemCallError
  include SystemCallError
end
class Errno::EOWNERDEAD < SystemCallError
  include SystemCallError
end
class Errno::EPERM < SystemCallError
  include SystemCallError
end
class Errno::EPFNOSUPPORT < SystemCallError
  include SystemCallError
end
class Errno::EPIPE < SystemCallError
  include SystemCallError
end
class Errno::EPROCLIM < SystemCallError
  include SystemCallError
end
class Errno::EPROCUNAVAIL < SystemCallError
  include SystemCallError
end
class Errno::EPROGMISMATCH < SystemCallError
  include SystemCallError
end
class Errno::EPROGUNAVAIL < SystemCallError
  include SystemCallError
end
class Errno::EPROTO < SystemCallError
  include SystemCallError
end
class Errno::EPROTONOSUPPORT < SystemCallError
  include SystemCallError
end
class Errno::EPROTOTYPE < SystemCallError
  include SystemCallError
end
class Errno::ERANGE < SystemCallError
  include SystemCallError
end
class Errno::EREMOTE < SystemCallError
  include SystemCallError
end
class Errno::EROFS < SystemCallError
  include SystemCallError
end
class Errno::ERPCMISMATCH < SystemCallError
  include SystemCallError
end
class Errno::ESHUTDOWN < SystemCallError
  include SystemCallError
end
class Errno::ESOCKTNOSUPPORT < SystemCallError
  include SystemCallError
end
class Errno::ESPIPE < SystemCallError
  include SystemCallError
end
class Errno::ESRCH < SystemCallError
  include SystemCallError
end
class Errno::ESTALE < SystemCallError
  include SystemCallError
end
class Errno::ETIME < SystemCallError
  include SystemCallError
end
class Errno::ETIMEDOUT < SystemCallError
  include SystemCallError
end
class Errno::ETOOMANYREFS < SystemCallError
  include SystemCallError
end
class Errno::ETXTBSY < SystemCallError
  include SystemCallError
end
class Errno::EUSERS < SystemCallError
  include SystemCallError
end
class Errno::EXDEV < SystemCallError
  include SystemCallError
end
class Errno::NOERROR < SystemCallError
  include SystemCallError
end
module Etc
end
class Etc::Group < Struct
  include Struct
end
class Etc::Passwd < Struct
  include Struct
end
class FalseClass < Object
  include Object
end
class Fiber < Object
  include Object
end
class FiberError < StandardError
  include StandardError
end
class File::Stat < Object
  include Comparable
  include Object
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
  include Object
end
module FileUtils::NoWrite
end
module FileUtils::Verbose
end
class Float < Numeric
  include Numeric
end
class RangeError < StandardError
  include StandardError
end
class FloatDomainError < RangeError
  include RangeError
end
module Forwardable
end
module GC
end
module GC::Profiler
end
class RuntimeError < StandardError
  include StandardError
end
class Gem::Exception < RuntimeError
  include RuntimeError
end
class Gem::CommandLineError < Gem::Exception
  include Gem::Exception
end
module Gem::DefaultUserInteraction
end
module Gem::UserInteraction
end
class Gem::ConfigFile < Object
  include Gem::UserInteraction
  include Gem::DefaultUserInteraction
  include Object
end
class ScriptError < Exception
  include Exception
end
class LoadError < ScriptError
  include ScriptError
end
class Gem::LoadError < LoadError
  include LoadError
end
class Gem::ConflictError < Gem::LoadError
  include Gem::LoadError
end
class Gem::ConsoleUI < Gem::StreamUI
  include Gem::StreamUI
end
class Gem::DependencyError < Gem::Exception
  include Gem::Exception
end
class Gem::DependencyRemovalException < Gem::Exception
  include Gem::Exception
end
class Gem::DependencyResolutionError < Gem::DependencyError
  include Gem::DependencyError
end
module Gem::Deprecate
end
class Gem::DocumentError < Gem::Exception
  include Gem::Exception
end
class Gem::EndOfYAMLException < Gem::Exception
  include Gem::Exception
end
class Gem::ErrorReason < Object
  include Object
end
module Gem::Ext
end
class Gem::Ext::Builder < Object
  include Gem::UserInteraction
  include Gem::DefaultUserInteraction
  include Object
end
class Gem::FilePermissionError < Gem::Exception
  include Gem::Exception
end
class Gem::FormatException < Gem::Exception
  include Gem::Exception
end
class Gem::GemNotFoundException < Gem::Exception
  include Gem::Exception
end
class Gem::GemNotInHomeException < Gem::Exception
  include Gem::Exception
end
class Gem::ImpossibleDependenciesError < Gem::Exception
  include Gem::Exception
end
class Gem::InstallError < Gem::Exception
  include Gem::Exception
end
class Gem::InvalidSpecificationException < Gem::Exception
  include Gem::Exception
end
class Gem::List < Object
  include Enumerable
  include Object
end
class Gem::MissingSpecError < Gem::LoadError
  include Gem::LoadError
end
class Gem::MissingSpecVersionError < Gem::MissingSpecError
  include Gem::MissingSpecError
end
class Gem::OperationNotSupportedError < Gem::Exception
  include Gem::Exception
end
class Gem::PathSupport < Object
  include Object
end
class Gem::Platform < Object
  include Object
end
class Gem::PlatformMismatch < Gem::ErrorReason
  include Gem::ErrorReason
end
class Gem::RemoteError < Gem::Exception
  include Gem::Exception
end
class Gem::RemoteInstallationCancelled < Gem::Exception
  include Gem::Exception
end
class Gem::RemoteInstallationSkipped < Gem::Exception
  include Gem::Exception
end
class Gem::RemoteSourceException < Gem::Exception
  include Gem::Exception
end
class Gem::Requirement < Object
  include Object
end
class Gem::Requirement::BadRequirementError < ArgumentError
  include ArgumentError
end
class Gem::RubyVersionMismatch < Gem::Exception
  include Gem::Exception
end
class Gem::Source < Object
  include Comparable
  include Object
end
class Gem::Source::Git < Gem::Source
  include Gem::Source
end
class Gem::Source::Installed < Gem::Source
  include Gem::Source
end
class Gem::Source::Local < Gem::Source
  include Gem::Source
end
class Gem::Source::Lock < Gem::Source
  include Gem::Source
end
class Gem::Source::SpecificFile < Gem::Source
  include Gem::Source
end
class Gem::Source::Vendor < Gem::Source::Installed
  include Gem::Source::Installed
end
class Gem::SourceFetchProblem < Gem::ErrorReason
  include Gem::ErrorReason
end
class Gem::SpecificGemNotFoundException < Gem::GemNotFoundException
  include Gem::GemNotFoundException
end
class Gem::StreamUI::SilentDownloadReporter < Object
  include Object
end
class Gem::StreamUI::SilentProgressReporter < Object
  include Object
end
class Gem::StreamUI::SimpleProgressReporter < Object
  include Gem::DefaultUserInteraction
  include Object
end
class Gem::StreamUI::VerboseDownloadReporter < Object
  include Object
end
class Gem::StreamUI::VerboseProgressReporter < Object
  include Gem::DefaultUserInteraction
  include Object
end
class Gem::StubSpecification < Gem::BasicSpecification
  include Gem::BasicSpecification
end
class Gem::StubSpecification::StubLine < Object
  include Object
end
class SystemExit < Exception
  include Exception
end
class Gem::SystemExitException < SystemExit
  include SystemExit
end
class Gem::UnsatisfiableDependencyError < Gem::DependencyError
  include Gem::DependencyError
end
module Gem::Util
end
class Gem::VerificationError < Gem::Exception
  include Gem::Exception
end
class Gem::Version < Object
  include Comparable
  include Object
end
class Hash < Object
  include Enumerable
  include Object
end
module IO::WaitReadable
end
class IO::EAGAINWaitReadable < Errno::EAGAIN
  include IO::WaitReadable
  include Errno::EAGAIN
end
module IO::WaitWritable
end
class IO::EAGAINWaitWritable < Errno::EAGAIN
  include IO::WaitWritable
  include Errno::EAGAIN
end
class IO::EINPROGRESSWaitReadable < Errno::EINPROGRESS
  include IO::WaitReadable
  include Errno::EINPROGRESS
end
class IO::EINPROGRESSWaitWritable < Errno::EINPROGRESS
  include IO::WaitWritable
  include Errno::EINPROGRESS
end
class IPSocket < BasicSocket
  include BasicSocket
end
class Integer < Numeric
  include Numeric
end
class SignalException < Exception
  include Exception
end
class Interrupt < SignalException
  include SignalException
end
class KeyError < IndexError
  include IndexError
end
class LocalJumpError < StandardError
  include StandardError
end
module Marshal
end
class MatchData < Object
  include Object
end
module Math
end
class Math::DomainError < StandardError
  include StandardError
end
class Method < Object
  include Object
end
module MonitorMixin
end
class Monitor < Object
  include MonitorMixin
  include Object
end
class MonitorMixin::ConditionVariable < Object
  include Object
end
class MonitorMixin::ConditionVariable::Timeout < Exception
  include Exception
end
class NameError < StandardError
  include NameError
  include StandardError
end
class NilClass < Object
  include Object
end
class NoMemoryError < Exception
  include Exception
end
class NoMethodError < NameError
  include NameError
end
class NotImplementedError < ScriptError
  include ScriptError
end
module ObjectSpace
end
class ObjectSpace::WeakMap < Object
  include Enumerable
  include Object
end
module Open3
end
module Opus::Dev
end
module Opus::Dev::BundleManager
end
class Pathname < Object
  include Object
end
class Proc < Object
  include Object
end
module Process
end
module Process::GID
end
class Process::Status < Object
  include Object
end
module Process::Sys
end
class Process::Tms < Struct
  include Struct
end
module Process::UID
end
class Thread < Object
  include Object
end
class Process::Waiter < Thread
  include Thread
end
module Random::Formatter
end
class Random < Object
  include Random::Formatter
  include Object
end
class Range < Object
  include Enumerable
  include Object
end
class Rational < Numeric
  include Numeric
end
module RbConfig
end
class Regexp < Object
  include Object
end
class RegexpError < StandardError
  include StandardError
end
class RubyVM < Object
  include Object
end
class RubyVM::InstructionSequence < Object
  include Object
end
class SecurityError < Exception
  include Exception
end
class Set < Object
  include Enumerable
  include Object
end
module Shellwords
end
module Signal
end
module SingleForwardable
end
class Socket < BasicSocket
  include BasicSocket
end
class Socket::AncillaryData < Object
  include Object
end
module Socket::Constants
end
class Socket::Ifaddr < Data
  include Data
end
class Socket::Option < Object
  include Object
end
class Socket::UDPSource < Object
  include Object
end
class SocketError < StandardError
  include StandardError
end
class SortedSet < Set
  include Set
end
class String < Object
  include Comparable
  include Object
end
class StringIO < Data
  include IO::generic_writable
  include IO::generic_readable
  include Enumerable
  include Data
end
class Symbol < Object
  include Comparable
  include Object
end
class SyntaxError < ScriptError
  include ScriptError
end
class SystemStackError < Exception
  include Exception
end
class TCPSocket < IPSocket
  include IPSocket
end
class TCPServer < TCPSocket
  include TCPSocket
end
class TSort::Cyclic < StandardError
  include StandardError
end
class Tempfile::Remover < Object
  include Object
end
class Thread::Backtrace < Object
  include Object
end
class Thread::Backtrace::Location < Object
  include Object
end
class Thread::ConditionVariable < Object
  include Object
end
class Thread::Mutex < Object
  include Object
end
class Thread::Queue < Object
  include Object
end
class Thread::SizedQueue < Thread::Queue
  include Thread::Queue
end
class ThreadError < StandardError
  include StandardError
end
class ThreadGroup < Object
  include Object
end
class Time < Object
  include Comparable
  include Object
end
class TracePoint < Object
  include Object
end
class TrueClass < Object
  include Object
end
class TypeError < StandardError
  include StandardError
end
class UDPSocket < IPSocket
  include IPSocket
end
class UNIXSocket < BasicSocket
  include BasicSocket
end
class UNIXServer < UNIXSocket
  include UNIXSocket
end
module URI::RFC2396_REGEXP
end
module URI
end
class URI::Error < StandardError
  include StandardError
end
class URI::BadURIError < URI::Error
  include URI::Error
end
module URI::Escape
end
class URI::Generic < Object
  include URI
  include URI::RFC2396_REGEXP
  include Object
end
class URI::FTP < URI::Generic
  include URI::Generic
end
class URI::HTTP < URI::Generic
  include URI::Generic
end
class URI::HTTPS < URI::HTTP
  include URI::HTTP
end
class URI::InvalidComponentError < URI::Error
  include URI::Error
end
class URI::InvalidURIError < URI::Error
  include URI::Error
end
class URI::LDAP < URI::Generic
  include URI::Generic
end
class URI::LDAPS < URI::LDAP
  include URI::LDAP
end
class URI::MailTo < URI::Generic
  include URI::Generic
end
class URI::RFC2396_Parser < Object
  include URI::RFC2396_REGEXP
  include Object
end
module URI::RFC2396_REGEXP::PATTERN
end
class URI::RFC3986_Parser < Object
  include Object
end
module URI::Util
end
class UnboundMethod < Object
  include Object
end
class UncaughtThrowError < ArgumentError
  include ArgumentError
end
module Warning
end
class ZeroDivisionError < StandardError
  include StandardError
end
class Array
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?
  end
end

class Array
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Array
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def length
  end
end

class Base64
  standard_method(
    {
      str: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.decode64
  end
end

class Base64
  standard_method(
    {
      bin: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.encode64
  end
end

class Base64
  standard_method(
    {
      str: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.strict_decode64
  end
end

class Base64
  standard_method(
    {
      bin: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.strict_encode64
  end
end

class Base64
  standard_method(
    {
      str: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.urlsafe_decode64
  end
end

class Base64
  standard_method(
    {
      bin: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.urlsafe_encode64
  end
end

class BasicObject
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class BasicObject
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?
  end
end

class BasicObject
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def !
  end
end

class BasicObject
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def !=
  end
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
  def instance_eval
  end
end

class Benchmark
  standard_method(
    {
      label: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(Benchmark::Tms)
  )
  def self.measure
  end
end

class Benchmark
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.realtime
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def %
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def +
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def -
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def -@
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def +@
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def *
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def **
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def /
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Object)
  )
  def <=>
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def abs
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def abs2
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ceil
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def conj
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def conjugate
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def denominator
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer)
  )
  def div
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def fdiv
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def finite?
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def floor
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imag
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imaginary
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(NilClass, Integer)
  )
  def infinite?
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def magnitude
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def modulo
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def nan?
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def numerator
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(BigDecimal, Complex)
  )
  def quo
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def real
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass)
  )
  def real?
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, BigDecimal)
  )
  def round
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_i
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_int
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, Rational)
  )
  def truncate
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def remainder
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def fix
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(BigDecimal)
  )
  def frac
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def power
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Object)
  )
  def nonzero?
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def exponent
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def sign
  end
end

class BigDecimal
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def _dump
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def sqrt
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      _1: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def add
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      _1: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def sub
  end
end

class BigDecimal
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      _1: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def mult
  end
end

class BigMath
  standard_method(
    {
      _: Opus::Types.any(Integer),
      _1: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def self.exp
  end
end

class BigMath
  standard_method(
    {
      _: Opus::Types.any(Integer),
      _1: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def self.log
  end
end

class BigMath
  standard_method(
    {
      prec: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def E
  end
end

class BigMath
  standard_method(
    {
      prec: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def PI
  end
end

class BigMath
  standard_method(
    {
      x: Opus::Types.any(Integer),
      prec: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def atan
  end
end

class BigMath
  standard_method(
    {
      x: Opus::Types.any(Integer),
      prec: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def cos
  end
end

class BigMath
  standard_method(
    {
      x: Opus::Types.any(Integer),
      prec: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def sin
  end
end

class BigMath
  standard_method(
    {
      x: Opus::Types.any(Integer),
      prec: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(BigDecimal)
  )
  def sqrt
  end
end

class Class
  standard_method(
    {},
    returns: Opus::Types.any(BasicObject)
  )
  def allocate
  end
end

class Class
  standard_method(
    {
      _: Opus::Types.any(Class),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def inherited
  end
end

class Class
  standard_method(
    {},
    returns: Opus::Types.any(Class, NilClass)
  )
  def superclass
  end
end

class Class
  standard_method(
    {},
    returns: Opus::Types.any(Class)
  )
  def class
  end
end

class Class
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def name
  end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex)
  )
  def *
  end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex)
  )
  def **
  end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex)
  )
  def +
  end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex)
  )
  def -
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def -@
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def +@
  end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex)
  )
  def /
  end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def abs
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def abs2
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def angle
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def arg
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def conj
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def conjugate
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def denominator
  end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?
  end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?
  end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex)
  )
  def fdiv
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def imag
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def imaginary
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def magnitude
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def numerator
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def phase
  end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Complex, BigDecimal)
  )
  def quo
  end
end

class Complex
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational)
  )
  def rationalize
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def real
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(FalseClass)
  )
  def real?
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_i
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class Complex
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?
  end
end

class Coverage
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def self.start
  end
end

class Date
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def strftime
  end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.chroot
  end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.delete
  end
end

class Dir
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.exist?
  end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def self.getwd
  end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.home
  end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.mkdir
  end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def self.pwd
  end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.rmdir
  end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.unlink
  end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def close
  end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def fileno
  end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def path
  end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def pos
  end
end

class Dir
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def pos=
  end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def read
  end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def tell
  end
end

class Dir
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def to_path
  end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(Opus::Types.hash_of(keys: String, values: String))
  )
  def self.aliases
  end
end

class Encoding
  standard_method(
    {
      obj1: Opus::Types.any(BasicObject),
      obj2: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Encoding, NilClass)
  )
  def self.compatible?
  end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(Encoding)
  )
  def self.default_external
  end
end

class Encoding
  standard_method(
    {
      _: Opus::Types.any(String, Encoding),
    },
    returns: Opus::Types.any(String, Encoding)
  )
  def self.default_external=
  end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(Encoding)
  )
  def self.default_internal
  end
end

class Encoding
  standard_method(
    {
      _: Opus::Types.any(String, Encoding),
    },
    returns: Opus::Types.any(String, NilClass, Encoding)
  )
  def self.default_internal=
  end
end

class Encoding
  standard_method(
    {
      _: Opus::Types.any(String, Encoding),
    },
    returns: Opus::Types.any(Encoding)
  )
  def self.find
  end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ascii_compatible?
  end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def dummy?
  end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Encoding
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def name
  end
end

class Encoding
  standard_method(
    {
      name: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Encoding)
  )
  def replicate
  end
end

class Enumerable
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def all?
  end
end

class Enumerable
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def any?
  end
end

class Enumerable
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Integer)
  )
  def count
  end
end

class Enumerable
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?
  end
end

class Enumerable
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def none?
  end
end

class Enumerable
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def one?
  end
end

class Enumerator
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Enumerator
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, NilClass)
  )
  def size
  end
end

class Exception
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class Exception
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def cause
  end
end

class Exception
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(Exception)
  )
  def exception
  end
end

class Exception
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Exception
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def message
  end
end

class Exception
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
      dir: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.absolute_path
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
      suffix: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.basename
  end
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
  def self.binread
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.blockdev?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.chardev?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.directory?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.dirname
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.executable?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.executable_real?
  end
end

class File
  standard_method(
    {
      path: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.extname
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.file?
  end
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
  def self.fnmatch
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.ftype
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.grpowned?
  end
end

class File
  standard_method(
    {
      file_1: Opus::Types.any(String, IO),
      file_2: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.identical?
  end
end

class File
  standard_method(
    {
      old: Opus::Types.any(String),
      new: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.link
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(File::Stat)
  )
  def self.lstat
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.owned?
  end
end

class File
  standard_method(
    {
      path: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.path
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.pipe?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.readable?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.readable_real?
  end
end

class File
  standard_method(
    {
      link: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.readlink
  end
end

class File
  standard_method(
    {
      pathname: Opus::Types.any(String),
      dir: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.realpath
  end
end

class File
  standard_method(
    {
      old: Opus::Types.any(String),
      new: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.rename
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.setgid?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.setuid?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.size
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.size?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.socket?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.sticky?
  end
end

class File
  standard_method(
    {
      old: Opus::Types.any(String),
      new: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.symlink
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.symlink?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.truncate
  end
end

class File
  standard_method(
    {
      _: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.umask
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.world_readable?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.world_writable?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.writable?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.writable_real?
  end
end

class File
  standard_method(
    {
      file: Opus::Types.any(String, IO),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.zero?
  end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def atime
  end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def birthtime
  end
end

class File
  standard_method(
    {
      mode: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def chmod
  end
end

class File
  standard_method(
    {
      owner: Opus::Types.any(Integer),
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def chown
  end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def ctime
  end
end

class File
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, TrueClass, FalseClass)
  )
  def flock
  end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(File::Stat)
  )
  def lstat
  end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def mtime
  end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def path
  end
end

class File
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size
  end
end

class File
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def truncate
  end
end

class File::Stat
  standard_method(
    {
      other: Opus::Types.any(File::Stat),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def atime
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def birthtime
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def blksize
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def blockdev?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def blocks
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def chardev?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def ctime
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def dev
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def dev_major
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def dev_minor
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def directory?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def executable?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def executable_real?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def file?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def ftype
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def gid
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def grpowned?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ino
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def mode
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def mtime
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def nlink
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def owned?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def rdev
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def rdev_major
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def rdev_minor
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def readable?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def readable_real?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def setgid?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def setuid?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def socket?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sticky?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def symlink?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def uid
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def world_readable?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def world_writable?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def writable?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def writable_real?
  end
end

class File::Stat
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float, BigDecimal)
  )
  def %
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def *
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, Integer, Rational, BigDecimal, Complex)
  )
  def **
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def +
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def -
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def -@
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def +@
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def /
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Object)
  )
  def <=>
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def abs
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def abs2
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer)
  )
  def div
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ceil
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def denominator
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def fdiv
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def finite?
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def floor
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Object)
  )
  def infinite?
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def magnitude
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float, BigDecimal)
  )
  def modulo
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def nan?
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def next_float
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def numerator
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def prev_float
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def quo
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational)
  )
  def rationalize
  end
end

class Float
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_i
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_int
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def truncate
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def conj
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def conjugate
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imag
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imaginary
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def real
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass)
  )
  def real?
  end
end

class Float
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c
  end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def self.binary_mode
  end
end

class Gem
  standard_method(
    {
      install_dir: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.bindir
  end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(Hash)
  )
  def self.clear_default_specs
  end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def self.clear_paths
  end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def self.config_file
  end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(Gem::ConfigFile)
  )
  def self.configuration
  end
end

class Gem
  standard_method(
    {
      config: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def self.configuration=
  end
end

class Gem
  standard_method(
    {
      gem_name: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def self.datadir
  end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_bindir
  end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_cert_path
  end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_dir
  end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_exec_format
  end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_key_path
  end
end

class Gem
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.default_path
  end
end

class Hash
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def compare_by_identity?
  end
end

class Hash
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?
  end
end

class Hash
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class Hash
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Hash
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def length
  end
end

class Hash
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def %
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def &
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def *
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def **
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def +
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def -
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def -@
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def +@
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def /
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def <<
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Object)
  )
  def <=>
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def >>
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Rational, Float, BigDecimal),
    },
    returns: Opus::Types.any(Integer)
  )
  def []
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def ^
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def |
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ~
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def abs
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def bit_length
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer)
  )
  def div
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float, BigDecimal, Complex)
  )
  def fdiv
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def magnitude
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def modulo
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def quo
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def abs2
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ceil
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Encoding),
    },
    returns: Opus::Types.any(String)
  )
  def chr
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def conj
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def conjugate
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def denominator
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def even?
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def gcd
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def floor
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imag
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imaginary
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass)
  )
  def integer?
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def lcm
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def next
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def numerator
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def odd?
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ord
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def pred
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational)
  )
  def rationalize
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def real
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass)
  )
  def real?
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def remainder
  end
end

class Integer
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def succ
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_i
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_int
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def truncate
  end
end

class Integer
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?
  end
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
  def advise
  end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def autoclose=
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def autoclose?
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def binmode?
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def close
  end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def close_on_exec=
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def close_on_exec?
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def close_read
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def close_write
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def closed?
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eof
  end
end

class IO
  standard_method(
    {
      integer_cmd: Opus::Types.any(Integer),
      arg: Opus::Types.any(String, Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def fcntl
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def fdatasync
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def fileno
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def fsync
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def getbyte
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def getc
  end
end

class IO
  standard_method(
    {
      sep: Opus::Types.any(String, NilClass),
      limit: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def gets
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Encoding)
  )
  def internal_encoding
  end
end

class IO
  standard_method(
    {
      integer_cmd: Opus::Types.any(Integer),
      arg: Opus::Types.any(String, Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def ioctl
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def isatty
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def lineno
  end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def lineno=
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def pid
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def pos
  end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def pos=
  end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(Numeric, String),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def putc
  end
end

class IO
  standard_method(
    {
      length: Opus::Types.any(Integer, NilClass),
      outbuf: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def read
  end
end

class IO
  standard_method(
    {
      len: Opus::Types.any(Integer),
      buf: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def read_nonblock
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def readbyte
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def readchar
  end
end

class IO
  standard_method(
    {
      sep: Opus::Types.any(String, NilClass),
      limit: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def readline
  end
end

class IO
  standard_method(
    {
      maxlen: Opus::Types.any(Integer),
      outbuf: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def readpartial
  end
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
  def reopen
  end
end

class IO
  standard_method(
    {
      amount: Opus::Types.any(Integer),
      whence: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def seek
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(File::Stat)
  )
  def stat
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sync
  end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sync=
  end
end

class IO
  standard_method(
    {
      maxlen: Opus::Types.any(Integer),
      outbuf: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def sysread
  end
end

class IO
  standard_method(
    {
      amount: Opus::Types.any(Integer),
      whence: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def sysseek
  end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def syswrite
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def tell
  end
end

class IO
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def tty?
  end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(String, Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def ungetbyte
  end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(NilClass)
  )
  def ungetc
  end
end

class IO
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def write
  end
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
  def self.binread
  end
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
  def self.copy_stream
  end
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
  def self.sysopen
  end
end

class Kernel
  standard_method(
    {
      x: Opus::Types.any(Numeric, String),
      y: Opus::Types.any(Numeric),
    },
    returns: Opus::Types.any(Complex)
  )
  def self.Complex
  end
end

class Kernel
  standard_method(
    {
      x: Opus::Types.any(Numeric),
    },
    returns: Opus::Types.any(Float)
  )
  def self.Float
  end
end

class Kernel
  standard_method(
    {
      arg: Opus::Types.any(Numeric, String),
      base: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.Integer
  end
end

class Kernel
  standard_method(
    {
      x: Opus::Types.any(Numeric, String),
      y: Opus::Types.any(Numeric),
    },
    returns: Opus::Types.any(Rational)
  )
  def self.Rational
  end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(Symbol, NilClass)
  )
  def self.__callee__
  end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def self.__dir__
  end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(Symbol, NilClass)
  )
  def self.__method__
  end
end

class Kernel
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.`
  end
end

class Kernel
  standard_method(
    {
      msg: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.abort
  end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(Proc)
  )
  def self.at_exit
  end
end

class Kernel
  standard_method(
    {
      module: Opus::Types.any(String, Symbol),
      filename: Opus::Types.any(String),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.autoload
  end
end

class Kernel
  standard_method(
    {
      name: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def self.autoload?
  end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(Binding)
  )
  def self.binding
  end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.block_given?
  end
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
  def self.eval
  end
end

class Kernel
  standard_method(
    {
      status: Opus::Types.any(Integer, TrueClass, FalseClass),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.exit
  end
end

class Kernel
  standard_method(
    {
      status: Opus::Types.any(Integer, TrueClass, FalseClass),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.exit!
  end
end

class Kernel
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.gets
  end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.iterator?
  end
end

class Kernel
  standard_method(
    {
      filename: Opus::Types.any(String),
      _: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.load
  end
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
  def self.open
  end
end

class Kernel
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.putc
  end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(NilClass)
  )
  def self.raise
  end
end

class Kernel
  standard_method(
    {
      max: Opus::Types.any(Integer, Range),
    },
    returns: Opus::Types.any(Numeric)
  )
  def self.rand
  end
end

class Kernel
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def self.readline
  end
end

class Kernel
  standard_method(
    {
      name: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.require
  end
end

class Kernel
  standard_method(
    {
      name: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.require_relative
  end
end

class Kernel
  standard_method(
    {
      duration: Opus::Types.any(Numeric),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.sleep
  end
end

class Kernel
  standard_method(
    {
      number: Opus::Types.any(Numeric),
    },
    returns: Opus::Types.any(Numeric)
  )
  def self.srand
  end
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
  def self.test
  end
end

class Kernel
  standard_method(
    {},
    returns: Opus::Types.any(Proc)
  )
  def proc
  end
end

class Marshal
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(Proc, NilClass),
    },
    returns: Opus::Types.any(Object)
  )
  def self.load
  end
end

class MatchData
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class MatchData
  standard_method(
    {
      n: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def begin
  end
end

class MatchData
  standard_method(
    {
      n: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def end
  end
end

class MatchData
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?
  end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash
  end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def length
  end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def post_match
  end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def pre_match
  end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(Regexp)
  )
  def regexp
  end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size
  end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def string
  end
end

class MatchData
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.acos
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.acosh
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.asin
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.asinh
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.atan
  end
end

class Math
  standard_method(
    {
      y: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.atan2
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.atanh
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.cbrt
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.cos
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.cosh
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.erf
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.erfc
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.exp
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.gamma
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      y: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.hypot
  end
end

class Math
  standard_method(
    {
      fraction: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      exponent: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.ldexp
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer, Float)
  )
  def self.lgamma
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
      base: Opus::Types.any(Integer, Float, Rational, BigDecimal, NilClass),
    },
    returns: Opus::Types.any(Float)
  )
  def self.log
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.log10
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.log2
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.sin
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.sinh
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.sqrt
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.tan
  end
end

class Math
  standard_method(
    {
      x: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Float)
  )
  def self.tanh
  end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass, NilClass)
  )
  def <
  end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass, NilClass)
  )
  def <=
  end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>
  end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===
  end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass, NilClass)
  )
  def >
  end
end

class Module
  standard_method(
    {
      other: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass, NilClass)
  )
  def >=
  end
end

class Module
  standard_method(
    {
      module: Opus::Types.any(Symbol),
      filename: Opus::Types.any(String),
    },
    returns: Opus::Types.any(NilClass)
  )
  def autoload
  end
end

class Module
  standard_method(
    {
      name: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def autoload?
  end
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
  def class_eval
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def class_variable_defined?
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def class_variable_get
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      _1: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def class_variable_set
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      inherit: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def const_defined?
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      inherit: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def const_get
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def const_missing
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      _1: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def const_set
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(UnboundMethod)
  )
  def instance_method
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def method_defined?
  end
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
  def module_eval
  end
end

class Module
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def name
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def private_method_defined?
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def protected_method_defined?
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(UnboundMethod)
  )
  def public_instance_method
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def public_method_defined?
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def remove_class_variable
  end
end

class Module
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def singleton_class?
  end
end

class Module
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol),
      _1: Opus::Types.any(Method),
    },
    returns: Opus::Types.any(Symbol)
  )
  def define_method
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def extend_object
  end
end

class Module
  standard_method(
    {
      othermod: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def extended
  end
end

class Module
  standard_method(
    {
      othermod: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def included
  end
end

class Module
  standard_method(
    {
      meth: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def method_added
  end
end

class Module
  standard_method(
    {
      method_name: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def method_removed
  end
end

class Module
  standard_method(
    {
      othermod: Opus::Types.any(Module),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def prepended
  end
end

class Module
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def remove_const
  end
end

class NilClass
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(FalseClass)
  )
  def &
  end
end

class NilClass
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ^
  end
end

class NilClass
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def |
  end
end

class NilClass
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def rationalize
  end
end

class NilClass
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c
  end
end

class NilClass
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f
  end
end

class NilClass
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r
  end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def %
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def -@
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def +@
  end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Object)
  )
  def <=>
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def abs
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def abs2
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ceil
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def conj
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def conjugate
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def denominator
  end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer)
  )
  def div
  end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?
  end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def fdiv
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def floor
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def i
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def imag
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def imaginary
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def integer?
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def magnitude
  end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def modulo
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def numerator
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase
  end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def quo
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def real
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def real?
  end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal)
  )
  def remainder
  end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round
  end
end

class Numeric
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(TypeError)
  )
  def singleton_method_added
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_int
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def truncate
  end
end

class Numeric
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?
  end
end

class Object
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def !~
  end
end

class Object
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>
  end
end

class Object
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===
  end
end

class Object
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(NilClass)
  )
  def =~
  end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(Class)
  )
  def class
  end
end

class Object
  standard_method(
    {
      port: Opus::Types.any(IO),
    },
    returns: Opus::Types.any(NilClass)
  )
  def display
  end
end

class Object
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?
  end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def frozen?
  end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash
  end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Class),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def instance_of?
  end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def instance_variable_defined?
  end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def instance_variable_get
  end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol, String),
      _1: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def instance_variable_set
  end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Class, Module),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def is_a?
  end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Class),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def kind_of?
  end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(Method)
  )
  def method
  end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def nil?
  end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(Method)
  )
  def public_method
  end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def remove_instance_variable
  end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(Class)
  )
  def singleton_class
  end
end

class Object
  standard_method(
    {
      _: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(Method)
  )
  def singleton_method
  end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def tainted?
  end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class Object
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def untrusted?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Pathname)
  )
  def self.getwd
  end
end

class Pathname
  standard_method(
    {
      other: Opus::Types.any(String, Pathname),
    },
    returns: Opus::Types.any(Pathname)
  )
  def +
  end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>
  end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def absolute?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(BasicObject)
  )
  def ascend
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def atime
  end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(Pathname)
  )
  def basename
  end
end

class Pathname
  standard_method(
    {
      length: Opus::Types.any(Integer, NilClass),
      offset: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def binread
  end
end

class Pathname
  standard_method(
    {
      _: Opus::Types.any(String),
      offset: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def binwrite
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def birthtime
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def blockdev?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def chardev?
  end
end

class Pathname
  standard_method(
    {
      mode: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def chmod
  end
end

class Pathname
  standard_method(
    {
      owner: Opus::Types.any(Integer),
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def chown
  end
end

class Pathname
  standard_method(
    {
      consider_symlink: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def cleanpath
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def ctime
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(BasicObject)
  )
  def delete
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(BasicObject)
  )
  def descend
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def directory?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Pathname)
  )
  def dirname
  end
end

class Pathname
  standard_method(
    {
      with_directory: Opus::Types.any(TrueClass, FalseClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def each_child
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(BasicObject)
  )
  def each_entry
  end
end

class Pathname
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def executable?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def executable_real?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def exist?
  end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String, Pathname, NilClass),
    },
    returns: Opus::Types.any(Pathname)
  )
  def expand_path
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def extname
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def file?
  end
end

class Pathname
  standard_method(
    {
      pattern: Opus::Types.any(String),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def fnmatch
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def ftype
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def grpowned?
  end
end

class Pathname
  standard_method(
    {
      mode: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def lchmod
  end
end

class Pathname
  standard_method(
    {
      owner: Opus::Types.any(Integer),
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def lchown
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(File::Stat)
  )
  def lstat
  end
end

class Pathname
  standard_method(
    {
      old: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def make_link
  end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def mkdir
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(BasicObject)
  )
  def mkpath
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def mountpoint?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def mtime
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def owned?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Pathname)
  )
  def parent
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def pipe?
  end
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
  def read
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def readable?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def readlink
  end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def realdirpath
  end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def realpath
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def relative?
  end
end

class Pathname
  standard_method(
    {
      base_directory: Opus::Types.any(String, Pathname),
    },
    returns: Opus::Types.any(Pathname)
  )
  def relative_path_from
  end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def rename
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def rmdir
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def rmtree
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def root?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def setgid?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def setuid?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def size?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def socket?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(File::Stat)
  )
  def stat
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sticky?
  end
end

class Pathname
  standard_method(
    {
      p1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Pathname)
  )
  def sub_ext
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def symlink?
  end
end

class Pathname
  standard_method(
    {
      mode: Opus::Types.any(Integer, NilClass),
      perm: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def sysopen
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_path
  end
end

class Pathname
  standard_method(
    {
      length: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def truncate
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def unlink
  end
end

class Pathname
  standard_method(
    {
      atime: Opus::Types.any(Time),
      mtime: Opus::Types.any(Time),
    },
    returns: Opus::Types.any(Integer)
  )
  def utime
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def world_readable?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def world_writable?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def writable?
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def writable_real?
  end
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
  def write
  end
end

class Pathname
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?
  end
end

class Proc
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def arity
  end
end

class Proc
  standard_method(
    {},
    returns: Opus::Types.any(Binding)
  )
  def binding
  end
end

class Proc
  standard_method(
    {
      arity: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Proc)
  )
  def curry
  end
end

class Proc
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash
  end
end

class Proc
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def lambda
  end
end

class Proc
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class Process
  standard_method(
    {
      msg: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def self.abort
  end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def self.argv0
  end
end

class Process
  standard_method(
    {
      clock_id: Opus::Types.any(Symbol, Integer),
      unit: Opus::Types.any(Symbol, NilClass),
    },
    returns: Opus::Types.any(Float, Integer)
  )
  def self.clock_getres
  end
end

class Process
  standard_method(
    {
      clock_id: Opus::Types.any(Symbol, Integer),
      unit: Opus::Types.any(Symbol, NilClass),
    },
    returns: Opus::Types.any(Float, Integer)
  )
  def self.clock_gettime
  end
end

class Process
  standard_method(
    {
      nochdir: Opus::Types.any(BasicObject, NilClass),
      noclose: Opus::Types.any(BasicObject, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.daemon
  end
end

class Process
  standard_method(
    {
      pid: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Thread)
  )
  def self.detach
  end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.egid
  end
end

class Process
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.egid=
  end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.euid
  end
end

class Process
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.euid=
  end
end

class Process
  standard_method(
    {
      status: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def self.exit
  end
end

class Process
  standard_method(
    {
      status: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(BasicObject)
  )
  def self.exit!
  end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def self.fork
  end
end

class Process
  standard_method(
    {
      pid: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.getpgid
  end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.getpgrp
  end
end

class Process
  standard_method(
    {
      kind: Opus::Types.any(Integer),
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.getpriority
  end
end

class Process
  standard_method(
    {
      pid: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.getsid
  end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.gid
  end
end

class Process
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.gid=
  end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.maxgroups
  end
end

class Process
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.maxgroups=
  end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.pid
  end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.ppid
  end
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
  def self.setpriority
  end
end

class Process
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def self.setproctitle
  end
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
  def self.setrlimit
  end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.setsid
  end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Process::Tms)
  )
  def self.times
  end
end

class Process
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.uid
  end
end

class Process
  standard_method(
    {
      user: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.uid=
  end
end

class Process
  standard_method(
    {
      pid: Opus::Types.any(Integer, NilClass),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.wait
  end
end

class Process
  standard_method(
    {
      pid: Opus::Types.any(Integer, NilClass),
      flags: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.waitpid
  end
end

class Process::GID
  standard_method(
    {
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.change_privilege
  end
end

class Process::GID
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.eid
  end
end

class Process::GID
  standard_method(
    {
      name: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.from_name
  end
end

class Process::GID
  standard_method(
    {
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.grant_privilege
  end
end

class Process::GID
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.re_exchange
  end
end

class Process::GID
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.re_exchangeable?
  end
end

class Process::GID
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.rid
  end
end

class Process::GID
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.sid_available?
  end
end

class Process::UID
  standard_method(
    {
      user: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.change_privilege
  end
end

class Process::UID
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.eid
  end
end

class Process::UID
  standard_method(
    {
      name: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.from_name
  end
end

class Process::UID
  standard_method(
    {
      user: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def self.grant_privilege
  end
end

class Process::UID
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.re_exchange
  end
end

class Process::UID
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.re_exchangeable?
  end
end

class Process::UID
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.rid
  end
end

class Process::UID
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.sid_available?
  end
end

class Process::Status
  standard_method(
    {
      num: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def &
  end
end

class Process::Status
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class Process::Status
  standard_method(
    {
      num: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer)
  )
  def >>
  end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def coredump?
  end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def exited?
  end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def exitstatus
  end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def pid
  end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def signaled?
  end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def stopped?
  end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def stopsig
  end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def success?
  end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def termsig
  end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_i
  end
end

class Process::Status
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class Process::Sys
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.geteuid
  end
end

class Process::Sys
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.getgid
  end
end

class Process::Sys
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.getuid
  end
end

class Process::Sys
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def self.issetugid
  end
end

class Process::Sys
  standard_method(
    {
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setegid
  end
end

class Process::Sys
  standard_method(
    {
      user: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.seteuid
  end
end

class Process::Sys
  standard_method(
    {
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setgid
  end
end

class Process::Sys
  standard_method(
    {
      rid: Opus::Types.any(Integer),
      eid: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setregid
  end
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
  def self.setresgid
  end
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
  def self.setresuid
  end
end

class Process::Sys
  standard_method(
    {
      rid: Opus::Types.any(Integer),
      eid: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setreuid
  end
end

class Process::Sys
  standard_method(
    {
      group: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setrgid
  end
end

class Process::Sys
  standard_method(
    {
      user: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setruid
  end
end

class Process::Sys
  standard_method(
    {
      user: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(NilClass)
  )
  def self.setuid
  end
end

class Process::Waiter
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def pid
  end
end

class Random
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class Random
  standard_method(
    {
      size: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(String)
  )
  def bytes
  end
end

class Random
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def seed
  end
end

class Random
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def self.new_seed
  end
end

class Random
  standard_method(
    {
      max: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Numeric)
  )
  def self.rand
  end
end

class Random
  standard_method(
    {
      number: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Numeric)
  )
  def self.srand
  end
end

class Range
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class Range
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===
  end
end

class Range
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def cover?
  end
end

class Range
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def exclude_end?
  end
end

class Range
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash
  end
end

class Range
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?
  end
end

class Range
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Range
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def size
  end
end

class Range
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal)
  )
  def %
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def *
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def +
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def -
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def -@
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def +@
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def **
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def /
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def <=
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def >=
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Object)
  )
  def <=>
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def abs
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def abs2
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def angle
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def arg
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Integer)
  )
  def div
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal)
  )
  def modulo
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def ceil
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def denominator
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def equal?
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Float)
  )
  def fdiv
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def floor
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def numerator
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def phase
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational, Float, BigDecimal, Complex)
  )
  def quo
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex),
    },
    returns: Opus::Types.any(Rational)
  )
  def rationalize
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, Float, Rational, BigDecimal, Complex)
  )
  def round
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def to_i
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class Rational
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, Rational)
  )
  def truncate
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def zero?
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def conj
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def conjugate
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imag
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def imaginary
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def real
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass)
  )
  def real?
  end
end

class Rational
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c
  end
end

class Regexp
  standard_method(
    {
      _: Opus::Types.any(String, Symbol),
    },
    returns: Opus::Types.any(String)
  )
  def self.escape
  end
end

class Regexp
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(MatchData, String)
  )
  def self.last_match
  end
end

class Regexp
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Regexp, NilClass)
  )
  def self.try_convert
  end
end

class Regexp
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class Regexp
  standard_method(
    {
      other: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===
  end
end

class Regexp
  standard_method(
    {
      str: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def =~
  end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def casefold?
  end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(Encoding)
  )
  def encoding
  end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def fixed_encoding?
  end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash
  end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Regexp
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(MatchData, NilClass)
  )
  def match
  end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def options
  end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def source
  end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class Regexp
  standard_method(
    {},
    returns: Opus::Types.any(Integer, NilClass)
  )
  def ~
  end
end

class Set
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?
  end
end

class Set
  standard_method(
    {},
    returns: Opus::Types.any(Set)
  )
  def flatten
  end
end

class Set
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(String)
  )
  def %
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(String)
  )
  def *
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def +
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(String)
  )
  def <<
  end
end

class String
  standard_method(
    {
      other: Opus::Types.any(String),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ===
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Object),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def =~
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ascii_only?
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def b
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Array)
  )
  def bytes
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def bytesize
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def capitalize
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def capitalize!
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(NilClass, Integer)
  )
  def casecmp
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer),
      _1: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def center
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Array)
  )
  def chars
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def chomp
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def chomp!
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def chop
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def chop!
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def chr
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def clear
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer, Object),
    },
    returns: Opus::Types.any(String)
  )
  def concat
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def crypt
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def downcase
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def downcase!
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def dump
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_byte
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_char
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_codepoint
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String, Enumerator)
  )
  def each_line
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Encoding)
  )
  def encoding
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, Encoding),
    },
    returns: Opus::Types.any(String)
  )
  def force_encoding
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def getbyte
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(String, Hash),
    },
    returns: Opus::Types.any(String, Enumerator)
  )
  def gsub
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String, NilClass, Enumerator)
  )
  def gsub!
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hex
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def include?
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def index
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def replace
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def insert
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def intern
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def length
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer),
      _1: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def ljust
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def lstrip
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def lstrip!
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(MatchData)
  )
  def match
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def next
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def next!
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def oct
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def ord
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def prepend
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def reverse
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, Regexp),
      _1: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def rindex
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer),
      _1: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def rjust
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def rstrip
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def rstrip!
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def scrub
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def scrub!
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def size
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def squeeze
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String, NilClass),
    },
    returns: Opus::Types.any(String)
  )
  def squeeze!
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def strip
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def strip!
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(String, Hash),
    },
    returns: Opus::Types.any(String)
  )
  def sub
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Regexp, String),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def sub!
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def succ
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def sum
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def swapcase
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def swapcase!
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Complex)
  )
  def to_c
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Integer)
  )
  def to_i
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_str
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def to_sym
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def tr
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def tr!
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def tr_s
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String, NilClass)
  )
  def tr_s!
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def upcase
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(String, NilClass)
  )
  def upcase!
  end
end

class String
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def valid_encoding?
  end
end

class StringScanner
  standard_method(
    {
      _: Opus::Types.any(String),
      _1: Opus::Types.any(TrueClass, FalseClass, NilClass),
    },
    returns: Opus::Types.any(StringScanner)
  )
  def self.new
  end
end

class StringScanner
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eos?
  end
end

class StringScanner
  standard_method(
    {
      _: Opus::Types.any(Regexp),
    },
    returns: Opus::Types.any(String)
  )
  def scan
  end
end

class StringScanner
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def getch
  end
end

class Symbol
  standard_method(
    {
      other: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>
  end
end

class Symbol
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def ==
  end
end

class Symbol
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def =~
  end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def capitalize
  end
end

class Symbol
  standard_method(
    {
      other: Opus::Types.any(Symbol),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def casecmp
  end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def downcase
  end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def empty?
  end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Encoding)
  )
  def encoding
  end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def id2name
  end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def length
  end
end

class Symbol
  standard_method(
    {
      obj: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def match
  end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def succ
  end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def swapcase
  end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Proc)
  )
  def to_proc
  end
end

class Symbol
  standard_method(
    {},
    returns: Opus::Types.any(Symbol)
  )
  def upcase
  end
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
  def self.at
  end
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
  def self.gm
  end
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
  def self.local
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def self.now
  end
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
  def self.utc
  end
end

class Time
  standard_method(
    {
      _: Opus::Types.any(Numeric),
    },
    returns: Opus::Types.any(Time)
  )
  def +
  end
end

class Time
  standard_method(
    {
      _: Opus::Types.any(Time, Numeric),
    },
    returns: Opus::Types.any(Float, Time)
  )
  def -
  end
end

class Time
  standard_method(
    {
      other: Opus::Types.any(Time),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def <=>
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def asctime
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def ctime
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def day
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def dst?
  end
end

class Time
  standard_method(
    {
      _: Opus::Types.any(BasicObject),
    },
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def eql?
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def friday?
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def getgm
  end
end

class Time
  standard_method(
    {
      utc_offset: Opus::Types.any(Integer, NilClass),
    },
    returns: Opus::Types.any(Time)
  )
  def getlocal
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def getutc
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def gmt?
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def gmt_offset
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hash
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def hour
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def inspect
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def isdst
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def mday
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def min
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def mon
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def monday?
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def nsec
  end
end

class Time
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Time)
  )
  def round
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def sec
  end
end

class Time
  standard_method(
    {
      _: Opus::Types.any(String),
    },
    returns: Opus::Types.any(String)
  )
  def strftime
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Numeric)
  )
  def subsec
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Time)
  )
  def succ
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def sunday?
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def thursday?
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Float)
  )
  def to_f
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Numeric)
  )
  def to_i
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Rational)
  )
  def to_r
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def to_s
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def tuesday?
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Numeric)
  )
  def tv_nsec
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Numeric)
  )
  def tv_sec
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Numeric)
  )
  def tv_usec
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Numeric)
  )
  def usec
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def utc?
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def utc_offset
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def wday
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(TrueClass, FalseClass)
  )
  def wednesday?
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def yday
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(Integer)
  )
  def year
  end
end

class Time
  standard_method(
    {},
    returns: Opus::Types.any(String)
  )
  def zone
  end
end

