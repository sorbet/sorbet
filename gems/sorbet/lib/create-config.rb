#!/usr/bin/env ruby
# frozen_string_literal: true

require 'fileutils'

class Sorbet; end
module Sorbet::Private; end
class Sorbet::Private::CreateConfig
  SORBET_DIR = 'sorbet'
  SORBET_CONFIG_FILE = "#{SORBET_DIR}/config"
  def self.main
    puts "Creating #{SORBET_CONFIG_FILE}..."

    FileUtils.mkdir_p(SORBET_DIR)

    File.open(SORBET_CONFIG_FILE, 'w') do |f|
      f.puts('.')
    end
  end
end

if $PROGRAM_NAME == __FILE__
  Sorbet::Private::CreateConfig.main
end
