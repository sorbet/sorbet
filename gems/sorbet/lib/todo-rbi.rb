#!/usr/bin/env ruby
# frozen_string_literal: true

require_relative './serialize'

require 'json'

class SorbetRBIGeneration::SymbolEntry
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

class SorbetRBIGeneration::TodoRBI
  OUTPUT = 'sorbet/rbi/todo.rbi'
  HEADER = SorbetRBIGeneration::Serialize.header('strong', 'todo')

  def self.find_all(symbol_map, symbols, parents=[])
    symbols.each do |s|
      name = s['name']['name']
      superclass_id = s['superClass']
      parent_name = (name == '<root>' || name == "T.untyped") ? [] : [name]
      symbol_map[s['id']] = SorbetRBIGeneration::SymbolEntry.new(name, superclass_id, parents)
      if s['children']
        find_all(symbol_map, s['children'], symbol_map[s['id']].parents + parent_name)
        symbol_map[s['id']].has_children = true
      end
    end
  end

  def self.main
    File.delete(OUTPUT) if File.exist?(OUTPUT)

    IO.popen(
      [
        'srb',
        'tc',
        '--print=symbol-table-json',
        '--stdout-hup-hack',
        '--silence-dev-message',
        '--no-error-count',
      ],
      err: '/dev/null',
    ) do |io|
      table = JSON.parse(io.read)
      symbol_by_id = {}
      find_all(symbol_by_id, [table])

      output = String.new
      output << HEADER
      symbol_by_id.each do |_, s|
        superclass = symbol_by_id[s.superclass_id]
        next if !(superclass && superclass.name == 'StubClass')
        next if s.final_name.include?("<")

        if s.has_children
          output << "module #{s.final_name}; end\n"
        else
          output << "#{s.final_name} = T.let(#{s.final_name}, T.untyped)\n"
        end
      end
      File.write(OUTPUT, output)
    end
  end
end

if $PROGRAM_NAME == __FILE__
  SorbetRBIGeneration::TodoRBI.main
end
