# typed: __STDLIB_INTERNAL

# RubyGems is the Ruby standard for publishing and managing third party
# libraries.
#
# For user documentation, see:
#
# *   `gem help` and `gem help [command]`
# *   [RubyGems User Guide](http://guides.rubygems.org/)
# *   [Frequently Asked Questions](http://guides.rubygems.org/faqs)
#
#
# For gem developer documentation see:
#
# *   [Creating Gems](http://guides.rubygems.org/make-your-own-gem)
# *   [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
# *   [`Gem::Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) for
#     version dependency notes
#
#
# Further RubyGems documentation can be found at:
#
# *   [RubyGems Guides](http://guides.rubygems.org)
# *   [RubyGems API](http://www.rubydoc.info/github/rubygems/rubygems) (also
#     available from `gem server`)
#
#
# ## RubyGems Plugins
#
# As of RubyGems 1.3.2, RubyGems will load plugins installed in gems or
# $LOAD\_PATH. Plugins must be named 'rubygems\_plugin' (.rb, .so, etc) and
# placed at the root of your gem's require\_path. Plugins are discovered via
# [`Gem::find_files`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-find_files)
# and then loaded.
#
# For an example plugin, see the [Graph gem](https://github.com/seattlerb/graph)
# which adds a `gem graph` command.
#
# ## RubyGems Defaults, Packaging
#
# RubyGems defaults are stored in lib/rubygems/defaults.rb.  If you're packaging
# RubyGems or implementing Ruby you can change RubyGems' defaults.
#
# For RubyGems packagers, provide lib/rubygems/defaults/operating\_system.rb and
# override any defaults from lib/rubygems/defaults.rb.
#
# For Ruby implementers, provide lib/rubygems/defaults/#{RUBY\_ENGINE}.rb and
# override any defaults from lib/rubygems/defaults.rb.
#
# If you need RubyGems to perform extra work on install or uninstall, your
# defaults override file can set pre/post install and uninstall hooks. See
# [`Gem::pre_install`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-pre_install),
# [`Gem::pre_uninstall`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-pre_uninstall),
# [`Gem::post_install`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-post_install),
# [`Gem::post_uninstall`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-post_uninstall).
#
# ## Bugs
#
# You can submit bugs to the [RubyGems bug
# tracker](https://github.com/rubygems/rubygems/issues) on GitHub
#
# ## Credits
#
# RubyGems is currently maintained by Eric Hodel.
#
# RubyGems was originally developed at RubyConf 2003 by:
#
# *   Rich Kilmer  -- rich(at)infoether.com
# *   Chad Fowler  -- chad(at)chadfowler.com
# *   David Black  -- dblack(at)wobblini.net
# *   Paul Brannan -- paul(at)atdesk.com
# *   Jim Weirich   -- jim(at)weirichhouse.org
#
#
# Contributors:
#
# *   Gavin Sinclair     -- gsinclair(at)soyabean.com.au
# *   George Marrows     -- george.marrows(at)ntlworld.com
# *   Dick Davies        -- rasputnik(at)hellooperator.net
# *   Mauricio Fernandez -- batsman.geo(at)yahoo.com
# *   Simon Strandgaard  -- neoneye(at)adslhome.dk
# *   Dave Glasser       -- glasser(at)mit.edu
# *   Paul Duncan        -- pabs(at)pablotron.org
# *   Ville Aine         -- vaine(at)cs.helsinki.fi
# *   Eric Hodel         -- drbrain(at)segment7.net
# *   Daniel Berger      -- djberg96(at)gmail.com
# *   Phil Hagelberg     -- technomancy(at)gmail.com
# *   Ryan Davis         -- ryand-ruby(at)zenspider.com
# *   Evan Phoenix       -- evan(at)fallingsnow.net
# *   Steve Klabnik      -- steve(at)steveklabnik.com
#
#
# (If your name is missing, PLEASE let us know!)
#
# ## License
#
# See LICENSE.txt for permissions.
#
# Thanks!
#
# -The RubyGems Team
module Gem
  DEFAULT_HOST = T.let(T.unsafe(nil), String)
  GEM_DEP_FILES = T.let(T.unsafe(nil), T::Array[T.untyped])
  GEM_PRELUDE_SUCKAGE = T.let(T.unsafe(nil), NilClass)
  LOADED_SPECS_MUTEX = T.let(T.unsafe(nil), Thread::Mutex)
  # Location of [`Marshal`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html)
  # quick gemspecs on remote repositories
  MARSHAL_SPEC_DIR = T.let(T.unsafe(nil), String)
  # [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html) classes
  # used in a
  # [`Gem.read_binary`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-read_binary)
  # `rescue` statement. Not all of these are defined in Ruby 1.8.7, hence the
  # need for this convoluted setup.
  READ_BINARY_ERRORS = T.let(T.unsafe(nil), T::Array[T.untyped])
  # Subdirectories in a gem repository for default gems
  REPOSITORY_DEFAULT_GEM_SUBDIRECTORIES = T.let(T.unsafe(nil), T::Array[T.untyped])
  # Subdirectories in a gem repository
  REPOSITORY_SUBDIRECTORIES = T.let(T.unsafe(nil), T::Array[T.untyped])
  RUBYGEMS_DIR = T.let(T.unsafe(nil), String)
  VERSION = T.let(T.unsafe(nil), String)
  # An [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of Regexps that
  # match windows Ruby platforms.
  WIN_PATTERNS = T.let(T.unsafe(nil), T::Array[T.untyped])
  # [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html) classes
  # used in
  # [`Gem.write_binary`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-write_binary)
  # `rescue` statement. Not all of these are defined in Ruby 1.8.7.
  WRITE_BINARY_ERRORS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # [`Find`](https://docs.ruby-lang.org/en/2.6.0/Find.html) the full path to the
  # executable for gem `name`. If the `exec_name` is not given, an exception
  # will be raised, otherwise the specified executable's path is returned.
  # `requirements` allows you to specify specific gem versions.
  sig do
    params(
        name: String,
        args: String,
        requirements: Gem::Requirement,
    )
    .returns(String)
  end
  def self.bin_path(name, args=T.unsafe(nil), *requirements); end

  # The mode needed to read a file as straight binary.
  sig {returns(String)}
  def self.binary_mode(); end

  # The path where gem executables are to be installed.
  sig do
    params(
        install_dir: String,
    )
    .returns(String)
  end
  def self.bindir(install_dir=T.unsafe(nil)); end

  # Clear default gem related variables. It is for test
  sig {returns(T::Hash[T.untyped, T.untyped])}
  def self.clear_default_specs(); end

  # Reset the `dir` and `path` values. The next time `dir` or `path` is
  # requested, the values will be calculated from scratch. This is mainly used
  # by the unit tests to provide test isolation.
  sig {returns(NilClass)}
  def self.clear_paths(); end

  # The path to standard location of the user's .gemrc file.
  sig {returns(String)}
  def self.config_file(); end

  # The standard configuration object for gems.
  sig {returns(T.untyped)}
  def self.configuration(); end

  # Use the given configuration object (which implements the ConfigFile
  # protocol) as the standard configuration object.
  sig do
    params(
        config: BasicObject,
    )
    .returns(T.untyped)
  end
  def self.configuration=(config); end

  # The path to the data directory specified by the gem name. If the package is
  # not available as a gem, return nil.
  sig do
    params(
        gem_name: String,
    )
    .returns(T.nilable(String))
  end
  def self.datadir(gem_name); end

  # The default directory for binaries
  sig {returns(T.nilable(String))}
  def self.default_bindir(); end

  # The default signing certificate chain path
  sig {returns(T.nilable(String))}
  def self.default_cert_path(); end

  # Default home directory path to be used if an alternate value is not
  # specified in the environment
  sig {returns(T.nilable(String))}
  def self.default_dir(); end

  # Deduce Ruby's --program-prefix and --program-suffix from its install name
  sig {returns(T.nilable(String))}
  def self.default_exec_format(); end

  # The default signing key path
  sig {returns(T.nilable(String))}
  def self.default_key_path(); end

  # Default gem load path
  sig {returns(T.nilable(String))}
  def self.default_path(); end

  # Paths where RubyGems' .rb files and bin files are installed
  sig {returns(T.nilable(T::Array[String]))}
  def self.default_rubygems_dirs(); end

  # An [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of the default
  # sources that come with RubyGems
  sig {returns(T.nilable(T::Array[String]))}
  def self.default_sources(); end
end

# [`BasicSpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/BasicSpecification.html)
# is an abstract class which implements some common code used by both
# Specification and StubSpecification.
class Gem::BasicSpecification < Object
end

class Gem::CommandLineError < Gem::Exception
end

# Raised when there are conflicting gem specs loaded
class Gem::ConflictError < Gem::LoadError
end

class Gem::Dependency < Object
  # Valid dependency types.
  TYPES = T.let(T.unsafe(nil), T::Array[T.untyped])
end

class Gem::DependencyError < Gem::Exception
end

class Gem::DependencyRemovalException < Gem::Exception
end

# Raised by
# [`Gem::Resolver`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver.html) when
# a Gem::Dependency::Conflict reaches the toplevel. Indicates which dependencies
# were incompatible through
# [`conflict`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyResolutionError.html#attribute-i-conflict)
# and
# [`conflicting_dependencies`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyResolutionError.html#method-i-conflicting_dependencies)
class Gem::DependencyResolutionError < Gem::DependencyError
end

# Provides a single method `deprecate` to be used to declare when something is
# going away.
#
# ```ruby
# class Legacy
#   def self.klass_method
#     # ...
#   end
#
#   def instance_method
#     # ...
#   end
#
#   extend Gem::Deprecate
#   deprecate :instance_method, "X.z", 2011, 4
#
#   class << self
#     extend Gem::Deprecate
#     deprecate :klass_method, :none, 2011, 4
#   end
# end
# ```
module Gem::Deprecate
end

class Gem::DocumentError < Gem::Exception
end

# Potentially raised when a specification is validated.
class Gem::EndOfYAMLException < Gem::Exception
end

class Gem::ErrorReason < Object
end

# Base exception class for RubyGems. All exception raised by RubyGems are a
# subclass of this one.
class Gem::Exception < RuntimeError
end

# Signals that a file permission error is preventing the user from operating on
# the given directory.
class Gem::FilePermissionError < Gem::Exception
end

# Used to raise parsing and loading errors
class Gem::FormatException < Gem::Exception
end

class Gem::GemNotFoundException < Gem::Exception
end

# Raised when attempting to uninstall a gem that isn't in GEM\_HOME.
class Gem::GemNotInHomeException < Gem::Exception
end

# Raised by
# [`Gem::Resolver`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver.html) when
# dependencies conflict and create the inability to find a valid possible spec
# for a request.
class Gem::ImpossibleDependenciesError < Gem::Exception
end

class Gem::InstallError < Gem::Exception
end

# Potentially raised when a specification is validated.
class Gem::InvalidSpecificationException < Gem::Exception
end

class Gem::List < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)
end

# Raised when RubyGems is unable to load or activate a gem. Contains the name
# and version requirements of the gem that either conflicts with already
# activated gems or that RubyGems is otherwise unable to activate.
class Gem::LoadError < LoadError
end

# Raised when trying to activate a gem, and that gem does not exist on the
# system. Instead of rescuing from this class, make sure to rescue from the
# superclass
# [`Gem::LoadError`](https://docs.ruby-lang.org/en/2.6.0/Gem/LoadError.html) to
# catch all types of load errors.
class Gem::MissingSpecError < Gem::LoadError
end

# Raised when trying to activate a gem, and the gem exists on the system, but
# not the requested version. Instead of rescuing from this class, make sure to
# rescue from the superclass
# [`Gem::LoadError`](https://docs.ruby-lang.org/en/2.6.0/Gem/LoadError.html) to
# catch all types of load errors.
class Gem::MissingSpecVersionError < Gem::MissingSpecError
end

class Gem::OperationNotSupportedError < Gem::Exception
end

# [`Gem::PathSupport`](https://docs.ruby-lang.org/en/2.6.0/Gem/PathSupport.html)
# facilitates the GEM\_HOME and GEM\_PATH environment settings to the rest of
# RubyGems.
class Gem::PathSupport < Object
end

# Available list of platforms for targeting
# [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html) installations.
#
# See `gem help platform` for information on platform matching.
class Gem::Platform < Object
  # A platform-specific gem that is built for the packaging Ruby's platform.
  # This will be replaced with
  # [`Gem::Platform::local`](https://docs.ruby-lang.org/en/2.6.0/Gem/Platform.html#method-c-local).
  CURRENT = T.let(T.unsafe(nil), String)
  # A pure-Ruby gem that may use
  # [`Gem::Specification#extensions`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-extensions)
  # to build binary files.
  RUBY = T.let(T.unsafe(nil), String)
end

# Generated when trying to lookup a gem to indicate that the gem was found, but
# that it isn't usable on the current platform.
#
# fetch and install read these and report them to the user to aid in figuring
# out why a gem couldn't be installed.
class Gem::PlatformMismatch < Gem::ErrorReason
end

# Signals that a remote operation cannot be conducted, probably due to not being
# connected (or just not finding host).
class Gem::RemoteError < Gem::Exception
end

class Gem::RemoteInstallationCancelled < Gem::Exception
end

class Gem::RemoteInstallationSkipped < Gem::Exception
end

# Represents an error communicating via HTTP.
class Gem::RemoteSourceException < Gem::Exception
end

# A [`Requirement`](https://docs.ruby-lang.org/en/2.6.0/Gem/Requirement.html) is
# a set of one or more version restrictions. It supports a few (`=, !=, >, <,
# >=, <=, ~>`) different restriction operators.
#
# See [`Gem::Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) for
# a description on how versions and requirements work together in RubyGems.
class Gem::Requirement < Object
  OPS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  # A regular expression that matches a requirement
  PATTERN = T.let(T.unsafe(nil), Regexp)
  PATTERN_RAW = T.let(T.unsafe(nil), String)
  SOURCE_SET_REQUIREMENT = T.let(T.unsafe(nil), Object)
end

# Raised when a bad requirement is encountered
class Gem::Requirement::BadRequirementError < ArgumentError
end

# Raised when a gem dependencies file specifies a ruby version that does not
# match the current version.
class Gem::RubyVersionMismatch < Gem::Exception
end

# An error that indicates we weren't able to fetch some data from a source
class Gem::SourceFetchProblem < Gem::ErrorReason
end

# The
# [`Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
# class contains the information for a
# [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html). Typically defined in a
# .gemspec file or a Rakefile, and looks like this:
#
# ```ruby
# Gem::Specification.new do |s|
#   s.name        = 'example'
#   s.version     = '0.1.0'
#   s.licenses    = ['MIT']
#   s.summary     = "This is an example!"
#   s.description = "Much longer explanation of the example!"
#   s.authors     = ["Ruby Coder"]
#   s.email       = 'rubycoder@example.com'
#   s.files       = ["lib/example.rb"]
#   s.homepage    = 'https://rubygems.org/gems/example'
#   s.metadata    = { "source_code_uri" => "https://github.com/example/example" }
# end
# ```
#
# Starting in RubyGems 2.0, a
# [`Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
# can hold arbitrary metadata. See
# [`metadata`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#attribute-i-metadata)
# for restrictions on the format and size of metadata items you may add to a
# specification.
class Gem::Specification < Gem::BasicSpecification
  Elem = type_template(fixed: T.untyped)

  CURRENT_SPECIFICATION_VERSION = T.let(T.unsafe(nil), Integer)
  EMPTY = T.let(T.unsafe(nil), T::Array[T.untyped])
  MARSHAL_FIELDS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  # The version number of a specification that does not specify one (i.e.
  # RubyGems 0.7 or earlier).
  NONEXISTENT_SPECIFICATION_VERSION = T.let(T.unsafe(nil), Integer)
  NOT_FOUND = T.let(T.unsafe(nil), Object)
  SPECIFICATION_VERSION_HISTORY = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  TODAY = T.let(T.unsafe(nil), Time)
  VALID_NAME_PATTERN = T.let(T.unsafe(nil), Regexp)
end

# Raised by the DependencyInstaller when a specific gem cannot be found
class Gem::SpecificGemNotFoundException < Gem::GemNotFoundException
end

# [`Gem::StubSpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/StubSpecification.html)
# reads the stub: line from the gemspec. This prevents us having to eval the
# entire gemspec in order to find out certain information.
class Gem::StubSpecification < Gem::BasicSpecification
  OPEN_MODE = T.let(T.unsafe(nil), String)
  PREFIX = T.let(T.unsafe(nil), String)
end

class Gem::StubSpecification::StubLine < Object
  NO_EXTENSIONS = T.let(T.unsafe(nil), T::Array[T.untyped])
  REQUIRE_PATHS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  REQUIRE_PATH_LIST = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
end

# Raised to indicate that a system exit should occur with the specified
# [`exit_code`](https://docs.ruby-lang.org/en/2.6.0/Gem/SystemExitException.html#attribute-i-exit_code)
class Gem::SystemExitException < SystemExit
end

# Raised by Resolver when a dependency requests a gem for which there is no
# spec.
class Gem::UnsatisfiableDependencyError < Gem::DependencyError
end

# Raised by
# [`Gem::Validator`](https://docs.ruby-lang.org/en/2.6.0/Gem/Validator.html)
# when something is not right in a gem.
class Gem::VerificationError < Gem::Exception
end

# The [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) class
# processes string versions into comparable values. A version string should
# normally be a series of numbers separated by periods. Each part (digits
# separated by periods) is considered its own number, and these are used for
# sorting. So for instance, 3.10 sorts higher than 3.2 because ten is greater
# than two.
#
# If any part contains letters (currently only a-z are supported) then that
# version is considered prerelease. Versions with a prerelease part in the Nth
# part sort less than versions with N-1 parts. Prerelease parts are sorted
# alphabetically using the normal Ruby string sorting rules. If a prerelease
# part contains both letters and numbers, it will be broken into multiple parts
# to provide expected sort behavior (1.0.a10 becomes 1.0.a.10, and is greater
# than 1.0.a9).
#
# Prereleases sort between real releases (newest to oldest):
#
# 1.  1.0
# 2.  1.0.b1
# 3.  1.0.a.2
# 4.  0.9
#
#
# If you want to specify a version restriction that includes both prereleases
# and regular releases of the 1.x series this is the best way:
#
# ```ruby
# s.add_dependency 'example', '>= 1.0.0.a', '< 2.0.0'
# ```
#
# ## How Software Changes
#
# Users expect to be able to specify a version constraint that gives them some
# reasonable expectation that new versions of a library will work with their
# software if the version constraint is true, and not work with their software
# if the version constraint is false. In other words, the perfect system will
# accept all compatible versions of the library and reject all incompatible
# versions.
#
# Libraries change in 3 ways (well, more than 3, but stay focused here!).
#
# 1.  The change may be an implementation detail only and have no effect on the
#     client software.
# 2.  The change may add new features, but do so in a way that client software
#     written to an earlier version is still compatible.
# 3.  The change may change the public interface of the library in such a way
#     that old software is no longer compatible.
#
#
# Some examples are appropriate at this point. Suppose I have a Stack class that
# supports a `push` and a `pop` method.
#
# ### Examples of Category 1 changes:
#
# *   Switch from an array based implementation to a linked-list based
#     implementation.
# *   Provide an automatic (and transparent) backing store for large stacks.
#
#
# ### Examples of Category 2 changes might be:
#
# *   Add a `depth` method to return the current depth of the stack.
# *   Add a `top` method that returns the current top of stack (without changing
#     the stack).
# *   Change `push` so that it returns the item pushed (previously it had no
#     usable return value).
#
#
# ### Examples of Category 3 changes might be:
#
# *   Changes `pop` so that it no longer returns a value (you must use `top` to
#     get the top of the stack).
# *   Rename the methods to `push_item` and `pop_item`.
#
#
# ## RubyGems [`Rational`](https://docs.ruby-lang.org/en/2.6.0/Rational.html) Versioning
#
# *   Versions shall be represented by three non-negative integers, separated by
#     periods (e.g. 3.1.4). The first integers is the "major" version number,
#     the second integer is the "minor" version number, and the third integer is
#     the "build" number.
#
# *   A category 1 change (implementation detail) will increment the build
#     number.
#
# *   A category 2 change (backwards compatible) will increment the minor
#     version number and reset the build number.
#
# *   A category 3 change (incompatible) will increment the major build number
#     and reset the minor and build numbers.
#
# *   Any "public" release of a gem should have a different version. Normally
#     that means incrementing the build number. This means a developer can
#     generate builds all day long, but as soon as they make a public release,
#     the version must be updated.
#
#
# ### Examples
#
# Let's work through a project lifecycle using our Stack example from above.
#
# [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) 0.0.1
# :   The initial Stack class is release.
# [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) 0.0.2
# :   Switched to a linked=list implementation because it is cooler.
# [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) 0.1.0
# :   Added a `depth` method.
# [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) 1.0.0
# :   Added `top` and made `pop` return nil (`pop` used to return the  old top
#     item).
# [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) 1.1.0
# :   `push` now returns the value pushed (it used it return nil).
# [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) 1.1.1
# :   Fixed a bug in the linked list implementation.
# [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) 1.1.2
# :   Fixed a bug introduced in the last fix.
#
#
# Client A needs a stack with basic push/pop capability. They write to the
# original interface (no `top`), so their version constraint looks like:
#
# ```ruby
# gem 'stack', '>= 0.0'
# ```
#
# Essentially, any version is OK with Client A. An incompatible change to the
# library will cause them grief, but they are willing to take the chance (we
# call Client A optimistic).
#
# Client B is just like Client A except for two things: (1) They use the `depth`
# method and (2) they are worried about future incompatibilities, so they write
# their version constraint like this:
#
# ```ruby
# gem 'stack', '~> 0.1'
# ```
#
# The `depth` method was introduced in version 0.1.0, so that version or
# anything later is fine, as long as the version stays below version 1.0 where
# incompatibilities are introduced. We call Client B pessimistic because they
# are worried about incompatible future changes (it is OK to be pessimistic!).
#
# ## Preventing [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) Catastrophe:
#
# From: http://blog.zenspider.com/2008/10/rubygems-howto-preventing-cata.html
#
# Let's say you're depending on the fnord gem version 2.y.z. If you specify your
# dependency as ">= 2.0.0" then, you're good, right? What happens if fnord 3.0
# comes out and it isn't backwards compatible with 2.y.z? Your stuff will break
# as a result of using ">=". The better route is to specify your dependency with
# an "approximate" version specifier ("~>"). They're a tad confusing, so here is
# how the dependency specifiers work:
#
# ```
# Specification From  ... To (exclusive)
# ">= 3.0"      3.0   ... &infin;
# "~> 3.0"      3.0   ... 4.0
# "~> 3.0.0"    3.0.0 ... 3.1
# "~> 3.5"      3.5   ... 4.0
# "~> 3.5.0"    3.5.0 ... 3.6
# "~> 3"        3.0   ... 4.0
# ```
#
# For the last example, single-digit versions are automatically extended with a
# zero to give a sensible result.
class Gem::Version < Object
  include Comparable

  ANCHORED_VERSION_PATTERN = T.let(T.unsafe(nil), Regexp)
  VERSION_PATTERN = T.let(T.unsafe(nil), String)

  sig do
    params(
      version: String,
    )
    .void
  end
  def initialize(version); end
end
