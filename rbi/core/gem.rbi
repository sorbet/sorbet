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

  Sorbet.sig(
      name: String,
      args: String,
      requirements: Gem::Requirement,
  )
  .returns(String)
  def self.bin_path(name, args=T.unsafe(nil), *requirements); end

  Sorbet.sig.returns(String)
  def self.binary_mode(); end

  Sorbet.sig(
      install_dir: String,
  )
  .returns(String)
  def self.bindir(install_dir=T.unsafe(nil)); end

  Sorbet.sig.returns(Hash)
  def self.clear_default_specs(); end

  Sorbet.sig.returns(NilClass)
  def self.clear_paths(); end

  Sorbet.sig.returns(String)
  def self.config_file(); end

  Sorbet.sig.returns(T.untyped)
  def self.configuration(); end

  Sorbet.sig(
      config: BasicObject,
  )
  .returns(T.untyped)
  def self.configuration=(config); end

  Sorbet.sig(
      gem_name: String,
  )
  .returns(T.nilable(String))
  def self.datadir(gem_name); end

  Sorbet.sig.returns(T.nilable(String))
  def self.default_bindir(); end

  Sorbet.sig.returns(T.nilable(String))
  def self.default_cert_path(); end

  Sorbet.sig.returns(T.nilable(String))
  def self.default_dir(); end

  Sorbet.sig.returns(T.nilable(String))
  def self.default_exec_format(); end

  Sorbet.sig.returns(T.nilable(String))
  def self.default_key_path(); end

  Sorbet.sig.returns(T.nilable(String))
  def self.default_path(); end

  Sorbet.sig.returns(T.nilable(T::Array[String]))
  def self.default_rubygems_dirs(); end

  Sorbet.sig.returns(T.nilable(T::Array[String]))
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
