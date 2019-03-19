# frozen_string_literal: true
# typed: true

require_relative './gem_loader'

class SorbetRBIGeneration::RequireEverything
  # Goes through the most common ways to require all your userland code
  def self.require_everything
    load_rails
    load_bundler # this comes second since some rails projects fail `Bundler.require' before rails is loaded
    require_all_files
  end

  def self.load_rails
    return unless File.exist?('config/application.rb')
    begin
      require 'rails'
    rescue
      return
    end
    require './config/application'
    Rails.application.require_environment!
    Rails.application.eager_load!
  end

  def self.load_bundler
    return unless File.exists?('Gemfile')
    begin
      require 'bundler'
    rescue
      return
    end
    SorbetRBIGeneration::GemLoader.require_all_gems
  end

  def self.require_all_files
    files = Dir.glob("**/*.rb")
    errors = []
    files.each do |file|
      begin
        watching_thread = Thread.new do
          sleep 5
          puts "Requiring #{file} has taken more than 5 seconds."
        end
        require_relative "#{Dir.pwd}/#{file}"
      rescue LoadError, NoMethodError, SyntaxError
        next
      rescue
        errors << file
        next
      ensure
        watching_thread.kill
      end
    end
    # one more chance for order dependent things
    errors.each do |file|
      begin
        require_relative "#{Dir.pwd}/#{file}"
      rescue
      end
    end
  end
end

def at_exit(*args, &block)
  if File.split($0).last == 'rake'
    # Let `rake test` work
    super
    return
  end
  puts "Ignoring at_exit: #{args} #{block}"
end
