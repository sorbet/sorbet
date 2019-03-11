#!/usr/bin/env ruby

require 'bundler'
require 'set'

module SorbetRBIGeneration; end
class SorbetRBIGeneration::FetchRBIs
  def self.main
    add_bundler_line
  end

  def self.add_bundler_line
    gemspecs = Bundler.load.specs
    gemspecs.each do |gemspec|
      return if gemspec.name == 'rbis'
    end

    puts "Adding `rbis` gem to Gemfile"
    File.open('Gemfile', 'a') {|f| f.write("gem 'rbis', git: 'git@github.com:coinbase/rbis.git'")}

    puts "Running `bundle install`"
    `bundle install`
  end
end

if $PROGRAM_NAME == __FILE__
  SorbetRBIGeneration::FetchRBIs.main
end
