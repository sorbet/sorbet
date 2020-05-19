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
  ConfigMap = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  DEFAULT_HOST = T.let(T.unsafe(nil), String)

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

  RbConfigPriorities = T.let(T.unsafe(nil), T::Array[T.untyped])

  RubyGemsPackageVersion = T.let(T.unsafe(nil), String)

  RubyGemsVersion = T.let(T.unsafe(nil), String)

  VERSION = T.let(T.unsafe(nil), String)

  # An [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of Regexps that
  # match windows Ruby platforms.
  WIN_PATTERNS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html) classes
  # used in
  # [`Gem.write_binary`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-write_binary)
  # `rescue` statement. Not all of these are defined in Ruby 1.8.7.
  WRITE_BINARY_ERRORS = T.let(T.unsafe(nil), T::Array[T.untyped])

  def self._deprecated_detect_gemdeps(path = _); end

  def self._deprecated_gunzip(data); end

  def self._deprecated_gzip(data); end

  def self._deprecated_inflate(data); end

  def self.activate_bin_path(name, *args); end

  # Returns binary extensions dir for specified RubyGems base dir or nil if such
  # directory cannot be determined.
  #
  # By default, the binary extensions are located side by side with their Ruby
  # counterparts, therefore nil is returned
  def self.default_ext_dir_for(base_dir); end

  # Whether to expect full paths in default gems - true for non-MRI ruby
  # implementations
  def self.default_gems_use_full_paths?; end

  # Default spec directory path to be used if an alternate value is not
  # specified in the environment
  def self.default_spec_cache_dir; end

  # A
  # [`Zlib::Deflate.deflate`](https://docs.ruby-lang.org/en/2.6.0/Zlib/Deflate.html#method-c-deflate)
  # wrapper
  def self.deflate(data); end

  # TODO remove with RubyGems 4.0
  #
  # Alias for:
  # [`use_gemdeps`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-use_gemdeps)
  def self.detect_gemdeps(*args, &block); end

  # The path where gems are to be installed.
  def self.dir; end

  # Adds a post-installs hook that will be passed a
  # [`Gem::DependencyInstaller`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyInstaller.html)
  # and a list of installed specifications when
  # [`Gem::DependencyInstaller#install`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyInstaller.html#method-i-install)
  # is complete
  def self.done_installing(&hook); end

  # The list of hooks to be run after
  # [`Gem::DependencyInstaller`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyInstaller.html)
  # installs a set of gems
  def self.done_installing_hooks; end

  # Quietly ensure the [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html)
  # directory `dir` contains all the proper subdirectories for handling default
  # gems. If we can't create a directory due to a permission problem, then we
  # will silently continue.
  #
  # If `mode` is given, missing directories are created with this mode.
  #
  # World-writable directories will never be created.
  def self.ensure_default_gem_subdirectories(dir = _, mode = _); end

  # Quietly ensure the [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html)
  # directory `dir` contains all the proper subdirectories. If we can't create a
  # directory due to a permission problem, then we will silently continue.
  #
  # If `mode` is given, missing directories are created with this mode.
  #
  # World-writable directories will never be created.
  def self.ensure_gem_subdirectories(dir = _, mode = _); end

  def self.ensure_subdirectories(dir, mode, subdirs); end

  def self.env_requirement(gem_name); end

  def self.extension_api_version; end

  # Returns a list of paths matching `glob` that can be used by a gem to pick up
  # features from other gems. For example:
  #
  # ```ruby
  # Gem.find_files('rdoc/discover').each do |path| load path end
  # ```
  #
  # if `check_load_path` is true (the default), then
  # [`find_files`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-find_files)
  # also searches $LOAD\_PATH for files as well as gems.
  #
  # Note that
  # [`find_files`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-find_files)
  # will return all files even if they are from different versions of the same
  # gem. See also
  # [`find_latest_files`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-find_latest_files)
  def self.find_files(glob, check_load_path = _); end

  def self.find_files_from_load_path(glob); end

  # Returns a list of paths matching `glob` from the latest gems that can be
  # used by a gem to pick up features from other gems. For example:
  #
  # ```ruby
  # Gem.find_latest_files('rdoc/discover').each do |path| load path end
  # ```
  #
  # if `check_load_path` is true (the default), then
  # [`find_latest_files`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-find_latest_files)
  # also searches $LOAD\_PATH for files as well as gems.
  #
  # Unlike
  # [`find_files`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-find_files),
  # [`find_latest_files`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-find_latest_files)
  # will return only files from the latest version of a gem.
  def self.find_latest_files(glob, check_load_path = _); end

  # [`Find`](https://docs.ruby-lang.org/en/2.6.0/Find.html) a
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # of default gem from `path`
  def self.find_unresolved_default_spec(path); end

  def self.finish_resolve(*_); end

  # GemDependencyAPI object, which is set when .use\_gemdeps is called. This
  # contains all the information from the
  # [Gemfile](https://docs.ruby-lang.org/en/2.6.0/2_6_0/bundler/templates/Gemfile.html).
  def self.gemdeps; end

  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.6.0/Zlib/GzipReader.html)
  # wrapper that unzips `data`.
  def self.gunzip(*args, &block); end

  # [`Zlib::GzipWriter`](https://docs.ruby-lang.org/en/2.6.0/Zlib/GzipWriter.html)
  # wrapper that zips `data`.
  def self.gzip(*args, &block); end

  # Get the default RubyGems API host. This is normally `https://rubygems.org`.
  def self.host; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the default RubyGems
  # API host.
  def self.host=(host); end

  # A
  # [`Zlib::Inflate#inflate`](https://docs.ruby-lang.org/en/2.6.0/Zlib/Inflate.html#method-i-inflate)
  # wrapper
  def self.inflate(*args, &block); end

  # Top level install helper method. Allows you to install gems interactively:
  #
  # ```
  # % irb
  # >> Gem.install "minitest"
  # Fetching: minitest-3.0.1.gem (100%)
  # => [#<Gem::Specification:0x1013b4528 @name="minitest", ...>]
  # ```
  def self.install(name, version = _, *options); end

  def self.install_extension_in_lib; end

  # Returns the latest release version of RubyGems.
  def self.latest_rubygems_version; end

  # Returns the latest release-version specification for the gem `name`.
  def self.latest_spec_for(name); end

  # Returns the version of the latest release-version of gem `name`
  def self.latest_version_for(name); end

  # [`Find`](https://docs.ruby-lang.org/en/2.6.0/Find.html) all
  # 'rubygems\_plugin' files in $LOAD\_PATH and load them
  def self.load_env_plugins; end

  # The index to insert activated gem paths into the $LOAD\_PATH. The activated
  # gem's paths are inserted before site lib directory by default.
  def self.load_path_insert_index; end

  def self.load_plugin_files(plugins); end

  # [`Find`](https://docs.ruby-lang.org/en/2.6.0/Find.html) the
  # 'rubygems\_plugin' files in the latest installed gems and load them
  def self.load_plugins; end

  # Loads [`YAML`](https://docs.ruby-lang.org/en/2.6.0/YAML.html), preferring
  # [`Psych`](https://docs.ruby-lang.org/en/2.6.0/Psych.html)
  def self.load_yaml; end

  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) of loaded
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # keyed by name
  def self.loaded_specs; end

  # The file name and line number of the caller of the caller of this method.
  #
  # `depth` is how many layers up the call stack it should go.
  #
  # e.g.,
  #
  # def a;
  # [`Gem.location_of_caller`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-location_of_caller);
  # end a #=> ["x.rb", 2]  # (it'll vary depending on file name and line number)
  #
  # def b; c; end def c;
  # [`Gem.location_of_caller(2)`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-location_of_caller);
  # end b #=> ["x.rb", 6]  # (it'll vary depending on file name and line number)
  def self.location_of_caller(depth = _); end

  # The version of the
  # [`Marshal`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html) format for
  # your Ruby.
  def self.marshal_version; end

  def self.needs; end

  # Default options for gem commands for Ruby packagers.
  #
  # The options here should be structured as an array of string "gem" command
  # names as keys and a string of the default options as values.
  #
  # Example:
  #
  # def self.operating\_system\_defaults
  #
  # ```ruby
  # {
  #     'install' => '--no-rdoc --no-ri --env-shebang',
  #     'update' => '--no-rdoc --no-ri --env-shebang'
  # }
  # ```
  #
  # end
  def self.operating_system_defaults; end

  def self.path; end

  # How [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html)
  # [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html) paths should be split.
  # Overridable for esoteric platforms.
  def self.path_separator; end

  # Retrieve the PathSupport object that RubyGems uses to lookup files.
  def self.paths; end

  # Initialize the filesystem paths to use from `env`. `env` is a hash-like
  # object (typically [`ENV`](https://docs.ruby-lang.org/en/2.6.0/ENV.html))
  # that is queried for 'GEM\_HOME', 'GEM\_PATH', and 'GEM\_SPEC\_CACHE' Keys
  # for the `env` hash should be Strings, and values of the hash should be
  # Strings or `nil`.
  def self.paths=(env); end

  # Default options for gem commands for Ruby implementers.
  #
  # The options here should be structured as an array of string "gem" command
  # names as keys and a string of the default options as values.
  #
  # Example:
  #
  # def self.platform\_defaults
  #
  # ```ruby
  # {
  #     'install' => '--no-rdoc --no-ri --env-shebang',
  #     'update' => '--no-rdoc --no-ri --env-shebang'
  # }
  # ```
  #
  # end
  def self.platform_defaults; end

  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of platforms this
  # RubyGems supports.
  def self.platforms; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) array of platforms
  # this RubyGems supports (primarily for testing).
  def self.platforms=(platforms); end

  # Adds a post-build hook that will be passed an
  # [`Gem::Installer`](https://docs.ruby-lang.org/en/2.6.0/Gem/Installer.html)
  # instance when
  # [`Gem::Installer#install`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-i-install)
  # is called. The hook is called after the gem has been extracted and
  # extensions have been built but before the executables or gemspec has been
  # written. If the hook returns `false` then the gem's files will be removed
  # and the install will be aborted.
  def self.post_build(&hook); end

  # The list of hooks to be run after
  # [`Gem::Installer#install`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-i-install)
  # extracts files and builds extensions
  def self.post_build_hooks; end

  # Adds a post-install hook that will be passed an
  # [`Gem::Installer`](https://docs.ruby-lang.org/en/2.6.0/Gem/Installer.html)
  # instance when
  # [`Gem::Installer#install`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-i-install)
  # is called
  def self.post_install(&hook); end

  # The list of hooks to be run after
  # [`Gem::Installer#install`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-i-install)
  # completes installation
  def self.post_install_hooks; end

  # Adds a hook that will get run after
  # [`Gem::Specification.reset`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-c-reset)
  # is run.
  def self.post_reset(&hook); end

  # The list of hooks to be run after
  # [`Gem::Specification.reset`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-c-reset)
  # is run.
  def self.post_reset_hooks; end

  # Adds a post-uninstall hook that will be passed a
  # [`Gem::Uninstaller`](https://docs.ruby-lang.org/en/2.6.0/Gem/Uninstaller.html)
  # instance and the spec that was uninstalled when
  # [`Gem::Uninstaller#uninstall`](https://docs.ruby-lang.org/en/2.6.0/Gem/Uninstaller.html#method-i-uninstall)
  # is called
  def self.post_uninstall(&hook); end

  # The list of hooks to be run after
  # [`Gem::Uninstaller#uninstall`](https://docs.ruby-lang.org/en/2.6.0/Gem/Uninstaller.html#method-i-uninstall)
  # completes installation
  def self.post_uninstall_hooks; end

  # Adds a pre-install hook that will be passed an
  # [`Gem::Installer`](https://docs.ruby-lang.org/en/2.6.0/Gem/Installer.html)
  # instance when
  # [`Gem::Installer#install`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-i-install)
  # is called. If the hook returns `false` then the install will be aborted.
  def self.pre_install(&hook); end

  # The list of hooks to be run before
  # [`Gem::Installer#install`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-i-install)
  # does any work
  def self.pre_install_hooks; end

  # Adds a hook that will get run before
  # [`Gem::Specification.reset`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-c-reset)
  # is run.
  def self.pre_reset(&hook); end

  # The list of hooks to be run before
  # [`Gem::Specification.reset`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-c-reset)
  # is run.
  def self.pre_reset_hooks; end

  # Adds a pre-uninstall hook that will be passed an
  # [`Gem::Uninstaller`](https://docs.ruby-lang.org/en/2.6.0/Gem/Uninstaller.html)
  # instance and the spec that will be uninstalled when
  # [`Gem::Uninstaller#uninstall`](https://docs.ruby-lang.org/en/2.6.0/Gem/Uninstaller.html#method-i-uninstall)
  # is called
  def self.pre_uninstall(&hook); end

  # The list of hooks to be run before
  # [`Gem::Uninstaller#uninstall`](https://docs.ruby-lang.org/en/2.6.0/Gem/Uninstaller.html#method-i-uninstall)
  # does any work
  def self.pre_uninstall_hooks; end

  # The directory prefix this RubyGems was installed at. If your prefix is in a
  # standard location (ie, rubygems is installed where you'd expect it to be),
  # then prefix returns nil.
  def self.prefix; end

  # Safely read a file in binary mode on all platforms.
  def self.read_binary(path); end

  # Refresh available gems from disk.
  def self.refresh; end

  # Register a
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # for default gem.
  #
  # Two formats for the specification are supported:
  #
  # *   MRI 2.0 style, where spec.files contains unprefixed require names. The
  #     spec's filenames will be registered as-is.
  # *   New style, where spec.files contains files prefixed with paths from
  #     spec.require\_paths. The prefixes are stripped before registering the
  #     spec's filenames. Unprefixed files are omitted.
  def self.register_default_spec(spec); end

  # Remove needless
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # of default gem from unresolved default gem list
  def self.remove_unresolved_default_spec(spec); end

  # The path to the running Ruby interpreter.
  def self.ruby; end

  # Returns a [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html)
  # containing the API compatibility version of Ruby
  def self.ruby_api_version; end

  # A wrapper around RUBY\_ENGINE const that may not be defined
  def self.ruby_engine; end

  # A [`Gem::Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) for
  # the currently running Ruby.
  def self.ruby_version; end

  # A [`Gem::Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) for
  # the currently running RubyGems
  def self.rubygems_version; end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # sources to fetch remote gems from. Uses
  # [`default_sources`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-default_sources)
  # if the sources list is empty.
  def self.sources; end

  # Need to be able to set the sources without calling
  # [`Gem.sources`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-sources).replace
  # since that would cause an infinite loop.
  #
  # DOC: This comment is not documentation about the method itself, it's more of
  # a code comment about the implementation.
  def self.sources=(new_sources); end

  def self.spec_cache_dir; end

  # Glob pattern for require-able path suffixes.
  def self.suffix_pattern; end

  # Suffixes for require-able paths.
  def self.suffixes; end

  # Prints the amount of time the supplied block takes to run using the debug UI
  # output.
  def self.time(msg, width = _, display = _); end

  # Try to activate a gem containing `path`. Returns true if activation
  # succeeded or wasn't needed because it was already activated. Returns false
  # if it can't find the path in a gem.
  def self.try_activate(path); end

  # Lazily loads DefaultUserInteraction and returns the default UI.
  def self.ui; end

  # Looks for a gem dependency file at `path` and activates the gems in the file
  # if found. If the file is not found an
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html) is
  # raised.
  #
  # If `path` is not given the RUBYGEMS\_GEMDEPS environment variable is used,
  # but if no file is found no exception is raised.
  #
  # If '-' is given for `path` RubyGems searches up from the current working
  # directory for gem dependency files (gem.deps.rb,
  # [Gemfile](https://docs.ruby-lang.org/en/2.6.0/2_6_0/bundler/templates/Gemfile.html),
  # Isolate) and activates the gems in the first one found.
  #
  # You can run this automatically when rubygems starts. To enable, set the
  # `RUBYGEMS_GEMDEPS` environment variable to either the path of your gem
  # dependencies file or "-" to auto-discover in parent directories.
  #
  # NOTE: Enabling automatic discovery on multiuser systems can lead to
  # execution of arbitrary code when used from directories outside your control.
  #
  # Also aliased as:
  # [`detect_gemdeps`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-detect_gemdeps)
  def self.use_gemdeps(path = _); end

  # Use the `home` and `paths` values for
  # [`Gem.dir`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-dir) and
  # [`Gem.path`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-path).
  # Used mainly by the unit tests to provide environment isolation.
  def self.use_paths(home, *paths); end

  # Path for gems in the user's home directory
  def self.user_dir; end

  # The home directory for the user.
  def self.user_home; end

  def self.vendor_dir; end

  # Is this a windows platform?
  def self.win_platform?; end

  # Safely write a file in binary mode on all platforms.
  def self.write_binary(path, data); end
end

class Gem::AvailableSet
  include(::Enumerable)

  Elem = type_member(:out)

  def self.new; end

  def <<(o); end

  def add(spec, source); end

  def all_specs; end

  # Yields each
  # [`Tuple`](https://docs.ruby-lang.org/en/2.6.0/Gem/AvailableSet.html#Tuple)
  # in this
  # [`AvailableSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/AvailableSet.html)
  def each; end

  # Yields the
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # for each
  # [`Tuple`](https://docs.ruby-lang.org/en/2.6.0/Gem/AvailableSet.html#Tuple)
  # in this
  # [`AvailableSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/AvailableSet.html)
  def each_spec; end

  def empty?; end

  # Used by the Resolver, the protocol to use a
  # [`AvailableSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/AvailableSet.html)
  # as a search [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html).
  def find_all(req); end

  def inject_into_list(dep_list); end

  def match_platform!; end

  def pick_best!; end

  def prefetch(reqs); end

  def remote; end

  def remote=(_); end

  def remove_installed!(dep); end

  def set; end

  def size; end

  def sorted; end

  def source_for(spec); end

  # Converts this
  # [`AvailableSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/AvailableSet.html)
  # into a RequestSet that can be used to install gems.
  #
  # If `development` is :none then no development dependencies are installed.
  # Other options are :shallow for only direct development dependencies of the
  # gems in this set or :all for all development dependencies.
  def to_request_set(development = _); end
end

class Gem::AvailableSet::Tuple < ::Struct
  Elem = type_member(:out, fixed: T.untyped)

  def source; end

  def source=(_); end

  def spec; end

  def spec=(_); end

  def self.[](*_); end

  def self.inspect; end

  def self.members; end

  def self.new(*_); end
end

# [`BasicSpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/BasicSpecification.html)
# is an abstract class which implements some common code used by both
# Specification and StubSpecification.
class Gem::BasicSpecification
  def self.new; end

  # True when the gem has been activated
  def activated?; end

  # Returns the full path to the base gem directory.
  #
  # eg: /usr/local/lib/ruby/gems/1.8
  def base_dir; end

  def base_dir=(_); end

  # Return true if this spec can require `file`.
  def contains_requirable_file?(file); end

  # The path to the data directory for this gem.
  def datadir; end

  def default_gem?; end

  # Returns full path to the directory where gem's extensions are installed.
  def extension_dir; end

  def extension_dir=(_); end

  # Returns path to the extensions directory.
  def extensions_dir; end

  # The full path to the gem (install path + full name).
  def full_gem_path; end

  def full_gem_path=(_); end

  # Returns the full name (name-version) of this
  # [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html). Platform information
  # is included (name-version-platform) if it is specified and not the default
  # Ruby platform.
  def full_name; end

  # Full paths in the gem to add to `$LOAD_PATH` when this gem is activated.
  def full_require_paths; end

  def gem_build_complete_path; end

  # Returns the full path to this spec's gem directory. eg:
  # /usr/local/lib/ruby/1.8/gems/mygem-1.0
  def gem_dir; end

  # Returns the full path to the gems directory containing this spec's gem
  # directory. eg: /usr/local/lib/ruby/1.8/gems
  def gems_dir; end

  def ignored=(_); end

  def internal_init; end

  # Returns a string usable in
  # [`Dir.glob`](https://docs.ruby-lang.org/en/2.6.0/Dir.html#method-c-glob) to
  # match all requirable paths for this spec.
  def lib_dirs_glob; end

  # The path this gemspec was loaded from. This attribute is not persisted.
  def loaded_from; end

  # The path this gemspec was loaded from. This attribute is not persisted.
  def loaded_from=(_); end

  # Return all files in this gem that match for `glob`.
  def matches_for_glob(glob); end

  # Name of the gem
  def name; end

  # Platform of the gem
  def platform; end

  def raw_require_paths; end

  # Paths in the gem to add to `$LOAD_PATH` when this gem is activated.
  #
  # See also require\_paths=
  #
  # If you have an extension you do not need to add `"ext"` to the require path,
  # the extension build process will copy the extension files into "lib" for
  # you.
  #
  # The default value is `"lib"`
  #
  # Usage:
  #
  # ```ruby
  # # If all library files are in the root directory...
  # spec.require_path = '.'
  # ```
  def require_paths; end

  # Returns the paths to the source files for use with analysis and
  # documentation tools. These paths are relative to full\_gem\_path.
  def source_paths; end

  # Whether this specification is stubbed - i.e. we have information about the
  # gem from a stub line, without having to evaluate the entire gemspec file.
  def stubbed?; end

  def this; end

  # Full path of the target library file. If the file is not in this gem, return
  # nil.
  def to_fullpath(path); end

  # Return a
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # from this gem
  def to_spec; end

  # Version of the gem
  def version; end

  def self.default_specifications_dir; end
end

# Base class for all [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html)
# commands. When creating a new gem command, define initialize,
# [`execute`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html#method-i-execute),
# [`arguments`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html#method-i-arguments),
# [`defaults_str`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html#method-i-defaults_str),
# [`description`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html#method-i-description)
# and
# [`usage`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html#method-i-usage)
# (as appropriate). See the above mentioned methods for details.
#
# A very good example to look at is
# [`Gem::Commands::ContentsCommand`](https://docs.ruby-lang.org/en/2.6.0/Gem/Commands/ContentsCommand.html)
class Gem::Command
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)

  HELP = T.let(T.unsafe(nil), String)

  # Initializes a generic gem command named `command`. `summary` is a short
  # description displayed in `gem help commands`. `defaults` are the default
  # options. Defaults should be mirrored in
  # [`defaults_str`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html#method-i-defaults_str),
  # unless there are none.
  #
  # When defining a new command subclass, use
  # [`add_option`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html#method-i-add_option)
  # to add command-line switches.
  #
  # Unhandled arguments (gem names, files, etc.) are left in `options[:args]`.
  def self.new(command, summary = _, defaults = _); end

  # Adds extra args from ~/.gemrc
  def add_extra_args(args); end

  # Add a command-line option and handler to the command.
  #
  # See
  # [`OptionParser#make_switch`](https://docs.ruby-lang.org/en/2.6.0/OptionParser.html#method-i-make_switch)
  # for an explanation of `opts`.
  #
  # `handler` will be called with two values, the value of the argument and the
  # options hash.
  #
  # If the first argument of
  # [`add_option`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html#method-i-add_option)
  # is a [`Symbol`](https://docs.ruby-lang.org/en/2.6.0/Symbol.html), it's used
  # to group options in output. See `gem help list` for an example.
  def add_option(*opts, &handler); end

  # Override to provide details of the arguments a command takes. It should
  # return a left-justified string, one argument per line.
  #
  # For example:
  #
  # ```ruby
  # def usage
  #   "#{program_name} FILE [FILE ...]"
  # end
  #
  # def arguments
  #   "FILE          name of file to find"
  # end
  # ```
  def arguments; end

  # True if `long` begins with the characters from `short`.
  def begins?(long, short); end

  # The name of the command.
  def command; end

  # The default options for the command.
  def defaults; end

  # The default options for the command.
  def defaults=(_); end

  # Override to display the default values of the command options. (similar to
  # `arguments`, but displays the default values).
  #
  # For example:
  #
  # ```
  # def defaults_str
  #   --no-gems-first --no-all
  # end
  # ```
  def defaults_str; end

  # Override to display a longer description of what this command does.
  def description; end

  # Override to provide command handling.
  #
  # [`options`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html#attribute-i-options)
  # will be filled in with your parsed options, unparsed options will be left in
  # `options[:args]`.
  #
  # See also:
  # [`get_all_gem_names`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html#method-i-get_all_gem_names),
  # [`get_one_gem_name`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html#method-i-get_one_gem_name),
  # [`get_one_optional_argument`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html#method-i-get_one_optional_argument)
  def execute; end

  # Get all gem names from the command line.
  def get_all_gem_names; end

  # Get all [gem, version] from the command line.
  #
  # An argument in the form gem:ver is pull apart into the gen name and version,
  # respectively.
  def get_all_gem_names_and_versions; end

  # Get a single gem name from the command line. Fail if there is no gem name or
  # if there is more than one gem name given.
  def get_one_gem_name; end

  # Get a single optional argument from the command line. If more than one
  # argument is given, return only the first. Return nil if none are given.
  def get_one_optional_argument; end

  # Handle the given list of arguments by parsing them and recording the
  # results.
  def handle_options(args); end

  # True if the command handles the given argument list.
  def handles?(args); end

  # Invoke the command with the given list of arguments.
  def invoke(*args); end

  # Invoke the command with the given list of normal arguments and additional
  # build arguments.
  def invoke_with_build_args(args, build_args); end

  # Merge a set of command options with the set of default options (without
  # modifying the default option hash).
  def merge_options(new_options); end

  # The options for the command.
  def options; end

  # The name of the command for command-line invocation.
  def program_name; end

  # The name of the command for command-line invocation.
  def program_name=(_); end

  # Remove previously defined command-line argument `name`.
  def remove_option(name); end

  # Display the help message for the command.
  def show_help; end

  # Display to the user that a gem couldn't be found and reasons why
  def show_lookup_failure(gem_name, version, errors, domain, required_by = _); end

  # A short description of the command.
  def summary; end

  # A short description of the command.
  def summary=(_); end

  # Override to display the usage for an individual gem command.
  #
  # The text "[options]" is automatically appended to the usage text.
  def usage; end

  # Call the given block when invoked.
  #
  # Normal command invocations just executes the `execute` method of the
  # command. Specifying an invocation block allows the test methods to override
  # the normal action of a command to determine that it has been invoked
  # correctly.
  def when_invoked(&block); end

  def self.add_common_option(*args, &handler); end

  # Add a list of extra arguments for the given command. `args` may be an array
  # or a string to be split on white space.
  def self.add_specific_extra_args(cmd, args); end

  # Arguments used when building gems
  def self.build_args; end

  def self.build_args=(value); end

  def self.common_options; end

  def self.extra_args; end

  def self.extra_args=(value); end

  # Return an array of extra arguments for the command. The extra arguments come
  # from the gem configuration file read at program startup.
  def self.specific_extra_args(cmd); end

  # Accessor for the specific extra args hash (self initializing).
  def self.specific_extra_args_hash; end
end

class Gem::CommandLineError < ::Gem::Exception; end

# The command manager registers and installs all the individual sub-commands
# supported by the gem command.
#
# Extra commands can be provided by writing a rubygems\_plugin.rb file in an
# installed gem. You should register your command against the
# [`Gem::CommandManager`](https://docs.ruby-lang.org/en/2.6.0/Gem/CommandManager.html)
# instance, like this:
#
# ```ruby
# # file rubygems_plugin.rb
# require 'rubygems/command_manager'
#
# Gem::CommandManager.instance.register_command :edit
# ```
#
# You should put the implementation of your command in rubygems/commands.
#
# ```ruby
# # file rubygems/commands/edit_command.rb
# class Gem::Commands::EditCommand < Gem::Command
#   # ...
# end
# ```
#
# See [`Gem::Command`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html) for
# instructions on writing gem commands.
class Gem::CommandManager
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)

  ALIAS_COMMANDS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  BUILTIN_COMMANDS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Register all the subcommands supported by the gem command.
  def self.new; end

  # Returns a Command instance for `command_name`
  def [](command_name); end

  # Return a sorted list of all command names as strings.
  def command_names; end

  def find_alias_command(cmd_name); end

  def find_command(cmd_name); end

  def find_command_possibilities(cmd_name); end

  # Returns self. Allows a
  # [`CommandManager`](https://docs.ruby-lang.org/en/2.6.0/Gem/CommandManager.html)
  # instance to stand in for the class itself.
  def instance; end

  def process_args(args, build_args = _); end

  # Register the [`Symbol`](https://docs.ruby-lang.org/en/2.6.0/Symbol.html)
  # `command` as a gem command.
  def register_command(command, obj = _); end

  # Run the command specified by `args`.
  def run(args, build_args = _); end

  # Unregister the [`Symbol`](https://docs.ruby-lang.org/en/2.6.0/Symbol.html)
  # `command` as a gem command.
  def unregister_command(command); end

  # Return the authoritative instance of the command manager.
  def self.instance; end

  # Reset the authoritative instance of the command manager.
  def self.reset; end
end

# [`Commands`](https://docs.ruby-lang.org/en/2.6.0/Gem/Commands.html) will be
# placed in this namespace
module Gem::Commands; end

# [`Gem::ConfigFile`](https://docs.ruby-lang.org/en/2.6.0/Gem/ConfigFile.html)
# RubyGems options and gem command options from gemrc.
#
# gemrc is a [`YAML`](https://docs.ruby-lang.org/en/2.6.0/YAML.html) file that
# uses strings to match gem command arguments and symbols to match RubyGems
# options.
#
# [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html) command arguments use a
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) key that matches
# the command name and allow you to specify default arguments:
#
# ```
# install: --no-rdoc --no-ri
# update: --no-rdoc --no-ri
# ```
#
# You can use `gem:` to set default arguments for all commands.
#
# RubyGems options use symbol keys. Valid options are:
#
# `:backtrace`
# :   See
#     [`backtrace`](https://docs.ruby-lang.org/en/2.6.0/Gem/ConfigFile.html#attribute-i-backtrace)
# `:sources`
# :   Sets
#     [`Gem::sources`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-sources)
# `:verbose`
# :   See
#     [`verbose`](https://docs.ruby-lang.org/en/2.6.0/Gem/ConfigFile.html#attribute-i-verbose)
# `:concurrent_downloads`
# :   See
#     [`concurrent_downloads`](https://docs.ruby-lang.org/en/2.6.0/Gem/ConfigFile.html#attribute-i-concurrent_downloads)
#
#
# gemrc files may exist in various locations and are read and merged in the
# following order:
#
# *   system wide (/etc/gemrc)
# *   per user (~/.gemrc)
# *   per environment (gemrc files listed in the GEMRC environment variable)
class Gem::ConfigFile
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)

  DEFAULT_BULK_THRESHOLD = T.let(T.unsafe(nil), Integer)

  DEFAULT_CERT_EXPIRATION_LENGTH_DAYS = T.let(T.unsafe(nil), Integer)

  DEFAULT_CONCURRENT_DOWNLOADS = T.let(T.unsafe(nil), Integer)

  DEFAULT_UPDATE_SOURCES = T.let(T.unsafe(nil), TrueClass)

  DEFAULT_VERBOSITY = T.let(T.unsafe(nil), TrueClass)

  # For Ruby packagers to set configuration defaults.
  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) in
  # rubygems/defaults/operating\_system.rb
  OPERATING_SYSTEM_DEFAULTS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # For Ruby implementers to set configuration defaults.
  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) in
  # rubygems/defaults/#{RUBY\_ENGINE}.rb
  PLATFORM_DEFAULTS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  SYSTEM_CONFIG_PATH = T.let(T.unsafe(nil), String)

  SYSTEM_WIDE_CONFIG_FILE = T.let(T.unsafe(nil), String)

  # Create the config file object. `args` is the list of arguments from the
  # command line.
  #
  # The following command line options are handled early here rather than later
  # at the time most command options are processed.
  #
  # `--config-file`, `--config-file==NAME`
  # :   Obviously these need to be handled by the
  #     [`ConfigFile`](https://docs.ruby-lang.org/en/2.6.0/Gem/ConfigFile.html)
  #     object to ensure we get the right config file.
  #
  # `--backtrace`
  # :   Backtrace needs to be turned on early so that errors before normal
  #     option parsing can be properly handled.
  #
  # `--debug`
  # :   Enable Ruby level debug messages. Handled early for the same reason as
  #     --backtrace.
  def self.new(args); end

  def ==(other); end

  # Return the configuration information for `key`.
  def [](key); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) configuration option
  # `key` to `value`.
  def []=(key, value); end

  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) of RubyGems.org and
  # alternate API keys
  def api_keys; end

  # List of arguments supplied to the config file object.
  def args; end

  # True if we print backtraces on errors.
  def backtrace; end

  # True if we print backtraces on errors.
  def backtrace=(_); end

  # Bulk threshold value. If the number of missing gems are above this threshold
  # value, then a bulk download technique is used. (deprecated)
  def bulk_threshold; end

  # Bulk threshold value. If the number of missing gems are above this threshold
  # value, then a bulk download technique is used. (deprecated)
  def bulk_threshold=(_); end

  # Expiration length to sign a certificate
  def cert_expiration_length_days; end

  # Expiration length to sign a certificate
  def cert_expiration_length_days=(_); end

  # Checks the permissions of the credentials file. If they are not 0600 an
  # error message is displayed and RubyGems aborts.
  def check_credentials_permissions; end

  # Number of gem downloads that should be performed concurrently.
  def concurrent_downloads; end

  # Number of gem downloads that should be performed concurrently.
  def concurrent_downloads=(_); end

  # The name of the configuration file.
  def config_file_name; end

  # Location of RubyGems.org credentials
  def credentials_path; end

  # True if we want to force specification of gem server when pushing a gem
  def disable_default_gem_server; end

  # True if we want to force specification of gem server when pushing a gem
  def disable_default_gem_server=(_); end

  # Delegates to @hash
  def each(&block); end

  # Handle the command arguments.
  def handle_arguments(arg_list); end

  # Where to install gems (deprecated)
  def home; end

  # Where to install gems (deprecated)
  def home=(_); end

  def load_api_keys; end

  def load_file(filename); end

  # Where to look for gems (deprecated)
  def path; end

  # Where to look for gems (deprecated)
  def path=(_); end

  # Really verbose mode gives you extra output.
  def really_verbose; end

  # Returns the RubyGems.org API key
  def rubygems_api_key; end

  # Sets the RubyGems.org API key to `api_key`
  def rubygems_api_key=(api_key); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) a specific host's API
  # key to `api_key`
  def set_api_key(host, api_key); end

  # sources to look for gems
  def sources; end

  # sources to look for gems
  def sources=(_); end

  # Path name of directory or file of openssl CA certificate, used for remote
  # https connection
  def ssl_ca_cert; end

  # Path name of directory or file of openssl CA certificate, used for remote
  # https connection
  def ssl_ca_cert=(_); end

  # Path name of directory or file of openssl client certificate, used for
  # remote https connection with client authentication
  def ssl_client_cert; end

  # openssl verify mode value, used for remote https connection
  def ssl_verify_mode; end

  def to_yaml; end

  # Remove the +~/.gem/credentials+ file to clear all the current sessions.
  def unset_api_key!; end

  # True if we want to update the SourceInfoCache every time, false otherwise
  def update_sources; end

  # True if we want to update the SourceInfoCache every time, false otherwise
  def update_sources=(_); end

  # Verbose level of output:
  # *   false -- No output
  # *   true -- Normal output
  # *   :loud -- Extra output
  def verbose; end

  # Verbose level of output:
  # *   false -- No output
  # *   true -- Normal output
  # *   :loud -- Extra output
  def verbose=(_); end

  # Writes out this config file, replacing its source.
  def write; end

  protected

  def hash; end
end

# Raised when there are conflicting gem specs loaded
class Gem::ConflictError < ::Gem::LoadError
  def self.new(target, conflicts); end

  # A [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) mapping
  # conflicting specifications to the dependencies that caused the conflict
  def conflicts; end

  # The specification that had the conflict
  def target; end
end

# Subclass of StreamUI that instantiates the user interaction using STDIN,
# STDOUT, and STDERR.
class Gem::ConsoleUI < ::Gem::StreamUI
  # The Console UI has no arguments as it defaults to reading input from stdin,
  # output to stdout and warnings or errors to stderr.
  def self.new; end
end

# When rubygems/test\_case is required the default user interaction is a
# MockGemUi.
# [`Module`](https://docs.ruby-lang.org/en/2.6.0/Module.html) that defines the
# default UserInteraction. Any class including this module will have access to
# the `ui` method that returns the default UI.
module Gem::DefaultUserInteraction
  # See
  # [`DefaultUserInteraction::ui`](https://docs.ruby-lang.org/en/2.6.0/Gem/DefaultUserInteraction.html#method-c-ui)
  def ui; end

  # See
  # [`DefaultUserInteraction::ui=`](https://docs.ruby-lang.org/en/2.6.0/Gem/DefaultUserInteraction.html#method-c-ui-3D)
  def ui=(new_ui); end

  # See
  # [`DefaultUserInteraction::use_ui`](https://docs.ruby-lang.org/en/2.6.0/Gem/DefaultUserInteraction.html#method-c-use_ui)
  def use_ui(new_ui, &block); end

  # Return the default UI.
  def self.ui; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the default UI. If the
  # default UI is never explicitly set, a simple console based UserInteraction
  # will be used automatically.
  def self.ui=(new_ui); end

  # Use `new_ui` for the duration of `block`.
  def self.use_ui(new_ui); end
end

class Gem::Dependency
  # Valid dependency types.
  TYPES = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Constructs a dependency with `name` and `requirements`. The last argument
  # can optionally be the dependency type, which defaults to `:runtime`.
  def self.new(name, *requirements); end

  # Dependencies are ordered by name.
  def <=>(other); end

  def ==(other); end

  # Alias for:
  # [`=~`](https://docs.ruby-lang.org/en/2.6.0/Gem/Dependency.html#method-i-3D~)
  def ===(other); end

  # Uses this dependency as a pattern to compare to `other`. This dependency
  # will match if the name matches the other's name, and other has only an equal
  # version requirement that satisfies this dependency.
  #
  # Also aliased as:
  # [`===`](https://docs.ruby-lang.org/en/2.6.0/Gem/Dependency.html#method-i-3D-3D-3D)
  def =~(other); end

  def all_sources; end

  def all_sources=(_); end

  def encode_with(coder); end

  def eql?(other); end

  def groups; end

  def groups=(_); end

  def hash; end

  def inspect; end

  # Is this dependency simply asking for the latest version of a gem?
  def latest_version?; end

  # Does this dependency match the specification described by `name` and
  # `version` or match `spec`?
  #
  # NOTE:  Unlike
  # [`matches_spec?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Dependency.html#method-i-matches_spec-3F)
  # this method does not return true when the version is a prerelease version
  # unless this is a prerelease dependency.
  def match?(obj, version = _, allow_prerelease = _); end

  # Backport of performance enhancement added to RubyGems 1.4
  def matches_spec?(spec); end

  def matching_specs(platform_only = _); end

  # Merges the requirements of `other` into this dependency
  def merge(other); end

  # [`Dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Dependency.html) name
  # or regular expression.
  def name; end

  # [`Dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Dependency.html) name
  # or regular expression.
  def name=(_); end

  # Allows you to force this dependency to be a prerelease.
  def prerelease=(_); end

  # Does this dependency require a prerelease?
  def prerelease?; end

  def pretty_print(q); end

  def requirement; end

  def requirements_list; end

  def runtime?; end

  def source; end

  def source=(_); end

  # True if the dependency will not always match the latest version.
  def specific?; end

  def to_lock; end

  def to_s; end

  def to_spec; end

  def to_specs; end

  def to_yaml_properties; end

  # [`Dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Dependency.html)
  # type.
  def type; end
end

class Gem::DependencyError < ::Gem::Exception; end

# Installs a gem along with all its dependencies from local and remote gems.
class Gem::DependencyInstaller
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)
  extend(::Gem::Deprecate)

  DEFAULT_OPTIONS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Creates a new installer instance.
  #
  # Options are:
  # :cache\_dir
  # :   Alternate repository path to store .gem files in.
  # :domain
  # :   :local, :remote, or :both. :local only searches gems in the current
  #     directory. :remote searches only gems in
  #     [`Gem::sources`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-sources).
  #     :both searches both.
  # :env\_shebang
  # :   See
  #     [`Gem::Installer::new`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-c-new).
  # :force
  # :   See
  #     [`Gem::Installer#install`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-i-install).
  # :format\_executable
  # :   See Gem::Installer#initialize.
  # :ignore\_dependencies
  # :   Don't install any dependencies.
  # :install\_dir
  # :   See
  #     [`Gem::Installer#install`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-i-install).
  # :prerelease
  # :   Allow prerelease versions. See
  #     [`install`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyInstaller.html#method-i-install).
  # :security\_policy
  # :   See
  #     [`Gem::Installer::new`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-c-new)
  #     and
  #     [`Gem::Security`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html).
  # :user\_install
  # :   See
  #     [`Gem::Installer.new`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-c-new)
  # :wrappers
  # :   See
  #     [`Gem::Installer::new`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-c-new)
  # :build\_args
  # :   See
  #     [`Gem::Installer::new`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-c-new)
  def self.new(options = _); end

  def _deprecated_add_found_dependencies(to_do, dependency_list); end

  def _deprecated_gather_dependencies; end

  def add_found_dependencies(*args, &block); end

  def available_set_for(dep_or_name, version); end

  # Indicated, based on the requested domain, if local gems should be
  # considered.
  def consider_local?; end

  # Indicated, based on the requested domain, if remote gems should be
  # considered.
  def consider_remote?; end

  # Documentation types. For use by the
  # [`Gem.done_installing`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-done_installing)
  # hook
  def document; end

  # Errors from SpecFetcher while searching for remote specifications
  def errors; end

  def find_gems_with_sources(dep, best_only = _); end

  # Finds a spec and the source\_uri it came from for gem `gem_name` and
  # `version`. Returns an
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of specs and
  # sources required for installation of the gem.
  def find_spec_by_name_and_version(gem_name, version = _, prerelease = _); end

  def gather_dependencies(*args, &block); end

  def in_background(what); end

  # Installs the gem `dep_or_name` and all its dependencies. Returns an
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of installed gem
  # specifications.
  #
  # If the `:prerelease` option is set and there is a prerelease for
  # `dep_or_name` the prerelease version will be installed.
  #
  # Unless explicitly specified as a prerelease dependency, prerelease gems that
  # `dep_or_name` depend on will not be installed.
  #
  # If c-1.a depends on b-1 and a-1.a and there is a gem b-1.a available then
  # c-1.a, b-1 and a-1.a will be installed. b-1.a will need to be installed
  # separately.
  def install(dep_or_name, version = _); end

  def install_development_deps; end

  # List of gems installed by
  # [`install`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyInstaller.html#method-i-install)
  # in alphabetic order
  def installed_gems; end

  def resolve_dependencies(dep_or_name, version); end
end

# [`Gem::DependencyList`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyList.html)
# is used for installing and uninstalling gems in the correct order to avoid
# conflicts.
class Gem::DependencyList
  include(::TSort)
  include(::Enumerable)

  Elem = type_member(:out)

  # Creates a new
  # [`DependencyList`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyList.html).
  # If `development` is true, development dependencies will be included.
  def self.new(development = _); end

  # Adds `gemspecs` to the dependency list.
  def add(*gemspecs); end

  def clear; end

  # Return a list of the gem specifications in the dependency list, sorted in
  # order so that no gemspec in the list depends on a gemspec earlier in the
  # list.
  #
  # This is useful when removing gems from a set of installed gems. By removing
  # them in the returned order, you don't get into as many dependency issues.
  #
  # If there are circular dependencies (yuck!), then gems will be returned in
  # order until only the circular dependents and anything they reference are
  # left. Then arbitrary gemspecs will be returned until the circular dependency
  # is broken, after which gems will be returned in dependency order again.
  def dependency_order; end

  # Allows enabling/disabling use of development dependencies
  def development; end

  # Allows enabling/disabling use of development dependencies
  def development=(_); end

  # Iterator over
  # [`dependency_order`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyList.html#method-i-dependency_order)
  def each(&block); end

  def find_name(full_name); end

  def inspect; end

  # Are all the dependencies in the list satisfied?
  def ok?; end

  # Is is ok to remove a gemspec from the dependency list?
  #
  # If removing the gemspec creates breaks a currently ok dependency, then it is
  # NOT ok to remove the gemspec.
  def ok_to_remove?(full_name, check_dev = _); end

  # Removes the gemspec matching `full_name` from the dependency list
  def remove_by_name(full_name); end

  # Remove everything in the
  # [`DependencyList`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyList.html)
  # that matches but doesn't satisfy items in `dependencies` (a hash of gem
  # names to arrays of dependencies).
  def remove_specs_unsatisfied_by(dependencies); end

  # Return a hash of predecessors. `result[spec]` is an
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of gemspecs that
  # have a dependency satisfied by the named gemspec.
  def spec_predecessors; end

  def specs; end

  def tsort_each_child(node); end

  def tsort_each_node(&block); end

  def why_not_ok?(quick = _); end

  # Creates a
  # [`DependencyList`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyList.html)
  # from the current specs.
  def self.from_specs; end
end

class Gem::DependencyRemovalException < ::Gem::Exception; end

# Raised by
# [`Gem::Resolver`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver.html) when
# a Gem::Dependency::Conflict reaches the toplevel. Indicates which dependencies
# were incompatible through
# [`conflict`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyResolutionError.html#attribute-i-conflict)
# and
# [`conflicting_dependencies`](https://docs.ruby-lang.org/en/2.6.0/Gem/DependencyResolutionError.html#method-i-conflicting_dependencies)
class Gem::DependencyResolutionError < ::Gem::DependencyError
  def self.new(conflict); end

  def conflict; end

  def conflicting_dependencies; end
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
  # Simple deprecation method that deprecates `name` by wrapping it up in a
  # dummy method. It warns on each call to the dummy method telling the user of
  # `repl` (unless `repl` is :none) and the year/month that it is planned to go
  # away.
  def self.deprecate(name, repl, year, month); end

  def self.skip; end

  def self.skip=(v); end

  # Temporarily turn off warnings. Intended for tests only.
  def self.skip_during; end
end

class Gem::DocumentError < ::Gem::Exception; end

# Potentially raised when a specification is validated.
class Gem::EndOfYAMLException < ::Gem::Exception; end

class Gem::ErrorReason; end

# Base exception class for RubyGems. All exception raised by RubyGems are a
# subclass of this one.
class Gem::Exception < ::RuntimeError
  extend(::Gem::Deprecate)

  def _deprecated_source_exception; end

  def source_exception(*args, &block); end

  def source_exception=(_); end
end

# Classes for building C extensions live here.
module Gem::Ext; end

# Raised when there is an error while building extensions.
class Gem::Ext::BuildError < ::Gem::InstallError; end

class Gem::Ext::Builder
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)

  CHDIR_MONITOR = T.let(T.unsafe(nil), Monitor)

  CHDIR_MUTEX = T.let(T.unsafe(nil), Monitor)

  # Creates a new extension builder for `spec`. If the `spec` does not yet have
  # build arguments, saved, set `build_args` which is an ARGV-style array.
  def self.new(spec, build_args = _); end

  def build_args; end

  def build_args=(_); end

  def build_error(build_dir, output, backtrace = _); end

  def build_extension(extension, dest_path); end

  # Builds extensions. Valid types of extensions are extconf.rb files, configure
  # scripts and rakefiles or mkrf\_conf files.
  def build_extensions; end

  def builder_for(extension); end

  def write_gem_make_out(output); end

  def self.class_name; end

  def self.make(dest_path, results); end

  def self.redirector; end

  def self.run(command, results, command_name = _); end
end

class Gem::Ext::CmakeBuilder < ::Gem::Ext::Builder
  def self.build(extension, dest_path, results, args = _, lib_dir = _); end
end

class Gem::Ext::ConfigureBuilder < ::Gem::Ext::Builder
  def self.build(extension, dest_path, results, args = _, lib_dir = _); end
end

class Gem::Ext::ExtConfBuilder < ::Gem::Ext::Builder
  FileEntry = FileUtils::Entry_

  def self.build(extension, dest_path, results, args = _, lib_dir = _); end

  def self.get_relative_path(path); end
end

class Gem::Ext::RakeBuilder < ::Gem::Ext::Builder
  def self.build(extension, dest_path, results, args = _, lib_dir = _); end
end

# Signals that a file permission error is preventing the user from operating on
# the given directory.
class Gem::FilePermissionError < ::Gem::Exception
  def self.new(directory); end

  def directory; end
end

# Used to raise parsing and loading errors
class Gem::FormatException < ::Gem::Exception
  def file_path; end

  def file_path=(_); end
end

class Gem::GemNotFoundException < ::Gem::Exception; end

# Raised when attempting to uninstall a gem that isn't in GEM\_HOME.
class Gem::GemNotInHomeException < ::Gem::Exception
  def spec; end

  def spec=(_); end
end

# Run an instance of the gem program.
#
# [`Gem::GemRunner`](https://docs.ruby-lang.org/en/2.6.0/Gem/GemRunner.html) is
# only intended for internal use by RubyGems itself. It does not form any public
# API and may change at any time for any reason.
#
# If you would like to duplicate functionality of `gem` commands, use the
# classes they call directly.
class Gem::GemRunner
  def self.new(options = _); end

  def extract_build_args(args); end

  # Run the gem command with the following arguments.
  def run(args); end
end

# Raised by
# [`Gem::Resolver`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver.html) when
# dependencies conflict and create the inability to find a valid possible spec
# for a request.
class Gem::ImpossibleDependenciesError < ::Gem::Exception
  def self.new(request, conflicts); end

  def build_message; end

  def conflicts; end

  def dependency; end

  def request; end
end

# Top level class for building the gem repository index.
class Gem::Indexer
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)

  # Create an indexer that will index the gems in `directory`.
  def self.new(directory, options = _); end

  # Build various indices
  def build_indices; end

  # Builds [`Marshal`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html) quick
  # index gemspecs.
  def build_marshal_gemspecs(specs); end

  # Build indexes for RubyGems 1.2.0 and newer when true
  def build_modern; end

  # Build indexes for RubyGems 1.2.0 and newer when true
  def build_modern=(_); end

  # Build a single index for RubyGems 1.2 and newer
  def build_modern_index(index, file, name); end

  # Builds indices for RubyGems 1.2 and newer. Handles full, latest, prerelease
  def build_modern_indices(specs); end

  # Compacts [`Marshal`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html)
  # output for the specs index data source by using identical objects as much as
  # possible.
  def compact_specs(specs); end

  # Compress `filename` with `extension`.
  def compress(filename, extension); end

  # Compresses indices on disk
  def compress_indices; end

  # Index install location
  def dest_directory; end

  # Latest specs index install location
  def dest_latest_specs_index; end

  # Prerelease specs index install location
  def dest_prerelease_specs_index; end

  # Specs index install location
  def dest_specs_index; end

  # Index build directory
  def directory; end

  # List of gem file names to index.
  def gem_file_list; end

  # Builds and installs indices.
  def generate_index; end

  # [`Zlib::GzipWriter`](https://docs.ruby-lang.org/en/2.6.0/Zlib/GzipWriter.html)
  # wrapper that gzips `filename` on disk.
  def gzip(filename); end

  # Install generated indices into the destination directory.
  def install_indices; end

  # Make directories for index generation
  def make_temp_directories; end

  def map_gems_to_specs(gems); end

  # Ensure `path` and path with `extension` are identical.
  def paranoid(path, extension); end

  # Perform an in-place update of the repository from newly added gems.
  def update_index; end

  # Combines specs in `index` and `source` then writes out a new copy to `dest`.
  # For a latest index, does not ensure the new file is minimal.
  def update_specs_index(index, source, dest); end
end

class Gem::InstallError < ::Gem::Exception; end

# The installer installs the files contained in the .gem into the Gem.home.
#
# [`Gem::Installer`](https://docs.ruby-lang.org/en/2.6.0/Gem/Installer.html)
# does the work of putting files in all the right places on the filesystem
# including unpacking the gem into its gem dir, installing the gemspec in the
# specifications dir, storing the cached gem in the cache dir, and installing
# either wrappers or symlinks for executables.
#
# The installer invokes pre and post install hooks. Hooks can be added either
# through a rubygems\_plugin.rb file in an installed gem or via a
# rubygems/defaults/#{RUBY\_ENGINE}.rb or rubygems/defaults/operating\_system.rb
# file. See
# [`Gem.pre_install`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-pre_install)
# and
# [`Gem.post_install`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-post_install)
# for details.
class Gem::Installer
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)
  extend(::Gem::Deprecate)

  # Paths where env(1) might live. Some systems are broken and have it in /bin
  ENV_PATHS = T.let(T.unsafe(nil), T::Array[T.untyped])

  ExtensionBuildError = Gem::Ext::BuildError

  # Constructs an
  # [`Installer`](https://docs.ruby-lang.org/en/2.6.0/Installer.html) instance
  # that will install the gem at `package` which can either be a path or an
  # instance of
  # [`Gem::Package`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package.html).
  # `options` is a [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) with
  # the following keys:
  #
  # :bin\_dir
  # :   Where to put a bin wrapper if needed.
  # :development
  # :   Whether or not development dependencies should be installed.
  # :env\_shebang
  # :   Use /usr/bin/env in bin wrappers.
  # :force
  # :   Overrides all version checks and security policy checks, except for a
  #     signed-gems-only policy.
  # :format\_executable
  # :   Format the executable the same as the Ruby executable. If your Ruby is
  #     ruby18, foo\_exec will be installed as foo\_exec18.
  # :ignore\_dependencies
  # :   Don't raise if a dependency is missing.
  # :install\_dir
  # :   The directory to install the gem into.
  # :security\_policy
  # :   Use the specified security policy. See
  #     [`Gem::Security`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html)
  # :user\_install
  # :   Indicate that the gem should be unpacked into the users personal gem
  #     directory.
  # :only\_install\_dir
  # :   Only validate dependencies against what is in the install\_dir
  # :wrappers
  # :   Install wrappers if true, symlinks if false.
  # :build\_args
  # :   An [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  #     arguments to pass to the extension builder process. If not set, then
  #     [`Gem::Command.build_args`](https://docs.ruby-lang.org/en/2.6.0/Gem/Command.html#method-c-build_args)
  #     is used
  # :post\_install\_message
  # :   Print gem post install message if true
  def self.new(package, options = _); end

  def _deprecated_extension_build_error(build_dir, output, backtrace = _); end

  # Return the text for an application file.
  def app_script_text(bin_file_name); end

  # The directory a gem's executables will be installed into
  def bin_dir; end

  # Builds extensions. Valid types of extensions are extconf.rb files, configure
  # scripts and rakefiles or mkrf\_conf files.
  def build_extensions; end

  def build_root; end

  def check_executable_overwrite(filename); end

  def check_that_user_bin_dir_is_in_path; end

  # The location of the default spec file for default gems.
  def default_spec_file; end

  # Return the target directory where the gem is to be installed. This directory
  # is not guaranteed to be populated.
  def dir; end

  def ensure_dependencies_met; end

  # Ensure that the dependency is satisfied by the current installation of gem.
  # If it is not an exception is raised.
  #
  # spec
  # :   [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # dependency
  # :   [`Gem::Dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Dependency.html)
  def ensure_dependency(spec, dependency); end

  # Ensures the
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # written out for this gem is loadable upon installation.
  def ensure_loadable_spec; end

  def ensure_required_ruby_version_met; end

  def ensure_required_rubygems_version_met; end

  def extension_build_error(*args, &block); end

  # Extracts only the bin/ files from the gem into the gem directory. This is
  # used by default gems to allow a gem-aware stub to function without the full
  # gem installed.
  def extract_bin; end

  # Reads the file index and extracts each file into the gem directory.
  #
  # Ensures that files can't be installed outside the gem directory.
  def extract_files; end

  # Prefix and suffix the program filename the same as ruby.
  def formatted_program_filename(filename); end

  # Filename of the gem being installed.
  def gem; end

  # Available through requiring rubygems/installer\_test\_case
  def gem_dir; end

  # The gem repository the gem will be installed into
  def gem_home; end

  def generate_bin; end

  # Creates the scripts to run the applications in the gem.
  def generate_bin_script(filename, bindir); end

  # Creates the symlinks to run the applications in the gem. Moves the symlink
  # if the gem being installed has a newer version.
  def generate_bin_symlink(filename, bindir); end

  # Creates windows .bat files for easy running of commands
  def generate_windows_script(filename, bindir); end

  # Installs the gem and returns a loaded
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # for the installed gem.
  #
  # The gem will be installed with the following structure:
  #
  # ```
  # @gem_home/
  #   cache/<gem-version>.gem #=> a cached copy of the installed gem
  #   gems/<gem-version>/... #=> extracted files
  #   specifications/<gem-version>.gemspec #=> the Gem::Specification
  # ```
  def install; end

  # True if the gems in the system satisfy `dependency`.
  def installation_satisfies_dependency?(dependency); end

  # Return an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # Specifications contained within the gem\_home we'll be installing into.
  def installed_specs; end

  # The options passed when the
  # [`Gem::Installer`](https://docs.ruby-lang.org/en/2.6.0/Gem/Installer.html)
  # was instantiated.
  def options; end

  # Performs various checks before installing the gem such as the install
  # repository is writable and its directories exist, required Ruby and rubygems
  # versions are met and that dependencies are installed.
  #
  # Version and dependency checks are skipped if this install is forced.
  #
  # The dependent check will be skipped if the install is ignoring dependencies.
  def pre_install_checks; end

  def process_options; end

  def run_post_build_hooks; end

  def run_post_install_hooks; end

  def run_pre_install_hooks; end

  # Generates a #! line for `bin_file_name`'s wrapper copying arguments if
  # necessary.
  #
  # If the :custom\_shebang config is set, then it is used as a template for how
  # to create the shebang used for to run a gem's executables.
  #
  # The template supports 4 expansions:
  #
  # ```
  # $env    the path to the unix env utility
  # $ruby   the path to the currently running ruby interpreter
  # $exec   the path to the gem's executable
  # $name   the name of the gem the executable is for
  # ```
  def shebang(bin_file_name); end

  # Lazy accessor for the installer's spec.
  def spec; end

  # The location of the spec file that is installed.
  def spec_file; end

  # Unpacks the gem into the given directory.
  def unpack(directory); end

  def verify_gem_home(unpack = _); end

  def verify_spec_name; end

  # return the stub script text used to launch the true Ruby script
  def windows_stub_script(bindir, bin_file_name); end

  # Writes the file containing the arguments for building this gem's extensions.
  def write_build_info_file; end

  # Writes the .gem file to the cache directory
  def write_cache_file; end

  # Writes the full .gemspec specification (in Ruby) to the gem home's
  # specifications/default directory.
  def write_default_spec; end

  # Writes the .gemspec specification (in Ruby) to the gem home's specifications
  # directory.
  def write_spec; end

  # Construct an installer object for the gem file located at `path`
  def self.at(path, options = _); end

  # Overrides the executable format.
  #
  # This is a sprintf format with a "%s" which will be replaced with the
  # executable name. It is based off the ruby executable name's difference from
  # "ruby".
  def self.exec_format; end

  # Overrides the executable format.
  #
  # This is a sprintf format with a "%s" which will be replaced with the
  # executable name. It is based off the ruby executable name's difference from
  # "ruby".
  def self.exec_format=(_); end

  # Construct an installer object for an ephemeral gem (one where we don't
  # actually have a .gem file, just a spec)
  def self.for_spec(spec, options = _); end

  # Certain aspects of the install process are not thread-safe. This lock is
  # used to allow multiple threads to install Gems at the same time.
  def self.install_lock; end

  # True if we've warned about PATH not including
  # [`Gem.bindir`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-bindir)
  def self.path_warning; end

  # True if we've warned about PATH not including
  # [`Gem.bindir`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-bindir)
  def self.path_warning=(_); end
end

class Gem::Installer::FakePackage
  def self.new(spec); end

  def copy_to(path); end

  def data_mode; end

  def data_mode=(_); end

  def dir_mode; end

  def dir_mode=(_); end

  def extract_files(destination_dir, pattern = _); end

  def prog_mode; end

  def prog_mode=(_); end

  def spec; end

  def spec=(_); end
end

# Potentially raised when a specification is validated.
class Gem::InvalidSpecificationException < ::Gem::Exception; end

class Gem::Licenses
  extend(::Gem::Text)

  # exception identifiers
  EXCEPTION_IDENTIFIERS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Software Package [`Data`](https://docs.ruby-lang.org/en/2.6.0/Data.html)
  # Exchange (SPDX) standard open-source software license identifiers
  LICENSE_IDENTIFIERS = T.let(T.unsafe(nil), T::Array[T.untyped])

  NONSTANDARD = T.let(T.unsafe(nil), String)

  REGEXP = T.let(T.unsafe(nil), Regexp)

  def self.match?(license); end

  def self.suggestions(license); end
end

class Gem::List
  include(::Enumerable)

  def self.new(value = _, tail = _); end

  def each; end

  def prepend(value); end

  def pretty_print(q); end

  def tail; end

  def tail=(_); end

  def to_a; end

  def value; end

  def value=(_); end

  def self.prepend(list, value); end
end

# Raised when RubyGems is unable to load or activate a gem. Contains the name
# and version requirements of the gem that either conflicts with already
# activated gems or that RubyGems is otherwise unable to activate.
class Gem::LoadError < ::LoadError
  # Name of gem
  def name; end

  # Name of gem
  def name=(_); end

  # Version requirement of gem
  def requirement; end

  # Version requirement of gem
  def requirement=(_); end
end

# Raised when trying to activate a gem, and that gem does not exist on the
# system. Instead of rescuing from this class, make sure to rescue from the
# superclass
# [`Gem::LoadError`](https://docs.ruby-lang.org/en/2.6.0/Gem/LoadError.html) to
# catch all types of load errors.
class Gem::MissingSpecError < ::Gem::LoadError
  def self.new(name, requirement); end

  def message; end
end

# Raised when trying to activate a gem, and the gem exists on the system, but
# not the requested version. Instead of rescuing from this class, make sure to
# rescue from the superclass
# [`Gem::LoadError`](https://docs.ruby-lang.org/en/2.6.0/Gem/LoadError.html) to
# catch all types of load errors.
class Gem::MissingSpecVersionError < ::Gem::MissingSpecError
  def self.new(name, requirement, specs); end

  def specs; end
end

class Gem::NameTuple
  include(::Comparable)

  def self.new(name, version, platform = _); end

  def <=>(other); end

  # Compare with `other`. Supports another
  # [`NameTuple`](https://docs.ruby-lang.org/en/2.6.0/Gem/NameTuple.html) or an
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) in the [name,
  # version, platform] format.
  #
  # Also aliased as:
  # [`eql?`](https://docs.ruby-lang.org/en/2.6.0/Gem/NameTuple.html#method-i-eql-3F)
  def ==(other); end

  # Alias for:
  # [`==`](https://docs.ruby-lang.org/en/2.6.0/Gem/NameTuple.html#method-i-3D-3D)
  def eql?(other); end

  # Returns the full name (name-version) of this
  # [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html). Platform information
  # is included if it is not the default Ruby platform. This mimics the behavior
  # of
  # [`Gem::Specification#full_name`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-full_name).
  def full_name; end

  def hash; end

  def inspect; end

  # Indicate if this
  # [`NameTuple`](https://docs.ruby-lang.org/en/2.6.0/Gem/NameTuple.html)
  # matches the current platform.
  def match_platform?; end

  def name; end

  def platform; end

  # Indicate if this
  # [`NameTuple`](https://docs.ruby-lang.org/en/2.6.0/Gem/NameTuple.html) is for
  # a prerelease version.
  def prerelease?; end

  # Return the name that the gemspec file would be
  def spec_name; end

  # Convert back to the [name, version, platform] tuple
  def to_a; end

  def to_s; end

  def version; end

  # Turn an array of [name, version, platform] into an array of
  # [`NameTuple`](https://docs.ruby-lang.org/en/2.6.0/Gem/NameTuple.html)
  # objects.
  def self.from_list(list); end

  # A null
  # [`NameTuple`](https://docs.ruby-lang.org/en/2.6.0/Gem/NameTuple.html), ie
  # name=nil, version=0
  def self.null; end

  # Turn an array of
  # [`NameTuple`](https://docs.ruby-lang.org/en/2.6.0/Gem/NameTuple.html)
  # objects back into an array of
  # name, version, platform
  # :   tuples.
  def self.to_basic(list); end
end

class Gem::OperationNotSupportedError < ::Gem::Exception; end

class Gem::Package
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)

  # Creates a new
  # [`Gem::Package`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package.html) for
  # the file at `gem`. `gem` can also be provided as an
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object.
  #
  # If `gem` is an existing file in the old format a
  # [`Gem::Package::Old`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/Old.html)
  # will be returned.
  def self.new(gem, security_policy); end

  # Adds a checksum for each entry in the gem to checksums.yaml.gz.
  def add_checksums(tar); end

  def add_contents(tar); end

  def add_files(tar); end

  def add_metadata(tar); end

  # Builds this package based on the specification set by
  # [`spec=`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package.html#attribute-i-spec)
  def build(skip_validation = _, strict_validation = _); end

  def build_time; end

  def build_time=(_); end

  # Checksums for the contents of the package
  def checksums; end

  # A list of file names contained in this gem
  def contents; end

  # Copies this package to `path` (if possible)
  def copy_to(path); end

  # Permission for other files
  def data_mode; end

  # Permission for other files
  def data_mode=(_); end

  def digest(entry); end

  # Permission for directories
  def dir_mode; end

  # Permission for directories
  def dir_mode=(_); end

  # Extracts the files in this package into `destination_dir`
  #
  # If `pattern` is specified, only entries matching that glob will be
  # extracted.
  def extract_files(destination_dir, pattern = _); end

  def extract_tar_gz(io, destination_dir, pattern = _); end

  def file_mode(mode); end

  # The files in this package. This is not the contents of the gem, just the
  # files in the top-level container.
  def files; end

  # Gzips content written to `gz_io` to `io`.
  def gzip_to(io); end

  def install_location(filename, destination_dir); end

  def load_spec(entry); end

  def mkdir_p_safe(mkdir, mkdir_options, destination_dir, file_name); end

  def normalize_path(pathname); end

  def open_tar_gz(io); end

  # Permission for program files
  def prog_mode; end

  # Permission for program files
  def prog_mode=(_); end

  # Reads and loads checksums.yaml.gz from the tar file `gem`
  def read_checksums(gem); end

  # The security policy used for verifying the contents of this package.
  def security_policy; end

  # The security policy used for verifying the contents of this package.
  def security_policy=(_); end

  # Prepares the gem for signing and checksum generation. If a signing
  # certificate and key are not present only checksum generation is set up.
  def setup_signer(signer_options: _); end

  # Sets the
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # to use to build this package.
  def spec; end

  # Sets the
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # to use to build this package.
  def spec=(_); end

  # Verifies that this gem:
  #
  # *   Contains a valid gem specification
  # *   Contains a contents archive
  # *   The contents archive is not corrupt
  #
  #
  # After verification the gem specification from the gem is available from
  # [`spec`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package.html#attribute-i-spec)
  def verify; end

  def verify_checksums(digests, checksums); end

  # Verifies `entry` in a .gem file.
  def verify_entry(entry); end

  # Verifies the files of the `gem`
  def verify_files(gem); end

  def verify_gz(entry); end

  def self.build(spec, skip_validation = _, strict_validation = _, file_name = _); end

  # Creates a new
  # [`Gem::Package`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package.html) for
  # the file at `gem`. `gem` can also be provided as an
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object.
  #
  # If `gem` is an existing file in the old format a
  # [`Gem::Package::Old`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/Old.html)
  # will be returned.
  def self.new(gem, security_policy = _); end
end

# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) wrapper that creates
# digests of contents written to the
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) it wraps.
class Gem::Package::DigestIO
  # Creates a new
  # [`DigestIO`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/DigestIO.html)
  # instance. Using
  # [`::wrap`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/DigestIO.html#method-c-wrap)
  # is recommended, see the
  # [`::wrap`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/DigestIO.html#method-c-wrap)
  # documentation for documentation of `io` and `digests`.
  def self.new(io, digests); end

  # Collected digests for wrapped writes.
  #
  # ```
  # {
  #   'SHA1'   => #<OpenSSL::Digest: [...]>,
  #   'SHA512' => #<OpenSSL::Digest: [...]>,
  # }
  # ```
  def digests; end

  # Writes `data` to the underlying
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) and updates the digests
  def write(data); end

  # Wraps `io` and updates digest for each of the digest algorithms in the
  # `digests` [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html). Returns
  # the digests hash. Example:
  #
  # ```ruby
  # io = StringIO.new
  # digests = {
  #   'SHA1'   => OpenSSL::Digest.new('SHA1'),
  #   'SHA512' => OpenSSL::Digest.new('SHA512'),
  # }
  #
  # Gem::Package::DigestIO.wrap io, digests do |digest_io|
  #   digest_io.write "hello"
  # end
  #
  # digests['SHA1'].hexdigest   #=> "aaf4c61d[...]"
  # digests['SHA512'].hexdigest #=> "9b71d224[...]"
  # ```
  def self.wrap(io, digests); end
end

class Gem::Package::Error < ::Gem::Exception; end

class Gem::Package::FileSource < ::Gem::Package::Source
  def self.new(path); end

  def path; end

  def present?; end

  def start; end

  def with_read_io(&block); end

  def with_write_io(&block); end
end

class Gem::Package::FormatError < ::Gem::Package::Error
  def self.new(message, source = _); end

  def path; end
end

class Gem::Package::IOSource < ::Gem::Package::Source
  def self.new(io); end

  def io; end

  def path; end

  def present?; end

  def start; end

  def with_read_io; end

  def with_write_io; end
end

class Gem::Package::NonSeekableIO < ::Gem::Package::Error; end

# The format class knows the guts of the ancient .gem file format and provides
# the capability to read such ancient gems.
#
# Please pretend this doesn't exist.
class Gem::Package::Old < ::Gem::Package
  # Creates a new old-format package reader for `gem`. Old-format packages
  # cannot be written.
  def self.new(gem, security_policy); end

  # A list of file names contained in this gem
  def contents; end

  # Extracts the files in this package into `destination_dir`
  def extract_files(destination_dir); end

  def file_list(io); end

  def read_until_dashes(io); end

  def skip_ruby(io); end

  # The specification for this gem
  def spec; end

  # Raises an exception if a security policy that verifies data is active.
  # [`Old`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/Old.html) format
  # gems cannot be verified as signed.
  def verify; end
end

class Gem::Package::PathError < ::Gem::Package::Error
  def self.new(destination, destination_dir); end
end

class Gem::Package::Source; end

# A header for a tar file
class Gem::Package::TarHeader
  EMPTY_HEADER = T.let(T.unsafe(nil), String)

  # Fields in the tar header
  FIELDS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Pack format for a tar header
  PACK_FORMAT = T.let(T.unsafe(nil), String)

  # Unpack format for a tar header
  UNPACK_FORMAT = T.let(T.unsafe(nil), String)

  # Creates a new
  # [`TarHeader`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarHeader.html)
  # using `vals`
  def self.new(vals); end

  def ==(other); end

  def checksum; end

  def devmajor; end

  def devminor; end

  # Is the tar entry empty?
  def empty?; end

  def gid; end

  def gname; end

  def linkname; end

  def magic; end

  def mode; end

  def mtime; end

  def name; end

  def prefix; end

  def size; end

  def to_s; end

  def typeflag; end

  def uid; end

  def uname; end

  # Updates the TarHeader's checksum
  def update_checksum; end

  def version; end

  # Creates a tar header from
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) `stream`
  def self.from(stream); end

  def self.strict_oct(str); end
end

# Raised when a tar file is corrupt
class Gem::Package::TarInvalidError < ::Gem::Package::Error; end

# [`TarReader`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarReader.html)
# reads tar files and allows iteration over their items
class Gem::Package::TarReader
  include(::Enumerable)

  Elem = type_member(:out)

  # Creates a new tar file reader on `io` which needs to respond to pos, eof?,
  # read, getc and pos=
  def self.new(io); end

  # Close the tar file
  def close; end

  # Iterates over files in the tarball yielding each entry
  #
  # Also aliased as:
  # [`each_entry`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarReader.html#method-i-each_entry)
  def each; end

  # Alias for:
  # [`each`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarReader.html#method-i-each)
  def each_entry; end

  # NOTE: Do not call
  # [`rewind`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarReader.html#method-i-rewind)
  # during
  # [`each`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarReader.html#method-i-each)
  def rewind; end

  # Seeks through the tar file until it finds the `entry` with `name` and yields
  # it. Rewinds the tar file to the beginning when the block terminates.
  def seek(name); end

  # Creates a new tar file reader on `io` which needs to respond to pos, eof?,
  # read, getc and pos=
  def self.new(io); end
end

# [`Class`](https://docs.ruby-lang.org/en/2.6.0/Class.html) for reading entries
# out of a tar file
class Gem::Package::TarReader::Entry
  # Creates a new tar entry for `header` that will be read from `io`
  def self.new(header, io); end

  # Number of bytes read out of the tar entry
  def bytes_read; end

  def check_closed; end

  # Closes the tar entry
  def close; end

  # Is the tar entry closed?
  def closed?; end

  # Is this tar entry a directory?
  def directory?; end

  # Are we at the end of the tar entry?
  def eof?; end

  # Is this tar entry a file?
  def file?; end

  # Full name of the tar entry
  def full_name; end

  # Read one byte from the tar entry
  def getc; end

  # Header for this tar entry
  def header; end

  # Alias for:
  # [`size`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarReader/Entry.html#method-i-size)
  def length; end

  # The position in the tar entry
  def pos; end

  # Reads `len` bytes from the tar file entry, or the rest of the entry if nil
  def read(len = _); end

  def readpartial(maxlen = _, outbuf = _); end

  # Rewinds to the beginning of the tar file entry
  def rewind; end

  # Also aliased as:
  # [`length`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarReader/Entry.html#method-i-length)
  def size; end

  # Is this tar entry a symlink?
  def symlink?; end
end

# Raised if the tar [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) is not
# seekable
class Gem::Package::TarReader::UnexpectedEOF < ::StandardError; end

# Allows writing of tar files
class Gem::Package::TarWriter
  # Creates a new
  # [`TarWriter`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarWriter.html)
  # that will write to `io`
  def self.new(io); end

  # Adds file `name` with permissions `mode`, and yields an
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) for writing the file to
  def add_file(name, mode); end

  # Adds `name` with permissions `mode` to the tar, yielding `io` for writing
  # the file. The `digest_algorithm` is written to a read-only `name`.sum file
  # following the given file contents containing the digest name and hexdigest
  # separated by a tab.
  #
  # The created digest object is returned.
  def add_file_digest(name, mode, digest_algorithms); end

  # Adds `name` with permissions `mode` to the tar, yielding `io` for writing
  # the file. The `signer` is used to add a digest file using its
  # digest\_algorithm per
  # [`add_file_digest`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarWriter.html#method-i-add_file_digest)
  # and a cryptographic signature in `name`.sig. If the signer has no key only
  # the checksum file is added.
  #
  # Returns the digest.
  def add_file_signed(name, mode, signer); end

  # Add file `name` with permissions `mode` `size` bytes long. Yields an
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) to write the file to.
  def add_file_simple(name, mode, size); end

  # Adds symlink `name` with permissions `mode`, linking to `target`.
  def add_symlink(name, target, mode); end

  # Raises [`IOError`](https://docs.ruby-lang.org/en/2.6.0/IOError.html) if the
  # [`TarWriter`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarWriter.html)
  # is closed
  def check_closed; end

  # Closes the
  # [`TarWriter`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarWriter.html)
  def close; end

  # Is the
  # [`TarWriter`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarWriter.html)
  # closed?
  def closed?; end

  # Flushes the TarWriter's [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html)
  def flush; end

  # Creates a new directory in the tar file `name` with `mode`
  def mkdir(name, mode); end

  def split_name(name); end

  # Creates a new
  # [`TarWriter`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarWriter.html)
  # that will write to `io`
  def self.new(io); end
end

# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) wrapper that allows
# writing a limited amount of data
class Gem::Package::TarWriter::BoundedStream
  # Wraps `io` and allows up to `limit` bytes to be written
  def self.new(io, limit); end

  # Maximum number of bytes that can be written
  def limit; end

  # Writes `data` onto the [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html),
  # raising a FileOverflow exception if the number of bytes will be more than
  # [`limit`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarWriter/BoundedStream.html#attribute-i-limit)
  def write(data); end

  # Number of bytes written
  def written; end
end

class Gem::Package::TarWriter::FileOverflow < ::StandardError; end

# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) wrapper that provides only
# [`write`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarWriter/RestrictedStream.html#method-i-write)
class Gem::Package::TarWriter::RestrictedStream
  # Creates a new
  # [`RestrictedStream`](https://docs.ruby-lang.org/en/2.6.0/Gem/Package/TarWriter/RestrictedStream.html)
  # wrapping `io`
  def self.new(io); end

  # Writes `data` onto the [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html)
  def write(data); end
end

class Gem::Package::TooLongFileName < ::Gem::Package::Error; end

# [`Gem::PathSupport`](https://docs.ruby-lang.org/en/2.6.0/Gem/PathSupport.html)
# facilitates the GEM\_HOME and GEM\_PATH environment settings to the rest of
# RubyGems.
class Gem::PathSupport
  # Constructor. Takes a single argument which is to be treated like a
  # hashtable, or defaults to
  # [`ENV`](https://docs.ruby-lang.org/en/2.6.0/ENV.html), the system
  # environment.
  def self.new(env); end

  # The default system path for managing Gems.
  def home; end

  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of paths to search
  # for Gems.
  def path; end

  def spec_cache_dir; end
end

# Available list of platforms for targeting
# [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html) installations.
#
# See `gem help platform` for information on platform matching.
class Gem::Platform
  def self.new(arch); end

  # Is `other` equal to this platform?  Two platforms are equal if they have the
  # same CPU, OS and version.
  #
  # Also aliased as:
  # [`eql?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Platform.html#method-i-eql-3F),
  # [`eql?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Platform.html#method-i-eql-3F)
  def ==(other); end

  # Does `other` match this platform?  Two platforms match if they have the same
  # CPU, or either has a CPU of 'universal', they have the same OS, and they
  # have the same version, or either has no version.
  #
  # Additionally, the platform will match if the local CPU is 'arm' and the
  # other CPU starts with "arm" (for generic ARM family support).
  def ===(other); end

  # Does `other` match this platform?  If `other` is a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) it will be
  # converted to a
  # [`Gem::Platform`](https://docs.ruby-lang.org/en/2.6.0/Gem/Platform.html)
  # first. See
  # [`===`](https://docs.ruby-lang.org/en/2.6.0/Gem/Platform.html#method-i-3D-3D-3D)
  # for matching rules.
  def =~(other); end

  def cpu; end

  def cpu=(_); end

  # Alias for:
  # [`==`](https://docs.ruby-lang.org/en/2.6.0/Gem/Platform.html#method-i-3D-3D)
  def eql?(other); end

  def hash; end

  def inspect; end

  def os; end

  def os=(_); end

  def to_a; end

  def to_s; end

  def version; end

  def version=(_); end

  def self.installable?(spec); end

  def self.local; end

  def self.match(platform); end

  def self.new(arch); end
end

# A platform-specific gem that is built for the packaging Ruby's platform. This
# will be replaced with
# [`Gem::Platform::local`](https://docs.ruby-lang.org/en/2.6.0/Gem/Platform.html#method-c-local).
Gem::Platform::CURRENT = T.let(T.unsafe(nil), String)

Gem::Platform::JAVA = T.let(T.unsafe(nil), Gem::Platform)

Gem::Platform::MINGW = T.let(T.unsafe(nil), Gem::Platform)

Gem::Platform::MSWIN = T.let(T.unsafe(nil), Gem::Platform)

Gem::Platform::MSWIN64 = T.let(T.unsafe(nil), Gem::Platform)

# A pure-Ruby gem that may use
# [`Gem::Specification#extensions`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-extensions)
# to build binary files.
Gem::Platform::RUBY = T.let(T.unsafe(nil), String)

Gem::Platform::X64_MINGW = T.let(T.unsafe(nil), Gem::Platform)

# Generated when trying to lookup a gem to indicate that the gem was found, but
# that it isn't usable on the current platform.
#
# fetch and install read these and report them to the user to aid in figuring
# out why a gem couldn't be installed.
class Gem::PlatformMismatch < ::Gem::ErrorReason
  def self.new(name, version); end

  # append a platform to the list of mismatched platforms.
  #
  # Platforms are added via this instead of injected via the constructor so that
  # we can loop over a list of mismatches and just add them rather than perform
  # some kind of calculation mismatch summary before creation.
  def add_platform(platform); end

  # the name of the gem
  def name; end

  # The platforms that are mismatched
  def platforms; end

  # the version
  def version; end

  # A wordy description of the error.
  def wordy; end
end

# Signals that a remote operation cannot be conducted, probably due to not being
# connected (or just not finding host).
class Gem::RemoteError < ::Gem::Exception; end

# [`RemoteFetcher`](https://docs.ruby-lang.org/en/2.6.0/Gem/RemoteFetcher.html)
# handles the details of fetching gems and gem information from a remote source.
class Gem::RemoteFetcher
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)

  BASE64_URI_TRANSLATE = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Initialize a remote fetcher using the source
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) and possible proxy
  # information.
  #
  # `proxy`
  # *   [String]: explicit specification of proxy; overrides any environment
  #
  # ```ruby
  # variable setting
  # ```
  #
  # *   nil: respect environment variables (HTTP\_PROXY, HTTP\_PROXY\_USER,
  #
  # ```
  # HTTP_PROXY_PASS)
  # ```
  #
  # *   `:no_proxy`: ignore environment variables and \_don't\_ use a proxy
  #
  #
  # `headers`: A set of additional HTTP headers to be sent to the server when
  #
  # ```
  # fetching the gem.
  # ```
  def self.new(proxy = _, dns = _, headers = _); end

  # Downloads `uri` to `path` if necessary. If no path is given, it just passes
  # the data.
  def cache_update_path(uri, path = _, update = _); end

  def close_all; end

  def correct_for_windows_path(path); end

  # Moves the gem `spec` from `source_uri` to the cache dir unless it is already
  # there. If the source\_uri is local the gem cache dir copy is always
  # replaced.
  def download(spec, source_uri, install_dir = _); end

  # Given a name and requirement, downloads this gem into cache and returns the
  # filename. Returns nil if the gem cannot be located.
  def download_to_cache(dependency); end

  # [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html) Fetcher. Dispatched
  # by `fetch_path`. Use it instead.
  def fetch_file(uri, *_); end

  # HTTP Fetcher. Dispatched by `fetch_path`. Use it instead.
  #
  # Also aliased as:
  # [`fetch_https`](https://docs.ruby-lang.org/en/2.6.0/Gem/RemoteFetcher.html#method-i-fetch_https)
  def fetch_http(uri, last_modified = _, head = _, depth = _); end

  # Alias for:
  # [`fetch_http`](https://docs.ruby-lang.org/en/2.6.0/Gem/RemoteFetcher.html#method-i-fetch_http)
  def fetch_https(uri, last_modified = _, head = _, depth = _); end

  # Downloads `uri` and returns it as a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html).
  def fetch_path(uri, mtime = _, head = _); end

  def fetch_s3(uri, mtime = _, head = _); end

  # Returns the size of `uri` in bytes.
  def fetch_size(uri); end

  def headers; end

  def headers=(_); end

  def https?(uri); end

  # Performs a [`Net::HTTP`](https://docs.ruby-lang.org/en/2.6.0/Net/HTTP.html)
  # request of type `request_class` on `uri` returning a
  # [`Net::HTTP`](https://docs.ruby-lang.org/en/2.6.0/Net/HTTP.html) response
  # object. request maintains a table of persistent connections to reduce
  # connect overhead.
  def request(uri, request_class, last_modified = _); end

  protected

  def s3_expiration; end

  # we have our own signing code here to avoid a dependency on the aws-sdk gem
  # fortunately, a simple GET request isn't too complex to sign properly
  def sign_s3_url(uri, expiration = _); end

  # Cached
  # [`RemoteFetcher`](https://docs.ruby-lang.org/en/2.6.0/Gem/RemoteFetcher.html)
  # instance.
  def self.fetcher; end
end

# A
# [`FetchError`](https://docs.ruby-lang.org/en/2.6.0/Gem/RemoteFetcher/FetchError.html)
# exception wraps up the various possible
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) and HTTP failures that
# could happen while downloading from the internet.
class Gem::RemoteFetcher::FetchError < ::Gem::Exception
  def self.new(message, uri); end

  def to_s; end

  # The [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) which was being
  # accessed when the exception happened.
  def uri; end

  # The [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) which was being
  # accessed when the exception happened.
  def uri=(_); end
end

# A FetchError that indicates that the reason for not being able to fetch data
# was that the host could not be contacted
class Gem::RemoteFetcher::UnknownHostError < ::Gem::RemoteFetcher::FetchError; end

class Gem::RemoteInstallationCancelled < ::Gem::Exception; end

class Gem::RemoteInstallationSkipped < ::Gem::Exception; end

# Represents an error communicating via HTTP.
class Gem::RemoteSourceException < ::Gem::Exception; end

class Gem::Request
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)
  extend(::Gem::UserInteraction)
  extend(::Gem::DefaultUserInteraction)

  def self.new(uri, request_class, last_modified, pool); end

  def cert_files; end

  # Creates or an HTTP connection based on `uri`, or retrieves an existing
  # connection, using a proxy if needed.
  def connection_for(uri); end

  def fetch; end

  def perform_request(request); end

  def proxy_uri; end

  # Resets HTTP connection `connection`.
  def reset(connection); end

  def user_agent; end

  def self.configure_connection_for_https(connection, cert_files); end

  def self.create_with_proxy(uri, request_class, last_modified, proxy); end

  def self.get_cert_files; end

  # Returns a proxy [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) for
  # the given `scheme` if one is set in the environment variables.
  def self.get_proxy_from_env(scheme = _); end

  def self.proxy_uri(proxy); end

  def self.verify_certificate(store_context); end

  def self.verify_certificate_message(error_number, cert); end
end

class Gem::Request::ConnectionPools
  def self.new(proxy_uri, cert_files); end

  def close_all; end

  def pool_for(uri); end

  def self.client; end

  def self.client=(_); end
end

class Gem::Request::HTTPPool
  def self.new(http_args, cert_files, proxy_uri); end

  def cert_files; end

  def checkin(connection); end

  def checkout; end

  def close_all; end

  def proxy_uri; end
end

class Gem::Request::HTTPSPool < ::Gem::Request::HTTPPool; end

# A [`RequestSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet.html)
# groups a request to activate a set of dependencies.
#
# ```ruby
# nokogiri = Gem::Dependency.new 'nokogiri', '~> 1.6'
# pg = Gem::Dependency.new 'pg', '~> 0.14'
#
# set = Gem::RequestSet.new nokogiri, pg
#
# requests = set.resolve
#
# p requests.map { |r| r.full_name }
# #=> ["nokogiri-1.6.0", "mini_portile-0.5.1", "pg-0.17.0"]
# ```
class Gem::RequestSet
  include(::TSort)

  # Creates a
  # [`RequestSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet.html) for
  # a list of
  # [`Gem::Dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Dependency.html)
  # objects, `deps`. You can then
  # [`resolve`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet.html#method-i-resolve)
  # and
  # [`install`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet.html#method-i-install)
  # the resolved list of dependencies.
  #
  # ```ruby
  # nokogiri = Gem::Dependency.new 'nokogiri', '~> 1.6'
  # pg = Gem::Dependency.new 'pg', '~> 0.14'
  #
  # set = Gem::RequestSet.new nokogiri, pg
  # ```
  def self.new(*deps); end

  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of gems to install
  # even if already installed
  def always_install; end

  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of gems to install
  # even if already installed
  def always_install=(_); end

  def dependencies; end

  def development; end

  def development=(_); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) to true if you want to
  # install only direct development dependencies.
  def development_shallow; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) to true if you want to
  # install only direct development dependencies.
  def development_shallow=(_); end

  # Errors fetching gems during resolution.
  def errors; end

  # Declare that a gem of name `name` with `reqs` requirements is needed.
  def gem(name, *reqs); end

  def git_set; end

  # When true, dependency resolution is not performed, only the requested gems
  # are installed.
  def ignore_dependencies; end

  # When true, dependency resolution is not performed, only the requested gems
  # are installed.
  def ignore_dependencies=(_); end

  # Add `deps`
  # [`Gem::Dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Dependency.html)
  # objects to the set.
  def import(deps); end

  # Installs gems for this
  # [`RequestSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet.html)
  # using the
  # [`Gem::Installer`](https://docs.ruby-lang.org/en/2.6.0/Gem/Installer.html)
  # `options`.
  #
  # If a `block` is given an activation `request` and `installer` are yielded.
  # The `installer` will be `nil` if a gem matching the request was already
  # installed.
  def install(options, &block); end

  def install_dir; end

  # Installs from the gem dependencies files in the `:gemdeps` option in
  # `options`, yielding to the `block` as in
  # [`install`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet.html#method-i-install).
  #
  # If `:without_groups` is given in the `options`, those groups in the gem
  # dependencies file are not used. See
  # [`Gem::Installer`](https://docs.ruby-lang.org/en/2.6.0/Gem/Installer.html)
  # for other `options`.
  def install_from_gemdeps(options, &block); end

  # Call hooks on installed gems
  def install_hooks(requests, options); end

  def install_into(dir, force = _, options = _); end

  # Load a dependency management file.
  def load_gemdeps(path, without_groups = _, installing = _); end

  # If true, allow dependencies to match prerelease gems.
  def prerelease; end

  # If true, allow dependencies to match prerelease gems.
  def prerelease=(_); end

  def pretty_print(q); end

  # When false no remote sets are used for resolving gems.
  def remote; end

  # When false no remote sets are used for resolving gems.
  def remote=(_); end

  # Resolve the requested dependencies and return an
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of Specification
  # objects to be activated.
  def resolve(set = _); end

  # Resolve the requested dependencies against the gems available via
  # [`Gem.path`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-path) and
  # return an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # Specification objects to be activated.
  def resolve_current; end

  def resolver; end

  def sets; end

  # Treat missing dependencies as silent errors
  def soft_missing; end

  # Treat missing dependencies as silent errors
  def soft_missing=(_); end

  def sorted_requests; end

  # The set of source gems imported via load\_gemdeps.
  def source_set; end

  def specs; end

  def specs_in(dir); end

  def tsort_each_child(node); end

  def tsort_each_node(&block); end

  def vendor_set; end
end

# A semi-compatible DSL for the
# [`Bundler`](https://docs.ruby-lang.org/en/2.6.0/Bundler.html)
# [Gemfile](https://docs.ruby-lang.org/en/2.6.0/2_6_0/bundler/templates/Gemfile.html)
# and Isolate gem dependencies files.
#
# To work with both the
# [`Bundler`](https://docs.ruby-lang.org/en/2.6.0/Bundler.html)
# [Gemfile](https://docs.ruby-lang.org/en/2.6.0/2_6_0/bundler/templates/Gemfile.html)
# and Isolate formats this implementation takes some liberties to allow
# compatibility with each, most notably in
# [`source`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet/GemDependencyAPI.html#method-i-source).
#
# A basic gem dependencies file will look like the following:
#
# ```
# source 'https://rubygems.org'
#
# gem 'rails', '3.2.14a
# gem 'devise', '~> 2.1', '>= 2.1.3'
# gem 'cancan'
# gem 'airbrake'
# gem 'pg'
# ```
#
# RubyGems recommends saving this as gem.deps.rb over
# [Gemfile](https://docs.ruby-lang.org/en/2.6.0/2_6_0/bundler/templates/Gemfile.html)
# or Isolate.
#
# To install the gems in this
# [Gemfile](https://docs.ruby-lang.org/en/2.6.0/2_6_0/bundler/templates/Gemfile.html)
# use `gem install -g` to install it and create a lockfile. The lockfile will
# ensure that when you make changes to your gem dependencies file a minimum
# amount of change is made to the dependencies of your gems.
#
# RubyGems can activate all the gems in your dependencies file at startup using
# the RUBYGEMS\_GEMDEPS environment variable or through
# [`Gem.use_gemdeps`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-use_gemdeps).
# See
# [`Gem.use_gemdeps`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-use_gemdeps)
# for details and warnings.
#
# See `gem help install` and `gem help gem\_dependencies` for further details.
class Gem::RequestSet::GemDependencyAPI
  ENGINE_MAP = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  PLATFORM_MAP = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  VERSION_MAP = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  WINDOWS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Creates a new
  # [`GemDependencyAPI`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet/GemDependencyAPI.html)
  # that will add dependencies to the
  # [`Gem::RequestSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet.html)
  # `set` based on the dependency API description in `path`.
  def self.new(set, path); end

  # The gems required by
  # [`gem`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet/GemDependencyAPI.html#method-i-gem)
  # statements in the gem.deps.rb file
  def dependencies; end

  def find_gemspec(name, path); end

  # Specifies a gem dependency with the given `name` and `requirements`. You may
  # also supply `options` following the `requirements`
  #
  # `options` include:
  #
  # require:
  # :   RubyGems does not provide any autorequire features so requires in a gem
  #     dependencies file are recorded but ignored.
  #
  #     In bundler the require: option overrides the file to require during
  #     [`Bundler.require`](https://docs.ruby-lang.org/en/2.6.0/Bundler.html#method-c-require).
  #     By default the name of the dependency is required in
  #     [`Bundler`](https://docs.ruby-lang.org/en/2.6.0/Bundler.html). A single
  #     file or an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  #     files may be given.
  #
  #     To disable requiring any file give `false`:
  #
  # ```ruby
  # gem 'rake', require: false
  # ```
  #
  # group:
  # :   Place the dependencies in the given dependency group. A single group or
  #     an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of groups
  #     may be given.
  #
  #     See also
  #     [`group`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet/GemDependencyAPI.html#method-i-group)
  #
  # platform:
  # :   Only install the dependency on the given platform. A single platform or
  #     an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  #     platforms may be given.
  #
  #     See
  #     [`platform`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet/GemDependencyAPI.html#method-i-platform)
  #     for a list of platforms available.
  #
  # path:
  # :   Install this dependency from an unpacked gem in the given directory.
  #
  # ```ruby
  # gem 'modified_gem', path: 'vendor/modified_gem'
  # ```
  #
  # git:
  # :   Install this dependency from a git repository:
  #
  # ```
  # gem 'private_gem', git: git@my.company.example:private_gem.git'
  # ```
  #
  # gist:
  # :   Install this dependency from the gist ID:
  #
  # ```ruby
  # gem 'bang', gist: '1232884'
  # ```
  #
  # github:
  # :   Install this dependency from a github git repository:
  #
  # ```ruby
  # gem 'private_gem', github: 'my_company/private_gem'
  # ```
  #
  # submodules:
  # :   [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) to `true` to
  #     include submodules when fetching the git repository for git:, gist: and
  #     github: dependencies.
  #
  # ref:
  # :   Use the given commit name or SHA for git:, gist: and github:
  #     dependencies.
  #
  # branch:
  # :   Use the given branch for git:, gist: and github: dependencies.
  #
  # tag:
  # :   Use the given tag for git:, gist: and github: dependencies.
  def gem(name, *requirements); end

  def gem_deps_file; end

  def gem_git_reference(options); end

  # Loads dependencies from a gemspec file.
  #
  # `options` include:
  #
  # name:
  # :   The name portion of the gemspec file. Defaults to searching for any
  #     gemspec file in the current directory.
  #
  # ```ruby
  # gemspec name: 'my_gem'
  # ```
  #
  # path:
  # :   The path the gemspec lives in. Defaults to the current directory:
  #
  # ```ruby
  # gemspec 'my_gem', path: 'gemspecs', name: 'my_gem'
  # ```
  #
  # development\_group:
  # :   The group to add development dependencies to. By default this is
  #     :development. Only one group may be specified.
  def gemspec(options = _); end

  # Block form for specifying gems from a git `repository`.
  #
  # ```ruby
  # git 'https://github.com/rails/rails.git' do
  #   gem 'activesupport'
  #   gem 'activerecord'
  # end
  # ```
  def git(repository); end

  def git_set; end

  # Defines a custom git source that uses `name` to expand git repositories for
  # use in gems built from git repositories. You must provide a block that
  # accepts a git repository name for expansion.
  def git_source(name, &callback); end

  # Block form for placing a dependency in the given `groups`.
  #
  # ```ruby
  # group :development do
  #   gem 'debugger'
  # end
  #
  # group :development, :test do
  #   gem 'minitest'
  # end
  # ```
  #
  # Groups can be excluded at install time using `gem install -g --without
  # development`. See `gem help install` and `gem help gem\_dependencies` for
  # further details.
  def group(*groups); end

  def installing=(installing); end

  # Loads the gem dependency file and returns self.
  def load; end

  # Block form for restricting gems to a set of platforms.
  #
  # The gem dependencies platform is different from
  # [`Gem::Platform`](https://docs.ruby-lang.org/en/2.6.0/Gem/Platform.html). A
  # platform gem.deps.rb platform matches on the ruby engine, the ruby version
  # and whether or not windows is allowed.
  #
  # :ruby, :ruby\_XY
  # :   Matches non-windows, non-jruby implementations where X and Y can be used
  #     to match releases in the 1.8, 1.9, 2.0 or 2.1 series.
  #
  # :mri, :mri\_XY
  # :   Matches non-windows C Ruby (Matz Ruby) or only the 1.8, 1.9, 2.0 or 2.1
  #     series.
  #
  # :mingw, :mingw\_XY
  # :   Matches 32 bit C Ruby on MinGW or only the 1.8, 1.9, 2.0 or 2.1 series.
  #
  # :x64\_mingw, :x64\_mingw\_XY
  # :   Matches 64 bit C Ruby on MinGW or only the 1.8, 1.9, 2.0 or 2.1 series.
  #
  # :mswin, :mswin\_XY
  # :   Matches 32 bit C Ruby on Microsoft Windows or only the 1.8, 1.9, 2.0 or
  #     2.1 series.
  #
  # :mswin64, :mswin64\_XY
  # :   Matches 64 bit C Ruby on Microsoft Windows or only the 1.8, 1.9, 2.0 or
  #     2.1 series.
  #
  # :jruby, :jruby\_XY
  # :   Matches [`JRuby`](https://docs.ruby-lang.org/en/2.6.0/JRuby.html) or
  #     [`JRuby`](https://docs.ruby-lang.org/en/2.6.0/JRuby.html) in 1.8 or 1.9
  #     mode.
  #
  # :maglev
  # :   Matches Maglev
  #
  # :rbx
  # :   Matches non-windows Rubinius
  #
  #
  # NOTE:  There is inconsistency in what environment a platform matches. You
  # may need to read the source to know the exact details.
  #
  # Also aliased as:
  # [`platforms`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet/GemDependencyAPI.html#method-i-platforms)
  def platform(*platforms); end

  # Block form for restricting gems to a particular set of platforms. See
  # [`platform`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet/GemDependencyAPI.html#method-i-platform).
  #
  # Alias for:
  # [`platform`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet/GemDependencyAPI.html#method-i-platform)
  def platforms(*platforms); end

  # A [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) containing gem
  # names and files to require from those gems.
  def requires; end

  # Restricts this gem dependencies file to the given ruby `version`.
  #
  # You may also provide `engine:` and `engine_version:` options to restrict
  # this gem dependencies file to a particular ruby engine and its engine
  # version. This matching is performed by using the RUBY\_ENGINE and
  # engine\_specific VERSION constants. (For
  # [`JRuby`](https://docs.ruby-lang.org/en/2.6.0/JRuby.html), JRUBY\_VERSION).
  def ruby(version, options = _); end

  # Sets `url` as a source for gems for this dependency API. RubyGems uses the
  # default configured sources if no source was given. If a source is set only
  # that source is used.
  #
  # This method differs in behavior from Bundler:
  #
  # *   The `:gemcutter`, # `:rubygems` and `:rubyforge` sources are not
  #     supported as they are deprecated in bundler.
  # *   The `prepend:` option is not supported. If you wish to order sources
  #     then list them in your preferred order.
  def source(url); end

  def vendor_set; end

  def without_groups; end

  def without_groups=(_); end
end

# Parses a gem.deps.rb.lock file and constructs a LockSet containing the
# dependencies found inside. If the lock file is missing no LockSet is
# constructed.
class Gem::RequestSet::Lockfile
  def self.new(request_set, gem_deps_file, dependencies); end

  def add_DEPENDENCIES(out); end

  def add_GEM(out, spec_groups); end

  def add_GIT(out, git_requests); end

  def add_PATH(out, path_requests); end

  def add_PLATFORMS(out); end

  # The platforms for this
  # [`Lockfile`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet/Lockfile.html)
  def platforms; end

  def relative_path_from(dest, base); end

  def spec_groups; end

  # The contents of the lock file.
  def to_s; end

  # Writes the lock file alongside the gem dependencies file
  def write; end

  # Creates a new
  # [`Lockfile`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet/Lockfile.html)
  # for the given `request_set` and `gem_deps_file` location.
  def self.build(request_set, gem_deps_file, dependencies = _); end

  def self.requests_to_deps(requests); end
end

# Raised when a lockfile cannot be parsed
class Gem::RequestSet::Lockfile::ParseError < ::Gem::Exception
  # Raises a
  # [`ParseError`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet/Lockfile/ParseError.html)
  # with the given `message` which was encountered at a `line` and `column`
  # while parsing.
  def self.new(message, column, line, path); end

  # The column where the error was encountered
  def column; end

  # The line where the error was encountered
  def line; end

  # The location of the lock file
  def path; end
end

class Gem::RequestSet::Lockfile::Parser
  # Parses lockfiles
  def self.new(tokenizer, set, platforms, filename = _); end

  def get(expected_types = _, expected_value = _); end

  def parse; end

  def parse_DEPENDENCIES; end

  def parse_GEM; end

  def parse_GIT; end

  def parse_PATH; end

  def parse_PLATFORMS; end

  def parse_dependency(name, op); end
end

class Gem::RequestSet::Lockfile::Tokenizer
  EOF = T.let(T.unsafe(nil), Gem::RequestSet::Lockfile::Tokenizer::Token)

  def self.new(input, filename = _, line = _, pos = _); end

  def empty?; end

  def make_parser(set, platforms); end

  # Also aliased as:
  # [`shift`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet/Lockfile/Tokenizer.html#method-i-shift)
  def next_token; end

  def peek; end

  # Alias for:
  # [`next_token`](https://docs.ruby-lang.org/en/2.6.0/Gem/RequestSet/Lockfile/Tokenizer.html#method-i-next_token)
  def shift; end

  def skip(type); end

  def to_a; end

  def token_pos(byte_offset); end

  def unshift(token); end

  def self.from_file(file); end
end

class Gem::RequestSet::Lockfile::Tokenizer::Token < ::Struct
  Elem = type_member(:out, fixed: T.untyped)

  def column; end

  def column=(_); end

  def line; end

  def line=(_); end

  def type; end

  def type=(_); end

  def value; end

  def value=(_); end

  def self.[](*_); end

  def self.inspect; end

  def self.members; end

  def self.new(*_); end
end

# A [`Requirement`](https://docs.ruby-lang.org/en/2.6.0/Gem/Requirement.html) is
# a set of one or more version restrictions. It supports a few (`=, !=, >, <,
# >=, <=, ~>`) different restriction operators.
#
# See [`Gem::Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) for
# a description on how versions and requirements work together in RubyGems.
class Gem::Requirement
  # The default requirement matches any version
  DefaultRequirement = T.let(T.unsafe(nil), T::Array[T.untyped])

  OPS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # A regular expression that matches a requirement
  PATTERN = T.let(T.unsafe(nil), Regexp)

  PATTERN_RAW = T.let(T.unsafe(nil), String)

  SOURCE_SET_REQUIREMENT = T.let(T.unsafe(nil), T.untyped)

  # Constructs a requirement from `requirements`. Requirements can be Strings,
  # Gem::Versions, or Arrays of those. `nil` and duplicate requirements are
  # ignored. An empty set of `requirements` is the same as `">= 0"`.
  def self.new(*requirements); end

  def ==(other); end

  # Alias for:
  # [`satisfied_by?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Requirement.html#method-i-satisfied_by-3F)
  def ===(version); end

  # Alias for:
  # [`satisfied_by?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Requirement.html#method-i-satisfied_by-3F)
  def =~(version); end

  def as_list; end

  # Concatenates the `new` requirements onto this requirement.
  def concat(new); end

  def encode_with(coder); end

  # Backport of performance enhancement added to RubyGems 2.2
  def exact?; end

  def for_lockfile; end

  def hash; end

  def init_with(coder); end

  def marshal_dump; end

  def marshal_load(array); end

  # Backport of performance enhancement added to RubyGems 1.4
  def none?; end

  # A requirement is a prerelease if any of the versions inside of it are
  # prereleases
  def prerelease?; end

  def pretty_print(q); end

  def requirements; end

  # True if `version` satisfies this
  # [`Requirement`](https://docs.ruby-lang.org/en/2.6.0/Gem/Requirement.html).
  #
  # Also aliased as:
  # [`===`](https://docs.ruby-lang.org/en/2.6.0/Gem/Requirement.html#method-i-3D-3D-3D),
  # [`=~`](https://docs.ruby-lang.org/en/2.6.0/Gem/Requirement.html#method-i-3D~)
  def satisfied_by?(version); end

  # True if the requirement will not always match the latest version.
  def specific?; end

  def to_s; end

  def to_yaml_properties; end

  def yaml_initialize(tag, vals); end

  protected

  def _tilde_requirements; end

  # Factory method to create a
  # [`Gem::Requirement`](https://docs.ruby-lang.org/en/2.6.0/Gem/Requirement.html)
  # object. Input may be a Version, a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), or nil.
  # Intended to simplify client code.
  #
  # If the input is "weird", the default version requirement is returned.
  def self.create(*inputs); end

  # A default "version requirement" can surely *only* be '>= 0'.
  def self.default; end

  # Parse `obj`, returning an `[op, version]` pair. `obj` can be a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) or a
  # [`Gem::Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html).
  #
  # If `obj` is a [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html),
  # it can be either a full requirement specification, like `">= 1.2"`, or a
  # simple version number, like `"1.2"`.
  #
  # ```ruby
  # parse("> 1.0")                 # => [">", Gem::Version.new("1.0")]
  # parse("1.0")                   # => ["=", Gem::Version.new("1.0")]
  # parse(Gem::Version.new("1.0")) # => ["=,  Gem::Version.new("1.0")]
  # ```
  def self.parse(obj); end

  def self.source_set; end
end

# Raised when a bad requirement is encountered
class Gem::Requirement::BadRequirementError < ::ArgumentError; end

# Given a set of
# [`Gem::Dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Dependency.html)
# objects as `needed` and a way to query the set of available specs via `set`,
# calculates a set of ActivationRequest objects which indicate all the specs
# that should be activated to meet the all the requirements.
class Gem::Resolver
  include(::Gem::Resolver::Molinillo::SpecificationProvider)
  include(::Gem::Resolver::Molinillo::UI)

  DependencyConflict = Gem::Resolver::Conflict

  # Create [`Resolver`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver.html)
  # object which will resolve the tree starting with `needed` Dependency
  # objects.
  #
  # `set` is an object that provides where to look for specifications to satisfy
  # the Dependencies. This defaults to IndexSet, which will query rubygems.org.
  def self.new(needed, set = _); end

  def activation_request(dep, possible); end

  def allow_missing?(dependency); end

  def debug?; end

  def dependencies_for(specification); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) to true if all
  # development dependencies should be considered.
  def development; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) to true if all
  # development dependencies should be considered.
  def development=(_); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) to true if immediate
  # development dependencies should be considered.
  def development_shallow; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) to true if immediate
  # development dependencies should be considered.
  def development_shallow=(_); end

  def explain(stage, *data); end

  def explain_list(stage); end

  def find_possible(dependency); end

  # When true, no dependencies are looked up for requested gems.
  def ignore_dependencies; end

  # When true, no dependencies are looked up for requested gems.
  def ignore_dependencies=(_); end

  # List of dependencies that could not be found in the configured sources.
  def missing; end

  def name_for(dependency); end

  def output; end

  def requests(s, act, reqs = _); end

  def requirement_satisfied_by?(requirement, activated, spec); end

  # Proceed with resolution! Returns an array of ActivationRequest objects.
  def resolve; end

  def search_for(dependency); end

  def select_local_platforms(specs); end

  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) of gems to skip
  # resolution. Keyed by gem name, with arrays of gem specifications as values.
  def skip_gems; end

  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) of gems to skip
  # resolution. Keyed by gem name, with arrays of gem specifications as values.
  def skip_gems=(_); end

  # When a missing dependency, don't stop. Just go on and record what was
  # missing.
  def soft_missing; end

  # When a missing dependency, don't stop. Just go on and record what was
  # missing.
  def soft_missing=(_); end

  def sort_dependencies(dependencies, activated, conflicts); end

  def stats; end

  # Combines `sets` into a ComposedSet that allows specification lookup in a
  # uniform manner. If one of the `sets` is itself a ComposedSet its sets are
  # flattened into the result ComposedSet.
  def self.compose_sets(*sets); end

  # Creates a
  # [`Resolver`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver.html) that
  # queries only against the already installed gems for the `needed`
  # dependencies.
  def self.for_current_gems(needed); end
end

# The global rubygems pool, available via the rubygems.org API. Returns
# instances of APISpecification.
class Gem::Resolver::APISet < ::Gem::Resolver::Set
  # Creates a new
  # [`APISet`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/APISet.html)
  # that will retrieve gems from `uri` using the RubyGems API URL `dep_uri`
  # which is described at http://guides.rubygems.org/rubygems-org-api
  def self.new(dep_uri = _); end

  def dep_uri; end

  # Return an array of APISpecification objects matching DependencyRequest
  # `req`.
  def find_all(req); end

  # A hint run by the resolver to allow the
  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) to fetch data for
  # DependencyRequests `reqs`.
  def prefetch(reqs); end

  def prefetch_now; end

  def pretty_print(q); end

  # The [`Gem::Source`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source.html)
  # that gems are fetched from
  def source; end

  # The corresponding place to fetch gems.
  def uri; end

  def versions(name); end
end

# Represents a specification retrieved via the rubygems.org API.
#
# This is used to avoid loading the full Specification object when all we need
# is the name, version, and dependencies.
class Gem::Resolver::APISpecification < ::Gem::Resolver::Specification
  # Creates an
  # [`APISpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/APISpecification.html)
  # for the given `set` from the rubygems.org `api_data`.
  #
  # See http://guides.rubygems.org/rubygems-org-api/#misc\_methods for the
  # format of the `api_data`.
  def self.new(set, api_data); end

  def ==(other); end

  def fetch_development_dependencies; end

  def installable_platform?; end

  def pretty_print(q); end

  def source; end

  def spec; end
end

# Specifies a Specification object that should be activated. Also contains a
# dependency that was used to introduce this activation.
class Gem::Resolver::ActivationRequest
  # Creates a new
  # [`ActivationRequest`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/ActivationRequest.html)
  # that will activate `spec`. The parent `request` is used to provide
  # diagnostics in case of conflicts.
  #
  # `others_possible` indicates that other specifications may also match this
  # activation request.
  def self.new(spec, request, others_possible = _); end

  def ==(other); end

  # Is this activation request for a development dependency?
  def development?; end

  # Downloads a gem at `path` and returns the file path.
  def download(path); end

  # The full name of the specification to be activated.
  #
  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/ActivationRequest.html#method-i-to_s)
  def full_name; end

  # The
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # for this activation request.
  def full_spec; end

  def inspect; end

  # True if the requested gem has already been installed.
  def installed?; end

  # The name of this activation request's specification
  def name; end

  # Indicate if this activation is one of a set of possible requests for the
  # same Dependency request.
  def others_possible?; end

  # Return the
  # [`ActivationRequest`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/ActivationRequest.html)
  # that contained the dependency that we were activated for.
  def parent; end

  def pretty_print(q); end

  # The parent request for this activation request.
  def request; end

  # The specification to be activated.
  def spec; end

  # Alias for:
  # [`full_name`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/ActivationRequest.html#method-i-full_name)
  def to_s; end

  # The version of this activation request's specification
  def version; end
end

# The [`BestSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/BestSet.html)
# chooses the best available method to query a remote index.
#
# It combines IndexSet and APISet
class Gem::Resolver::BestSet < ::Gem::Resolver::ComposedSet
  # Creates a
  # [`BestSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/BestSet.html)
  # for the given `sources` or
  # [`Gem::sources`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-sources)
  # if none are specified. `sources` must be a
  # [`Gem::SourceList`](https://docs.ruby-lang.org/en/2.6.0/Gem/SourceList.html).
  def self.new(sources = _); end

  def find_all(req); end

  def pick_sets; end

  def prefetch(reqs); end

  def pretty_print(q); end

  def replace_failed_api_set(error); end
end

# A
# [`ComposedSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/ComposedSet.html)
# allows multiple sets to be queried like a single set.
#
# To create a composed set with any number of sets use:
#
# ```ruby
# Gem::Resolver.compose_sets set1, set2
# ```
#
# This method will eliminate nesting of composed sets.
class Gem::Resolver::ComposedSet < ::Gem::Resolver::Set
  # Creates a new
  # [`ComposedSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/ComposedSet.html)
  # containing `sets`. Use
  # [`Gem::Resolver::compose_sets`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver.html#method-c-compose_sets)
  # instead.
  def self.new(*sets); end

  def errors; end

  # Finds all specs matching `req` in all sets.
  def find_all(req); end

  # Prefetches `reqs` in all sets.
  def prefetch(reqs); end

  # When `allow_prerelease` is set to `true` prereleases gems are allowed to
  # match dependencies.
  def prerelease=(allow_prerelease); end

  # Sets the remote network access for all composed sets.
  def remote=(remote); end

  def sets; end
end

# Used internally to indicate that a dependency conflicted with a spec that
# would be activated.
class Gem::Resolver::Conflict
  # Creates a new resolver conflict when `dependency` is in conflict with an
  # already `activated` specification.
  def self.new(dependency, activated, failed_dep = _); end

  def ==(other); end

  # The specification that was activated prior to the conflict
  def activated; end

  # Return the 2 dependency objects that conflicted
  def conflicting_dependencies; end

  # The dependency that is in conflict with the activated gem.
  def dependency; end

  # A string explanation of the conflict.
  def explain; end

  # Explanation of the conflict used by exceptions to print useful messages
  def explanation; end

  def failed_dep; end

  # Returns true if the conflicting dependency's name matches `spec`.
  def for_spec?(spec); end

  def pretty_print(q); end

  # Path of activations from the `current` list.
  def request_path(current); end

  # Return the Specification that listed the dependency
  def requester; end
end

# A set which represents the installed gems. Respects all the normal settings
# that control where to look for installed gems.
class Gem::Resolver::CurrentSet < ::Gem::Resolver::Set
  def find_all(req); end
end

# Used Internally. Wraps a Dependency object to also track which spec contained
# the Dependency.
class Gem::Resolver::DependencyRequest
  # Creates a new
  # [`DependencyRequest`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/DependencyRequest.html)
  # for `dependency` from `requester`. +requester may be nil if the request came
  # from a user.
  def self.new(dependency, requester); end

  def ==(other); end

  # The wrapped
  # [`Gem::Dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Dependency.html)
  def dependency; end

  # Is this dependency a development dependency?
  def development?; end

  # Indicate that the request is for a gem explicitly requested by the user
  def explicit?; end

  # Indicate that the request is for a gem requested as a dependency of another
  # gem
  def implicit?; end

  # Does this dependency request match `spec`?
  #
  # NOTE:
  # [`match?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/DependencyRequest.html#method-i-match-3F)
  # only matches prerelease versions when
  # [`dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/DependencyRequest.html#attribute-i-dependency)
  # is a prerelease dependency.
  def match?(spec, allow_prerelease = _); end

  # Does this dependency request match `spec`?
  #
  # NOTE:
  # [`matches_spec?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/DependencyRequest.html#method-i-matches_spec-3F)
  # matches prerelease versions. See also
  # [`match?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/DependencyRequest.html#method-i-match-3F)
  def matches_spec?(spec); end

  # The name of the gem this dependency request is requesting.
  def name; end

  def pretty_print(q); end

  # Return a [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html)
  # indicating who caused this request to be added (only valid for implicit
  # requests)
  def request_context; end

  # The request for this dependency.
  def requester; end

  # The version requirement for this dependency request
  def requirement; end

  def to_s; end

  def type; end
end

# A [`GitSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/GitSet.html)
# represents gems that are sourced from git repositories.
#
# This is used for gem dependency file support.
#
# Example:
#
# ```ruby
# set = Gem::Resolver::GitSet.new
# set.add_git_gem 'rake', 'git://example/rake.git', tag: 'rake-10.1.0'
# ```
class Gem::Resolver::GitSet < ::Gem::Resolver::Set
  def self.new; end

  def add_git_gem(name, repository, reference, submodules); end

  def add_git_spec(name, version, repository, reference, submodules); end

  # Finds all git gems matching `req`
  def find_all(req); end

  def need_submodules; end

  # Prefetches specifications from the git repositories in this set.
  def prefetch(reqs); end

  def pretty_print(q); end

  def repositories; end

  # The root directory for git gems in this set. This is usually
  # [`Gem.dir`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-dir), the
  # installation directory for regular gems.
  def root_dir; end

  # The root directory for git gems in this set. This is usually
  # [`Gem.dir`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-dir), the
  # installation directory for regular gems.
  def root_dir=(_); end

  def specs; end
end

# A
# [`GitSpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/GitSpecification.html)
# represents a gem that is sourced from a git repository and is being loaded
# through a gem dependencies file through the `git:` option.
class Gem::Resolver::GitSpecification < ::Gem::Resolver::SpecSpecification
  def ==(other); end

  def add_dependency(dependency); end

  # Installing a git gem only involves building the extensions and generating
  # the executables.
  def install(options = _); end

  def pretty_print(q); end
end

# The global rubygems pool represented via the traditional source index.
class Gem::Resolver::IndexSet < ::Gem::Resolver::Set
  def self.new(source = _); end

  # Return an array of IndexSpecification objects matching DependencyRequest
  # `req`.
  def find_all(req); end

  def pretty_print(q); end
end

# Represents a possible Specification object returned from IndexSet. Used to
# delay needed to download full Specification objects when only the `name` and
# `version` are needed.
class Gem::Resolver::IndexSpecification < ::Gem::Resolver::Specification
  # An
  # [`IndexSpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/IndexSpecification.html)
  # is created from the index format described in `gem help generate\_index`.
  #
  # The `set` contains other specifications for this (URL) `source`.
  #
  # The `name`, `version` and `platform` are the name, version and platform of
  # the gem.
  def self.new(set, name, version, source, platform); end

  # The dependencies of the gem for this specification
  def dependencies; end

  def inspect; end

  def pretty_print(q); end

  def spec; end
end

# An
# [`InstalledSpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/InstalledSpecification.html)
# represents a gem that is already installed locally.
class Gem::Resolver::InstalledSpecification < ::Gem::Resolver::SpecSpecification
  def ==(other); end

  # This is a null install as this specification is already installed. `options`
  # are ignored.
  def install(options = _); end

  # Returns `true` if this gem is installable for the current platform.
  def installable_platform?; end

  def pretty_print(q); end

  # The source for this specification
  def source; end
end

# A set of gems for installation sourced from remote sources and local .gem
# files
class Gem::Resolver::InstallerSet < ::Gem::Resolver::Set
  # Creates a new
  # [`InstallerSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/InstallerSet.html)
  # that will look for gems in `domain`.
  def self.new(domain); end

  # Looks up the latest specification for `dependency` and adds it to the
  # always\_install list.
  def add_always_install(dependency); end

  # Adds a local gem requested using `dep_name` with the given `spec` that can
  # be loaded and installed using the `source`.
  def add_local(dep_name, spec, source); end

  def always_install; end

  def consider_local?; end

  def consider_remote?; end

  # Errors encountered while resolving gems
  def errors; end

  # Returns an array of IndexSpecification objects matching DependencyRequest
  # `req`.
  def find_all(req); end

  def ignore_dependencies; end

  def ignore_dependencies=(_); end

  def ignore_installed; end

  def ignore_installed=(_); end

  def inspect; end

  def load_spec(name, ver, platform, source); end

  def local?(dep_name); end

  def prefetch(reqs); end

  def prerelease=(allow_prerelease); end

  def pretty_print(q); end

  def remote=(remote); end

  def remote_set; end
end

# A
# [`LocalSpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/LocalSpecification.html)
# comes from a .gem file on the local filesystem.
class Gem::Resolver::LocalSpecification < ::Gem::Resolver::SpecSpecification
  # Returns `true` if this gem is installable for the current platform.
  def installable_platform?; end

  def local?; end

  def pretty_print(q); end
end

# A set of gems from a gem dependencies lockfile.
class Gem::Resolver::LockSet < ::Gem::Resolver::Set
  # Creates a new
  # [`LockSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/LockSet.html)
  # from the given `sources`
  def self.new(sources); end

  def add(name, version, platform); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # IndexSpecification objects matching the DependencyRequest `req`.
  def find_all(req); end

  def load_spec(name, version, platform, source); end

  def pretty_print(q); end

  def specs; end
end

# The
# [`LockSpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/LockSpecification.html)
# comes from a lockfile (Gem::RequestSet::Lockfile).
#
# A LockSpecification's dependency information is pre-filled from the lockfile.
class Gem::Resolver::LockSpecification < ::Gem::Resolver::Specification
  def self.new(set, name, version, sources, platform); end

  def add_dependency(dependency); end

  # This is a null install as a locked specification is considered installed.
  # `options` are ignored.
  def install(options = _); end

  def pretty_print(q); end

  def sources; end

  # A specification constructed from the lockfile is returned
  def spec; end
end

# [`Gem::Resolver::Molinillo`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo.html)
# is a generic dependency resolution algorithm.
module Gem::Resolver::Molinillo
  # The version of
  # [`Gem::Resolver::Molinillo`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo.html).
  VERSION = T.let(T.unsafe(nil), String)
end

# An error caused by attempting to fulfil a dependency that was circular
#
# @note This exception will be thrown iff a {Vertex} is added to a
#
# ```
# {DependencyGraph} that has a {DependencyGraph::Vertex#path_to?} an
# existing {DependencyGraph::Vertex}
# ```
class Gem::Resolver::Molinillo::CircularDependencyError < ::Gem::Resolver::Molinillo::ResolverError
  # Initializes a new error with the given circular vertices. @param
  # [Array<DependencyGraph::Vertex>] nodes the nodes in the dependency
  #
  # ```ruby
  # that caused the error
  # ```
  def self.new(nodes); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html)<Object>
  # :   the dependencies responsible for causing the error
  def dependencies; end
end

# @!visibility private
# @!visibility private
module Gem::Resolver::Molinillo::Delegates; end

# [`Delegates`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/Delegates.html)
# all {Gem::Resolver::Molinillo::ResolutionState} methods to a `#state`
# property.
module Gem::Resolver::Molinillo::Delegates::ResolutionState
  # (see Gem::Resolver::Molinillo::ResolutionState#activated)
  def activated; end

  # (see Gem::Resolver::Molinillo::ResolutionState#conflicts)
  def conflicts; end

  # (see Gem::Resolver::Molinillo::ResolutionState#depth)
  def depth; end

  # (see Gem::Resolver::Molinillo::ResolutionState#name)
  def name; end

  # (see Gem::Resolver::Molinillo::ResolutionState#possibilities)
  def possibilities; end

  # (see Gem::Resolver::Molinillo::ResolutionState#requirement)
  def requirement; end

  # (see Gem::Resolver::Molinillo::ResolutionState#requirements)
  def requirements; end
end

# [`Delegates`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/Delegates.html)
# all {Gem::Resolver::Molinillo::SpecificationProvider} methods to a
# `#specification\_provider` property.
module Gem::Resolver::Molinillo::Delegates::SpecificationProvider
  # (see
  # [`Gem::Resolver::Molinillo::SpecificationProvider#allow_missing?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/SpecificationProvider.html#method-i-allow_missing-3F))
  def allow_missing?(dependency); end

  # (see
  # [`Gem::Resolver::Molinillo::SpecificationProvider#dependencies_for`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/SpecificationProvider.html#method-i-dependencies_for))
  def dependencies_for(specification); end

  # (see
  # [`Gem::Resolver::Molinillo::SpecificationProvider#name_for`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/SpecificationProvider.html#method-i-name_for))
  def name_for(dependency); end

  # (see
  # [`Gem::Resolver::Molinillo::SpecificationProvider#name_for_explicit_dependency_source`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/SpecificationProvider.html#method-i-name_for_explicit_dependency_source))
  def name_for_explicit_dependency_source; end

  # (see
  # [`Gem::Resolver::Molinillo::SpecificationProvider#name_for_locking_dependency_source`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/SpecificationProvider.html#method-i-name_for_locking_dependency_source))
  def name_for_locking_dependency_source; end

  # (see
  # [`Gem::Resolver::Molinillo::SpecificationProvider#requirement_satisfied_by?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/SpecificationProvider.html#method-i-requirement_satisfied_by-3F))
  def requirement_satisfied_by?(requirement, activated, spec); end

  # (see
  # [`Gem::Resolver::Molinillo::SpecificationProvider#search_for`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/SpecificationProvider.html#method-i-search_for))
  def search_for(dependency); end

  # (see
  # [`Gem::Resolver::Molinillo::SpecificationProvider#sort_dependencies`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/SpecificationProvider.html#method-i-sort_dependencies))
  def sort_dependencies(dependencies, activated, conflicts); end
end

# A directed acyclic graph that is tuned to hold named dependencies
class Gem::Resolver::Molinillo::DependencyGraph
  include(::TSort)
  include(::Enumerable)

  Elem = type_member(:out)

  # Initializes an empty dependency graph
  def self.new; end

  # @return [Boolean] whether the two dependency graphs are equal, determined
  #
  # ```
  # by a recursive traversal of each {#root_vertices} and its
  # {Vertex#successors}
  # ```
  def ==(other); end

  # @param [String] name @param [Object] payload @param [Array<String>]
  # parent\_names @param [Object] requirement the requirement that is requiring
  # the child @return [void]
  def add_child_vertex(name, payload, parent_names, requirement); end

  # Adds a new {Edge} to the dependency graph @param [Vertex] origin @param
  # [Vertex] destination @param [Object] requirement the requirement that this
  # edge represents @return [Edge] the added edge
  def add_edge(origin, destination, requirement); end

  # Adds a vertex with the given name, or updates the existing one. @param
  # [String] name @param [Object] payload @return [Vertex] the vertex that was
  # added to `self`
  def add_vertex(name, payload, root = _); end

  # Deletes an {Edge} from the dependency graph @param [Edge] edge @return
  # [Void]
  def delete_edge(edge); end

  # Detaches the {#vertex\_named} `name` {Vertex} from the graph, recursively
  # removing any non-root vertices that were orphaned in the process @param
  # [String] name @return [Array<Vertex>] the vertices which have been detached
  def detach_vertex_named(name); end

  # Enumerates through the vertices of the graph. @return [Array<Vertex>] The
  # graph's vertices.
  #
  # Also aliased as:
  # [`tsort_each_node`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph.html#method-i-tsort_each_node)
  def each; end

  # @return [String] a string suitable for debugging
  def inspect; end

  # @return [Log] the op log for this graph
  def log; end

  # Rewinds the graph to the state tagged as `tag` @param  [Object] tag the tag
  # to rewind to @return [Void]
  def rewind_to(tag); end

  # @param [String] name @return [Vertex,nil] the root vertex with the given
  # name
  def root_vertex_named(name); end

  # Sets the payload of the vertex with the given name @param [String] name the
  # name of the vertex @param [Object] payload the payload @return [Void]
  def set_payload(name, payload); end

  # Tags the current state of the dependency as the given tag @param  [Object]
  # tag an opaque tag for the current state of the graph @return [Void]
  def tag(tag); end

  # @param [Hash] options options for dot output. @return [String] Returns a dot
  # format representation of the graph
  def to_dot(options = _); end

  # @!visibility private
  def tsort_each_child(vertex, &block); end

  # @!visibility private
  #
  # Alias for:
  # [`each`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph.html#method-i-each)
  def tsort_each_node; end

  # @param [String] name @return [Vertex,nil] the vertex with the given name
  def vertex_named(name); end

  # @return [{String => Vertex}] the vertices of the dependency graph, keyed
  #
  # ```
  # by {Vertex#name}
  # ```
  def vertices; end

  # Topologically sorts the given vertices. @param [Enumerable<Vertex>] vertices
  # the vertices to be sorted, which must
  #
  # ```
  # all belong to the same graph.
  # ```
  #
  # @return [Array<Vertex>] The sorted vertices.
  def self.tsort(vertices); end
end

# An action that modifies a {DependencyGraph} that is reversible. @abstract
class Gem::Resolver::Molinillo::DependencyGraph::Action
  # Reverses the action on the given graph. @param  [DependencyGraph] graph the
  # graph to reverse the action on. @return [Void]
  def down(graph); end

  # @return [Action,Nil] The next action
  def next; end

  # @return [Action,Nil] The next action
  def next=(_); end

  # @return [Action,Nil] The previous action
  def previous; end

  # @return [Action,Nil] The previous action
  def previous=(_); end

  # Performs the action on the given graph. @param  [DependencyGraph] graph the
  # graph to perform the action on. @return [Void]
  def up(graph); end

  # @return [Symbol] The name of the action.
  def self.action_name; end
end

# @!visibility private (see
# [`DependencyGraph#add_edge_no_circular`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph.html#method-i-add_edge_no_circular))
class Gem::Resolver::Molinillo::DependencyGraph::AddEdgeNoCircular < ::Gem::Resolver::Molinillo::DependencyGraph::Action
  # Initialize an action to add an edge to a dependency graph @param [String]
  # origin the name of the origin of the edge @param [String] destination the
  # name of the destination of the edge @param [Object] requirement the
  # requirement that the edge represents
  def self.new(origin, destination, requirement); end

  # @return [String] the name of the destination of the edge
  def destination; end

  # (see Action#down)
  def down(graph); end

  # @param  [DependencyGraph] graph the graph to find vertices from @return
  # [Edge] The edge this action adds
  def make_edge(graph); end

  # @return [String] the name of the origin of the edge
  def origin; end

  # @return [Object] the requirement that the edge represents
  def requirement; end

  # (see Action#up)
  def up(graph); end

  # (see Action.action\_name)
  def self.action_name; end
end

class Gem::Resolver::Molinillo::DependencyGraph::AddVertex < ::Gem::Resolver::Molinillo::DependencyGraph::Action
  def self.new(name, payload, root); end

  def down(graph); end

  def name; end

  def payload; end

  def root; end

  def up(graph); end

  def self.action_name; end
end

# @!visibility private (see
# [`DependencyGraph#delete_edge`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph.html#method-i-delete_edge))
class Gem::Resolver::Molinillo::DependencyGraph::DeleteEdge < ::Gem::Resolver::Molinillo::DependencyGraph::Action
  # Initialize an action to add an edge to a dependency graph @param [String]
  # [`origin_name`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph/DeleteEdge.html#attribute-i-origin_name)
  # the name of the origin of the edge @param [String]
  # [`destination_name`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph/DeleteEdge.html#attribute-i-destination_name)
  # the name of the destination of the edge @param [Object] requirement the
  # requirement that the edge represents
  def self.new(origin_name, destination_name, requirement); end

  # @return [String] the name of the destination of the edge
  def destination_name; end

  # (see Action#down)
  def down(graph); end

  # @param  [DependencyGraph] graph the graph to find vertices from @return
  # [Edge] The edge this action adds
  def make_edge(graph); end

  # @return [String] the name of the origin of the edge
  def origin_name; end

  # @return [Object] the requirement that the edge represents
  def requirement; end

  # (see Action#up)
  def up(graph); end

  # (see Action.action\_name)
  def self.action_name; end
end

# @!visibility private @see
# [`DependencyGraph#detach_vertex_named`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph.html#method-i-detach_vertex_named)
class Gem::Resolver::Molinillo::DependencyGraph::DetachVertexNamed < ::Gem::Resolver::Molinillo::DependencyGraph::Action
  # Initialize an action to detach a vertex from a dependency graph @param
  # [String] name the name of the vertex to detach
  def self.new(name); end

  # (see Action#down)
  def down(graph); end

  # @return [String] the name of the vertex to detach
  def name; end

  # (see Action#up)
  def up(graph); end

  # (see Action#name)
  def self.action_name; end
end

# A directed edge of a {DependencyGraph} @attr [Vertex] origin The origin of the
# directed edge @attr [Vertex] destination The destination of the directed edge
# @attr [Object] requirement The requirement the directed edge represents
class Gem::Resolver::Molinillo::DependencyGraph::Edge < ::Struct
  Elem = type_member(:out, fixed: T.untyped)

  def destination; end

  def destination=(_); end

  def origin; end

  def origin=(_); end

  def requirement; end

  def requirement=(_); end

  def self.[](*_); end

  def self.inspect; end

  def self.members; end

  def self.new(*_); end
end

# A log for dependency graph actions
class Gem::Resolver::Molinillo::DependencyGraph::Log
  extend(::Enumerable)

  Elem = type_member(:out, fixed: T.untyped)

  # Initializes an empty log
  def self.new; end

  # @macro action
  def add_edge_no_circular(graph, origin, destination, requirement); end

  # @macro action
  def add_vertex(graph, name, payload, root); end

  # {include:DependencyGraph#delete\_edge} @param [Graph] graph the graph to
  # perform the action on @param [String] origin\_name @param [String]
  # destination\_name @param [Object] requirement @return (see
  # [`DependencyGraph#delete_edge`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph.html#method-i-delete_edge))
  def delete_edge(graph, origin_name, destination_name, requirement); end

  # @macro action
  def detach_vertex_named(graph, name); end

  # @!visibility private Enumerates each action in the log @yield [Action]
  def each; end

  # Pops the most recent action from the log and undoes the action @param
  # [DependencyGraph] graph @return [Action] the action that was popped off the
  # log
  def pop!(graph); end

  # @!visibility private Enumerates each action in the log in reverse order
  # @yield [Action]
  def reverse_each; end

  # @macro action
  def rewind_to(graph, tag); end

  # @macro action
  def set_payload(graph, name, payload); end

  # @macro action
  def tag(graph, tag); end

  class << self
    Elem = type_member(:out)
  end
end

class Gem::Resolver::Molinillo::DependencyGraph::SetPayload < ::Gem::Resolver::Molinillo::DependencyGraph::Action
  def self.new(name, payload); end

  def down(graph); end

  def name; end

  def payload; end

  def up(graph); end

  def self.action_name; end
end

# @!visibility private @see
# [`DependencyGraph#tag`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph.html#method-i-tag)
class Gem::Resolver::Molinillo::DependencyGraph::Tag < ::Gem::Resolver::Molinillo::DependencyGraph::Action
  # Initialize an action to tag a state of a dependency graph @param [Object]
  # tag an opaque tag
  def self.new(tag); end

  # (see Action#down)
  def down(_graph); end

  # @return [Object] An opaque tag
  def tag; end

  # (see Action#up)
  def up(_graph); end

  # (see Action.action\_name)
  def self.action_name; end
end

# A vertex in a {DependencyGraph} that encapsulates a {#name} and a {#payload}
class Gem::Resolver::Molinillo::DependencyGraph::Vertex
  # Initializes a vertex with the given name and payload. @param [String] name
  # see {#name} @param [Object] payload see {#payload}
  def self.new(name, payload); end

  # @return [Boolean] whether the two vertices are equal, determined
  #
  # ```
  # by a recursive traversal of each {Vertex#successors}
  # ```
  #
  #
  # Also aliased as:
  # [`eql?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph/Vertex.html#method-i-eql-3F)
  def ==(other); end

  # Is there a path from `other` to `self` following edges in the dependency
  # graph? @return true iff there is a path following edges within this {#graph}
  #
  # Also aliased as:
  # [`is_reachable_from?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph/Vertex.html#method-i-is_reachable_from-3F)
  def ancestor?(other); end

  # Alias for:
  # [`path_to?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph/Vertex.html#method-i-path_to-3F)
  def descendent?(other); end

  # Alias for:
  # [`==`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph/Vertex.html#method-i-3D-3D)
  def eql?(other); end

  # @return [Array<Object>] the explicit requirements that required
  #
  # ```ruby
  # this vertex
  # ```
  def explicit_requirements; end

  # @return [Fixnum] a hash for the vertex based upon its {#name}
  def hash; end

  # @return [Array<Edge>] the edges of {#graph} that have `self` as their
  #
  # ```
  # {Edge#destination}
  # ```
  def incoming_edges; end

  # @return [Array<Edge>] the edges of {#graph} that have `self` as their
  #
  # ```
  # {Edge#destination}
  # ```
  def incoming_edges=(_); end

  # @return [String] a string suitable for debugging
  def inspect; end

  # Alias for:
  # [`ancestor?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph/Vertex.html#method-i-ancestor-3F)
  def is_reachable_from?(other); end

  # @return [String] the name of the vertex
  def name; end

  # @return [String] the name of the vertex
  def name=(_); end

  # @return [Array<Edge>] the edges of {#graph} that have `self` as their
  #
  # ```
  # {Edge#origin}
  # ```
  def outgoing_edges; end

  # @return [Array<Edge>] the edges of {#graph} that have `self` as their
  #
  # ```
  # {Edge#origin}
  # ```
  def outgoing_edges=(_); end

  # Is there a path from `self` to `other` following edges in the dependency
  # graph? @return true iff there is a path following edges within this {#graph}
  #
  # Also aliased as:
  # [`descendent?`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/DependencyGraph/Vertex.html#method-i-descendent-3F)
  def path_to?(other); end

  # @return [Object] the payload the vertex holds
  def payload; end

  # @return [Object] the payload the vertex holds
  def payload=(_); end

  # @return [Array<Vertex>] the vertices of {#graph} that have an edge with
  #
  # ```
  # `self` as their {Edge#destination}
  # ```
  def predecessors; end

  # @return [Array<Vertex>] the vertices of {#graph} where `self` is a
  #
  # ```
  # {#descendent?}
  # ```
  def recursive_predecessors; end

  # @return [Array<Vertex>] the vertices of {#graph} where `self` is an
  #
  # ```
  # {#ancestor?}
  # ```
  def recursive_successors; end

  # @return [Array<Object>] all of the requirements that required
  #
  # ```ruby
  # this vertex
  # ```
  def requirements; end

  # @return [Boolean] whether the vertex is considered a root vertex
  def root; end

  # @return [Boolean] whether the vertex is considered a root vertex
  def root=(_); end

  # @return [Boolean] whether the vertex is considered a root vertex
  def root?; end

  # @param  [Vertex] other the other vertex to compare to @return [Boolean]
  # whether the two vertices are equal, determined
  #
  # ```
  # solely by {#name} and {#payload} equality
  # ```
  def shallow_eql?(other); end

  # @return [Array<Vertex>] the vertices of {#graph} that have an edge with
  #
  # ```
  # `self` as their {Edge#origin}
  # ```
  def successors; end
end

# A state that encapsulates a set of {#requirements} with an {Array} of
# possibilities
class Gem::Resolver::Molinillo::DependencyState < ::Gem::Resolver::Molinillo::ResolutionState
  Elem = type_member(:out)

  # Removes a possibility from `self` @return [PossibilityState] a state with a
  # single possibility,
  #
  # ```ruby
  # the possibility that was removed from `self`
  # ```
  def pop_possibility_state; end
end

# An error caused by searching for a dependency that is completely unknown, i.e.
# has no versions available whatsoever.
class Gem::Resolver::Molinillo::NoSuchDependencyError < ::Gem::Resolver::Molinillo::ResolverError
  # Initializes a new error with the given missing dependency. @param [Object]
  # dependency @see {#dependency} @param [Array<Object>]
  # [`required_by`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/NoSuchDependencyError.html#attribute-i-required_by)
  # @see {#required\_by}
  def self.new(dependency, required_by = _); end

  # @return [Object] the dependency that could not be found
  def dependency; end

  # @return [Object] the dependency that could not be found
  def dependency=(_); end

  # The error message for the missing dependency, including the specifications
  # that had this dependency.
  def message; end

  # @return [Array<Object>] the specifications that depended upon {#dependency}
  def required_by; end

  # @return [Array<Object>] the specifications that depended upon {#dependency}
  def required_by=(_); end
end

# A state that encapsulates a single possibility to fulfill the given
# {#requirement}
class Gem::Resolver::Molinillo::PossibilityState < ::Gem::Resolver::Molinillo::ResolutionState
  Elem = type_member(:out, fixed: T.untyped)
end

class Gem::Resolver::Molinillo::ResolutionState < ::Struct
  Elem = type_member(:out, fixed: T.untyped)

  def activated; end

  def activated=(_); end

  def conflicts; end

  def conflicts=(_); end

  def depth; end

  def depth=(_); end

  def name; end

  def name=(_); end

  def possibilities; end

  def possibilities=(_); end

  def requirement; end

  def requirement=(_); end

  def requirements; end

  def requirements=(_); end

  def self.[](*_); end

  # Returns an empty resolution state @return [ResolutionState] an empty state
  def self.empty; end

  def self.inspect; end

  def self.members; end

  def self.new(*_); end
end

# This class encapsulates a dependency resolver. The resolver is responsible for
# determining which set of dependencies to activate, with feedback from the
# {#specification\_provider}
class Gem::Resolver::Molinillo::Resolver
  # Initializes a new resolver. @param  [SpecificationProvider]
  # [`specification_provider`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/Resolver.html#attribute-i-specification_provider)
  #
  # ```
  # see {#specification_provider}
  # ```
  #
  # @param  [UI]
  # [`resolver_ui`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/Resolver.html#attribute-i-resolver_ui)
  #
  # ```
  # see {#resolver_ui}
  # ```
  def self.new(specification_provider, resolver_ui); end

  # Resolves the requested dependencies into a {DependencyGraph}, locking to the
  # base dependency graph (if specified) @param [Array] requested an array of
  # 'requested' dependencies that the
  #
  # ```
  # {#specification_provider} can understand
  # ```
  #
  # @param [DependencyGraph,nil] base the base dependency graph to which
  #
  # ```ruby
  # dependencies should be 'locked'
  # ```
  def resolve(requested, base = _); end

  # @return [UI] the UI module used to communicate back to the user
  #
  # ```ruby
  # during the resolution process
  # ```
  def resolver_ui; end

  # @return [SpecificationProvider] the specification provider used
  #
  # ```
  # in the resolution process
  # ```
  def specification_provider; end
end

# A specific resolution from a given {Resolver}
class Gem::Resolver::Molinillo::Resolver::Resolution
  include(::Gem::Resolver::Molinillo::Delegates::SpecificationProvider)
  include(::Gem::Resolver::Molinillo::Delegates::ResolutionState)

  # Initializes a new resolution. @param [SpecificationProvider]
  # [`specification_provider`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/Resolver/Resolution.html#attribute-i-specification_provider)
  #
  # ```
  # see {#specification_provider}
  # ```
  #
  # @param [UI]
  # [`resolver_ui`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/Resolver/Resolution.html#attribute-i-resolver_ui)
  # see {#resolver\_ui} @param [Array] requested see {#original\_requested}
  # @param [DependencyGraph] base see {#base}
  def self.new(specification_provider, resolver_ui, requested, base); end

  # @return [DependencyGraph] the base dependency graph to which
  #
  # ```ruby
  # dependencies should be 'locked'
  # ```
  def base; end

  # @return [Integer] the number of resolver iterations in between calls to
  #
  # ```
  # {#resolver_ui}'s {UI#indicate_progress} method
  # ```
  def iteration_rate=(_); end

  # @return [Array] the dependencies that were explicitly required
  def original_requested; end

  # Resolves the {#original\_requested} dependencies into a full dependency
  #
  # ```ruby
  # graph
  # ```
  #
  # @raise [ResolverError] if successful resolution is impossible @return
  # [DependencyGraph] the dependency graph of successfully resolved
  #
  # ```ruby
  # dependencies
  # ```
  def resolve; end

  # @return [UI] the UI that knows how to communicate feedback about the
  #
  # ```ruby
  # resolution process back to the user
  # ```
  def resolver_ui; end

  # @return [SpecificationProvider] the provider that knows about
  #
  # ```
  # dependencies, requirements, specifications, versions, etc.
  # ```
  def specification_provider; end

  # @return [Time] the time at which resolution began
  def started_at=(_); end

  # @return [Array<ResolutionState>] the stack of states for the resolution
  def states=(_); end
end

# A conflict that the resolution process encountered @attr [Object] requirement
# the requirement that immediately led to the conflict @attr
# [{[String,Nil=>](Object)}] requirements the requirements that caused the
# conflict @attr [Object, nil] existing the existing spec that was in conflict
# with
#
# ```
# the {#possibility}
# ```
#
# @attr [Object] possibility the spec that was unable to be activated due
#
# ```ruby
# to a conflict
# ```
#
# @attr [Object] locked\_requirement the relevant locking requirement. @attr
# [Array<Array<Object>>]
# [`requirement_trees`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo/Resolver/Resolution.html#method-i-requirement_trees)
# the different requirement
#
# ```
# trees that led to every requirement for the conflicting name.
# ```
#
# @attr [{String=>Object}] activated\_by\_name the already-activated specs.
class Gem::Resolver::Molinillo::Resolver::Resolution::Conflict < ::Struct
  Elem = type_member(:out, fixed: T.untyped)

  def activated_by_name; end

  def activated_by_name=(_); end

  def existing; end

  def existing=(_); end

  def locked_requirement; end

  def locked_requirement=(_); end

  def possibility; end

  def possibility=(_); end

  def requirement; end

  def requirement=(_); end

  def requirement_trees; end

  def requirement_trees=(_); end

  def requirements; end

  def requirements=(_); end

  def self.[](*_); end

  def self.inspect; end

  def self.members; end

  def self.new(*_); end
end

# An error that occurred during the resolution process
class Gem::Resolver::Molinillo::ResolverError < ::StandardError; end

# Provides information about specifcations and dependencies to the resolver,
# allowing the {Resolver} class to remain generic while still providing power
# and flexibility.
#
# This module contains the methods that users of
# [`Gem::Resolver::Molinillo`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Molinillo.html)
# must to implement, using knowledge of their own model classes.
module Gem::Resolver::Molinillo::SpecificationProvider
  # Returns whether this dependency, which has no possible matching
  # specifications, can safely be ignored.
  #
  # @param [Object] dependency @return [Boolean] whether this dependency can
  # safely be skipped.
  def allow_missing?(dependency); end

  # Returns the dependencies of `specification`. @note This method should be
  # 'pure', i.e. the return value should depend
  #
  # ```
  # only on the `specification` parameter.
  # ```
  #
  # @param [Object] specification @return [Array<Object>] the dependencies that
  # are required by the given
  #
  # ```
  # `specification`.
  # ```
  def dependencies_for(specification); end

  # Returns the name for the given `dependency`. @note This method should be
  # 'pure', i.e. the return value should depend
  #
  # ```
  # only on the `dependency` parameter.
  # ```
  #
  # @param [Object] dependency @return [String] the name for the given
  # `dependency`.
  def name_for(dependency); end

  # @return [String] the name of the source of explicit dependencies, i.e.
  #
  # ```
  # those passed to {Resolver#resolve} directly.
  # ```
  def name_for_explicit_dependency_source; end

  # @return [String] the name of the source of 'locked' dependencies, i.e.
  #
  # ```
  # those passed to {Resolver#resolve} directly as the `base`
  # ```
  def name_for_locking_dependency_source; end

  # Determines whether the given `requirement` is satisfied by the given `spec`,
  # in the context of the current `activated` dependency graph.
  #
  # @param [Object] requirement @param [DependencyGraph] activated the current
  # dependency graph in the
  #
  # ```
  # resolution process.
  # ```
  #
  # @param [Object] spec @return [Boolean] whether `requirement` is satisfied by
  # `spec` in the
  #
  # ```
  # context of the current `activated` dependency graph.
  # ```
  def requirement_satisfied_by?(requirement, activated, spec); end

  # Search for the specifications that match the given dependency. The
  # specifications in the returned array will be considered in reverse order, so
  # the latest version ought to be last. @note This method should be 'pure',
  # i.e. the return value should depend
  #
  # ```
  # only on the `dependency` parameter.
  # ```
  #
  # @param [Object] dependency @return [Array<Object>] the specifications that
  # satisfy the given
  #
  # ```
  # `dependency`.
  # ```
  def search_for(dependency); end

  # Sort dependencies so that the ones that are easiest to resolve are first.
  # Easiest to resolve is (usually) defined by:
  #
  # ```
  # 1) Is this dependency already activated?
  # 2) How relaxed are the requirements?
  # 3) Are there any conflicts for this dependency?
  # 4) How many possibilities are there to satisfy this dependency?
  # ```
  #
  # @param [Array<Object>] dependencies @param [DependencyGraph] activated the
  # current dependency graph in the
  #
  # ```
  # resolution process.
  # ```
  #
  # @param [{String =>
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html)<Conflict>}]
  # conflicts @return [Array<Object>] a sorted copy of `dependencies`.
  def sort_dependencies(dependencies, activated, conflicts); end
end

# Conveys information about the resolution process to a user.
module Gem::Resolver::Molinillo::UI
  # Called after resolution ends (either successfully or with an error). By
  # default, prints a newline.
  #
  # @return [void]
  def after_resolution; end

  # Called before resolution begins.
  #
  # @return [void]
  def before_resolution; end

  # Conveys debug information to the user.
  #
  # @param [Integer] depth the current depth of the resolution process. @return
  # [void]
  def debug(depth = _); end

  # Whether or not debug messages should be printed. By default, whether or not
  # the `MOLINILLO\_DEBUG` environment variable is set.
  #
  # @return [Boolean]
  def debug?; end

  # Called roughly every {#progress\_rate}, this method should convey progress
  # to the user.
  #
  # @return [void]
  def indicate_progress; end

  # The {IO} object that should be used to print output. `STDOUT`, by default.
  #
  # @return [IO]
  def output; end

  # How often progress should be conveyed to the user via {#indicate\_progress},
  # in seconds. A third of a second, by default.
  #
  # @return [Float]
  def progress_rate; end
end

# An error caused by conflicts in version
class Gem::Resolver::Molinillo::VersionConflict < ::Gem::Resolver::Molinillo::ResolverError
  # Initializes a new error with the given version conflicts. @param [{String =>
  # Resolution::Conflict}] conflicts see {#conflicts}
  def self.new(conflicts); end

  # @return [{String => Resolution::Conflict}] the conflicts that caused
  #
  # ```ruby
  # resolution to fail
  # ```
  def conflicts; end
end

# [`RequirementList`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/RequirementList.html)
# is used to hold the requirements being considered while resolving a set of
# gems.
#
# The
# [`RequirementList`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/RequirementList.html)
# acts like a queue where the oldest items are removed first.
class Gem::Resolver::RequirementList
  include(::Enumerable)

  Elem = type_member(:out)

  # Creates a new
  # [`RequirementList`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/RequirementList.html).
  def self.new; end

  # Adds Resolver::DependencyRequest `req` to this requirements list.
  def add(req); end

  def each; end

  # Is the list empty?
  def empty?; end

  # Returns the oldest five entries from the list.
  def next5; end

  # Remove the oldest DependencyRequest from the list.
  def remove; end

  # How many elements are in the list
  def size; end
end

# Resolver sets are used to look up specifications (and their dependencies) used
# in resolution. This set is abstract.
class Gem::Resolver::Set
  def self.new; end

  # Errors encountered when resolving gems
  def errors; end

  # Errors encountered when resolving gems
  def errors=(_); end

  # The find\_all method must be implemented. It returns all Resolver
  # Specification objects matching the given DependencyRequest `req`.
  def find_all(req); end

  # The prefetch method may be overridden, but this is not necessary. This
  # default implementation does nothing, which is suitable for sets where
  # looking up a specification is cheap (such as installed gems).
  #
  # When overridden, the prefetch method should look up specifications matching
  # `reqs`.
  def prefetch(reqs); end

  # When true, allows matching of requests to prerelease gems.
  def prerelease; end

  # When true, allows matching of requests to prerelease gems.
  def prerelease=(_); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Set.html) to true
  # to disable network access for this set
  def remote; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Set.html) to true
  # to disable network access for this set
  def remote=(_); end

  def remote?; end
end

# The
# [`SourceSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/SourceSet.html)
# chooses the best available method to query a remote index.
#
# Kind off like BestSet but filters the sources for gems
class Gem::Resolver::SourceSet < ::Gem::Resolver::Set
  # Creates a
  # [`SourceSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/SourceSet.html)
  # for the given `sources` or
  # [`Gem::sources`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-sources)
  # if none are specified. `sources` must be a
  # [`Gem::SourceList`](https://docs.ruby-lang.org/en/2.6.0/Gem/SourceList.html).
  def self.new; end

  def add_source_gem(name, source); end

  def find_all(req); end

  def prefetch(reqs); end
end

# The Resolver::SpecSpecification contains common functionality for Resolver
# specifications that are backed by a
# [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html).
class Gem::Resolver::SpecSpecification < ::Gem::Resolver::Specification
  # A
  # [`SpecSpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/SpecSpecification.html)
  # is created for a `set` for a
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # in `spec`. The `source` is either where the `spec` came from, or should be
  # loaded from.
  def self.new(set, spec, source = _); end

  # The dependencies of the gem for this specification
  def dependencies; end

  # The name and version of the specification.
  #
  # Unlike
  # [`Gem::Specification#full_name`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-full_name),
  # the platform is not included.
  def full_name; end

  # The name of the gem for this specification
  def name; end

  # The platform this gem works on.
  def platform; end

  # The version of the gem for this specification.
  def version; end
end

# A Resolver::Specification contains a subset of the information contained in a
# [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html).
# Only the information necessary for dependency resolution in the resolver is
# included.
class Gem::Resolver::Specification
  # Sets default instance variables for the specification.
  def self.new; end

  # The dependencies of the gem for this specification
  def dependencies; end

  def download(options); end

  def fetch_development_dependencies; end

  # The name and version of the specification.
  #
  # Unlike
  # [`Gem::Specification#full_name`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-full_name),
  # the platform is not included.
  def full_name; end

  # Installs this specification using the
  # [`Gem::Installer`](https://docs.ruby-lang.org/en/2.6.0/Gem/Installer.html)
  # `options`. The install method yields a
  # [`Gem::Installer`](https://docs.ruby-lang.org/en/2.6.0/Gem/Installer.html)
  # instance, which indicates the gem will be installed, or `nil`, which
  # indicates the gem is already installed.
  #
  # After installation
  # [`spec`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Specification.html#attribute-i-spec)
  # is updated to point to the just-installed specification.
  def install(options = _); end

  # Returns true if this specification is installable on this platform.
  def installable_platform?; end

  def local?; end

  # The name of the gem for this specification
  def name; end

  # The platform this gem works on.
  def platform; end

  # The set this specification came from.
  def set; end

  # The source for this specification
  def source; end

  # The
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # for this Resolver::Specification.
  #
  # Implementers, note that
  # [`install`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/Specification.html#method-i-install)
  # updates @spec, so be sure to cache the
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # in @spec when overriding.
  def spec; end

  # The version of the gem for this specification.
  def version; end
end

class Gem::Resolver::Stats
  PATTERN = T.let(T.unsafe(nil), String)

  def self.new; end

  def backtracking!; end

  def display; end

  def iteration!; end

  def record_depth(stack); end

  def record_requirements(reqs); end

  def requirement!; end
end

# A
# [`VendorSet`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/VendorSet.html)
# represents gems that have been unpacked into a specific directory that
# contains a gemspec.
#
# This is used for gem dependency file support.
#
# Example:
#
# ```ruby
# set = Gem::Resolver::VendorSet.new
#
# set.add_vendor_gem 'rake', 'vendor/rake'
# ```
#
# The directory vendor/rake must contain an unpacked rake gem along with a
# rake.gemspec (watching the given name).
class Gem::Resolver::VendorSet < ::Gem::Resolver::Set
  def self.new; end

  def add_vendor_gem(name, directory); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # VendorSpecification objects matching the DependencyRequest `req`.
  def find_all(req); end

  def load_spec(name, version, platform, source); end

  def pretty_print(q); end

  def specs; end
end

# A
# [`VendorSpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/VendorSpecification.html)
# represents a gem that has been unpacked into a project and is being loaded
# through a gem dependencies file through the `path:` option.
class Gem::Resolver::VendorSpecification < ::Gem::Resolver::SpecSpecification
  def ==(other); end

  # This is a null install as this gem was unpacked into a directory. `options`
  # are ignored.
  def install(options = _); end
end

# Raised when a gem dependencies file specifies a ruby version that does not
# match the current version.
class Gem::RubyVersionMismatch < ::Gem::Exception; end

class Gem::RuntimeRequirementNotMetError < ::Gem::InstallError
  def message; end

  def suggestion; end

  def suggestion=(_); end
end

# # Signing gems
#
# The [`Gem::Security`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html)
# implements cryptographic signatures for gems. The section below is a
# step-by-step guide to using signed gems and generating your own.
#
# ## Walkthrough
#
# ### Building your certificate
#
# In order to start signing your gems, you'll need to build a private key and a
# self-signed certificate. Here's how:
#
# ```
# # build a private key and certificate for yourself:
# $ gem cert --build you@example.com
# ```
#
# This could take anywhere from a few seconds to a minute or two, depending on
# the speed of your computer (public key algorithms aren't exactly the speediest
# crypto algorithms in the world). When it's finished, you'll see the files
# "gem-private\_key.pem" and "gem-public\_cert.pem" in the current directory.
#
# First things first: Move both files to ~/.gem if you don't already have a key
# and certificate in that directory. Ensure the file permissions make the key
# unreadable by others (by default the file is saved securely).
#
# Keep your private key hidden; if it's compromised, someone can sign packages
# as you (note: PKI has ways of mitigating the risk of stolen keys; more on that
# later).
#
# ### Signing Gems
#
# In RubyGems 2 and newer there is no extra work to sign a gem. RubyGems will
# automatically find your key and certificate in your home directory and use
# them to sign newly packaged gems.
#
# If your certificate is not self-signed (signed by a third party) RubyGems will
# attempt to load the certificate chain from the trusted certificates. Use `gem
# cert --add signing_cert.pem` to add your signers as trusted certificates. See
# below for further information on certificate chains.
#
# If you build your gem it will automatically be signed. If you peek inside your
# gem file, you'll see a couple of new files have been added:
#
# ```
# $ tar tf your-gem-1.0.gem
# metadata.gz
# metadata.gz.sum
# metadata.gz.sig # metadata signature
# data.tar.gz
# data.tar.gz.sum
# data.tar.gz.sig # data signature
# ```
#
# ### Manually signing gems
#
# If you wish to store your key in a separate secure location you'll need to set
# your gems up for signing by hand. To do this, set the `signing_key` and
# `cert_chain` in the gemspec before packaging your gem:
#
# ```ruby
# s.signing_key = '/secure/path/to/gem-private_key.pem'
# s.cert_chain = %w[/secure/path/to/gem-public_cert.pem]
# ```
#
# When you package your gem with these options set RubyGems will automatically
# load your key and certificate from the secure paths.
#
# ### Signed gems and security policies
#
# Now let's verify the signature. Go ahead and install the gem, but add the
# following options: `-P HighSecurity`, like this:
#
# ```
# # install the gem with using the security policy "HighSecurity"
# $ sudo gem install your.gem -P HighSecurity
# ```
#
# The `-P` option sets your security policy -- we'll talk about that in just a
# minute. Eh, what's this?
#
# ```
# $ gem install -P HighSecurity your-gem-1.0.gem
# ERROR:  While executing gem ... (Gem::Security::Exception)
#     root cert /CN=you/DC=example is not trusted
# ```
#
# The culprit here is the security policy. RubyGems has several different
# security policies. Let's take a short break and go over the security policies.
# Here's a list of the available security policies, and a brief description of
# each one:
#
# *   [`NoSecurity`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#NoSecurity)
#     - Well, no security at all. Signed packages are treated like unsigned
#     packages.
# *   [`LowSecurity`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#LowSecurity)
#     - Pretty much no security. If a package is signed then RubyGems will make
#     sure the signature matches the signing certificate, and that the signing
#     certificate hasn't expired, but that's it. A malicious user could easily
#     circumvent this kind of security.
# *   [`MediumSecurity`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#MediumSecurity)
#     - Better than
#     [`LowSecurity`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#LowSecurity)
#     and
#     [`NoSecurity`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#NoSecurity),
#     but still fallible. Package contents are verified against the signing
#     certificate, and the signing certificate is checked for validity, and
#     checked against the rest of the certificate chain (if you don't know what
#     a certificate chain is, stay tuned, we'll get to that). The biggest
#     improvement over
#     [`LowSecurity`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#LowSecurity)
#     is that
#     [`MediumSecurity`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#MediumSecurity)
#     won't install packages that are signed by untrusted sources.
#     Unfortunately,
#     [`MediumSecurity`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#MediumSecurity)
#     still isn't totally secure -- a malicious user can still unpack the gem,
#     strip the signatures, and distribute the gem unsigned.
# *   [`HighSecurity`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#HighSecurity)
#     - Here's the bugger that got us into this mess. The
#     [`HighSecurity`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#HighSecurity)
#     policy is identical to the
#     [`MediumSecurity`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#MediumSecurity)
#     policy, except that it does not allow unsigned gems. A malicious user
#     doesn't have a whole lot of options here; they can't modify the package
#     contents without invalidating the signature, and they can't modify or
#     remove signature or the signing certificate chain, or RubyGems will simply
#     refuse to install the package. Oh well, maybe they'll have better luck
#     causing problems for CPAN users instead :).
#
#
# The reason RubyGems refused to install your shiny new signed gem was because
# it was from an untrusted source. Well, your code is infallible (naturally), so
# you need to add yourself as a trusted source:
#
# ```
# # add trusted certificate
# gem cert --add ~/.gem/gem-public_cert.pem
# ```
#
# You've now added your public certificate as a trusted source. Now you can
# install packages signed by your private key without any hassle. Let's try the
# install command above again:
#
# ```
# # install the gem with using the HighSecurity policy (and this time
# # without any shenanigans)
# $ gem install -P HighSecurity your-gem-1.0.gem
# Successfully installed your-gem-1.0
# 1 gem installed
# ```
#
# This time RubyGems will accept your signed package and begin installing.
#
# While you're waiting for RubyGems to work it's magic, have a look at some of
# the other security commands by running `gem help cert`:
#
# ```
# Options:
#   -a, --add CERT                   Add a trusted certificate.
#   -l, --list [FILTER]              List trusted certificates where the
#                                    subject contains FILTER
#   -r, --remove FILTER              Remove trusted certificates where the
#                                    subject contains FILTER
#   -b, --build EMAIL_ADDR           Build private key and self-signed
#                                    certificate for EMAIL_ADDR
#   -C, --certificate CERT           Signing certificate for --sign
#   -K, --private-key KEY            Key for --sign or --build
#   -s, --sign CERT                  Signs CERT with the key from -K
#                                    and the certificate from -C
# ```
#
# We've already covered the `--build` option, and the `--add`, `--list`, and
# `--remove` commands seem fairly straightforward; they allow you to add, list,
# and remove the certificates in your trusted certificate list. But what's with
# this `--sign` option?
#
# ### Certificate chains
#
# To answer that question, let's take a look at "certificate chains", a concept
# I mentioned earlier. There are a couple of problems with self-signed
# certificates: first of all, self-signed certificates don't offer a whole lot
# of security. Sure, the certificate says Yukihiro Matsumoto, but how do I know
# it was actually generated and signed by matz himself unless he gave me the
# certificate in person?
#
# The second problem is scalability. Sure, if there are 50 gem authors, then I
# have 50 trusted certificates, no problem. What if there are 500 gem authors?
# 1000?  Having to constantly add new trusted certificates is a pain, and it
# actually makes the trust system less secure by encouraging RubyGems users to
# blindly trust new certificates.
#
# Here's where certificate chains come in. A certificate chain establishes an
# arbitrarily long chain of trust between an issuing certificate and a child
# certificate. So instead of trusting certificates on a per-developer basis, we
# use the PKI concept of certificate chains to build a logical hierarchy of
# trust. Here's a hypothetical example of a trust hierarchy based (roughly) on
# geography:
#
# ```
#                     --------------------------
#                     | rubygems@rubygems.org |
#                     --------------------------
#                                 |
#               -----------------------------------
#               |                                 |
#   ----------------------------    -----------------------------
#   |  seattlerb@seattlerb.org |    | dcrubyists@richkilmer.com |
#   ----------------------------    -----------------------------
#        |                |                 |             |
# ---------------   ----------------   -----------   --------------
# |   drbrain   |   |   zenspider  |   | pabs@dc |   | tomcope@dc |
# ---------------   ----------------   -----------   --------------
# ```
#
# Now, rather than having 4 trusted certificates (one for drbrain, zenspider,
# pabs@dc, and tomecope@dc), a user could actually get by with one certificate,
# the "rubygems@rubygems.org" certificate.
#
# Here's how it works:
#
# I install "rdoc-3.12.gem", a package signed by "drbrain". I've never heard of
# "drbrain", but his certificate has a valid signature from the
# "seattle.rb@seattlerb.org" certificate, which in turn has a valid signature
# from the "rubygems@rubygems.org" certificate. Voila!  At this point, it's much
# more reasonable for me to trust a package signed by "drbrain", because I can
# establish a chain to "rubygems@rubygems.org", which I do trust.
#
# ### Signing certificates
#
# The `--sign` option allows all this to happen. A developer creates their build
# certificate with the `--build` option, then has their certificate signed by
# taking it with them to their next regional Ruby meetup (in our hypothetical
# example), and it's signed there by the person holding the regional RubyGems
# signing certificate, which is signed at the next RubyConf by the holder of the
# top-level RubyGems certificate. At each point the issuer runs the same
# command:
#
# ```
# # sign a certificate with the specified key and certificate
# # (note that this modifies client_cert.pem!)
# $ gem cert -K /mnt/floppy/issuer-priv_key.pem -C issuer-pub_cert.pem
#    --sign client_cert.pem
# ```
#
# Then the holder of issued certificate (in this case, your buddy "drbrain"),
# can start using this signed certificate to sign RubyGems. By the way, in order
# to let everyone else know about his new fancy signed certificate, "drbrain"
# would save his newly signed certificate as `~/.gem/gem-public_cert.pem`
#
# Obviously this RubyGems trust infrastructure doesn't exist yet. Also, in the
# "real world", issuers actually generate the child certificate from a
# certificate request, rather than sign an existing certificate. And our
# hypothetical infrastructure is missing a certificate revocation system. These
# are that can be fixed in the future...
#
# At this point you should know how to do all of these new and interesting
# things:
#
# *   build a gem signing key and certificate
# *   adjust your security policy
# *   modify your trusted certificate list
# *   sign a certificate
#
#
# ## Manually verifying signatures
#
# In case you don't trust RubyGems you can verify gem signatures manually:
#
# 1.  Fetch and unpack the gem
#
# ```
# gem fetch some_signed_gem
# tar -xf some_signed_gem-1.0.gem
# ```
#
# 2.  Grab the public key from the gemspec
#
# ```
# gem spec some_signed_gem-1.0.gem cert_chain | \
#   ruby -ryaml -e 'puts YAML.load_documents($stdin)' > public_key.crt
# ```
#
# 3.  Generate a SHA1 hash of the data.tar.gz
#
# ```ruby
# openssl dgst -sha1 < data.tar.gz > my.hash
# ```
#
# 4.  Verify the signature
#
# ```
# openssl rsautl -verify -inkey public_key.crt -certin \
#   -in data.tar.gz.sig > verified.hash
# ```
#
# 5.  Compare your hash to the verified hash
#
# ```
# diff -s verified.hash my.hash
# ```
#
# 6.  Repeat 5 and 6 with metadata.gz
#
#
# ## [`OpenSSL`](https://docs.ruby-lang.org/en/2.6.0/OpenSSL.html) Reference
#
# The .pem files generated by --build and --sign are PEM files. Here's a couple
# of useful [`OpenSSL`](https://docs.ruby-lang.org/en/2.6.0/OpenSSL.html)
# commands for manipulating them:
#
# ```
# # convert a PEM format X509 certificate into DER format:
# # (note: Windows .cer files are X509 certificates in DER format)
# $ openssl x509 -in input.pem -outform der -out output.der
#
# # print out the certificate in a human-readable format:
# $ openssl x509 -in input.pem -noout -text
# ```
#
# And you can do the same thing with the private key file as well:
#
# ```
# # convert a PEM format RSA key into DER format:
# $ openssl rsa -in input_key.pem -outform der -out output_key.der
#
# # print out the key in a human readable format:
# $ openssl rsa -in input_key.pem -noout -text
# ```
#
# ## Bugs/TODO
#
# *   There's no way to define a system-wide trust list.
# *   custom security policies (from a
#     [`YAML`](https://docs.ruby-lang.org/en/2.6.0/YAML.html) file, etc)
# *   Simple method to generate a signed certificate request
# *   Support for OCSP, SCVP, CRLs, or some other form of cert status check
#     (list is in order of preference)
# *   Support for encrypted private keys
# *   Some sort of semi-formal trust hierarchy (see long-winded explanation
#     above)
# *   Path discovery (for gem certificate chains that don't have a self-signed
#     root) -- by the way, since we don't have this, THE ROOT OF THE CERTIFICATE
#     CHAIN MUST BE SELF SIGNED if Policy#verify\_root is true (and it is for
#     the
#     [`MediumSecurity`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#MediumSecurity)
#     and
#     [`HighSecurity`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#HighSecurity)
#     policies)
# *   Better explanation of X509 naming (ie, we don't have to use email
#     addresses)
# *   Honor AIA field (see note about OCSP above)
# *   Honor extension restrictions
# *   Might be better to store the certificate chain as a PKCS#7 or PKCS#12
#     file, instead of an array embedded in the metadata.
# *   Flexible signature and key algorithms, not hard-coded to RSA and SHA1.
#
#
# ## Original author
#
# Paul Duncan <pabs@pablotron.org> http://pablotron.org/
module Gem::Security
  # AlmostNo security policy: only verify that the signing certificate is the
  # one that actually signed the data. Make no attempt to verify the signing
  # certificate chain.
  #
  # This policy is basically useless. better than nothing, but can still be
  # easily spoofed, and is not recommended.
  AlmostNoSecurity = T.let(T.unsafe(nil), Gem::Security::Policy)

  # [`Digest`](https://docs.ruby-lang.org/en/2.6.0/Digest.html) algorithm used
  # to sign gems
  DIGEST_ALGORITHM = OpenSSL::Digest::SHA256

  DIGEST_NAME = T.let(T.unsafe(nil), String)

  # The default set of extensions are:
  #
  # *   The certificate is not a certificate authority
  # *   The key for the certificate may be used for key and data encipherment
  #     and digital signatures
  # *   The certificate contains a subject key identifier
  EXTENSIONS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # High security policy: only allow signed gems to be installed, verify the
  # signing certificate, verify the signing certificate chain all the way to the
  # root certificate, and only trust root certificates that we have explicitly
  # allowed trust for.
  #
  # This security policy is significantly more difficult to bypass, and offers a
  # reasonable guarantee that the contents of the gem have not been altered.
  HighSecurity = T.let(T.unsafe(nil), Gem::Security::Policy)

  # Algorithm for creating the key pair used to sign gems
  KEY_ALGORITHM = OpenSSL::PKey::RSA

  # Cipher used to encrypt the key pair used to sign gems. Must be in the list
  # returned by
  # [`OpenSSL::Cipher.ciphers`](https://docs.ruby-lang.org/en/2.6.0/OpenSSL/Cipher.html#method-c-ciphers)
  KEY_CIPHER = T.let(T.unsafe(nil), OpenSSL::Cipher)

  # Length of keys created by
  # [`KEY_ALGORITHM`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#KEY_ALGORITHM)
  KEY_LENGTH = T.let(T.unsafe(nil), Integer)

  # Low security policy: only verify that the signing certificate is actually
  # the gem signer, and that the signing certificate is valid.
  #
  # This policy is better than nothing, but can still be easily spoofed, and is
  # not recommended.
  LowSecurity = T.let(T.unsafe(nil), Gem::Security::Policy)

  # Medium security policy: verify the signing certificate, verify the signing
  # certificate chain all the way to the root certificate, and only trust root
  # certificates that we have explicitly allowed trust for.
  #
  # This security policy is reasonable, but it allows unsigned packages, so a
  # malicious person could simply delete the package signature and pass the gem
  # off as unsigned.
  MediumSecurity = T.let(T.unsafe(nil), Gem::Security::Policy)

  # No security policy: all package signature checks are disabled.
  NoSecurity = T.let(T.unsafe(nil), Gem::Security::Policy)

  # One day in seconds
  ONE_DAY = T.let(T.unsafe(nil), Integer)

  # One year in seconds
  ONE_YEAR = T.let(T.unsafe(nil), Integer)

  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) of configured
  # security policies
  Policies = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  def self.alt_name_or_x509_entry(certificate, x509_entry); end

  # Creates an unsigned certificate for `subject` and `key`. The lifetime of the
  # key is from the current time to `age` which defaults to one year.
  #
  # The `extensions` restrict the key to the indicated uses.
  def self.create_cert(subject, key, age = _, extensions = _, serial = _); end

  # Creates a self-signed certificate with an issuer and subject from `email`, a
  # subject alternative name of `email` and the given `extensions` for the
  # `key`.
  def self.create_cert_email(email, key, age = _, extensions = _); end

  # Creates a self-signed certificate with an issuer and subject of `subject`
  # and the given `extensions` for the `key`.
  def self.create_cert_self_signed(subject, key, age = _, extensions = _, serial = _); end

  # Creates a new key pair of the specified `length` and `algorithm`. The
  # default is a 3072 bit RSA key.
  def self.create_key(length = _, algorithm = _); end

  # Turns `email_address` into an
  # [`OpenSSL::X509::Name`](https://docs.ruby-lang.org/en/2.6.0/OpenSSL/X509/Name.html)
  def self.email_to_name(email_address); end

  # Signs `expired_certificate` with `private_key` if the keys match and the
  # expired certificate was self-signed.
  def self.re_sign(expired_certificate, private_key, age = _, extensions = _); end

  # Resets the trust directory for verifying gems.
  def self.reset; end

  # Sign the public key from `certificate` with the `signing_key` and
  # `signing_cert`, using the
  # [`Gem::Security::DIGEST_ALGORITHM`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html#DIGEST_ALGORITHM).
  # Uses the default certificate validity range and extensions.
  #
  # Returns the newly signed certificate.
  def self.sign(certificate, signing_key, signing_cert, age = _, extensions = _, serial = _); end

  # Returns a
  # [`Gem::Security::TrustDir`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security/TrustDir.html)
  # which wraps the directory where trusted certificates live.
  def self.trust_dir; end

  # Enumerates the trusted certificates via
  # [`Gem::Security::TrustDir`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security/TrustDir.html).
  def self.trusted_certificates(&block); end

  # Writes `pemmable`, which must respond to `to_pem` to `path` with the given
  # `permissions`. If passed `cipher` and `passphrase` those arguments will be
  # passed to `to_pem`.
  def self.write(pemmable, path, permissions = _, passphrase = _, cipher = _); end
end

# [`Gem::Security`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html)
# default exception type
class Gem::Security::Exception < ::Gem::Exception; end

# A
# [`Gem::Security::Policy`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security/Policy.html)
# object encapsulates the settings for verifying signed gem files. This is the
# base class. You can either declare an instance of this or use one of the
# preset security policies in Gem::Security::Policies.
class Gem::Security::Policy
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)

  # Create a new
  # [`Gem::Security::Policy`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security/Policy.html)
  # object with the given mode and options.
  def self.new(name, policy = _, opt = _); end

  # Ensures that `signer` is valid for `time` and was signed by the `issuer`. If
  # the `issuer` is `nil` no verification is performed.
  def check_cert(signer, issuer, time); end

  # Verifies each certificate in `chain` has signed the following certificate
  # and is valid for the given `time`.
  def check_chain(chain, time); end

  # Verifies that `data` matches the `signature` created by `public_key` and the
  # `digest` algorithm.
  def check_data(public_key, digest, signature, data); end

  # Ensures the public key of `key` matches the public key in `signer`
  def check_key(signer, key); end

  # Ensures the root certificate in `chain` is self-signed and valid for `time`.
  def check_root(chain, time); end

  # Ensures the root of `chain` has a trusted certificate in `trust_dir` and the
  # digests of the two certificates match according to `digester`
  def check_trust(chain, digester, trust_dir); end

  def inspect; end

  def name; end

  def only_signed; end

  def only_signed=(_); end

  def only_trusted; end

  def only_trusted=(_); end

  def subject(certificate); end

  def to_s; end

  # For `full_name`, verifies the certificate `chain` is valid, the `digests`
  # match the signatures `signatures` created by the signer depending on the
  # `policy` settings.
  #
  # If `key` is given it is used to validate the signing certificate.
  def verify(chain, key = _, digests = _, signatures = _, full_name = _); end

  def verify_chain; end

  def verify_chain=(_); end

  def verify_data; end

  def verify_data=(_); end

  def verify_root; end

  def verify_root=(_); end

  # Extracts the certificate chain from the `spec` and calls
  # [`verify`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security/Policy.html#method-i-verify)
  # to ensure the signatures and certificate chain is valid according to the
  # policy..
  def verify_signatures(spec, digests, signatures); end

  def verify_signer; end

  def verify_signer=(_); end
end

class Gem::Security::Signer
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)

  DEFAULT_OPTIONS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  SigningPolicy = T.let(T.unsafe(nil), Gem::Security::Policy)

  # Creates a new signer with an RSA `key` or path to a key, and a certificate
  # `chain` containing X509 certificates, encoding certificates or paths to
  # certificates.
  def self.new(key, cert_chain, passphrase = _, options = _); end

  # The chain of certificates for signing including the signing certificate
  def cert_chain; end

  # The chain of certificates for signing including the signing certificate
  def cert_chain=(_); end

  # The digest algorithm used to create the signature
  def digest_algorithm; end

  def digest_name; end

  def extract_name(cert); end

  # The private key for the signing certificate
  def key; end

  # The private key for the signing certificate
  def key=(_); end

  def load_cert_chain; end

  # [`Gem::Security::Signer`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security/Signer.html)
  # options
  def options; end

  def re_sign_key(expiration_length: _); end

  # Sign data with given digest algorithm
  def sign(data); end

  # Attemps to re-sign an expired cert with a given private key
  def self.re_sign_cert(expired_cert, expired_cert_path, private_key); end
end

# The
# [`TrustDir`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security/TrustDir.html)
# manages the trusted certificates for gem signature verification.
class Gem::Security::TrustDir
  # Default permissions for the trust directory and its contents
  DEFAULT_PERMISSIONS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Creates a new
  # [`TrustDir`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security/TrustDir.html)
  # using `dir` where the directory and file permissions will be checked
  # according to `permissions`
  def self.new(dir, permissions = _); end

  # Returns the path to the trusted `certificate`
  def cert_path(certificate); end

  # The directory where trusted certificates will be stored.
  def dir; end

  # Enumerates trusted certificates.
  def each_certificate; end

  # Returns the issuer certificate of the given `certificate` if it exists in
  # the trust directory.
  def issuer_of(certificate); end

  # Loads the given `certificate_file`
  def load_certificate(certificate_file); end

  # Returns the path to the trusted certificate with the given ASN.1 `name`
  def name_path(name); end

  # Add a certificate to trusted certificate list.
  def trust_cert(certificate); end

  # Make sure the trust directory exists. If it does exist, make sure it's
  # actually a directory. If not, then create it with the appropriate
  # permissions.
  def verify; end
end

# [`Gem::Server`](https://docs.ruby-lang.org/en/2.6.0/Gem/Server.html) and
# allows users to serve gems for consumption by `gem --remote-install`.
#
# gem\_server starts an HTTP server on the given port and serves the following:
# *   "/" - Browsing of gem spec files for installed gems
# *   "/specs.#{Gem.marshal\_version}.gz" - specs name/version/platform index
# *   "/latest\_specs.#{Gem.marshal\_version}.gz" - latest specs
#     name/version/platform index
# *   "/quick/" - Individual gemspecs
# *   "/gems" - Direct access to download the installable gems
# *   "/rdoc?q=" - Search for installed rdoc documentation
#
#
# ## Usage
#
# ```ruby
# gem_server = Gem::Server.new Gem.dir, 8089, false
# gem_server.run
# ```
class Gem::Server
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)
  include(::ERB::Util)

  DOC_TEMPLATE = T.let(T.unsafe(nil), String)

  # CSS is copy & paste from rdoc-style.css,
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) V1.0.1 - 20041108
  RDOC_CSS = T.let(T.unsafe(nil), String)

  RDOC_NO_DOCUMENTATION = T.let(T.unsafe(nil), String)

  RDOC_SEARCH_TEMPLATE = T.let(T.unsafe(nil), String)

  SEARCH = T.let(T.unsafe(nil), String)

  def self.new(gem_dirs, port, daemon, launch = _, addresses = _); end

  def add_date(res); end

  def doc_root(gem_name); end

  def have_rdoc_4_plus?; end

  def latest_specs(req, res); end

  def launch; end

  # Creates server sockets based on the addresses option. If no addresses were
  # given a server socket for all interfaces is created.
  def listen(addresses = _); end

  def prerelease_specs(req, res); end

  def quick(req, res); end

  # Can be used for quick navigation to the rdoc documentation. You can then
  # define a search shortcut for your browser. E.g. in Firefox connect
  # 'shortcut:rdoc' to http://localhost:8808/rdoc?q=%s template. Then you can
  # directly open the ActionPack documentation by typing 'rdoc actionp'. If
  # there are multiple hits for the search term, they are presented as a list
  # with links.
  #
  # Search algorithm aims for an intuitive search:
  # 1.  first try to find the gems and documentation folders which name starts
  #     with the search term
  # 2.  search for entries, that **contain** the search term
  # 3.  show all the gems
  #
  #
  # If there is only one search hit, user is immediately redirected to the
  # documentation for the particular gem, otherwise a list with results is
  # shown.
  #
  # ### Additional trick - install documentation for Ruby core
  #
  # Note: please adjust paths accordingly use for example 'locate yaml.rb' and
  # 'gem environment' to identify directories, that are specific for your local
  # installation
  #
  # 1.  install Ruby sources
  #
  # ```
  # cd /usr/src
  # sudo apt-get source ruby
  # ```
  #
  # 2.  generate documentation
  #
  # ```
  # rdoc -o /usr/lib/ruby/gems/1.8/doc/core/rdoc \
  #   /usr/lib/ruby/1.8 ruby1.8-1.8.7.72
  # ```
  #
  #
  # By typing 'rdoc core' you can now access the core documentation
  def rdoc(req, res); end

  def reset_gems; end

  def root(req, res); end

  def run; end

  # Returns true and prepares http response, if rdoc for the requested gem name
  # pattern was found.
  #
  # The search is based on the file system content, not on the gems metadata.
  # This allows additional documentation folders like 'core' for the Ruby core
  # documentation - just put it underneath the main doc folder.
  def show_rdoc_for_pattern(pattern, res); end

  def spec_dirs; end

  def specs(req, res); end

  def uri_encode(str); end

  def self.run(options); end
end

# [`SilentUI`](https://docs.ruby-lang.org/en/2.6.0/Gem/SilentUI.html) is a UI
# choice that is absolutely silent.
class Gem::SilentUI < ::Gem::StreamUI
  # The [`SilentUI`](https://docs.ruby-lang.org/en/2.6.0/Gem/SilentUI.html) has
  # no arguments as it does not use any stream.
  def self.new; end

  def close; end

  def download_reporter(*args); end

  def progress_reporter(*args); end
end

# A [`Source`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source.html) knows how to
# list and fetch gems from a RubyGems marshal index.
#
# There are other
# [`Source`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source.html) subclasses for
# installed gems, local gems, the bundler dependency API and so-forth.
class Gem::Source
  include(::Comparable)

  FILES = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # Creates a new
  # [`Source`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source.html) which will
  # use the index located at `uri`.
  def self.new(uri); end

  # Sources are ordered by installation preference.
  def <=>(other); end

  def ==(other); end

  # Returns the local directory to write `uri` to.
  def cache_dir(uri); end

  def dependency_resolver_set; end

  # Downloads `spec` and writes it to `dir`. See also
  # [`Gem::RemoteFetcher#download`](https://docs.ruby-lang.org/en/2.6.0/Gem/RemoteFetcher.html#method-i-download).
  def download(spec, dir = _); end

  def eql?(other); end

  # Fetches a specification for the given `name_tuple`.
  def fetch_spec(name_tuple); end

  def hash; end

  # Loads `type` kind of specs fetching from +@uri+ if the on-disk cache is out
  # of date.
  #
  # `type` is one of the following:
  #
  # :released   => Return the list of all released specs :latest     => Return
  # the list of only the highest version of each gem :prerelease => Return the
  # list of all prerelease only specs
  def load_specs(type); end

  def pretty_print(q); end

  # Returns true when it is possible and safe to update the cache directory.
  def update_cache?; end

  # The [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) this source will
  # fetch gems from.
  def uri; end
end

# A git gem for use in a gem dependencies file.
#
# Example:
#
# ```ruby
# source =
#   Gem::Source::Git.new 'rake', 'git@example:rake.git', 'rake-10.1.0', false
#
# source.specs
# ```
class Gem::Source::Git < ::Gem::Source
  # Creates a new git gem source for a gems from loaded from `repository` at the
  # given `reference`. The `name` is only used to track the repository back to a
  # gem dependencies file, it has no real significance as a git repository may
  # contain multiple gems. If `submodules` is true, submodules will be checked
  # out when the gem is installed.
  def self.new(name, repository, reference, submodules = _); end

  def <=>(other); end

  def ==(other); end

  def base_dir; end

  def cache; end

  def checkout; end

  def dir_shortref; end

  def download(full_spec, path); end

  def install_dir; end

  # The name of the gem created by this git gem.
  def name; end

  # Does this repository need submodules checked out too?
  def need_submodules; end

  def pretty_print(q); end

  # The commit reference used for checking out this git gem.
  def reference; end

  # When false the cache for this repository will not be updated.
  def remote; end

  # When false the cache for this repository will not be updated.
  def remote=(_); end

  def repo_cache_dir; end

  # The git repository this gem is sourced from.
  def repository; end

  def rev_parse; end

  # The directory for cache and git gem installation
  def root_dir; end

  # The directory for cache and git gem installation
  def root_dir=(_); end

  # Loads all gemspecs in the repository
  def specs; end

  def uri_hash; end
end

# Represents an installed gem. This is used for dependency resolution.
class Gem::Source::Installed < ::Gem::Source
  def self.new; end

  # [`Installed`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source/Installed.html)
  # sources sort before all other sources
  def <=>(other); end

  # We don't need to download an installed gem
  def download(spec, path); end

  def pretty_print(q); end
end

# The local source finds gems in the current directory for fulfilling
# dependencies.
class Gem::Source::Local < ::Gem::Source
  def self.new; end

  # [`Local`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source/Local.html) sorts
  # before [`Gem::Source`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source.html)
  # and after
  # [`Gem::Source::Installed`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source/Installed.html)
  def <=>(other); end

  def download(spec, cache_dir = _); end

  def fetch_spec(name); end

  def find_gem(gem_name, version = _, prerelease = _); end

  def inspect; end

  def load_specs(type); end

  def pretty_print(q); end
end

# A [`Lock`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source/Lock.html) source
# wraps an installed gem's source and sorts before other sources during
# dependency resolution. This allows RubyGems to prefer gems from dependency
# lock files.
class Gem::Source::Lock < ::Gem::Source
  # Creates a new
  # [`Lock`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source/Lock.html) source
  # that wraps `source` and moves it earlier in the sort list.
  def self.new(source); end

  def <=>(other); end

  def ==(other); end

  # Delegates to the wrapped source's
  # [`fetch_spec`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source/Lock.html#method-i-fetch_spec)
  # method.
  def fetch_spec(name_tuple); end

  def hash; end

  def uri; end

  # The wrapped
  # [`Gem::Source`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source.html)
  def wrapped; end
end

# A source representing a single .gem file. This is used for installation of
# local gems.
class Gem::Source::SpecificFile < ::Gem::Source
  # Creates a new
  # [`SpecificFile`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source/SpecificFile.html)
  # for the gem in `file`
  def self.new(file); end

  # Orders this source against `other`.
  #
  # If `other` is a
  # [`SpecificFile`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source/SpecificFile.html)
  # from a different gem name `nil` is returned.
  #
  # If `other` is a
  # [`SpecificFile`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source/SpecificFile.html)
  # from the same gem name the versions are compared using Gem::Version#<=>
  #
  # Otherwise Gem::Source#<=> is used.
  def <=>(other); end

  def download(spec, dir = _); end

  def fetch_spec(name); end

  def load_specs(*a); end

  # The path to the gem for this specific file.
  def path; end

  def pretty_print(q); end

  # The
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # extracted from this .gem.
  def spec; end
end

# This represents a vendored source that is similar to an installed gem.
class Gem::Source::Vendor < ::Gem::Source::Installed
  # Creates a new
  # [`Vendor`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source/Vendor.html)
  # source for a gem that was unpacked at `path`.
  def self.new(path); end

  def <=>(other); end
end

# An error that indicates we weren't able to fetch some data from a source
class Gem::SourceFetchProblem < ::Gem::ErrorReason
  # Creates a new
  # [`SourceFetchProblem`](https://docs.ruby-lang.org/en/2.6.0/Gem/SourceFetchProblem.html)
  # for the given `source` and `error`.
  def self.new(source, error); end

  # The fetch error which is an
  # [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html) subclass.
  def error; end

  # The fetch error which is an
  # [`Exception`](https://docs.ruby-lang.org/en/2.6.0/Exception.html) subclass.
  def exception; end

  # The source that had the fetch problem.
  def source; end

  # An [`English`](https://docs.ruby-lang.org/en/2.6.0/English.html) description
  # of the error.
  def wordy; end
end

# The [`SourceList`](https://docs.ruby-lang.org/en/2.6.0/Gem/SourceList.html)
# represents the sources rubygems has been configured to use. A source may be
# created from an array of sources:
#
# ```ruby
# Gem::SourceList.from %w[https://rubygems.example https://internal.example]
# ```
#
# Or by adding them:
#
# ```ruby
# sources = Gem::SourceList.new
# sources << 'https://rubygems.example'
# ```
#
# The most common way to get a
# [`SourceList`](https://docs.ruby-lang.org/en/2.6.0/Gem/SourceList.html) is
# [`Gem.sources`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-sources).
class Gem::SourceList
  include(::Enumerable)

  Elem = type_member(:out)

  # Creates a new
  # [`SourceList`](https://docs.ruby-lang.org/en/2.6.0/Gem/SourceList.html)
  def self.new; end

  # Appends `obj` to the source list which may be a
  # [`Gem::Source`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source.html),
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) or
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html)
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html).
  def <<(obj); end

  def ==(other); end

  # Removes all sources from the
  # [`SourceList`](https://docs.ruby-lang.org/en/2.6.0/Gem/SourceList.html).
  def clear; end

  # Deletes `source` from the source list which may be a
  # [`Gem::Source`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source.html) or a
  # [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html).
  def delete(source); end

  # Yields each source [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) in
  # the list.
  def each; end

  # Yields each source in the list.
  def each_source(&b); end

  # Returns true if there are no sources in this
  # [`SourceList`](https://docs.ruby-lang.org/en/2.6.0/Gem/SourceList.html).
  def empty?; end

  # Returns the first source in the list.
  def first; end

  # Returns true if this source list includes `other` which may be a
  # [`Gem::Source`](https://docs.ruby-lang.org/en/2.6.0/Gem/Source.html) or a
  # source [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html).
  def include?(other); end

  # Replaces this
  # [`SourceList`](https://docs.ruby-lang.org/en/2.6.0/Gem/SourceList.html) with
  # the sources in `other`  See
  # [`<<`](https://docs.ruby-lang.org/en/2.6.0/Gem/SourceList.html#method-i-3C-3C)
  # for acceptable items in `other`.
  def replace(other); end

  # The sources in this list
  def sources; end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of
  # source [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) Strings.
  #
  # Also aliased as:
  # [`to_ary`](https://docs.ruby-lang.org/en/2.6.0/Gem/SourceList.html#method-i-to_ary)
  def to_a; end

  # Alias for:
  # [`to_a`](https://docs.ruby-lang.org/en/2.6.0/Gem/SourceList.html#method-i-to_a)
  def to_ary; end

  # Creates a new
  # [`SourceList`](https://docs.ruby-lang.org/en/2.6.0/Gem/SourceList.html) from
  # an array of sources.
  def self.from(ary); end
end

# [`SpecFetcher`](https://docs.ruby-lang.org/en/2.6.0/Gem/SpecFetcher.html)
# handles metadata updates from remote gem repositories.
class Gem::SpecFetcher
  include(::Gem::Text)
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)

  # Creates a new
  # [`SpecFetcher`](https://docs.ruby-lang.org/en/2.6.0/Gem/SpecFetcher.html).
  # Ordinarily you want to use the default fetcher from
  # [`Gem::SpecFetcher::fetcher`](https://docs.ruby-lang.org/en/2.6.0/Gem/SpecFetcher.html#method-c-fetcher)
  # which uses the
  # [`Gem.sources`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-sources).
  #
  # If you need to retrieve specifications from a different `source`, you can
  # send it as an argument.
  def self.new(sources = _); end

  # Returns a list of gems available for each source in
  # [`Gem::sources`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-sources).
  #
  # `type` can be one of 3 values: :released   => Return the list of all
  # released specs :complete   => Return the list of all specs :latest     =>
  # Return the list of only the highest version of each gem :prerelease =>
  # Return the list of all prerelease only specs
  def available_specs(type); end

  # Return all gem name tuples who's names match `obj`
  def detect(type = _); end

  def latest_specs; end

  def prerelease_specs; end

  # [`Find`](https://docs.ruby-lang.org/en/2.6.0/Find.html) and fetch gem name
  # tuples that match `dependency`.
  #
  # If `matching_platform` is false, gems for all platforms are returned.
  def search_for_dependency(dependency, matching_platform = _); end

  def sources; end

  # [`Find`](https://docs.ruby-lang.org/en/2.6.0/Find.html) and fetch specs that
  # match `dependency`.
  #
  # If `matching_platform` is false, gems for all platforms are returned.
  def spec_for_dependency(dependency, matching_platform = _); end

  def specs; end

  # Suggests gems based on the supplied `gem_name`. Returns an array of
  # alternative gem names.
  def suggest_gems_from_name(gem_name, type = _); end

  def tuples_for(source, type, gracefully_ignore = _); end

  # Default fetcher instance. Use this instead of
  # [`::new`](https://docs.ruby-lang.org/en/2.6.0/Gem/SpecFetcher.html#method-c-new)
  # to reduce object allocation.
  def self.fetcher; end

  def self.fetcher=(fetcher); end
end

# Raised by the DependencyInstaller when a specific gem cannot be found
class Gem::SpecificGemNotFoundException < ::Gem::GemNotFoundException
  # Creates a new
  # [`SpecificGemNotFoundException`](https://docs.ruby-lang.org/en/2.6.0/Gem/SpecificGemNotFoundException.html)
  # for a gem with the given `name` and `version`. Any `errors` encountered when
  # attempting to find the gem are also stored.
  def self.new(name, version, errors = _); end

  # Errors encountered attempting to find the gem.
  def errors; end

  # The name of the gem that could not be found.
  def name; end

  # The version of the gem that could not be found.
  def version; end
end

# The
# [`Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
# class contains the information for a
# [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html). Typically defined in a
# .gemspec file or a
# [Rakefile](https://docs.ruby-lang.org/en/2.6.0/gems/2_6_0/gems/mini_portile2-2_4_0/Rakefile.html),
# and looks like this:
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
class Gem::Specification < ::Gem::BasicSpecification
  extend(::Enumerable)
  extend(::Gem::Deprecate)

  CURRENT_SPECIFICATION_VERSION = T.let(T.unsafe(nil), Integer)

  DateLike = T.let(T.unsafe(nil), Object)

  DateTimeFormat = T.let(T.unsafe(nil), Regexp)

  EMPTY = T.let(T.unsafe(nil), T::Array[T.untyped])

  INITIALIZE_CODE_FOR_DEFAULTS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  MARSHAL_FIELDS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  # The version number of a specification that does not specify one (i.e.
  # RubyGems 0.7 or earlier).
  NONEXISTENT_SPECIFICATION_VERSION = T.let(T.unsafe(nil), Integer)

  NOT_FOUND = T.let(T.unsafe(nil), T.untyped)

  SPECIFICATION_VERSION_HISTORY = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  TODAY = T.let(T.unsafe(nil), Time)

  VALID_NAME_PATTERN = T.let(T.unsafe(nil), Regexp)

  # [`Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # constructor. Assigns the default values to the attributes and yields itself
  # for further initialization. Optionally takes `name` and `version`.
  def self.new(name = _, version = _); end

  def <=>(other); end

  def ==(other); end

  def _deprecated_default_executable; end

  def _deprecated_default_executable=(_); end

  def _deprecated_has_rdoc; end

  def _deprecated_has_rdoc=(ignored); end

  def _deprecated_has_rdoc?(*args, &block); end

  # Dump only crucial instance variables.
  def _dump(limit); end

  # Abbreviate the spec for downloading. Abbreviated specs are only used for
  # searching, downloading and related activities and do not need deployment
  # specific information (e.g. list of files). So we abbreviate the spec, making
  # it much smaller for quicker downloads.
  def abbreviate; end

  # Activate this spec, registering it as a loaded spec and adding it's lib
  # paths to $LOAD\_PATH. Returns true if the spec was activated, false if it
  # was previously activated. Freaks out if there are conflicts upon activation.
  def activate; end

  # Activate all unambiguously resolved runtime dependencies of this spec. Add
  # any ambiguous dependencies to the unresolved list to be resolved later, as
  # needed.
  def activate_dependencies; end

  # True when this gemspec has been activated. This attribute is not persisted.
  def activated; end

  # True when this gemspec has been activated. This attribute is not persisted.
  def activated=(_); end

  # True when this gemspec has been activated. This attribute is not persisted.
  def activated?; end

  # Returns an array with bindir attached to each executable in the
  # `executables` list
  def add_bindir(executables); end

  # Alias for:
  # [`add_runtime_dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-add_runtime_dependency)
  def add_dependency(gem, *requirements); end

  # Adds a development dependency named `gem` with `requirements` to this gem.
  #
  # Usage:
  #
  # ```ruby
  # spec.add_development_dependency 'example', '~> 1.1', '>= 1.1.4'
  # ```
  #
  # Development dependencies aren't installed by default and aren't activated
  # when a gem is required.
  def add_development_dependency(gem, *requirements); end

  # Adds a runtime dependency named `gem` with `requirements` to this gem.
  #
  # Usage:
  #
  # ```ruby
  # spec.add_runtime_dependency 'example', '~> 1.1', '>= 1.1.4'
  # ```
  #
  #
  # Also aliased as:
  # [`add_dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-add_dependency)
  def add_runtime_dependency(gem, *requirements); end

  # Adds this spec's require paths to LOAD\_PATH, in the proper location.
  def add_self_to_load_path; end

  # Singular reader for
  # [`authors`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-authors).
  # Returns the first author in the list
  def author; end

  # Singular (alternative) writer for
  # [`authors`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-authors)
  #
  # Usage:
  #
  # ```ruby
  # spec.author = 'John Jones'
  # ```
  def author=(o); end

  # The list of author names who wrote this gem.
  #
  # ```ruby
  # spec.authors = ['Chad Fowler', 'Jim Weirich', 'Rich Kilmer']
  # ```
  def authors; end

  # A list of authors for this gem.
  #
  # Alternatively, a single author can be specified by assigning a string to
  # `spec.author`
  #
  # Usage:
  #
  # ```ruby
  # spec.authors = ['John Jones', 'Mary Smith']
  # ```
  def authors=(value); end

  def autorequire; end

  def autorequire=(_); end

  def base_dir; end

  # Returns the full path to installed gem's bin directory.
  #
  # NOTE: do not confuse this with `bindir`, which is just 'bin', not a full
  # path.
  def bin_dir; end

  # Returns the full path to an executable named `name` in this gem.
  def bin_file(name); end

  # The path in the gem for executable scripts. Usually 'bin'
  #
  # Usage:
  #
  # ```ruby
  # spec.bindir = 'bin'
  # ```
  def bindir; end

  # The path in the gem for executable scripts. Usually 'bin'
  #
  # Usage:
  #
  # ```ruby
  # spec.bindir = 'bin'
  # ```
  def bindir=(_); end

  # Returns the
  # [`build_args`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-build_args)
  # used to install the gem
  def build_args; end

  def build_extensions; end

  # Returns the full path to the build info directory
  def build_info_dir; end

  # Returns the full path to the file containing the build information generated
  # when the gem was installed
  def build_info_file; end

  # Returns the full path to the cache directory containing this spec's cached
  # gem.
  def cache_dir; end

  # Returns the full path to the cached gem for this spec.
  def cache_file; end

  # The certificate chain used to sign this gem. See
  # [`Gem::Security`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html) for
  # details.
  def cert_chain; end

  # The certificate chain used to sign this gem. See
  # [`Gem::Security`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html) for
  # details.
  def cert_chain=(_); end

  def conficts_when_loaded_with?(list_of_specs); end

  # Return any possible conflicts against the currently loaded specs.
  def conflicts; end

  # The date this gem was created.
  #
  # If SOURCE\_DATE\_EPOCH is set as an environment variable, use that to
  # support reproducible builds; otherwise, default to the current UTC date.
  #
  # Details on SOURCE\_DATE\_EPOCH:
  # https://reproducible-builds.org/specs/source-date-epoch/
  def date; end

  # The date this gem was created
  #
  # DO NOT set this, it is set automatically when the gem is packaged.
  def date=(date); end

  # Sets the default executable for this gem.
  #
  # Deprecated: You must now specify the executable name to
  # [`Gem.bin_path`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-bin_path).
  def default_executable(*args, &block); end

  # Sets the default executable for this gem.
  #
  # Deprecated: You must now specify the executable name to
  # [`Gem.bin_path`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-bin_path).
  def default_executable=(*args, &block); end

  # The default value for specification attribute `name`
  def default_value(name); end

  # A list of
  # [`Gem::Dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Dependency.html)
  # objects this gem depends on.
  #
  # Use
  # [`add_dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-add_dependency)
  # or
  # [`add_development_dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-add_development_dependency)
  # to add dependencies to a gem.
  def dependencies; end

  # Return a list of all gems that have a dependency on this gemspec. The list
  # is structured with entries that conform to:
  #
  # ```ruby
  # [depending_gem, dependency, [list_of_gems_that_satisfy_dependency]]
  # ```
  def dependent_gems; end

  # Returns all specs that matches this spec's runtime dependencies.
  def dependent_specs; end

  # A long description of this gem
  #
  # The description should be more detailed than the summary but not excessively
  # long. A few paragraphs is a recommended length with no examples or
  # formatting.
  #
  # Usage:
  #
  # ```ruby
  # spec.description = <<-EOF
  #   Rake is a Make-like program implemented in Ruby. Tasks and
  #   dependencies are specified in standard Ruby syntax.
  # EOF
  # ```
  def description; end

  # A detailed description of this gem. See also
  # [`summary`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#attribute-i-summary)
  def description=(str); end

  # List of dependencies that are used for development
  def development_dependencies; end

  # Returns the full path to this spec's documentation directory. If `type` is
  # given it will be appended to the end. For example:
  #
  # ```ruby
  # spec.doc_dir      # => "/path/to/gem_repo/doc/a-1"
  #
  # spec.doc_dir 'ri' # => "/path/to/gem_repo/doc/a-1/ri"
  # ```
  def doc_dir(type = _); end

  # A contact email address (or addresses) for this gem
  #
  # Usage:
  #
  # ```ruby
  # spec.email = 'john.jones@example.com'
  # spec.email = ['jack@example.com', 'jill@example.com']
  # ```
  def email; end

  # A contact email address (or addresses) for this gem
  #
  # Usage:
  #
  # ```ruby
  # spec.email = 'john.jones@example.com'
  # spec.email = ['jack@example.com', 'jill@example.com']
  # ```
  def email=(_); end

  def encode_with(coder); end

  def eql?(other); end

  # Singular accessor for
  # [`executables`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-executables)
  def executable; end

  # Singular accessor for
  # [`executables`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-executables)
  def executable=(o); end

  # Executables included in the gem.
  #
  # For example, the rake gem has rake as an executable. You don’t specify the
  # full path (as in bin/rake); all application-style files are expected to be
  # found in bindir. These files must be executable Ruby files. Files that use
  # bash or other interpreters will not work.
  #
  # Executables included may only be ruby scripts, not scripts for other
  # languages or compiled binaries.
  #
  # Usage:
  #
  # ```ruby
  # spec.executables << 'rake'
  # ```
  def executables; end

  # Sets executables to `value`, ensuring it is an array. Don't use this, push
  # onto the array instead.
  def executables=(value); end

  def extension_dir; end

  # Extensions to build when installing the gem, specifically the paths to
  # extconf.rb-style files used to compile extensions.
  #
  # These files will be run when the gem is installed, causing the C (or
  # whatever) code to be compiled on the user’s machine.
  #
  # Usage:
  #
  # ```ruby
  # spec.extensions << 'ext/rmagic/extconf.rb'
  # ```
  #
  # See
  # [`Gem::Ext::Builder`](https://docs.ruby-lang.org/en/2.6.0/Gem/Ext/Builder.html)
  # for information about writing extensions for gems.
  def extensions; end

  # Sets extensions to `extensions`, ensuring it is an array. Don't use this,
  # push onto the array instead.
  def extensions=(extensions); end

  # Extra files to add to
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) such as
  # [README](https://docs.ruby-lang.org/en/2.6.0/README_md.html) or
  # doc/examples.txt
  #
  # When the user elects to generate the
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) documentation for a
  # gem (typically at install time), all the library files are sent to
  # [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html) for processing. This
  # option allows you to have some non-code files included for a more complete
  # set of documentation.
  #
  # Usage:
  #
  # ```ruby
  # spec.extra_rdoc_files = ['README', 'doc/user-guide.txt']
  # ```
  def extra_rdoc_files; end

  # Sets
  # [`extra_rdoc_files`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-extra_rdoc_files)
  # to `files`, ensuring it is an array. Don't use this, push onto the array
  # instead.
  def extra_rdoc_files=(files); end

  # The default (generated) file name of the gem. See also
  # [`spec_name`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-spec_name).
  #
  # ```ruby
  # spec.file_name # => "example-1.0.gem"
  # ```
  def file_name; end

  # Files included in this gem. You cannot append to this accessor, you must
  # assign to it.
  #
  # Only add files you can require to this list, not directories, etc.
  #
  # Directories are automatically stripped from this list when building a gem,
  # other non-files cause an error.
  #
  # Usage:
  #
  # ```ruby
  # require 'rake'
  # spec.files = FileList['lib     .rb',
  #                       'bin/*',
  #                       '[A-Z]*',
  #                       'test/   *'].to_a
  #
  # # or without Rake...
  # spec.files = Dir['lib/   *.rb'] + Dir['bin/*']
  # spec.files += Dir['[A-Z]*'] + Dir['test/**/*']
  # spec.files.reject! { |fn| fn.include? "CVS" }
  # ```
  def files; end

  # Sets files to `files`, ensuring it is an array.
  def files=(files); end

  # Creates a duplicate spec without large blobs that aren't used at runtime.
  def for_cache; end

  def full_gem_path; end

  def full_name; end

  def gem_dir; end

  def gems_dir; end

  def git_version; end

  def groups; end

  # Return true if there are possible conflicts against the currently loaded
  # specs.
  def has_conflicts?; end

  def has_rdoc(*args, &block); end

  def has_rdoc=(*args, &block); end

  def has_rdoc?(*args, &block); end

  def has_test_suite?; end

  def has_unit_tests?; end

  def hash; end

  # The URL of this gem's home page
  #
  # Usage:
  #
  # ```ruby
  # spec.homepage = 'https://github.com/ruby/rake'
  # ```
  def homepage; end

  # The URL of this gem's home page
  #
  # Usage:
  #
  # ```ruby
  # spec.homepage = 'https://github.com/ruby/rake'
  # ```
  def homepage=(_); end

  def init_with(coder); end

  def inspect; end

  def installed_by_version; end

  def installed_by_version=(version); end

  def internal_init; end

  def keep_only_files_and_directories; end

  # Files in the [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html) under one
  # of the require\_paths
  def lib_files; end

  # Singular accessor for
  # [`licenses`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-licenses)
  def license; end

  # The license for this gem.
  #
  # The license must be no more than 64 characters.
  #
  # This should just be the name of your license. The full text of the license
  # should be inside of the gem (at the top level) when you build it.
  #
  # The simplest way, is to specify the standard SPDX ID
  # https://spdx.org/licenses/ for the license. Ideally you should pick one that
  # is OSI (Open Source Initiative) http://opensource.org/licenses/alphabetical
  # approved.
  #
  # The most commonly used OSI approved licenses are MIT and Apache-2.0. GitHub
  # also provides a license picker at http://choosealicense.com/.
  #
  # You should specify a license for your gem so that people know how they are
  # permitted to use it, and any restrictions you're placing on it. Not
  # specifying a license means all rights are reserved; others have no rights to
  # use the code for any purpose.
  #
  # You can set multiple licenses with
  # [`licenses=`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-licenses-3D)
  #
  # Usage:
  #
  # ```ruby
  # spec.license = 'MIT'
  # ```
  def license=(o); end

  # Plural accessor for setting licenses
  #
  # See
  # [`license=`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-license-3D)
  # for details
  def licenses; end

  # The license(s) for the library.
  #
  # Each license must be a short name, no more than 64 characters.
  #
  # This should just be the name of your license. The full text of the license
  # should be inside of the gem when you build it.
  #
  # See
  # [`license=`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-license-3D)
  # for more discussion
  #
  # Usage:
  #
  # ```ruby
  # spec.licenses = ['MIT', 'GPL-2.0']
  # ```
  def licenses=(licenses); end

  def load_paths; end

  # Also aliased as:
  # [`rg_loaded_from`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-rg_loaded_from)
  def loaded_from; end

  def location; end

  def location=(_); end

  # Sets the
  # [`rubygems_version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#attribute-i-rubygems_version)
  # to the current RubyGems version.
  def mark_version; end

  # The metadata holds extra data for this gem that may be useful to other
  # consumers and is settable by gem authors without requiring an update to the
  # rubygems software.
  #
  # Metadata items have the following restrictions:
  #
  # *   The metadata must be a
  #     [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) object
  # *   All keys and values must be Strings
  # *   Keys can be a maximum of 128 bytes and values can be a maximum of 1024
  #     bytes
  # *   All strings must be UTF-8, no binary data is allowed
  #
  #
  # You can use metadata to specify links to your gem's homepage, codebase,
  # documentation, wiki, mailing list, issue tracker and changelog.
  #
  # ```ruby
  # s.metadata = {
  #   "bug_tracker_uri"   => "https://example.com/user/bestgemever/issues",
  #   "changelog_uri"     => "https://example.com/user/bestgemever/CHANGELOG.md",
  #   "documentation_uri" => "https://www.example.info/gems/bestgemever/0.0.1",
  #   "homepage_uri"      => "https://bestgemever.example.io",
  #   "mailing_list_uri"  => "https://groups.example.com/bestgemever",
  #   "source_code_uri"   => "https://example.com/user/bestgemever",
  #   "wiki_uri"          => "https://example.com/user/bestgemever/wiki"
  # }
  # ```
  #
  # These links will be used on your gem's page on rubygems.org and must pass
  # validation against following regex.
  #
  # ```ruby
  # %r{\Ahttps?:\/\/([^\s:@]+:[^\s:@]*@)?[A-Za-z\d\-]+(\.[A-Za-z\d\-]+)+\.?(:\d{1,5})?([\/?]\S*)?\z}
  # ```
  def metadata; end

  # The metadata holds extra data for this gem that may be useful to other
  # consumers and is settable by gem authors without requiring an update to the
  # rubygems software.
  #
  # Metadata items have the following restrictions:
  #
  # *   The metadata must be a
  #     [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html) object
  # *   All keys and values must be Strings
  # *   Keys can be a maximum of 128 bytes and values can be a maximum of 1024
  #     bytes
  # *   All strings must be UTF-8, no binary data is allowed
  #
  #
  # You can use metadata to specify links to your gem's homepage, codebase,
  # documentation, wiki, mailing list, issue tracker and changelog.
  #
  # ```ruby
  # s.metadata = {
  #   "bug_tracker_uri"   => "https://example.com/user/bestgemever/issues",
  #   "changelog_uri"     => "https://example.com/user/bestgemever/CHANGELOG.md",
  #   "documentation_uri" => "https://www.example.info/gems/bestgemever/0.0.1",
  #   "homepage_uri"      => "https://bestgemever.example.io",
  #   "mailing_list_uri"  => "https://groups.example.com/bestgemever",
  #   "source_code_uri"   => "https://example.com/user/bestgemever",
  #   "wiki_uri"          => "https://example.com/user/bestgemever/wiki"
  # }
  # ```
  #
  # These links will be used on your gem's page on rubygems.org and must pass
  # validation against following regex.
  #
  # ```ruby
  # %r{\Ahttps?:\/\/([^\s:@]+:[^\s:@]*@)?[A-Za-z\d\-]+(\.[A-Za-z\d\-]+)+\.?(:\d{1,5})?([\/?]\S*)?\z}
  # ```
  def metadata=(_); end

  def method_missing(sym, *a, &b); end

  # Is this specification missing its extensions?  When this returns true you
  # probably want to build\_extensions
  def missing_extensions?; end

  # This gem's name.
  #
  # Usage:
  #
  # ```ruby
  # spec.name = 'rake'
  # ```
  def name; end

  # This gem's name.
  #
  # Usage:
  #
  # ```ruby
  # spec.name = 'rake'
  # ```
  def name=(_); end

  # Return a NameTuple that represents this
  # [`Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  def name_tuple; end

  def nondevelopment_dependencies; end

  # Normalize the list of files so that:
  # *   All file lists have redundancies removed.
  # *   Files referenced in the
  #     [`extra_rdoc_files`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-extra_rdoc_files)
  #     are included in the package file list.
  def normalize; end

  def original_name; end

  def original_platform; end

  def original_platform=(_); end

  # The platform this gem runs on. See
  # [`Gem::Platform`](https://docs.ruby-lang.org/en/2.6.0/Gem/Platform.html) for
  # details.
  def platform; end

  # The platform this gem runs on.
  #
  # This is usually Gem::Platform::RUBY or Gem::Platform::CURRENT.
  #
  # Most gems contain pure Ruby code; they should simply leave the default value
  # in place. Some gems contain C (or other) code to be compiled into a Ruby
  # "extension". The gem should leave the default value in place unless the code
  # will only compile on a certain type of system. Some gems consist of
  # pre-compiled code ("binary gems"). It's especially important that they set
  # the platform attribute appropriately. A shortcut is to set the platform to
  # Gem::Platform::CURRENT, which will cause the gem builder to set the platform
  # to the appropriate value for the system on which the build is being
  # performed.
  #
  # If this attribute is set to a non-default value, it will be included in the
  # filename of the gem when it is built such as: nokogiri-1.6.0-x86-mingw32.gem
  #
  # Usage:
  #
  # ```ruby
  # spec.platform = Gem::Platform.local
  # ```
  def platform=(platform); end

  # A message that gets displayed after the gem is installed.
  #
  # Usage:
  #
  # ```ruby
  # spec.post_install_message = "Thanks for installing!"
  # ```
  def post_install_message; end

  # A message that gets displayed after the gem is installed.
  #
  # Usage:
  #
  # ```ruby
  # spec.post_install_message = "Thanks for installing!"
  # ```
  def post_install_message=(_); end

  def pretty_print(q); end

  def raise_if_conflicts; end

  def raw_require_paths; end

  # Specifies the rdoc options to be used when generating API documentation.
  #
  # Usage:
  #
  # ```ruby
  # spec.rdoc_options << '--title' << 'Rake -- Ruby Make' <<
  #   '--main' << 'README' <<
  #   '--line-numbers'
  # ```
  def rdoc_options; end

  # Sets
  # [`rdoc_options`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-rdoc_options)
  # to `value`, ensuring it is an array. Don't use this, push onto the array
  # instead.
  def rdoc_options=(options); end

  def relative_loaded_from; end

  def relative_loaded_from=(_); end

  def remote; end

  def remote=(_); end

  # Singular accessor for
  # [`require_paths`](https://docs.ruby-lang.org/en/2.6.0/Gem/BasicSpecification.html#method-i-require_paths)
  def require_path; end

  # Singular accessor for
  # [`require_paths`](https://docs.ruby-lang.org/en/2.6.0/Gem/BasicSpecification.html#method-i-require_paths)
  def require_path=(path); end

  # Paths in the gem to add to `$LOAD_PATH` when this gem is activated. If you
  # have an extension you do not need to add `"ext"` to the require path, the
  # extension build process will copy the extension files into "lib" for you.
  #
  # The default value is `"lib"`
  #
  # Usage:
  #
  # ```ruby
  # # If all library files are in the root directory...
  # spec.require_paths = ['.']
  # ```
  def require_paths=(val); end

  # The version of Ruby required by this gem
  def required_ruby_version; end

  # The version of Ruby required by this gem. The ruby version can be specified
  # to the patch-level:
  #
  # ```
  # $ ruby -v -e 'p Gem.ruby_version'
  # ruby 2.0.0p247 (2013-06-27 revision 41674) [x86_64-darwin12.4.0]
  # #<Gem::Version "2.0.0.247">
  # ```
  #
  # Prereleases can also be specified.
  #
  # Usage:
  #
  # ```ruby
  # # This gem will work with 1.8.6 or greater...
  # spec.required_ruby_version = '>= 1.8.6'
  #
  # # Only with ruby 2.0.x
  # spec.required_ruby_version = '~> 2.0'
  #
  # # Only prereleases or final releases after 2.6.0.preview2
  # spec.required_ruby_version = '> 2.6.0.preview2'
  # ```
  def required_ruby_version=(req); end

  # The RubyGems version required by this gem
  def required_rubygems_version; end

  # The RubyGems version required by this gem
  def required_rubygems_version=(req); end

  # Lists the external (to RubyGems) requirements that must be met for this gem
  # to work. It's simply information for the user.
  #
  # Usage:
  #
  # ```ruby
  # spec.requirements << 'libmagick, v6.0'
  # spec.requirements << 'A good graphics card'
  # ```
  def requirements; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) requirements to `req`,
  # ensuring it is an array. Don't use this, push onto the array instead.
  def requirements=(req); end

  # Reset nil attributes to their default values to make the spec valid
  def reset_nil_attributes_to_default; end

  def rg_extension_dir; end

  def rg_full_gem_path; end

  # Alias for:
  # [`loaded_from`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-loaded_from)
  def rg_loaded_from; end

  # Returns the full path to this spec's ri directory.
  def ri_dir; end

  # Deprecated and ignored.
  #
  # Formerly used to set rubyforge project.
  def rubyforge_project=(_); end

  # The version of RubyGems used to create this gem.
  #
  # Do not set this, it is set automatically when the gem is packaged.
  def rubygems_version; end

  # The version of RubyGems used to create this gem.
  #
  # Do not set this, it is set automatically when the gem is packaged.
  def rubygems_version=(_); end

  # List of dependencies that will automatically be activated at runtime.
  def runtime_dependencies; end

  # Sanitize the descriptive fields in the spec. Sometimes non-ASCII characters
  # will garble the site index. Non-ASCII characters will be replaced by their
  # [`XML`](https://docs.ruby-lang.org/en/2.6.0/XML.html) entity equivalent.
  def sanitize; end

  # Sanitize a single string.
  def sanitize_string(string); end

  # Checks if this specification meets the requirement of `dependency`.
  def satisfies_requirement?(dependency); end

  # The key used to sign this gem. See
  # [`Gem::Security`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html) for
  # details.
  def signing_key; end

  # The key used to sign this gem. See
  # [`Gem::Security`](https://docs.ruby-lang.org/en/2.6.0/Gem/Security.html) for
  # details.
  def signing_key=(_); end

  # Returns an object you can use to sort specifications in sort\_by.
  def sort_obj; end

  def source; end

  def source=(_); end

  # Returns the full path to the directory containing this spec's gemspec file.
  # eg: /usr/local/lib/ruby/gems/1.8/specifications
  def spec_dir; end

  # Returns the full path to this spec's gemspec file. eg:
  # /usr/local/lib/ruby/gems/1.8/specifications/mygem-1.0.gemspec
  def spec_file; end

  # The default name of the gemspec. See also
  # [`file_name`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-file_name)
  #
  # ```ruby
  # spec.spec_name # => "example-1.0.gemspec"
  # ```
  def spec_name; end

  # The
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # version of this gemspec.
  #
  # Do not set this, it is set automatically when the gem is packaged.
  def specification_version; end

  # The
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # version of this gemspec.
  #
  # Do not set this, it is set automatically when the gem is packaged.
  def specification_version=(_); end

  def stubbed?; end

  # A short summary of this gem's description. Displayed in `gem list -d`.
  #
  # The
  # [`description`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#attribute-i-description)
  # should be more detailed than the summary.
  #
  # Usage:
  #
  # ```ruby
  # spec.summary = "This is a small summary of my gem"
  # ```
  def summary; end

  # A short summary of this gem's description.
  def summary=(str); end

  def test_file; end

  def test_file=(file); end

  def test_files; end

  def test_files=(files); end

  def to_gemfile(path = _); end

  # Returns a Ruby code representation of this specification, such that it can
  # be eval'ed and reconstruct the same specification later. Attributes that
  # still have their default values are omitted.
  def to_ruby; end

  # Returns a Ruby lighter-weight code representation of this specification,
  # used for indexing only.
  #
  # See
  # [`to_ruby`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-i-to_ruby).
  def to_ruby_for_cache; end

  def to_s; end

  # Returns self
  def to_spec; end

  def to_yaml(opts = _); end

  # Recursively walk dependencies of this spec, executing the `block` for each
  # hop.
  def traverse(trail = _, visited = _, &block); end

  # Checks that the specification contains all required fields, and does a very
  # basic sanity check.
  #
  # Raises InvalidSpecificationException if the spec does not pass the checks..
  def validate(packaging = _, strict = _); end

  # Checks that dependencies use requirements as we recommend. Warnings are
  # issued when dependencies are open-ended or overly strict for semantic
  # versioning.
  def validate_dependencies; end

  def validate_metadata; end

  # Checks to see if the files to be packaged are world-readable.
  def validate_permissions; end

  # This gem's version.
  #
  # The version string can contain numbers and periods, such as `1.0.0`. A gem
  # is a 'prerelease' gem if the version has a letter in it, such as
  # `1.0.0.pre`.
  #
  # Usage:
  #
  # ```ruby
  # spec.version = '0.4.1'
  # ```
  def version; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the version to
  # `version`, potentially also setting
  # [`required_rubygems_version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#attribute-i-required_rubygems_version)
  # if `version` indicates it is a prerelease.
  def version=(version); end

  def yaml_initialize(tag, vals); end

  def self._all; end

  def self._clear_load_cache; end

  def self._latest_specs(specs, prerelease = _); end

  # Load custom marshal format, re-initializing defaults as needed
  def self._load(str); end

  def self._resort!(specs); end

  # Adds `spec` to the known specifications, keeping the collection properly
  # sorted.
  def self.add_spec(spec); end

  # Adds multiple specs to the known specifications.
  def self.add_specs(*specs); end

  # Returns all specifications. This method is discouraged from use. You
  # probably want to use one of the
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html) methods
  # instead.
  def self.all; end

  # Sets the known specs to `specs`. Not guaranteed to work for you in the
  # future. Use at your own risk. Caveat emptor. Doomy doom doom.
  # [`Etc`](https://docs.ruby-lang.org/en/2.6.0/Etc.html) etc.
  def self.all=(specs); end

  # Return full names of all specs in sorted order.
  def self.all_names; end

  # Return the list of all array-oriented instance variables.
  def self.array_attributes; end

  # Return the list of all instance variables.
  def self.attribute_names; end

  # Return the directories that
  # [`Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # uses to find specs.
  def self.dirs; end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the directories that
  # [`Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # uses to find specs. Setting this resets the list of known specs.
  def self.dirs=(dirs); end

  # Enumerate every known spec. See
  # [`::dirs=`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-c-dirs-3D)
  # and
  # [`::add_spec`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-c-add_spec)
  # to set the list of specs.
  def self.each; end

  def self.each_gemspec(dirs); end

  def self.each_spec(dirs); end

  def self.find_active_stub_by_path(path); end

  # Returns every spec that has the given `full_name`
  def self.find_all_by_full_name(full_name); end

  # Returns every spec that matches `name` and optional `requirements`.
  def self.find_all_by_name(name, *requirements); end

  # [`Find`](https://docs.ruby-lang.org/en/2.6.0/Find.html) the best
  # specification matching a `name` and `requirements`. Raises if the dependency
  # doesn't resolve to a valid specification.
  def self.find_by_name(name, *requirements); end

  # Return the best specification that contains the file matching `path`.
  def self.find_by_path(path); end

  # Return currently unresolved specs that contain the file matching `path`.
  def self.find_in_unresolved(path); end

  # Search through all unresolved deps and sub-dependencies and return specs
  # that contain the file matching `path`.
  def self.find_in_unresolved_tree(path); end

  # Return the best specification that contains the file matching `path` amongst
  # the specs that are not activated.
  def self.find_inactive_by_path(path); end

  # Special loader for [`YAML`](https://docs.ruby-lang.org/en/2.6.0/YAML.html)
  # files. When a
  # [`Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # object is loaded from a
  # [`YAML`](https://docs.ruby-lang.org/en/2.6.0/YAML.html) file, it bypasses
  # the normal Ruby object initialization routine (#initialize). This method
  # makes up for that and deals with gems of different ages.
  #
  # `input` can be anything that YAML.load() accepts:
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) or
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html).
  def self.from_yaml(input); end

  # Return the latest specs, optionally including prerelease specs if
  # `prerelease` is true.
  def self.latest_specs(prerelease = _); end

  # Loads Ruby format gemspec from `file`.
  def self.load(file); end

  # Loads the default specifications. It should be called only once.
  def self.load_defaults; end

  # [`Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # attributes that must be non-nil
  def self.non_nil_attributes; end

  # Make sure the [`YAML`](https://docs.ruby-lang.org/en/2.6.0/YAML.html)
  # specification is properly formatted with dashes
  def self.normalize_yaml_input(input); end

  # Return a list of all outdated local gem names. This method is HEAVY as it
  # must go fetch specifications from the server.
  #
  # Use
  # [`outdated_and_latest_version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html#method-c-outdated_and_latest_version)
  # if you wish to retrieve the latest remote version as well.
  def self.outdated; end

  # Enumerates the outdated local gems yielding the local specification and the
  # latest remote version.
  #
  # This method may take some time to return as it must check each local gem
  # against the server's index.
  def self.outdated_and_latest_version; end

  # Removes `spec` from the known specs.
  def self.remove_spec(spec); end

  # Is `name` a required attribute?
  def self.required_attribute?(name); end

  # Required specification attributes
  def self.required_attributes; end

  # Reset the list of known specs, running pre and post reset hooks registered
  # in [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html).
  def self.reset; end

  # Returns a
  # [`Gem::StubSpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/StubSpecification.html)
  # for every installed gem
  def self.stubs; end

  # Returns a
  # [`Gem::StubSpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/StubSpecification.html)
  # for installed gem named `name` only returns stubs that match
  # [`Gem.platforms`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-platforms)
  def self.stubs_for(name); end

  # DOC: This method needs documented or nodoc'd
  def self.unresolved_deps; end
end

class Gem::SpecificationPolicy < ::SimpleDelegator
  HOMEPAGE_URI_PATTERN = T.let(T.unsafe(nil), Regexp)

  LAZY = T.let(T.unsafe(nil), String)

  LAZY_PATTERN = T.let(T.unsafe(nil), Regexp)

  METADATA_LINK_KEYS = T.let(T.unsafe(nil), T::Array[T.untyped])

  SPECIAL_CHARACTERS = T.let(T.unsafe(nil), Regexp)

  VALID_NAME_PATTERN = T.let(T.unsafe(nil), Regexp)

  VALID_URI_PATTERN = T.let(T.unsafe(nil), Regexp)

  def self.new(specification); end

  # If set to true, run packaging-specific checks, as well.
  def packaging; end

  # If set to true, run packaging-specific checks, as well.
  def packaging=(_); end

  # Checks that the specification contains all required fields, and does a very
  # basic sanity check.
  #
  # Raises InvalidSpecificationException if the spec does not pass the checks.
  def validate(strict = _); end

  def validate_dependencies; end

  # Implementation for Specification#validate\_metadata
  def validate_metadata; end

  # Issues a warning for each file to be packaged which is world-readable.
  #
  # Implementation for Specification#validate\_permissions
  def validate_permissions; end
end

# [`Gem::StreamUI`](https://docs.ruby-lang.org/en/2.6.0/Gem/StreamUI.html)
# implements a simple stream based user interface.
class Gem::StreamUI
  extend(::Gem::Deprecate)

  # Creates a new
  # [`StreamUI`](https://docs.ruby-lang.org/en/2.6.0/Gem/StreamUI.html) wrapping
  # `in_stream` for user input, `out_stream` for standard output, `err_stream`
  # for error output. If `usetty` is true then special operations (like asking
  # for passwords) will use the TTY commands to disable character echo.
  def self.new(in_stream, out_stream, err_stream = _, usetty = _); end

  def _deprecated_debug(statement); end

  def _gets_noecho; end

  # Display an informational alert. Will ask `question` if it is not nil.
  def alert(statement, question = _); end

  # Display an error message in a location expected to get error messages. Will
  # ask `question` if it is not nil.
  def alert_error(statement, question = _); end

  # Display a warning on stderr. Will ask `question` if it is not nil.
  def alert_warning(statement, question = _); end

  # Ask a question. Returns an answer if connected to a tty, nil otherwise.
  def ask(question); end

  # Ask for a password. Does not echo response to terminal.
  def ask_for_password(question); end

  # Ask a question. Returns a true for yes, false for no. If not connected to a
  # tty, raises an exception if default is nil, otherwise returns default.
  def ask_yes_no(question, default = _); end

  # Prints a formatted backtrace to the errors stream if backtraces are enabled.
  def backtrace(exception); end

  # Choose from a list of options. `question` is a prompt displayed above the
  # list. `list` is a list of option strings. Returns the pair [option\_name,
  # option\_index].
  def choose_from_list(question, list); end

  def close; end

  # Display a debug message on the same location as error messages.
  def debug(*args, &block); end

  # Return a download reporter object chosen from the current verbosity
  def download_reporter(*args); end

  # The error stream
  def errs; end

  # The input stream
  def ins; end

  # The output stream
  def outs; end

  # Return a progress reporter object chosen from the current verbosity.
  def progress_reporter(*args); end

  def require_io_console; end

  # Display a statement.
  def say(statement = _); end

  # Terminate the application with exit code `status`, running any exit handlers
  # that might have been defined.
  def terminate_interaction(status = _); end

  # Returns true if TTY methods should be used on this
  # [`StreamUI`](https://docs.ruby-lang.org/en/2.6.0/Gem/StreamUI.html).
  def tty?; end
end

# An absolutely silent download reporter.
class Gem::StreamUI::SilentDownloadReporter
  # The silent download reporter ignores all arguments
  def self.new(out_stream, *args); end

  # The silent download reporter won't tell you when the download is done.
  # Because it is silent.
  def done; end

  # The silent download reporter does not display `filename` or care about
  # `filesize` because it is silent.
  def fetch(filename, filesize); end

  # Nothing can update the silent download reporter.
  def update(current); end
end

# An absolutely silent progress reporter.
# An absolutely silent progress reporter.
class Gem::StreamUI::SilentProgressReporter
  # Creates a silent progress reporter that ignores all input arguments.
  def self.new(out_stream, size, initial_message, terminal_message = _); end

  # The count of items is never updated for the silent progress reporter.
  def count; end

  # Does not print anything when complete as this object has taken a vow of
  # silence.
  def done; end

  # Does not print `message` when updated as this object has taken a vow of
  # silence.
  def updated(message); end
end

# A basic dotted progress reporter.
class Gem::StreamUI::SimpleProgressReporter
  include(::Gem::DefaultUserInteraction)

  # Creates a new progress reporter that will write to `out_stream` for `size`
  # items. Shows the given `initial_message` when progress starts and the
  # `terminal_message` when it is complete.
  def self.new(out_stream, size, initial_message, terminal_message = _); end

  # The number of progress items counted so far.
  def count; end

  # Prints out the terminal message.
  def done; end

  # Prints out a dot and ignores `message`.
  def updated(message); end
end

# A progress reporter that behaves nicely with threaded downloading.
class Gem::StreamUI::ThreadedDownloadReporter
  MUTEX = T.let(T.unsafe(nil), Thread::Mutex)

  # Creates a new threaded download reporter that will display on `out_stream`.
  # The other arguments are ignored.
  def self.new(out_stream, *args); end

  # Indicates the download is complete.
  def done; end

  # Tells the download reporter that the `file_name` is being fetched. The other
  # arguments are ignored.
  def fetch(file_name, *args); end

  # The current file name being displayed
  def file_name; end

  # Updates the threaded download reporter for the given number of `bytes`.
  def update(bytes); end
end

# A progress reporter that prints out messages about the current progress.
class Gem::StreamUI::VerboseProgressReporter
  include(::Gem::DefaultUserInteraction)

  # Creates a new progress reporter that will write to `out_stream` for `size`
  # items. Shows the given `initial_message` when progress starts and the
  # `terminal_message` when it is complete.
  def self.new(out_stream, size, initial_message, terminal_message = _); end

  # The number of progress items counted so far.
  def count; end

  # Prints out the terminal message.
  def done; end

  # Prints out the position relative to the total and the `message`.
  def updated(message); end
end

# [`Gem::StubSpecification`](https://docs.ruby-lang.org/en/2.6.0/Gem/StubSpecification.html)
# reads the stub: line from the gemspec. This prevents us having to eval the
# entire gemspec in order to find out certain information.
class Gem::StubSpecification < ::Gem::BasicSpecification
  OPEN_MODE = T.let(T.unsafe(nil), String)

  PREFIX = T.let(T.unsafe(nil), String)

  def self.new(filename, base_dir, gems_dir, default_gem); end

  def activated?; end

  def base_dir; end

  def build_extensions; end

  def default_gem?; end

  def extensions; end

  def full_name; end

  def gems_dir; end

  def missing_extensions?; end

  def name; end

  def platform; end

  def raw_require_paths; end

  def stubbed?; end

  def to_spec; end

  def valid?; end

  def version; end

  def self.default_gemspec_stub(filename, base_dir, gems_dir); end

  def self.gemspec_stub(filename, base_dir, gems_dir); end
end

class Gem::StubSpecification::StubLine
  NO_EXTENSIONS = T.let(T.unsafe(nil), T::Array[T.untyped])

  REQUIRE_PATHS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  REQUIRE_PATH_LIST = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])

  def self.new(data, extensions); end

  def extensions; end

  def full_name; end

  def name; end

  def platform; end

  def require_paths; end

  def version; end
end

# Raised to indicate that a system exit should occur with the specified
# [`exit_code`](https://docs.ruby-lang.org/en/2.6.0/Gem/SystemExitException.html#attribute-i-exit_code)
class Gem::SystemExitException < ::SystemExit
  # Creates a new
  # [`SystemExitException`](https://docs.ruby-lang.org/en/2.6.0/Gem/SystemExitException.html)
  # with the given `exit_code`
  def self.new(exit_code); end

  # The exit code for the process
  def exit_code; end

  # The exit code for the process
  def exit_code=(_); end
end

# A collection of text-wrangling methods
module Gem::Text
  # Remove any non-printable characters and make the text suitable for printing.
  def clean_text(text); end

  # Wraps `text` to `wrap` characters and optionally indents by `indent`
  # characters
  def format_text(text, wrap, indent = _); end

  # This code is based directly on the
  # [`Text`](https://docs.ruby-lang.org/en/2.6.0/Gem/Text.html) gem
  # implementation Returns a value representing the "cost" of transforming str1
  # into str2
  def levenshtein_distance(str1, str2); end

  def min3(a, b, c); end

  def truncate_text(text, description, max_length = _); end
end

Gem::USE_BUNDLER_FOR_GEMDEPS = T.let(T.unsafe(nil), TrueClass)

# Raised when removing a gem with the uninstall command fails
class Gem::UninstallError < ::Gem::Exception
  def spec; end

  def spec=(_); end
end

# An [`Uninstaller`](https://docs.ruby-lang.org/en/2.6.0/Gem/Uninstaller.html).
#
# The uninstaller fires pre and post uninstall hooks. Hooks can be added either
# through a rubygems\_plugin.rb file in an installed gem or via a
# rubygems/defaults/#{RUBY\_ENGINE}.rb or rubygems/defaults/operating\_system.rb
# file. See
# [`Gem.pre_uninstall`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-pre_uninstall)
# and
# [`Gem.post_uninstall`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-post_uninstall)
# for details.
class Gem::Uninstaller
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)

  # Constructs an uninstaller that will uninstall `gem`
  def self.new(gem, options = _); end

  def abort_on_dependent?; end

  def ask_if_ok(spec); end

  # The directory a gem's executables will be installed into
  def bin_dir; end

  def dependencies_ok?(spec); end

  def formatted_program_filename(filename); end

  # The gem repository the gem will be installed into
  def gem_home; end

  # Is `spec` in `gem_dir`?
  def path_ok?(gem_dir, spec); end

  # spec
  # :   the spec of the gem to be uninstalled
  # list
  # :   the list of all such gems
  #
  #
  # Warning: this method modifies the `list` parameter. Once it has uninstalled
  # a gem, it is removed from that list.
  def remove(spec); end

  # Removes all gems in `list`.
  #
  # NOTE: removes uninstalled gems from `list`.
  def remove_all(list); end

  # Removes installed executables and batch files (windows only) for `gemspec`.
  def remove_executables(spec); end

  def safe_delete(&block); end

  # The
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.6.0/Gem/Specification.html)
  # for the gem being uninstalled, only set during
  # [`uninstall_gem`](https://docs.ruby-lang.org/en/2.6.0/Gem/Uninstaller.html#method-i-uninstall_gem)
  def spec; end

  # Performs the uninstall of the gem. This removes the spec, the
  # [`Gem`](https://docs.ruby-lang.org/en/2.6.0/Gem.html) directory, and the
  # cached .gem file.
  def uninstall; end

  # Uninstalls gem `spec`
  def uninstall_gem(spec); end
end

# Raised by Resolver when a dependency requests a gem for which there is no
# spec.
class Gem::UnsatisfiableDependencyError < ::Gem::DependencyError
  # Creates a new
  # [`UnsatisfiableDependencyError`](https://docs.ruby-lang.org/en/2.6.0/Gem/UnsatisfiableDependencyError.html)
  # for the unsatisfiable
  # [`Gem::Resolver::DependencyRequest`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/DependencyRequest.html)
  # `dep`
  def self.new(dep, platform_mismatch = _); end

  # The unsatisfiable dependency. This is a
  # [`Gem::Resolver::DependencyRequest`](https://docs.ruby-lang.org/en/2.6.0/Gem/Resolver/DependencyRequest.html),
  # not a
  # [`Gem::Dependency`](https://docs.ruby-lang.org/en/2.6.0/Gem/Dependency.html)
  def dependency; end

  # Errors encountered which may have contributed to this exception
  def errors; end

  # Errors encountered which may have contributed to this exception
  def errors=(_); end

  # The name of the unresolved dependency
  def name; end

  # The Requirement of the unresolved dependency (not Version).
  def version; end
end

# The
# [`UriFormatter`](https://docs.ruby-lang.org/en/2.6.0/Gem/UriFormatter.html)
# handles URIs from user-input and escaping.
#
# ```ruby
# uf = Gem::UriFormatter.new 'example.com'
#
# p uf.normalize #=> 'http://example.com'
# ```
class Gem::UriFormatter
  # Creates a new [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html)
  # formatter for `uri`.
  def self.new(uri); end

  # Escapes the
  # [`uri`](https://docs.ruby-lang.org/en/2.6.0/Gem/UriFormatter.html#attribute-i-uri)
  # for use as a [`CGI`](https://docs.ruby-lang.org/en/2.6.0/CGI.html) parameter
  def escape; end

  # Normalize the [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) by
  # adding "http://" if it is missing.
  def normalize; end

  # Unescapes the
  # [`uri`](https://docs.ruby-lang.org/en/2.6.0/Gem/UriFormatter.html#attribute-i-uri)
  # which came from a [`CGI`](https://docs.ruby-lang.org/en/2.6.0/CGI.html)
  # parameter
  def unescape; end

  # The [`URI`](https://docs.ruby-lang.org/en/2.6.0/URI.html) to be formatted.
  def uri; end
end

# [`UserInteraction`](https://docs.ruby-lang.org/en/2.6.0/Gem/UserInteraction.html)
# allows RubyGems to interact with the user through standard methods that can be
# replaced with more-specific UI methods for different displays.
#
# Since
# [`UserInteraction`](https://docs.ruby-lang.org/en/2.6.0/Gem/UserInteraction.html)
# dispatches to a concrete UI class you may need to reference other classes for
# specific behavior such as
# [`Gem::ConsoleUI`](https://docs.ruby-lang.org/en/2.6.0/Gem/ConsoleUI.html) or
# [`Gem::SilentUI`](https://docs.ruby-lang.org/en/2.6.0/Gem/SilentUI.html).
#
# Example:
#
# ```ruby
# class X
#   include Gem::UserInteraction
#
#   def get_answer
#     n = ask("What is the meaning of life?")
#   end
# end
# ```
module Gem::UserInteraction
  include(::Gem::DefaultUserInteraction)

  # Displays an alert `statement`. Asks a `question` if given.
  def alert(statement, question = _); end

  # Displays an error `statement` to the error output location. Asks a
  # `question` if given.
  def alert_error(statement, question = _); end

  # Displays a warning `statement` to the warning output location. Asks a
  # `question` if given.
  def alert_warning(statement, question = _); end

  # Asks a `question` and returns the answer.
  def ask(question); end

  # Asks for a password with a `prompt`
  def ask_for_password(prompt); end

  # Asks a yes or no `question`. Returns true for yes, false for no.
  def ask_yes_no(question, default = _); end

  # Asks the user to answer `question` with an answer from the given `list`.
  def choose_from_list(question, list); end

  # Displays the given `statement` on the standard output (or equivalent).
  def say(statement = _); end

  # Terminates the RubyGems process with the given `exit_code`
  def terminate_interaction(exit_code = _); end

  # Calls `say` with `msg` or the results of the block if really\_verbose is
  # true.
  def verbose(msg = _); end
end

# This module contains various utility methods as module methods.
module Gem::Util
  # Globs for files matching `pattern` inside of `directory`, returning absolute
  # paths to the matching files.
  def self.glob_files_in_dir(glob, base_path); end

  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.6.0/Zlib/GzipReader.html)
  # wrapper that unzips `data`.
  def self.gunzip(data); end

  # [`Zlib::GzipWriter`](https://docs.ruby-lang.org/en/2.6.0/Zlib/GzipWriter.html)
  # wrapper that zips `data`.
  def self.gzip(data); end

  # A
  # [`Zlib::Inflate#inflate`](https://docs.ruby-lang.org/en/2.6.0/Zlib/Inflate.html#method-i-inflate)
  # wrapper
  def self.inflate(data); end

  # This calls
  # [`IO.popen`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-popen)
  # where it accepts an array for a `command` (Ruby 1.9+) and implements an
  # [`IO.popen`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-popen)-like
  # behavior where it does not accept an array for a command.
  def self.popen(*command); end

  # Invokes system, but silences all output.
  def self.silent_system(*command); end

  # Enumerates the parents of `directory`.
  def self.traverse_parents(directory, &block); end
end

# [`Validator`](https://docs.ruby-lang.org/en/2.6.0/Gem/Validator.html) performs
# various gem file and gem database validation
class Gem::Validator
  include(::Gem::UserInteraction)
  include(::Gem::DefaultUserInteraction)

  def self.new; end

  # Checks the gem directory for the following potential
  # inconsistencies/problems:
  #
  # *   Checksum gem itself
  # *   For each file in each gem, check consistency of installed versions
  # *   Check for files that aren't part of the gem but are in the gems
  #     directory
  # *   1 cache - 1 spec - 1 directory.
  #
  #
  # returns a hash of ErrorData objects, keyed on the problem gem's name.
  def alien(gems = _); end

  # Given a gem file's contents, validates against its own MD5 checksum
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html)
  # :       Contents of the gem file
  def verify_gem(gem_data); end

  # Given the path to a gem file, validates against its own MD5 checksum
  #
  #     [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html)
  # :       Path to gem file
  def verify_gem_file(gem_path); end
end

class Gem::Validator::ErrorData < ::Struct
  Elem = type_member(:out, fixed: T.untyped)

  def <=>(other); end

  def path; end

  def path=(_); end

  def problem; end

  def problem=(_); end

  def self.[](*_); end

  def self.inspect; end

  def self.members; end

  def self.new(*_); end
end

# Raised by
# [`Gem::Validator`](https://docs.ruby-lang.org/en/2.6.0/Gem/Validator.html)
# when something is not right in a gem.
class Gem::VerificationError < ::Gem::Exception; end

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
class Gem::Version
  include(::Comparable)

  ANCHORED_VERSION_PATTERN = T.let(T.unsafe(nil), Regexp)

  Requirement = Gem::Requirement

  VERSION_PATTERN = T.let(T.unsafe(nil), String)

  # Compares this version with `other` returning -1, 0, or 1 if the other
  # version is larger, the same, or smaller than this one. Attempts to compare
  # to something that's not a `Gem::Version` return `nil`.
  def <=>(other); end

  # A recommended version for use with a ~> Requirement.
  def approximate_recommendation; end

  # Return a new version object where the next to the last revision number is
  # one greater (e.g., 5.3.1 => 5.4).
  #
  # Pre-release (alpha) parts, e.g, 5.3.1.b.2 => 5.4, are ignored.
  def bump; end

  def canonical_segments; end

  def encode_with(coder); end

  # A [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) is only
  # eql? to another version if it's specified to the same precision.
  # [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) "1.0" is
  # not the same as version "1".
  def eql?(other); end

  def hash; end

  def init_with(coder); end

  def inspect; end

  # Dump only the raw version string, not the complete object. It's a string for
  # backwards (RubyGems 1.3.5 and earlier) compatibility.
  def marshal_dump; end

  # Load custom marshal format. It's a string for backwards (RubyGems 1.3.5 and
  # earlier) compatibility.
  def marshal_load(array); end

  # A version is considered a prerelease if it contains a letter.
  def prerelease?; end

  def pretty_print(q); end

  # The release for this version (e.g. 1.2.0.a -> 1.2.0). Non-prerelease
  # versions return themselves.
  def release; end

  def segments; end

  # Alias for:
  # [`version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html#method-i-version)
  def to_s; end

  def to_yaml_properties; end

  # A string representation of this
  # [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html).
  #
  # Also aliased as:
  # [`to_s`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html#method-i-to_s)
  def version; end

  def yaml_initialize(tag, map); end

  protected

  def _segments; end

  def _split_segments; end

  def _version; end

  # True if the `version` string matches RubyGems' requirements.
  def self.correct?(version); end

  # Factory method to create a
  # [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) object.
  # Input may be a
  # [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) or a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html). Intended to
  # simplify client code.
  #
  # ```ruby
  # ver1 = Version.create('1.3.17')   # -> (Version object)
  # ver2 = Version.create(ver1)       # -> (ver1)
  # ver3 = Version.create(nil)        # -> nil
  # ```
  def self.create(input); end

  # Constructs a
  # [`Version`](https://docs.ruby-lang.org/en/2.6.0/Gem/Version.html) from the
  # `version` string. A version string is a series of digits or ASCII letters
  # separated by dots.
  def self.new(version); end
end
