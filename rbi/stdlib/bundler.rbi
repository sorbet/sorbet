# typed: true

module Bundler
  class Installer
    def generate_standalone_bundler_executable_stubs(spec)
    end

    def post_install_messages()
    end

    def generate_bundler_executable_stubs(spec, options = _)
    end

    def run(options)
    end
  end

  class YamlSyntaxError < ::Bundler::BundlerError
    def orig_exception()
    end

    def status_code()
    end
  end

  class TemporaryResourceError < ::Bundler::PermissionError
    def status_code()
    end

    def message()
    end
  end

  class VirtualProtocolError < ::Bundler::BundlerError
    def status_code()
    end

    def message()
    end
  end

  class NoSpaceOnDeviceError < ::Bundler::PermissionError
    def status_code()
    end

    def message()
    end
  end

  class GenericSystemCallError < ::Bundler::BundlerError
    def status_code()
    end

    def underlying_error()
    end
  end

  class SecurityError < ::Bundler::BundlerError
    def status_code()
    end
  end

  NULL = T.let(T.unsafe(nil), String)

  class Resolver
    include(Bundler::Molinillo::SpecificationProvider)
    include(Bundler::Molinillo::UI)

    class SpecGroup
      include(Bundler::GemHelpers)

      def dependencies_for_activated_platforms()
      end

      def name=(_)
      end

      def to_s()
      end

      def hash()
      end

      def ==(other)
      end

      def ignores_bundler_dependencies()
      end

      def ignores_bundler_dependencies=(_)
      end

      def version=(_)
      end

      def source()
      end

      def activate_platform!(platform)
      end

      def name()
      end

      def for?(platform)
      end

      def eql?(other)
      end

      def source=(_)
      end

      def to_specs()
      end

      def version()
      end
    end

    def dependencies_for(specification)
    end

    def start(requirements)
    end

    def search_for(dependency)
    end

    def debug(depth = _)
    end

    def index_for(dependency)
    end

    def debug?()
    end

    def name_for(dependency)
    end

    def name_for_explicit_dependency_source()
    end

    def name_for_locking_dependency_source()
    end

    def requirement_satisfied_by?(requirement, activated, spec)
    end

    def relevant_sources_for_vertex(vertex)
    end

    def sort_dependencies(dependencies, activated, conflicts)
    end

    def before_resolution()
    end

    def after_resolution()
    end

    def indicate_progress()
    end
  end

  class SudoNotPermittedError < ::Bundler::BundlerError
    def status_code()
    end
  end

  class MarshalError < ::StandardError

  end

  class Dependency < ::Gem::Dependency
    PLATFORM_MAP = T.let(T.unsafe(nil), Hash)

    REVERSE_PLATFORM_MAP = T.let(T.unsafe(nil), Hash)

    def autorequire()
    end

    def specific?()
    end

    def groups()
    end

    def gemfile()
    end

    def gem_platforms(valid_platforms)
    end

    def to_lock()
    end

    def platforms()
    end

    def should_include?()
    end

    def current_env?()
    end

    def current_platform?()
    end
  end

  class StubSpecification < ::Bundler::RemoteSpecification
    def load_paths()
    end

    def ignored()
    end

    def stub()
    end

    def ignored=(_)
    end

    def matches_for_glob(glob)
    end

    def full_require_paths()
    end

    def default_gem()
    end

    def activated()
    end

    def full_gem_path()
    end

    def loaded_from()
    end

    def activated=(activated)
    end

    def raw_require_paths()
    end

    def source=(source)
    end

    def to_yaml()
    end

    def stub=(_)
    end

    def missing_extensions?()
    end
  end

  class EnvironmentPreserver
    INTENTIONALLY_NIL = T.let(T.unsafe(nil), String)

    BUNDLER_KEYS = T.let(T.unsafe(nil), Array)

    BUNDLER_PREFIX = T.let(T.unsafe(nil), String)

    def backup()
    end

    def restore()
    end
  end

  module Plugin
    class UndefinedCommandError < ::Bundler::PluginError

    end

    class Installer
      class Rubygems < ::Bundler::Source::Rubygems
        def version_message(spec)
        end
      end

      class Git < ::Bundler::Source::Git
        def root()
        end

        def cache_path()
        end

        def install_path()
        end

        def generate_bin(spec, disable_extensions = _)
        end

        def version_message(spec)
        end
      end

      def install(names, options)
      end

      def install_definition(definition)
      end
    end

    class SourceList < ::Bundler::SourceList
      def add_git_source(options = _)
      end

      def add_rubygems_source(options = _)
      end

      def all_sources()
      end
    end

    class API
      module Source
        def fetch_gemspec_files()
        end

        def options_to_lock()
        end

        def gem_install_dir()
        end

        def app_cache_dirname()
        end

        def ==(other)
        end

        def to_s()
        end

        def bundler_plugin_api_source?()
        end

        def uri_hash()
        end

        def post_install(spec, disable_exts = _)
        end

        def dependency_names()
        end

        def name()
        end

        def unmet_deps()
        end

        def include?(other)
        end

        def eql?(other)
        end

        def to_lock()
        end

        def can_lock?(spec)
        end

        def double_check_for(*_)
        end

        def installed?()
        end

        def root()
        end

        def cache(spec, custom_path = _)
        end

        def dependency_names=(_)
        end

        def specs()
        end

        def hash()
        end

        def options()
        end

        def remote!()
        end

        def install_path()
        end

        def cached!()
        end

        def install(spec, opts)
        end

        def unlock!()
        end

        def app_cache_path(custom_path = _)
        end

        def uri()
        end
      end

      def tmp(*names)
      end

      def cache_dir()
      end

      def method_missing(name, *args, &blk)
      end
    end

    class UnknownSourceError < ::Bundler::PluginError

    end

    class Index
      class SourceConflict < ::Bundler::PluginError

      end

      class CommandConflict < ::Bundler::PluginError

      end

      def load_paths(name)
      end

      def command_plugin(command)
      end

      def commands()
      end

      def source?(source)
      end

      def source_plugin(name)
      end

      def global_index_file()
      end

      def local_index_file()
      end

      def index_file()
      end

      def hook_plugins(event)
      end

      def register_plugin(name, path, load_paths, commands, sources, hooks)
      end

      def plugin_path(name)
      end

      def installed?(name)
      end
    end

    PLUGIN_FILE_NAME = T.let(T.unsafe(nil), String)

    class DSL < ::Bundler::Dsl
      class PluginGemfileError < ::Bundler::PluginError

      end

      def inferred_plugins()
      end

      def plugin(name, *args)
      end

      def method_missing(name, *args)
      end

      def source(source, *args, &blk)
      end

      def _gem(name, *args)
      end
    end

    module Events
      GEM_AFTER_INSTALL = T.let(T.unsafe(nil), String)

      GEM_BEFORE_INSTALL_ALL = T.let(T.unsafe(nil), String)

      GEM_AFTER_INSTALL_ALL = T.let(T.unsafe(nil), String)

      GEM_BEFORE_INSTALL = T.let(T.unsafe(nil), String)
    end

    class MalformattedPlugin < ::Bundler::PluginError

    end
  end

  ORIGINAL_ENV = T.let(T.unsafe(nil), Hash)

  SUDO_MUTEX = T.let(T.unsafe(nil), Thread::Mutex)

  class GemspecError < ::Bundler::BundlerError
    def status_code()
    end
  end

  class Definition
    include(Bundler::GemHelpers)

    def gem_version_promoter()
    end

    def new_specs()
    end

    def removed_specs()
    end

    def new_platform?()
    end

    def missing_specs()
    end

    def index()
    end

    def current_dependencies()
    end

    def platforms()
    end

    def lockfile()
    end

    def groups()
    end

    def has_rubygems_remotes?()
    end

    def has_local_dependencies?()
    end

    def spec_git_paths()
    end

    def locked_bundler_version()
    end

    def locked_ruby_version()
    end

    def gemfiles()
    end

    def ruby_version()
    end

    def specs()
    end

    def dependencies()
    end

    def validate_ruby!()
    end

    def validate_platforms!()
    end

    def lock(file, preserve_unknown_sections = _)
    end

    def add_platform(platform)
    end

    def remove_platform(platform)
    end

    def find_resolved_spec(current_spec)
    end

    def find_indexed_specs(current_spec)
    end

    def ensure_equivalent_gemfile_and_lockfile(explicit_flag = _)
    end

    def requested_specs()
    end

    def to_lock()
    end

    def validate_runtime!()
    end

    def locked_deps()
    end

    def requires()
    end

    def resolve()
    end

    def locked_gems()
    end

    def locked_ruby_version_object()
    end

    def add_current_platform()
    end

    def unlocking?()
    end

    def nothing_changed?()
    end

    def missing_specs?()
    end

    def resolve_with_cache!()
    end

    def resolve_remotely!()
    end

    def specs_for(groups)
    end
  end

  class CurrentRuby
    KNOWN_PLATFORMS = T.let(T.unsafe(nil), Array)

    KNOWN_MINOR_VERSIONS = T.let(T.unsafe(nil), Array)

    KNOWN_MAJOR_VERSIONS = T.let(T.unsafe(nil), Array)

    def maglev_24?()
    end

    def mingw_24?()
    end

    def mri_24?()
    end

    def mswin_24?()
    end

    def mswin64_24?()
    end

    def rbx_24?()
    end

    def ruby_24?()
    end

    def truffleruby_24?()
    end

    def x64_mingw_24?()
    end

    def on_25?()
    end

    def jruby_25?()
    end

    def maglev_25?()
    end

    def mingw_25?()
    end

    def mri_25?()
    end

    def mswin_25?()
    end

    def mswin64_25?()
    end

    def rbx_25?()
    end

    def ruby_25?()
    end

    def truffleruby_25?()
    end

    def x64_mingw_25?()
    end

    def on_26?()
    end

    def jruby_26?()
    end

    def maglev_26?()
    end

    def mingw_26?()
    end

    def mri_26?()
    end

    def mswin_26?()
    end

    def mswin64_26?()
    end

    def rbx_26?()
    end

    def ruby_26?()
    end

    def truffleruby_26?()
    end

    def x64_mingw_26?()
    end

    def on_1?()
    end

    def jruby_1?()
    end

    def maglev_1?()
    end

    def mingw_1?()
    end

    def mri_1?()
    end

    def mswin_1?()
    end

    def mswin64_1?()
    end

    def rbx_1?()
    end

    def ruby_1?()
    end

    def truffleruby_1?()
    end

    def x64_mingw_1?()
    end

    def on_2?()
    end

    def jruby_2?()
    end

    def maglev_2?()
    end

    def mingw_2?()
    end

    def mri_2?()
    end

    def mswin_2?()
    end

    def mswin64_2?()
    end

    def rbx_2?()
    end

    def ruby_2?()
    end

    def truffleruby_2?()
    end

    def x64_mingw_2?()
    end

    def ruby?()
    end

    def mswin?()
    end

    def mri?()
    end

    def rbx?()
    end

    def jruby?()
    end

    def maglev?()
    end

    def truffleruby?()
    end

    def mswin64?()
    end

    def mingw?()
    end

    def x64_mingw?()
    end

    def on_18?()
    end

    def jruby_18?()
    end

    def maglev_18?()
    end

    def mingw_18?()
    end

    def mri_18?()
    end

    def mswin_18?()
    end

    def mswin64_18?()
    end

    def rbx_18?()
    end

    def ruby_18?()
    end

    def truffleruby_18?()
    end

    def x64_mingw_18?()
    end

    def on_19?()
    end

    def jruby_19?()
    end

    def maglev_19?()
    end

    def mingw_19?()
    end

    def mri_19?()
    end

    def mswin_19?()
    end

    def mswin64_19?()
    end

    def rbx_19?()
    end

    def ruby_19?()
    end

    def truffleruby_19?()
    end

    def x64_mingw_19?()
    end

    def on_20?()
    end

    def jruby_20?()
    end

    def maglev_20?()
    end

    def mingw_20?()
    end

    def mri_20?()
    end

    def mswin_20?()
    end

    def mswin64_20?()
    end

    def rbx_20?()
    end

    def ruby_20?()
    end

    def truffleruby_20?()
    end

    def x64_mingw_20?()
    end

    def on_21?()
    end

    def jruby_21?()
    end

    def maglev_21?()
    end

    def mingw_21?()
    end

    def mri_21?()
    end

    def mswin_21?()
    end

    def mswin64_21?()
    end

    def rbx_21?()
    end

    def ruby_21?()
    end

    def truffleruby_21?()
    end

    def x64_mingw_21?()
    end

    def on_22?()
    end

    def jruby_22?()
    end

    def maglev_22?()
    end

    def mingw_22?()
    end

    def mri_22?()
    end

    def mswin_22?()
    end

    def mswin64_22?()
    end

    def rbx_22?()
    end

    def ruby_22?()
    end

    def truffleruby_22?()
    end

    def x64_mingw_22?()
    end

    def on_23?()
    end

    def jruby_23?()
    end

    def maglev_23?()
    end

    def mingw_23?()
    end

    def mri_23?()
    end

    def mswin_23?()
    end

    def mswin64_23?()
    end

    def rbx_23?()
    end

    def ruby_23?()
    end

    def truffleruby_23?()
    end

    def x64_mingw_23?()
    end

    def on_24?()
    end

    def jruby_24?()
    end
  end

  class Dsl
    include(Bundler::RubyDsl)

    class DSLError < ::Bundler::GemfileError
      def dsl_path()
      end

      def contents()
      end

      def description()
      end

      def status_code()
      end

      def backtrace()
      end

      def to_s()
      end
    end

    VALID_PLATFORMS = T.let(T.unsafe(nil), Array)

    VALID_KEYS = T.let(T.unsafe(nil), Array)

    def plugin(*args)
    end

    def git(uri, options = _, &blk)
    end

    def platforms(*platforms)
    end

    def env(name)
    end

    def eval_gemfile(gemfile, contents = _)
    end

    def gemspec(opts = _)
    end

    def install_if(*args)
    end

    def to_definition(lockfile, unlock)
    end

    def gemspecs()
    end

    def group(*args, &blk)
    end

    def gem(name, *args)
    end

    def source(source, *args, &blk)
    end

    def method_missing(name, *args)
    end

    def dependencies()
    end

    def dependencies=(_)
    end

    def platform(*platforms)
    end

    def git_source(name, &block)
    end

    def github(repo, options = _)
    end

    def path(path, options = _, &blk)
    end
  end

  class EndpointSpecification < ::Gem::Specification
    ILLFORMED_MESSAGE = T.let(T.unsafe(nil), String)

    def fetch_platform()
    end

    def load_paths()
    end

    def __swap__(spec)
    end

    def remote()
    end

    def _local_specification()
    end

    def remote=(_)
    end

    def name()
    end

    def extensions()
    end

    def post_install_message()
    end

    def required_ruby_version()
    end

    def version()
    end

    def required_rubygems_version()
    end

    def require_paths()
    end

    def checksum()
    end

    def source()
    end

    def executables()
    end

    def dependencies()
    end

    def dependencies=(_)
    end

    def platform()
    end

    def bindir()
    end

    def source=(_)
    end
  end

  class DepProxy
    def requirement()
    end

    def name()
    end

    def __platform()
    end

    def dep()
    end

    def eql?(other)
    end

    def type()
    end

    def ==(other)
    end

    def hash()
    end

    def to_s()
    end
  end

  module GemHelpers
    class PlatformMatch < ::Struct
      EXACT_MATCH = T.let(T.unsafe(nil), Bundler::GemHelpers::PlatformMatch)

      WORST_MATCH = T.let(T.unsafe(nil), Bundler::GemHelpers::PlatformMatch)

      def cpu_match()
      end

      def platform_version_match()
      end

      def os_match=(_)
      end

      def cpu_match=(_)
      end

      def platform_version_match=(_)
      end

      def <=>(other)
      end

      def os_match()
      end
    end

    GENERIC_CACHE = T.let(T.unsafe(nil), Hash)

    GENERICS = T.let(T.unsafe(nil), Array)
  end

  class FeatureFlag
    def setup_makes_kernel_gem_public?()
    end

    def update_requires_all_flag?()
    end

    def forget_cli_options?()
    end

    def list_command?()
    end

    def plugins?()
    end

    def prefer_gems_rb?()
    end

    def skip_default_git_sources?()
    end

    def cache_all?()
    end

    def specific_platform?()
    end

    def allow_bundler_dependency_conflicts?()
    end

    def bundler_2_mode?()
    end

    def lockfile_uses_separate_rubygems_sources?()
    end

    def allow_offline_install?()
    end

    def use_gem_version_promoter_for_major_updates?()
    end

    def unlock_source_unlocks_spec?()
    end

    def global_path_appends_ruby_scope?()
    end

    def default_install_uses_path?()
    end

    def viz_command?()
    end

    def global_gem_cache?()
    end

    def deployment_means_frozen?()
    end

    def suppress_install_using_messages?()
    end

    def auto_config_jobs?()
    end

    def only_update_to_newer_versions?()
    end

    def default_cli_command()
    end

    def auto_clean_without_path?()
    end

    def bundler_1_mode?()
    end

    def bundler_3_mode?()
    end

    def error_on_stderr?()
    end

    def bundler_4_mode?()
    end

    def bundler_5_mode?()
    end

    def bundler_6_mode?()
    end

    def bundler_7_mode?()
    end

    def bundler_8_mode?()
    end

    def bundler_9_mode?()
    end

    def bundler_10_mode?()
    end

    def cache_command_is_package?()
    end

    def console_command?()
    end

    def print_only_version_number?()
    end

    def init_gems_rb?()
    end

    def disable_multisource?()
    end

    def path_relative_to_cwd?()
    end
  end

  class Env

  end

  class Fetcher
    FETCHERS = T.let(T.unsafe(nil), Array)

    class SSLError < ::Bundler::HTTPError

    end

    class CompactIndex < ::Bundler::Fetcher::Base
      class ClientFetcher < ::Struct
        def ui=(_)
        end

        def call(path, headers)
        end

        def fetcher()
        end

        def fetcher=(_)
        end

        def ui()
        end
      end

      def fetch_spec(*args, &blk)
      end

      def api_fetcher?()
      end

      def specs(*args, &blk)
      end

      def specs_for_names(gem_names)
      end

      def available?(*args, &blk)
      end
    end

    class Downloader
      def connection()
      end

      def redirect_limit()
      end

      def request(uri, headers)
      end

      def fetch(uri, headers = _, counter = _)
      end
    end

    class NetworkDownError < ::Bundler::HTTPError

    end

    class FallbackError < ::Bundler::HTTPError

    end

    class CertificateFailureError < ::Bundler::HTTPError

    end

    class AuthenticationRequiredError < ::Bundler::HTTPError

    end

    class BadAuthenticationError < ::Bundler::HTTPError

    end

    NET_ERRORS = T.let(T.unsafe(nil), Array)

    FAIL_ERRORS = T.let(T.unsafe(nil), Array)

    HTTP_ERRORS = T.let(T.unsafe(nil), Array)

    class Index < ::Bundler::Fetcher::Base
      def specs(_gem_names)
      end

      def fetch_spec(spec)
      end
    end

    class Dependency < ::Bundler::Fetcher::Base
      def dependency_api_uri(gem_names = _)
      end

      def dependency_specs(gem_names)
      end

      def specs(gem_names, full_dependency_list = _, last_spec_list = _)
      end

      def available?()
      end

      def unmarshalled_dep_gems(gem_names)
      end

      def api_fetcher?()
      end

      def get_formatted_specs_and_deps(gem_list)
      end
    end

    class Base
      def fetch_uri()
      end

      def remote()
      end

      def downloader()
      end

      def remote_uri()
      end

      def available?()
      end

      def api_fetcher?()
      end

      def display_uri()
      end
    end

    def inspect()
    end

    def specs(gem_names, source)
    end

    def http_proxy()
    end

    def user_agent()
    end

    def fetchers()
    end

    def fetch_spec(spec)
    end

    def use_api()
    end

    def uri()
    end

    def specs_with_retry(gem_names, source)
    end
  end

  module BuildMetadata

  end

  class GemHelper
    def install_gem(built_gem_path = _, local = _)
    end

    def gemspec()
    end

    def build_gem()
    end

    def base()
    end

    def install()
    end

    def spec_path()
    end
  end

  class Graph
    GRAPH_NAME = T.let(T.unsafe(nil), Symbol)

    class GraphVizClient
      def g()
      end

      def run()
      end
    end

    def relations()
    end

    def node_options()
    end

    def edge_options()
    end

    def viz()
    end

    def groups()
    end

    def output_file()
    end

    def output_format()
    end
  end

  class GemRemoteFetcher < ::Gem::RemoteFetcher
    def fetch_http(uri, last_modified = _, head = _, depth = _)
    end

    def headers=(_)
    end

    def headers()
    end
  end

  module MatchPlatform
    include(Bundler::GemHelpers)

    def match_platform(p)
    end
  end

  class LazySpecification
    include(Bundler::MatchPlatform)
    include(Bundler::GemHelpers)

    class Identifier < ::Struct
      include(Comparable)

      def <=>(other)
      end

      def name=(_)
      end

      def source()
      end

      def version=(_)
      end

      def dependencies()
      end

      def name()
      end

      def dependencies=(_)
      end

      def platform()
      end

      def platform=(_)
      end

      def source=(_)
      end

      def version()
      end
    end

    def remote()
    end

    def ==(other)
    end

    def full_name()
    end

    def to_s()
    end

    def source()
    end

    def git_version()
    end

    def dependencies()
    end

    def remote=(_)
    end

    def name()
    end

    def satisfies?(dependency)
    end

    def platform()
    end

    def to_lock()
    end

    def respond_to?(*args)
    end

    def source=(_)
    end

    def identifier()
    end

    def version()
    end

    def __materialize__()
    end
  end

  class Index
    include(Enumerable)

    RUBY = T.let(T.unsafe(nil), String)

    NULL = T.let(T.unsafe(nil), String)

    EMPTY_SEARCH = T.let(T.unsafe(nil), Array)

    def local_search(query, base = _)
    end

    def <<(spec)
    end

    def dependencies_eql?(spec, other_spec)
    end

    def ==(other)
    end

    def [](query, base = _)
    end

    def spec_names()
    end

    def search(query, base = _)
    end

    def empty?()
    end

    def search_all(name)
    end

    def dependency_names()
    end

    def unmet_dependency_names()
    end

    def inspect()
    end

    def sort_specs(specs)
    end

    def add_source(index)
    end

    def sources()
    end

    def size()
    end

    def use(other, override_dupes = _)
    end

    def each(&blk)
    end
  end

  class Injector
    INJECTED_GEMS = T.let(T.unsafe(nil), String)

    def remove(gemfile_path, lockfile_path)
    end

    def inject(gemfile_path, lockfile_path)
    end
  end

  class RubygemsIntegration
    class Future < ::Bundler::RubygemsIntegration
      def gem_remote_fetcher()
      end

      def gem_from_path(path, policy = _)
      end

      def install_with_build_args(args)
      end

      def stub_rubygems(specs)
      end

      def download_gem(spec, uri, path)
      end

      def build(spec, skip_validation = _)
      end

      def repository_subdirectories()
      end

      def path_separator()
      end

      def all_specs()
      end

      def find_name(name)
      end

      def fetch_specs(source, remote, name)
      end

      def fetch_all_remote_specs(remote)
      end
    end

    class Legacy < ::Bundler::RubygemsIntegration
      def all_specs()
      end

      def find_name(name)
      end

      def stub_rubygems(specs)
      end

      def post_reset_hooks()
      end

      def validate(spec)
      end

      def reset()
      end
    end

    class MoreFuture < ::Bundler::RubygemsIntegration::Future
      def binstubs_call_gem?()
      end

      def stubs_provide_full_functionality?()
      end

      def all_specs()
      end

      def find_name(name)
      end

      def backport_ext_builder_monitor()
      end

      def use_gemdeps(gemfile)
      end
    end

    class Ancient < ::Bundler::RubygemsIntegration::Legacy

    end

    class Transitional < ::Bundler::RubygemsIntegration::Legacy
      def stub_rubygems(specs)
      end

      def validate(spec)
      end
    end

    class Modern < ::Bundler::RubygemsIntegration
      def stub_rubygems(specs)
      end

      def all_specs()
      end

      def find_name(name)
      end
    end

    class AlmostModern < ::Bundler::RubygemsIntegration::Modern
      def preserve_paths()
      end
    end

    class MoreModern < ::Bundler::RubygemsIntegration::Modern
      def build(spec, skip_validation = _)
      end
    end

    EXT_LOCK = T.let(T.unsafe(nil), Monitor)

    def replace_bin_path(specs, specs_by_name)
    end

    def replace_refresh()
    end

    def replace_entrypoints(specs)
    end

    def gem_bindir()
    end

    def platforms()
    end

    def validate(spec)
    end

    def ruby_engine()
    end

    def backport_yaml_initialize()
    end

    def backport_base_dir()
    end

    def backport_segment_generation()
    end

    def backport_spec_file()
    end

    def gem_dir()
    end

    def build_args()
    end

    def path_separator()
    end

    def backport_cache_file()
    end

    def loaded_specs(name)
    end

    def read_binary(path)
    end

    def version()
    end

    def bin_path(gem, bin, ver)
    end

    def gem_path()
    end

    def method_visibility(klass, method)
    end

    def undo_replacements()
    end

    def build(spec, skip_validation = _)
    end

    def sources()
    end

    def sources=(val)
    end

    def clear_paths()
    end

    def reset()
    end

    def provides?(req_str)
    end

    def user_home()
    end

    def configuration()
    end

    def build_args=(args)
    end

    def mark_loaded(spec)
    end

    def load_plugin_files(files)
    end

    def set_installed_by_version(spec, installed_by_version = _)
    end

    def spec_missing_extensions?(spec, default = _)
    end

    def spec_default_gem?(spec)
    end

    def spec_matches_for_glob(spec, glob)
    end

    def load_plugins()
    end

    def spec_extension_dir(spec)
    end

    def stub_set_spec(stub, spec)
    end

    def gem_cache()
    end

    def spec_cache_dirs()
    end

    def marshal_spec_dir()
    end

    def repository_subdirectories()
    end

    def preserve_paths()
    end

    def loaded_gem_paths()
    end

    def ui=(obj)
    end

    def ext_lock()
    end

    def fetch_specs(all, pre, &blk)
    end

    def fetch_prerelease_specs()
    end

    def fetch_all_remote_specs(remote)
    end

    def with_build_args(args)
    end

    def install_with_build_args(args)
    end

    def gem_from_path(path, policy = _)
    end

    def spec_from_gem(path, policy = _)
    end

    def security_policies()
    end

    def build_gem(gem_dir, spec)
    end

    def post_reset_hooks()
    end

    def config_map()
    end

    def suffix_pattern()
    end

    def download_gem(spec, uri, path)
    end

    def security_policy_keys()
    end

    def reverse_rubygems_kernel_mixin()
    end

    def redefine_method(klass, method, unbound_method = _, &block)
    end

    def binstubs_call_gem?()
    end

    def inflate(obj)
    end

    def stubs_provide_full_functionality?()
    end

    def replace_gem(specs, specs_by_name)
    end

    def stub_source_index(specs)
    end

    def load_path_insert_index()
    end

    def path(obj)
    end
  end

  class LockfileParser
    NAME_VERSION = T.let(T.unsafe(nil), Regexp)

    ENVIRONMENT_VERSION_SECTIONS = T.let(T.unsafe(nil), Array)

    TYPES = T.let(T.unsafe(nil), Hash)

    PLUGIN = T.let(T.unsafe(nil), String)

    BUNDLED = T.let(T.unsafe(nil), String)

    RUBY = T.let(T.unsafe(nil), String)

    DEPENDENCIES = T.let(T.unsafe(nil), String)

    PLATFORMS = T.let(T.unsafe(nil), String)

    GIT = T.let(T.unsafe(nil), String)

    PATH = T.let(T.unsafe(nil), String)

    SPECS = T.let(T.unsafe(nil), String)

    SOURCE = T.let(T.unsafe(nil), Array)

    SECTIONS_BY_VERSION_INTRODUCED = T.let(T.unsafe(nil), Hash)

    KNOWN_SECTIONS = T.let(T.unsafe(nil), Array)

    GEM = T.let(T.unsafe(nil), String)

    OPTIONS = T.let(T.unsafe(nil), Regexp)

    def bundler_version()
    end

    def specs()
    end

    def platforms()
    end

    def ruby_version()
    end

    def sources()
    end

    def dependencies()
    end

    def warn_for_outdated_bundler_version()
    end
  end

  class Retry
    def total_runs()
    end

    def name=(_)
    end

    def current_run()
    end

    def attempts(&block)
    end

    def attempt(&block)
    end

    def name()
    end

    def total_runs=(_)
    end

    def current_run=(_)
    end
  end

  class ProcessLock

  end

  class RemoteSpecification
    include(Comparable)
    include(Bundler::MatchPlatform)
    include(Bundler::GemHelpers)

    def fetch_platform()
    end

    def __swap__(spec)
    end

    def <=>(other)
    end

    def remote()
    end

    def full_name()
    end

    def to_s()
    end

    def source()
    end

    def git_version()
    end

    def dependencies()
    end

    def remote=(_)
    end

    def name()
    end

    def dependencies=(_)
    end

    def platform()
    end

    def respond_to?(method, include_all = _)
    end

    def sort_obj()
    end

    def source=(_)
    end

    def version()
    end
  end

  class GemVersionPromoter
    def level=(value)
    end

    def unlock_gems()
    end

    def minor?()
    end

    def level()
    end

    def sort_versions(dep, spec_groups)
    end

    def major?()
    end

    def prerelease_specified()
    end

    def prerelease_specified=(_)
    end

    def strict()
    end

    def strict=(_)
    end

    def locked_specs()
    end
  end

  module RubyDsl
    def ruby(*ruby_version)
    end
  end

  class RubyGemsGemInstaller < ::Gem::Installer
    def check_executable_overwrite(filename)
    end

    def pre_install_checks()
    end

    def build_extensions()
    end
  end

  class RubyVersion
    PATTERN = T.let(T.unsafe(nil), Regexp)

    def to_gem_version_with_patchlevel()
    end

    def engine_gem_version()
    end

    def versions_string(versions)
    end

    def exact?()
    end

    def single_version_string()
    end

    def ==(other)
    end

    def to_s(versions = _)
    end

    def diff(other)
    end

    def gem_version()
    end

    def host()
    end

    def patchlevel()
    end

    def engine()
    end

    def engine_versions()
    end

    def versions()
    end
  end

  class Runtime
    include(Bundler::SharedHelpers)

    REQUIRE_ERRORS = T.let(T.unsafe(nil), Array)

    def require(*groups)
    end

    def cache(custom_path = _)
    end

    def current_dependencies()
    end

    def specs()
    end

    def dependencies()
    end

    def setup(*groups)
    end

    def lock(opts = _)
    end

    def requested_specs()
    end

    def gems()
    end

    def prune_cache(cache_path)
    end

    def requires()
    end

    def clean(dry_run = _)
    end
  end

  class Settings
    NORMALIZE_URI_OPTIONS_PATTERN = T.let(T.unsafe(nil), Regexp)

    class Path < ::Struct
      def use_system_gems?()
      end

      def explicit_path=(_)
      end

      def append_ruby_scope=(_)
      end

      def system_path=(_)
      end

      def default_install_uses_path=(_)
      end

      def validate!()
      end

      def system_path()
      end

      def explicit_path()
      end

      def append_ruby_scope()
      end

      def base_path_relative_to_pwd()
      end

      def default_install_uses_path()
      end

      def path()
      end

      def base_path()
      end
    end

    BOOL_KEYS = T.let(T.unsafe(nil), Array)

    CONFIG_REGEX = T.let(T.unsafe(nil), Regexp)

    class Mirror
      DEFAULT_FALLBACK_TIMEOUT = T.let(T.unsafe(nil), Float)

      def validate!(probe = _)
      end

      def fallback_timeout()
      end

      def fallback_timeout=(timeout)
      end

      def valid?()
      end

      def ==(other)
      end

      def uri()
      end

      def uri=(uri)
      end
    end

    class Mirrors
      def each()
      end

      def parse(key, value)
      end

      def for(uri)
      end
    end

    class Validator
      class Rule
        def fail!(key, value, *reasons)
        end

        def description()
        end

        def validate!(key, value, settings)
        end

        def k(key)
        end

        def set(settings, key, value, *reasons)
        end
      end
    end

    NUMBER_KEYS = T.let(T.unsafe(nil), Array)

    ARRAY_KEYS = T.let(T.unsafe(nil), Array)

    DEFAULT_CONFIG = T.let(T.unsafe(nil), Hash)

    PER_URI_OPTIONS = T.let(T.unsafe(nil), Array)

    def temporary(update)
    end

    def local_overrides()
    end

    def key_for(key)
    end

    def set_command_option(key, value)
    end

    def set_local(key, value)
    end

    def set_command_option_if_given(key, value)
    end

    def set_global(key, value)
    end

    def [](name)
    end

    def credentials_for(uri)
    end

    def allow_sudo?()
    end

    def gem_mirrors()
    end

    def pretty_values_for(exposed_key)
    end

    def validate!()
    end

    def ignore_config?()
    end

    def locations(key)
    end

    def mirror_for(uri)
    end

    def all()
    end

    def app_cache_path()
    end

    def path()
    end
  end

  Deprecate = Gem::Deprecate

  class SpecSet
    include(TSort)
    include(Enumerable)
    extend(Forwardable)

    def remove(*args, &block)
    end

    def valid_for?(deps)
    end

    def <<(*args, &block)
    end

    def to_hash()
    end

    def to_a()
    end

    def [](key)
    end

    def []=(key, value)
    end

    def merge(set)
    end

    def empty?(*args, &block)
    end

    def sort!()
    end

    def what_required(spec)
    end

    def add(*args, &block)
    end

    def length(*args, &block)
    end

    def size(*args, &block)
    end

    def for(dependencies, skip = _, check = _, match_current_platform = _, raise_on_missing = _)
    end

    def each(*args, &block)
    end

    def materialized_for_all_platforms()
    end

    def find_by_name_and_platform(name, platform)
    end

    def materialize(deps, missing_specs = _)
    end
  end

  module UI
    class RGProxy < ::Gem::SilentUI
      def say(message)
      end
    end

    class Shell
      LEVELS = T.let(T.unsafe(nil), Array)

      def confirm(msg, newline = _)
      end

      def level=(level)
      end

      def debug(msg, newline = _)
      end

      def silence(&blk)
      end

      def warn(msg, newline = _)
      end

      def shell=(_)
      end

      def quiet?()
      end

      def unprinted_warnings()
      end

      def ask(msg)
      end

      def level(name = _)
      end

      def debug?()
      end

      def add_color(string, *color)
      end

      def trace(e, newline = _, force = _)
      end

      def error(msg, newline = _)
      end

      def yes?(msg)
      end

      def info(msg, newline = _)
      end

      def no?()
      end
    end

    class Silent
      def confirm(message, newline = _)
      end

      def level=(name)
      end

      def debug(message, newline = _)
      end

      def silence()
      end

      def shell=(_)
      end

      def quiet?()
      end

      def unprinted_warnings()
      end

      def ask(message)
      end

      def level(name = _)
      end

      def warn(message, newline = _)
      end

      def debug?()
      end

      def add_color(string, color)
      end

      def trace(message, newline = _, force = _)
      end

      def error(message, newline = _)
      end

      def yes?(msg)
      end

      def info(message, newline = _)
      end

      def no?()
      end
    end
  end

  module URICredentialsFilter

  end

  module SharedHelpers
    extend(Bundler::SharedHelpers)

    def with_clean_git_env(&block)
    end

    def digest(name)
    end

    def set_bundle_environment()
    end

    def filesystem_access(path, action = _, &block)
    end

    def const_get_safely(constant_name, namespace)
    end

    def print_major_deprecations!()
    end

    def major_deprecation(major_version, message)
    end

    def default_gemfile()
    end

    def root()
    end

    def default_lockfile()
    end

    def in_bundle?()
    end

    def ensure_same_dependencies(spec, old_deps, new_deps)
    end

    def md5_available?()
    end

    def write_to_gemfile(gemfile_path, contents)
    end

    def chdir(dir, &blk)
    end

    def pwd()
    end

    def pretty_dependency(dep, print_source = _)
    end

    def set_env(key, value)
    end

    def default_bundle_dir()
    end

    def trap(signal, override = _, &block)
    end
  end

  class SourceList
    def add_git_source(options = _)
    end

    def add_rubygems_source(options = _)
    end

    def all_sources()
    end

    def git_sources()
    end

    def get(source)
    end

    def path_sources()
    end

    def rubygems_sources()
    end

    def metadata_source()
    end

    def plugin_sources()
    end

    def global_rubygems_source()
    end

    def add_path_source(options = _)
    end

    def add_plugin_source(source, options = _)
    end

    def global_rubygems_source=(uri)
    end

    def add_rubygems_remote(uri)
    end

    def default_source()
    end

    def rubygems_remotes()
    end

    def cached!()
    end

    def remote!()
    end

    def lock_sources()
    end

    def replace_sources!(replacement_sources)
    end

    def rubygems_primary_remotes()
    end
  end

  module VersionRanges
    class NEq < ::Struct
      def version()
      end

      def version=(_)
      end
    end

    class ReqR < ::Struct
      class Endpoint < ::Struct
        def inclusive()
        end

        def version()
        end

        def inclusive=(_)
        end

        def version=(_)
        end
      end

      UNIVERSAL = T.let(T.unsafe(nil), Bundler::VersionRanges::ReqR)

      ZERO = T.let(T.unsafe(nil), Gem::Version)

      INFINITY = T.let(T.unsafe(nil), Object)

      def cover?(v)
      end

      def left=(_)
      end

      def right=(_)
      end

      def left()
      end

      def right()
      end

      def empty?()
      end

      def single?()
      end

      def to_s()
      end
    end
  end

  class InstallError < ::Bundler::BundlerError
    def status_code()
    end
  end

  class OperationNotSupportedError < ::Bundler::PermissionError
    def status_code()
    end

    def message()
    end
  end

  class GemNotFound < ::Bundler::BundlerError
    def status_code()
    end
  end

  class RubyVersionMismatch < ::Bundler::BundlerError
    def status_code()
    end
  end

  module FileUtils
    include(Bundler::FileUtils::StreamUtils_)
    extend(Bundler::FileUtils::StreamUtils_)

    LOW_METHODS = T.let(T.unsafe(nil), Array)

    module LowMethods

    end

    METHODS = T.let(T.unsafe(nil), Array)

    module StreamUtils_

    end

    module Verbose
      include(Bundler::FileUtils)
      include(Bundler::FileUtils::StreamUtils_)
      extend(Bundler::FileUtils::Verbose)
      extend(Bundler::FileUtils)
      extend(Bundler::FileUtils::StreamUtils_)
    end

    module NoWrite
      include(Bundler::FileUtils::LowMethods)
      include(Bundler::FileUtils)
      include(Bundler::FileUtils::StreamUtils_)
      extend(Bundler::FileUtils::NoWrite)
      extend(Bundler::FileUtils::LowMethods)
      extend(Bundler::FileUtils)
      extend(Bundler::FileUtils::StreamUtils_)
    end

    module DryRun
      include(Bundler::FileUtils::LowMethods)
      include(Bundler::FileUtils)
      include(Bundler::FileUtils::StreamUtils_)
      extend(Bundler::FileUtils::DryRun)
      extend(Bundler::FileUtils::LowMethods)
      extend(Bundler::FileUtils)
      extend(Bundler::FileUtils::StreamUtils_)
    end

    OPT_TABLE = T.let(T.unsafe(nil), Hash)

    class Entry_
      include(Bundler::FileUtils::StreamUtils_)

      SYSCASE = T.let(T.unsafe(nil), String)

      DIRECTORY_TERM = T.let(T.unsafe(nil), String)

      S_IF_DOOR = T.let(T.unsafe(nil), Integer)

      def path()
      end

      def remove()
      end

      def copy_file(dest)
      end

      def copy(dest)
      end

      def file?()
      end

      def entries()
      end

      def pipe?()
      end

      def symlink?()
      end

      def socket?()
      end

      def blockdev?()
      end

      def chardev?()
      end

      def dereference?()
      end

      def lstat!()
      end

      def remove_file()
      end

      def door?()
      end

      def stat()
      end

      def stat!()
      end

      def prefix()
      end

      def copy_metadata(path)
      end

      def lstat()
      end

      def remove_dir1()
      end

      def platform_support()
      end

      def rel()
      end

      def chown(uid, gid)
      end

      def chmod(mode)
      end

      def preorder_traverse()
      end

      def postorder_traverse()
      end

      def wrap_traverse(pre, post)
      end

      def inspect()
      end

      def traverse()
      end

      def directory?()
      end

      def exist?()
      end
    end
  end

  class GemfileError < ::Bundler::BundlerError
    def status_code()
    end
  end

  class VersionConflict < ::Bundler::BundlerError
    def conflicts()
    end

    def status_code()
    end
  end

  class InstallHookError < ::Bundler::BundlerError
    def status_code()
    end
  end

  class GitError < ::Bundler::BundlerError
    def status_code()
    end
  end

  VERSION = T.let(T.unsafe(nil), String)

  class DeprecatedError < ::Bundler::BundlerError
    def status_code()
    end
  end

  class BundlerError < ::StandardError

  end

  class HTTPError < ::Bundler::BundlerError
    def status_code()
    end

    def filter_uri(uri)
    end
  end

  class InvalidOption < ::Bundler::BundlerError
    def status_code()
    end
  end

  class ProductionError < ::Bundler::BundlerError
    def status_code()
    end
  end

  class LockfileError < ::Bundler::BundlerError
    def status_code()
    end
  end

  class GemfileNotFound < ::Bundler::BundlerError
    def status_code()
    end
  end

  class CyclicDependencyError < ::Bundler::BundlerError
    def status_code()
    end
  end

  class PluginError < ::Bundler::BundlerError
    def status_code()
    end
  end

  class ThreadCreationError < ::Bundler::BundlerError
    def status_code()
    end
  end

  class APIResponseMismatchError < ::Bundler::BundlerError
    def status_code()
    end
  end

  class GemfileEvalError < ::Bundler::GemfileError

  end

  class PermissionError < ::Bundler::BundlerError
    def status_code()
    end

    def action()
    end

    def message()
    end
  end

  class GemfileLockNotFound < ::Bundler::BundlerError
    def status_code()
    end
  end

  class PathError < ::Bundler::BundlerError
    def status_code()
    end
  end

  class Source
    class Git < ::Bundler::Source::Path
      class GitNotInstalledError < ::Bundler::GitError

      end

      class GitNotAllowedError < ::Bundler::GitError

      end

      class GitProxy
        def contains?(commit)
        end

        def ref()
        end

        def checkout()
        end

        def uri=(_)
        end

        def copy_to(destination, submodules = _)
        end

        def revision()
        end

        def version()
        end

        def ref=(_)
        end

        def branch()
        end

        def revision=(_)
        end

        def path=(_)
        end

        def uri()
        end

        def path()
        end

        def full_version()
        end
      end

      class GitCommandError < ::Bundler::GitError

      end

      class MissingGitRevisionError < ::Bundler::GitError

      end

      def submodules()
      end

      def ref()
      end

      def load_spec_files()
      end

      def app_cache_dirname()
      end

      def ==(other)
      end

      def to_s()
      end

      def allow_git_ops?()
      end

      def extension_dir_name()
      end

      def cache_path()
      end

      def revision()
      end

      def name()
      end

      def to_lock()
      end

      def eql?(other)
      end

      def cache(spec, custom_path = _)
      end

      def specs(*_)
      end

      def hash()
      end

      def branch()
      end

      def options()
      end

      def install_path()
      end

      def install(spec, options = _)
      end

      def unlock!()
      end

      def local_override!(path)
      end

      def uri()
      end

      def path()
      end
    end

    class Metadata < ::Bundler::Source
      def options()
      end

      def remote!()
      end

      def cached!()
      end

      def install(spec, _opts = _)
      end

      def eql?(other)
      end

      def specs()
      end

      def to_s()
      end

      def hash()
      end

      def ==(other)
      end

      def version_message(spec)
      end
    end

    class Path < ::Bundler::Source
      class Installer < ::Bundler::RubyGemsGemInstaller
        def spec()
        end

        def post_install()
        end
      end

      DEFAULT_GLOB = T.let(T.unsafe(nil), String)

      def local_specs(*_)
      end

      def app_cache_dirname()
      end

      def ==(other)
      end

      def to_s()
      end

      def version=(_)
      end

      def root_path()
      end

      def name()
      end

      def expanded_original_path()
      end

      def to_lock()
      end

      def eql?(other)
      end

      def version()
      end

      def root()
      end

      def cache(spec, custom_path = _)
      end

      def name=(_)
      end

      def hash()
      end

      def specs()
      end

      def options()
      end

      def remote!()
      end

      def cached!()
      end

      def install(spec, options = _)
      end

      def path()
      end
    end

    class Rubygems < ::Bundler::Source
      class Remote
        def original_uri()
        end

        def cache_slug()
        end

        def to_s()
        end

        def uri()
        end

        def anonymized_uri()
        end
      end

      API_REQUEST_LIMIT = T.let(T.unsafe(nil), Integer)

      API_REQUEST_SIZE = T.let(T.unsafe(nil), Integer)

      def equivalent_remotes?(other_remotes)
      end

      def ==(other)
      end

      def replace_remotes(other_remotes, allow_equivalent = _)
      end

      def fetchers()
      end

      def include?(o)
      end

      def name()
      end

      def eql?(other)
      end

      def cache(spec, custom_path = _)
      end

      def specs()
      end

      def to_s()
      end

      def unmet_deps()
      end

      def remotes()
      end

      def to_lock()
      end

      def can_lock?(spec)
      end

      def double_check_for(unmet_dependency_names)
      end

      def dependency_names_to_double_check()
      end

      def hash()
      end

      def caches()
      end

      def options()
      end

      def remote!()
      end

      def cached!()
      end

      def add_remote(source)
      end

      def install(spec, opts = _)
      end

      def cached_built_in_gem(spec)
      end
    end

    class Gemspec < ::Bundler::Source::Path
      def gemspec()
      end

      def as_path_source()
      end
    end

    def dependency_names=(_)
    end

    def inspect()
    end

    def version_message(spec)
    end

    def dependency_names()
    end

    def unmet_deps()
    end

    def include?(other)
    end

    def can_lock?(spec)
    end

    def double_check_for(*_)
    end

    def dependency_names_to_double_check()
    end

    def path?()
    end

    def extension_cache_path(spec)
    end
  end

  class GemRequireError < ::Bundler::BundlerError
    def orig_exception()
    end

    def status_code()
    end
  end
end

module Bundler::Molinillo
  module SpecificationProvider
    def dependencies_for(specification)
    end

    def name_for(dependency)
    end

    def search_for(dependency)
    end

    def name_for_locking_dependency_source()
    end

    def requirement_satisfied_by?(requirement, activated, spec)
    end

    def name_for_explicit_dependency_source()
    end

    def allow_missing?(dependency)
    end

    def sort_dependencies(dependencies, activated, conflicts)
    end
  end

  class ResolutionState < ::Struct
    def depth=(_)
    end

    def requirements()
    end

    def requirements=(_)
    end

    def depth()
    end

    def name=(_)
    end

    def unused_unwind_options()
    end

    def conflicts()
    end

    def activated()
    end

    def conflicts=(_)
    end

    def unused_unwind_options=(_)
    end

    def activated=(_)
    end

    def name()
    end

    def possibilities=(_)
    end

    def requirement()
    end

    def possibilities()
    end

    def requirement=(_)
    end
  end

  module UI
    def output()
    end

    def debug?()
    end

    def debug(depth = _)
    end

    def progress_rate()
    end

    def before_resolution()
    end

    def after_resolution()
    end

    def indicate_progress()
    end
  end

  class DependencyState < ::Bundler::Molinillo::ResolutionState
    def pop_possibility_state()
    end
  end

  class DependencyGraph
    include(TSort)
    include(Enumerable)

    class AddEdgeNoCircular < ::Bundler::Molinillo::DependencyGraph::Action
      def requirement()
      end

      def origin()
      end

      def destination()
      end

      def down(graph)
      end

      def up(graph)
      end

      def make_edge(graph)
      end
    end

    class Log
      extend(Enumerable)

      def detach_vertex_named(graph, name)
      end

      def delete_edge(graph, origin_name, destination_name, requirement)
      end

      def set_payload(graph, name, payload)
      end

      def reverse_each()
      end

      def add_vertex(graph, name, payload, root)
      end

      def tag(graph, tag)
      end

      def rewind_to(graph, tag)
      end

      def pop!(graph)
      end

      def add_edge_no_circular(graph, origin, destination, requirement)
      end

      def each()
      end
    end

    class DeleteEdge < ::Bundler::Molinillo::DependencyGraph::Action
      def origin_name()
      end

      def destination_name()
      end

      def requirement()
      end

      def down(graph)
      end

      def up(graph)
      end

      def make_edge(graph)
      end
    end

    class SetPayload < ::Bundler::Molinillo::DependencyGraph::Action
      def payload()
      end

      def up(graph)
      end

      def name()
      end

      def down(graph)
      end
    end

    class Vertex
      def path_to?(other)
      end

      def root?()
      end

      def ==(other)
      end

      def name()
      end

      def eql?(other)
      end

      def root()
      end

      def payload()
      end

      def requirements()
      end

      def payload=(_)
      end

      def root=(_)
      end

      def explicit_requirements()
      end

      def incoming_edges()
      end

      def name=(_)
      end

      def inspect()
      end

      def hash()
      end

      def successors()
      end

      def predecessors()
      end

      def recursive_successors()
      end

      def shallow_eql?(other)
      end

      def outgoing_edges()
      end

      def descendent?(other)
      end

      def ancestor?(other)
      end

      def outgoing_edges=(_)
      end

      def incoming_edges=(_)
      end

      def is_reachable_from?(other)
      end

      def recursive_predecessors()
      end
    end

    class Edge < ::Struct
      def destination()
      end

      def requirement=(_)
      end

      def origin=(_)
      end

      def destination=(_)
      end

      def requirement()
      end

      def origin()
      end
    end

    class Action
      def previous=(_)
      end

      def next=(_)
      end

      def up(graph)
      end

      def next()
      end

      def previous()
      end

      def down(graph)
      end
    end

    class Tag < ::Bundler::Molinillo::DependencyGraph::Action
      def tag()
      end

      def up(_graph)
      end

      def down(_graph)
      end
    end

    class AddVertex < ::Bundler::Molinillo::DependencyGraph::Action
      def root()
      end

      def payload()
      end

      def up(graph)
      end

      def name()
      end

      def down(graph)
      end
    end

    class DetachVertexNamed < ::Bundler::Molinillo::DependencyGraph::Action
      def up(graph)
      end

      def name()
      end

      def down(graph)
      end
    end

    def add_child_vertex(name, payload, parent_names, requirement)
    end

    def add_edge(origin, destination, requirement)
    end

    def detach_vertex_named(name)
    end

    def root_vertex_named(name)
    end

    def delete_edge(edge)
    end

    def vertex_named(name)
    end

    def set_payload(name, payload)
    end

    def inspect()
    end

    def ==(other)
    end

    def tsort_each_node()
    end

    def tsort_each_child(vertex, &block)
    end

    def add_vertex(name, payload, root = _)
    end

    def tag(tag)
    end

    def rewind_to(tag)
    end

    def log()
    end

    def vertices()
    end

    def to_dot(options = _)
    end

    def each()
    end
  end

  class VersionConflict < ::Bundler::Molinillo::ResolverError
    include(Bundler::Molinillo::Delegates::SpecificationProvider)

    def specification_provider()
    end

    def conflicts()
    end

    def message_with_trees(opts = _)
    end
  end

  VERSION = T.let(T.unsafe(nil), String)

  module Compatibility

  end

  class ResolverError < ::StandardError

  end

  class NoSuchDependencyError < ::Bundler::Molinillo::ResolverError
    def message()
    end

    def dependency=(_)
    end

    def dependency()
    end

    def required_by=(_)
    end

    def required_by()
    end
  end

  class Resolver
    class Resolution
      include(Bundler::Molinillo::Delegates::SpecificationProvider)
      include(Bundler::Molinillo::Delegates::ResolutionState)

      class Conflict < ::Struct
        def possibility()
        end

        def requirements()
        end

        def activated_by_name()
        end

        def requirements=(_)
        end

        def underlying_error()
        end

        def locked_requirement()
        end

        def existing=(_)
        end

        def possibility_set=(_)
        end

        def locked_requirement=(_)
        end

        def requirement_trees()
        end

        def requirement()
        end

        def existing()
        end

        def requirement_trees=(_)
        end

        def activated_by_name=(_)
        end

        def underlying_error=(_)
        end

        def possibility_set()
        end

        def requirement=(_)
        end
      end

      class PossibilitySet < ::Struct
        def latest_version()
        end

        def possibilities()
        end

        def dependencies()
        end

        def to_s()
        end

        def dependencies=(_)
        end

        def possibilities=(_)
        end
      end

      class UnwindDetails < ::Struct
        include(Comparable)

        def state_index()
        end

        def state_requirement()
        end

        def requirement_tree()
        end

        def conflicting_requirements()
        end

        def requirements_unwound_to_instead()
        end

        def <=>(other)
        end

        def reversed_requirement_tree_index()
        end

        def unwinding_to_primary_requirement?()
        end

        def sub_dependencies_to_avoid()
        end

        def all_requirements()
        end

        def requirement_trees()
        end

        def requirement_trees=(_)
        end

        def state_index=(_)
        end

        def state_requirement=(_)
        end

        def requirement_tree=(_)
        end

        def conflicting_requirements=(_)
        end

        def requirements_unwound_to_instead=(_)
        end
      end

      def iteration_rate=(_)
      end

      def base()
      end

      def started_at=(_)
      end

      def states=(_)
      end

      def original_requested()
      end

      def specification_provider()
      end

      def resolve()
      end

      def resolver_ui()
      end
    end

    def resolve(requested, base = _)
    end

    def resolver_ui()
    end

    def specification_provider()
    end
  end

  class CircularDependencyError < ::Bundler::Molinillo::ResolverError
    def dependencies()
    end
  end

  module Delegates
    module SpecificationProvider
      def dependencies_for(specification)
      end

      def name_for(dependency)
      end

      def search_for(dependency)
      end

      def name_for_locking_dependency_source()
      end

      def requirement_satisfied_by?(requirement, activated, spec)
      end

      def name_for_explicit_dependency_source()
      end

      def allow_missing?(dependency)
      end

      def sort_dependencies(dependencies, activated, conflicts)
      end
    end

    module ResolutionState
      def requirements()
      end

      def requirement()
      end

      def name()
      end

      def depth()
      end

      def possibilities()
      end

      def unused_unwind_options()
      end

      def conflicts()
      end

      def activated()
      end
    end
  end

  class PossibilityState < ::Bundler::Molinillo::ResolutionState

  end
end
