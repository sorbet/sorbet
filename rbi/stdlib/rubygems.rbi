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

  def self._deprecated_detect_gemdeps(path=T.unsafe(nil)); end

  def self._deprecated_gunzip(data); end

  def self._deprecated_gzip(data); end

  def self._deprecated_inflate(data); end

  def self.activate_bin_path(name, *args); end

  def self.add_to_load_path(*paths); end

  def self.default_ext_dir_for(base_dir); end

  def self.default_spec_cache_dir(); end

  def self.default_specifications_dir(); end

  def self.deflate(data); end

  def self.detect_gemdeps(*args, &block); end

  def self.dir(); end

  def self.done_installing(&hook); end

  def self.done_installing_hooks(); end

  def self.ensure_default_gem_subdirectories(dir=T.unsafe(nil), mode=T.unsafe(nil)); end

  def self.ensure_gem_subdirectories(dir=T.unsafe(nil), mode=T.unsafe(nil)); end

  def self.ensure_subdirectories(dir, mode, subdirs); end

  def self.env_requirement(gem_name); end

  def self.extension_api_version(); end

  def self.find_files(glob, check_load_path=T.unsafe(nil)); end

  def self.find_files_from_load_path(glob); end

  def self.find_latest_files(glob, check_load_path=T.unsafe(nil)); end

  def self.find_unresolved_default_spec(path); end

  def self.finish_resolve(*_); end

  def self.gemdeps(); end

  def self.gunzip(*args, &block); end

  def self.gzip(*args, &block); end

  def self.host(); end

  def self.host=(host); end

  def self.inflate(*args, &block); end

  def self.install(name, version=T.unsafe(nil), *options); end

  def self.install_extension_in_lib(); end

  def self.java_platform?(); end

  def self.latest_rubygems_version(); end

  def self.latest_spec_for(name); end

  def self.latest_version_for(name); end

  def self.load_env_plugins(); end

  def self.load_path_insert_index(); end

  def self.load_plugin_files(plugins); end

  def self.load_plugins(); end

  def self.load_yaml(); end

  def self.loaded_specs(); end

  def self.location_of_caller(depth=T.unsafe(nil)); end

  def self.marshal_version(); end

  def self.needs(); end

  def self.operating_system_defaults(); end

  def self.path(); end

  def self.path_separator(); end

  def self.paths(); end

  def self.paths=(env); end

  def self.platform_defaults(); end

  def self.platforms(); end

  def self.platforms=(platforms); end

  def self.post_build(&hook); end

  def self.post_build_hooks(); end

  def self.post_install(&hook); end

  def self.post_install_hooks(); end

  def self.post_reset(&hook); end

  def self.post_reset_hooks(); end

  def self.post_uninstall(&hook); end

  def self.post_uninstall_hooks(); end

  def self.pre_install(&hook); end

  def self.pre_install_hooks(); end

  def self.pre_reset(&hook); end

  def self.pre_reset_hooks(); end

  def self.pre_uninstall(&hook); end

  def self.pre_uninstall_hooks(); end

  def self.prefix(); end

  def self.read_binary(path); end

  def self.refresh(); end

  def self.register_default_spec(spec); end

  def self.ruby(); end

  def self.ruby_api_version(); end

  def self.ruby_engine(); end

  def self.ruby_version(); end

  def self.rubygems_version(); end

  def self.source_date_epoch(); end

  def self.source_date_epoch_string(); end

  def self.sources(); end

  def self.sources=(new_sources); end

  def self.spec_cache_dir(); end

  def self.suffix_pattern(); end

  def self.suffix_regexp(); end

  def self.suffixes(); end

  def self.time(msg, width=T.unsafe(nil), display=T.unsafe(nil)); end

  def self.try_activate(path); end

  def self.ui(); end

  def self.use_gemdeps(path=T.unsafe(nil)); end

  def self.use_paths(home, *paths); end

  def self.user_dir(); end

  def self.user_home(); end

  def self.vendor_dir(); end

  def self.win_platform?(); end

  def self.write_binary(path, data); end
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
  def conflicts(); end

  def initialize(target, conflicts); end

  def target(); end
end

class Gem::Dependency < Object
  # Valid dependency types.
  TYPES = T.let(T.unsafe(nil), T::Array[T.untyped])

  def ==(other); end

  def ===(other); end

  def =~(other); end

  def all_sources(); end

  def all_sources=(all_sources); end

  def encode_with(coder); end

  def eql?(other); end

  def groups(); end

  def groups=(groups); end

  def identity(); end

  def initialize(name, *requirements); end

  def latest_version?(); end

  def match?(obj, version=T.unsafe(nil), allow_prerelease=T.unsafe(nil)); end

  def matches_spec?(spec); end

  def matching_specs(platform_only=T.unsafe(nil)); end

  def merge(other); end

  def name(); end

  def name=(name); end

  def prerelease=(prerelease); end

  def prerelease?(); end

  def requirement(); end

  def requirements_list(); end

  def runtime?(); end

  def source(); end

  def source=(source); end

  def specific?(); end

  def to_lock(); end

  def to_spec(); end

  def to_specs(); end

  def to_yaml_properties(); end

  def type(); end
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
  def conflict(); end

  def conflicting_dependencies(); end

  def initialize(conflict); end
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
  def self.deprecate(name, repl, year, month); end

  def self.skip(); end

  def self.skip=(v); end

  def self.skip_during(); end
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
  def _deprecated_source_exception(); end

  def source_exception(*args, &block); end

  def source_exception=(source_exception); end
end

# Signals that a file permission error is preventing the user from operating on
# the given directory.
class Gem::FilePermissionError < Gem::Exception
  def directory(); end

  def initialize(directory); end
end

# Used to raise parsing and loading errors
class Gem::FormatException < Gem::Exception
  def file_path(); end

  def file_path=(file_path); end
end

class Gem::GemNotFoundException < Gem::Exception
end

# Raised when attempting to uninstall a gem that isn't in GEM\_HOME.
class Gem::GemNotInHomeException < Gem::Exception
  def spec(); end

  def spec=(spec); end
end

# Raised by
# [`Gem::Resolver`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver.html) when
# dependencies conflict and create the inability to find a valid possible spec
# for a request.
class Gem::ImpossibleDependenciesError < Gem::Exception
  def build_message(); end

  def conflicts(); end

  def dependency(); end

  def initialize(request, conflicts); end

  def request(); end
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

  def each(&blk); end

  def initialize(value=T.unsafe(nil), tail=T.unsafe(nil)); end

  def prepend(value); end

  def tail(); end

  def tail=(tail); end

  def to_a(); end

  def value(); end

  def value=(value); end

  def self.prepend(list, value); end
end

# Raised when RubyGems is unable to load or activate a gem. Contains the name
# and version requirements of the gem that either conflicts with already
# activated gems or that RubyGems is otherwise unable to activate.
class Gem::LoadError < LoadError
  def name(); end

  def name=(name); end

  def requirement(); end

  def requirement=(requirement); end
end

# Raised when trying to activate a gem, and that gem does not exist on the
# system. Instead of rescuing from this class, make sure to rescue from the
# superclass
# [`Gem::LoadError`](https://docs.ruby-lang.org/en/2.6.0/Gem/LoadError.html) to
# catch all types of load errors.
class Gem::MissingSpecError < Gem::LoadError
  def initialize(name, requirement); end
end

# Raised when trying to activate a gem, and the gem exists on the system, but
# not the requested version. Instead of rescuing from this class, make sure to
# rescue from the superclass
# [`Gem::LoadError`](https://docs.ruby-lang.org/en/2.6.0/Gem/LoadError.html) to
# catch all types of load errors.
class Gem::MissingSpecVersionError < Gem::MissingSpecError
  def initialize(name, requirement, specs); end

  def specs(); end
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

  JAVA = ::T.let(nil, ::T.untyped)
  MINGW = ::T.let(nil, ::T.untyped)
  MSWIN = ::T.let(nil, ::T.untyped)
  MSWIN64 = ::T.let(nil, ::T.untyped)
  X64_MINGW = ::T.let(nil, ::T.untyped)

  def ==(other); end

  def ===(other); end

  def =~(other); end

  def cpu(); end

  def cpu=(cpu); end

  def eql?(other); end

  def initialize(arch); end

  def os(); end

  def os=(os); end

  def to_a(); end

  def version(); end

  def version=(version); end

  def self.installable?(spec); end

  def self.local(); end

  def self.match(platform); end

  def self.new(arch); end
end

# Generated when trying to lookup a gem to indicate that the gem was found, but
# that it isn't usable on the current platform.
#
# fetch and install read these and report them to the user to aid in figuring
# out why a gem couldn't be installed.
class Gem::PlatformMismatch < Gem::ErrorReason
  def add_platform(platform); end

  def initialize(name, version); end

  def name(); end

  def platforms(); end

  def version(); end

  def wordy(); end
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
  DefaultPrereleaseRequirement = ::T.let(nil, ::T.untyped)
  DefaultRequirement = ::T.let(nil, ::T.untyped)

  def ==(other); end

  def ===(version); end

  def =~(version); end

  def _tilde_requirements(); end

  def as_list(); end

  def concat(new); end

  def encode_with(coder); end

  def exact?(); end

  def for_lockfile(); end

  def init_with(coder); end

  def initialize(*requirements); end

  def marshal_dump(); end

  def marshal_load(array); end

  def none?(); end

  def prerelease?(); end

  def requirements(); end

  def satisfied_by?(version); end

  def specific?(); end

  def to_yaml_properties(); end

  def yaml_initialize(tag, vals); end

  def self.create(*inputs); end

  def self.default(); end

  def self.default_prerelease(); end

  def self.parse(obj); end

  def self.source_set(); end
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

  DateLike = ::T.let(nil, ::T.untyped)
  DateTimeFormat = ::T.let(nil, ::T.untyped)
  INITIALIZE_CODE_FOR_DEFAULTS = ::T.let(nil, ::T.untyped)
  LOAD_CACHE_MUTEX = ::T.let(nil, ::T.untyped)

  def ==(other); end

  def _deprecated_default_executable(); end

  def _deprecated_default_executable=(_deprecated_default_executable); end

  def _deprecated_has_rdoc(); end

  def _deprecated_has_rdoc=(ignored); end

  def _deprecated_has_rdoc?(*args, &block); end

  def _deprecated_rubyforge_project=(_deprecated_rubyforge_project); end

  def _dump(limit); end

  def abbreviate(); end

  def activate(); end

  def activate_dependencies(); end

  def activated(); end

  def activated=(activated); end

  def add_bindir(executables); end

  def add_dependency(gem, *requirements); end

  def add_development_dependency(gem, *requirements); end

  def add_runtime_dependency(gem, *requirements); end

  def add_self_to_load_path(); end

  def author(); end

  def author=(o); end

  def authors(); end

  def authors=(value); end

  def autorequire(); end

  def autorequire=(autorequire); end

  def bin_dir(); end

  def bin_file(name); end

  def bindir(); end

  def bindir=(bindir); end

  def build_args(); end

  def build_extensions(); end

  def build_info_dir(); end

  def build_info_file(); end

  def cache_dir(); end

  def cache_file(); end

  def cert_chain(); end

  def cert_chain=(cert_chain); end

  def conficts_when_loaded_with?(list_of_specs); end

  def conflicts(); end

  def date(); end

  def date=(date); end

  def default_executable(*args, &block); end

  def default_executable=(*args, &block); end

  def default_value(name); end

  def dependencies(); end

  def dependent_gems(); end

  def dependent_specs(); end

  def description(); end

  def description=(str); end

  def development_dependencies(); end

  def doc_dir(type=T.unsafe(nil)); end

  def email(); end

  def email=(email); end

  def encode_with(coder); end

  def eql?(other); end

  def executable(); end

  def executable=(o); end

  def executables(); end

  def executables=(value); end

  def extensions(); end

  def extensions=(extensions); end

  def extra_rdoc_files(); end

  def extra_rdoc_files=(files); end

  def file_name(); end

  def files(); end

  def files=(files); end

  def for_cache(); end

  def git_version(); end

  def groups(); end

  def has_conflicts?(); end

  def has_rdoc(*args, &block); end

  def has_rdoc=(*args, &block); end

  def has_rdoc?(*args, &block); end

  def has_test_suite?(); end

  def has_unit_tests?(); end

  def homepage(); end

  def homepage=(homepage); end

  def init_with(coder); end

  def initialize(name=T.unsafe(nil), version=T.unsafe(nil)); end

  def installed_by_version(); end

  def installed_by_version=(version); end

  def keep_only_files_and_directories(); end

  def lib_files(); end

  def license(); end

  def license=(o); end

  def licenses(); end

  def licenses=(licenses); end

  def load_paths(); end

  def location(); end

  def location=(location); end

  def mark_version(); end

  def metadata(); end

  def metadata=(metadata); end

  def method_missing(sym, *a, &b); end

  def missing_extensions?(); end

  def name=(name); end

  def name_tuple(); end

  def nondevelopment_dependencies(); end

  def normalize(); end

  def original_name(); end

  def original_platform(); end

  def original_platform=(original_platform); end

  def platform=(platform); end

  def post_install_message(); end

  def post_install_message=(post_install_message); end

  def raise_if_conflicts(); end

  def rdoc_options(); end

  def rdoc_options=(options); end

  def relative_loaded_from(); end

  def relative_loaded_from=(relative_loaded_from); end

  def remote(); end

  def remote=(remote); end

  def require_path(); end

  def require_path=(path); end

  def require_paths=(val); end

  def required_ruby_version(); end

  def required_ruby_version=(req); end

  def required_rubygems_version(); end

  def required_rubygems_version=(req); end

  def requirements(); end

  def requirements=(req); end

  def reset_nil_attributes_to_default(); end

  def rg_extension_dir(); end

  def rg_full_gem_path(); end

  def rg_loaded_from(); end

  def ri_dir(); end

  def rubyforge_project=(*args, &block); end

  def rubygems_version(); end

  def rubygems_version=(rubygems_version); end

  def runtime_dependencies(); end

  def sanitize(); end

  def sanitize_string(string); end

  def satisfies_requirement?(dependency); end

  def signing_key(); end

  def signing_key=(signing_key); end

  def sort_obj(); end

  def source(); end

  def source=(source); end

  def spec_dir(); end

  def spec_file(); end

  def spec_name(); end

  def specification_version(); end

  def specification_version=(specification_version); end

  def summary(); end

  def summary=(str); end

  def test_file(); end

  def test_file=(file); end

  def test_files(); end

  def test_files=(files); end

  def to_gemfile(path=T.unsafe(nil)); end

  def to_ruby_for_cache(); end

  def to_yaml(opts=T.unsafe(nil)); end

  def traverse(trail=T.unsafe(nil), visited=T.unsafe(nil), &block); end

  def validate(packaging=T.unsafe(nil), strict=T.unsafe(nil)); end

  def validate_dependencies(); end

  def validate_metadata(); end

  def validate_permissions(); end

  def version=(version); end

  def yaml_initialize(tag, vals); end

  def self._all(); end

  def self._clear_load_cache(); end

  def self._latest_specs(specs, prerelease=T.unsafe(nil)); end

  def self._load(str); end

  def self._resort!(specs); end

  def self.all(); end

  def self.all=(specs); end

  def self.all_names(); end

  def self.array_attributes(); end

  def self.attribute_names(); end

  def self.default_stubs(pattern=T.unsafe(nil)); end

  def self.dirs(); end

  def self.dirs=(dirs); end

  def self.each(&blk); end

  def self.each_gemspec(dirs); end

  def self.each_spec(dirs); end

  def self.find_active_stub_by_path(path); end

  def self.find_all_by_full_name(full_name); end

  def self.find_all_by_name(name, *requirements); end

  def self.find_by_name(name, *requirements); end

  def self.find_by_path(path); end

  def self.find_in_unresolved(path); end

  def self.find_in_unresolved_tree(path); end

  def self.find_inactive_by_path(path); end

  def self.from_yaml(input); end

  def self.latest_specs(prerelease=T.unsafe(nil)); end

  def self.load(file); end

  def self.load_defaults(); end

  def self.non_nil_attributes(); end

  def self.normalize_yaml_input(input); end

  def self.outdated(); end

  def self.outdated_and_latest_version(); end

  def self.required_attribute?(name); end

  def self.required_attributes(); end

  def self.reset(); end

  def self.stubs(); end

  def self.stubs_for(name); end

  def self.unresolved_deps(); end
end

# Raised by the DependencyInstaller when a specific gem cannot be found
class Gem::SpecificGemNotFoundException < Gem::GemNotFoundException
  def errors(); end

  def initialize(name, version, errors=T.unsafe(nil)); end

  def name(); end

  def version(); end
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
  def exit_code(); end

  def exit_code=(exit_code); end

  def initialize(exit_code); end
end

# Raised by Resolver when a dependency requests a gem for which there is no
# spec.
class Gem::UnsatisfiableDependencyError < Gem::DependencyError
  def dependency(); end

  def errors(); end

  def errors=(errors); end

  def initialize(dep, platform_mismatch=T.unsafe(nil)); end

  def name(); end

  def version(); end
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

  def _segments(); end

  def _split_segments(); end

  def _version(); end

  def approximate_recommendation(); end

  def bump(); end

  def canonical_segments(); end

  def encode_with(coder); end

  def eql?(other); end

  def init_with(coder); end

  def marshal_dump(); end

  def marshal_load(array); end

  def prerelease?(); end

  def release(); end

  def segments(); end

  def to_yaml_properties(); end

  def version(); end

  def yaml_initialize(tag, map); end

  def self.correct?(version); end

  def self.create(input); end

  def self.new(version); end
end

class Gem::Package
  include Gem::UserInteraction

  def build_time; end

  def build_time=(build_time); end

  def checksums; end

  def files; end

  def security_policy; end

  def security_policy=(security_policy); end

  def spec=(spec); end

  def dir_mode; end

  def dir_mode=(dir_mode); end

  def prog_mode; end

  def prog_mode=(prog_mode); end

  def data_mode; end

  def data_mode=(data_mode); end

  def self.build(spec, skip_validation = false, strict_validation = false, file_name = nil); end

  def initialize(gem, security_policy = nil); end

  def copy_to(path); end

  def add_checksums(tar); end

  def add_contents(tar); end

  def add_files(tar); end

  def add_metadata(tar); end

  def build(skip_validation = false, strict_validation = false); end

  def contents; end

  def digest(entry); end

  def extract_files(destination_dir, pattern = "*"); end

  def extract_tar_gz(io, destination_dir, pattern = "*"); end

  def file_mode(mode); end

  def gzip_to(io); end

  def install_location(filename, destination_dir); end

  def normalize_path(pathname); end

  def mkdir_p_safe(mkdir, mkdir_options, destination_dir, file_name); end

  def load_spec(entry); end

  def open_tar_gz(io); end

  def read_checksums(gem); end

  def setup_signer(signer_options: {}); end

  def spec; end

  def verify; end

  def verify_checksums(digests, checksums); end

  def verify_entry(entry); end

  def verify_files(gem); end

  def verify_gz(entry); end
end

class Gem::Package::Error < Gem::Exception; end

class Gem::Package::FormatError < Gem::Package::Error
  def path; end

  def initialize(message, source = nil); end
end

class Gem::Package::PathError < Gem::Package::Error
  def initialize(destination, destination_dir); end
end

class Gem::Package::NonSeekableIO < Gem::Package::Error
end

class Gem::Package::TooLongFileName < Gem::Package::Error
end

class Gem::Package::TarInvalidError < Gem::Package::Error
end

class Gem::Package::TarReader
  include Enumerable

  Elem = type_member(fixed: T.untyped)

  def initialize(io); end

  def close; end

  def each; end

  def each_entry; end

  def rewind; end

  def seek(name); end
end

class Gem::Package::TarReader::UnexpectedEOF < StandardError
end

class Gem::Package::TarWriter
  def initialize(io); end

  def add_file(name, mode); end

  def add_file_digest(name, mode, digest_algorithms); end

  def add_file_signed(name, mode, signer); end

  def add_file_simple(name, mode, size); end

  def add_symlink(name, target, mode); end

  def check_closed; end

  def close; end

  def closed?; end

  def flush; end

  def mkdir(name, mode); end

  def split_name(name); end
end

class Gem::Package::TarWriter::FileOverflow < StandardError
end

class Gem::Package::TarWriter::BoundedStream
  def limit; end

  def written; end

  def initialize(io, limit); end

  def write(data); end
end

class Gem::Package::TarWriter::RestrictedStream
  def initialize(io); end

  def write(data); end
end

module Gem::DefaultUserInteraction
  include Gem::Text

  def self.ui; end

  def self.ui=(new_ui); end

  def self.use_ui(new_ui); end

  def ui; end

  def ui=(new_ui); end

  def use_ui(new_ui, &block); end
end

module Gem::UserInteraction
  include Gem::DefaultUserInteraction

  def alert(statement, question = nil); end

  def alert_error(statement, question = nil); end

  def alert_warning(statement, question = nil); end

  def ask(question); end

  def ask_for_password(prompt); end

  def ask_yes_no(question, default = nil); end

  def choose_from_list(question, list); end

  def say(statement = ''); end

  def terminate_interaction(exit_code = 0); end

  def verbose(msg = nil); end
end

class Gem::StreamUI
  extend Gem::Deprecate

  def ins; end

  def outs; end

  def errs; end

  def initialize(in_stream, out_stream, err_stream=STDERR, usetty=true); end

  def tty?; end

  def backtrace(exception); end

  def choose_from_list(question, list); end

  def ask_yes_no(question, default=nil); end

  def ask(question); end

  def ask_for_password(question); end

  def require_io_console; end

  def _gets_noecho; end

  def say(statement=""); end

  def alert(statement, question=nil); end

  def alert_warning(statement, question=nil); end

  def alert_error(statement, question=nil); end

  def debug(statement); end

  def terminate_interaction(status = 0); end

  def close; end

  def progress_reporter(*args); end

  def download_reporter(*args); end
end

class Gem::Stream::UI::SilentProgressReporter
  def count; end

  def initialize(out_stream, size, initial_message, terminal_message = nil); end

  def updated(message); end

  def done; end
end

class Gem::Stream::UI::SimpleProgressReporter
  include Gem::DefaultUserInteraction

  def count; end

  def initialize(out_stream, size, initial_message, terminal_message = "complete"); end

  def updated(message); end

  def done; end
end

class Gem::Stream::UI::VerboseProgressReporter
  include Gem::DefaultUserInteraction

  def count; end

  def initialize(out_stream, size, initial_message, terminal_message = 'complete'); end

  def updated(message); end

  def done; end
end

class Gem::Stream::UI::SilentDownloadReporter
  def initialize(out_stream, *args); end

  def fetch(filename, filesize); end

  def update(current); end

  def done; end
end

class Gem::Stream::UI::ThreadedDownloadReporter
  MUTEX = Mutex.new

  def file_name; end

  def initialize(out_stream, *args); end

  def fetch(file_name, *args); end

  def update(bytes); end

  def done; end
end

class Gem::ConsoleUI < Gem::StreamUI
  def initialize; end
end

class Gem::SilentUI < Gem::StreamUI
  def initialize; end

  def close; end

  def download_reporter(*args);
  end

  def progress_reporter(*args);
  end
end

module Gem::Text
  def clean_text(text); end

  def truncate_text(text, description, max_length = 100_000); end

  def format_text(text, wrap, indent=0); end

  def min3(a, b, c); end

  def levenshtein_distance(str1, str2); end
end

class Gem::AvailableSet
  include Enumerable

  Elem = type_member(fixed: T.untyped)

  def <<(o); end

  def add(spec, source); end

  def all_specs(); end

  def each(&blk); end

  def each_spec(); end

  def empty?(); end

  def find_all(req); end

  def inject_into_list(dep_list); end

  def match_platform!(); end

  def pick_best!(); end

  def prefetch(reqs); end

  def remote(); end

  def remote=(remote); end

  def remove_installed!(dep); end

  def set(); end

  def size(); end

  def sorted(); end

  def source_for(spec); end

  def to_request_set(development=T.unsafe(nil)); end
end

class Gem::AvailableSet::Tuple < Struct
  Elem = type_member(:out, fixed: T.untyped)

  def source(); end

  def source=(_); end

  def spec(); end

  def spec=(_); end

  def self.[](*_); end

  def self.members(); end
end

class Gem::BasicSpecification
  def activated?(); end

  def base_dir(); end

  def base_dir=(base_dir); end

  def contains_requirable_file?(file); end

  def datadir(); end

  def default_gem?(); end

  def extension_dir(); end

  def extension_dir=(extension_dir); end

  def extensions_dir(); end

  def full_gem_path(); end

  def full_gem_path=(full_gem_path); end

  def full_name(); end

  def full_require_paths(); end

  def gem_build_complete_path(); end

  def gem_dir(); end

  def gems_dir(); end

  def ignored=(ignored); end

  def internal_init(); end

  def lib_dirs_glob(); end

  def loaded_from(); end

  def loaded_from=(loaded_from); end

  def matches_for_glob(glob); end

  def name(); end

  def platform(); end

  def raw_require_paths(); end

  def require_paths(); end

  def source_paths(); end

  def stubbed?(); end

  def this(); end

  def to_fullpath(path); end

  def to_spec(); end

  def version(); end

  def self._deprecated_default_specifications_dir(); end

  def self.default_specifications_dir(*args, &block); end
end

module Gem::BundlerVersionFinder
  def self.bundler_version(); end

  def self.bundler_version_with_reason(); end

  def self.compatible?(spec); end

  def self.filter!(specs); end

  def self.missing_version_message(); end
end

class Gem::Command
  include ::Gem::UserInteraction
  include ::Gem::DefaultUserInteraction
  include ::Gem::Text

  def add_extra_args(args); end

  def add_option(*opts, &handler); end

  def arguments(); end

  def begins?(long, short); end

  def check_deprecated_options(options); end

  def command(); end

  def defaults(); end

  def defaults=(defaults); end

  def defaults_str(); end

  def deprecate_option(name, version: T.unsafe(nil), extra_msg: T.unsafe(nil)); end

  def description(); end

  def execute(); end

  def get_all_gem_names(); end

  def get_all_gem_names_and_versions(); end

  def get_one_gem_name(); end

  def get_one_optional_argument(); end

  def handle_options(args); end

  def handles?(args); end

  def initialize(command, summary=T.unsafe(nil), defaults=T.unsafe(nil)); end

  def invoke(*args); end

  def invoke_with_build_args(args, build_args); end

  def merge_options(new_options); end

  def options(); end

  def program_name(); end

  def program_name=(program_name); end

  def remove_option(name); end

  def show_help(); end

  def show_lookup_failure(gem_name, version, errors, suppress_suggestions=T.unsafe(nil), required_by=T.unsafe(nil)); end

  def summary(); end

  def summary=(summary); end

  def usage(); end

  def when_invoked(&block); end

  def self.add_common_option(*args, &handler); end

  def self.add_specific_extra_args(cmd, args); end

  def self.build_args(); end

  def self.build_args=(value); end

  def self.common_options(); end

  def self.extra_args(); end

  def self.extra_args=(value); end

  def self.specific_extra_args(cmd); end

  def self.specific_extra_args_hash(); end

  HELP = ::T.let(nil, ::T.untyped)
end

module Gem::Commands
end

class Gem::ConfigFile
  include ::Gem::UserInteraction
  include ::Gem::DefaultUserInteraction
  include ::Gem::Text

  def ==(other); end

  def [](key); end

  def []=(key, value); end

  def api_keys(); end

  def args(); end

  def backtrace(); end

  def backtrace=(backtrace); end

  def bulk_threshold(); end

  def bulk_threshold=(bulk_threshold); end

  def cert_expiration_length_days(); end

  def cert_expiration_length_days=(cert_expiration_length_days); end

  def check_credentials_permissions(); end

  def concurrent_downloads(); end

  def concurrent_downloads=(concurrent_downloads); end

  def config_file_name(); end

  def credentials_path(); end

  def disable_default_gem_server(); end

  def disable_default_gem_server=(disable_default_gem_server); end

  def each(&block); end

  def handle_arguments(arg_list); end

  def home(); end

  def home=(home); end

  def initialize(args); end

  def load_api_keys(); end

  def load_file(filename); end

  def path(); end

  def path=(path); end

  def really_verbose(); end

  def rubygems_api_key(); end

  def rubygems_api_key=(api_key); end

  def set_api_key(host, api_key); end

  def sources(); end

  def sources=(sources); end

  def ssl_ca_cert(); end

  def ssl_ca_cert=(ssl_ca_cert); end

  def ssl_client_cert(); end

  def ssl_verify_mode(); end

  def to_yaml(); end

  def unset_api_key!(); end

  def update_sources(); end

  def update_sources=(update_sources); end

  def verbose(); end

  def verbose=(verbose); end

  def write(); end

  DEFAULT_BACKTRACE = ::T.let(nil, ::T.untyped)
  DEFAULT_BULK_THRESHOLD = ::T.let(nil, ::T.untyped)
  DEFAULT_CERT_EXPIRATION_LENGTH_DAYS = ::T.let(nil, ::T.untyped)
  DEFAULT_CONCURRENT_DOWNLOADS = ::T.let(nil, ::T.untyped)
  DEFAULT_UPDATE_SOURCES = ::T.let(nil, ::T.untyped)
  DEFAULT_VERBOSITY = ::T.let(nil, ::T.untyped)
  OPERATING_SYSTEM_DEFAULTS = ::T.let(nil, ::T.untyped)
  PLATFORM_DEFAULTS = ::T.let(nil, ::T.untyped)
  SYSTEM_CONFIG_PATH = ::T.let(nil, ::T.untyped)
  SYSTEM_WIDE_CONFIG_FILE = ::T.let(nil, ::T.untyped)
end

class Gem::DependencyInstaller
  include ::Gem::UserInteraction
  include ::Gem::DefaultUserInteraction
  include ::Gem::Text
  extend ::Gem::Deprecate

  def _deprecated_available_set_for(dep_or_name, version); end

  def _deprecated_find_gems_with_sources(dep, best_only=T.unsafe(nil)); end

  def _deprecated_find_spec_by_name_and_version(gem_name, version=T.unsafe(nil), prerelease=T.unsafe(nil)); end

  def available_set_for(*args, &block); end

  def consider_local?(); end

  def consider_remote?(); end

  def document(); end

  def errors(); end

  def find_gems_with_sources(*args, &block); end

  def find_spec_by_name_and_version(*args, &block); end

  def in_background(what); end

  def initialize(options=T.unsafe(nil)); end

  def install(dep_or_name, version=T.unsafe(nil)); end

  def install_development_deps(); end

  def installed_gems(); end

  def resolve_dependencies(dep_or_name, version); end

  DEFAULT_OPTIONS = ::T.let(nil, ::T.untyped)
end

class Gem::DependencyList
  include Enumerable
  include ::TSort

  Elem = type_member(fixed: T.untyped)

  def add(*gemspecs); end

  def clear(); end

  def dependency_order(); end

  def development(); end

  def development=(development); end

  def each(&block); end

  def find_name(full_name); end

  def initialize(development=T.unsafe(nil)); end

  def ok?(); end

  def ok_to_remove?(full_name, check_dev=T.unsafe(nil)); end

  def remove_by_name(full_name); end

  def remove_specs_unsatisfied_by(dependencies); end

  def spec_predecessors(); end

  def specs(); end

  def tsort_each_node(&block); end

  def why_not_ok?(quick=T.unsafe(nil)); end

  def self.from_specs(); end
end

class Gem::Ext::Builder
  include ::Gem::UserInteraction
  include ::Gem::DefaultUserInteraction
  include ::Gem::Text

  CHDIR_MONITOR = ::T.let(nil, ::T.untyped)
  CHDIR_MUTEX = ::T.let(nil, ::T.untyped)

  def build_args(); end

  def build_args=(build_args); end

  def build_error(output, backtrace=T.unsafe(nil)); end

  def build_extension(extension, dest_path); end

  def build_extensions(); end

  def builder_for(extension); end

  def initialize(spec, build_args=T.unsafe(nil)); end

  def write_gem_make_out(output); end

  def self.class_name(); end

  def self.make(dest_path, results); end

  def self.run(command, results, command_name=T.unsafe(nil)); end
end

class Gem::Ext::CmakeBuilder < Gem::Ext::Builder
  def self.build(extension, dest_path, results, args=T.unsafe(nil), lib_dir=T.unsafe(nil)); end
end

class Gem::Ext::ConfigureBuilder < Gem::Ext::Builder
  def self.build(extension, dest_path, results, args=T.unsafe(nil), lib_dir=T.unsafe(nil)); end
end

class Gem::Ext::RakeBuilder < Gem::Ext::Builder
  def self.build(extension, dest_path, results, args=T.unsafe(nil), lib_dir=T.unsafe(nil)); end
end

module Gem::Ext
end

class Gem::Installer
  include ::Gem::UserInteraction
  include ::Gem::DefaultUserInteraction
  include ::Gem::Text
  extend ::Gem::Deprecate

  ENV_PATHS = ::T.let(nil, ::T.untyped)

  def _deprecated_extension_build_error(build_dir, output, backtrace=T.unsafe(nil)); end

  def _deprecated_unpack(directory); end

  def app_script_text(bin_file_name); end

  def bin_dir(); end

  def build_extensions(); end

  def build_root(); end

  def check_executable_overwrite(filename); end

  def check_that_user_bin_dir_is_in_path(); end

  def default_spec_file(); end

  def dir(); end

  def ensure_dependencies_met(); end

  def ensure_dependency(spec, dependency); end

  def ensure_loadable_spec(); end

  def ensure_required_ruby_version_met(); end

  def ensure_required_rubygems_version_met(); end

  def extension_build_error(*args, &block); end

  def extract_bin(); end

  def extract_files(); end

  def formatted_program_filename(filename); end

  def gem(); end

  def gem_dir(); end

  def gem_home(); end

  def generate_bin(); end

  def generate_bin_script(filename, bindir); end

  def generate_bin_symlink(filename, bindir); end

  def generate_windows_script(filename, bindir); end

  def initialize(package, options=T.unsafe(nil)); end

  def install(); end

  def installation_satisfies_dependency?(dependency); end

  def installed_specs(); end

  def options(); end

  def package(); end

  def pre_install_checks(); end

  def process_options(); end

  def run_post_build_hooks(); end

  def run_post_install_hooks(); end

  def run_pre_install_hooks(); end

  def shebang(bin_file_name); end

  def spec(); end

  def spec_file(); end

  def unpack(*args, &block); end

  def verify_gem_home(); end

  def verify_spec(); end

  def windows_stub_script(bindir, bin_file_name); end

  def write_build_info_file(); end

  def write_cache_file(); end

  def write_default_spec(); end

  def write_spec(); end

  def self.at(path, options=T.unsafe(nil)); end

  def self.exec_format(); end

  def self.exec_format=(exec_format); end

  def self.for_spec(spec, options=T.unsafe(nil)); end

  def self.install_lock(); end

  def self.path_warning(); end

  def self.path_warning=(path_warning); end
end

class Gem::Licenses
  extend ::Gem::Text

  EXCEPTION_IDENTIFIERS = ::T.let(nil, ::T.untyped)
  LICENSE_IDENTIFIERS = ::T.let(nil, ::T.untyped)
  NONSTANDARD = ::T.let(nil, ::T.untyped)
  REGEXP = ::T.let(nil, ::T.untyped)

  def self.match?(license); end

  def self.suggestions(license); end
end

class Gem::NameTuple
  include ::Comparable

  def ==(other); end

  def eql?(other); end

  def full_name(); end

  def initialize(name, version, platform=T.unsafe(nil)); end

  def match_platform?(); end

  def name(); end

  def platform(); end

  def prerelease?(); end

  def spec_name(); end

  def to_a(); end

  def version(); end

  def self.from_list(list); end

  def self.null(); end

  def self.to_basic(list); end
end

class Gem::RemoteFetcher
  include ::Gem::UserInteraction
  include ::Gem::DefaultUserInteraction
  include ::Gem::Text
  include ::Gem::UriParsing
  extend ::Gem::Deprecate

  def self.fetcher(); end

  def _deprecated_fetch_size(uri); end

  def cache_update_path(uri, path=T.unsafe(nil), update=T.unsafe(nil)); end

  def close_all(); end

  def download(spec, source_uri, install_dir=T.unsafe(nil)); end

  def download_to_cache(dependency); end

  def fetch_file(uri, *_); end

  def fetch_http(uri, last_modified=T.unsafe(nil), head=T.unsafe(nil), depth=T.unsafe(nil)); end

  def fetch_https(uri, last_modified=T.unsafe(nil), head=T.unsafe(nil), depth=T.unsafe(nil)); end

  def fetch_path(uri, mtime=T.unsafe(nil), head=T.unsafe(nil)); end

  def fetch_s3(uri, mtime=T.unsafe(nil), head=T.unsafe(nil)); end

  def fetch_size(*args, &block); end

  def headers(); end

  def headers=(headers); end

  def https?(uri); end

  def initialize(proxy=T.unsafe(nil), dns=T.unsafe(nil), headers=T.unsafe(nil)); end

  def request(uri, request_class, last_modified=T.unsafe(nil)); end

  def s3_uri_signer(uri); end
end

class Gem::Request
  include ::Gem::UserInteraction
  include ::Gem::DefaultUserInteraction
  include ::Gem::Text

  def cert_files(); end

  def connection_for(uri); end

  def fetch(); end

  def initialize(uri, request_class, last_modified, pool); end

  def perform_request(request); end

  def proxy_uri(); end

  def reset(connection); end

  def user_agent(); end

  def self.configure_connection_for_https(connection, cert_files); end

  def self.create_with_proxy(uri, request_class, last_modified, proxy); end

  def self.get_cert_files(); end

  def self.get_proxy_from_env(scheme=T.unsafe(nil)); end

  def self.proxy_uri(proxy); end

  def self.verify_certificate(store_context); end

  def self.verify_certificate_message(error_number, cert); end
end

class Gem::Request::ConnectionPools
  def close_all(); end

  def initialize(proxy_uri, cert_files); end

  def pool_for(uri); end

  def self.client(); end

  def self.client=(client); end
end

class Gem::Request::HTTPPool
  def cert_files(); end

  def checkin(connection); end

  def checkout(); end

  def close_all(); end

  def initialize(http_args, cert_files, proxy_uri); end

  def proxy_uri(); end
end

class Gem::Request::HTTPSPool < Gem::Request::HTTPPool
end

class Gem::RequestSet
  include ::TSort

  def always_install(); end

  def always_install=(always_install); end

  def dependencies(); end

  def development(); end

  def development=(development); end

  def development_shallow(); end

  def development_shallow=(development_shallow); end

  def errors(); end

  def gem(name, *reqs); end

  def git_set(); end

  def ignore_dependencies(); end

  def ignore_dependencies=(ignore_dependencies); end

  def import(deps); end

  def initialize(*deps); end

  def install(options, &block); end

  def install_dir(); end

  def install_from_gemdeps(options, &block); end

  def install_hooks(requests, options); end

  def install_into(dir, force=T.unsafe(nil), options=T.unsafe(nil)); end

  def load_gemdeps(path, without_groups=T.unsafe(nil), installing=T.unsafe(nil)); end

  def prerelease(); end

  def prerelease=(prerelease); end

  def remote(); end

  def remote=(remote); end

  def resolve(set=T.unsafe(nil)); end

  def resolve_current(); end

  def resolver(); end

  def sets(); end

  def soft_missing(); end

  def soft_missing=(soft_missing); end

  def sorted_requests(); end

  def source_set(); end

  def specs(); end

  def specs_in(dir); end

  def tsort_each_node(&block); end

  def vendor_set(); end
end

class Gem::RequestSet::GemDependencyAPI
  def dependencies(); end

  def find_gemspec(name, path); end

  def gem(name, *requirements); end

  def gem_deps_file(); end

  def gem_git_reference(options); end

  def gemspec(options=T.unsafe(nil)); end

  def git(repository); end

  def git_set(); end

  def git_source(name, &callback); end

  def group(*groups); end

  def initialize(set, path); end

  def installing=(installing); end

  def load(); end

  def platform(*platforms); end

  def platforms(*platforms); end

  def requires(); end

  def ruby(version, options=T.unsafe(nil)); end

  def source(url); end

  def vendor_set(); end

  def without_groups(); end

  def without_groups=(without_groups); end

  ENGINE_MAP = ::T.let(nil, ::T.untyped)
  PLATFORM_MAP = ::T.let(nil, ::T.untyped)
  VERSION_MAP = ::T.let(nil, ::T.untyped)
  WINDOWS = ::T.let(nil, ::T.untyped)
end

class Gem::RequestSet::Lockfile
  def add_DEPENDENCIES(out); end

  def add_GEM(out, spec_groups); end

  def add_GIT(out, git_requests); end

  def add_PATH(out, path_requests); end

  def add_PLATFORMS(out); end

  def initialize(request_set, gem_deps_file, dependencies); end

  def platforms(); end

  def relative_path_from(dest, base); end

  def spec_groups(); end

  def write(); end

  def self.build(request_set, gem_deps_file, dependencies=T.unsafe(nil)); end

  def self.requests_to_deps(requests); end
end

class Gem::RequestSet::Lockfile::ParseError < Gem::Exception
  def column(); end

  def initialize(message, column, line, path); end

  def line(); end

  def path(); end
end

class Gem::RequestSet::Lockfile::Parser
  def get(expected_types=T.unsafe(nil), expected_value=T.unsafe(nil)); end

  def initialize(tokenizer, set, platforms, filename=T.unsafe(nil)); end

  def parse(); end

  def parse_DEPENDENCIES(); end

  def parse_GEM(); end

  def parse_GIT(); end

  def parse_PATH(); end

  def parse_PLATFORMS(); end

  def parse_dependency(name, op); end
end

class Gem::RequestSet::Lockfile::Tokenizer
  def empty?(); end

  def initialize(input, filename=T.unsafe(nil), line=T.unsafe(nil), pos=T.unsafe(nil)); end

  def make_parser(set, platforms); end

  def next_token(); end

  def peek(); end

  def shift(); end

  def skip(type); end

  def to_a(); end

  def token_pos(byte_offset); end

  def unshift(token); end

  def self.from_file(file); end

  EOF = ::T.let(nil, ::T.untyped)
end

class Gem::RequestSet::Lockfile::Tokenizer::Token < Struct
  Elem = type_member(:out, fixed: T.untyped)

  def column(); end

  def column=(_); end

  def line(); end

  def line=(_); end

  def type(); end

  def type=(_); end

  def value(); end

  def value=(_); end

  def self.[](*_); end

  def self.members(); end
end

class Gem::Resolver
  include ::Gem::Resolver::Molinillo::UI
  include ::Gem::Resolver::Molinillo::SpecificationProvider

  DEBUG_RESOLVER = ::T.let(nil, ::T.untyped)

  def activation_request(dep, possible); end

  def development(); end

  def development=(development); end

  def development_shallow(); end

  def development_shallow=(development_shallow); end

  def explain(stage, *data); end

  def explain_list(stage); end

  def find_possible(dependency); end

  def ignore_dependencies(); end

  def ignore_dependencies=(ignore_dependencies); end

  def initialize(needed, set=T.unsafe(nil)); end

  def missing(); end

  def requests(s, act, reqs=T.unsafe(nil)); end

  def resolve(); end

  def select_local_platforms(specs); end

  def skip_gems(); end

  def skip_gems=(skip_gems); end

  def soft_missing(); end

  def soft_missing=(soft_missing); end

  def stats(); end

  def self.compose_sets(*sets); end

  def self.for_current_gems(needed); end
end

class Gem::Resolver::APISet < Gem::Resolver::Set
  def dep_uri(); end

  def initialize(dep_uri=T.unsafe(nil)); end

  def prefetch_now(); end

  def source(); end

  def uri(); end

  def versions(name); end
end

class Gem::Resolver::APISpecification < Gem::Resolver::Specification
  def ==(other); end

  def initialize(set, api_data); end
end

class Gem::Resolver::ActivationRequest
  def ==(other); end

  def development?(); end

  def download(path); end

  def full_name(); end

  def full_spec(); end

  def initialize(spec, request); end

  def installed?(); end

  def name(); end

  def parent(); end

  def platform(); end

  def request(); end

  def spec(); end

  def version(); end
end

class Gem::Resolver::BestSet < Gem::Resolver::ComposedSet
  def initialize(sources=T.unsafe(nil)); end

  def pick_sets(); end

  def replace_failed_api_set(error); end
end

class Gem::Resolver::ComposedSet < Gem::Resolver::Set
  def initialize(*sets); end

  def prerelease=(allow_prerelease); end

  def remote=(remote); end

  def sets(); end
end

class Gem::Resolver::Conflict
  def ==(other); end

  def activated(); end

  def conflicting_dependencies(); end

  def dependency(); end

  def explain(); end

  def explanation(); end

  def failed_dep(); end

  def for_spec?(spec); end

  def initialize(dependency, activated, failed_dep=T.unsafe(nil)); end

  def request_path(current); end

  def requester(); end
end

class Gem::Resolver::DependencyRequest
  def ==(other); end

  def dependency(); end

  def development?(); end

  def explicit?(); end

  def implicit?(); end

  def initialize(dependency, requester); end

  def match?(spec, allow_prerelease=T.unsafe(nil)); end

  def matches_spec?(spec); end

  def name(); end

  def request_context(); end

  def requester(); end

  def requirement(); end

  def type(); end
end

class Gem::Resolver::GitSet < Gem::Resolver::Set
  def add_git_gem(name, repository, reference, submodules); end

  def add_git_spec(name, version, repository, reference, submodules); end

  def need_submodules(); end

  def repositories(); end

  def root_dir(); end

  def root_dir=(root_dir); end

  def specs(); end
end

class Gem::Resolver::GitSpecification < Gem::Resolver::SpecSpecification
  def ==(other); end

  def add_dependency(dependency); end
end

class Gem::Resolver::IndexSet < Gem::Resolver::Set
  def initialize(source=T.unsafe(nil)); end
end

class Gem::Resolver::IndexSpecification < Gem::Resolver::Specification
  def initialize(set, name, version, source, platform); end
end

class Gem::Resolver::InstalledSpecification < Gem::Resolver::SpecSpecification
  def ==(other); end
end

class Gem::Resolver::InstallerSet < Gem::Resolver::Set
  def add_always_install(dependency); end

  def add_local(dep_name, spec, source); end

  def always_install(); end

  def consider_local?(); end

  def consider_remote?(); end

  def ignore_dependencies(); end

  def ignore_dependencies=(ignore_dependencies); end

  def ignore_installed(); end

  def ignore_installed=(ignore_installed); end

  def initialize(domain); end

  def load_spec(name, ver, platform, source); end

  def local?(dep_name); end

  def prerelease=(allow_prerelease); end

  def remote=(remote); end

  def remote_set(); end
end

class Gem::Resolver::LockSet < Gem::Resolver::Set
  def add(name, version, platform); end

  def initialize(sources); end

  def load_spec(name, version, platform, source); end

  def specs(); end
end

class Gem::Resolver::LockSpecification < Gem::Resolver::Specification
  def add_dependency(dependency); end

  def initialize(set, name, version, sources, platform); end

  def sources(); end
end

module Gem::Resolver::Molinillo
  VERSION = ::T.let(nil, ::T.untyped)
end

class Gem::Resolver::Molinillo::CircularDependencyError < Gem::Resolver::Molinillo::ResolverError
  def dependencies(); end

  def initialize(nodes); end
end

module Gem::Resolver::Molinillo::Delegates
end

module Gem::Resolver::Molinillo::Delegates::ResolutionState
  def activated(); end

  def conflicts(); end

  def depth(); end

  def name(); end

  def possibilities(); end

  def requirement(); end

  def requirements(); end
end

module Gem::Resolver::Molinillo::Delegates::SpecificationProvider
  def allow_missing?(dependency); end

  def dependencies_for(specification); end

  def name_for(dependency); end

  def name_for_explicit_dependency_source(); end

  def name_for_locking_dependency_source(); end

  def requirement_satisfied_by?(requirement, activated, spec); end

  def search_for(dependency); end

  def sort_dependencies(dependencies, activated, conflicts); end
end

class Gem::Resolver::Molinillo::DependencyGraph
  include Enumerable
  include ::TSort

  Elem = type_member(fixed: T.untyped)

  def ==(other); end

  def add_child_vertex(name, payload, parent_names, requirement); end

  def add_edge(origin, destination, requirement); end

  def add_vertex(name, payload, root=T.unsafe(nil)); end

  def delete_edge(edge); end

  def detach_vertex_named(name); end

  def each(&blk); end

  def log(); end

  def rewind_to(tag); end

  def root_vertex_named(name); end

  def set_payload(name, payload); end

  def tag(tag); end

  def to_dot(options=T.unsafe(nil)); end

  def tsort_each_child(vertex, &block); end

  def vertex_named(name); end

  def vertices(); end

  def self.tsort(vertices); end
end

class Gem::Resolver::Molinillo::DependencyGraph::Action
  def down(graph); end

  def next(); end

  def next=(_); end

  def previous(); end

  def previous=(previous); end

  def up(graph); end

  def self.action_name(); end
end

class Gem::Resolver::Molinillo::DependencyGraph::AddEdgeNoCircular < Gem::Resolver::Molinillo::DependencyGraph::Action
  def destination(); end

  def initialize(origin, destination, requirement); end

  def make_edge(graph); end

  def origin(); end

  def requirement(); end
end

class Gem::Resolver::Molinillo::DependencyGraph::AddVertex < Gem::Resolver::Molinillo::DependencyGraph::Action
  def initialize(name, payload, root); end

  def name(); end

  def payload(); end

  def root(); end
end

class Gem::Resolver::Molinillo::DependencyGraph::DeleteEdge < Gem::Resolver::Molinillo::DependencyGraph::Action
  def destination_name(); end

  def initialize(origin_name, destination_name, requirement); end

  def make_edge(graph); end

  def origin_name(); end

  def requirement(); end
end

class Gem::Resolver::Molinillo::DependencyGraph::DetachVertexNamed < Gem::Resolver::Molinillo::DependencyGraph::Action
  def initialize(name); end

  def name(); end
end

class Gem::Resolver::Molinillo::DependencyGraph::Edge < Struct
  Elem = type_member(:out, fixed: T.untyped)

  def destination(); end

  def destination=(_); end

  def origin(); end

  def origin=(_); end

  def requirement(); end

  def requirement=(_); end

  def self.[](*_); end

  def self.members(); end
end

class Gem::Resolver::Molinillo::DependencyGraph::SetPayload < Gem::Resolver::Molinillo::DependencyGraph::Action
  def initialize(name, payload); end

  def name(); end

  def payload(); end
end

class Gem::Resolver::Molinillo::DependencyGraph::Tag < Gem::Resolver::Molinillo::DependencyGraph::Action
  def down(_graph); end

  def initialize(tag); end

  def tag(); end

  def up(_graph); end
end

class Gem::Resolver::Molinillo::DependencyGraph::Vertex
  def ==(other); end

  def ancestor?(other); end

  def descendent?(other); end

  def eql?(other); end

  def explicit_requirements(); end

  def incoming_edges(); end

  def incoming_edges=(incoming_edges); end

  def initialize(name, payload); end

  def is_reachable_from?(other); end

  def name(); end

  def name=(name); end

  def outgoing_edges(); end

  def outgoing_edges=(outgoing_edges); end

  def path_to?(other); end

  def payload(); end

  def payload=(payload); end

  def predecessors(); end

  def recursive_predecessors(); end

  def recursive_successors(); end

  def requirements(); end

  def root(); end

  def root=(root); end

  def root?(); end

  def shallow_eql?(other); end

  def successors(); end
end

class Gem::Resolver::Molinillo::DependencyState < Gem::Resolver::Molinillo::ResolutionState
  Elem = type_member(:out, fixed: T.untyped)

  def pop_possibility_state(); end
end


class Gem::Resolver::Molinillo::NoSuchDependencyError < Gem::Resolver::Molinillo::ResolverError
  def dependency(); end

  def dependency=(dependency); end

  def initialize(dependency, required_by=T.unsafe(nil)); end

  def required_by(); end

  def required_by=(required_by); end
end

class Gem::Resolver::Molinillo::PossibilityState < Gem::Resolver::Molinillo::ResolutionState
  Elem = type_member(:out, fixed: T.untyped)
end

class Gem::Resolver::Molinillo::ResolutionState < Struct
  Elem = type_member(:out, fixed: T.untyped)

  def activated(); end

  def activated=(_); end

  def conflicts(); end

  def conflicts=(_); end

  def depth(); end

  def depth=(_); end

  def name(); end

  def name=(_); end

  def possibilities(); end

  def possibilities=(_); end

  def requirement(); end

  def requirement=(_); end

  def requirements(); end

  def requirements=(_); end

  def self.[](*_); end

  def self.empty(); end

  def self.members(); end
end

class Gem::Resolver::Molinillo::Resolver
  def initialize(specification_provider, resolver_ui); end

  def resolve(requested, base=T.unsafe(nil)); end

  def resolver_ui(); end

  def specification_provider(); end
end

class Gem::Resolver::Molinillo::Resolver::Resolution
  include ::Gem::Resolver::Molinillo::Delegates::ResolutionState
  include ::Gem::Resolver::Molinillo::Delegates::SpecificationProvider

  def base(); end

  def initialize(specification_provider, resolver_ui, requested, base); end

  def iteration_rate=(iteration_rate); end

  def original_requested(); end

  def resolve(); end

  def resolver_ui(); end

  def specification_provider(); end

  def started_at=(started_at); end

  def states=(states); end
end

class Gem::Resolver::Molinillo::Resolver::Resolution::Conflict < Struct
  Elem = type_member(:out, fixed: T.untyped)

  def activated_by_name(); end

  def activated_by_name=(_); end

  def existing(); end

  def existing=(_); end

  def locked_requirement(); end

  def locked_requirement=(_); end

  def possibility(); end

  def possibility=(_); end

  def requirement(); end

  def requirement=(_); end

  def requirement_trees(); end

  def requirement_trees=(_); end

  def requirements(); end

  def requirements=(_); end

  def self.[](*_); end

  def self.members(); end
end

class Gem::Resolver::Molinillo::ResolverError < StandardError
end

module Gem::Resolver::Molinillo::SpecificationProvider
  def allow_missing?(dependency); end

  def dependencies_for(specification); end

  def name_for(dependency); end

  def name_for_explicit_dependency_source(); end

  def name_for_locking_dependency_source(); end

  def requirement_satisfied_by?(requirement, activated, spec); end

  def search_for(dependency); end

  def sort_dependencies(dependencies, activated, conflicts); end
end

module Gem::Resolver::Molinillo::UI
  def after_resolution(); end

  def before_resolution(); end

  def debug(depth=T.unsafe(nil)); end

  def debug?(); end

  def indicate_progress(); end

  def output(); end

  def progress_rate(); end
end

class Gem::Resolver::Molinillo::VersionConflict < Gem::Resolver::Molinillo::ResolverError
  def conflicts(); end

  def initialize(conflicts); end
end

class Gem::Resolver::RequirementList
  include Enumerable

  Elem = type_member(fixed: T.untyped)

  def add(req); end

  def each(&blk); end

  def empty?(); end

  def next5(); end

  def remove(); end

  def size(); end
end

class Gem::Resolver::Set
  def errors(); end

  def errors=(errors); end

  def find_all(req); end

  def prefetch(reqs); end

  def prerelease(); end

  def prerelease=(prerelease); end

  def remote(); end

  def remote=(remote); end

  def remote?(); end
end

class Gem::Resolver::SourceSet < Gem::Resolver::Set
  def add_source_gem(name, source); end
end

class Gem::Resolver::SpecSpecification < Gem::Resolver::Specification
  def initialize(set, spec, source=T.unsafe(nil)); end
end

class Gem::Resolver::Specification
  def dependencies(); end

  def download(options); end

  def fetch_development_dependencies(); end

  def full_name(); end

  def install(options=T.unsafe(nil)); end

  def installable_platform?(); end

  def local?(); end

  def name(); end

  def platform(); end

  def set(); end

  def source(); end

  def spec(); end

  def version(); end
end

class Gem::Resolver::Stats
  PATTERN = ::T.let(nil, ::T.untyped)

  def backtracking!(); end

  def display(); end

  def iteration!(); end

  def record_depth(stack); end

  def record_requirements(reqs); end

  def requirement!(); end
end

class Gem::Resolver::VendorSet < Gem::Resolver::Set
  def add_vendor_gem(name, directory); end

  def load_spec(name, version, platform, source); end

  def specs(); end
end

class Gem::Resolver::VendorSpecification < Gem::Resolver::SpecSpecification
  def ==(other); end
end

class Gem::S3URISigner
  BASE64_URI_TRANSLATE = ::T.let(nil, ::T.untyped)
  EC2_IAM_INFO = ::T.let(nil, ::T.untyped)
  EC2_IAM_SECURITY_CREDENTIALS = ::T.let(nil, ::T.untyped)

  def initialize(uri); end

  def sign(expiration=T.unsafe(nil)); end

  def uri(); end

  def uri=(uri); end
end

class Gem::S3URISigner::ConfigurationError
  def initialize(message); end
end

class Gem::S3URISigner::InstanceProfileError
  def initialize(message); end
end

class Gem::S3URISigner::S3Config
  def access_key_id(); end

  def access_key_id=(_); end

  def region(); end

  def region=(_); end

  def secret_access_key(); end

  def secret_access_key=(_); end

  def security_token(); end

  def security_token=(_); end

  def self.[](*_); end

  def self.members(); end
end

module Gem::Security
  AlmostNoSecurity = ::T.let(nil, ::T.untyped)
  DIGEST_NAME = ::T.let(nil, ::T.untyped)
  EXTENSIONS = ::T.let(nil, ::T.untyped)
  HighSecurity = ::T.let(nil, ::T.untyped)
  KEY_CIPHER = ::T.let(nil, ::T.untyped)
  KEY_LENGTH = ::T.let(nil, ::T.untyped)
  LowSecurity = ::T.let(nil, ::T.untyped)
  MediumSecurity = ::T.let(nil, ::T.untyped)
  NoSecurity = ::T.let(nil, ::T.untyped)
  ONE_DAY = ::T.let(nil, ::T.untyped)
  ONE_YEAR = ::T.let(nil, ::T.untyped)
  Policies = ::T.let(nil, ::T.untyped)
  SigningPolicy = ::T.let(nil, ::T.untyped)
end

class Gem::Security::DIGEST_ALGORITHM < OpenSSL::Digest
  def initialize(data=T.unsafe(nil)); end

  def self.digest(data); end

  def self.hexdigest(data); end
end

class Gem::Security::Signer
  DEFAULT_OPTIONS = ::T.let(nil, ::T.untyped)
end

class Gem::Security::TrustDir
  DEFAULT_PERMISSIONS = ::T.let(nil, ::T.untyped)
end

class Gem::Source
  include ::Comparable
  include ::Gem::Text

  FILES = ::T.let(nil, ::T.untyped)

  def ==(other); end

  def cache_dir(uri); end

  def dependency_resolver_set(); end

  def download(spec, dir=T.unsafe(nil)); end

  def eql?(other); end

  def fetch_spec(name_tuple); end

  def initialize(uri); end

  def load_specs(type); end

  def typo_squatting?(host, distance_threshold=T.unsafe(nil)); end

  def update_cache?(); end

  def uri(); end
end

class Gem::Source::Git < Gem::Source
  def base_dir(); end

  def cache(); end

  def checkout(); end

  def dir_shortref(); end

  def download(full_spec, path); end

  def initialize(name, repository, reference, submodules=T.unsafe(nil)); end

  def install_dir(); end

  def name(); end

  def need_submodules(); end

  def reference(); end

  def remote(); end

  def remote=(remote); end

  def repo_cache_dir(); end

  def repository(); end

  def rev_parse(); end

  def root_dir(); end

  def root_dir=(root_dir); end

  def specs(); end

  def uri_hash(); end
end

class Gem::Source::Installed < Gem::Source
  def download(spec, path); end

  def initialize(); end
end

class Gem::Source::Local < Gem::Source
  def download(spec, cache_dir=T.unsafe(nil)); end

  def fetch_spec(name); end

  def find_gem(gem_name, version=T.unsafe(nil), prerelease=T.unsafe(nil)); end

  def initialize(); end
end

class Gem::Source::Lock < Gem::Source
  def initialize(source); end

  def wrapped(); end
end

class Gem::Source::SpecificFile < Gem::Source
  def fetch_spec(name); end

  def initialize(file); end

  def load_specs(*a); end

  def path(); end

  def spec(); end
end

class Gem::Source::Vendor < Gem::Source::Installed
  def initialize(path); end
end

class Gem::SourceFetchProblem
  def error(); end

  def exception(); end

  def initialize(source, error); end

  def source(); end

  def wordy(); end
end

class Gem::SourceList
  include Enumerable

  Elem = type_member(:out)

  def <<(obj); end

  def ==(other); end

  def clear(); end

  def delete(source); end

  def each(&blk); end

  def each_source(&b); end

  def empty?(); end

  def first(); end

  def include?(other); end

  def replace(other); end

  def sources(); end

  def to_a(); end

  def to_ary(); end

  def self.from(ary); end
end

class Gem::UriFormatter
  def escape(); end

  def initialize(uri); end

  def normalize(); end

  def unescape(); end

  def uri(); end
end

class Gem::UriParser
  def parse(uri); end

  def parse!(uri); end
end

module Gem::UriParsing
end

module Gem::Util
  def self.correct_for_windows_path(path); end

  def self.glob_files_in_dir(glob, base_path); end

  def self.gunzip(data); end

  def self.gzip(data); end

  def self.inflate(data); end

  def self.popen(*command); end

  def self.silent_system(*command); end

  def self.traverse_parents(directory, &block); end
end

# RubyGems adds the
# [`gem`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-gem) method
# to allow activation of specific gem versions and overrides the
# [`require`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-require)
# method on [`Kernel`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html) to make
# gems appear as if they live on the `$LOAD_PATH`. See the documentation of
# these methods for further detail.
module Kernel
  # Use
  # [`Kernel#gem`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-gem)
  # to activate a specific version of `gem_name`.
  #
  # `requirements` is a list of version requirements that the specified gem must
  # match, most commonly "= example.version.number". See
  # [`Gem::Requirement`](https://docs.ruby-lang.org/en/2.6.0/Gem/Requirement.html)
  # for how to specify a version requirement.
  #
  # If you will be activating the latest version of a gem, there is no need to
  # call
  # [`Kernel#gem`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-gem),
  # [`Kernel#require`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-require)
  # will do the right thing for you.
  #
  # [`Kernel#gem`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-gem)
  # returns true if the gem was activated, otherwise false. If the gem could not
  # be found, didn't match the version requirements, or a different version was
  # already activated, an exception will be raised.
  #
  # [`Kernel#gem`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-gem)
  # should be called **before** any require statements (otherwise RubyGems may
  # load a conflicting library version).
  #
  # [`Kernel#gem`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-gem)
  # only loads prerelease versions when prerelease `requirements` are given:
  #
  # ```ruby
  # gem 'rake', '>= 1.1.a', '< 2'
  # ```
  #
  # In older RubyGems versions, the environment variable GEM\_SKIP could be used
  # to skip activation of specified gems, for example to test out changes that
  # haven't been installed yet. Now RubyGems defers to -I and the RUBYLIB
  # environment variable to skip activation of a gem.
  #
  # Example:
  #
  # ```
  # GEM_SKIP=libA:libB ruby -I../libA -I../libB ./mycode.rb
  # ```
  def gem(dep, *reqs); end
end
