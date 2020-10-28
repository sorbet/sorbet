#!/usr/bin/env ruby
# typed: false

require_relative './step_interface'
require_relative './t'

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

  SORBET_TYPED_REPO = ENV['SRB_SORBET_TYPED_REPO'] || 'https://github.com/sorbet/sorbet-typed.git'
  SORBET_TYPED_REVISION = ENV['SRB_SORBET_TYPED_REVISION'] || 'origin/master'

  HEADER = Sorbet::Private::Serialize.header(false, 'sorbet-typed')

  include Sorbet::Private::StepInterface

  # Ensure our cache is up-to-date
  T::Sig::WithoutRuntime.sig {void}
  def self.fetch_sorbet_typed
    if File.directory?(RBI_CACHE_DIR)
      cached_remote = IO.popen(["git", "-C", RBI_CACHE_DIR, "config", "--get", "remote.origin.url"]) {|pipe| pipe.read}.strip

      # Compare the <owner>/<repo>.git to be agnostic of https vs ssh urls
      cached_remote_repo = cached_remote.split(%r{github.com[:/]}).last
      requested_remote_repo = SORBET_TYPED_REPO.split(%r{github.com[:/]}).last

      if cached_remote_repo != requested_remote_repo
        raise "Cached remote #{cached_remote_repo} does not match requested remote #{requested_remote_repo}. Delete #{RBI_CACHE_DIR} and try again."
      end
    else
      IO.popen(["git", "clone", SORBET_TYPED_REPO, RBI_CACHE_DIR]) {|pipe| pipe.read}
      raise "Failed to git pull" if $?.exitstatus != 0
    end

    FileUtils.cd(RBI_CACHE_DIR) do
      IO.popen(%w{git fetch --all}) {|pipe| pipe.read}
      raise "Failed to git fetch" if $?.exitstatus != 0
      IO.popen(%w{git checkout -q} + [SORBET_TYPED_REVISION]) {|pipe| pipe.read}
      raise "Failed to git checkout" if $?.exitstatus != 0
    end
  end

  # List of directories whose names satisfy the given Gem::Version (+ 'all/')
  T::Sig::WithoutRuntime.sig do
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
  T::Sig::WithoutRuntime.sig {params(ruby_version: Gem::Version).returns(T::Array[String])}
  def self.paths_for_ruby_version(ruby_version)
    ruby_dir = "#{RBI_CACHE_DIR}/lib/ruby"
    matching_version_directories(ruby_dir, ruby_version)
  end

  # List of directories in lib/gemspec.name whose names satisfy gemspec.version
  T::Sig::WithoutRuntime.sig {params(gemspec: T.untyped).returns(T::Array[String])}
  def self.paths_for_gem_version(gemspec)
    local_dir = "#{RBI_CACHE_DIR}/lib/#{gemspec.name}"
    matching_version_directories(local_dir, gemspec.version)
  end

  # Copy the relevant RBIs into their repo, with matching folder structure.
  T::Sig::WithoutRuntime.sig {params(vendor_paths: T::Array[String]).void}
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

  T::Sig::WithoutRuntime.sig {void}
  def self.main
    fetch_sorbet_typed

    gemspecs = Bundler.load.specs.sort_by(&:name)

    vendor_paths = T.let([], T::Array[String])
    vendor_paths += paths_for_ruby_version(Gem::Version.create(RUBY_VERSION))
    gemspecs.each do |gemspec|
      vendor_paths += paths_for_gem_version(gemspec)
    end

    # Remove the sorbet-typed directory before repopulating it.
    FileUtils.rm_r(SORBET_RBI_SORBET_TYPED) if Dir.exist?(SORBET_RBI_SORBET_TYPED)
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
