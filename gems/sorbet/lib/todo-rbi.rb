#!/usr/bin/env ruby
# frozen_string_literal: true

require 'json'

require_relative './serialize'
require_relative './step_interface'

class Sorbet::Private::TodoRBI
  OUTPUT = 'sorbet/rbi/todo.rbi'
  HEADER = Sorbet::Private::Serialize.header('strong', 'todo')

  include Sorbet::Private::StepInterface

  def self.main
    File.delete(OUTPUT) if File.exist?(OUTPUT)

    IO.popen(
      [
        File.realpath("#{__dir__}/../bin/srb"),
        'tc',
        '--print=missing-constants',
        '--stdout-hup-hack',
        '--silence-dev-message',
        '--no-error-count',
      ],
      err: '/dev/null',
    ) do |io|
      missing_constants = io.read.split("\n")

      output = String.new
      output << HEADER
      missing_constants.each do |const|
        next if const.include?("<") || const.include?("class_of")
        output << "module #{const.gsub('T.untyped::', '')}; end\n"
      end
      File.write(OUTPUT, output) if output != HEADER
    end
  end

  def self.output_file
    OUTPUT
  end
end

if $PROGRAM_NAME == __FILE__
  Sorbet::Private::TodoRBI.main
end
