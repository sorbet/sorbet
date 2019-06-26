#!/usr/bin/env ruby
# frozen_string_literal: true

require_relative './step_interface'
require_relative './user_input'
require 'fileutils'

class Sorbet; end
module Sorbet::Private; end
class Sorbet::Private::CreateConfig
  SORBET_DIR = 'sorbet'
  SORBET_CONFIG_FILE = "#{SORBET_DIR}/config"

  include Sorbet::Private::StepInterface

  def self.main
    FileUtils.mkdir_p(SORBET_DIR)

    create_file =
      !File.exist?(SORBET_CONFIG_FILE) ||
        Sorbet::Private::UserInput.prompt_for_confirmation!("#{SORBET_CONFIG_FILE} already exists. Overwrite?")

    if create_file
      File.open(SORBET_CONFIG_FILE, 'w') do |f|
        f.puts('--dir')
        f.puts('.')
      end
    end
  end

  def self.output_file
    SORBET_CONFIG_FILE
  end
end

if $PROGRAM_NAME == __FILE__
  Sorbet::Private::CreateConfig.main
end
