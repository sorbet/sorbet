# typed: true
module Gem
  DEFAULT_HOST = T.let(T.unsafe(nil), String)
  GEM_DEP_FILES = T.let(T.unsafe(nil), Array)
  GEM_PRELUDE_SUCKAGE = T.let(T.unsafe(nil), NilClass)
  LOADED_SPECS_MUTEX = T.let(T.unsafe(nil), Thread::Mutex)
  MARSHAL_SPEC_DIR = T.let(T.unsafe(nil), String)
  READ_BINARY_ERRORS = T.let(T.unsafe(nil), Array)
  REPOSITORY_DEFAULT_GEM_SUBDIRECTORIES = T.let(T.unsafe(nil), Array)
  REPOSITORY_SUBDIRECTORIES = T.let(T.unsafe(nil), Array)
  RUBYGEMS_DIR = T.let(T.unsafe(nil), String)
  VERSION = T.let(T.unsafe(nil), String)
  WIN_PATTERNS = T.let(T.unsafe(nil), Array)
  WRITE_BINARY_ERRORS = T.let(T.unsafe(nil), Array)

  sig do
    params(
        name: String,
        args: String,
        requirements: Gem::Requirement,
    )
    .returns(String)
  end
  def self.bin_path(name, args=T.unsafe(nil), *requirements); end

  sig {returns(String)}
  def self.binary_mode(); end

  sig do
    params(
        install_dir: String,
    )
    .returns(String)
  end
  def self.bindir(install_dir=T.unsafe(nil)); end

  sig {returns(Hash)}
  def self.clear_default_specs(); end

  sig {returns(NilClass)}
  def self.clear_paths(); end

  sig {returns(String)}
  def self.config_file(); end

  sig {returns(T.untyped)}
  def self.configuration(); end

  sig do
    params(
        config: BasicObject,
    )
    .returns(T.untyped)
  end
  def self.configuration=(config); end

  sig do
    params(
        gem_name: String,
    )
    .returns(T.nilable(String))
  end
  def self.datadir(gem_name); end

  sig {returns(T.nilable(String))}
  def self.default_bindir(); end

  sig {returns(T.nilable(String))}
  def self.default_cert_path(); end

  sig {returns(T.nilable(String))}
  def self.default_dir(); end

  sig {returns(T.nilable(String))}
  def self.default_exec_format(); end

  sig {returns(T.nilable(String))}
  def self.default_key_path(); end

  sig {returns(T.nilable(String))}
  def self.default_path(); end

  sig {returns(T.nilable(T::Array[String]))}
  def self.default_rubygems_dirs(); end

  sig {returns(T.nilable(T::Array[String]))}
  def self.default_sources(); end
end

class Gem::Dependency < Object
  TYPES = T.let(T.unsafe(nil), Array)
end

class Gem::Platform < Object
  CURRENT = T.let(T.unsafe(nil), String)
  RUBY = T.let(T.unsafe(nil), String)
end

class Gem::Requirement < Object
  OPS = T.let(T.unsafe(nil), Hash)
  PATTERN = T.let(T.unsafe(nil), Regexp)
  PATTERN_RAW = T.let(T.unsafe(nil), String)
  SOURCE_SET_REQUIREMENT = T.let(T.unsafe(nil), Object)
end

class Gem::Specification < Gem::BasicSpecification
  CURRENT_SPECIFICATION_VERSION = T.let(T.unsafe(nil), Integer)
  EMPTY = T.let(T.unsafe(nil), Array)
  MARSHAL_FIELDS = T.let(T.unsafe(nil), Hash)
  NONEXISTENT_SPECIFICATION_VERSION = T.let(T.unsafe(nil), Integer)
  NOT_FOUND = T.let(T.unsafe(nil), Object)
  SPECIFICATION_VERSION_HISTORY = T.let(T.unsafe(nil), Hash)
  TODAY = T.let(T.unsafe(nil), Time)
  VALID_NAME_PATTERN = T.let(T.unsafe(nil), Regexp)
end

class Gem::StubSpecification < Gem::BasicSpecification
  OPEN_MODE = T.let(T.unsafe(nil), String)
  PREFIX = T.let(T.unsafe(nil), String)
end

class Gem::StubSpecification::StubLine < Object
  NO_EXTENSIONS = T.let(T.unsafe(nil), Array)
  REQUIRE_PATHS = T.let(T.unsafe(nil), Hash)
  REQUIRE_PATH_LIST = T.let(T.unsafe(nil), Hash)
end

class Gem::Version < Object
  include Comparable

  ANCHORED_VERSION_PATTERN = T.let(T.unsafe(nil), Regexp)
  VERSION_PATTERN = T.let(T.unsafe(nil), String)
end
