#!/usr/bin/env ruby
# frozen_string_literal: true

require_relative './step_interface'

require 'fileutils'

class Sorbet; end
module Sorbet::Private; end
class Sorbet::Private::CreateConfig
  SORBET_DIR = 'sorbet'
  SORBET_CONFIG_FILE = "#{SORBET_DIR}/config"

  include Sorbet::Private::StepInterface

  def self.main
    FileUtils.mkdir_p(SORBET_DIR)

    if File.file?(SORBET_CONFIG_FILE)
      puts "Reusing existing config file: #{SORBET_CONFIG_FILE}"
      return
    end

    File.open(SORBET_CONFIG_FILE, 'w') do |f|
      f.puts('--dir')
      f.puts('.')
      f.puts('--ignore=/tmp/')
      f.puts('--ignore=/vendor/bundle')
    end
  end

  def self.output_file
    SORBET_CONFIG_FILE
  end
end

if $PROGRAM_NAME == __FILE__
  Sorbet::Private::CreateConfig.main
end
