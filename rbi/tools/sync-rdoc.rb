#!/usr/bin/env ruby
require 'rdoc'

class RBIFile
  attr_reader :path

  def initialize(path)
    @path = path
  end

  private def io
    @io ||= File.open(path, "r+:UTF-8")
  end

  def content
    @content ||= io.read
  end

  def lines
    @lines ||= content.lines
  end

  def write_back!
    io.seek(0)
    io.truncate(0)
    io.write(lines.join)
    io.flush
  end
end

class ScopeStack
  class DeclState
    attr_reader :name
    attr_accessor :last_sig

    def initialize(name)
      @name = name
    end
  end

  def stack
    @stack ||= [DeclState.new(nil)] # for toplevel declarations
  end

  def with(*ids, &blk)
    states = ids.map {|id| DeclState.new(id)}
    stack.concat(states)
    yield
    stack.pop(states.length)
  end

  def last_sig
    stack.last.last_sig
  end

  def last_sig=(node)
    stack.last.last_sig = node
  end

  def current_namespace
    stack.map(&:name).compact.join("::")
  end

  def defer!(sig_node)
    self.last_sig = sig_node if last_sig.nil? # take the first sig in overloads
  end

  def consume!
    consumed = last_sig
    self.last_sig = nil
    consumed
  end
end

class DocParser
  attr_reader :file

  def initialize(file)
    @file = file
  end

  private def root
    @root ||= RubyVM::AbstractSyntaxTree.parse(file.content)
  end

  private def error!(node, msg)
    raise "#{file.path}:#{node.first_lineno}:#{node.first_column}: #{msg}"
  end

  private def unexpected!(node)
    error! node, "unexpected node of type #{node.type}"
  end

  private def scope_stack
    @scope_stack ||= ScopeStack.new
  end

  private def colon_to_a(node)
    case node.type
    when :COLON2 # Foo::Bar
      lhs, name = node.children
      [*(lhs && colon_to_a(lhs)), name]
    when :COLON3, # ::T
         :CONST # T
      name, = node.children
      [name]
    else
      unexpected!(node)
    end
  end

  private def is_sig?(node)
    case node.type
    when :FCALL
      name, _args = node.children
      name == :sig
    else
      unexpected!(node)
    end
  end

  private def assert_clean!
    error!(scope_stack.last_sig, "unconsumed sig") unless scope_stack.last_sig.nil?
  end

  private def walk_scope(node, &blk)
    case node.type
    when :MODULE, :CLASS
      path, *, scope = node.children
      assert_clean!
      scope_stack.with(*colon_to_a(path)) do
        yield scope_stack.current_namespace, node, node
        walk_scope(scope, &blk)
        assert_clean!
      end
    when :SCOPE, :BLOCK, :BEGIN
      assert_clean!
      node.children
        .select {|child| child.is_a?(RubyVM::AbstractSyntaxTree::Node)}
        .each {|child| walk_scope(child, &blk)}
    when :CDECL
      names = if node.children.length == 2
        name, _rhs = node.children
        [name]
      else
        # for some reason, name is duplicated as the second child when it's already in path
        path, _name, _rhs = node.children
        [*colon_to_a(path)]
      end
      names.unshift(scope_stack.current_namespace) unless scope_stack.current_namespace.empty?
      namespace = names.join("::")
      yield namespace, node, node

    when :ITER # sig {...}
      iter, _scope = node.children
      scope_stack.defer!(node) if is_sig?(iter)
    when :DEFN # def foo
      name, _scope = node.children
      namespace = scope_stack.current_namespace
      namespace = namespace.empty? ? "Object" : namespace
      yield "#{namespace}\##{name}", node, scope_stack.consume! || node
    when :DEFS # def self.foo
      receiver, name, scope = node.children
      error!(node, "expected self") unless receiver.type == :SELF
      namespace = scope_stack.current_namespace
      namespace = namespace.empty? ? "Object" : namespace
      yield "#{namespace}.#{name}", node, scope_stack.consume! || node

    when :FCALL, # extend Foo
         :VCALL # private
      assert_clean!
    else
      unexpected!(node)
    end
  end

  def each_def(&blk)
    walk_scope(root, &blk)
  end

  private def find_comment_before(node)
    start_line = end_line = node.first_lineno - 1 # lineno is 1-indexed
    (start_line - 1).downto(0) do |i| # line-by-line, upwards from the def
      line = file.lines[i]
      if line =~ /\A\s*\z/ # blank line; don't include but keep going
        # keep going
      elsif line =~ /\A\s*\#(?!\s*typed:)/ # comment (except sigil); include in range
        start_line = i
      else
        break
      end
    end

    first_line_of_sig_or_node = file.lines[end_line]
    indentation = /\A\s*/.match(first_line_of_sig_or_node)[0]
    [(start_line...end_line), indentation]
  end

  def each_doc(&blk)
    each_def do |path, def_node, node|
      doc_range, indentation = find_comment_before(node)
      yield path, def_node, doc_range, indentation
    end
  end
end

class SyncRDoc
  # some classes are not where they say they are
  RENAMES = {
    "Thread::ConditionVariable" => "ConditionVariable",
    "Thread::Mutex" => "Mutex",
    "Thread::Queue" => "Queue",
    "Thread::SizedQueue" => "SizedQueue",
  }

  private def driver
    @driver ||= RDoc::RI::Driver.new
  end

  private def store
    @store ||= driver.stores.find {|s| s.source == 'ruby'}
  end

  private def apply_renames(namespace)
    RENAMES.fetch(namespace, namespace)
  end

  private def find_module(name)
    begin
      store.load_class(apply_renames(name))
    rescue RDoc::Store::Error
      nil
    end
  end

  private def find_class_method(namespace, name)
    begin
      store.load_method(apply_renames(namespace), "::#{name}")
    rescue RDoc::Store::Error
      nil
    end
  end

  private def find_instance_method(namespace, name)
    begin
      store.load_method(apply_renames(namespace), "\##{name}")
    rescue RDoc::Store::Error
      nil
    end
  end

  private def find_constant(namespace, name)
    find_module(namespace)&.constants_hash&.[](name)
  end

  private def render_comment(doc, indentation)
    formatter = RDoc::Markup::ToMarkdown.new
    doc.comment.accept(formatter)
    s = formatter.res.join
    s.lines.map {|line| "#{indentation}\# #{line}".rstrip + "\n"}
  end

  def process_file!(file)
    to_replace = []
    DocParser.new(file).each_doc do |path, def_node, doc_range, indentation|
      next if path =~ /\A(?:Sorbet|T)(?:\z|::|\.|\#)/
      namespace, separator, name = path.rpartition(/::|\.|\#/)
      doc = case separator
      when ""
        # toplevel constants could be documented as a class or constant, so try both (e.g. ENV)
        find_module(path) || find_constant("Object", name)
      when "::"
        # classes could be documented as constants and vice versa, so try both
        find_module(path) || find_constant(namespace, name)
      when "."
        find_class_method(namespace, name)
      when "#"
        find_instance_method(namespace, name)
      else
        raise "impossible"
      end

      if doc
        to_replace.push([doc_range, render_comment(doc, indentation)])
      elsif doc_range.size > 0
        Warning.warn("#{file.path}:#{def_node.first_lineno}:#{def_node.first_column}: #{path} has existing doc but can't find it with ri; not clobbering\n")
      end
    end

    to_replace.sort_by! do |replacement|
      doc_range, _comment = replacement
      doc_range.begin
    end
    to_replace.reverse_each do |doc_range, comment| # mutate in reverse to avoid keeping track of shifted ranges
      file.lines[doc_range] = comment
    end
    file.write_back!
  end

  def run!(argv)
    if argv.empty?
      puts "Usage: #{$0} <rbi_path>..."
      return 1
    end

    argv.each do |dir|
      Dir.glob(File.join('**', '*.rbi'), base: dir) do |path|
        process_file!(RBIFile.new(File.join(dir, path)))
      end
    end
    0
  end

  def self.run!
    exit(SyncRDoc.new.run!(ARGV))
  end
end

if __FILE__ == $0
  SyncRDoc.run!
end
