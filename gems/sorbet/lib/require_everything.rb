# frozen_string_literal: true
# typed: true

require_relative './gem_loader'

class Sorbet::Private::RequireEverything
  class ExitCalledError < RuntimeError
  end

  # Goes through the most common ways to require all your userland code
  def self.require_everything
    patch_kernel
    is_rails = load_rails
    load_bundler # this comes second since some rails projects fail `Bundler.require' before rails is loaded
    require_all_files if !is_rails
  end

  def self.load_rails
    return unless File.exist?('config/application.rb')
    begin
      require 'rails'
    rescue
      return false
    end
    require './config/application'
    rails = T.unsafe(Rails)
    rails.application.require_environment!
    rails.application.eager_load!
    true
  end

  def self.load_bundler
    return unless File.exist?('Gemfile')
    begin
      require 'bundler'
    rescue
      return
    end
    Sorbet::Private::GemLoader.require_all_gems
  end

  def self.require_all_files
    abs_paths = Dir.glob("#{Dir.pwd}/**/*.rb")
    errors = []
    abs_paths.each_with_index do |abs_path, i|
      # Executable files are likely not meant to be required.
      # Some things we're trying to prevent against:
      # - misbehaving require-time side effects (removing files, reading from stdin, etc.)
      # - extra long runtime (making network requests, running a benchmark)
      # While this isn't a perfect heuristic for these things, it's pretty good.
      next if File.executable?(abs_path)

      begin
        my_require(abs_path, i+1, abs_paths.size)
      rescue LoadError, NoMethodError, SyntaxError
        next
      rescue
        errors << abs_path
        next
      end
    end
    # one more chance for order dependent things
    errors.each_with_index do |abs_path, i|
      begin
        my_require(abs_path, i+1, errors.size)
      rescue
        nil
      end
    end
    puts
  end

  def self.my_require(path, numerator, denominator)
    print "\r[#{numerator}/#{denominator}] require_relative #{path}"
    require_relative path
  end

  def self.patch_kernel
    Kernel.send(:define_method, :exit) do |*|
      puts 'Kernel#exit was called while requiring ruby source files'
      raise ExitCalledError.new
    end

    Kernel.send(:define_method, :at_exit) do |&block|
      if File.split($PROGRAM_NAME).last == 'rake'
        # Let `rake test` work
        super
        return proc {}
      end
      puts "Ignoring at_exit: #{block}"
      proc {}
    end
  end
end
