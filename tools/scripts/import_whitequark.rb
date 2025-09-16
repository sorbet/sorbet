# This script imports test cases from whitequark/parser repo
# and saves them into test/whitequark
# import_whitequark.sh executes it with all necessary env variables.
#
# Each assertion from whitequark/parser/test/test_parser.rb
# produces two files
#   - test_<TEST_NAME>_<N>.rb
#   - test_<TEST_NAME>_<N>.parse-tree-whitequark.exp
#
# Where:
#   - TEST_NAME is the name of the test (i.e. the name of the minitest method)
#   - N is the number of assertion in this method
#
# Here we run a single test_parser.rb file with a patch that
# instead of running an assertion records expected/actual values
# (see ParseHelperPatch module)
#
# Also, there's a small difference between whitequark/parser and sorbet/parser
# that is handled by the "Rewriter" class which is a simple recursive visitor

def print_usage
  puts 'format: TARGET_RUBY_VERSION="2.x" IMPORT_FROM=<parser dir> IMPORT_TO=<target dir> ruby import_whitequark.rb'
  exit 1
end

PARSER_DIR = (path = ENV['IMPORT_FROM'] || print_usage) && File.expand_path(path)
TARGET_DIR = (path = ENV['IMPORT_TO']   || print_usage) && File.expand_path(path)
TARGET_RUBY_VERSION = ENV['TARGET_RUBY_VERSION'] || print_usage

puts "Importing test cases from #{PARSER_DIR} to #{TARGET_DIR}"

$LOAD_PATH << File.join(PARSER_DIR, 'lib')
$LOAD_PATH << File.join(PARSER_DIR, 'test')

ENV['BUNDLE_GEMFILE'] = File.join(PARSER_DIR, 'Gemfile')
require 'bundler/setup'
require 'helper'
require 'parse_helper'

puts ARGV.inspect

TESTS = Hash.new { |hash, test_name| hash[test_name] = [] }

module Parser
  module Meta
    ORIGINAL_NODE_TYPES = remove_const(:NODE_TYPES).dup
    NODE_TYPES = ORIGINAL_NODE_TYPES

    NODE_TYPES << :assign
  end
end

class Parser::Builders::Default
  def assign(lhs, eql_t, rhs)
    loc = lhs.loc.with_operator(loc(eql_t)).with_expression(join_exprs(lhs, rhs))

    case lhs.type
    when :send, :csend
      (lhs << rhs).updated(nil, nil, location: loc)
    else
      n(:assign, [lhs, rhs], loc)
    end
  end

  # Turn off "modern" mode for now
  %i[
    emit_lambda
    emit_procarg0
    emit_index
    emit_arg_inside_procarg0
  ].each do |method_name|
    define_singleton_method(method_name) { false }
  end

  def initialize
    @emit_file_line_as_literals = false
  end
end

class Rewriter < Parser::AST::Processor
  # For numeric literals Sorbet emits their source, not the value
  def replace_to_original_source(node)
    node.updated(nil, [node.location.expression.source.delete('r').delete('i')])
  end

  alias on_int replace_to_original_source
  alias on_float replace_to_original_source
  alias on_rational replace_to_original_source
  alias on_complex replace_to_original_source

  # Sorbet emits flags as a string s(:regopt, "mix")
  def on_regopt(node)
    node.updated(nil, [node.children.join])
  end

  # Sorbet doesn't emit implicit empty s(:params)
  def on_def(node)
    node = super
    name, args, body_node = *node
    args = nil if !args.nil? && args.children.empty? && args.loc.expression.nil?
    node.updated(nil, [name, args, body_node])
  end

  # Sorbet doesn't emit implicit empty s(:params)
  def on_defs(node)
    node = super
    definee_node, name, args, body_node = *node
    args = nil if !args.nil? && args.children.empty? && args.loc.expression.nil?
    node.updated(nil, [definee_node, name, args, body_node])
  end

  # Sorbet doesn't emit implicit empty s(:params)
  def on_block(node)
    node = super
    send, args, body = *node
    args = nil if !args.nil? && args.children.empty? && args.loc.expression.nil?
    node.updated(nil, [send, args, body])
  end

  alias on_assign process_regular_node

  # Sorbet always gives a names to the restarg ('*' if there's no name)
  def on_restarg(node)
    node = super
    name, _ = *node
    node.updated(nil, [name || :'*'])
  end

  # Sorbet always gives a names to the restarg ('*' if there's no name)
  def on_kwrestarg(node)
    node = super
    name, _ = *node
    node.updated(nil, [name || :'**'])
  end

  # Sorbet always gives a name to the splat (nil if there's no name)
  def on_splat(name)
    node = super
    name, _ = *node
    node.updated(nil, [name])
  end

  private

  def s(type, *children)
    Parser::AST::Node.new(type, children)
  end
end

REWRITER = Rewriter.new

module ParseHelperPatch
  def assert_parses(ast, code, source_maps='', versions=ParseHelper::ALL_VERSIONS)
    if versions.include?(TARGET_RUBY_VERSION)
      parsed_ast = nil

      with_versions([TARGET_RUBY_VERSION]) do |version, parser|
        source_file = Parser::Source::Buffer.new('(assert_parses)')
        source_file.source = code

        begin
          parsed_ast = parser.parse(source_file)
        rescue => exc
          backtrace = exc.backtrace
          Exception.instance_method(:initialize).bind(exc).
            call("(#{version}) #{exc.message}")
          exc.set_backtrace(backtrace)
          raise
        end

        if parsed_ast
          parsed_ast = REWRITER.process(parsed_ast)
        end
      end

      TESTS[name] << { input: code, output: parsed_ast.inspect }
    end
  end

  def assert_diagnoses(diagnostic, code, source_maps='', versions=ParseHelper::ALL_VERSIONS)
    if versions.include?(TARGET_RUBY_VERSION)
      with_versions([TARGET_RUBY_VERSION]) do |version, parser|
        level, reason, arguments = diagnostic
        arguments ||= {}
        message     = Parser::MESSAGES[reason] % arguments

        if code.split("\n").length > 1
          # importing multi-line errors is complicated
          # all of them are related:
          # 1. multiline block comments =begin/=end
          # 2. heredocs
          next
        end

        if level == :error
          input = "#{code} # error: #{message}"

          TESTS[name] << { input: input }
        end
      end
    end
  end
end

ParseHelper.prepend(ParseHelperPatch)

class Minitest::Test
  IGNORE = [
    # we are not interested in legacy behavior,
    # sorbet always emits __ENCODING__ as s(:__ENCODING__)
    'test___ENCODING___legacy_',

    # heredoc values are incorrect, but we can parse them into strings
    'test_slash_newline_in_heredocs',
    'test_parser_slash_slash_n_escaping_in_literals',
    'test_dedenting_heredoc',
    'test_heredoc',
     # escaping issues, string values are incorrect
    'test_ruby_bug_11990',
    'test_regexp_encoding',
    'test_parser_bug_198',
    'test_ruby_bug_11989',

    # sorbet doesn't declare lvars from "string" =~ //
    'test_lvar_injecting_match',

    # whitequark/parser returns nil, sorbet returns s(:begin)
    'test_empty_stmt',

    # This one is tricky:
    # whitequark/parser relies on source maps in the AST builder,
    # and Sorbet simply doesn't enough information to mirror this behavior.
    #
    # "topstmt" rule must create a Loc with begin=null,end=null,expr=range
    #
    # technically AST is correct, but it has redundant :begin nodes
    'test_cond_begin_masgn',

    # Sorbet ignores invalid chars in strings and regexps
    'test_bug_ascii_8bit_in_literal',
    'test_regex_error',

    # Sorbet emits multiple errors, but whitequark/parser stops on the first one
    'test_meth_ref__before_27'
  ]

  def after_teardown
    TESTS.each do |test_name, cases|
      next if IGNORE.include?(test_name)

      cases.each_with_index do |capture, idx|
        full_test_name = "#{test_name}_#{idx}".gsub(/_{2,}/, '_')
        puts "Creating input/output files for #{full_test_name}"

        input_filepath = File.join(TARGET_DIR, "#{full_test_name}.rb")
        output_filepath = File.join(TARGET_DIR, "#{full_test_name}.parse-tree-whitequark.exp")

        if [input_filepath, output_filepath].any? { |path| path.include?(' ') }
          raise "bug: #{full_test_name.inspect} contains space"
        end

        File.write(input_filepath, "# typed: true\n\n" + capture[:input] + "\n")

        if capture[:output]
          File.write(output_filepath, capture[:output] + "\n")
        end
      end
    end
  end
end

require 'test_parser'
