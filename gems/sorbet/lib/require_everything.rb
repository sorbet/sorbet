# frozen_string_literal: true
# typed: true

require 'pathname'

require_relative './gem_loader'
require_relative './status'

class ExitCalledError < RuntimeError
end

class Sorbet::Private::RequireEverything
  # Goes through the most common ways to require all your userland code
  def self.require_everything
    return if @already_ran
    @already_ran = true
    patch_kernel
    load_rails
    load_bundler # this comes second since some rails projects fail `Bundler.require' before rails is loaded
    require_all_files
  end

  def self.load_rails
    return unless rails?
    require './config/application'
    rails = Object.const_get(:Rails)
    rails.application.require_environment!
    rails.application.eager_load!
    true
  end

  def self.load_bundler
    return unless File.exist?('Gemfile')
    begin
      require 'bundler'
    rescue LoadError
      return
    end
    Sorbet::Private::GemLoader.require_all_gems
  end

  def self.require_all_files
    excluded_paths = Set.new
    excluded_paths += excluded_rails_files if rails?

    abs_paths = rb_file_paths
    errors = []
    abs_paths.each_with_index do |abs_path, i|
      # Executable files are likely not meant to be required.
      # Some things we're trying to prevent against:
      # - misbehaving require-time side effects (removing files, reading from stdin, etc.)
      # - extra long runtime (making network requests, running a benchmark)
      # While this isn't a perfect heuristic for these things, it's pretty good.
      next if File.executable?(abs_path)
      next if excluded_paths.include?(abs_path)

      # Skip db/schema.rb, as requiring it can wipe the database. This is left
      # out of exclude_rails_files, as it is possible to use the packages that
      # generate it without using the whole rails ecosystem.
      next if /db\/schema.rb$/.match(abs_path)

      # Skip **/extconf.rb, as running it will emit build configuration artifacts
      next if /\/extconf.rb$/.match(abs_path)

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
      end
    end

    Sorbet::Private::Status.done
  end

  def self.my_require(abs_path, numerator, denominator)
    rel_path = Pathname.new(abs_path).relative_path_from(Pathname.new(Dir.pwd)).to_s
    Sorbet::Private::Status.say("[#{numerator}/#{denominator}] require_relative './#{rel_path}'")
    require_relative abs_path
  end

  def self.patch_kernel
    Kernel.send(:define_method, :exit) do |*|
      puts 'Kernel#exit was called while requiring ruby source files'
      raise ExitCalledError.new
    end

    Kernel.send(:define_method, :at_exit) do |&block|
      if File.split($0).last == 'rake'
        # Let `rake test` work
        super
        return proc {}
      end
      # puts "Ignoring at_exit: #{block}"
      proc {}
    end
  end

  private

  def self.rb_file_paths
    srb = File.realpath("#{__dir__}/../bin/srb")
    output = IO.popen([
      srb,
      "tc",
      "-p",
      "file-table-json",
      "--stop-after=parser",
      "--silence-dev-message",
      "--no-error-count",
      "-e",
      "''",
    ]) {|io| io.read}
    # This returns a hash with structure:
    # { files:
    #   [
    #     {
    #       "strict": ["Ignore"|"False"|"True"|"Strict"|"Strong"|"Stdlib"],
    #       "path": "./path/to/file",
    #       ...
    #     }
    #     ...
    #   ]
    # }
    parsed = JSON.parse(output)
    parsed
      .fetch('files', [])
      .reject{|file| ["Ignore", "Stdlib"].include?(file["strict"])}
      .map{|file| file["path"]}
      .select{|path| File.file?(path)} # Some files have https:// paths. We ignore those here.
      .select{|path| /.rb$/.match(path)}
      .map{|path| File.expand_path(path)} # Requires absolute path
  end

  def self.excluded_rails_files
    excluded_paths = Set.new

    # Exclude files that have already been loaded by rails
    self.rails_load_paths.each do |path|
      excluded_paths += Dir.glob("#{path}/**/*.rb")
    end

    # Exclude initializers, as they have already been run by rails and
    # can contain side-effects like monkey-patching that should
    # only be run once.
    excluded_paths += Dir.glob("#{Dir.pwd}/config/initializers/**/*.rb")
  end

  def self.rails_load_paths
    rails = Object.const_get(:Rails)

    # As per changes made to change the arity of this method:
    # https://github.com/rails/rails/commit/b6e17b6a4b67ccc9fac5fe16741c3db720f00959
    # This sets the `add_autoload_paths_to_load_path` parameter to `true` which will
    # provide parity with older versions of Rails prior to the mentioned commit.
    if Gem::Version.new(rails.version) >= Gem::Version.new('6.0.0.rc2')
      rails.application.send(:_all_load_paths, true)
    else
      rails.application.send(:_all_load_paths)
    end
  end

  def self.rails?
    return false unless File.exist?('config/application.rb')
    begin
      require 'rails'
    rescue LoadError
      return false
    end
    true
  end
end
