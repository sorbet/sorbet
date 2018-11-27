# typed: true

module Gem
  class StreamUI
    class SimpleProgressReporter
      include(Gem::DefaultUserInteraction)

      def count()
      end

      def updated(message)
      end

      def done()
      end
    end

    class VerboseProgressReporter
      include(Gem::DefaultUserInteraction)

      def count()
      end

      def updated(message)
      end

      def done()
      end
    end

    class SilentDownloadReporter
      def update(current)
      end

      def done()
      end

      def fetch(filename, filesize)
      end
    end

    class VerboseDownloadReporter
      def update(bytes)
      end

      def fetch(file_name, total_bytes)
      end

      def total_bytes()
      end

      def progress()
      end

      def done()
      end

      def file_name()
      end
    end

    class SilentProgressReporter
      def count()
      end

      def updated(message)
      end

      def done()
      end
    end

    def require_io_console()
    end

    def _gets_noecho()
    end

    def alert_warning(statement, question = _)
    end

    def say(statement = _)
    end

    def debug(statement)
    end

    def progress_reporter(*args)
    end

    def backtrace(exception)
    end

    def alert(statement, question = _)
    end

    def ask(question)
    end

    def ask_for_password(question)
    end

    def ask_yes_no(question, default = _)
    end

    def choose_from_list(question, list)
    end

    def download_reporter(*args)
    end

    def ins()
    end

    def outs()
    end

    def errs()
    end

    def close()
    end

    def tty?()
    end

    def alert_error(statement, question = _)
    end

    def terminate_interaction(status = _)
    end
  end

  module DefaultUserInteraction
    def ui=(new_ui)
    end

    def ui()
    end

    def use_ui(new_ui, &block)
    end
  end

  class Source
    include(Comparable)

    class Git < ::Gem::Source
      def root_dir()
      end

      def need_submodules()
      end

      def repository()
      end

      def <=>(other)
      end

      def remote()
      end

      def cache()
      end

      def checkout()
      end

      def ==(other)
      end

      def repo_cache_dir()
      end

      def rev_parse()
      end

      def dir_shortref()
      end

      def pretty_print(q)
      end

      def base_dir()
      end

      def specs()
      end

      def uri_hash()
      end

      def remote=(_)
      end

      def name()
      end

      def root_dir=(_)
      end

      def install_dir()
      end

      def download(full_spec, path)
      end

      def reference()
      end
    end

    class Local < ::Gem::Source
      def pretty_print(q)
      end

      def fetch_spec(name)
      end

      def <=>(other)
      end

      def load_specs(type)
      end

      def inspect()
      end

      def find_gem(gem_name, version = _, prerelease = _)
      end

      def download(spec, cache_dir = _)
      end
    end

    class Lock < ::Gem::Source
      def fetch_spec(name_tuple)
      end

      def <=>(other)
      end

      def ==(other)
      end

      def hash()
      end

      def uri()
      end

      def wrapped()
      end
    end

    class SpecificFile < ::Gem::Source
      def pretty_print(q)
      end

      def fetch_spec(name)
      end

      def load_specs(*a)
      end

      def <=>(other)
      end

      def spec()
      end

      def download(spec, dir = _)
      end

      def path()
      end
    end

    class Vendor < ::Gem::Source::Installed
      def <=>(other)
      end
    end

    class Installed < ::Gem::Source
      def pretty_print(q)
      end

      def download(spec, path)
      end

      def <=>(other)
      end
    end

    FILES = T.let(T.unsafe(nil), Hash)

    def pretty_print(q)
    end

    def api_uri()
    end

    def <=>(other)
    end

    def download(spec, dir = _)
    end

    def dependency_resolver_set()
    end

    def ==(other)
    end

    def hash()
    end

    def update_cache?()
    end

    def fetch_spec(name_tuple)
    end

    def load_specs(type)
    end

    def eql?(other)
    end

    def uri()
    end

    def cache_dir(uri)
    end
  end

  WRITE_BINARY_ERRORS = T.let(T.unsafe(nil), Array)

  RubyGemsVersion = T.let(T.unsafe(nil), String)

  RbConfigPriorities = T.let(T.unsafe(nil), Array)

  ConfigMap = T.let(T.unsafe(nil), Hash)

  RubyGemsPackageVersion = T.let(T.unsafe(nil), String)

  class Platform
    JAVA = T.let(T.unsafe(nil), Gem::Platform)

    CURRENT = T.let(T.unsafe(nil), String)

    MSWIN = T.let(T.unsafe(nil), Gem::Platform)

    MSWIN64 = T.let(T.unsafe(nil), Gem::Platform)

    RUBY = T.let(T.unsafe(nil), String)

    X64_MINGW = T.let(T.unsafe(nil), Gem::Platform)

    MINGW = T.let(T.unsafe(nil), Gem::Platform)

    def cpu()
    end

    def os()
    end

    def inspect()
    end

    def to_s()
    end

    def hash()
    end

    def to_a()
    end

    def cpu=(_)
    end

    def version=(_)
    end

    def ==(other)
    end

    def ===(other)
    end

    def os=(_)
    end

    def =~(other)
    end

    def eql?(other)
    end

    def version()
    end
  end

  class Specification < ::Gem::BasicSpecification
    include(Bundler::MatchPlatform)
    include(Bundler::GemHelpers)
    extend(Gem::Deprecate)
    extend(Enumerable)

    INITIALIZE_CODE_FOR_DEFAULTS = T.let(T.unsafe(nil), Hash)

    NONEXISTENT_SPECIFICATION_VERSION = T.let(T.unsafe(nil), Integer)

    CURRENT_SPECIFICATION_VERSION = T.let(T.unsafe(nil), Integer)

    SPECIFICATION_VERSION_HISTORY = T.let(T.unsafe(nil), Hash)

    MARSHAL_FIELDS = T.let(T.unsafe(nil), Hash)

    NOT_FOUND = T.let(T.unsafe(nil), T.untyped)

    TODAY = T.let(T.unsafe(nil), Time)

    VALID_NAME_PATTERN = T.let(T.unsafe(nil), Regexp)

    DateTimeFormat = T.let(T.unsafe(nil), Regexp)

    EMPTY = T.let(T.unsafe(nil), Array)

    DateLike = T.let(T.unsafe(nil), Object)

    def relative_loaded_from=(_)
    end

    def validate(packaging = _)
    end

    def groups()
    end

    def validate_metadata()
    end

    def warning(statement)
    end

    def validate_permissions()
    end

    def validate_dependencies()
    end

    def rubygems_version()
    end

    def stubbed?()
    end

    def specification_version=(_)
    end

    def to_spec()
    end

    def extension_dir()
    end

    def full_gem_path()
    end

    def <=>(other)
    end

    def ==(other)
    end

    def base_dir()
    end

    def eql?(other)
    end

    def inspect()
    end

    def full_name()
    end

    def conflicts()
    end

    def method_missing(sym, *a, &b)
    end

    def dependencies()
    end

    def platform()
    end

    def to_s()
    end

    def specification_version()
    end

    def date()
    end

    def summary()
    end

    def authors()
    end

    def autorequire()
    end

    def cert_chain()
    end

    def description()
    end

    def email()
    end

    def extensions()
    end

    def extra_rdoc_files()
    end

    def homepage()
    end

    def licenses()
    end

    def metadata()
    end

    def post_install_message()
    end

    def rdoc_options()
    end

    def required_ruby_version()
    end

    def required_rubygems_version()
    end

    def rubyforge_project()
    end

    def signing_key()
    end

    def test_files()
    end

    def hash()
    end

    def add_bindir(executables)
    end

    def author=(o)
    end

    def authors=(value)
    end

    def license=(o)
    end

    def licenses=(licenses)
    end

    def platform=(platform)
    end

    def require_paths=(val)
    end

    def add_development_dependency(gem, *requirements)
    end

    def add_runtime_dependency(gem, *requirements)
    end

    def installed_by_version()
    end

    def installed_by_version=(version)
    end

    def required_ruby_version=(req)
    end

    def required_rubygems_version=(req)
    end

    def test_files=(files)
    end

    def activated()
    end

    def default_executable()
    end

    def original_platform()
    end

    def activated=(_)
    end

    def name()
    end

    def original_name()
    end

    def gems_dir()
    end

    def traverse(trail = _, visited = _, &block)
    end

    def has_conflicts?()
    end

    def _dump(limit)
    end

    def conficts_when_loaded_with?(list_of_specs)
    end

    def load_paths()
    end

    def remote()
    end

    def reset_nil_attributes_to_default()
    end

    def location()
    end

    def relative_loaded_from()
    end

    def rg_full_gem_path()
    end

    def rg_loaded_from()
    end

    def rg_extension_dir()
    end

    def git_version()
    end

    def loaded_from()
    end

    def to_gemfile(path = _)
    end

    def nondevelopment_dependencies()
    end

    def date=(date)
    end

    def source()
    end

    def sort_obj()
    end

    def raise_if_conflicts()
    end

    def activate_dependencies()
    end

    def add_self_to_load_path()
    end

    def runtime_dependencies()
    end

    def abbreviate()
    end

    def files=(files)
    end

    def rdoc_options=(options)
    end

    def extra_rdoc_files=(files)
    end

    def cert_chain=(_)
    end

    def sanitize()
    end

    def summary=(str)
    end

    def sanitize_string(string)
    end

    def description=(str)
    end

    def post_install_message=(_)
    end

    def add_dependency(gem, *requirements)
    end

    def activated?()
    end

    def activate()
    end

    def author()
    end

    def bin_dir()
    end

    def gem_dir()
    end

    def build_args()
    end

    def build_info_file()
    end

    def remote=(_)
    end

    def build_extensions()
    end

    def build_info_dir()
    end

    def bundled_gem_in_old_ruby?()
    end

    def version()
    end

    def cache_dir()
    end

    def cache_file()
    end

    def satisfies_requirement?(dependency)
    end

    def requirements()
    end

    def bin_file(name)
    end

    def name=(_)
    end

    def default_value(name)
    end

    def executables()
    end

    def dependent_gems()
    end

    def email=(_)
    end

    def homepage=(_)
    end

    def metadata=(_)
    end

    def bindir=(_)
    end

    def doc_dir(type = _)
    end

    def bindir()
    end

    def rubygems_version=(_)
    end

    def location=(_)
    end

    def signing_key=(_)
    end

    def encode_with(coder)
    end

    def autorequire=(_)
    end

    def default_executable=(_)
    end

    def original_platform=(_)
    end

    def rubyforge_project=(_)
    end

    def executable=(o)
    end

    def dependent_specs()
    end

    def development_dependencies()
    end

    def file_name()
    end

    def executables=(value)
    end

    def extensions=(extensions)
    end

    def executable()
    end

    def for_cache()
    end

    def has_rdoc=(ignored)
    end

    def has_unit_tests?()
    end

    def has_rdoc()
    end

    def yaml_initialize(tag, vals)
    end

    def has_test_suite?()
    end

    def init_with(coder)
    end

    def version=(version)
    end

    def has_rdoc?()
    end

    def mark_version()
    end

    def lib_files()
    end

    def license()
    end

    def internal_init()
    end

    def normalize()
    end

    def missing_extensions?()
    end

    def name_tuple()
    end

    def source=(_)
    end

    def pretty_print(q)
    end

    def require_path()
    end

    def require_path=(path)
    end

    def requirements=(req)
    end

    def files()
    end

    def ri_dir()
    end

    def spec_dir()
    end

    def spec_file()
    end

    def spec_name()
    end

    def test_file()
    end

    def test_file=(file)
    end

    def to_ruby()
    end

    def raw_require_paths()
    end

    def to_ruby_for_cache()
    end

    def to_yaml(opts = _)
    end
  end

  module UserInteraction
    include(Gem::DefaultUserInteraction)

    def ask_yes_no(question, default = _)
    end

    def choose_from_list(question, list)
    end

    def say(statement = _)
    end

    def alert_warning(statement, question = _)
    end

    def verbose(msg = _)
    end

    def terminate_interaction(exit_code = _)
    end

    def alert(statement, question = _)
    end

    def alert_error(statement, question = _)
    end

    def ask(question)
    end

    def ask_for_password(prompt)
    end
  end

  class RequestSet
    include(TSort)

    class Lockfile
      class ParseError < ::Gem::Exception
        def column()
        end

        def line()
        end

        def path()
        end
      end

      class Parser
        def parse_dependency(name, op)
        end

        def get(expected_types = _, expected_value = _)
        end

        def parse_DEPENDENCIES()
        end

        def parse_GIT()
        end

        def parse()
        end

        def parse_GEM()
        end

        def parse_PATH()
        end

        def parse_PLATFORMS()
        end
      end

      class Tokenizer
        class Token < ::Struct
          def column()
          end

          def value()
          end

          def line()
          end

          def type=(_)
          end

          def type()
          end

          def column=(_)
          end

          def value=(_)
          end

          def line=(_)
          end
        end

        EOF = T.let(T.unsafe(nil), Gem::RequestSet::Lockfile::Tokenizer::Token)

        def skip(type)
        end

        def empty?()
        end

        def make_parser(set, platforms)
        end

        def peek()
        end

        def to_a()
        end

        def token_pos(byte_offset)
        end

        def unshift(token)
        end

        def next_token()
        end

        def shift()
        end
      end

      def add_PATH(out, path_requests)
      end

      def add_PLATFORMS(out)
      end

      def platforms()
      end

      def to_s()
      end

      def write()
      end

      def relative_path_from(dest, base)
      end

      def add_DEPENDENCIES(out)
      end

      def add_GEM(out, spec_groups)
      end

      def spec_groups()
      end

      def add_GIT(out, git_requests)
      end
    end

    class GemDependencyAPI
      WINDOWS = T.let(T.unsafe(nil), Hash)

      VERSION_MAP = T.let(T.unsafe(nil), Hash)

      ENGINE_MAP = T.let(T.unsafe(nil), Hash)

      PLATFORM_MAP = T.let(T.unsafe(nil), Hash)

      def git_set()
      end

      def git_source(name, &callback)
      end

      def load()
      end

      def platforms(*platforms)
      end

      def find_gemspec(name, path)
      end

      def vendor_set()
      end

      def gem_git_reference(options)
      end

      def ruby(version, options = _)
      end

      def git(repository)
      end

      def without_groups()
      end

      def gem_deps_file()
      end

      def gemspec(options = _)
      end

      def group(*groups)
      end

      def gem(name, *requirements)
      end

      def source(url)
      end

      def dependencies()
      end

      def platform(*platforms)
      end

      def installing=(installing)
      end

      def without_groups=(_)
      end

      def requires()
      end
    end

    def development_shallow=(_)
    end

    def development()
    end

    def ignore_dependencies=(_)
    end

    def soft_missing=(_)
    end

    def always_install()
    end

    def development_shallow()
    end

    def git_set()
    end

    def ignore_dependencies()
    end

    def prerelease()
    end

    def resolver()
    end

    def sets()
    end

    def errors()
    end

    def vendor_set()
    end

    def soft_missing()
    end

    def tsort_each_node(&block)
    end

    def remote()
    end

    def always_install=(_)
    end

    def remote=(_)
    end

    def import(deps)
    end

    def load_gemdeps(path, without_groups = _, installing = _)
    end

    def tsort_each_child(node)
    end

    def source_set()
    end

    def install_into(dir, force = _, options = _)
    end

    def pretty_print(q)
    end

    def resolve_current()
    end

    def sorted_requests()
    end

    def install_from_gemdeps(options, &block)
    end

    def resolve(set = _)
    end

    def specs()
    end

    def gem(name, *reqs)
    end

    def specs_in(dir)
    end

    def dependencies()
    end

    def install(options, &block)
    end

    def install_dir()
    end

    def prerelease=(_)
    end

    def development=(_)
    end
  end

  class List
    include(Enumerable)
    extend T::Generic
    Elem = type_member(:out)

    def pretty_print(q)
    end

    def value()
    end

    def tail()
    end

    def value=(_)
    end

    def tail=(_)
    end

    def each()
    end

    def to_a()
    end

    def prepend(value)
    end
  end

  class SilentUI < ::Gem::StreamUI
    def progress_reporter(*args)
    end

    def download_reporter(*args)
    end

    def close()
    end
  end

  class Dependency
    TYPES = T.let(T.unsafe(nil), Array)

    def match?(obj, version = _, allow_prerelease = _)
    end

    def to_yaml_properties()
    end

    def <=>(other)
    end

    def type()
    end

    def ==(other)
    end

    def to_s()
    end

    def prerelease?()
    end

    def =~(other)
    end

    def ===(other)
    end

    def groups()
    end

    def merge(other)
    end

    def name()
    end

    def groups=(_)
    end

    def to_spec()
    end

    def eql?(other)
    end

    def all_sources()
    end

    def to_lock()
    end

    def pretty_print(q)
    end

    def specific?()
    end

    def runtime?()
    end

    def matches_spec?(spec)
    end

    def matching_specs(platform_only = _)
    end

    def name=(_)
    end

    def hash()
    end

    def inspect()
    end

    def latest_version?()
    end

    def source()
    end

    def requirement()
    end

    def encode_with(coder)
    end

    def requirements_list()
    end

    def prerelease=(_)
    end

    def to_specs()
    end

    def source=(_)
    end

    def all_sources=(_)
    end
  end

  class StubSpecification < ::Gem::BasicSpecification
    OPEN_MODE = T.let(T.unsafe(nil), String)

    PREFIX = T.let(T.unsafe(nil), String)

    class StubLine
      REQUIRE_PATH_LIST = T.let(T.unsafe(nil), Hash)

      NO_EXTENSIONS = T.let(T.unsafe(nil), Array)

      REQUIRE_PATHS = T.let(T.unsafe(nil), Hash)

      def require_paths()
      end

      def name()
      end

      def extensions()
      end

      def platform()
      end

      def full_name()
      end

      def version()
      end
    end

    def default_gem?()
    end

    def activated?()
    end

    def full_name()
    end

    def base_dir()
    end

    def name()
    end

    def build_extensions()
    end

    def raw_require_paths()
    end

    def platform()
    end

    def to_spec()
    end

    def valid?()
    end

    def gems_dir()
    end

    def stubbed?()
    end

    def missing_extensions?()
    end

    def version()
    end

    def extensions()
    end
  end

  class Requirement
    PATTERN_RAW = T.let(T.unsafe(nil), String)

    PATTERN = T.let(T.unsafe(nil), Regexp)

    DefaultRequirement = T.let(T.unsafe(nil), Array)

    class BadRequirementError < ::ArgumentError

    end

    OPS = T.let(T.unsafe(nil), Hash)

    SOURCE_SET_REQUIREMENT = T.let(T.unsafe(nil), T.untyped)

    def marshal_dump()
    end

    def marshal_load(array)
    end

    def to_yaml_properties()
    end

    def init_with(coder)
    end

    def yaml_initialize(tag, vals)
    end

    def ===(version)
    end

    def prerelease?()
    end

    def =~(version)
    end

    def to_s()
    end

    def ==(other)
    end

    def for_lockfile()
    end

    def exact?()
    end

    def pretty_print(q)
    end

    def requirements()
    end

    def none?()
    end

    def satisfied_by?(version)
    end

    def specific?()
    end

    def concat(new)
    end

    def hash()
    end

    def as_list()
    end

    def encode_with(coder)
    end
  end

  class MissingSpecError < ::Gem::LoadError
    def message()
    end
  end

  class SpecFetcher
    include(Gem::Text)
    include(Gem::UserInteraction)
    include(Gem::DefaultUserInteraction)

    def sources()
    end

    def detect(type = _)
    end

    def tuples_for(source, type, gracefully_ignore = _)
    end

    def available_specs(type)
    end

    def spec_for_dependency(dependency, matching_platform = _)
    end

    def search_for_dependency(dependency, matching_platform = _)
    end

    def specs()
    end

    def suggest_gems_from_name(gem_name, type = _)
    end

    def prerelease_specs()
    end

    def latest_specs()
    end
  end

  class MissingSpecVersionError < ::Gem::MissingSpecError
    def specs()
    end
  end

  class InvalidSpecificationException < ::Gem::Exception

  end

  module BundlerVersionFinder

  end

  class Version
    include(Comparable)

    ANCHORED_VERSION_PATTERN = T.let(T.unsafe(nil), Regexp)

    VERSION_PATTERN = T.let(T.unsafe(nil), String)

    def canonical_segments()
    end

    def marshal_dump()
    end

    def marshal_load(array)
    end

    def to_yaml_properties()
    end

    def release()
    end

    def <=>(other)
    end

    def approximate_recommendation()
    end

    def init_with(coder)
    end

    def yaml_initialize(tag, map)
    end

    def to_s()
    end

    def prerelease?()
    end

    def eql?(other)
    end

    def version()
    end

    def pretty_print(q)
    end

    def inspect()
    end

    def hash()
    end

    def segments()
    end

    def encode_with(coder)
    end

    def bump()
    end
  end

  class ConflictError < ::Gem::LoadError
    def conflicts()
    end

    def target()
    end
  end

  class GemNotFoundException < ::Gem::Exception

  end

  class SourceList
    include(Enumerable)

    def first()
    end

    def to_ary()
    end

    def <<(obj)
    end

    def delete(source)
    end

    def replace(other)
    end

    def clear()
    end

    def ==(other)
    end

    def to_a()
    end

    def sources()
    end

    def include?(other)
    end

    def empty?()
    end

    def each()
    end

    def each_source(&b)
    end
  end

  class ErrorReason

  end

  class PlatformMismatch < ::Gem::ErrorReason
    def wordy()
    end

    def platforms()
    end

    def name()
    end

    def version()
    end

    def add_platform(platform)
    end
  end

  class SourceFetchProblem < ::Gem::ErrorReason
    def wordy()
    end

    def error()
    end

    def exception()
    end

    def source()
    end
  end

  class CommandLineError < ::Gem::Exception

  end

  class DependencyError < ::Gem::Exception

  end

  class BasicSpecification
    def source_paths()
    end

    def lib_dirs_glob()
    end

    def base_dir=(_)
    end

    def extension_dir=(_)
    end

    def ignored=(_)
    end

    def full_gem_path=(_)
    end

    def datadir()
    end

    def default_gem?()
    end

    def full_require_paths()
    end

    def activated?()
    end

    def base_dir()
    end

    def gem_dir()
    end

    def loaded_from()
    end

    def loaded_from=(_)
    end

    def name()
    end

    def gem_build_complete_path()
    end

    def internal_init()
    end

    def to_spec()
    end

    def gems_dir()
    end

    def version()
    end

    def require_paths()
    end

    def this()
    end

    def matches_for_glob(glob)
    end

    def full_name()
    end

    def extension_dir()
    end

    def full_gem_path()
    end

    def extensions_dir()
    end

    def contains_requirable_file?(file)
    end

    def stubbed?()
    end

    def platform()
    end

    def to_fullpath(path)
    end

    def raw_require_paths()
    end
  end

  class DependencyRemovalException < ::Gem::Exception

  end

  class ConfigFile
    include(Gem::UserInteraction)
    include(Gem::DefaultUserInteraction)

    DEFAULT_VERBOSITY = T.let(T.unsafe(nil), TrueClass)

    DEFAULT_UPDATE_SOURCES = T.let(T.unsafe(nil), TrueClass)

    OPERATING_SYSTEM_DEFAULTS = T.let(T.unsafe(nil), Hash)

    PLATFORM_DEFAULTS = T.let(T.unsafe(nil), Hash)

    SYSTEM_CONFIG_PATH = T.let(T.unsafe(nil), String)

    SYSTEM_WIDE_CONFIG_FILE = T.let(T.unsafe(nil), String)

    DEFAULT_BULK_THRESHOLD = T.let(T.unsafe(nil), Integer)

    def ssl_client_cert()
    end

    def path()
    end

    def disable_default_gem_server()
    end

    def args()
    end

    def to_yaml()
    end

    def bulk_threshold()
    end

    def load_file(filename)
    end

    def ==(other)
    end

    def config_file_name()
    end

    def [](key)
    end

    def []=(key, value)
    end

    def really_verbose()
    end

    def write()
    end

    def handle_arguments(arg_list)
    end

    def api_keys()
    end

    def load_api_keys()
    end

    def check_credentials_permissions()
    end

    def credentials_path()
    end

    def rubygems_api_key()
    end

    def rubygems_api_key=(api_key)
    end

    def set_api_key(host, api_key)
    end

    def home=(_)
    end

    def backtrace=(_)
    end

    def bulk_threshold=(_)
    end

    def verbose=(_)
    end

    def update_sources=(_)
    end

    def disable_default_gem_server=(_)
    end

    def ssl_ca_cert=(_)
    end

    def unset_api_key!()
    end

    def backtrace()
    end

    def sources()
    end

    def home()
    end

    def sources=(_)
    end

    def each(&block)
    end

    def path=(_)
    end

    def update_sources()
    end

    def verbose()
    end

    def ssl_verify_mode()
    end

    def ssl_ca_cert()
    end
  end

  class DependencyResolutionError < ::Gem::DependencyError
    def conflicting_dependencies()
    end

    def conflict()
    end
  end

  class EndOfYAMLException < ::Gem::Exception

  end

  class GemNotInHomeException < ::Gem::Exception
    def spec()
    end

    def spec=(_)
    end
  end

  module Deprecate

  end

  class DocumentError < ::Gem::Exception

  end

  class FilePermissionError < ::Gem::Exception
    def directory()
    end
  end

  class SpecificGemNotFoundException < ::Gem::GemNotFoundException
    def errors()
    end

    def name()
    end

    def version()
    end
  end

  class ImpossibleDependenciesError < ::Gem::Exception
    def request()
    end

    def build_message()
    end

    def dependency()
    end

    def conflicts()
    end
  end

  class PathSupport
    def home()
    end

    def path()
    end

    def spec_cache_dir()
    end
  end

  class FormatException < ::Gem::Exception
    def file_path=(_)
    end

    def file_path()
    end
  end

  class InstallError < ::Gem::Exception

  end

  class RuntimeRequirementNotMetError < ::Gem::InstallError
    def suggestion()
    end

    def suggestion=(_)
    end

    def message()
    end
  end

  class OperationNotSupportedError < ::Gem::Exception

  end

  class RemoteError < ::Gem::Exception

  end

  class RemoteInstallationCancelled < ::Gem::Exception

  end

  class RemoteInstallationSkipped < ::Gem::Exception

  end

  class RemoteSourceException < ::Gem::Exception

  end

  class RubyVersionMismatch < ::Gem::Exception

  end

  class VerificationError < ::Gem::Exception

  end

  class SystemExitException < ::SystemExit
    def exit_code()
    end

    def exit_code=(_)
    end
  end

  class UnsatisfiableDependencyError < ::Gem::DependencyError
    def name()
    end

    def dependency()
    end

    def errors()
    end

    def errors=(_)
    end

    def version()
    end
  end

  class Exception < ::RuntimeError
    def source_exception()
    end

    def source_exception=(_)
    end
  end

  class ConsoleUI < ::Gem::StreamUI

  end

  VERSION = T.let(T.unsafe(nil), String)

  MARSHAL_SPEC_DIR = T.let(T.unsafe(nil), String)

  RUBYGEMS_DIR = T.let(T.unsafe(nil), String)

  WIN_PATTERNS = T.let(T.unsafe(nil), Array)

  class Installer
    include(Gem::UserInteraction)
    include(Gem::DefaultUserInteraction)

    ENV_PATHS = T.let(T.unsafe(nil), Array)

    ExtensionBuildError = Gem::Ext::BuildError

    class FakePackage
      def spec=(_)
      end

      def spec()
      end

      def extract_files(destination_dir, pattern = _)
      end

      def copy_to(path)
      end
    end

    def generate_windows_script(filename, bindir)
    end

    def write_cache_file()
    end

    def unpack(directory)
    end

    def windows_stub_script(bindir, bin_file_name)
    end

    def generate_bin_script(filename, bindir)
    end

    def generate_bin_symlink(filename, bindir)
    end

    def app_script_text(bin_file_name)
    end

    def spec()
    end

    def bin_dir()
    end

    def ensure_loadable_spec()
    end

    def gem_dir()
    end

    def ensure_required_rubygems_version_met()
    end

    def ensure_required_ruby_version_met()
    end

    def ensure_dependencies_met()
    end

    def build_extensions()
    end

    def installed_specs()
    end

    def verify_gem_home(unpack = _)
    end

    def verify_spec_name()
    end

    def shebang(bin_file_name)
    end

    def extension_build_error(build_dir, output, backtrace = _)
    end

    def dir()
    end

    def build_root()
    end

    def extract_files()
    end

    def process_options()
    end

    def gem()
    end

    def check_executable_overwrite(filename)
    end

    def gem_home()
    end

    def check_that_user_bin_dir_is_in_path()
    end

    def formatted_program_filename(filename)
    end

    def spec_file()
    end

    def options()
    end

    def pre_install_checks()
    end

    def run_pre_install_hooks()
    end

    def default_spec_file()
    end

    def install()
    end

    def write_default_spec()
    end

    def write_build_info_file()
    end

    def run_post_build_hooks()
    end

    def generate_bin()
    end

    def write_spec()
    end

    def extract_bin()
    end

    def run_post_install_hooks()
    end

    def ensure_dependency(spec, dependency)
    end

    def installation_satisfies_dependency?(dependency)
    end
  end

  class Licenses
    extend(Gem::Text)

    REGEXP = T.let(T.unsafe(nil), Regexp)

    NONSTANDARD = T.let(T.unsafe(nil), String)

    IDENTIFIERS = T.let(T.unsafe(nil), Array)
  end

  class Resolver
    include(Gem::Resolver::Molinillo::SpecificationProvider)
    include(Gem::Resolver::Molinillo::UI)

    class ComposedSet < ::Gem::Resolver::Set
      def prefetch(reqs)
      end

      def remote=(remote)
      end

      def find_all(req)
      end

      def prerelease=(allow_prerelease)
      end

      def sets()
      end

      def errors()
      end
    end

    class Set
      def prefetch(reqs)
      end

      def remote=(_)
      end

      def find_all(req)
      end

      def remote()
      end

      def remote?()
      end

      def prerelease()
      end

      def prerelease=(_)
      end

      def errors()
      end

      def errors=(_)
      end
    end

    class CurrentSet < ::Gem::Resolver::Set
      def find_all(req)
      end
    end

    class Stats
      PATTERN = T.let(T.unsafe(nil), String)

      def display()
      end

      def requirement!()
      end

      def record_depth(stack)
      end

      def record_requirements(reqs)
      end

      def backtracking!()
      end

      def iteration!()
      end
    end

    class Specification
      def local?()
      end

      def spec()
      end

      def full_name()
      end

      def installable_platform?()
      end

      def fetch_development_dependencies()
      end

      def source()
      end

      def dependencies()
      end

      def name()
      end

      def install(options = _)
      end

      def platform()
      end

      def set()
      end

      def version()
      end
    end

    class APISpecification < ::Gem::Resolver::Specification
      def source()
      end

      def ==(other)
      end

      def installable_platform?()
      end

      def spec()
      end

      def fetch_development_dependencies()
      end

      def pretty_print(q)
      end
    end

    class DependencyRequest
      def pretty_print(q)
      end

      def explicit?()
      end

      def request_context()
      end

      def dependency()
      end

      def match?(spec, allow_prerelease = _)
      end

      def matches_spec?(spec)
      end

      def type()
      end

      def ==(other)
      end

      def to_s()
      end

      def development?()
      end

      def requester()
      end

      def requirement()
      end

      def name()
      end

      def implicit?()
      end
    end

    module Molinillo
      module UI
        def debug(depth = _)
        end

        def output()
        end

        def progress_rate()
        end

        def before_resolution()
        end

        def after_resolution()
        end

        def debug?()
        end

        def indicate_progress()
        end
      end

      class VersionConflict < ::Gem::Resolver::Molinillo::ResolverError
        def conflicts()
        end
      end

      VERSION = T.let(T.unsafe(nil), String)

      class ResolutionState < ::Struct
        def possibilities()
        end

        def requirements()
        end

        def requirements=(_)
        end

        def depth()
        end

        def name=(_)
        end

        def possibilities=(_)
        end

        def depth=(_)
        end

        def conflicts()
        end

        def activated()
        end

        def activated=(_)
        end

        def name()
        end

        def requirement()
        end

        def conflicts=(_)
        end

        def requirement=(_)
        end
      end

      class ResolverError < ::StandardError

      end

      class NoSuchDependencyError < ::Gem::Resolver::Molinillo::ResolverError
        def required_by()
        end

        def dependency()
        end

        def message()
        end

        def dependency=(_)
        end

        def required_by=(_)
        end
      end

      class PossibilityState < ::Gem::Resolver::Molinillo::ResolutionState

      end

      class CircularDependencyError < ::Gem::Resolver::Molinillo::ResolverError
        def dependencies()
        end
      end

      class Resolver
        class Resolution
          include(Gem::Resolver::Molinillo::Delegates::SpecificationProvider)
          include(Gem::Resolver::Molinillo::Delegates::ResolutionState)

          class Conflict < ::Struct
            def requirements()
            end

            def existing=(_)
            end

            def possibility=(_)
            end

            def locked_requirement=(_)
            end

            def requirement_trees=(_)
            end

            def requirements=(_)
            end

            def activated_by_name=(_)
            end

            def existing()
            end

            def requirement()
            end

            def possibility()
            end

            def locked_requirement()
            end

            def activated_by_name()
            end

            def requirement_trees()
            end

            def requirement=(_)
            end
          end

          def iteration_rate=(_)
          end

          def original_requested()
          end

          def resolve()
          end

          def started_at=(_)
          end

          def states=(_)
          end

          def base()
          end

          def specification_provider()
          end

          def resolver_ui()
          end
        end

        def specification_provider()
        end

        def resolver_ui()
        end

        def resolve(requested, base = _)
        end
      end

      class DependencyState < ::Gem::Resolver::Molinillo::ResolutionState
        def pop_possibility_state()
        end
      end

      module SpecificationProvider
        def search_for(dependency)
        end

        def dependencies_for(specification)
        end

        def requirement_satisfied_by?(requirement, activated, spec)
        end

        def name_for(dependency)
        end

        def name_for_explicit_dependency_source()
        end

        def name_for_locking_dependency_source()
        end

        def sort_dependencies(dependencies, activated, conflicts)
        end

        def allow_missing?(dependency)
        end
      end

      class DependencyGraph
        include(TSort)
        include(Enumerable)

        class Edge < ::Struct
          def origin()
          end

          def origin=(_)
          end

          def destination=(_)
          end

          def destination()
          end

          def requirement()
          end

          def requirement=(_)
          end
        end

        class Action
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

          def previous=(_)
          end
        end

        class Tag < ::Gem::Resolver::Molinillo::DependencyGraph::Action
          def tag()
          end

          def up(_graph)
          end

          def down(_graph)
          end
        end

        class AddVertex < ::Gem::Resolver::Molinillo::DependencyGraph::Action
          def root()
          end

          def up(graph)
          end

          def payload()
          end

          def name()
          end

          def down(graph)
          end
        end

        class DetachVertexNamed < ::Gem::Resolver::Molinillo::DependencyGraph::Action
          def up(graph)
          end

          def name()
          end

          def down(graph)
          end
        end

        class AddEdgeNoCircular < ::Gem::Resolver::Molinillo::DependencyGraph::Action
          def up(graph)
          end

          def origin()
          end

          def requirement()
          end

          def make_edge(graph)
          end

          def destination()
          end

          def down(graph)
          end
        end

        class Log
          extend(Enumerable)

          def rewind_to(graph, tag)
          end

          def reverse_each()
          end

          def pop!(graph)
          end

          def add_vertex(graph, name, payload, root)
          end

          def add_edge_no_circular(graph, origin, destination, requirement)
          end

          def tag(graph, tag)
          end

          def each()
          end

          def detach_vertex_named(graph, name)
          end

          def delete_edge(graph, origin_name, destination_name, requirement)
          end

          def set_payload(graph, name, payload)
          end
        end

        class DeleteEdge < ::Gem::Resolver::Molinillo::DependencyGraph::Action
          def up(graph)
          end

          def requirement()
          end

          def make_edge(graph)
          end

          def origin_name()
          end

          def destination_name()
          end

          def down(graph)
          end
        end

        class SetPayload < ::Gem::Resolver::Molinillo::DependencyGraph::Action
          def up(graph)
          end

          def payload()
          end

          def name()
          end

          def down(graph)
          end
        end

        class Vertex
          def successors()
          end

          def root?()
          end

          def outgoing_edges()
          end

          def recursive_predecessors()
          end

          def recursive_successors()
          end

          def predecessors()
          end

          def shallow_eql?(other)
          end

          def explicit_requirements()
          end

          def ancestor?(other)
          end

          def outgoing_edges=(_)
          end

          def incoming_edges=(_)
          end

          def is_reachable_from?(other)
          end

          def descendent?(other)
          end

          def name()
          end

          def eql?(other)
          end

          def ==(other)
          end

          def path_to?(other)
          end

          def root()
          end

          def requirements()
          end

          def name=(_)
          end

          def inspect()
          end

          def hash()
          end

          def incoming_edges()
          end

          def payload()
          end

          def payload=(_)
          end

          def root=(_)
          end
        end

        def set_payload(name, payload)
        end

        def rewind_to(tag)
        end

        def vertex_named(name)
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

        def to_dot(options = _)
        end

        def tag(tag)
        end

        def add_child_vertex(name, payload, parent_names, requirement)
        end

        def log()
        end

        def each()
        end

        def detach_vertex_named(name)
        end

        def root_vertex_named(name)
        end

        def add_edge(origin, destination, requirement)
        end

        def delete_edge(edge)
        end

        def vertices()
        end
      end

      module Delegates
        module ResolutionState
          def possibilities()
          end

          def requirements()
          end

          def requirement()
          end

          def name()
          end

          def depth()
          end

          def conflicts()
          end

          def activated()
          end
        end

        module SpecificationProvider
          def search_for(dependency)
          end

          def dependencies_for(specification)
          end

          def requirement_satisfied_by?(requirement, activated, spec)
          end

          def name_for(dependency)
          end

          def name_for_explicit_dependency_source()
          end

          def name_for_locking_dependency_source()
          end

          def sort_dependencies(dependencies, activated, conflicts)
          end

          def allow_missing?(dependency)
          end
        end
      end
    end

    class IndexSpecification < ::Gem::Resolver::Specification
      def pretty_print(q)
      end

      def dependencies()
      end

      def inspect()
      end

      def spec()
      end
    end

    class Conflict
      def dependency()
      end

      def conflicting_dependencies()
      end

      def explanation()
      end

      def explain()
      end

      def ==(other)
      end

      def activated()
      end

      def requester()
      end

      def failed_dep()
      end

      def request_path(current)
      end

      def for_spec?(spec)
      end

      def pretty_print(q)
      end
    end

    class InstallerSet < ::Gem::Resolver::Set
      def pretty_print(q)
      end

      def ignore_dependencies=(_)
      end

      def add_always_install(dependency)
      end

      def local?(dep_name)
      end

      def add_local(dep_name, spec, source)
      end

      def always_install()
      end

      def consider_local?()
      end

      def ignore_dependencies()
      end

      def inspect()
      end

      def ignore_installed=(_)
      end

      def errors()
      end

      def load_spec(name, ver, platform, source)
      end

      def prefetch(reqs)
      end

      def find_all(req)
      end

      def remote=(remote)
      end

      def prerelease=(allow_prerelease)
      end

      def ignore_installed()
      end

      def remote_set()
      end

      def consider_remote?()
      end
    end

    class ActivationRequest
      def pretty_print(q)
      end

      def version()
      end

      def spec()
      end

      def to_s()
      end

      def full_name()
      end

      def ==(other)
      end

      def development?()
      end

      def inspect()
      end

      def request()
      end

      def others_possible?()
      end

      def name()
      end

      def parent()
      end

      def download(path)
      end

      def installed?()
      end

      def full_spec()
      end
    end

    class LockSet < ::Gem::Resolver::Set
      def pretty_print(q)
      end

      def specs()
      end

      def find_all(req)
      end

      def load_spec(name, version, platform, source)
      end

      def add(name, version, platform)
      end
    end

    class InstalledSpecification < ::Gem::Resolver::SpecSpecification
      def ==(other)
      end

      def installable_platform?()
      end

      def install(options = _)
      end

      def source()
      end

      def pretty_print(q)
      end
    end

    class IndexSet < ::Gem::Resolver::Set
      def find_all(req)
      end

      def pretty_print(q)
      end
    end

    class LockSpecification < ::Gem::Resolver::Specification
      def sources()
      end

      def pretty_print(q)
      end

      def add_dependency(dependency)
      end

      def spec()
      end

      def install(options = _)
      end
    end

    class LocalSpecification < ::Gem::Resolver::SpecSpecification
      def pretty_print(q)
      end

      def installable_platform?()
      end

      def local?()
      end
    end

    class APISet < ::Gem::Resolver::Set
      def pretty_print(q)
      end

      def prefetch(reqs)
      end

      def find_all(req)
      end

      def dep_uri()
      end

      def prefetch_now()
      end

      def versions(name)
      end

      def uri()
      end

      def source()
      end
    end

    class SpecSpecification < ::Gem::Resolver::Specification
      def dependencies()
      end

      def full_name()
      end

      def name()
      end

      def version()
      end

      def platform()
      end
    end

    class GitSet < ::Gem::Resolver::Set
      def root_dir()
      end

      def need_submodules()
      end

      def repositories()
      end

      def find_all(req)
      end

      def root_dir=(_)
      end

      def prefetch(reqs)
      end

      def add_git_spec(name, version, repository, reference, submodules)
      end

      def pretty_print(q)
      end

      def specs()
      end

      def add_git_gem(name, repository, reference, submodules)
      end
    end

    class VendorSet < ::Gem::Resolver::Set
      def pretty_print(q)
      end

      def add_vendor_gem(name, directory)
      end

      def specs()
      end

      def find_all(req)
      end

      def load_spec(name, version, platform, source)
      end
    end

    class SourceSet < ::Gem::Resolver::Set
      def prefetch(reqs)
      end

      def find_all(req)
      end

      def add_source_gem(name, source)
      end
    end

    class VendorSpecification < ::Gem::Resolver::SpecSpecification
      def install(options = _)
      end

      def ==(other)
      end
    end

    class GitSpecification < ::Gem::Resolver::SpecSpecification
      def ==(other)
      end

      def add_dependency(dependency)
      end

      def install(options = _)
      end

      def pretty_print(q)
      end
    end

    class RequirementList
      include(Enumerable)

      def next5()
      end

      def remove()
      end

      def size()
      end

      def empty?()
      end

      def each()
      end

      def add(req)
      end
    end

    class BestSet < ::Gem::Resolver::ComposedSet
      def pretty_print(q)
      end

      def prefetch(reqs)
      end

      def find_all(req)
      end

      def pick_sets()
      end

      def replace_failed_api_set(error)
      end
    end

    def development_shallow=(_)
    end

    def development()
    end

    def ignore_dependencies=(_)
    end

    def skip_gems()
    end

    def skip_gems=(_)
    end

    def development_shallow()
    end

    def ignore_dependencies()
    end

    def missing()
    end

    def explain_list(stage)
    end

    def soft_missing()
    end

    def soft_missing=(_)
    end

    def activation_request(dep, possible)
    end

    def output()
    end

    def debug?()
    end

    def requests(s, act, reqs = _)
    end

    def find_possible(dependency)
    end

    def select_local_platforms(specs)
    end

    def search_for(dependency)
    end

    def dependencies_for(specification)
    end

    def requirement_satisfied_by?(requirement, activated, spec)
    end

    def name_for(dependency)
    end

    def allow_missing?(dependency)
    end

    def resolve()
    end

    def explain(stage, *data)
    end

    def stats()
    end

    def sort_dependencies(dependencies, activated, conflicts)
    end

    def development=(_)
    end
  end

  REPOSITORY_SUBDIRECTORIES = T.let(T.unsafe(nil), Array)

  module Util
    NULL_DEVICE = T.let(T.unsafe(nil), String)
  end

  GEM_DEP_FILES = T.let(T.unsafe(nil), Array)

  REPOSITORY_DEFAULT_GEM_SUBDIRECTORIES = T.let(T.unsafe(nil), Array)

  READ_BINARY_ERRORS = T.let(T.unsafe(nil), Array)

  DEFAULT_HOST = T.let(T.unsafe(nil), String)

  LOADED_SPECS_MUTEX = T.let(T.unsafe(nil), Thread::Mutex)

  class LoadError < ::LoadError
    def name=(_)
    end

    def requirement()
    end

    def name()
    end

    def requirement=(_)
    end
  end

  class DependencyList
    include(TSort)
    include(Enumerable)

    def development()
    end

    def remove_by_name(full_name)
    end

    def remove_specs_unsatisfied_by(dependencies)
    end

    def spec_predecessors()
    end

    def specs()
    end

    def clear()
    end

    def tsort_each_node(&block)
    end

    def inspect()
    end

    def add(*gemspecs)
    end

    def tsort_each_child(node)
    end

    def dependency_order()
    end

    def ok?()
    end

    def find_name(full_name)
    end

    def why_not_ok?(quick = _)
    end

    def each(&block)
    end

    def ok_to_remove?(full_name, check_dev = _)
    end

    def development=(_)
    end
  end
end

module Gem::Text
  def clean_text(text)
  end

  def truncate_text(text, description, max_length = _)
  end

  def min3(a, b, c)
  end

  def format_text(text, wrap, indent = _)
  end

  def levenshtein_distance(str1, str2)
  end
end

class Gem::RequestSet::GemDependencyAPI
  WINDOWS = T.let(T.unsafe(nil), Hash)

  VERSION_MAP = T.let(T.unsafe(nil), Hash)

  ENGINE_MAP = T.let(T.unsafe(nil), Hash)

  PLATFORM_MAP = T.let(T.unsafe(nil), Hash)

  def git_set()
  end

  def git_source(name, &callback)
  end

  def load()
  end

  def platforms(*platforms)
  end

  def find_gemspec(name, path)
  end

  def vendor_set()
  end

  def gem_git_reference(options)
  end

  def ruby(version, options = _)
  end

  def git(repository)
  end

  def without_groups()
  end

  def gem_deps_file()
  end

  def gemspec(options = _)
  end

  def group(*groups)
  end

  def gem(name, *requirements)
  end

  def source(url)
  end

  def dependencies()
  end

  def platform(*platforms)
  end

  def installing=(installing)
  end

  def without_groups=(_)
  end

  def requires()
  end
end

module Gem::Ext
  class ExtConfBuilder < ::Gem::Ext::Builder
    FileEntry = FileUtils::Entry_
  end

  class BuildError < ::Gem::InstallError

  end

  class ConfigureBuilder < ::Gem::Ext::Builder

  end

  class RakeBuilder < ::Gem::Ext::Builder

  end

  class Builder
    include(Gem::UserInteraction)
    include(Gem::DefaultUserInteraction)

    CHDIR_MUTEX = T.let(T.unsafe(nil), Thread::Mutex)

    def builder_for(extension)
    end

    def build_args()
    end

    def build_error(build_dir, output, backtrace = _)
    end

    def build_args=(_)
    end

    def build_extensions()
    end

    def write_gem_make_out(output)
    end

    def build_extension(extension, dest_path)
    end
  end

  class CmakeBuilder < ::Gem::Ext::Builder

  end
end
