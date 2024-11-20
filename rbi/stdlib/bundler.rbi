# typed: __STDLIB_INTERNAL

# [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html) provides a
# consistent environment for Ruby projects by tracking and installing the exact
# gems and versions that are needed.
#
# Since Ruby 2.6, [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html)
# is a part of Ruby's standard library.
#
# Bunder is used by creating *gemfiles* listing all the project dependencies and
# (optionally) their versions and then using
#
# ```ruby
# require 'bundler/setup'
# ```
#
# or
# [`Bundler.setup`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html#method-c-setup)
# to setup environment where only specified gems and their specified versions
# could be used.
#
# See [Bundler website](https://bundler.io/docs.html) for extensive
# documentation on gemfiles creation and
# [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html) usage.
#
# As a standard library inside project,
# [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html) could be used
# for introspection of loaded and required modules.
module Bundler
  FREEBSD = ::T.let(nil, T.untyped)
  NULL = ::T.let(nil, T.untyped)
  ORIGINAL_ENV = ::T.let(nil, T.untyped)
  SUDO_MUTEX = ::T.let(nil, T.untyped)
  # We're doing this because we might write tests that deal with other versions
  # of bundler and we are unsure how to handle this better.
  VERSION = ::T.let(nil, T.untyped)
  WINDOWS = ::T.let(nil, T.untyped)

  sig do
    params(
      custom_path: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.app_cache(custom_path=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def self.app_config_path(); end

  # Returns absolute location of where binstubs are installed to.
  sig {returns(T.untyped)}
  def self.bin_path(); end

  # Returns absolute path of where gems are installed on the filesystem.
  sig {returns(T.untyped)}
  def self.bundle_path(); end

  sig {returns(T.untyped)}
  def self.bundler_major_version(); end

  # @deprecated Use `unbundled\_env` instead
  sig {returns(T.untyped)}
  def self.clean_env(); end

  # @deprecated Use `unbundled\_exec` instead
  sig do
    params(
      args: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.clean_exec(*args); end

  # @deprecated Use `unbundled\_system` instead
  sig do
    params(
      args: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.clean_system(*args); end

  sig {returns(T.untyped)}
  def self.clear_gemspec_cache(); end

  sig {returns(T.untyped)}
  def self.configure(); end

  sig {returns(T.untyped)}
  def self.configured_bundle_path(); end

  # Returns current version of Ruby
  #
  # @return [CurrentRuby] Current version of Ruby
  sig {returns(T.untyped)}
  def self.current_ruby(); end

  sig {returns(T.untyped)}
  def self.default_bundle_dir(); end

  sig {returns(T.untyped)}
  def self.default_gemfile(); end

  sig {returns(T.untyped)}
  def self.default_lockfile(); end

  # Returns an instance of
  # [`Bundler::Definition`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Definition.html)
  # for given Gemfile and lockfile
  #
  # @param unlock [Hash, Boolean, nil] Gems that have been requested
  #
  # ```ruby
  # to be updated or true if all gems should be updated
  # ```
  #
  # @return [Bundler::Definition]
  sig do
    params(
      unlock: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.definition(unlock=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def self.environment(); end

  sig {returns(T.untyped)}
  def self.feature_flag(); end

  sig {returns(T.untyped)}
  def self.frozen_bundle?(); end

  sig {returns(T.untyped)}
  def self.git_present?(); end

  sig {returns(T.untyped)}
  def self.home(); end

  sig {returns(T.untyped)}
  def self.install_path(); end

  sig {returns(T.untyped)}
  def self.load(); end

  sig do
    params(
      file: T.untyped,
      validate: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.load_gemspec(file, validate=T.unsafe(nil)); end

  sig do
    params(
      file: T.untyped,
      validate: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.load_gemspec_uncached(file, validate=T.unsafe(nil)); end

  sig do
    params(
      data: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.load_marshal(data); end

  sig {returns(T.untyped)}
  def self.local_platform(); end

  sig {returns(T.untyped)}
  def self.locked_gems(); end

  sig do
    params(
      path: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mkdir_p(path, options=T.unsafe(nil)); end

  # @return [Hash] Environment present before
  # [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html) was activated
  sig {returns(T.untyped)}
  def self.original_env(); end

  sig do
    params(
      file: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.read_file(file); end

  # Setups [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html)
  # environment (see
  # [`Bundler.setup`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html#method-c-setup))
  # if it is not already set, and loads all gems from groups specified. Unlike
  # [`::setup`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html#method-c-setup),
  # can be called multiple times with different groups (if they were allowed by
  # setup).
  #
  # Assuming Gemfile
  #
  # ```ruby
  # gem 'first_gem', '= 1.0'
  # group :test do
  #   gem 'second_gem', '= 1.0'
  # end
  # ```
  #
  # The code will work as follows:
  #
  # ```ruby
  # Bundler.setup # allow all groups
  # Bundler.require(:default) # requires only first_gem
  # # ...later
  # Bundler.require(:test)   # requires second_gem
  # ```
  sig do
    params(
      groups: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.require(*groups); end

  sig {returns(T.untyped)}
  def self.require_thor_actions(); end

  sig {returns(T.untyped)}
  def self.requires_sudo?(); end

  sig {returns(T.untyped)}
  def self.reset!(); end

  sig {returns(T.untyped)}
  def self.reset_paths!(); end

  sig {returns(T.untyped)}
  def self.reset_rubygems!(); end

  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm_rf(path); end

  sig {returns(T.untyped)}
  def self.root(); end

  sig {returns(T.untyped)}
  def self.ruby_scope(); end

  sig {returns(T.untyped)}
  def self.rubygems(); end

  sig {returns(T.untyped)}
  def self.settings(); end

  # Turns on the [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html)
  # runtime. After `Bundler.setup` call, all `load` or `require` of the gems
  # would be allowed only if they are part of the Gemfile or Ruby's standard
  # library. If the versions specified in Gemfile, only those versions would be
  # loaded.
  #
  # Assuming Gemfile
  #
  # ```ruby
  # gem 'first_gem', '= 1.0'
  # group :test do
  #   gem 'second_gem', '= 1.0'
  # end
  # ```
  #
  # The code using
  # [`Bundler.setup`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html#method-c-setup)
  # works as follows:
  #
  # ```ruby
  # require 'third_gem' # allowed, required from global gems
  # require 'first_gem' # allowed, loads the last installed version
  # Bundler.setup
  # require 'fourth_gem' # fails with LoadError
  # require 'second_gem' # loads exactly version 1.0
  # ```
  #
  # `Bundler.setup` can be called only once, all subsequent calls are no-op.
  #
  # If *groups* list is provided, only gems from specified groups would be
  # allowed (gems specified outside groups belong to special `:default` group).
  #
  # To require all gems from Gemfile (or only some groups), see
  # [`Bundler.require`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html#method-c-require).
  sig do
    params(
      groups: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.setup(*groups); end

  sig {returns(T.untyped)}
  def self.specs_path(); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.sudo(str); end

  sig {returns(T.untyped)}
  def self.system_bindir(); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.tmp(name=T.unsafe(nil)); end

  sig do
    params(
      login: T.untyped,
      warning: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.tmp_home_path(login, warning); end

  sig {returns(T.untyped)}
  def self.ui(); end

  sig do
    params(
      ui: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.ui=(ui); end

  sig {returns(T.untyped)}
  def self.use_system_gems?(); end

  sig do
    params(
      dir: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.user_bundle_path(dir=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def self.user_cache(); end

  sig {returns(T.untyped)}
  def self.user_home(); end

  sig do
    params(
      executable: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.which(executable); end

  # @deprecated Use `with\_unbundled\_env` instead
  sig {params(block: T.proc.returns(T.untyped)).returns(T.untyped)}
  def self.with_clean_env(&block); end

  sig {params(block: T.proc.returns(T.untyped)).returns(T.untyped)}
  def self.with_unbundled_env(&block); end

  # Run block with environment present before
  # [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html) was activated
  sig {params(block: T.proc.returns(T.untyped)).returns(T.untyped)}
  def self.with_original_env(&block); end
end

class Bundler::APIResponseMismatchError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

# Represents metadata from when the
# [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html) gem was built.
module Bundler::BuildMetadata
  # A string representing the date the bundler gem was built.
  sig {returns(T.untyped)}
  def self.built_at(); end

  # The SHA for the git commit the bundler gem was built from.
  sig {returns(T.untyped)}
  def self.git_commit_sha(); end

  # Whether this is an official release build of
  # [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html).
  sig {returns(T.untyped)}
  def self.release?(); end

  # A hash representation of the build metadata.
  sig {returns(T.untyped)}
  def self.to_h(); end
end

class Bundler::BundlerError < StandardError
  sig {returns(T.untyped)}
  def self.all_errors(); end

  sig do
    params(
      code: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.status_code(code); end
end

class Bundler::CurrentRuby
  KNOWN_MAJOR_VERSIONS = ::T.let(nil, T.untyped)
  KNOWN_MINOR_VERSIONS = ::T.let(nil, T.untyped)
  KNOWN_PLATFORMS = ::T.let(nil, T.untyped)

  sig {returns(T.untyped)}
  def jruby?(); end

  sig {returns(T.untyped)}
  def jruby_18?(); end

  sig {returns(T.untyped)}
  def jruby_19?(); end

  sig {returns(T.untyped)}
  def jruby_1?(); end

  sig {returns(T.untyped)}
  def jruby_20?(); end

  sig {returns(T.untyped)}
  def jruby_21?(); end

  sig {returns(T.untyped)}
  def jruby_22?(); end

  sig {returns(T.untyped)}
  def jruby_23?(); end

  sig {returns(T.untyped)}
  def jruby_24?(); end

  sig {returns(T.untyped)}
  def jruby_25?(); end

  sig {returns(T.untyped)}
  def jruby_26?(); end

  sig {returns(T.untyped)}
  def jruby_27?(); end

  sig {returns(T.untyped)}
  def jruby_2?(); end

  sig {returns(T.untyped)}
  def maglev?(); end

  sig {returns(T.untyped)}
  def maglev_18?(); end

  sig {returns(T.untyped)}
  def maglev_19?(); end

  sig {returns(T.untyped)}
  def maglev_1?(); end

  sig {returns(T.untyped)}
  def maglev_20?(); end

  sig {returns(T.untyped)}
  def maglev_21?(); end

  sig {returns(T.untyped)}
  def maglev_22?(); end

  sig {returns(T.untyped)}
  def maglev_23?(); end

  sig {returns(T.untyped)}
  def maglev_24?(); end

  sig {returns(T.untyped)}
  def maglev_25?(); end

  sig {returns(T.untyped)}
  def maglev_26?(); end

  sig {returns(T.untyped)}
  def maglev_27?(); end

  sig {returns(T.untyped)}
  def maglev_2?(); end

  sig {returns(T.untyped)}
  def mingw?(); end

  sig {returns(T.untyped)}
  def mingw_18?(); end

  sig {returns(T.untyped)}
  def mingw_19?(); end

  sig {returns(T.untyped)}
  def mingw_1?(); end

  sig {returns(T.untyped)}
  def mingw_20?(); end

  sig {returns(T.untyped)}
  def mingw_21?(); end

  sig {returns(T.untyped)}
  def mingw_22?(); end

  sig {returns(T.untyped)}
  def mingw_23?(); end

  sig {returns(T.untyped)}
  def mingw_24?(); end

  sig {returns(T.untyped)}
  def mingw_25?(); end

  sig {returns(T.untyped)}
  def mingw_26?(); end

  sig {returns(T.untyped)}
  def mingw_27?(); end

  sig {returns(T.untyped)}
  def mingw_2?(); end

  sig {returns(T.untyped)}
  def mri?(); end

  sig {returns(T.untyped)}
  def mri_18?(); end

  sig {returns(T.untyped)}
  def mri_19?(); end

  sig {returns(T.untyped)}
  def mri_1?(); end

  sig {returns(T.untyped)}
  def mri_20?(); end

  sig {returns(T.untyped)}
  def mri_21?(); end

  sig {returns(T.untyped)}
  def mri_22?(); end

  sig {returns(T.untyped)}
  def mri_23?(); end

  sig {returns(T.untyped)}
  def mri_24?(); end

  sig {returns(T.untyped)}
  def mri_25?(); end

  sig {returns(T.untyped)}
  def mri_26?(); end

  sig {returns(T.untyped)}
  def mri_27?(); end

  sig {returns(T.untyped)}
  def mri_2?(); end

  sig {returns(T.untyped)}
  def mswin64?(); end

  sig {returns(T.untyped)}
  def mswin64_18?(); end

  sig {returns(T.untyped)}
  def mswin64_19?(); end

  sig {returns(T.untyped)}
  def mswin64_1?(); end

  sig {returns(T.untyped)}
  def mswin64_20?(); end

  sig {returns(T.untyped)}
  def mswin64_21?(); end

  sig {returns(T.untyped)}
  def mswin64_22?(); end

  sig {returns(T.untyped)}
  def mswin64_23?(); end

  sig {returns(T.untyped)}
  def mswin64_24?(); end

  sig {returns(T.untyped)}
  def mswin64_25?(); end

  sig {returns(T.untyped)}
  def mswin64_26?(); end

  sig {returns(T.untyped)}
  def mswin64_27?(); end

  sig {returns(T.untyped)}
  def mswin64_2?(); end

  sig {returns(T.untyped)}
  def mswin?(); end

  sig {returns(T.untyped)}
  def mswin_18?(); end

  sig {returns(T.untyped)}
  def mswin_19?(); end

  sig {returns(T.untyped)}
  def mswin_1?(); end

  sig {returns(T.untyped)}
  def mswin_20?(); end

  sig {returns(T.untyped)}
  def mswin_21?(); end

  sig {returns(T.untyped)}
  def mswin_22?(); end

  sig {returns(T.untyped)}
  def mswin_23?(); end

  sig {returns(T.untyped)}
  def mswin_24?(); end

  sig {returns(T.untyped)}
  def mswin_25?(); end

  sig {returns(T.untyped)}
  def mswin_26?(); end

  sig {returns(T.untyped)}
  def mswin_27?(); end

  sig {returns(T.untyped)}
  def mswin_2?(); end

  sig {returns(T.untyped)}
  def on_18?(); end

  sig {returns(T.untyped)}
  def on_19?(); end

  sig {returns(T.untyped)}
  def on_1?(); end

  sig {returns(T.untyped)}
  def on_20?(); end

  sig {returns(T.untyped)}
  def on_21?(); end

  sig {returns(T.untyped)}
  def on_22?(); end

  sig {returns(T.untyped)}
  def on_23?(); end

  sig {returns(T.untyped)}
  def on_24?(); end

  sig {returns(T.untyped)}
  def on_25?(); end

  sig {returns(T.untyped)}
  def on_26?(); end

  sig {returns(T.untyped)}
  def on_27?(); end

  sig {returns(T.untyped)}
  def on_2?(); end

  sig {returns(T.untyped)}
  def rbx?(); end

  sig {returns(T.untyped)}
  def rbx_18?(); end

  sig {returns(T.untyped)}
  def rbx_19?(); end

  sig {returns(T.untyped)}
  def rbx_1?(); end

  sig {returns(T.untyped)}
  def rbx_20?(); end

  sig {returns(T.untyped)}
  def rbx_21?(); end

  sig {returns(T.untyped)}
  def rbx_22?(); end

  sig {returns(T.untyped)}
  def rbx_23?(); end

  sig {returns(T.untyped)}
  def rbx_24?(); end

  sig {returns(T.untyped)}
  def rbx_25?(); end

  sig {returns(T.untyped)}
  def rbx_26?(); end

  sig {returns(T.untyped)}
  def rbx_27?(); end

  sig {returns(T.untyped)}
  def rbx_2?(); end

  sig {returns(T.untyped)}
  def ruby?(); end

  sig {returns(T.untyped)}
  def ruby_18?(); end

  sig {returns(T.untyped)}
  def ruby_19?(); end

  sig {returns(T.untyped)}
  def ruby_1?(); end

  sig {returns(T.untyped)}
  def ruby_20?(); end

  sig {returns(T.untyped)}
  def ruby_21?(); end

  sig {returns(T.untyped)}
  def ruby_22?(); end

  sig {returns(T.untyped)}
  def ruby_23?(); end

  sig {returns(T.untyped)}
  def ruby_24?(); end

  sig {returns(T.untyped)}
  def ruby_25?(); end

  sig {returns(T.untyped)}
  def ruby_26?(); end

  sig {returns(T.untyped)}
  def ruby_27?(); end

  sig {returns(T.untyped)}
  def ruby_2?(); end

  sig {returns(T.untyped)}
  def truffleruby?(); end

  sig {returns(T.untyped)}
  def truffleruby_18?(); end

  sig {returns(T.untyped)}
  def truffleruby_19?(); end

  sig {returns(T.untyped)}
  def truffleruby_1?(); end

  sig {returns(T.untyped)}
  def truffleruby_20?(); end

  sig {returns(T.untyped)}
  def truffleruby_21?(); end

  sig {returns(T.untyped)}
  def truffleruby_22?(); end

  sig {returns(T.untyped)}
  def truffleruby_23?(); end

  sig {returns(T.untyped)}
  def truffleruby_24?(); end

  sig {returns(T.untyped)}
  def truffleruby_25?(); end

  sig {returns(T.untyped)}
  def truffleruby_26?(); end

  sig {returns(T.untyped)}
  def truffleruby_27?(); end

  sig {returns(T.untyped)}
  def truffleruby_2?(); end

  sig {returns(T.untyped)}
  def x64_mingw?(); end

  sig {returns(T.untyped)}
  def x64_mingw_18?(); end

  sig {returns(T.untyped)}
  def x64_mingw_19?(); end

  sig {returns(T.untyped)}
  def x64_mingw_1?(); end

  sig {returns(T.untyped)}
  def x64_mingw_20?(); end

  sig {returns(T.untyped)}
  def x64_mingw_21?(); end

  sig {returns(T.untyped)}
  def x64_mingw_22?(); end

  sig {returns(T.untyped)}
  def x64_mingw_23?(); end

  sig {returns(T.untyped)}
  def x64_mingw_24?(); end

  sig {returns(T.untyped)}
  def x64_mingw_25?(); end

  sig {returns(T.untyped)}
  def x64_mingw_26?(); end

  sig {returns(T.untyped)}
  def x64_mingw_27?(); end

  sig {returns(T.untyped)}
  def x64_mingw_2?(); end
end

class Bundler::CyclicDependencyError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::Definition
  include ::Bundler::GemHelpers
  sig {returns(T.untyped)}
  def add_current_platform(); end

  sig do
    params(
      platform: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_platform(platform); end

  sig {returns(T.untyped)}
  def current_dependencies(); end

  sig {returns(T.untyped)}
  def dependencies(); end

  sig do
    params(
      explicit_flag: T.untyped,
    )
    .returns(T.untyped)
  end
  def ensure_equivalent_gemfile_and_lockfile(explicit_flag=T.unsafe(nil)); end

  sig do
    params(
      current_spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def find_indexed_specs(current_spec); end

  sig do
    params(
      current_spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def find_resolved_spec(current_spec); end

  sig {returns(T.untyped)}
  def gem_version_promoter(); end

  sig {returns(T.untyped)}
  def gemfiles(); end

  sig {returns(T.untyped)}
  def groups(); end

  sig {returns(T.untyped)}
  def has_local_dependencies?(); end

  sig {returns(T.untyped)}
  def has_rubygems_remotes?(); end

  sig {returns(T.untyped)}
  def index(); end

  sig do
    params(
      lockfile: T.untyped,
      dependencies: T.untyped,
      sources: T.untyped,
      unlock: T.untyped,
      ruby_version: T.untyped,
      optional_groups: T.untyped,
      gemfiles: T.untyped,
    )
    .void
  end
  def initialize(lockfile, dependencies, sources, unlock, ruby_version=T.unsafe(nil), optional_groups=T.unsafe(nil), gemfiles=T.unsafe(nil)); end

  sig do
    params(
      file: T.untyped,
      preserve_unknown_sections: T.untyped,
    )
    .returns(T.untyped)
  end
  def lock(file, preserve_unknown_sections=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def locked_bundler_version(); end

  sig {returns(T.untyped)}
  def locked_deps(); end

  sig {returns(T.untyped)}
  def locked_gems(); end

  sig {returns(T.untyped)}
  def locked_ruby_version(); end

  sig {returns(T.untyped)}
  def locked_ruby_version_object(); end

  sig {returns(T.untyped)}
  def lockfile(); end

  sig {returns(T.untyped)}
  def missing_specs(); end

  sig {returns(T.untyped)}
  def missing_specs?(); end

  sig {returns(T.untyped)}
  def new_platform?(); end

  sig {returns(T.untyped)}
  def new_specs(); end

  sig {returns(T.untyped)}
  def nothing_changed?(); end

  sig {returns(T.untyped)}
  def platforms(); end

  sig do
    params(
      platform: T.untyped,
    )
    .returns(T.untyped)
  end
  def remove_platform(platform); end

  sig {returns(T.untyped)}
  def removed_specs(); end

  sig {returns(T.untyped)}
  def requested_specs(); end

  sig {returns(T.untyped)}
  def requires(); end

  # Resolve all the dependencies specified in Gemfile. It ensures that
  # dependencies that have been already resolved via locked file and are fresh
  # are reused when resolving dependencies
  #
  # @return [SpecSet] resolved dependencies
  sig {returns(T.untyped)}
  def resolve(); end

  sig {returns(T.untyped)}
  def resolve_remotely!(); end

  sig {returns(T.untyped)}
  def resolve_with_cache!(); end

  sig {returns(T.untyped)}
  def ruby_version(); end

  sig {returns(T.untyped)}
  def spec_git_paths(); end

  # For given dependency list returns a SpecSet with Gemspec of all the required
  # dependencies.
  #
  # ```
  # 1. The method first resolves the dependencies specified in Gemfile
  # 2. After that it tries and fetches gemspec of resolved dependencies
  # ```
  #
  # @return [Bundler::SpecSet]
  sig {returns(T.untyped)}
  def specs(); end

  sig do
    params(
      groups: T.untyped,
    )
    .returns(T.untyped)
  end
  def specs_for(groups); end

  sig {returns(T.untyped)}
  def to_lock(); end

  sig {returns(T.untyped)}
  def unlocking?(); end

  sig {returns(T.untyped)}
  def validate_platforms!(); end

  sig {returns(T.untyped)}
  def validate_ruby!(); end

  sig {returns(T.untyped)}
  def validate_runtime!(); end

  # Given a gemfile and lockfile creates a
  # [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html) definition
  #
  # @param gemfile [Pathname] Path to Gemfile @param lockfile [Pathname,nil]
  # Path to Gemfile.lock @param unlock [Hash, Boolean, nil] Gems that have been
  # requested
  #
  # ```ruby
  # to be updated or true if all gems should be updated
  # ```
  #
  # @return [Bundler::Definition]
  sig do
    params(
      gemfile: T.untyped,
      lockfile: T.untyped,
      unlock: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.build(gemfile, lockfile, unlock); end
end

class Bundler::DepProxy
  # Also aliased as:
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/Bundler/DepProxy.html#method-i-eql-3F)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def ==(other); end

  sig {returns(T.untyped)}
  def __platform(); end

  sig {returns(T.untyped)}
  def dep(); end

  # Alias for:
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/Bundler/DepProxy.html#method-i-3D-3D)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def eql?(other); end

  sig {returns(T.untyped)}
  def hash(); end

  sig do
    params(
      dep: T.untyped,
      platform: T.untyped,
    )
    .void
  end
  def initialize(dep, platform); end

  sig {returns(T.untyped)}
  def name(); end

  sig {returns(T.untyped)}
  def requirement(); end

  sig {returns(T.untyped)}
  def to_s(); end

  sig {returns(T.untyped)}
  def type(); end
end

class Bundler::Dependency < Gem::Dependency
  PLATFORM_MAP = ::T.let(nil, T.untyped)
  REVERSE_PLATFORM_MAP = ::T.let(nil, T.untyped)

  sig {returns(T.untyped)}
  def autorequire(); end

  sig {returns(T.untyped)}
  def current_env?(); end

  sig {returns(T.untyped)}
  def current_platform?(); end

  # Returns the platforms this dependency is valid for, in the same order as
  # passed in the `valid\_platforms` parameter
  sig do
    params(
      valid_platforms: T.untyped,
    )
    .returns(T.untyped)
  end
  def gem_platforms(valid_platforms); end

  sig {returns(T.untyped)}
  def gemfile(); end

  sig {returns(T.untyped)}
  def groups(); end

  sig do
    params(
      name: T.untyped,
      version: T.untyped,
      options: T.untyped,
      blk: T.untyped,
    )
    .void
  end
  def initialize(name, version, options=T.unsafe(nil), &blk); end

  sig {returns(T.untyped)}
  def platforms(); end

  sig {returns(T.untyped)}
  def should_include?(); end

  sig {returns(T.untyped)}
  def specific?(); end

  sig {returns(T.untyped)}
  def to_lock(); end
end

class Bundler::DeprecatedError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::Dsl
  include ::Bundler::RubyDsl
  VALID_KEYS = ::T.let(nil, T.untyped)
  VALID_PLATFORMS = ::T.let(nil, T.untyped)

  sig {returns(T.untyped)}
  def dependencies(); end

  sig do
    params(
      dependencies: T.untyped,
    )
    .returns(T.untyped)
  end
  def dependencies=(dependencies); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def env(name); end

  sig do
    params(
      gemfile: T.untyped,
      contents: T.untyped,
    )
    .returns(T.untyped)
  end
  def eval_gemfile(gemfile, contents=T.unsafe(nil)); end

  sig do
    params(
      name: T.untyped,
      args: T.untyped,
    )
    .returns(T.untyped)
  end
  def gem(name, *args); end

  sig do
    params(
      opts: T.untyped,
    )
    .returns(T.untyped)
  end
  def gemspec(opts=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def gemspecs(); end

  sig do
    params(
      uri: T.untyped,
      options: T.untyped,
      blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def git(uri, options=T.unsafe(nil), &blk); end

  sig do
    params(
      name: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def git_source(name, &block); end

  sig do
    params(
      repo: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def github(repo, options=T.unsafe(nil)); end

  sig do
    params(
      args: T.untyped,
      blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def group(*args, &blk); end

  sig {void}
  def initialize(); end

  sig do
    params(
      args: T.untyped,
    )
    .returns(T.untyped)
  end
  def install_if(*args); end

  sig do
    params(
      name: T.untyped,
      args: T.untyped,
    )
    .returns(T.untyped)
  end
  def method_missing(name, *args); end

  sig do
    params(
      path: T.untyped,
      options: T.untyped,
      blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def path(path, options=T.unsafe(nil), &blk); end

  # Alias for:
  # [`platforms`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Dsl.html#method-i-platforms)
  sig do
    params(
      platforms: T.untyped,
    )
    .returns(T.untyped)
  end
  def platform(*platforms); end

  # Also aliased as:
  # [`platform`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Dsl.html#method-i-platform)
  sig do
    params(
      platforms: T.untyped,
    )
    .returns(T.untyped)
  end
  def platforms(*platforms); end

  sig do
    params(
      args: T.untyped,
    )
    .returns(T.untyped)
  end
  def plugin(*args); end

  sig do
    params(
      source: T.untyped,
      args: T.untyped,
      blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def source(source, *args, &blk); end

  sig do
    params(
      lockfile: T.untyped,
      unlock: T.untyped,
    )
    .returns(T.untyped)
  end
  def to_definition(lockfile, unlock); end

  sig do
    params(
      gemfile: T.untyped,
      lockfile: T.untyped,
      unlock: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.evaluate(gemfile, lockfile, unlock); end
end

class Bundler::Dsl::DSLError < Bundler::GemfileError
  # @return [Exception] the backtrace of the exception raised by the
  #
  # ```
  # evaluation of the dsl file.
  # ```
  sig {returns(T.untyped)}
  def backtrace(); end

  # @return [String] the contents of the DSL that cause the exception to
  #
  # ```
  # be raised.
  # ```
  sig {returns(T.untyped)}
  def contents(); end

  # @return [String] the description that should be presented to the user.
  sig {returns(T.untyped)}
  def description(); end

  # @return [String] the path of the dsl file that raised the exception.
  sig {returns(T.untyped)}
  def dsl_path(); end

  sig do
    params(
      description: T.untyped,
      dsl_path: T.untyped,
      backtrace: T.untyped,
      contents: T.untyped,
    )
    .void
  end
  def initialize(description, dsl_path, backtrace, contents=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def status_code(); end

  # The message of the exception reports the content of podspec for the line
  # that generated the original exception.
  #
  # @example Output
  #
  # ```
  # Invalid podspec at `RestKit.podspec` - undefined method
  # `exclude_header_search_paths=' for #<Pod::Specification for
  # `RestKit/Network (0.9.3)`>
  #
  #     from spec-repos/master/RestKit/0.9.3/RestKit.podspec:36
  #     -------------------------------------------
  #         # because it would break: #import <CoreData/CoreData.h>
  #  >      ns.exclude_header_search_paths = 'Code/RestKit.h'
  #       end
  #     -------------------------------------------
  # ```
  #
  # @return [String] the message of the exception.
  sig {returns(T.untyped)}
  def to_s(); end
end

# used for Creating Specifications from the Gemcutter Endpoint
class Bundler::EndpointSpecification < Gem::Specification
  # we need this because Gem::Specification extends Enumerable
  Elem = type_template
  ILLFORMED_MESSAGE = ::T.let(nil, T.untyped)

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def __swap__(spec); end

  sig {returns(T.untyped)}
  def _local_specification(); end

  # needed for bundle clean
  sig {returns(T.untyped)}
  def bindir(); end

  sig {returns(T.untyped)}
  def checksum(); end

  sig {returns(T.untyped)}
  def dependencies(); end

  sig do
    params(
      dependencies: T.untyped,
    )
    .returns(T.untyped)
  end
  def dependencies=(dependencies); end

  # needed for binstubs
  sig {returns(T.untyped)}
  def executables(); end

  # needed for "with native extensions" during install
  sig {returns(T.untyped)}
  def extensions(); end

  sig {returns(T.untyped)}
  def fetch_platform(); end

  sig do
    params(
      name: T.untyped,
      version: T.untyped,
      platform: T.untyped,
      dependencies: T.untyped,
      metadata: T.untyped,
    )
    .void
  end
  def initialize(name, version, platform, dependencies, metadata=T.unsafe(nil)); end

  # needed for inline
  sig {returns(T.untyped)}
  def load_paths(); end

  sig {returns(T.untyped)}
  def name(); end

  sig {returns(T.untyped)}
  def platform(); end

  # needed for post\_install\_messages during install
  sig {returns(T.untyped)}
  def post_install_message(); end

  sig {returns(T.untyped)}
  def remote(); end

  sig do
    params(
      remote: T.untyped,
    )
    .returns(T.untyped)
  end
  def remote=(remote); end

  # needed for standalone, load required\_paths from local gemspec after the gem
  # is installed
  sig {returns(T.untyped)}
  def require_paths(); end

  sig {returns(T.untyped)}
  def required_ruby_version(); end

  sig {returns(T.untyped)}
  def required_rubygems_version(); end

  sig {returns(T.untyped)}
  def source(); end

  sig do
    params(
      source: T.untyped,
    )
    .returns(T.untyped)
  end
  def source=(source); end

  sig {returns(T.untyped)}
  def version(); end
end

class Bundler::EnvironmentPreserver
  BUNDLER_KEYS = ::T.let(nil, T.untyped)
  BUNDLER_PREFIX = ::T.let(nil, T.untyped)
  INTENTIONALLY_NIL = ::T.let(nil, T.untyped)

  # @return [Hash]
  sig {returns(T.untyped)}
  def backup(); end

  sig do
    params(
      env: T.untyped,
      keys: T.untyped,
    )
    .void
  end
  def initialize(env, keys); end

  # @return [Hash]
  sig {returns(T.untyped)}
  def restore(); end
end

class Bundler::FeatureFlag
  sig {returns(T.untyped)}
  def allow_bundler_dependency_conflicts?(); end

  sig {returns(T.untyped)}
  def allow_offline_install?(); end

  sig {returns(T.untyped)}
  def auto_clean_without_path?(); end

  sig {returns(T.untyped)}
  def auto_config_jobs?(); end

  sig {returns(T.untyped)}
  def bundler_10_mode?(); end

  sig {returns(T.untyped)}
  def bundler_1_mode?(); end

  sig {returns(T.untyped)}
  def bundler_2_mode?(); end

  sig {returns(T.untyped)}
  def bundler_3_mode?(); end

  sig {returns(T.untyped)}
  def bundler_4_mode?(); end

  sig {returns(T.untyped)}
  def bundler_5_mode?(); end

  sig {returns(T.untyped)}
  def bundler_6_mode?(); end

  sig {returns(T.untyped)}
  def bundler_7_mode?(); end

  sig {returns(T.untyped)}
  def bundler_8_mode?(); end

  sig {returns(T.untyped)}
  def bundler_9_mode?(); end

  sig {returns(T.untyped)}
  def cache_all?(); end

  sig {returns(T.untyped)}
  def cache_command_is_package?(); end

  sig {returns(T.untyped)}
  def console_command?(); end

  sig {returns(T.untyped)}
  def default_cli_command(); end

  sig {returns(T.untyped)}
  def default_install_uses_path?(); end

  sig {returns(T.untyped)}
  def deployment_means_frozen?(); end

  sig {returns(T.untyped)}
  def disable_multisource?(); end

  sig {returns(T.untyped)}
  def error_on_stderr?(); end

  sig {returns(T.untyped)}
  def forget_cli_options?(); end

  sig {returns(T.untyped)}
  def global_gem_cache?(); end

  sig {returns(T.untyped)}
  def init_gems_rb?(); end

  sig do
    params(
      bundler_version: T.untyped,
    )
    .void
  end
  def initialize(bundler_version); end

  sig {returns(T.untyped)}
  def list_command?(); end

  sig {returns(T.untyped)}
  def lockfile_uses_separate_rubygems_sources?(); end

  sig {returns(T.untyped)}
  def only_update_to_newer_versions?(); end

  sig {returns(T.untyped)}
  def path_relative_to_cwd?(); end

  sig {returns(T.untyped)}
  def plugins?(); end

  sig {returns(T.untyped)}
  def prefer_gems_rb?(); end

  sig {returns(T.untyped)}
  def print_only_version_number?(); end

  sig {returns(T.untyped)}
  def setup_makes_kernel_gem_public?(); end

  sig {returns(T.untyped)}
  def skip_default_git_sources?(); end

  sig {returns(T.untyped)}
  def specific_platform?(); end

  sig {returns(T.untyped)}
  def suppress_install_using_messages?(); end

  sig {returns(T.untyped)}
  def unlock_source_unlocks_spec?(); end

  sig {returns(T.untyped)}
  def update_requires_all_flag?(); end

  sig {returns(T.untyped)}
  def use_gem_version_promoter_for_major_updates?(); end

  sig {returns(T.untyped)}
  def viz_command?(); end
end

# # fileutils.rb
#
# Copyright (c) 2000-2007 Minero Aoki
#
# This program is free software. You can distribute/modify this program under
# the same terms of ruby.
#
# ## module [`Bundler::FileUtils`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils.html)
#
# Namespace for several file utility methods for copying, moving, removing, etc.
#
# ### [`Module`](https://docs.ruby-lang.org/en/2.7.0/Module.html) Functions
#
# ```ruby
# require 'bundler/vendor/fileutils/lib/fileutils'
#
# Bundler::FileUtils.cd(dir, **options)
# Bundler::FileUtils.cd(dir, **options) {|dir| block }
# Bundler::FileUtils.pwd()
# Bundler::FileUtils.mkdir(dir, **options)
# Bundler::FileUtils.mkdir(list, **options)
# Bundler::FileUtils.mkdir_p(dir, **options)
# Bundler::FileUtils.mkdir_p(list, **options)
# Bundler::FileUtils.rmdir(dir, **options)
# Bundler::FileUtils.rmdir(list, **options)
# Bundler::FileUtils.ln(target, link, **options)
# Bundler::FileUtils.ln(targets, dir, **options)
# Bundler::FileUtils.ln_s(target, link, **options)
# Bundler::FileUtils.ln_s(targets, dir, **options)
# Bundler::FileUtils.ln_sf(target, link, **options)
# Bundler::FileUtils.cp(src, dest, **options)
# Bundler::FileUtils.cp(list, dir, **options)
# Bundler::FileUtils.cp_r(src, dest, **options)
# Bundler::FileUtils.cp_r(list, dir, **options)
# Bundler::FileUtils.mv(src, dest, **options)
# Bundler::FileUtils.mv(list, dir, **options)
# Bundler::FileUtils.rm(list, **options)
# Bundler::FileUtils.rm_r(list, **options)
# Bundler::FileUtils.rm_rf(list, **options)
# Bundler::FileUtils.install(src, dest, **options)
# Bundler::FileUtils.chmod(mode, list, **options)
# Bundler::FileUtils.chmod_R(mode, list, **options)
# Bundler::FileUtils.chown(user, group, list, **options)
# Bundler::FileUtils.chown_R(user, group, list, **options)
# Bundler::FileUtils.touch(list, **options)
# ```
#
# Possible `options` are:
#
# `:force`
# :   forced operation (rewrite files if exist, remove directories if not empty,
#     etc.);
# `:verbose`
# :   print command to be run, in bash syntax, before performing it;
# `:preserve`
# :   preserve object's group, user and modification time on copying;
# `:noop`
# :   no changes are made (usable in combination with `:verbose` which will
#     print the command to run)
#
#
# Each method documents the options that it honours. See also
# [`::commands`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-commands),
# [`::options`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-options)
# and
# [`::options_of`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-options_of)
# methods to introspect which command have which options.
#
# All methods that have the concept of a "source" file or directory can take
# either one file or a list of files in that argument. See the method
# documentation for examples.
#
# There are some 'low level' methods, which do not accept keyword arguments:
#
# ```ruby
# Bundler::FileUtils.copy_entry(src, dest, preserve = false, dereference_root = false, remove_destination = false)
# Bundler::FileUtils.copy_file(src, dest, preserve = false, dereference = true)
# Bundler::FileUtils.copy_stream(srcstream, deststream)
# Bundler::FileUtils.remove_entry(path, force = false)
# Bundler::FileUtils.remove_entry_secure(path, force = false)
# Bundler::FileUtils.remove_file(path, force = false)
# Bundler::FileUtils.compare_file(path_a, path_b)
# Bundler::FileUtils.compare_stream(stream_a, stream_b)
# Bundler::FileUtils.uptodate?(file, cmp_list)
# ```
#
# ## module [`Bundler::FileUtils::Verbose`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils/Verbose.html)
#
# This module has all methods of
# [`Bundler::FileUtils`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils.html)
# module, but it outputs messages before acting. This equates to passing the
# `:verbose` flag to methods in
# [`Bundler::FileUtils`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils.html).
#
# ## module [`Bundler::FileUtils::NoWrite`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils/NoWrite.html)
#
# This module has all methods of
# [`Bundler::FileUtils`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils.html)
# module, but never changes files/directories.  This equates to passing the
# `:noop` flag to methods in
# [`Bundler::FileUtils`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils.html).
#
# ## module [`Bundler::FileUtils::DryRun`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils/DryRun.html)
#
# This module has all methods of
# [`Bundler::FileUtils`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils.html)
# module, but never changes files/directories.  This equates to passing the
# `:noop` and `:verbose` flags to methods in
# [`Bundler::FileUtils`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils.html).
module Bundler::FileUtils
  include ::Bundler::FileUtils::StreamUtils_
  extend ::Bundler::FileUtils::StreamUtils_
  LOW_METHODS = ::T.let(nil, T.untyped)
  METHODS = ::T.let(nil, T.untyped)
  OPT_TABLE = ::T.let(nil, T.untyped)

  # Changes the current directory to the directory `dir`.
  #
  # If this method is called with block, resumes to the previous working
  # directory after the block execution has finished.
  #
  # ```ruby
  # Bundler::FileUtils.cd('/')  # change directory
  #
  # Bundler::FileUtils.cd('/', verbose: true)   # change directory and report it
  #
  # Bundler::FileUtils.cd('/') do  # change directory
  #   # ...               # do something
  # end                   # return to original directory
  # ```
  #
  #
  # Also aliased as:
  # [`chdir`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-chdir)
  sig do
    params(
      dir: T.untyped,
      verbose: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cd(dir, verbose: T.unsafe(nil), &block); end

  # Alias for:
  # [`cd`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-cd)
  sig do
    params(
      dir: T.untyped,
      verbose: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chdir(dir, verbose: T.unsafe(nil), &block); end

  # Changes permission bits on the named files (in `list`) to the bit pattern
  # represented by `mode`.
  #
  # `mode` is the symbolic and absolute mode can be used.
  #
  # Absolute mode is
  #
  # ```ruby
  # Bundler::FileUtils.chmod 0755, 'somecommand'
  # Bundler::FileUtils.chmod 0644, %w(my.rb your.rb his.rb her.rb)
  # Bundler::FileUtils.chmod 0755, '/usr/bin/ruby', verbose: true
  # ```
  #
  # Symbolic mode is
  #
  # ```ruby
  # Bundler::FileUtils.chmod "u=wrx,go=rx", 'somecommand'
  # Bundler::FileUtils.chmod "u=wr,go=rr", %w(my.rb your.rb his.rb her.rb)
  # Bundler::FileUtils.chmod "u=wrx,go=rx", '/usr/bin/ruby', verbose: true
  # ```
  #
  # "a"
  # :   is user, group, other mask.
  # "u"
  # :   is user's mask.
  # "g"
  # :   is group's mask.
  # "o"
  # :   is other's mask.
  # "w"
  # :   is write permission.
  # "r"
  # :   is read permission.
  # "x"
  # :   is execute permission.
  # "X"
  # :   is execute permission for directories only, must be used in conjunction
  #     with "+"
  # "s"
  # :   is uid, gid.
  # "t"
  # :   is sticky bit.
  # "+"
  # :   is added to a class given the specified mode.
  # "-"
  # :   Is removed from a given class given mode.
  # "="
  # :   Is the exact nature of the class will be given a specified mode.
  sig do
    params(
      mode: T.untyped,
      list: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chmod(mode, list, noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Changes permission bits on the named files (in `list`) to the bit pattern
  # represented by `mode`.
  #
  # ```ruby
  # Bundler::FileUtils.chmod_R 0700, "/tmp/app.#{$$}"
  # Bundler::FileUtils.chmod_R "u=wrx", "/tmp/app.#{$$}"
  # ```
  sig do
    params(
      mode: T.untyped,
      list: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
      force: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chmod_R(mode, list, noop: T.unsafe(nil), verbose: T.unsafe(nil), force: T.unsafe(nil)); end

  # Changes owner and group on the named files (in `list`) to the user `user`
  # and the group `group`. `user` and `group` may be an ID (Integer/String) or a
  # name ([`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)). If
  # `user` or `group` is nil, this method does not change the attribute.
  #
  # ```ruby
  # Bundler::FileUtils.chown 'root', 'staff', '/usr/local/bin/ruby'
  # Bundler::FileUtils.chown nil, 'bin', Dir.glob('/usr/bin/*'), verbose: true
  # ```
  sig do
    params(
      user: T.untyped,
      group: T.untyped,
      list: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chown(user, group, list, noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Changes owner and group on the named files (in `list`) to the user `user`
  # and the group `group` recursively. `user` and `group` may be an ID
  # (Integer/String) or a name
  # ([`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)). If `user` or
  # `group` is nil, this method does not change the attribute.
  #
  # ```ruby
  # Bundler::FileUtils.chown_R 'www', 'www', '/var/www/htdocs'
  # Bundler::FileUtils.chown_R 'cvs', 'cvs', '/var/cvs', verbose: true
  # ```
  sig do
    params(
      user: T.untyped,
      group: T.untyped,
      list: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
      force: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chown_R(user, group, list, noop: T.unsafe(nil), verbose: T.unsafe(nil), force: T.unsafe(nil)); end

  # Alias for:
  # [`compare_file`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-compare_file)
  sig do
    params(
      a: T.untyped,
      b: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cmp(a, b); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # methods names which have the option `opt`.
  #
  # ```ruby
  # p Bundler::FileUtils.collect_method(:preserve) #=> ["cp", "cp_r", "copy", "install"]
  # ```
  sig do
    params(
      opt: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.collect_method(opt); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # names of high-level methods that accept any keyword arguments.
  #
  # ```ruby
  # p Bundler::FileUtils.commands  #=> ["chmod", "cp", "cp_r", "install", ...]
  # ```
  sig {returns(T.untyped)}
  def self.commands(); end

  # Returns true if the contents of a file `a` and a file `b` are identical.
  #
  # ```ruby
  # Bundler::FileUtils.compare_file('somefile', 'somefile')       #=> true
  # Bundler::FileUtils.compare_file('/dev/null', '/dev/urandom')  #=> false
  # ```
  #
  #
  # Also aliased as:
  # [`identical?`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-identical-3F),
  # [`cmp`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-cmp)
  sig do
    params(
      a: T.untyped,
      b: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.compare_file(a, b); end

  # Returns true if the contents of a stream `a` and `b` are identical.
  sig do
    params(
      a: T.untyped,
      b: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.compare_stream(a, b); end

  # Alias for:
  # [`cp`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-cp)
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      preserve: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy(src, dest, preserve: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Copies a file system entry `src` to `dest`. If `src` is a directory, this
  # method copies its contents recursively. This method preserves file types,
  # c.f. symlink, directory... (FIFO, device files and etc. are not supported
  # yet)
  #
  # Both of `src` and `dest` must be a path name. `src` must exist, `dest` must
  # not exist.
  #
  # If `preserve` is true, this method preserves owner, group, and modified
  # time. Permissions are copied regardless `preserve`.
  #
  # If `dereference_root` is true, this method dereference tree root.
  #
  # If `remove_destination` is true, this method removes each destination file
  # before copy.
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      preserve: T.untyped,
      dereference_root: T.untyped,
      remove_destination: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy_entry(src, dest, preserve=T.unsafe(nil), dereference_root=T.unsafe(nil), remove_destination=T.unsafe(nil)); end

  # Copies file contents of `src` to `dest`. Both of `src` and `dest` must be a
  # path name.
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      preserve: T.untyped,
      dereference: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy_file(src, dest, preserve=T.unsafe(nil), dereference=T.unsafe(nil)); end

  # Copies stream `src` to `dest`. `src` must respond to read(n) and `dest` must
  # respond to write(str).
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy_stream(src, dest); end

  # Copies a file content `src` to `dest`. If `dest` is a directory, copies
  # `src` to `dest/src`.
  #
  # If `src` is a list of files, then `dest` must be a directory.
  #
  # ```ruby
  # Bundler::FileUtils.cp 'eval.c', 'eval.c.org'
  # Bundler::FileUtils.cp %w(cgi.rb complex.rb date.rb), '/usr/lib/ruby/1.6'
  # Bundler::FileUtils.cp %w(cgi.rb complex.rb date.rb), '/usr/lib/ruby/1.6', verbose: true
  # Bundler::FileUtils.cp 'symlink', 'dest'   # copy content, "dest" is not a symlink
  # ```
  #
  #
  # Also aliased as:
  # [`copy`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-copy)
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      preserve: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cp(src, dest, preserve: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Copies `src` to `dest`. If `src` is a directory, this method copies all its
  # contents recursively. If `dest` is a directory, copies `src` to `dest/src`.
  #
  # `src` can be a list of files.
  #
  # If `dereference_root` is true, this method dereference tree root.
  #
  # If `remove_destination` is true, this method removes each destination file
  # before copy.
  #
  # ```ruby
  # # Installing Ruby library "mylib" under the site_ruby
  # Bundler::FileUtils.rm_r site_ruby + '/mylib', force: true
  # Bundler::FileUtils.cp_r 'lib/', site_ruby + '/mylib'
  #
  # # Examples of copying several files to target directory.
  # Bundler::FileUtils.cp_r %w(mail.rb field.rb debug/), site_ruby + '/tmail'
  # Bundler::FileUtils.cp_r Dir.glob('*.rb'), '/home/foo/lib/ruby', noop: true, verbose: true
  #
  # # If you want to copy all contents of a directory instead of the
  # # directory itself, c.f. src/x -> dest/x, src/y -> dest/y,
  # # use following code.
  # Bundler::FileUtils.cp_r 'src/.', 'dest'     # cp_r('src', 'dest') makes dest/src,
  #                                    # but this doesn't.
  # ```
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      preserve: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
      dereference_root: T.untyped,
      remove_destination: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cp_r(src, dest, preserve: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil), dereference_root: T.unsafe(nil), remove_destination: T.unsafe(nil)); end

  # Alias for:
  # [`pwd`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-pwd)
  sig {returns(T.untyped)}
  def self.getwd(); end

  # Returns true if the method `mid` have an option `opt`.
  #
  # ```ruby
  # p Bundler::FileUtils.have_option?(:cp, :noop)     #=> true
  # p Bundler::FileUtils.have_option?(:rm, :force)    #=> true
  # p Bundler::FileUtils.have_option?(:rm, :preserve) #=> false
  # ```
  sig do
    params(
      mid: T.untyped,
      opt: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.have_option?(mid, opt); end

  # Alias for:
  # [`compare_file`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-compare_file)
  sig do
    params(
      a: T.untyped,
      b: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.identical?(a, b); end

  # If `src` is not same as `dest`, copies it and changes the permission mode to
  # `mode`. If `dest` is a directory, destination is `dest`/`src`. This method
  # removes destination before copy.
  #
  # ```ruby
  # Bundler::FileUtils.install 'ruby', '/usr/local/bin/ruby', mode: 0755, verbose: true
  # Bundler::FileUtils.install 'lib.rb', '/usr/local/lib/ruby/site_ruby', verbose: true
  # ```
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      mode: T.untyped,
      owner: T.untyped,
      group: T.untyped,
      preserve: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.install(src, dest, mode: T.unsafe(nil), owner: T.unsafe(nil), group: T.unsafe(nil), preserve: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Alias for:
  # [`ln`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-ln)
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      force: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.link(src, dest, force: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # In the first form, creates a hard link `link` which points to `target`. If
  # `link` already exists, raises Errno::EEXIST. But if the `force` option is
  # set, overwrites `link`.
  #
  # ```ruby
  # Bundler::FileUtils.ln 'gcc', 'cc', verbose: true
  # Bundler::FileUtils.ln '/usr/bin/emacs21', '/usr/bin/emacs'
  # ```
  #
  # In the second form, creates a link `dir/target` pointing to `target`. In the
  # third form, creates several hard links in the directory `dir`, pointing to
  # each item in `targets`. If `dir` is not a directory, raises Errno::ENOTDIR.
  #
  # ```ruby
  # Bundler::FileUtils.cd '/sbin'
  # Bundler::FileUtils.ln %w(cp mv mkdir), '/bin'   # Now /sbin/cp and /bin/cp are linked.
  # ```
  #
  #
  # Also aliased as:
  # [`link`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-link)
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      force: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.ln(src, dest, force: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # In the first form, creates a symbolic link `link` which points to `target`.
  # If `link` already exists, raises Errno::EEXIST. But if the `force` option is
  # set, overwrites `link`.
  #
  # ```ruby
  # Bundler::FileUtils.ln_s '/usr/bin/ruby', '/usr/local/bin/ruby'
  # Bundler::FileUtils.ln_s 'verylongsourcefilename.c', 'c', force: true
  # ```
  #
  # In the second form, creates a link `dir/target` pointing to `target`. In the
  # third form, creates several symbolic links in the directory `dir`, pointing
  # to each item in `targets`. If `dir` is not a directory, raises
  # Errno::ENOTDIR.
  #
  # ```ruby
  # Bundler::FileUtils.ln_s Dir.glob('/bin/*.rb'), '/home/foo/bin'
  # ```
  #
  #
  # Also aliased as:
  # [`symlink`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-symlink)
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      force: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.ln_s(src, dest, force: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Same as
  #
  # ```ruby
  # Bundler::FileUtils.ln_s(*args, force: true)
  # ```
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.ln_sf(src, dest, noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Alias for:
  # [`mkdir_p`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-mkdir_p)
  sig do
    params(
      list: T.untyped,
      mode: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.makedirs(list, mode: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Creates one or more directories.
  #
  # ```ruby
  # Bundler::FileUtils.mkdir 'test'
  # Bundler::FileUtils.mkdir %w(tmp data)
  # Bundler::FileUtils.mkdir 'notexist', noop: true  # Does not really create.
  # Bundler::FileUtils.mkdir 'tmp', mode: 0700
  # ```
  sig do
    params(
      list: T.untyped,
      mode: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mkdir(list, mode: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Creates a directory and all its parent directories. For example,
  #
  # ```ruby
  # Bundler::FileUtils.mkdir_p '/usr/local/lib/ruby'
  # ```
  #
  # causes to make following directories, if they do not exist.
  #
  # *   /usr
  # *   /usr/local
  # *   /usr/local/lib
  # *   /usr/local/lib/ruby
  #
  #
  # You can pass several directories at a time in a list.
  #
  # Also aliased as:
  # [`mkpath`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-mkpath),
  # [`makedirs`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-makedirs)
  sig do
    params(
      list: T.untyped,
      mode: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mkdir_p(list, mode: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Alias for:
  # [`mkdir_p`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-mkdir_p)
  sig do
    params(
      list: T.untyped,
      mode: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mkpath(list, mode: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Alias for:
  # [`mv`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-mv)
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      force: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
      secure: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.move(src, dest, force: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil), secure: T.unsafe(nil)); end

  # Moves file(s) `src` to `dest`. If `file` and `dest` exist on the different
  # disk partition, the file is copied then the original file is removed.
  #
  # ```ruby
  # Bundler::FileUtils.mv 'badname.rb', 'goodname.rb'
  # Bundler::FileUtils.mv 'stuff.rb', '/notexist/lib/ruby', force: true  # no error
  #
  # Bundler::FileUtils.mv %w(junk.txt dust.txt), '/home/foo/.trash/'
  # Bundler::FileUtils.mv Dir.glob('test*.rb'), 'test', noop: true, verbose: true
  # ```
  #
  #
  # Also aliased as:
  # [`move`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-move)
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      force: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
      secure: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mv(src, dest, force: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil), secure: T.unsafe(nil)); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # option names.
  #
  # ```ruby
  # p Bundler::FileUtils.options  #=> ["noop", "force", "verbose", "preserve", "mode"]
  # ```
  sig {returns(T.untyped)}
  def self.options(); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of
  # option names of the method `mid`.
  #
  # ```ruby
  # p Bundler::FileUtils.options_of(:rm)  #=> ["noop", "verbose", "force"]
  # ```
  sig do
    params(
      mid: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.options_of(mid); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.private_module_function(name); end

  # Returns the name of the current directory.
  #
  # Also aliased as:
  # [`getwd`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-getwd)
  sig {returns(T.untyped)}
  def self.pwd(); end

  # Alias for:
  # [`rm`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-rm)
  sig do
    params(
      list: T.untyped,
      force: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove(list, force: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Removes a directory `dir` and its contents recursively. This method ignores
  # [`StandardError`](https://docs.ruby-lang.org/en/2.7.0/StandardError.html) if
  # `force` is true.
  sig do
    params(
      path: T.untyped,
      force: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_dir(path, force=T.unsafe(nil)); end

  # This method removes a file system entry `path`. `path` might be a regular
  # file, a directory, or something. If `path` is a directory, remove it
  # recursively.
  #
  # See also remove\_entry\_secure.
  sig do
    params(
      path: T.untyped,
      force: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_entry(path, force=T.unsafe(nil)); end

  # This method removes a file system entry `path`. `path` shall be a regular
  # file, a directory, or something. If `path` is a directory, remove it
  # recursively. This method is required to avoid TOCTTOU
  # (time-of-check-to-time-of-use) local security vulnerability of rm\_r.
  # [`rm_r`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-rm_r)
  # causes security hole when:
  #
  # *   Parent directory is world writable (including /tmp).
  # *   Removing directory tree includes world writable directory.
  # *   The system has symbolic link.
  #
  #
  # To avoid this security hole, this method applies special preprocess. If
  # `path` is a directory, this method chown(2) and chmod(2) all removing
  # directories. This requires the current process is the owner of the removing
  # whole directory tree, or is the super user (root).
  #
  # WARNING: You must ensure that **ALL** parent directories cannot be moved by
  # other untrusted users. For example, parent directories should not be owned
  # by untrusted users, and should not be world writable except when the sticky
  # bit set.
  #
  # WARNING: Only the owner of the removing directory tree, or Unix super user
  # (root) should invoke this method. Otherwise this method does not work.
  #
  # For details of this security vulnerability, see Perl's case:
  #
  # *   https://cve.mitre.org/cgi-bin/cvename.cgi?name=CAN-2005-0448
  # *   https://cve.mitre.org/cgi-bin/cvename.cgi?name=CAN-2004-0452
  #
  #
  # For fileutils.rb, this vulnerability is reported in [ruby-dev:26100].
  sig do
    params(
      path: T.untyped,
      force: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_entry_secure(path, force=T.unsafe(nil)); end

  # Removes a file `path`. This method ignores
  # [`StandardError`](https://docs.ruby-lang.org/en/2.7.0/StandardError.html) if
  # `force` is true.
  sig do
    params(
      path: T.untyped,
      force: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_file(path, force=T.unsafe(nil)); end

  # Remove file(s) specified in `list`. This method cannot remove directories.
  # All StandardErrors are ignored when the :force option is set.
  #
  # ```ruby
  # Bundler::FileUtils.rm %w( junk.txt dust.txt )
  # Bundler::FileUtils.rm Dir.glob('*.so')
  # Bundler::FileUtils.rm 'NotExistFile', force: true   # never raises exception
  # ```
  #
  #
  # Also aliased as:
  # [`remove`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-remove)
  sig do
    params(
      list: T.untyped,
      force: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm(list, force: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Equivalent to
  #
  # ```ruby
  # Bundler::FileUtils.rm(list, force: true)
  # ```
  #
  #
  # Also aliased as:
  # [`safe_unlink`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-safe_unlink)
  sig do
    params(
      list: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm_f(list, noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # remove files `list[0]` `list[1]`... If `list[n]` is a directory, removes its
  # all contents recursively. This method ignores
  # [`StandardError`](https://docs.ruby-lang.org/en/2.7.0/StandardError.html)
  # when :force option is set.
  #
  # ```ruby
  # Bundler::FileUtils.rm_r Dir.glob('/tmp/*')
  # Bundler::FileUtils.rm_r 'some_dir', force: true
  # ```
  #
  # WARNING: This method causes local vulnerability if one of parent directories
  # or removing directory tree are world writable (including /tmp, whose
  # permission is 1777), and the current process has strong privilege such as
  # Unix super user (root), and the system has symbolic link. For secure
  # removing, read the documentation of
  # [`remove_entry_secure`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-remove_entry_secure)
  # carefully, and set :secure option to true. Default is `secure: false`.
  #
  # NOTE: This method calls
  # [`remove_entry_secure`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-remove_entry_secure)
  # if :secure option is set. See also remove\_entry\_secure.
  sig do
    params(
      list: T.untyped,
      force: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
      secure: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm_r(list, force: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil), secure: T.unsafe(nil)); end

  # Equivalent to
  #
  # ```ruby
  # Bundler::FileUtils.rm_r(list, force: true)
  # ```
  #
  # WARNING: This method causes local vulnerability. Read the documentation of
  # [`rm_r`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-rm_r)
  # first.
  #
  # Also aliased as:
  # [`rmtree`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-c-rmtree)
  sig do
    params(
      list: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
      secure: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm_rf(list, noop: T.unsafe(nil), verbose: T.unsafe(nil), secure: T.unsafe(nil)); end

  # Removes one or more directories.
  #
  # ```ruby
  # Bundler::FileUtils.rmdir 'somedir'
  # Bundler::FileUtils.rmdir %w(somedir anydir otherdir)
  # # Does not really remove directory; outputs message.
  # Bundler::FileUtils.rmdir 'somedir', verbose: true, noop: true
  # ```
  sig do
    params(
      list: T.untyped,
      parents: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rmdir(list, parents: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Alias for:
  # [`rm_rf`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-rm_rf)
  sig do
    params(
      list: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
      secure: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rmtree(list, noop: T.unsafe(nil), verbose: T.unsafe(nil), secure: T.unsafe(nil)); end

  # Alias for:
  # [`rm_f`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-rm_f)
  sig do
    params(
      list: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.safe_unlink(list, noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Alias for:
  # [`ln_s`](https://docs.ruby-lang.org/en/2.7.0/FileUtils.html#method-i-ln_s)
  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      force: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.symlink(src, dest, force: T.unsafe(nil), noop: T.unsafe(nil), verbose: T.unsafe(nil)); end

  # Updates modification time (mtime) and access time (atime) of file(s) in
  # `list`. Files are created if they don't exist.
  #
  # ```ruby
  # Bundler::FileUtils.touch 'timestamp'
  # Bundler::FileUtils.touch Dir.glob('*.c');  system 'make'
  # ```
  sig do
    params(
      list: T.untyped,
      noop: T.untyped,
      verbose: T.untyped,
      mtime: T.untyped,
      nocreate: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.touch(list, noop: T.unsafe(nil), verbose: T.unsafe(nil), mtime: T.unsafe(nil), nocreate: T.unsafe(nil)); end

  # Returns true if `new` is newer than all `old_list`. Non-existent files are
  # older than any file.
  #
  # ```ruby
  # Bundler::FileUtils.uptodate?('hello.o', %w(hello.c hello.h)) or \
  #     system 'make hello.o'
  # ```
  sig do
    params(
      new: T.untyped,
      old_list: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.uptodate?(new, old_list); end
end

# This module has all methods of
# [`Bundler::FileUtils`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils.html)
# module, but never changes files/directories, with printing message before
# acting. This equates to passing the `:noop` and `:verbose` flag to methods in
# [`Bundler::FileUtils`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils.html).
module Bundler::FileUtils::DryRun
  include ::Bundler::FileUtils::LowMethods
  include ::Bundler::FileUtils
  include ::Bundler::FileUtils::StreamUtils_
  extend ::Bundler::FileUtils::DryRun
  extend ::Bundler::FileUtils::LowMethods
  extend ::Bundler::FileUtils
  extend ::Bundler::FileUtils::StreamUtils_
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cd(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chdir(*_); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chmod(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chmod_R(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chown(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chown_R(*args, **options); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cmp(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.compare_file(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.compare_stream(*_); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy(*args, **options); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy_entry(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy_file(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy_stream(*_); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cp(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cp_r(*args, **options); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.getwd(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.identical?(*_); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.install(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.link(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.ln(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.ln_s(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.ln_sf(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.makedirs(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mkdir(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mkdir_p(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mkpath(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.move(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mv(*args, **options); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.pwd(*_); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove(*args, **options); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_dir(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_entry(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_entry_secure(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_file(*_); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm_f(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm_r(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm_rf(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rmdir(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rmtree(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.safe_unlink(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.symlink(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.touch(*args, **options); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.uptodate?(*_); end
end

class Bundler::FileUtils::Entry_
  include ::Bundler::FileUtils::StreamUtils_
  DIRECTORY_TERM = ::T.let(nil, T.untyped)
  SYSCASE = ::T.let(nil, T.untyped)
  S_IF_DOOR = ::T.let(nil, T.untyped)

  sig {returns(T.untyped)}
  def blockdev?(); end

  sig {returns(T.untyped)}
  def chardev?(); end

  sig do
    params(
      mode: T.untyped,
    )
    .returns(T.untyped)
  end
  def chmod(mode); end

  sig do
    params(
      uid: T.untyped,
      gid: T.untyped,
    )
    .returns(T.untyped)
  end
  def chown(uid, gid); end

  sig do
    params(
      dest: T.untyped,
    )
    .returns(T.untyped)
  end
  def copy(dest); end

  sig do
    params(
      dest: T.untyped,
    )
    .returns(T.untyped)
  end
  def copy_file(dest); end

  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def copy_metadata(path); end

  sig {returns(T.untyped)}
  def dereference?(); end

  sig {returns(T.untyped)}
  def directory?(); end

  sig {returns(T.untyped)}
  def door?(); end

  sig {returns(T.untyped)}
  def entries(); end

  sig {returns(T.untyped)}
  def exist?(); end

  sig {returns(T.untyped)}
  def file?(); end

  sig do
    params(
      a: T.untyped,
      b: T.untyped,
      deref: T.untyped,
    )
    .void
  end
  def initialize(a, b=T.unsafe(nil), deref=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def inspect(); end

  sig {returns(T.untyped)}
  def lstat(); end

  sig {returns(T.untyped)}
  def lstat!(); end

  sig {returns(T.untyped)}
  def path(); end

  sig {returns(T.untyped)}
  def pipe?(); end

  sig {returns(T.untyped)}
  def platform_support(); end

  sig {returns(T.untyped)}
  def postorder_traverse(); end

  sig {returns(T.untyped)}
  def prefix(); end

  sig {returns(T.untyped)}
  def preorder_traverse(); end

  sig {returns(T.untyped)}
  def rel(); end

  sig {returns(T.untyped)}
  def remove(); end

  sig {returns(T.untyped)}
  def remove_dir1(); end

  sig {returns(T.untyped)}
  def remove_file(); end

  sig {returns(T.untyped)}
  def socket?(); end

  sig {returns(T.untyped)}
  def stat(); end

  sig {returns(T.untyped)}
  def stat!(); end

  sig {returns(T.untyped)}
  def symlink?(); end

  sig {returns(T.untyped)}
  def traverse(); end

  sig do
    params(
      pre: T.untyped,
      post: T.untyped,
    )
    .returns(T.untyped)
  end
  def wrap_traverse(pre, post); end
end

module Bundler::FileUtils::LowMethods
end

# This module has all methods of
# [`Bundler::FileUtils`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils.html)
# module, but never changes files/directories.  This equates to passing the
# `:noop` flag to methods in
# [`Bundler::FileUtils`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils.html).
module Bundler::FileUtils::NoWrite
  include ::Bundler::FileUtils::LowMethods
  include ::Bundler::FileUtils
  include ::Bundler::FileUtils::StreamUtils_
  extend ::Bundler::FileUtils::NoWrite
  extend ::Bundler::FileUtils::LowMethods
  extend ::Bundler::FileUtils
  extend ::Bundler::FileUtils::StreamUtils_
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cd(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chdir(*_); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chmod(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chmod_R(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chown(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chown_R(*args, **options); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cmp(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.compare_file(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.compare_stream(*_); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy(*args, **options); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy_entry(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy_file(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy_stream(*_); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cp(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cp_r(*args, **options); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.getwd(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.identical?(*_); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.install(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.link(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.ln(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.ln_s(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.ln_sf(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.makedirs(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mkdir(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mkdir_p(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mkpath(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.move(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mv(*args, **options); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.pwd(*_); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove(*args, **options); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_dir(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_entry(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_entry_secure(*_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_file(*_); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm_f(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm_r(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm_rf(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rmdir(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rmtree(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.safe_unlink(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.symlink(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.touch(*args, **options); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.uptodate?(*_); end
end

module Bundler::FileUtils::StreamUtils_
end

# This module has all methods of
# [`Bundler::FileUtils`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils.html)
# module, but it outputs messages before acting. This equates to passing the
# `:verbose` flag to methods in
# [`Bundler::FileUtils`](https://docs.ruby-lang.org/en/2.7.0/Bundler/FileUtils.html).
module Bundler::FileUtils::Verbose
  include ::Bundler::FileUtils
  include ::Bundler::FileUtils::StreamUtils_
  extend ::Bundler::FileUtils::Verbose
  extend ::Bundler::FileUtils
  extend ::Bundler::FileUtils::StreamUtils_
  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cd(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chdir(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chmod(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chmod_R(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chown(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.chown_R(*args, **options); end

  sig do
    params(
      a: T.untyped,
      b: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cmp(a, b); end

  sig do
    params(
      a: T.untyped,
      b: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.compare_file(a, b); end

  sig do
    params(
      a: T.untyped,
      b: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.compare_stream(a, b); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy(*args, **options); end

  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      preserve: T.untyped,
      dereference_root: T.untyped,
      remove_destination: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy_entry(src, dest, preserve=T.unsafe(nil), dereference_root=T.unsafe(nil), remove_destination=T.unsafe(nil)); end

  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
      preserve: T.untyped,
      dereference: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy_file(src, dest, preserve=T.unsafe(nil), dereference=T.unsafe(nil)); end

  sig do
    params(
      src: T.untyped,
      dest: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.copy_stream(src, dest); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cp(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cp_r(*args, **options); end

  sig {returns(T.untyped)}
  def self.getwd(); end

  sig do
    params(
      a: T.untyped,
      b: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.identical?(a, b); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.install(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.link(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.ln(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.ln_s(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.ln_sf(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.makedirs(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mkdir(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mkdir_p(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mkpath(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.move(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.mv(*args, **options); end

  sig {returns(T.untyped)}
  def self.pwd(); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove(*args, **options); end

  sig do
    params(
      path: T.untyped,
      force: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_dir(path, force=T.unsafe(nil)); end

  sig do
    params(
      path: T.untyped,
      force: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_entry(path, force=T.unsafe(nil)); end

  sig do
    params(
      path: T.untyped,
      force: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_entry_secure(path, force=T.unsafe(nil)); end

  sig do
    params(
      path: T.untyped,
      force: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.remove_file(path, force=T.unsafe(nil)); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm_f(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm_r(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rm_rf(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rmdir(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.rmtree(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.safe_unlink(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.symlink(*args, **options); end

  sig do
    params(
      args: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.touch(*args, **options); end

  sig do
    params(
      new: T.untyped,
      old_list: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.uptodate?(new, old_list); end
end

class Bundler::GemHelper
  sig {returns(T.untyped)}
  def allowed_push_host(); end

  sig {returns(T.untyped)}
  def already_tagged?(); end

  sig {returns(T.untyped)}
  def base(); end

  sig do
    params(
      built_gem_path: T.untyped,
    )
    .returns(T.untyped)
  end
  def build_checksum(built_gem_path=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def build_gem(); end

  sig {returns(T.untyped)}
  def built_gem_path(); end

  sig {returns(T.untyped)}
  def clean?(); end

  sig {returns(T.untyped)}
  def committed?(); end

  sig {returns(T.untyped)}
  def current_branch(); end

  sig {returns(T.untyped)}
  def default_remote(); end

  sig {returns(T.untyped)}
  def gem_command(); end

  sig {returns(T.untyped)}
  def gem_key(); end

  sig {returns(T.untyped)}
  def gem_push?(); end

  sig {returns(T.untyped)}
  def gem_push_host(); end

  sig {returns(T.untyped)}
  def gemspec(); end

  sig do
    params(
      remote: T.untyped,
    )
    .returns(T.untyped)
  end
  def git_push(remote=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def guard_clean(); end

  sig do
    params(
      base: T.untyped,
      name: T.untyped,
    )
    .void
  end
  def initialize(base=T.unsafe(nil), name=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def install(); end

  sig do
    params(
      built_gem_path: T.untyped,
      local: T.untyped,
    )
    .returns(T.untyped)
  end
  def install_gem(built_gem_path=T.unsafe(nil), local=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def name(); end

  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def rubygem_push(path); end

  sig do
    params(
      cmd: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def sh(cmd, &block); end

  sig do
    params(
      cmd: T.untyped,
    )
    .returns(T.untyped)
  end
  def sh_with_input(cmd); end

  sig do
    params(
      cmd: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def sh_with_status(cmd, &block); end

  sig {returns(T.untyped)}
  def spec_path(); end

  sig do
    params(
      tag_prefix: T.untyped,
    )
    .returns(T.untyped)
  end
  def tag_prefix=(tag_prefix); end

  sig {returns(T.untyped)}
  def tag_version(); end

  sig {returns(T.untyped)}
  def version(); end

  sig {returns(T.untyped)}
  def version_tag(); end

  sig {returns(T.untyped)}
  def self.instance(); end

  sig do
    params(
      instance: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.instance=(instance); end

  sig do
    params(
      opts: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.install_tasks(opts=T.unsafe(nil)); end

  sig do
    params(
      tag_prefix: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.tag_prefix=(tag_prefix); end

  sig do
    params(
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.gemspec(&block); end
end

module Bundler::GemHelpers
  GENERICS = ::T.let(nil, T.untyped)
  GENERIC_CACHE = ::T.let(nil, T.untyped)

  sig do
    params(
      p: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.generic(p); end

  sig {returns(T.untyped)}
  def self.generic_local_platform(); end

  sig do
    params(
      spec_platform: T.untyped,
      user_platform: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.platform_specificity_match(spec_platform, user_platform); end

  sig do
    params(
      specs: T.untyped,
      platform: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.select_best_platform_match(specs, platform); end
end

class Bundler::GemHelpers::PlatformMatch < Struct
  extend T::Generic
  Elem = type_member {{fixed: T.untyped}}

  EXACT_MATCH = ::T.let(nil, T.untyped)
  WORST_MATCH = ::T.let(nil, T.untyped)

  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def <=>(other); end

  sig {returns(T.untyped)}
  def cpu_match(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def cpu_match=(_); end

  sig {returns(T.untyped)}
  def os_match(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def os_match=(_); end

  sig {returns(T.untyped)}
  def platform_version_match(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def platform_version_match=(_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.[](*_); end

  sig do
    params(
      spec_platform: T.untyped,
      user_platform: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.cpu_match(spec_platform, user_platform); end

  sig {returns(T.untyped)}
  def self.members(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.new(*_); end

  sig do
    params(
      spec_platform: T.untyped,
      user_platform: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.os_match(spec_platform, user_platform); end

  sig do
    params(
      spec_platform: T.untyped,
      user_platform: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.platform_version_match(spec_platform, user_platform); end
end

class Bundler::GemNotFound < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::GemRequireError < Bundler::BundlerError
  sig do
    params(
      orig_exception: T.untyped,
      msg: T.untyped,
    )
    .void
  end
  def initialize(orig_exception, msg); end

  sig {returns(T.untyped)}
  def orig_exception(); end

  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::GemfileError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::GemfileEvalError < Bundler::GemfileError
end

class Bundler::GemfileLockNotFound < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::GemfileNotFound < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::GemspecError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::GenericSystemCallError < Bundler::BundlerError
  sig do
    params(
      underlying_error: T.untyped,
      message: T.untyped,
    )
    .void
  end
  def initialize(underlying_error, message); end

  sig {returns(T.untyped)}
  def status_code(); end

  sig {returns(T.untyped)}
  def underlying_error(); end
end

class Bundler::GitError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::HTTPError < Bundler::BundlerError
  sig do
    params(
      uri: T.untyped,
    )
    .returns(T.untyped)
  end
  def filter_uri(uri); end

  sig {returns(T.untyped)}
  def status_code(); end
end

# Handles all the fetching with the rubygems server
class Bundler::Fetcher; end

# This error is raised if HTTP authentication is required, but not provided.
class Bundler::Fetcher::AuthenticationRequiredError < Bundler::HTTPError
end

# This error is raised if HTTP authentication is provided, but incorrect.
class Bundler::Fetcher::BadAuthenticationError < Bundler::HTTPError
end

# This is the error raised if
# [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) fails the cert
# verification
class Bundler::Fetcher::CertificateFailureError < Bundler::HTTPError
end

# This error is raised if the API returns a 413 (only printed in verbose)
class Bundler::Fetcher::FallbackError < Bundler::HTTPError
end

# This error is raised when it looks like the network is down
class Bundler::Fetcher::NetworkDownError < Bundler::HTTPError
end

# This is the error raised when a source is HTTPS and
# [`OpenSSL`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL.html) didn't load
class Bundler::Fetcher::SSLError < Bundler::HTTPError
end

class Bundler::Index
  include T::Enumerable

  Elem = type_member(:out)

  EMPTY_SEARCH = ::T.let(nil, T.untyped)
  NULL = ::T.let(nil, T.untyped)
  RUBY = ::T.let(nil, T.untyped)

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def <<(spec); end

  # Whether all the specs in self are in other TODO: rename to
  # [`include?`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-include-3F)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def ==(other); end

  # Alias for:
  # [`search`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Index.html#method-i-search)
  sig do
    params(
      query: T.untyped,
      base: T.untyped,
    )
    .returns(T.untyped)
  end
  def [](query, base=T.unsafe(nil)); end

  sig do
    params(
      index: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_source(index); end

  sig {returns(T.untyped)}
  def all_specs(); end

  sig do
    params(
      spec: T.untyped,
      other_spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def dependencies_eql?(spec, other_spec); end

  sig {returns(T.untyped)}
  def dependency_names(); end

  sig do
    params(
      blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def each(&blk); end

  sig {returns(T.untyped)}
  def empty?(); end

  sig {void}
  def initialize(); end

  sig {returns(T.untyped)}
  def inspect(); end

  sig do
    params(
      query: T.untyped,
      base: T.untyped,
    )
    .returns(T.untyped)
  end
  def local_search(query, base=T.unsafe(nil)); end

  # Search this index's specs, and any source indexes that this index knows
  # about, returning all of the results.
  #
  # Also aliased as:
  # [`[]`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Index.html#method-i-5B-5D)
  sig do
    params(
      query: T.untyped,
      base: T.untyped,
    )
    .returns(T.untyped)
  end
  def search(query, base=T.unsafe(nil)); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def search_all(name); end

  sig {returns(T.untyped)}
  def size(); end

  sig do
    params(
      specs: T.untyped,
    )
    .returns(T.untyped)
  end
  def sort_specs(specs); end

  sig {returns(T.untyped)}
  def sources(); end

  sig {returns(T.untyped)}
  def spec_names(); end

  sig {returns(T.untyped)}
  def specs(); end

  # returns a list of the dependencies
  sig {returns(T.untyped)}
  def unmet_dependency_names(); end

  sig do
    params(
      query: T.untyped,
      base: T.untyped,
    )
    .returns(T.untyped)
  end
  def unsorted_search(query, base); end

  sig do
    params(
      other: T.untyped,
      override_dupes: T.untyped,
    )
    .returns(T.untyped)
  end
  def use(other, override_dupes=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def self.build(); end

  sig do
    params(
      specs: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.sort_specs(specs); end
end

class Bundler::InstallError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::InstallHookError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::InvalidOption < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::LazySpecification
  include ::Bundler::MatchPlatform
  include ::Bundler::GemHelpers
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def ==(other); end

  sig {returns(T.untyped)}
  def __materialize__(); end

  sig {returns(T.untyped)}
  def dependencies(); end

  sig {returns(T.untyped)}
  def full_name(); end

  sig {returns(T.untyped)}
  def git_version(); end

  sig {returns(T.untyped)}
  def identifier(); end

  sig do
    params(
      name: T.untyped,
      version: T.untyped,
      platform: T.untyped,
      source: T.untyped,
    )
    .void
  end
  def initialize(name, version, platform, source=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def name(); end

  sig {returns(T.untyped)}
  def platform(); end

  sig {returns(T.untyped)}
  def remote(); end

  sig do
    params(
      remote: T.untyped,
    )
    .returns(T.untyped)
  end
  def remote=(remote); end

  sig do
    params(
      args: T.untyped,
    )
    .returns(T.untyped)
  end
  def respond_to?(*args); end

  sig do
    params(
      dependency: T.untyped,
    )
    .returns(T.untyped)
  end
  def satisfies?(dependency); end

  sig {returns(T.untyped)}
  def source(); end

  sig do
    params(
      source: T.untyped,
    )
    .returns(T.untyped)
  end
  def source=(source); end

  sig {returns(T.untyped)}
  def to_lock(); end

  sig {returns(T.untyped)}
  def to_s(); end

  sig {returns(T.untyped)}
  def version(); end
end

class Bundler::LazySpecification::Identifier < Struct
  include ::Comparable
  extend ::T::Generic

  Elem = type_member {{fixed: T.untyped}}

  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def <=>(other); end

  sig {returns(T.untyped)}
  def dependencies(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def dependencies=(_); end

  sig {returns(T.untyped)}
  def name(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def name=(_); end

  sig {returns(T.untyped)}
  def platform(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def platform=(_); end

  sig {returns(T.untyped)}
  def platform_string(); end

  sig {returns(T.untyped)}
  def source(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def source=(_); end

  sig {returns(T.untyped)}
  def version(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def version=(_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.[](*_); end

  sig {returns(T.untyped)}
  def self.members(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.new(*_); end
end

class Bundler::LockfileError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::LockfileParser
  BUNDLED = ::T.let(nil, T.untyped)
  DEPENDENCIES = ::T.let(nil, T.untyped)
  ENVIRONMENT_VERSION_SECTIONS = ::T.let(nil, T.untyped)
  GEM = ::T.let(nil, T.untyped)
  GIT = ::T.let(nil, T.untyped)
  KNOWN_SECTIONS = ::T.let(nil, T.untyped)
  NAME_VERSION = ::T.let(nil, T.untyped)
  OPTIONS = ::T.let(nil, T.untyped)
  PATH = ::T.let(nil, T.untyped)
  PLATFORMS = ::T.let(nil, T.untyped)
  PLUGIN = ::T.let(nil, T.untyped)
  RUBY = ::T.let(nil, T.untyped)
  SECTIONS_BY_VERSION_INTRODUCED = ::T.let(nil, T.untyped)
  SOURCE = ::T.let(nil, T.untyped)
  SPECS = ::T.let(nil, T.untyped)
  TYPES = ::T.let(nil, T.untyped)

  sig {returns(T.untyped)}
  def bundler_version(); end

  sig {returns(T.untyped)}
  def dependencies(); end

  sig do
    params(
      lockfile: T.untyped,
    )
    .void
  end
  def initialize(lockfile); end

  sig {returns(T.untyped)}
  def platforms(); end

  sig {returns(T.untyped)}
  def ruby_version(); end

  sig {returns(T.untyped)}
  def sources(); end

  sig {returns(T.untyped)}
  def specs(); end

  sig {returns(T.untyped)}
  def warn_for_outdated_bundler_version(); end

  sig do
    params(
      lockfile_contents: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.sections_in_lockfile(lockfile_contents); end

  sig do
    params(
      base_version: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.sections_to_ignore(base_version=T.unsafe(nil)); end

  sig do
    params(
      lockfile_contents: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.unknown_sections_in_lockfile(lockfile_contents); end
end

class Bundler::MarshalError < StandardError
end

module Bundler::MatchPlatform
  include ::Bundler::GemHelpers
  sig do
    params(
      p: T.untyped,
    )
    .returns(T.untyped)
  end
  def match_platform(p); end

  sig do
    params(
      gemspec_platform: T.untyped,
      local_platform: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.platforms_match?(gemspec_platform, local_platform); end
end

# [`Bundler::Molinillo`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo.html)
# is a generic dependency resolution algorithm.
module Bundler::Molinillo
  # The version of
  # [`Bundler::Molinillo`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo.html).
  VERSION = ::T.let(nil, T.untyped)

end

# An error caused by attempting to fulfil a dependency that was circular
#
# @note This exception will be thrown iff a {Vertex} is added to a
#
# ```
# {DependencyGraph} that has a {DependencyGraph::Vertex#path_to?} an
# existing {DependencyGraph::Vertex}
# ```
class Bundler::Molinillo::CircularDependencyError < Bundler::Molinillo::ResolverError
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html)<Object>
  # :   the dependencies responsible for causing the error
  sig {returns(T.untyped)}
  def dependencies(); end

  sig do
    params(
      vertices: T.untyped,
    )
    .void
  end
  def initialize(vertices); end
end

# Hacks needed for old Ruby versions.
module Bundler::Molinillo::Compatibility
  sig do
    params(
      enum: T.untyped,
      blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.flat_map(enum, &blk); end
end

# @!visibility private
module Bundler::Molinillo::Delegates
end

# [`Delegates`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/Delegates.html)
# all {Bundler::Molinillo::ResolutionState} methods to a `#state` property.
module Bundler::Molinillo::Delegates::ResolutionState
  # (see Bundler::Molinillo::ResolutionState#activated)
  sig {returns(T.untyped)}
  def activated(); end

  # (see Bundler::Molinillo::ResolutionState#conflicts)
  sig {returns(T.untyped)}
  def conflicts(); end

  # (see Bundler::Molinillo::ResolutionState#depth)
  sig {returns(T.untyped)}
  def depth(); end

  # (see Bundler::Molinillo::ResolutionState#name)
  sig {returns(T.untyped)}
  def name(); end

  # (see Bundler::Molinillo::ResolutionState#possibilities)
  sig {returns(T.untyped)}
  def possibilities(); end

  # (see Bundler::Molinillo::ResolutionState#requirement)
  sig {returns(T.untyped)}
  def requirement(); end

  # (see Bundler::Molinillo::ResolutionState#requirements)
  sig {returns(T.untyped)}
  def requirements(); end

  # (see Bundler::Molinillo::ResolutionState#unused\_unwind\_options)
  sig {returns(T.untyped)}
  def unused_unwind_options(); end
end

# [`Delegates`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/Delegates.html)
# all {Bundler::Molinillo::SpecificationProvider} methods to a
# `#specification\_provider` property.
module Bundler::Molinillo::Delegates::SpecificationProvider
  # (see
  # [`Bundler::Molinillo::SpecificationProvider#allow_missing?`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/SpecificationProvider.html#method-i-allow_missing-3F))
  sig do
    params(
      dependency: T.untyped,
    )
    .returns(T.untyped)
  end
  def allow_missing?(dependency); end

  # (see
  # [`Bundler::Molinillo::SpecificationProvider#dependencies_for`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/SpecificationProvider.html#method-i-dependencies_for))
  sig do
    params(
      specification: T.untyped,
    )
    .returns(T.untyped)
  end
  def dependencies_for(specification); end

  # (see
  # [`Bundler::Molinillo::SpecificationProvider#name_for`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/SpecificationProvider.html#method-i-name_for))
  sig do
    params(
      dependency: T.untyped,
    )
    .returns(T.untyped)
  end
  def name_for(dependency); end

  # (see
  # [`Bundler::Molinillo::SpecificationProvider#name_for_explicit_dependency_source`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/SpecificationProvider.html#method-i-name_for_explicit_dependency_source))
  sig {returns(T.untyped)}
  def name_for_explicit_dependency_source(); end

  # (see
  # [`Bundler::Molinillo::SpecificationProvider#name_for_locking_dependency_source`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/SpecificationProvider.html#method-i-name_for_locking_dependency_source))
  sig {returns(T.untyped)}
  def name_for_locking_dependency_source(); end

  # (see
  # [`Bundler::Molinillo::SpecificationProvider#requirement_satisfied_by?`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/SpecificationProvider.html#method-i-requirement_satisfied_by-3F))
  sig do
    params(
      requirement: T.untyped,
      activated: T.untyped,
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def requirement_satisfied_by?(requirement, activated, spec); end

  # (see
  # [`Bundler::Molinillo::SpecificationProvider#search_for`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/SpecificationProvider.html#method-i-search_for))
  sig do
    params(
      dependency: T.untyped,
    )
    .returns(T.untyped)
  end
  def search_for(dependency); end

  # (see
  # [`Bundler::Molinillo::SpecificationProvider#sort_dependencies`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/SpecificationProvider.html#method-i-sort_dependencies))
  sig do
    params(
      dependencies: T.untyped,
      activated: T.untyped,
      conflicts: T.untyped,
    )
    .returns(T.untyped)
  end
  def sort_dependencies(dependencies, activated, conflicts); end
end

# A directed acyclic graph that is tuned to hold named dependencies
class Bundler::Molinillo::DependencyGraph
  include ::TSort
  include T::Enumerable

  Elem = type_member(:out)

  # @return [Boolean] whether the two dependency graphs are equal, determined
  #
  # ```
  # by a recursive traversal of each {#root_vertices} and its
  # {Vertex#successors}
  # ```
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def ==(other); end

  # @param [String] name @param [Object] payload @param [Array<String>]
  # parent\_names @param [Object] requirement the requirement that is requiring
  # the child @return [void]
  sig do
    params(
      name: T.untyped,
      payload: T.untyped,
      parent_names: T.untyped,
      requirement: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_child_vertex(name, payload, parent_names, requirement); end

  # Adds a new {Edge} to the dependency graph @param [Vertex] origin @param
  # [Vertex] destination @param [Object] requirement the requirement that this
  # edge represents @return [Edge] the added edge
  sig do
    params(
      origin: T.untyped,
      destination: T.untyped,
      requirement: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_edge(origin, destination, requirement); end

  # Adds a vertex with the given name, or updates the existing one. @param
  # [String] name @param [Object] payload @return [Vertex] the vertex that was
  # added to `self`
  sig do
    params(
      name: T.untyped,
      payload: T.untyped,
      root: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_vertex(name, payload, root=T.unsafe(nil)); end

  # Deletes an {Edge} from the dependency graph @param [Edge] edge @return
  # [Void]
  sig do
    params(
      edge: T.untyped,
    )
    .returns(T.untyped)
  end
  def delete_edge(edge); end

  # Detaches the {#vertex\_named} `name` {Vertex} from the graph, recursively
  # removing any non-root vertices that were orphaned in the process @param
  # [String] name @return [Array<Vertex>] the vertices which have been detached
  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def detach_vertex_named(name); end

  # Enumerates through the vertices of the graph. @return [Array<Vertex>] The
  # graph's vertices.
  #
  # Also aliased as:
  # [`tsort_each_node`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/DependencyGraph.html#method-i-tsort_each_node)
  sig do
    params(
      blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def each(&blk); end

  sig {void}
  def initialize(); end

  # @return [String] a string suitable for debugging
  sig {returns(T.untyped)}
  def inspect(); end

  # @return [Log] the op log for this graph
  sig {returns(T.untyped)}
  def log(); end

  # Rewinds the graph to the state tagged as `tag` @param  [Object] tag the tag
  # to rewind to @return [Void]
  sig do
    params(
      tag: T.untyped,
    )
    .returns(T.untyped)
  end
  def rewind_to(tag); end

  # @param [String] name @return [Vertex,nil] the root vertex with the given
  # name
  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def root_vertex_named(name); end

  # Sets the payload of the vertex with the given name @param [String] name the
  # name of the vertex @param [Object] payload the payload @return [Void]
  sig do
    params(
      name: T.untyped,
      payload: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_payload(name, payload); end

  # Tags the current state of the dependency as the given tag @param  [Object]
  # tag an opaque tag for the current state of the graph @return [Void]
  sig do
    params(
      tag: T.untyped,
    )
    .returns(T.untyped)
  end
  def tag(tag); end

  # @param [Hash] options options for dot output. @return [String] Returns a dot
  # format representation of the graph
  sig do
    params(
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def to_dot(options=T.unsafe(nil)); end

  # @!visibility private
  sig do
    params(
      vertex: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def tsort_each_child(vertex, &block); end

  # @!visibility private
  #
  # Alias for:
  # [`each`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/DependencyGraph.html#method-i-each)
  sig {returns(T.untyped)}
  def tsort_each_node(); end

  # @param [String] name @return [Vertex,nil] the vertex with the given name
  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def vertex_named(name); end

  # @return [{String => Vertex}] the vertices of the dependency graph, keyed
  #
  # ```
  # by {Vertex#name}
  # ```
  sig {returns(T.untyped)}
  def vertices(); end

  # Topologically sorts the given vertices. @param [Enumerable<Vertex>] vertices
  # the vertices to be sorted, which must
  #
  # ```
  # all belong to the same graph.
  # ```
  #
  # @return [Array<Vertex>] The sorted vertices.
  sig do
    params(
      vertices: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.tsort(vertices); end
end

# An action that modifies a {DependencyGraph} that is reversible. @abstract
class Bundler::Molinillo::DependencyGraph::Action
  # Reverses the action on the given graph. @param  [DependencyGraph] graph the
  # graph to reverse the action on. @return [Void]
  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def down(graph); end

  # @return [Action,Nil] The next action
  sig {returns(T.untyped)}
  def next(); end

  # @return [Action,Nil] The next action
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def next=(_); end

  # @return [Action,Nil] The previous action
  sig {returns(T.untyped)}
  def previous(); end

  # @return [Action,Nil] The previous action
  sig do
    params(
      previous: T.untyped,
    )
    .returns(T.untyped)
  end
  def previous=(previous); end

  # Performs the action on the given graph. @param  [DependencyGraph] graph the
  # graph to perform the action on. @return [Void]
  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def up(graph); end

  # @return [Symbol] The name of the action.
  sig {returns(T.untyped)}
  def self.action_name(); end
end

# @!visibility private (see
# [`DependencyGraph#add_edge_no_circular`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/DependencyGraph.html#method-i-add_edge_no_circular))
class Bundler::Molinillo::DependencyGraph::AddEdgeNoCircular < Bundler::Molinillo::DependencyGraph::Action
  # @return [String] the name of the destination of the edge
  sig {returns(T.untyped)}
  def destination(); end

  # (see Action#down)
  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def down(graph); end

  sig do
    params(
      origin: T.untyped,
      destination: T.untyped,
      requirement: T.untyped,
    )
    .void
  end
  def initialize(origin, destination, requirement); end

  # @param  [DependencyGraph] graph the graph to find vertices from @return
  # [Edge] The edge this action adds
  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def make_edge(graph); end

  # @return [String] the name of the origin of the edge
  sig {returns(T.untyped)}
  def origin(); end

  # @return [Object] the requirement that the edge represents
  sig {returns(T.untyped)}
  def requirement(); end

  # (see Action#up)
  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def up(graph); end

  # (see Action.action\_name)
  sig {returns(T.untyped)}
  def self.action_name(); end
end

class Bundler::Molinillo::DependencyGraph::AddVertex < Bundler::Molinillo::DependencyGraph::Action
  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def down(graph); end

  sig do
    params(
      name: T.untyped,
      payload: T.untyped,
      root: T.untyped,
    )
    .void
  end
  def initialize(name, payload, root); end

  sig {returns(T.untyped)}
  def name(); end

  sig {returns(T.untyped)}
  def payload(); end

  sig {returns(T.untyped)}
  def root(); end

  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def up(graph); end

  sig {returns(T.untyped)}
  def self.action_name(); end
end

# @!visibility private (see
# [`DependencyGraph#delete_edge`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/DependencyGraph.html#method-i-delete_edge))
class Bundler::Molinillo::DependencyGraph::DeleteEdge < Bundler::Molinillo::DependencyGraph::Action
  # @return [String] the name of the destination of the edge
  sig {returns(T.untyped)}
  def destination_name(); end

  # (see Action#down)
  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def down(graph); end

  sig do
    params(
      origin_name: T.untyped,
      destination_name: T.untyped,
      requirement: T.untyped,
    )
    .void
  end
  def initialize(origin_name, destination_name, requirement); end

  # @param  [DependencyGraph] graph the graph to find vertices from @return
  # [Edge] The edge this action adds
  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def make_edge(graph); end

  # @return [String] the name of the origin of the edge
  sig {returns(T.untyped)}
  def origin_name(); end

  # @return [Object] the requirement that the edge represents
  sig {returns(T.untyped)}
  def requirement(); end

  # (see Action#up)
  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def up(graph); end

  # (see Action.action\_name)
  sig {returns(T.untyped)}
  def self.action_name(); end
end

# @!visibility private @see
# [`DependencyGraph#detach_vertex_named`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/DependencyGraph.html#method-i-detach_vertex_named)
class Bundler::Molinillo::DependencyGraph::DetachVertexNamed < Bundler::Molinillo::DependencyGraph::Action
  # (see Action#down)
  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def down(graph); end

  sig do
    params(
      name: T.untyped,
    )
    .void
  end
  def initialize(name); end

  # @return [String] the name of the vertex to detach
  sig {returns(T.untyped)}
  def name(); end

  # (see Action#up)
  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def up(graph); end

  # (see Action#name)
  sig {returns(T.untyped)}
  def self.action_name(); end
end

# A directed edge of a {DependencyGraph} @attr [Vertex] origin The origin of the
# directed edge @attr [Vertex] destination The destination of the directed edge
# @attr [Object] requirement The requirement the directed edge represents
class Bundler::Molinillo::DependencyGraph::Edge < Struct
  extend T::Generic
  Elem = type_member {{fixed: T.untyped}}

  sig {returns(T.untyped)}
  def destination(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def destination=(_); end

  sig {returns(T.untyped)}
  def origin(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def origin=(_); end

  sig {returns(T.untyped)}
  def requirement(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def requirement=(_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.[](*_); end

  sig {returns(T.untyped)}
  def self.members(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.new(*_); end
end

# A log for dependency graph actions
class Bundler::Molinillo::DependencyGraph::Log
  extend T::Enumerable

  Elem = type_template {{fixed: T.untyped}}

  # @macro action
  sig do
    params(
      graph: T.untyped,
      origin: T.untyped,
      destination: T.untyped,
      requirement: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_edge_no_circular(graph, origin, destination, requirement); end

  # @macro action
  sig do
    params(
      graph: T.untyped,
      name: T.untyped,
      payload: T.untyped,
      root: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_vertex(graph, name, payload, root); end

  # {include:DependencyGraph#delete\_edge} @param [Graph] graph the graph to
  # perform the action on @param [String] origin\_name @param [String]
  # destination\_name @param [Object] requirement @return (see
  # [`DependencyGraph#delete_edge`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/DependencyGraph.html#method-i-delete_edge))
  sig do
    params(
      graph: T.untyped,
      origin_name: T.untyped,
      destination_name: T.untyped,
      requirement: T.untyped,
    )
    .returns(T.untyped)
  end
  def delete_edge(graph, origin_name, destination_name, requirement); end

  # @macro action
  sig do
    params(
      graph: T.untyped,
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def detach_vertex_named(graph, name); end

  # @!visibility private Enumerates each action in the log @yield [Action]
  sig do
    params(
      blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def each(&blk); end

  sig {void}
  def initialize(); end

  # Pops the most recent action from the log and undoes the action @param
  # [DependencyGraph] graph @return [Action] the action that was popped off the
  # log
  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def pop!(graph); end

  # @!visibility private Enumerates each action in the log in reverse order
  # @yield [Action]
  sig {returns(T.untyped)}
  def reverse_each(); end

  # @macro action
  sig do
    params(
      graph: T.untyped,
      tag: T.untyped,
    )
    .returns(T.untyped)
  end
  def rewind_to(graph, tag); end

  # @macro action
  sig do
    params(
      graph: T.untyped,
      name: T.untyped,
      payload: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_payload(graph, name, payload); end

  # @macro action
  sig do
    params(
      graph: T.untyped,
      tag: T.untyped,
    )
    .returns(T.untyped)
  end
  def tag(graph, tag); end
end

class Bundler::Molinillo::DependencyGraph::SetPayload < Bundler::Molinillo::DependencyGraph::Action
  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def down(graph); end

  sig do
    params(
      name: T.untyped,
      payload: T.untyped,
    )
    .void
  end
  def initialize(name, payload); end

  sig {returns(T.untyped)}
  def name(); end

  sig {returns(T.untyped)}
  def payload(); end

  sig do
    params(
      graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def up(graph); end

  sig {returns(T.untyped)}
  def self.action_name(); end
end

# @!visibility private @see
# [`DependencyGraph#tag`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/DependencyGraph.html#method-i-tag)
class Bundler::Molinillo::DependencyGraph::Tag < Bundler::Molinillo::DependencyGraph::Action
  # (see Action#down)
  sig do
    params(
      _graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def down(_graph); end

  sig do
    params(
      tag: T.untyped,
    )
    .void
  end
  def initialize(tag); end

  # @return [Object] An opaque tag
  sig {returns(T.untyped)}
  def tag(); end

  # (see Action#up)
  sig do
    params(
      _graph: T.untyped,
    )
    .returns(T.untyped)
  end
  def up(_graph); end

  # (see Action.action\_name)
  sig {returns(T.untyped)}
  def self.action_name(); end
end

# A vertex in a {DependencyGraph} that encapsulates a {#name} and a {#payload}
class Bundler::Molinillo::DependencyGraph::Vertex
  # @return [Boolean] whether the two vertices are equal, determined
  #
  # ```
  # by a recursive traversal of each {Vertex#successors}
  # ```
  #
  #
  # Also aliased as:
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/DependencyGraph/Vertex.html#method-i-eql-3F)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def ==(other); end

  # @param [Vertex] other the vertex to check if there's a path to @param
  # [Set<Vertex>] visited the vertices of {#graph} that have been visited
  # @return [Boolean] whether there is a path to `other` from `self`
  sig do
    params(
      other: T.untyped,
      visited: T.untyped,
    )
    .returns(T.untyped)
  end
  def _path_to?(other, visited=T.unsafe(nil)); end

  # Is there a path from `other` to `self` following edges in the dependency
  # graph? @return true iff there is a path following edges within this {#graph}
  #
  # Also aliased as:
  # [`is_reachable_from?`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/DependencyGraph/Vertex.html#method-i-is_reachable_from-3F)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def ancestor?(other); end

  # Alias for:
  # [`path_to?`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/DependencyGraph/Vertex.html#method-i-path_to-3F)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def descendent?(other); end

  # Alias for:
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/DependencyGraph/Vertex.html#method-i-3D-3D)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def eql?(other); end

  # @return [Array<Object>] the explicit requirements that required
  #
  # ```ruby
  # this vertex
  # ```
  sig {returns(T.untyped)}
  def explicit_requirements(); end

  # @return [Fixnum] a hash for the vertex based upon its {#name}
  sig {returns(T.untyped)}
  def hash(); end

  # @return [Array<Edge>] the edges of {#graph} that have `self` as their
  #
  # ```
  # {Edge#destination}
  # ```
  sig {returns(T.untyped)}
  def incoming_edges(); end

  # @return [Array<Edge>] the edges of {#graph} that have `self` as their
  #
  # ```
  # {Edge#destination}
  # ```
  sig do
    params(
      incoming_edges: T.untyped,
    )
    .returns(T.untyped)
  end
  def incoming_edges=(incoming_edges); end

  sig do
    params(
      name: T.untyped,
      payload: T.untyped,
    )
    .void
  end
  def initialize(name, payload); end

  # @return [String] a string suitable for debugging
  sig {returns(T.untyped)}
  def inspect(); end

  # Alias for:
  # [`ancestor?`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/DependencyGraph/Vertex.html#method-i-ancestor-3F)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def is_reachable_from?(other); end

  # @return [String] the name of the vertex
  sig {returns(T.untyped)}
  def name(); end

  # @return [String] the name of the vertex
  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def name=(name); end

  # @return [Array<Edge>] the edges of {#graph} that have `self` as their
  #
  # ```
  # {Edge#origin}
  # ```
  sig {returns(T.untyped)}
  def outgoing_edges(); end

  # @return [Array<Edge>] the edges of {#graph} that have `self` as their
  #
  # ```
  # {Edge#origin}
  # ```
  sig do
    params(
      outgoing_edges: T.untyped,
    )
    .returns(T.untyped)
  end
  def outgoing_edges=(outgoing_edges); end

  # Is there a path from `self` to `other` following edges in the dependency
  # graph? @return true iff there is a path following edges within this {#graph}
  #
  # Also aliased as:
  # [`descendent?`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/DependencyGraph/Vertex.html#method-i-descendent-3F)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def path_to?(other); end

  # @return [Object] the payload the vertex holds
  sig {returns(T.untyped)}
  def payload(); end

  # @return [Object] the payload the vertex holds
  sig do
    params(
      payload: T.untyped,
    )
    .returns(T.untyped)
  end
  def payload=(payload); end

  # @return [Array<Vertex>] the vertices of {#graph} that have an edge with
  #
  # ```
  # `self` as their {Edge#destination}
  # ```
  sig {returns(T.untyped)}
  def predecessors(); end

  # @return [Set<Vertex>] the vertices of {#graph} where `self` is a
  #
  # ```
  # {#descendent?}
  # ```
  sig {returns(T.untyped)}
  def recursive_predecessors(); end

  # @return [Set<Vertex>] the vertices of {#graph} where `self` is an
  #
  # ```
  # {#ancestor?}
  # ```
  sig {returns(T.untyped)}
  def recursive_successors(); end

  # @return [Array<Object>] all of the requirements that required
  #
  # ```ruby
  # this vertex
  # ```
  sig {returns(T.untyped)}
  def requirements(); end

  # @return [Boolean] whether the vertex is considered a root vertex
  sig {returns(T.untyped)}
  def root(); end

  # @return [Boolean] whether the vertex is considered a root vertex
  sig do
    params(
      root: T.untyped,
    )
    .returns(T.untyped)
  end
  def root=(root); end

  # @return [Boolean] whether the vertex is considered a root vertex
  sig {returns(T.untyped)}
  def root?(); end

  # @param  [Vertex] other the other vertex to compare to @return [Boolean]
  # whether the two vertices are equal, determined
  #
  # ```
  # solely by {#name} and {#payload} equality
  # ```
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def shallow_eql?(other); end

  # @return [Array<Vertex>] the vertices of {#graph} that have an edge with
  #
  # ```
  # `self` as their {Edge#origin}
  # ```
  sig {returns(T.untyped)}
  def successors(); end
end

# A state that encapsulates a set of {#requirements} with an {Array} of
# possibilities
class Bundler::Molinillo::DependencyState < Bundler::Molinillo::ResolutionState
  extend T::Generic
  Elem = type_member {{fixed: T.untyped}}

  # Removes a possibility from `self` @return [PossibilityState] a state with a
  # single possibility,
  #
  # ```ruby
  # the possibility that was removed from `self`
  # ```
  sig {returns(T.untyped)}
  def pop_possibility_state(); end
end

# An error caused by searching for a dependency that is completely unknown, i.e.
# has no versions available whatsoever.
class Bundler::Molinillo::NoSuchDependencyError < Bundler::Molinillo::ResolverError
  # @return [Object] the dependency that could not be found
  sig {returns(T.untyped)}
  def dependency(); end

  # @return [Object] the dependency that could not be found
  sig do
    params(
      dependency: T.untyped,
    )
    .returns(T.untyped)
  end
  def dependency=(dependency); end

  sig do
    params(
      dependency: T.untyped,
      required_by: T.untyped,
    )
    .void
  end
  def initialize(dependency, required_by=T.unsafe(nil)); end

  # The error message for the missing dependency, including the specifications
  # that had this dependency.
  sig {returns(T.untyped)}
  def message(); end

  # @return [Array<Object>] the specifications that depended upon {#dependency}
  sig {returns(T.untyped)}
  def required_by(); end

  # @return [Array<Object>] the specifications that depended upon {#dependency}
  sig do
    params(
      required_by: T.untyped,
    )
    .returns(T.untyped)
  end
  def required_by=(required_by); end
end

# A state that encapsulates a single possibility to fulfill the given
# {#requirement}
class Bundler::Molinillo::PossibilityState < Bundler::Molinillo::ResolutionState
  extend T::Generic
  Elem = type_member {{fixed: T.untyped}}
end

class Bundler::Molinillo::ResolutionState < Struct
  extend T::Generic
  Elem = type_member {{fixed: T.untyped}}

  sig {returns(T.untyped)}
  def activated(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def activated=(_); end

  sig {returns(T.untyped)}
  def conflicts(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def conflicts=(_); end

  sig {returns(T.untyped)}
  def depth(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def depth=(_); end

  sig {returns(T.untyped)}
  def name(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def name=(_); end

  sig {returns(T.untyped)}
  def possibilities(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def possibilities=(_); end

  sig {returns(T.untyped)}
  def requirement(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def requirement=(_); end

  sig {returns(T.untyped)}
  def requirements(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def requirements=(_); end

  sig {returns(T.untyped)}
  def unused_unwind_options(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def unused_unwind_options=(_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.[](*_); end

  # Returns an empty resolution state @return [ResolutionState] an empty state
  sig {returns(T.untyped)}
  def self.empty(); end

  sig {returns(T.untyped)}
  def self.members(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.new(*_); end
end

# This class encapsulates a dependency resolver. The resolver is responsible for
# determining which set of dependencies to activate, with feedback from the
# {#specification\_provider}
class Bundler::Molinillo::Resolver
  sig do
    params(
      specification_provider: T.untyped,
      resolver_ui: T.untyped,
    )
    .void
  end
  def initialize(specification_provider, resolver_ui); end

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
  sig do
    params(
      requested: T.untyped,
      base: T.untyped,
    )
    .returns(T.untyped)
  end
  def resolve(requested, base=T.unsafe(nil)); end

  # @return [UI] the UI module used to communicate back to the user
  #
  # ```ruby
  # during the resolution process
  # ```
  sig {returns(T.untyped)}
  def resolver_ui(); end

  # @return [SpecificationProvider] the specification provider used
  #
  # ```
  # in the resolution process
  # ```
  sig {returns(T.untyped)}
  def specification_provider(); end
end

# A specific resolution from a given {Resolver}
class Bundler::Molinillo::Resolver::Resolution
  include ::Bundler::Molinillo::Delegates::SpecificationProvider
  include ::Bundler::Molinillo::Delegates::ResolutionState
  # @return [DependencyGraph] the base dependency graph to which
  #
  # ```ruby
  # dependencies should be 'locked'
  # ```
  sig {returns(T.untyped)}
  def base(); end

  sig do
    params(
      specification_provider: T.untyped,
      resolver_ui: T.untyped,
      requested: T.untyped,
      base: T.untyped,
    )
    .void
  end
  def initialize(specification_provider, resolver_ui, requested, base); end

  # @return [Integer] the number of resolver iterations in between calls to
  #
  # ```
  # {#resolver_ui}'s {UI#indicate_progress} method
  # ```
  sig do
    params(
      iteration_rate: T.untyped,
    )
    .returns(T.untyped)
  end
  def iteration_rate=(iteration_rate); end

  # @return [Array] the dependencies that were explicitly required
  sig {returns(T.untyped)}
  def original_requested(); end

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
  sig {returns(T.untyped)}
  def resolve(); end

  # @return [UI] the UI that knows how to communicate feedback about the
  #
  # ```ruby
  # resolution process back to the user
  # ```
  sig {returns(T.untyped)}
  def resolver_ui(); end

  # @return [SpecificationProvider] the provider that knows about
  #
  # ```
  # dependencies, requirements, specifications, versions, etc.
  # ```
  sig {returns(T.untyped)}
  def specification_provider(); end

  # @return [Time] the time at which resolution began
  sig do
    params(
      started_at: T.untyped,
    )
    .returns(T.untyped)
  end
  def started_at=(started_at); end

  # @return [Array<ResolutionState>] the stack of states for the resolution
  sig do
    params(
      states: T.untyped,
    )
    .returns(T.untyped)
  end
  def states=(states); end
end

class Bundler::Molinillo::Resolver::Resolution::Conflict < Struct
  extend T::Generic
  Elem = type_member {{fixed: T.untyped}}

  sig {returns(T.untyped)}
  def activated_by_name(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def activated_by_name=(_); end

  sig {returns(T.untyped)}
  def existing(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def existing=(_); end

  sig {returns(T.untyped)}
  def locked_requirement(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def locked_requirement=(_); end

  # @return [Object] a spec that was unable to be activated due to a conflict
  sig {returns(T.untyped)}
  def possibility(); end

  sig {returns(T.untyped)}
  def possibility_set(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def possibility_set=(_); end

  sig {returns(T.untyped)}
  def requirement(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def requirement=(_); end

  sig {returns(T.untyped)}
  def requirement_trees(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def requirement_trees=(_); end

  sig {returns(T.untyped)}
  def requirements(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def requirements=(_); end

  sig {returns(T.untyped)}
  def underlying_error(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def underlying_error=(_); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.[](*_); end

  sig {returns(T.untyped)}
  def self.members(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.new(*_); end
end

class Bundler::Molinillo::Resolver::Resolution::PossibilitySet < Struct
  extend T::Generic
  Elem = type_member {{fixed: T.untyped}}

  sig {returns(T.untyped)}
  def dependencies(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def dependencies=(_); end

  # @return [Object] most up-to-date dependency in the possibility set
  sig {returns(T.untyped)}
  def latest_version(); end

  sig {returns(T.untyped)}
  def possibilities(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def possibilities=(_); end

  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) representation
  # of the possibility set, for debugging
  sig {returns(T.untyped)}
  def to_s(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.[](*_); end

  sig {returns(T.untyped)}
  def self.members(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.new(*_); end
end

class Bundler::Molinillo::Resolver::Resolution::UnwindDetails < Struct
  include ::Comparable
  extend T::Generic
  Elem = type_member {{fixed: T.untyped}}

  # We compare
  # [`UnwindDetails`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo/Resolver/Resolution/UnwindDetails.html)
  # when choosing which state to unwind to. If two options have the same
  # state\_index we prefer the one most removed from a requirement that caused
  # the conflict. Both options would unwind to the same state, but a
  # `grandparent` option will filter out fewer of its possibilities after doing
  # so - where a state is both a `parent` and a `grandparent` to requirements
  # that have caused a conflict this is the correct behaviour. @param
  # [UnwindDetail] other UnwindDetail to be compared @return [Integer] integer
  # specifying ordering
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def <=>(other); end

  # @return [Array] array of all the requirements that led to the need for
  #
  # ```ruby
  # this unwind
  # ```
  sig {returns(T.untyped)}
  def all_requirements(); end

  sig {returns(T.untyped)}
  def conflicting_requirements(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def conflicting_requirements=(_); end

  sig {returns(T.untyped)}
  def requirement_tree(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def requirement_tree=(_); end

  sig {returns(T.untyped)}
  def requirement_trees(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def requirement_trees=(_); end

  sig {returns(T.untyped)}
  def requirements_unwound_to_instead(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def requirements_unwound_to_instead=(_); end

  # @return [Integer] index of state requirement in reversed requirement tree
  #
  # ```ruby
  # (the conflicting requirement itself will be at position 0)
  # ```
  sig {returns(T.untyped)}
  def reversed_requirement_tree_index(); end

  sig {returns(T.untyped)}
  def state_index(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def state_index=(_); end

  sig {returns(T.untyped)}
  def state_requirement(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def state_requirement=(_); end

  # @return [Array] array of sub-dependencies to avoid when choosing a
  #
  # ```
  # new possibility for the state we've unwound to. Only relevant for
  # non-primary unwinds
  # ```
  sig {returns(T.untyped)}
  def sub_dependencies_to_avoid(); end

  # @return [Boolean] where the requirement of the state we're unwinding
  #
  # ```
  # to directly caused the conflict. Note: in this case, it is
  # impossible for the state we're unwinding to to be a parent of
  # any of the other conflicting requirements (or we would have
  # circularity)
  # ```
  sig {returns(T.untyped)}
  def unwinding_to_primary_requirement?(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.[](*_); end

  sig {returns(T.untyped)}
  def self.members(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.new(*_); end
end

# An error that occurred during the resolution process
class Bundler::Molinillo::ResolverError < StandardError
end

# Provides information about specifications and dependencies to the resolver,
# allowing the {Resolver} class to remain generic while still providing power
# and flexibility.
#
# This module contains the methods that users of
# [`Bundler::Molinillo`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Molinillo.html)
# must to implement, using knowledge of their own model classes.
module Bundler::Molinillo::SpecificationProvider
  # Returns whether this dependency, which has no possible matching
  # specifications, can safely be ignored.
  #
  # @param [Object] dependency @return [Boolean] whether this dependency can
  # safely be skipped.
  sig do
    params(
      dependency: T.untyped,
    )
    .returns(T.untyped)
  end
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
  sig do
    params(
      specification: T.untyped,
    )
    .returns(T.untyped)
  end
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
  sig do
    params(
      dependency: T.untyped,
    )
    .returns(T.untyped)
  end
  def name_for(dependency); end

  # @return [String] the name of the source of explicit dependencies, i.e.
  #
  # ```
  # those passed to {Resolver#resolve} directly.
  # ```
  sig {returns(T.untyped)}
  def name_for_explicit_dependency_source(); end

  # @return [String] the name of the source of 'locked' dependencies, i.e.
  #
  # ```
  # those passed to {Resolver#resolve} directly as the `base`
  # ```
  sig {returns(T.untyped)}
  def name_for_locking_dependency_source(); end

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
  sig do
    params(
      requirement: T.untyped,
      activated: T.untyped,
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
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
  sig do
    params(
      dependency: T.untyped,
    )
    .returns(T.untyped)
  end
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
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)<Conflict>}]
  # conflicts @return [Array<Object>] a sorted copy of `dependencies`.
  sig do
    params(
      dependencies: T.untyped,
      activated: T.untyped,
      conflicts: T.untyped,
    )
    .returns(T.untyped)
  end
  def sort_dependencies(dependencies, activated, conflicts); end
end

# Conveys information about the resolution process to a user.
module Bundler::Molinillo::UI
  # Called after resolution ends (either successfully or with an error). By
  # default, prints a newline.
  #
  # @return [void]
  sig {returns(T.untyped)}
  def after_resolution(); end

  # Called before resolution begins.
  #
  # @return [void]
  sig {returns(T.untyped)}
  def before_resolution(); end

  # Conveys debug information to the user.
  #
  # @param [Integer] depth the current depth of the resolution process. @return
  # [void]
  sig do
    params(
      depth: T.untyped,
    )
    .returns(T.untyped)
  end
  def debug(depth=T.unsafe(nil)); end

  # Whether or not debug messages should be printed. By default, whether or not
  # the `MOLINILLO\_DEBUG` environment variable is set.
  #
  # @return [Boolean]
  sig {returns(T.untyped)}
  def debug?(); end

  # Called roughly every {#progress\_rate}, this method should convey progress
  # to the user.
  #
  # @return [void]
  sig {returns(T.untyped)}
  def indicate_progress(); end

  # The {IO} object that should be used to print output. `STDOUT`, by default.
  #
  # @return [IO]
  sig {returns(T.untyped)}
  def output(); end

  # How often progress should be conveyed to the user via {#indicate\_progress},
  # in seconds. A third of a second, by default.
  #
  # @return [Float]
  sig {returns(T.untyped)}
  def progress_rate(); end
end

# An error caused by conflicts in version
class Bundler::Molinillo::VersionConflict < Bundler::Molinillo::ResolverError
  include ::Bundler::Molinillo::Delegates::SpecificationProvider
  # @return [{String => Resolution::Conflict}] the conflicts that caused
  #
  # ```ruby
  # resolution to fail
  # ```
  sig {returns(T.untyped)}
  def conflicts(); end

  sig do
    params(
      conflicts: T.untyped,
      specification_provider: T.untyped,
    )
    .void
  end
  def initialize(conflicts, specification_provider); end

  # @return [String] An error message that includes requirement trees,
  #
  # ```
  # which is much more detailed & customizable than the default message
  # ```
  #
  # @param [Hash] opts the options to create a message with. @option opts
  # [String] :solver\_name The user-facing name of the solver @option opts
  # [String] :possibility\_type The generic name of a possibility @option opts
  # [Proc] :reduce\_trees A proc that reduced the list of requirement trees
  # @option opts [Proc] :printable\_requirement A proc that pretty-prints
  # requirements @option opts [Proc] :additional\_message\_for\_conflict A proc
  # that appends additional
  #
  # ```
  # messages for each conflict
  # ```
  #
  # @option opts [Proc] :version\_for\_spec A proc that returns the version
  # number for a
  #
  # ```ruby
  # possibility
  # ```
  sig do
    params(
      opts: T.untyped,
    )
    .returns(T.untyped)
  end
  def message_with_trees(opts=T.unsafe(nil)); end

  # @return [SpecificationProvider] the specification provider used during
  #
  # ```ruby
  # resolution
  # ```
  sig {returns(T.untyped)}
  def specification_provider(); end
end

class Bundler::NoSpaceOnDeviceError < Bundler::PermissionError
  sig {returns(T.untyped)}
  def message(); end

  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::OperationNotSupportedError < Bundler::PermissionError
  sig {returns(T.untyped)}
  def message(); end

  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::PathError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::PermissionError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def action(); end

  sig do
    params(
      path: T.untyped,
      permission_type: T.untyped,
    )
    .void
  end
  def initialize(path, permission_type=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def message(); end

  sig {returns(T.untyped)}
  def status_code(); end
end

# This is the interfacing class represents the API that we intend to provide the
# plugins to use.
#
# For plugins to be independent of the
# [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html) internals they
# shall limit their interactions to methods of this class only. This will save
# them from breaking when some internal change.
#
# Currently we are delegating the methods defined in
# [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html) class to itself.
# So, this class acts as a buffer.
#
# If there is some change in the
# [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html) class that is
# incompatible to its previous behavior or if otherwise desired, we can
# reimplement(or implement) the method to preserve compatibility.
#
# To use this, either the class can inherit this class or use it directly. For
# example of both types of use, refer the file `spec/plugins/command.rb`
#
# To use it without inheriting, you will have to create an object of this to use
# the functions (except for declaration functions like command, source, and
# hooks).
# Manages which plugins are installed and their sources. This also is supposed
# to map which plugin does what (currently the features are not implemented so
# this class is now a stub class).
# Handles the installation of plugin in appropriate directories.
#
# This class is supposed to be wrapper over the existing gem installation infra
# but currently it itself handles everything as the Source's subclasses (e.g.
# Source::RubyGems) are heavily dependent on the Gemfile.
# SourceList object to be used while parsing the Gemfile, setting the
# approptiate options to be used with Source classes for plugin installation
module Bundler::Plugin
  PLUGIN_FILE_NAME = ::T.let(nil, T.untyped)

  sig do
    params(
      command: T.untyped,
      cls: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.add_command(command, cls); end

  sig do
    params(
      event: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.add_hook(event, &block); end

  sig do
    params(
      source: T.untyped,
      cls: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.add_source(source, cls); end

  sig {returns(T.untyped)}
  def self.cache(); end

  sig do
    params(
      command: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.command?(command); end

  sig do
    params(
      command: T.untyped,
      args: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.exec_command(command, args); end

  sig do
    params(
      gemfile: T.untyped,
      inline: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.gemfile_install(gemfile=T.unsafe(nil), &inline); end

  sig {returns(T.untyped)}
  def self.global_root(); end

  sig do
    params(
      event: T.untyped,
      args: T.untyped,
      arg_blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.hook(event, *args, &arg_blk); end

  sig {returns(T.untyped)}
  def self.index(); end

  sig do
    params(
      names: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.install(names, options); end

  sig do
    params(
      plugin: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.installed?(plugin); end

  sig {returns(T.untyped)}
  def self.local_root(); end

  sig {returns(T.untyped)}
  def self.reset!(); end

  sig {returns(T.untyped)}
  def self.root(); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.source(name); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.source?(name); end

  sig do
    params(
      locked_opts: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.source_from_lock(locked_opts); end
end

class Bundler::Plugin::API
  # The cache dir to be used by the plugins for storage
  #
  # @return [Pathname] path of the cache dir
  sig {returns(T.untyped)}
  def cache_dir(); end

  sig do
    params(
      name: T.untyped,
      args: T.untyped,
      blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def method_missing(name, *args, &blk); end

  # A tmp dir to be used by plugins Accepts names that get concatenated as
  # suffix
  #
  # @return [Pathname] object for the new directory created
  sig do
    params(
      names: T.untyped,
    )
    .returns(T.untyped)
  end
  def tmp(*names); end

  # The plugins should declare that they handle a command through this helper.
  #
  # @param [String] command being handled by them @param [Class] (optional)
  # class that handles the command. If not
  #
  # ```
  # provided, the `self` class will be used.
  # ```
  sig do
    params(
      command: T.untyped,
      cls: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.command(command, cls=T.unsafe(nil)); end

  sig do
    params(
      event: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.hook(event, &block); end

  # The plugins should declare that they provide a installation source through
  # this helper.
  #
  # @param [String] the source type they provide @param [Class] (optional) class
  # that handles the source. If not
  #
  # ```
  # provided, the `self` class will be used.
  # ```
  sig do
    params(
      source: T.untyped,
      cls: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.source(source, cls=T.unsafe(nil)); end
end

# Dsl to parse the Gemfile looking for plugins to install
class Bundler::Plugin::DSL < Bundler::Dsl
  sig do
    params(
      name: T.untyped,
      args: T.untyped
    )
    .returns(T.untyped)
  end
  def _gem(name, *args); end

  # This lists the plugins that was added automatically and not specified by the
  # user.
  #
  # When we encounter :type attribute with a source block, we add a plugin by
  # name bundler-source-<type> to list of plugins to be installed.
  #
  # These plugins are optional and are not installed when there is conflict with
  # any other plugin.
  sig { returns(T.untyped) }
  def inferred_plugins(); end

  sig do
    params(
      name: T.untyped,
      args: T.untyped
    )
    .returns(T.untyped)
  end
  def plugin(name, *args); end
end

module Bundler::Plugin::Events
  # Check if an event has been defined @param event [String] An event to check
  # @return [Boolean] A boolean indicating if the event has been defined
  def self.defined_event?(event); end
end

class Bundler::Plugin::Index
  # Fetch the name of plugin handling the command
  def command_plugin(command); end

  def commands(); end

  # Path where the global index file is stored
  def global_index_file(); end

  # Returns the list of plugin names handling the passed event
  def hook_plugins(event); end

  # Path of default index file
  def index_file(); end

  def installed?(name); end

  def load_paths(name); end

  # Path where the local index file is stored
  def local_index_file(); end

  def plugin_path(name); end

  # This function is to be called when a new plugin is installed. This function
  # shall add the functions of the plugin to existing maps and also the name to
  # source location.
  #
  # @param [String] name of the plugin to be registered @param [String] path
  # where the plugin is installed @param [Array<String>]
  # [`load_paths`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Plugin/Index.html#method-i-load_paths)
  # for the plugin @param [Array<String>] commands that are handled by the
  # plugin @param [Array<String>] sources that are handled by the plugin
  def register_plugin(name, path, load_paths, commands, sources, hooks); end

  def source?(source); end

  def source_plugin(name); end
end

class Bundler::Plugin::MalformattedPlugin < Bundler::PluginError
end

class Bundler::Plugin::UndefinedCommandError < Bundler::PluginError
end

class Bundler::Plugin::UnknownSourceError < Bundler::PluginError
end

class Bundler::Plugin::DSL::PluginGemfileError < Bundler::PluginError
end

class Bundler::PluginError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::ProductionError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

# Represents a lazily loaded gem specification, where the full specification is
# on the source server in rubygems' "quick" index. The proxy object is to be
# seeded with what we're given from the source's abbreviated index - the full
# specification will only be fetched when necessary.
class Bundler::RemoteSpecification
  include ::Comparable
  include ::Bundler::MatchPlatform
  include ::Bundler::GemHelpers
  # Compare this specification against another object. Using
  # [`sort_obj`](https://docs.ruby-lang.org/en/2.7.0/Bundler/RemoteSpecification.html#method-i-sort_obj)
  # is compatible with
  # [`Gem::Specification`](https://docs.ruby-lang.org/en/2.7.0/Gem/Specification.html)
  # and other [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html) or
  # RubyGems objects. Otherwise, use the default
  # [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) comparison.
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def <=>(other); end

  # Because Rubyforge cannot be trusted to provide valid specifications once the
  # remote gem is downloaded, the backend specification will be swapped out.
  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def __swap__(spec); end

  sig {returns(T.untyped)}
  def dependencies(); end

  sig do
    params(
      dependencies: T.untyped,
    )
    .returns(T.untyped)
  end
  def dependencies=(dependencies); end

  # Needed before installs, since the arch matters then and quick specs don't
  # bother to include the arch in the platform string
  sig {returns(T.untyped)}
  def fetch_platform(); end

  sig {returns(T.untyped)}
  def full_name(); end

  sig {returns(T.untyped)}
  def git_version(); end

  sig do
    params(
      name: T.untyped,
      version: T.untyped,
      platform: T.untyped,
      spec_fetcher: T.untyped,
    )
    .void
  end
  def initialize(name, version, platform, spec_fetcher); end

  sig {returns(T.untyped)}
  def name(); end

  sig {returns(T.untyped)}
  def platform(); end

  sig {returns(T.untyped)}
  def remote(); end

  sig do
    params(
      remote: T.untyped,
    )
    .returns(T.untyped)
  end
  def remote=(remote); end

  sig do
    params(
      method: T.untyped,
      include_all: T.untyped,
    )
    .returns(T.untyped)
  end
  def respond_to?(method, include_all=T.unsafe(nil)); end

  # Create a delegate used for sorting. This strategy is copied from RubyGems
  # 2.23 and ensures that Bundler's specifications can be compared and sorted
  # with RubyGems' own specifications.
  #
  # @see #<=> @see
  # [`Gem::Specification#sort_obj`](https://docs.ruby-lang.org/en/2.7.0/Gem/Specification.html#method-i-sort_obj)
  #
  # @return [Array] an object you can use to compare and sort this
  #
  # ```ruby
  # specification against other specifications
  # ```
  sig {returns(T.untyped)}
  def sort_obj(); end

  sig {returns(T.untyped)}
  def source(); end

  sig do
    params(
      source: T.untyped,
    )
    .returns(T.untyped)
  end
  def source=(source); end

  sig {returns(T.untyped)}
  def to_s(); end

  sig {returns(T.untyped)}
  def version(); end
end

class Bundler::Resolver
  include ::Bundler::Molinillo::SpecificationProvider
  include ::Bundler::Molinillo::UI
  sig {returns(T.untyped)}
  def after_resolution(); end

  sig {returns(T.untyped)}
  def before_resolution(); end

  # Conveys debug information to the user.
  #
  # @param [Integer] depth the current depth of the resolution process. @return
  # [void]
  sig do
    params(
      depth: T.untyped,
    )
    .returns(T.untyped)
  end
  def debug(depth=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def debug?(); end

  sig do
    params(
      specification: T.untyped,
    )
    .returns(T.untyped)
  end
  def dependencies_for(specification); end

  sig do
    params(
      dependency: T.untyped,
    )
    .returns(T.untyped)
  end
  def index_for(dependency); end

  sig {returns(T.untyped)}
  def indicate_progress(); end

  sig do
    params(
      index: T.untyped,
      source_requirements: T.untyped,
      base: T.untyped,
      gem_version_promoter: T.untyped,
      additional_base_requirements: T.untyped,
      platforms: T.untyped,
    )
    .void
  end
  def initialize(index, source_requirements, base, gem_version_promoter, additional_base_requirements, platforms); end

  sig do
    params(
      dependency: T.untyped,
    )
    .returns(T.untyped)
  end
  def name_for(dependency); end

  sig {returns(T.untyped)}
  def name_for_explicit_dependency_source(); end

  sig {returns(T.untyped)}
  def name_for_locking_dependency_source(); end

  sig do
    params(
      vertex: T.untyped,
    )
    .returns(T.untyped)
  end
  def relevant_sources_for_vertex(vertex); end

  sig do
    params(
      requirement: T.untyped,
      activated: T.untyped,
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def requirement_satisfied_by?(requirement, activated, spec); end

  sig do
    params(
      dependency: T.untyped,
    )
    .returns(T.untyped)
  end
  def search_for(dependency); end

  sig do
    params(
      dependencies: T.untyped,
      activated: T.untyped,
      conflicts: T.untyped,
    )
    .returns(T.untyped)
  end
  def sort_dependencies(dependencies, activated, conflicts); end

  sig do
    params(
      requirements: T.untyped,
    )
    .returns(T.untyped)
  end
  def start(requirements); end

  sig do
    params(
      platform: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.platform_sort_key(platform); end

  # Figures out the best possible configuration of gems that satisfies the list
  # of passed dependencies and any child dependencies without causing any gem
  # activation errors.
  #
  # #### Parameters
  # \*dependencies<Gem::Dependency>
  # :   The list of dependencies to resolve
  #
  #
  # #### Returns
  # <GemBundle>,nil
  # :   If the list of dependencies can be resolved, a
  #
  # ```
  # collection of gemspecs is returned. Otherwise, nil is returned.
  # ```
  sig do
    params(
      requirements: T.untyped,
      index: T.untyped,
      source_requirements: T.untyped,
      base: T.untyped,
      gem_version_promoter: T.untyped,
      additional_base_requirements: T.untyped,
      platforms: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.resolve(requirements, index, source_requirements=T.unsafe(nil), base=T.unsafe(nil), gem_version_promoter=T.unsafe(nil), additional_base_requirements=T.unsafe(nil), platforms=T.unsafe(nil)); end

  # Sort platforms from most general to most specific
  sig do
    params(
      platforms: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.sort_platforms(platforms); end
end

class Bundler::Resolver::SpecGroup
  include ::Bundler::GemHelpers
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def ==(other); end

  sig do
    params(
      platform: T.untyped,
    )
    .returns(T.untyped)
  end
  def activate_platform!(platform); end

  sig {returns(T.untyped)}
  def dependencies_for_activated_platforms(); end

  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def eql?(other); end

  sig do
    params(
      platform: T.untyped,
    )
    .returns(T.untyped)
  end
  def for?(platform); end

  sig {returns(T.untyped)}
  def hash(); end

  sig {returns(T.untyped)}
  def ignores_bundler_dependencies(); end

  sig do
    params(
      ignores_bundler_dependencies: T.untyped,
    )
    .returns(T.untyped)
  end
  def ignores_bundler_dependencies=(ignores_bundler_dependencies); end

  sig do
    params(
      all_specs: T.untyped,
    )
    .void
  end
  def initialize(all_specs); end

  sig {returns(T.untyped)}
  def name(); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def name=(name); end

  sig {returns(T.untyped)}
  def source(); end

  sig do
    params(
      source: T.untyped,
    )
    .returns(T.untyped)
  end
  def source=(source); end

  sig {returns(T.untyped)}
  def to_s(); end

  sig {returns(T.untyped)}
  def to_specs(); end

  sig {returns(T.untyped)}
  def version(); end

  sig do
    params(
      version: T.untyped,
    )
    .returns(T.untyped)
  end
  def version=(version); end
end

module Bundler::RubyDsl
  sig do
    params(
      ruby_version: T.untyped,
    )
    .returns(T.untyped)
  end
  def ruby(*ruby_version); end
end

class Bundler::RubyVersion
  # @private
  PATTERN = ::T.let(nil, T.untyped)

  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def ==(other); end

  # Returns a tuple of these things:
  #
  # ```ruby
  # [diff, this, other]
  # The priority of attributes are
  # 1. engine
  # 2. ruby_version
  # 3. engine_version
  # ```
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def diff(other); end

  sig {returns(T.untyped)}
  def engine(); end

  sig {returns(T.untyped)}
  def engine_gem_version(); end

  sig {returns(T.untyped)}
  def engine_versions(); end

  sig {returns(T.untyped)}
  def exact?(); end

  sig {returns(T.untyped)}
  def gem_version(); end

  sig {returns(T.untyped)}
  def host(); end

  sig do
    params(
      versions: T.untyped,
      patchlevel: T.untyped,
      engine: T.untyped,
      engine_version: T.untyped,
    )
    .void
  end
  def initialize(versions, patchlevel, engine, engine_version); end

  sig {returns(T.untyped)}
  def patchlevel(); end

  sig {returns(T.untyped)}
  def single_version_string(); end

  sig {returns(T.untyped)}
  def to_gem_version_with_patchlevel(); end

  sig do
    params(
      versions: T.untyped,
    )
    .returns(T.untyped)
  end
  def to_s(versions=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def versions(); end

  sig do
    params(
      versions: T.untyped,
    )
    .returns(T.untyped)
  end
  def versions_string(versions); end

  # Returns a
  # [`RubyVersion`](https://docs.ruby-lang.org/en/2.7.0/Bundler/RubyVersion.html)
  # from the given string. @param [String] the version string to match. @return
  # [RubyVersion,Nil] The version if the string is a valid
  # [`RubyVersion`](https://docs.ruby-lang.org/en/2.7.0/Bundler/RubyVersion.html)
  #
  # ```
  # description, and nil otherwise.
  # ```
  sig do
    params(
      string: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.from_string(string); end

  sig {returns(T.untyped)}
  def self.system(); end
end

class Bundler::RubyVersionMismatch < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::RubygemsIntegration
  EXT_LOCK = ::T.let(nil, T.untyped)

  # This backports base\_dir which replaces installation path RubyGems 1.8+
  sig {returns(T.untyped)}
  def backport_base_dir(); end

  sig {returns(T.untyped)}
  def backport_cache_file(); end

  # This backports the correct segment generation code from RubyGems 1.4+ by
  # monkeypatching it into the method in RubyGems 1.3.6 and 1.3.7.
  sig {returns(T.untyped)}
  def backport_segment_generation(); end

  sig {returns(T.untyped)}
  def backport_spec_file(); end

  # This backport fixes the marshaling of @segments.
  sig {returns(T.untyped)}
  def backport_yaml_initialize(); end

  sig do
    params(
      gem: T.untyped,
      bin: T.untyped,
      ver: T.untyped,
    )
    .returns(T.untyped)
  end
  def bin_path(gem, bin, ver); end

  sig {returns(T.untyped)}
  def binstubs_call_gem?(); end

  sig do
    params(
      spec: T.untyped,
      skip_validation: T.untyped,
    )
    .returns(T.untyped)
  end
  def build(spec, skip_validation=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def build_args(); end

  sig do
    params(
      args: T.untyped,
    )
    .returns(T.untyped)
  end
  def build_args=(args); end

  sig do
    params(
      gem_dir: T.untyped,
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def build_gem(gem_dir, spec); end

  sig {returns(T.untyped)}
  def clear_paths(); end

  sig {returns(T.untyped)}
  def config_map(); end

  sig {returns(T.untyped)}
  def configuration(); end

  sig do
    params(
      spec: T.untyped,
      uri: T.untyped,
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def download_gem(spec, uri, path); end

  sig {returns(T.untyped)}
  def ext_lock(); end

  # TODO: This is for older versions of RubyGems... should we support the
  # X-Gemfile-Source header on these old versions? Maybe the newer
  # implementation will work on older RubyGems? It seems difficult to keep this
  # implementation and still send the header.
  sig do
    params(
      remote: T.untyped,
    )
    .returns(T.untyped)
  end
  def fetch_all_remote_specs(remote); end

  sig {returns(T.untyped)}
  def fetch_prerelease_specs(); end

  sig do
    params(
      all: T.untyped,
      pre: T.untyped,
      blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def fetch_specs(all, pre, &blk); end

  sig {returns(T.untyped)}
  def gem_bindir(); end

  sig {returns(T.untyped)}
  def gem_cache(); end

  sig {returns(T.untyped)}
  def gem_dir(); end

  sig do
    params(
      path: T.untyped,
      policy: T.untyped,
    )
    .returns(T.untyped)
  end
  def gem_from_path(path, policy=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def gem_path(); end

  sig do
    params(
      obj: T.untyped,
    )
    .returns(T.untyped)
  end
  def inflate(obj); end

  sig {void}
  def initialize(); end

  sig do
    params(
      args: T.untyped,
    )
    .returns(T.untyped)
  end
  def install_with_build_args(args); end

  sig {returns(T.untyped)}
  def load_path_insert_index(); end

  sig do
    params(
      files: T.untyped,
    )
    .returns(T.untyped)
  end
  def load_plugin_files(files); end

  sig {returns(T.untyped)}
  def load_plugins(); end

  sig {returns(T.untyped)}
  def loaded_gem_paths(); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def loaded_specs(name); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def mark_loaded(spec); end

  sig {returns(T.untyped)}
  def marshal_spec_dir(); end

  sig do
    params(
      klass: T.untyped,
      method: T.untyped,
    )
    .returns(T.untyped)
  end
  def method_visibility(klass, method); end

  sig do
    params(
      obj: T.untyped,
    )
    .returns(T.untyped)
  end
  def path(obj); end

  sig {returns(T.untyped)}
  def path_separator(); end

  sig {returns(T.untyped)}
  def platforms(); end

  sig {returns(T.untyped)}
  def post_reset_hooks(); end

  sig {returns(T.untyped)}
  def preserve_paths(); end

  sig do
    params(
      req_str: T.untyped,
    )
    .returns(T.untyped)
  end
  def provides?(req_str); end

  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def read_binary(path); end

  sig do
    params(
      klass: T.untyped,
      method: T.untyped,
      unbound_method: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def redefine_method(klass, method, unbound_method=T.unsafe(nil), &block); end

  # Used to make bin stubs that are not created by bundler work under bundler.
  # The new
  # [`Gem.bin_path`](https://docs.ruby-lang.org/en/2.7.0/Gem.html#method-c-bin_path)
  # only considers gems in `specs`
  sig do
    params(
      specs: T.untyped,
      specs_by_name: T.untyped,
    )
    .returns(T.untyped)
  end
  def replace_bin_path(specs, specs_by_name); end

  # Replace or hook into RubyGems to provide a bundlerized view of the world.
  sig do
    params(
      specs: T.untyped,
    )
    .returns(T.untyped)
  end
  def replace_entrypoints(specs); end

  sig do
    params(
      specs: T.untyped,
      specs_by_name: T.untyped,
    )
    .returns(T.untyped)
  end
  def replace_gem(specs, specs_by_name); end

  # Because [`Bundler`](https://docs.ruby-lang.org/en/2.6.0/Bundler.html) has a
  # static view of what specs are available, we don't refresh, so stub it out.
  sig {returns(T.untyped)}
  def replace_refresh(); end

  sig {returns(T.untyped)}
  def repository_subdirectories(); end

  sig {returns(T.untyped)}
  def reset(); end

  sig {returns(T.untyped)}
  def reverse_rubygems_kernel_mixin(); end

  sig {returns(T.untyped)}
  def ruby_engine(); end

  sig {returns(T.untyped)}
  def security_policies(); end

  sig {returns(T.untyped)}
  def security_policy_keys(); end

  sig do
    params(
      spec: T.untyped,
      installed_by_version: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_installed_by_version(spec, installed_by_version=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def sources(); end

  sig do
    params(
      val: T.untyped,
    )
    .returns(T.untyped)
  end
  def sources=(val); end

  sig {returns(T.untyped)}
  def spec_cache_dirs(); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def spec_default_gem?(spec); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def spec_extension_dir(spec); end

  sig do
    params(
      path: T.untyped,
      policy: T.untyped,
    )
    .returns(T.untyped)
  end
  def spec_from_gem(path, policy=T.unsafe(nil)); end

  sig do
    params(
      spec: T.untyped,
      glob: T.untyped,
    )
    .returns(T.untyped)
  end
  def spec_matches_for_glob(spec, glob); end

  sig do
    params(
      spec: T.untyped,
      default: T.untyped,
    )
    .returns(T.untyped)
  end
  def spec_missing_extensions?(spec, default=T.unsafe(nil)); end

  sig do
    params(
      stub: T.untyped,
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def stub_set_spec(stub, spec); end

  sig do
    params(
      specs: T.untyped,
    )
    .returns(T.untyped)
  end
  def stub_source_index(specs); end

  sig {returns(T.untyped)}
  def stubs_provide_full_functionality?(); end

  sig {returns(T.untyped)}
  def suffix_pattern(); end

  sig do
    params(
      obj: T.untyped,
    )
    .returns(T.untyped)
  end
  def ui=(obj); end

  sig {returns(T.untyped)}
  def undo_replacements(); end

  sig {returns(T.untyped)}
  def user_home(); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def validate(spec); end

  sig {returns(T.untyped)}
  def version(); end

  sig do
    params(
      args: T.untyped,
    )
    .returns(T.untyped)
  end
  def with_build_args(args); end

  sig do
    params(
      req_str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.provides?(req_str); end

  sig {returns(T.untyped)}
  def self.version(); end
end

# RubyGems 1.8.0 to 1.8.4
class Bundler::RubygemsIntegration::AlmostModern < Bundler::RubygemsIntegration::Modern
  # RubyGems [>= 1.8.0, < 1.8.5] has a bug that changes
  # [`Gem.dir`](https://docs.ruby-lang.org/en/2.6.0/Gem.html#method-c-dir)
  # whenever you call
  # [`Gem::Installer#install`](https://docs.ruby-lang.org/en/2.6.0/Installer.html#method-i-install)
  # with an :install\_dir set. We have to change it back for our sudo mode to
  # work.
  sig {returns(T.untyped)}
  def preserve_paths(); end
end

# RubyGems versions 1.3.6 and 1.3.7
class Bundler::RubygemsIntegration::Ancient < Bundler::RubygemsIntegration::Legacy
  sig {void}
  def initialize(); end
end

# RubyGems 2.0
class Bundler::RubygemsIntegration::Future < Bundler::RubygemsIntegration
  sig {returns(T.untyped)}
  def all_specs(); end

  sig do
    params(
      spec: T.untyped,
      skip_validation: T.untyped,
    )
    .returns(T.untyped)
  end
  def build(spec, skip_validation=T.unsafe(nil)); end

  sig do
    params(
      spec: T.untyped,
      uri: T.untyped,
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def download_gem(spec, uri, path); end

  sig do
    params(
      remote: T.untyped,
    )
    .returns(T.untyped)
  end
  def fetch_all_remote_specs(remote); end

  sig do
    params(
      source: T.untyped,
      remote: T.untyped,
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def fetch_specs(source, remote, name); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def find_name(name); end

  sig do
    params(
      path: T.untyped,
      policy: T.untyped,
    )
    .returns(T.untyped)
  end
  def gem_from_path(path, policy=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def gem_remote_fetcher(); end

  sig do
    params(
      args: T.untyped,
    )
    .returns(T.untyped)
  end
  def install_with_build_args(args); end

  sig {returns(T.untyped)}
  def path_separator(); end

  sig {returns(T.untyped)}
  def repository_subdirectories(); end

  sig do
    params(
      specs: T.untyped,
    )
    .returns(T.untyped)
  end
  def stub_rubygems(specs); end
end

# RubyGems 1.4 through 1.6
class Bundler::RubygemsIntegration::Legacy < Bundler::RubygemsIntegration
  sig {returns(T.untyped)}
  def all_specs(); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def find_name(name); end

  sig {void}
  def initialize(); end

  sig {returns(T.untyped)}
  def post_reset_hooks(); end

  sig {returns(T.untyped)}
  def reset(); end

  sig do
    params(
      specs: T.untyped,
    )
    .returns(T.untyped)
  end
  def stub_rubygems(specs); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def validate(spec); end
end

# RubyGems 1.8.5-1.8.19
class Bundler::RubygemsIntegration::Modern < Bundler::RubygemsIntegration
  sig {returns(T.untyped)}
  def all_specs(); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def find_name(name); end

  sig do
    params(
      specs: T.untyped,
    )
    .returns(T.untyped)
  end
  def stub_rubygems(specs); end
end

# RubyGems 2.1.0
class Bundler::RubygemsIntegration::MoreFuture < Bundler::RubygemsIntegration::Future
  sig {returns(T.untyped)}
  def all_specs(); end

  # RubyGems-generated binstubs call
  # [`Kernel#gem`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-gem)
  sig {returns(T.untyped)}
  def binstubs_call_gem?(); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def find_name(name); end

  sig {void}
  def initialize(); end

  # only 2.5.2+ has all of the stub methods we want to use, and since this is a
  # performance optimization *only*, we'll restrict ourselves to the most recent
  # RG versions instead of all versions that have stubs
  sig {returns(T.untyped)}
  def stubs_provide_full_functionality?(); end

  sig do
    params(
      gemfile: T.untyped,
    )
    .returns(T.untyped)
  end
  def use_gemdeps(gemfile); end
end

# RubyGems 1.8.20+
class Bundler::RubygemsIntegration::MoreModern < Bundler::RubygemsIntegration::Modern
  # RubyGems 1.8.20 and adds the skip\_validation parameter, so that's when we
  # start passing it through.
  sig do
    params(
      spec: T.untyped,
      skip_validation: T.untyped,
    )
    .returns(T.untyped)
  end
  def build(spec, skip_validation=T.unsafe(nil)); end
end

# RubyGems 1.7
class Bundler::RubygemsIntegration::Transitional < Bundler::RubygemsIntegration::Legacy
  sig do
    params(
      specs: T.untyped,
    )
    .returns(T.untyped)
  end
  def stub_rubygems(specs); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def validate(spec); end
end

class Bundler::Runtime
  include ::Bundler::SharedHelpers
  REQUIRE_ERRORS = ::T.let(nil, T.untyped)

  sig do
    params(
      custom_path: T.untyped,
    )
    .returns(T.untyped)
  end
  def cache(custom_path=T.unsafe(nil)); end

  sig do
    params(
      dry_run: T.untyped,
    )
    .returns(T.untyped)
  end
  def clean(dry_run=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def current_dependencies(); end

  sig {returns(T.untyped)}
  def dependencies(); end

  sig {returns(T.untyped)}
  def gems(); end

  sig do
    params(
      root: T.untyped,
      definition: T.untyped,
    )
    .void
  end
  def initialize(root, definition); end

  sig do
    params(
      opts: T.untyped,
    )
    .returns(T.untyped)
  end
  def lock(opts=T.unsafe(nil)); end

  sig do
    params(
      cache_path: T.untyped,
    )
    .returns(T.untyped)
  end
  def prune_cache(cache_path); end

  sig {returns(T.untyped)}
  def requested_specs(); end

  sig do
    params(
      groups: T.untyped,
    )
    .returns(T.untyped)
  end
  def require(*groups); end

  sig {returns(T.untyped)}
  def requires(); end

  sig do
    params(
      groups: T.untyped,
    )
    .returns(T.untyped)
  end
  def setup(*groups); end

  sig {returns(T.untyped)}
  def specs(); end
end

class Bundler::SecurityError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::Settings
  ARRAY_KEYS = ::T.let(nil, T.untyped)
  BOOL_KEYS = ::T.let(nil, T.untyped)
  CONFIG_REGEX = ::T.let(nil, T.untyped)
  DEFAULT_CONFIG = ::T.let(nil, T.untyped)
  NORMALIZE_URI_OPTIONS_PATTERN = ::T.let(nil, T.untyped)
  NUMBER_KEYS = ::T.let(nil, T.untyped)
  PER_URI_OPTIONS = ::T.let(nil, T.untyped)

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def [](name); end

  sig {returns(T.untyped)}
  def all(); end

  sig {returns(T.untyped)}
  def allow_sudo?(); end

  sig {returns(T.untyped)}
  def app_cache_path(); end

  sig do
    params(
      uri: T.untyped,
    )
    .returns(T.untyped)
  end
  def credentials_for(uri); end

  sig {returns(T.untyped)}
  def gem_mirrors(); end

  sig {returns(T.untyped)}
  def ignore_config?(); end

  sig do
    params(
      root: T.untyped,
    )
    .void
  end
  def initialize(root=T.unsafe(nil)); end

  sig do
    params(
      key: T.untyped,
    )
    .returns(T.untyped)
  end
  def key_for(key); end

  sig {returns(T.untyped)}
  def local_overrides(); end

  sig do
    params(
      key: T.untyped,
    )
    .returns(T.untyped)
  end
  def locations(key); end

  sig do
    params(
      uri: T.untyped,
    )
    .returns(T.untyped)
  end
  def mirror_for(uri); end

  # for legacy reasons, in
  # [`Bundler`](https://docs.ruby-lang.org/en/2.7.0/Bundler.html) 2, we do not
  # respect :disable\_shared\_gems
  sig {returns(T.untyped)}
  def path(); end

  sig do
    params(
      exposed_key: T.untyped,
    )
    .returns(T.untyped)
  end
  def pretty_values_for(exposed_key); end

  sig do
    params(
      key: T.untyped,
      value: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_command_option(key, value); end

  sig do
    params(
      key: T.untyped,
      value: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_command_option_if_given(key, value); end

  sig do
    params(
      key: T.untyped,
      value: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_global(key, value); end

  sig do
    params(
      key: T.untyped,
      value: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_local(key, value); end

  sig do
    params(
      update: T.untyped,
    )
    .returns(T.untyped)
  end
  def temporary(update); end

  sig {returns(T.untyped)}
  def validate!(); end

  # TODO: duplicates Rubygems#normalize\_uri TODO: is this the correct place to
  # validate mirror URIs?
  sig do
    params(
      uri: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.normalize_uri(uri); end
end

class Bundler::Settings::Path < Struct
  extend T::Generic
  Elem = type_member {{fixed: T.untyped}}

  sig {returns(T.untyped)}
  def append_ruby_scope(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def append_ruby_scope=(_); end

  sig {returns(T.untyped)}
  def base_path(); end

  sig {returns(T.untyped)}
  def base_path_relative_to_pwd(); end

  sig {returns(T.untyped)}
  def default_install_uses_path(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def default_install_uses_path=(_); end

  sig {returns(T.untyped)}
  def explicit_path(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def explicit_path=(_); end

  sig {returns(T.untyped)}
  def path(); end

  sig {returns(T.untyped)}
  def system_path(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def system_path=(_); end

  sig {returns(T.untyped)}
  def use_system_gems?(); end

  sig {returns(T.untyped)}
  def validate!(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.[](*_); end

  sig {returns(T.untyped)}
  def self.members(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.new(*_); end
end

module Bundler::SharedHelpers
  extend ::Bundler::SharedHelpers
  sig do
    params(
      dir: T.untyped,
      blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def chdir(dir, &blk); end

  sig do
    params(
      constant_name: T.untyped,
      namespace: T.untyped,
    )
    .returns(T.untyped)
  end
  def const_get_safely(constant_name, namespace); end

  sig {returns(T.untyped)}
  def default_bundle_dir(); end

  sig {returns(T.untyped)}
  def default_gemfile(); end

  sig {returns(T.untyped)}
  def default_lockfile(); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def digest(name); end

  sig do
    params(
      spec: T.untyped,
      old_deps: T.untyped,
      new_deps: T.untyped,
    )
    .returns(T.untyped)
  end
  def ensure_same_dependencies(spec, old_deps, new_deps); end

  # Rescues permissions errors raised by file system operations (ie.
  # Errno:EACCESS, Errno::EAGAIN) and raises more friendly errors instead.
  #
  # @param path [String] the path that the action will be attempted to @param
  # action [Symbol, to\_s] the type of operation that will be
  #
  # ```
  # performed. For example: :write, :read, :exec
  # ```
  #
  # @yield path
  #
  # @raise [Bundler::PermissionError] if Errno:EACCES is raised in the
  #
  # ```ruby
  # given block
  # ```
  #
  # @raise [Bundler::TemporaryResourceError] if Errno:EAGAIN is raised in the
  #
  # ```ruby
  # given block
  # ```
  #
  # @example
  #
  # ```ruby
  # filesystem_access("vendor/cache", :write) do
  #   FileUtils.mkdir_p("vendor/cache")
  # end
  # ```
  #
  # @see {Bundler::PermissionError}
  sig do
    params(
      path: T.untyped,
      action: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def filesystem_access(path, action=T.unsafe(nil), &block); end

  sig {returns(T.untyped)}
  def in_bundle?(); end

  sig do
    params(
      major_version: T.untyped,
      message: T.untyped,
    )
    .returns(T.untyped)
  end
  def major_deprecation(major_version, message); end

  sig {returns(T.untyped)}
  def md5_available?(); end

  sig do
    params(
      dep: T.untyped,
      print_source: T.untyped,
    )
    .returns(T.untyped)
  end
  def pretty_dependency(dep, print_source=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def print_major_deprecations!(); end

  sig {returns(T.untyped)}
  def pwd(); end

  sig {returns(T.untyped)}
  def root(); end

  sig {returns(T.untyped)}
  def set_bundle_environment(); end

  sig do
    params(
      key: T.untyped,
      value: T.untyped,
    )
    .returns(T.untyped)
  end
  def set_env(key, value); end

  sig do
    params(
      signal: T.untyped,
      override: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def trap(signal, override=T.unsafe(nil), &block); end

  sig do
    params(
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def with_clean_git_env(&block); end

  sig do
    params(
      gemfile_path: T.untyped,
      contents: T.untyped,
    )
    .returns(T.untyped)
  end
  def write_to_gemfile(gemfile_path, contents); end
end

class Bundler::Source
  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def can_lock?(spec); end

  sig {returns(T.untyped)}
  def dependency_names(); end

  sig do
    params(
      dependency_names: T.untyped,
    )
    .returns(T.untyped)
  end
  def dependency_names=(dependency_names); end

  sig {returns(T.untyped)}
  def dependency_names_to_double_check(); end

  # it's possible that gems from one source depend on gems from some other
  # source, so now we download gemspecs and iterate over those dependencies,
  # looking for gems we don't have info on yet.
  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def double_check_for(*_); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def extension_cache_path(spec); end

  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def include?(other); end

  sig {returns(T.untyped)}
  def inspect(); end

  sig {returns(T.untyped)}
  def path?(); end

  sig {returns(T.untyped)}
  def unmet_deps(); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def version_message(spec); end
end

class Bundler::Source::Gemspec < Bundler::Source::Path
  sig {returns(T.untyped)}
  def as_path_source(); end

  sig {returns(T.untyped)}
  def gemspec(); end

  sig do
    params(
      options: T.untyped,
    )
    .void
  end
  def initialize(options); end
end

class Bundler::Source::Git < Bundler::Source::Path
  # Alias for:
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Git.html#method-i-eql-3F)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def ==(other); end

  sig {returns(T.untyped)}
  def allow_git_ops?(); end

  sig {returns(T.untyped)}
  def app_cache_dirname(); end

  sig {returns(T.untyped)}
  def branch(); end

  sig do
    params(
      spec: T.untyped,
      custom_path: T.untyped,
    )
    .returns(T.untyped)
  end
  def cache(spec, custom_path=T.unsafe(nil)); end

  # This is the path which is going to contain a cache of the git repository.
  # When using the same git repository across different projects, this cache
  # will be shared. When using local git repos, this is set to the local repo.
  sig {returns(T.untyped)}
  def cache_path(); end

  # Also aliased as:
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Git.html#method-i-3D-3D)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def eql?(other); end

  sig {returns(T.untyped)}
  def extension_dir_name(); end

  sig {returns(T.untyped)}
  def hash(); end

  sig do
    params(
      options: T.untyped,
    )
    .void
  end
  def initialize(options); end

  sig do
    params(
      spec: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def install(spec, options=T.unsafe(nil)); end

  # This is the path which is going to contain a specific checkout of the git
  # repository. When using local git repos, this is set to the local repo.
  #
  # Also aliased as:
  # [`path`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Git.html#method-i-path)
  sig {returns(T.untyped)}
  def install_path(); end

  sig {returns(T.untyped)}
  def load_spec_files(); end

  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def local_override!(path); end

  sig {returns(T.untyped)}
  def name(); end

  sig {returns(T.untyped)}
  def options(); end

  # Alias for:
  # [`install_path`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Git.html#method-i-install_path)
  sig {returns(T.untyped)}
  def path(); end

  sig {returns(T.untyped)}
  def ref(); end

  sig {returns(T.untyped)}
  def revision(); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def specs(*_); end

  sig {returns(T.untyped)}
  def submodules(); end

  sig {returns(T.untyped)}
  def to_lock(); end

  sig {returns(T.untyped)}
  def to_s(); end

  sig {returns(T.untyped)}
  def unlock!(); end

  sig {returns(T.untyped)}
  def uri(); end

  sig do
    params(
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.from_lock(options); end
end

class Bundler::Source::Git::GitCommandError < Bundler::GitError
  sig do
    params(
      command: T.untyped,
      path: T.untyped,
      extra_info: T.untyped,
    )
    .void
  end
  def initialize(command, path=T.unsafe(nil), extra_info=T.unsafe(nil)); end
end

class Bundler::Source::Git::GitNotAllowedError < Bundler::GitError
  sig do
    params(
      command: T.untyped,
    )
    .void
  end
  def initialize(command); end
end

class Bundler::Source::Git::GitNotInstalledError < Bundler::GitError
  sig {void}
  def initialize(); end
end

# The
# [`GitProxy`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Git/GitProxy.html)
# is responsible to interact with git repositories. All actions required by the
# [`Git`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Git.html) source is
# encapsulated in this object.
class Bundler::Source::Git::GitProxy
  sig {returns(T.untyped)}
  def branch(); end

  sig {returns(T.untyped)}
  def checkout(); end

  sig do
    params(
      commit: T.untyped,
    )
    .returns(T.untyped)
  end
  def contains?(commit); end

  sig do
    params(
      destination: T.untyped,
      submodules: T.untyped,
    )
    .returns(T.untyped)
  end
  def copy_to(destination, submodules=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def full_version(); end

  sig do
    params(
      path: T.untyped,
      uri: T.untyped,
      ref: T.untyped,
      revision: T.untyped,
      git: T.untyped,
    )
    .void
  end
  def initialize(path, uri, ref, revision=T.unsafe(nil), git=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def path(); end

  sig do
    params(
      path: T.untyped,
    )
    .returns(T.untyped)
  end
  def path=(path); end

  sig {returns(T.untyped)}
  def ref(); end

  sig do
    params(
      ref: T.untyped,
    )
    .returns(T.untyped)
  end
  def ref=(ref); end

  sig {returns(T.untyped)}
  def revision(); end

  sig do
    params(
      revision: T.untyped,
    )
    .returns(T.untyped)
  end
  def revision=(revision); end

  sig {returns(T.untyped)}
  def uri(); end

  sig do
    params(
      uri: T.untyped,
    )
    .returns(T.untyped)
  end
  def uri=(uri); end

  sig {returns(T.untyped)}
  def version(); end
end

class Bundler::Source::Git::MissingGitRevisionError < Bundler::GitError
  sig do
    params(
      ref: T.untyped,
      repo: T.untyped,
    )
    .void
  end
  def initialize(ref, repo); end
end

class Bundler::Source::Metadata < Bundler::Source
  # Also aliased as:
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Metadata.html#method-i-eql-3F)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def ==(other); end

  sig {returns(T.untyped)}
  def cached!(); end

  # Alias for:
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Metadata.html#method-i-3D-3D)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def eql?(other); end

  sig {returns(T.untyped)}
  def hash(); end

  sig do
    params(
      spec: T.untyped,
      _opts: T.untyped,
    )
    .returns(T.untyped)
  end
  def install(spec, _opts=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def options(); end

  sig {returns(T.untyped)}
  def remote!(); end

  sig {returns(T.untyped)}
  def specs(); end

  sig {returns(T.untyped)}
  def to_s(); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def version_message(spec); end
end

class Bundler::Source::Path < Bundler::Source
  DEFAULT_GLOB = ::T.let(nil, T.untyped)

  # Alias for:
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Path.html#method-i-eql-3F)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def ==(other); end

  sig {returns(T.untyped)}
  def app_cache_dirname(); end

  sig do
    params(
      spec: T.untyped,
      custom_path: T.untyped,
    )
    .returns(T.untyped)
  end
  def cache(spec, custom_path=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def cached!(); end

  # Also aliased as:
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Path.html#method-i-3D-3D)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def eql?(other); end

  sig {returns(T.untyped)}
  def expanded_original_path(); end

  sig {returns(T.untyped)}
  def hash(); end

  sig do
    params(
      options: T.untyped,
    )
    .void
  end
  def initialize(options); end

  sig do
    params(
      spec: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def install(spec, options=T.unsafe(nil)); end

  sig do
    params(
      _: T.untyped,
    )
    .returns(T.untyped)
  end
  def local_specs(*_); end

  sig {returns(T.untyped)}
  def name(); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def name=(name); end

  sig {returns(T.untyped)}
  def options(); end

  sig {returns(T.untyped)}
  def original_path(); end

  sig {returns(T.untyped)}
  def path(); end

  sig {returns(T.untyped)}
  def remote!(); end

  sig {returns(T.untyped)}
  def root(); end

  sig {returns(T.untyped)}
  def root_path(); end

  sig {returns(T.untyped)}
  def specs(); end

  sig {returns(T.untyped)}
  def to_lock(); end

  sig {returns(T.untyped)}
  def to_s(); end

  sig {returns(T.untyped)}
  def version(); end

  sig do
    params(
      version: T.untyped,
    )
    .returns(T.untyped)
  end
  def version=(version); end

  sig do
    params(
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.from_lock(options); end
end

class Bundler::Source::Rubygems < Bundler::Source
  # Use the API when installing less than X gems
  API_REQUEST_LIMIT = ::T.let(nil, T.untyped)
  # Ask for X gems per API request
  API_REQUEST_SIZE = ::T.let(nil, T.untyped)

  # Alias for:
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Rubygems.html#method-i-eql-3F)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def ==(other); end

  sig do
    params(
      source: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_remote(source); end

  sig {returns(T.untyped)}
  def api_fetchers(); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def builtin_gem?(spec); end

  sig do
    params(
      spec: T.untyped,
      custom_path: T.untyped,
    )
    .returns(T.untyped)
  end
  def cache(spec, custom_path=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def cache_path(); end

  sig {returns(T.untyped)}
  def cached!(); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def cached_built_in_gem(spec); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def cached_gem(spec); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def cached_path(spec); end

  sig {returns(T.untyped)}
  def cached_specs(); end

  sig {returns(T.untyped)}
  def caches(); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def can_lock?(spec); end

  sig {returns(T.untyped)}
  def credless_remotes(); end

  sig {returns(T.untyped)}
  def dependency_names_to_double_check(); end

  sig do
    params(
      unmet_dependency_names: T.untyped,
    )
    .returns(T.untyped)
  end
  def double_check_for(unmet_dependency_names); end

  # Also aliased as:
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Rubygems.html#method-i-3D-3D)
  sig do
    params(
      other: T.untyped,
    )
    .returns(T.untyped)
  end
  def eql?(other); end

  sig do
    params(
      other_remotes: T.untyped,
    )
    .returns(T.untyped)
  end
  def equivalent_remotes?(other_remotes); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def fetch_gem(spec); end

  sig do
    params(
      fetchers: T.untyped,
      dependency_names: T.untyped,
      index: T.untyped,
      override_dupes: T.untyped,
    )
    .returns(T.untyped)
  end
  def fetch_names(fetchers, dependency_names, index, override_dupes); end

  sig {returns(T.untyped)}
  def fetchers(); end

  sig {returns(T.untyped)}
  def hash(); end

  sig do
    params(
      o: T.untyped,
    )
    .returns(T.untyped)
  end
  def include?(o); end

  sig do
    params(
      options: T.untyped,
    )
    .void
  end
  def initialize(options=T.unsafe(nil)); end

  sig do
    params(
      spec: T.untyped,
      opts: T.untyped,
    )
    .returns(T.untyped)
  end
  def install(spec, opts=T.unsafe(nil)); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def installed?(spec); end

  sig {returns(T.untyped)}
  def installed_specs(); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def loaded_from(spec); end

  # Alias for:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Rubygems.html#method-i-to_s)
  sig {returns(T.untyped)}
  def name(); end

  sig do
    params(
      uri: T.untyped,
    )
    .returns(T.untyped)
  end
  def normalize_uri(uri); end

  sig {returns(T.untyped)}
  def options(); end

  sig {returns(T.untyped)}
  def remote!(); end

  sig {returns(T.untyped)}
  def remote_specs(); end

  sig {returns(T.untyped)}
  def remotes(); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def remotes_for_spec(spec); end

  sig do
    params(
      remote: T.untyped,
    )
    .returns(T.untyped)
  end
  def remove_auth(remote); end

  sig do
    params(
      other_remotes: T.untyped,
      allow_equivalent: T.untyped,
    )
    .returns(T.untyped)
  end
  def replace_remotes(other_remotes, allow_equivalent=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def requires_sudo?(); end

  sig {returns(T.untyped)}
  def rubygems_dir(); end

  sig {returns(T.untyped)}
  def specs(); end

  sig do
    params(
      remote: T.untyped,
    )
    .returns(T.untyped)
  end
  def suppress_configured_credentials(remote); end

  sig {returns(T.untyped)}
  def to_lock(); end

  # Also aliased as:
  # [`name`](https://docs.ruby-lang.org/en/2.7.0/Bundler/Source/Rubygems.html#method-i-name)
  sig {returns(T.untyped)}
  def to_s(); end

  sig {returns(T.untyped)}
  def unmet_deps(); end

  sig do
    params(
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.from_lock(options); end
end

class Bundler::SourceList
  sig do
    params(
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_git_source(options=T.unsafe(nil)); end

  sig do
    params(
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_path_source(options=T.unsafe(nil)); end

  sig do
    params(
      source: T.untyped,
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_plugin_source(source, options=T.unsafe(nil)); end

  sig do
    params(
      uri: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_rubygems_remote(uri); end

  sig do
    params(
      options: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_rubygems_source(options=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def all_sources(); end

  sig {returns(T.untyped)}
  def cached!(); end

  sig {returns(T.untyped)}
  def default_source(); end

  sig do
    params(
      source: T.untyped,
    )
    .returns(T.untyped)
  end
  def get(source); end

  sig {returns(T.untyped)}
  def git_sources(); end

  sig {returns(T.untyped)}
  def global_rubygems_source(); end

  sig do
    params(
      uri: T.untyped,
    )
    .returns(T.untyped)
  end
  def global_rubygems_source=(uri); end

  sig {void}
  def initialize(); end

  sig {returns(T.untyped)}
  def lock_sources(); end

  sig {returns(T.untyped)}
  def metadata_source(); end

  sig {returns(T.untyped)}
  def path_sources(); end

  sig {returns(T.untyped)}
  def plugin_sources(); end

  sig {returns(T.untyped)}
  def remote!(); end

  # Returns true if there are changes
  sig do
    params(
      replacement_sources: T.untyped,
    )
    .returns(T.untyped)
  end
  def replace_sources!(replacement_sources); end

  sig {returns(T.untyped)}
  def rubygems_primary_remotes(); end

  sig {returns(T.untyped)}
  def rubygems_remotes(); end

  sig {returns(T.untyped)}
  def rubygems_sources(); end
end

class Bundler::SpecSet
  include ::TSort
  include T::Enumerable
  extend ::Forwardable

  Elem = type_member(:out)

  sig do
    params(
      args: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def <<(*args, &block); end

  sig do
    params(
      key: T.untyped,
    )
    .returns(T.untyped)
  end
  def [](key); end

  sig do
    params(
      key: T.untyped,
      value: T.untyped,
    )
    .returns(T.untyped)
  end
  def []=(key, value); end

  sig do
    params(
      args: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def add(*args, &block); end

  sig do
    params(
      args: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def each(*args, &block); end

  sig do
    params(
      args: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def empty?(*args, &block); end

  sig do
    params(
      name: T.untyped,
      platform: T.untyped,
    )
    .returns(T.untyped)
  end
  def find_by_name_and_platform(name, platform); end

  sig do
    params(
      dependencies: T.untyped,
      skip: T.untyped,
      check: T.untyped,
      match_current_platform: T.untyped,
      raise_on_missing: T.untyped,
    )
    .returns(T.untyped)
  end
  def for(dependencies, skip=T.unsafe(nil), check=T.unsafe(nil), match_current_platform=T.unsafe(nil), raise_on_missing=T.unsafe(nil)); end

  sig do
    params(
      specs: T.untyped,
    )
    .void
  end
  def initialize(specs); end

  sig do
    params(
      args: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def length(*args, &block); end

  sig do
    params(
      deps: T.untyped,
      missing_specs: T.untyped,
    )
    .returns(T.untyped)
  end
  def materialize(deps, missing_specs=T.unsafe(nil)); end

  # Materialize for all the specs in the spec set, regardless of what platform
  # they're for This is in contrast to how for does platform filtering (and
  # specifically different from how `materialize` calls `for` only for the
  # current platform) @return [Array<Gem::Specification>]
  sig {returns(T.untyped)}
  def materialized_for_all_platforms(); end

  sig do
    params(
      set: T.untyped,
    )
    .returns(T.untyped)
  end
  def merge(set); end

  sig do
    params(
      args: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def remove(*args, &block); end

  sig do
    params(
      args: T.untyped,
      block: T.untyped,
    )
    .returns(T.untyped)
  end
  def size(*args, &block); end

  sig {returns(T.untyped)}
  def sort!(); end

  sig {returns(T.untyped)}
  def to_a(); end

  sig {returns(T.untyped)}
  def to_hash(); end

  sig do
    params(
      deps: T.untyped,
    )
    .returns(T.untyped)
  end
  def valid_for?(deps); end

  sig do
    params(
      spec: T.untyped,
    )
    .returns(T.untyped)
  end
  def what_required(spec); end
end

class Bundler::StubSpecification < Bundler::RemoteSpecification
  sig {returns(T.untyped)}
  def activated(); end

  sig do
    params(
      activated: T.untyped,
    )
    .returns(T.untyped)
  end
  def activated=(activated); end

  sig {returns(T.untyped)}
  def default_gem(); end

  sig {returns(T::Boolean)}
  def default_gem?; end

  sig {returns(T.untyped)}
  def full_gem_path(); end

  sig {returns(T.untyped)}
  def full_require_paths(); end

  sig {returns(T.untyped)}
  def ignored(); end

  sig do
    params(
      ignored: T.untyped,
    )
    .returns(T.untyped)
  end
  def ignored=(ignored); end

  # This is what we do in bundler/rubygems\_ext
  # [`full_require_paths`](https://docs.ruby-lang.org/en/2.6.0/Bundler/StubSpecification.html#method-i-full_require_paths)
  # is always implemented in >= 2.2.0
  sig {returns(T.untyped)}
  def load_paths(); end

  sig {returns(T.untyped)}
  def loaded_from(); end

  sig do
    params(
      glob: T.untyped,
    )
    .returns(T.untyped)
  end
  def matches_for_glob(glob); end

  # This is defined directly to avoid having to load every installed spec
  sig {returns(T.untyped)}
  def missing_extensions?(); end

  sig {returns(T.untyped)}
  def raw_require_paths(); end

  sig do
    params(
      source: T.untyped,
    )
    .returns(T.untyped)
  end
  def source=(source); end

  sig {returns(T.untyped)}
  def stub(); end

  sig do
    params(
      stub: T.untyped,
    )
    .returns(T.untyped)
  end
  def stub=(stub); end

  sig {returns(T.untyped)}
  def to_yaml(); end

  sig do
    params(
      stub: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.from_stub(stub); end
end

class Bundler::SudoNotPermittedError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::TemporaryResourceError < Bundler::PermissionError
  sig {returns(T.untyped)}
  def message(); end

  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::ThreadCreationError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def status_code(); end
end

module Bundler::UI
end

class Bundler::UI::RGProxy < Gem::SilentUI
  sig do
    params(
      ui: T.untyped,
    )
    .void
  end
  def initialize(ui); end

  sig do
    params(
      message: T.untyped,
    )
    .returns(T.untyped)
  end
  def say(message); end
end

class Bundler::UI::Silent
  sig do
    params(
      string: T.untyped,
      color: T.untyped,
    )
    .returns(T.untyped)
  end
  def add_color(string, color); end

  sig do
    params(
      message: T.untyped,
    )
    .returns(T.untyped)
  end
  def ask(message); end

  sig do
    params(
      message: T.untyped,
      newline: T.untyped,
    )
    .returns(T.untyped)
  end
  def confirm(message, newline=T.unsafe(nil)); end

  sig do
    params(
      message: T.untyped,
      newline: T.untyped,
    )
    .returns(T.untyped)
  end
  def debug(message, newline=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def debug?(); end

  sig do
    params(
      message: T.untyped,
      newline: T.untyped,
    )
    .returns(T.untyped)
  end
  def error(message, newline=T.unsafe(nil)); end

  sig do
    params(
      message: T.untyped,
      newline: T.untyped,
    )
    .returns(T.untyped)
  end
  def info(message, newline=T.unsafe(nil)); end

  sig {void}
  def initialize(); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def level(name=T.unsafe(nil)); end

  sig do
    params(
      name: T.untyped,
    )
    .returns(T.untyped)
  end
  def level=(name); end

  sig {returns(T.untyped)}
  def no?(); end

  sig {returns(T.untyped)}
  def quiet?(); end

  sig do
    params(
      shell: T.untyped,
    )
    .returns(T.untyped)
  end
  def shell=(shell); end

  sig {returns(T.untyped)}
  def silence(); end

  sig do
    params(
      message: T.untyped,
      newline: T.untyped,
      force: T.untyped,
    )
    .returns(T.untyped)
  end
  def trace(message, newline=T.unsafe(nil), force=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def unprinted_warnings(); end

  sig do
    params(
      message: T.untyped,
      newline: T.untyped,
    )
    .returns(T.untyped)
  end
  def warn(message, newline=T.unsafe(nil)); end

  sig do
    params(
      msg: T.untyped,
    )
    .returns(T.untyped)
  end
  def yes?(msg); end
end

module Bundler::URICredentialsFilter
  sig do
    params(
      str_to_filter: T.untyped,
      uri: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.credential_filtered_string(str_to_filter, uri); end

  sig do
    params(
      uri_to_anonymize: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.credential_filtered_uri(uri_to_anonymize); end
end

# Internal error, should be rescued
class Bundler::VersionConflict < Bundler::BundlerError
  sig {returns(T.untyped)}
  def conflicts(); end

  sig do
    params(
      conflicts: T.untyped,
      msg: T.untyped,
    )
    .void
  end
  def initialize(conflicts, msg=T.unsafe(nil)); end

  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::VirtualProtocolError < Bundler::BundlerError
  sig {returns(T.untyped)}
  def message(); end

  sig {returns(T.untyped)}
  def status_code(); end
end

# A stub yaml serializer that can handle only hashes and strings (as of now).
module Bundler::YAMLSerializer
  ARRAY_REGEX = ::T.let(nil, T.untyped)
  HASH_REGEX = ::T.let(nil, T.untyped)

  sig do
    params(
      hash: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.dump(hash); end

  sig do
    params(
      str: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.load(str); end
end

class Bundler::YamlSyntaxError < Bundler::BundlerError
  sig do
    params(
      orig_exception: T.untyped,
      msg: T.untyped,
    )
    .void
  end
  def initialize(orig_exception, msg); end

  sig {returns(T.untyped)}
  def orig_exception(); end

  sig {returns(T.untyped)}
  def status_code(); end
end

class Bundler::Installer
  sig {params(ambiguous_gems: T.untyped).returns(T.untyped)}
  def self.ambiguous_gems=(ambiguous_gems); end

  sig {returns(T.untyped)}
  def self.ambiguous_gems; end

  sig {returns(T.untyped)}
  def post_install_messages; end

  sig {params(root: T.untyped, definition: T.untyped, options: T.untyped).returns(T.untyped)}
  def self.install(root, definition, options = {}); end

  sig {params(root: T.untyped, definition: T.untyped).void}
  def initialize(root, definition); end

  sig {params(options: T.untyped).void}
  def run(options); end

  sig {params(spec: T.untyped, options: T.untyped).void}
  def generate_bundler_executable_stubs(spec, options = {}); end

  sig {params(spec: T.untyped, options: T.untyped).void}
  def generate_standalone_bundler_executable_stubs(spec, options = {}); end
end
