#!/usr/bin/env ruby
# typed: false

require_relative './step_interface'
require_relative './t'

require 'bundler'
require 'fileutils'
require 'set'

class Sorbet; end
module Sorbet::Private; end
class Sorbet::Private::FindGemRBIs
  XDG_CACHE_HOME = ENV['XDG_CACHE_HOME'] || "#{ENV['HOME']}/.cache"
  RBI_CACHE_DIR = "#{XDG_CACHE_HOME}/sorbet/"
  GEM_DIR = 'rbi'

  HEADER = Sorbet::Private::Serialize.header(false, 'find-gem-rbis')

  include Sorbet::Private::StepInterface

  # List of rbi folders in the gem's source
  T::Sig::WithoutRuntime.sig {params(gemspec: T.untyped).returns(T.nilable(String))}
  def self.paths_within_gem_sources(gemspec)
    gem_rbi = "#{gemspec.full_gem_path}/#{GEM_DIR}"
    gem_rbi if Dir.exist?(gem_rbi)
  end

  T::Sig::WithoutRuntime.sig {void}
  def self.main
    gemspecs = Bundler.load.specs.sort_by(&:name)

    gem_source_paths = T.let([], T::Array[String])
    gemspecs.each do |gemspec|
      gem_source_paths += paths_within_gem_sources(gemspec)
    end

    puts gem_source_paths.compact.join("\n")
  end

  def self.output_file
    RBI_CACHE_DIR + Digest::MD5.hexdigest(File.read('Gemfile.lock'))
  end
end

if $PROGRAM_NAME == __FILE__
  Sorbet::Private::FetchRBIs.main
end
