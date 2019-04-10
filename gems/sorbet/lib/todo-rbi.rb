#!/usr/bin/env ruby
# frozen_string_literal: true

require 'json'

require_relative './serialize'
require_relative './step_interface'

class Sorbet::Private::SymbolEntry
  attr_reader :name, :superclass_id, :parents
  attr_accessor :has_children

  def initialize(name, superclass_id, parents=[])
    @name = name
    @superclass_id = superclass_id
    @parents = parents
    @has_children = false
  end

  def final_name
    @final_name ||= begin
      parent_name = parents.join('::')
      entry_name = name
      if parent_name.empty?
        entry_name
      else
        "#{parent_name}::#{entry_name}"
      end
    end
  end
end

class Sorbet::Private::TodoRBI
  OUTPUT = 'sorbet/rbi/todo.rbi'
  HEADER = Sorbet::Private::Serialize.header('strong', 'todo')

  include Sorbet::Private::StepInterface

  def self.main
    File.delete(OUTPUT) if File.exist?(OUTPUT)

    IO.popen(
      [
        'srb',
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
        output << "module #{const}; end\n"
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
