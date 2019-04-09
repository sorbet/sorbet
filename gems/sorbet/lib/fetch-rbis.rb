#!/usr/bin/env ruby
# typed: false

require_relative './step_interface'
require_relative './sorbet'

require 'bundler'
require 'fileutils'
require 'set'

class Sorbet; end
module Sorbet::Private; end
class Sorbet::Private::FetchRBIs
  SORBET_DIR = 'sorbet'
  SORBET_CONFIG_FILE = "#{SORBET_DIR}/config"
  SORBET_RBI_LIST = "#{SORBET_DIR}/rbi_list"
  SORBET_RBI_SORBET_TYPED = "#{SORBET_DIR}/rbi/sorbet-typed/"

  XDG_CACHE_HOME = ENV['XDG_CACHE_HOME'] || "#{ENV['HOME']}/.cache"
  RBI_CACHE_DIR = "#{XDG_CACHE_HOME}/sorbet/sorbet-typed"

  SORBET_TYPED_REPO = 'git@github.com:sorbet/sorbet-typed.git'

  HEADER = Sorbet::Private::Serialize.header(false, 'sorbet-typed')

  include Sorbet::Private::StepInterface

  # Ensure our cache is up-to-date
  Sorbet.sig {void}
  def self.fetch_sorbet_typed
    if File.directory?(RBI_CACHE_DIR)
      FileUtils.cd(RBI_CACHE_DIR) do
        IO.popen(%w{git pull}) {|pipe| pipe.read}
        raise "Failed to git pull" if $?.exitstatus != 0
      end
    else
      IO.popen(["git", "clone", SORBET_TYPED_REPO, RBI_CACHE_DIR]) {|pipe| pipe.read}
      raise "Failed to git pull" if $?.exitstatus != 0
    end
  end

  # List of directories whose names satisfy the given Gem::Version (+ 'all/')
  Sorbet.sig do
    params(
      root: String,
      version: Gem::Version,
    )
    .returns(T::Array[String])
  end
  def self.matching_version_directories(root, version)
    paths = Dir.glob("#{root}/*/").select do |dir|
      basename = File.basename(dir.chomp('/'))
      requirements = basename.split(/[,&-]/) # split using ',', '-', or '&'
      requirements.all? do |requirement|
        Gem::Requirement::PATTERN =~ requirement &&
          Gem::Requirement.create(requirement).satisfied_by?(version)
      end
    end
    paths = paths.map {|dir| dir.chomp('/')}
    all_dir = "#{root}/all"
    paths << all_dir if Dir.exist?(all_dir)
    paths
  end

  # List of directories in lib/ruby whose names satisfy the current RUBY_VERSION
  Sorbet.sig {params(ruby_version: Gem::Version).returns(T::Array[String])}
  def self.paths_for_ruby_version(ruby_version)
    ruby_dir = "#{RBI_CACHE_DIR}/lib/ruby"
    matching_version_directories(ruby_dir, ruby_version)
  end

  # List of rbi folders in the gem's source
  Sorbet.sig {params(gemspec: T.untyped).returns(T::Array[String])}
  def self.paths_within_gem_sources(gemspec)
    paths = T.let([], T::Array[String])
    %w[rbi rbis].each do |dir|
      gem_rbi = "#{gemspec.full_gem_path}/#{dir}"
      paths << gem_rbi if Dir.exist?(gem_rbi)
    end
    paths
  end

  # List of directories in lib/gemspec.name whose names satisfy gemspec.version
  Sorbet.sig {params(gemspec: T.untyped).returns(T::Array[String])}
  def self.paths_for_gem_version(gemspec)
    local_dir = "#{RBI_CACHE_DIR}/lib/#{gemspec.name}"
    matching_version_directories(local_dir, gemspec.version)
  end

  # Make the config file that has a list of every non-vendored RBI
  # (we don't vendor these so that (1) people don't have to check them in (2) people aren't likely to patch them)
  Sorbet.sig {params(gem_source_paths: T::Array[String]).void}
  def self.serialize_rbi_list(gem_source_paths)
    File.open(SORBET_RBI_LIST, 'w') do |rbi_list|
      rbi_list.puts(gem_source_paths)
    end

    File.open(SORBET_CONFIG_FILE, 'r+') do |config|
      if config.lines.all? {|line| !line.match(/^@#{SORBET_RBI_LIST}$/)}
        config.puts("@#{SORBET_RBI_LIST}")
      end
    end
  end

  # Copy the relevant RBIs into their repo, with matching folder structure.
  Sorbet.sig {params(vendor_paths: T::Array[String]).void}
  def self.vendor_rbis_within_paths(vendor_paths)
    vendor_paths.each do |vendor_path|
      relative_vendor_path = vendor_path.sub(RBI_CACHE_DIR, '')

      dest = "#{SORBET_RBI_SORBET_TYPED}/#{relative_vendor_path}"
      FileUtils.mkdir_p(dest)

      Dir.glob("#{vendor_path}/*.rbi").each do |rbi|
        extra_header = "#
# If you would like to make changes to this file, great! Please upstream any changes you make here:
#
#   https://github.com/sorbet/sorbet-typed/edit/master#{relative_vendor_path}/#{File.basename(rbi)}
#
"
        File.write("#{dest}/#{File.basename(rbi)}", HEADER + extra_header + File.read(rbi))
      end
    end
  end

  Sorbet.sig {void}
  def self.main
    fetch_sorbet_typed

    gemspecs = Bundler.load.specs.sort_by(&:name)

    gem_source_paths = T.let([], T::Array[String])
    gemspecs.each do |gemspec|
      gem_source_paths += paths_within_gem_sources(gemspec)
    end

    vendor_paths = T.let([], T::Array[String])
    vendor_paths += paths_for_ruby_version(Gem::Version.create(RUBY_VERSION))
    gemspecs.each do |gemspec|
      vendor_paths += paths_for_gem_version(gemspec)
    end

    if gem_source_paths.length > 0
      serialize_rbi_list(gem_source_paths)
    end

    if vendor_paths.length > 0
      vendor_rbis_within_paths(vendor_paths)
    end
  end

  def self.output_file
    SORBET_RBI_SORBET_TYPED
  end
end

if $PROGRAM_NAME == __FILE__
  Sorbet::Private::FetchRBIs.main
end
