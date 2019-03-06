class SorbetRBIGeneration::RequireEverything
  # Goes through the most common ways to require all your userland code
  def self.require_everything
    load_rails
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
    Rails.application.eager_load!
  end


  def self.require_all_files
    files = Dir.glob("**/*.rb")
    files.each do |file|
      begin
        require_relative "#{Dir.pwd}/#{file}"
      rescue LoadError, NoMethodError
        next
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
