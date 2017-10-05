#!/usr/bin/env ruby

require 'parser/current'
Parser::Builders::Default.emit_procarg0 = true
Parser::Builders::Default.emit_lambda = true

def escape_rb(str)
  str.each_char.map do |c|
    case c
    when "\t"
      '\t'
    when "\r"
      '\r'
    when "\n"
      '\n'
    when "\\", "'", '"'
      "\\#{c}"
    when "\x20" .. "\x7e"
      c
    else
      "\\u{#{c.ord.to_s(16)}}"
    end
  end.join
end

def astprint(obj)
  case obj
  when String
    "\"#{escape_rb(obj)}\""
  when Rational
    obj.to_f.inspect
  else
    obj.inspect
  end
end

class AST::Node
  def to_sexp(indent=0)
    indented = "  " * indent
    sexp = "#{indented}(#{fancy_type}"

    first_node_child = children.index do |child|
      child.is_a?(AST::Node) || child.is_a?(Array)
    end || children.count

    children.each_with_index do |child, idx|
      if child.is_a?(AST::Node) && idx >= first_node_child
        sexp += "\n#{child.to_sexp(indent + 1)}"
      else
        sexp += " #{astprint(child)}"
      end
    end

    sexp += ")"
    sexp
  end
end

def parse_ruby(code, filename)
  parser = Parser::CurrentRuby.default_parser
  parser.builder.emit_file_line_as_literals = false

  buffer = Parser::Source::Buffer.new(filename, 1)
  buffer.source = code

  parser.parse(buffer)
end

if File.basename(__FILE__) == File.basename($0)
  filename = ARGV.first
  abort("usage: parse.rb [filename]") unless filename
  $stdout.print parse_ruby(File.read(filename), filename).to_s.strip
end
