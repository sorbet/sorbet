#!/usr/bin/env ruby
require 'rdoc'
require 'optparse'
require 'bundler/inline'

gemfile do
  source 'https://rubygems.org'
  gem 'commonmarker'
  gem 'nokogiri'
  gem 'diff-lcs', require: 'diff/lcs'
  gem 'paint'
  gem 'pry'
end

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

  private def warn(node, msg)
    Warning.warn("#{file.path}:#{node.first_lineno}:#{node.first_column}: #{msg}\n")
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
    when :SCLASS
      # There are a handful of places in RBIs where we have to use `class <<
      # self` because both a class and it's singleton class need to register a
      # type_member/type_template with the name Elem
      puts 'skipping :SCLASS node, documentation will not be generated'
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

    when :VCALL # private
      assert_clean!
    when :FCALL # extend Foo, module_function def foo; end, private def foo; end
      name, args = node.children
      if ![:module_function, :private].include?(name) ||
          args.type != :LIST ||
          args.children.length == 0 ||
          ![:DEFS, :DEFN].include?(args.children[0].type)
        assert_clean!
        return
      end

      walk_scope(args.children[0], &blk)
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
      if line =~ /\A\s*(\z|###)/
        # blank line or internal comment; don't include but keep going
      elsif line =~ /\A\s*\#(?!\s*typed:)/ # comment (except sigil); include in range
        start_line = i
      else
        break
      end
    end
    (end_line - 1).downto(start_line) do |i|
      line = file.lines[i]
      if line =~ /\A\s*###/
        end_line = i
      else
        break
      end
    end

    first_line_of_sig_or_node = file.lines[end_line]
    indentation = /\A\s*/.match(first_line_of_sig_or_node)[0]
    [(start_line...end_line), indentation]
  end

  private def find_comment_between(a, b)
    range = (a.last_lineno + 1...b.first_lineno)
    range.size == 0 ? [] : file.lines[range].filter {|l| l =~ /\A\s*#(?!##)/}
  end

  def each_doc(&blk)
    each_def do |path, def_node, node|
      warn(def_node, "found split comment") unless find_comment_between(node, def_node).empty?
      doc_range, indentation = find_comment_before(node)
      yield path, def_node, doc_range, indentation
    end
  end
end

# Fix generation of nested markup <code>foo<em>bar</em>baz</code>
module FormatterTagPatch
  TagRange = Struct.new(:tag, :begin, :end)

  def convert_flow(flow)
    tag_ranges = []
    tag_on_pos = {}
    flow.each_with_index do |item, i|
      next unless item.is_a?(RDoc::Markup::AttrChanger)
      @attr_tags
        .select {|t| item.turn_off & t.bit != 0}
        .each {|t| tag_ranges.push(TagRange.new(t, tag_on_pos.delete(t), i))}
      @attr_tags
        .select {|t| item.turn_on & t.bit != 0}
        .each {|t| tag_on_pos[t] = i}
    end

    tag_order = @attr_tags.each_with_index.to_h
    by_off = tag_ranges.group_by(&:end)
    by_on = tag_ranges.group_by(&:begin)
    new_flow = flow.each_with_index.flat_map do |item, i|
      case item
      when RDoc::Markup::AttrChanger
        off = by_off.fetch(i, [])
          .sort_by {|r| [-r.begin, -tag_order[r.tag]]}
          .map {|r| RDoc::Markup::AttrChanger.new(0, r.tag.bit)}
        on = by_on.fetch(i, [])
          .sort_by {|r| [-r.end, tag_order[r.tag]]}
          .map {|r| RDoc::Markup::AttrChanger.new(r.tag.bit, 0)}
        [*off, *on]
      else
        [item]
      end
    end

    super(new_flow)
  end
end

class ToMarkdownRef < RDoc::Markup::ToMarkdown
  include FormatterTagPatch

  def initialize(options, base_url, from_path, context, markup=nil)
    super(markup)
    @base_url = base_url
    @html_crossref = RDoc::Markup::ToHtmlCrossref.new(options, from_path, context, markup)
    @markup.add_regexp_handling(RDoc::Markup::ToHtmlCrossref::CROSSREF_REGEXP, :REF)
  end

  def convert_html_link(text)
    # sorry i'm parsing html with regex
    match = /<a href="([^"]*)">(.*)<\/a>/.match(text)
    if match
      href, text = match.captures
      href = @base_url + href
      text = text.gsub(/<\/?code>/, '`')
      "[#{text}](#{href})"
    else
      convert_string(text)
    end
  end

  def handle_regexp_REF(target)
    convert_html_link(@html_crossref.handle_regexp_CROSSREF(target))
  end

  def gen_url(url, text)
    if url.start_with?('rdoc-ref:')
      convert_html_link(@html_crossref.gen_url(url, text))
    else
      super
    end
  end

  def accept_verbatim(verbatim)
    @res.push("\n") unless @res.last(2).join =~ /\n\n\z/
    open = (verbatim.ruby? || @html_crossref.parseable?(verbatim.text.strip)) ? "```ruby\n" : "```\n"
    @res << open
    lines = verbatim.parts.join.lines
      .reverse_each
      .drop_while {|line| line.strip.empty?} # remove trailing blank lines
      .reverse
    lines.push("\n") unless lines.last.end_with?("\n")
    @res.push(*lines)
    @res << "```\n\n"
  end

  def convert_string(text)
    if !in_tt?
      text = text
        .gsub(/(?<=\A|\s|\()``/, '"')
        .gsub(/(?<!\s)''(?=\s|\z|\.|,|\))/, '"')
        .gsub(/`([\w\s]+)'/, "'\\1'")
        .gsub(/([\\\*_])/, '\\\\\\1') # fix special markdown chars likely to cause problems
        .gsub(/\.  /, '. ') # fix double-space after periods
    end
    super(text)
  end

  def annotate(tag)
    in_tt? ? '' : super
  end

  def wrap(text)
    i = 0
    line_re = /\G(.{,#{width - indent}}|[^ ]+)(?: +|\z)/
    indentation = " " * indent
    while i < text.length
      match = line_re.match(text, i)
      raise "impossible" unless match
      @res.push(indentation) if i > 0 || !use_prefix
      @res << match[1] << "\n"
      i = match.end(0)
    end
  end

  def link_for_method(method)
    convert_html_link(@html_crossref.link(method.pretty_name, method.name))
  end

  def add_alias_info(method)
    unless method.aliases.empty?
      @res << "\n"
      aliases = method.aliases.map {|a| link_for_method(a)}.join(", ")
      wrap("Also aliased as: #{aliases}")
    end

    alias_for = method.is_alias_for
    if alias_for
      @res << "\n"
      wrap("Alias for: #{link_for_method(alias_for)}")
    end
  end
end

module DocDiff
  def self.render_lines(lines)
    md = lines.map {|l| l.gsub(/\A\s*# /, '')}.join
    html = CommonMarker.render_html(md, :DEFAULT)
    root = Nokogiri::HTML.fragment(html)
    root.text
      .tr("“”‘’", %q{""''})
      .gsub(/—/, "---")
      .split
      .flat_map {|s| s.scan(/\G.+?\b/)}
  end

  def self.diff(old, new)
    Diff::LCS::sdiff(render_lines(old), render_lines(new))
  end

  def self.render_diff(diff)
    parts = []
    diff_chunks = diff.chunk {|change| change.action}.to_a
    diff_chunks.each_with_index do |action_chunk, i|
      action, chunk = action_chunk
      case action
      when '+'
        parts << Paint[chunk.map(&:new_element).join(" "), :green]
      when '-'
        parts << Paint[chunk.map(&:old_element).join(" "), :red, :inverse, :bold]
      when '!'
        parts << Paint[chunk.map(&:old_element).join(" "), :red, :inverse, :bold]
        parts << Paint[chunk.map(&:new_element).join(" "), :green]
      when '='
        context_start = 0
        context_end = chunk.length
        word_context = 2
        context_start += word_context unless i == 0
        context_end -= word_context unless i == diff_chunks.length - 1
        omit_range = (context_start...context_end)
        if omit_range.size == 0
          parts << Paint[chunk.map(&:old_element).join(" "), :faint]
        else
          start_range = (0...omit_range.begin)
          end_range = (omit_range.end...)
          if start_range.size > 0
            parts << Paint[chunk[start_range].map(&:old_element).join(" "), :faint]
          end
          parts << Paint["...", :faint]
          if end_range.size > 0
            parts << Paint[chunk[end_range].map(&:old_element).join(" "), :faint]
          end
        end
      else
        raise "impossible: #{action}"
      end
    end
    parts.join(" ")
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

  SKIP = [
    /\A(?:Sorbet|T)(?:\z|::|\.|\#)/,
    # rdoc is dumb sometimes
    "Kernel#require",
  ]

  ONLY_IN_FILE = {
    "Kernel" => "rbi/core/kernel.rbi"
  }

  private def driver
    @driver ||= RDoc::RI::Driver.new
  end

  private def store
    return @store if defined?(@store)

    @store = if defined?(@store_path)
      RDoc::Store.new(@store_path)
    else
      driver.stores.find {|s| s.source == 'ruby'}
    end
    rdoc.store = @store
    @store
  end

  private def options
    @options ||= begin
      opts = RDoc::Options.new
      opts.parse(['--all'])
      opts.setup_generator("darkfish")
      opts
    end
  end

  private def rdoc
    return @rdoc if defined?(@rdoc)
    @rdoc = RDoc::RDoc.new
    @rdoc.options = options
    @rdoc.generator = options.generator.new(store, options)
    @rdoc
  end

  private def apply_renames(namespace)
    RENAMES.fetch(namespace, namespace)
  end

  private def find_module(name)
    store.find_class_or_module(apply_renames(name))
  end

  private def find_class_method(namespace, name)
    mod = find_module(namespace)
    mod&.find_method(name, true) || mod&.find_attribute(name, true)
  end

  private def find_instance_method(namespace, name)
    mod = find_module(namespace)
    mod&.find_method(name, false) || mod&.find_attribute(name, false)
  end

  private def find_constant(namespace, name)
    find_module(namespace)&.find_constant_named(name)
  end

  private def render_comment(code_obj, indentation)
    context = code_obj.is_a?(RDoc::Context) ? code_obj : code_obj.parent

    formatter = ToMarkdownRef.new(options, "https://docs.ruby-lang.org/en/2.7.0/", "/", context)
    formatter.width -= indentation.gsub("\t", '  ').length # account for indentation (assuming tabstop is 2)
    code_obj.comment.accept(formatter)
    formatter.add_alias_info(code_obj) if code_obj.is_a?(RDoc::MethodAttr)
    formatter.res.join.lines
      .reverse_each
      .drop_while {|line| line.strip.empty?} # remove trailing blank lines
      .reverse
      .drop_while {|line| line.strip.empty?} # remove leading blank lines
      .map {|line| "#{indentation}\# #{line}".rstrip + "\n"}
  end

  def process_file!(file)
    to_replace = []
    puts file.path
    DocParser.new(file).each_doc do |path, def_node, doc_range, indentation|
      next if SKIP.any? {|s| s.is_a?(String) ? path == s : path =~ s}
      next unless ONLY_IN_FILE[path] == file.path
      namespace, separator, name = path.rpartition(/::|\.|\#/)
      code_obj = case separator
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

      rendered_lines = code_obj && render_comment(code_obj, indentation)
      if rendered_lines && !rendered_lines.empty?
        to_replace.push([doc_range, rendered_lines])
      elsif doc_range.size > 0
        Warning.warn("#{file.path}:#{def_node.first_lineno}:#{def_node.first_column}: #{path} has existing doc but can't find it with ri; not clobbering\n")
      end
    end

    if @diff
      to_replace.each do |doc_range, comment|
        next if doc_range.size == 0
        orig_comment = file.lines[doc_range]
        diff = DocDiff.diff(orig_comment, comment)
        has_changes = diff.any? {|change| change.action != '='}
        next unless has_changes

        print "#{file.path}:#{doc_range.begin}-#{doc_range.end}"
        puts DocDiff.render_diff(diff)
      end
    end

    to_replace
      .sort_by {|doc_range, _| doc_range.begin}
      .reverse_each do |doc_range, comment| # mutate in reverse to avoid keeping track of shifted ranges
        file.lines[doc_range] = comment
      end
    file.write_back! unless @dry_run
  end

  def run!(argv)
    @dry_run = false
    @diff = false

    parser = OptionParser.new do |opts|
      opts.banner = "Usage: #{opts.program_name} [options] <rbi_path>..."

      opts.on("--[no-]dry-run", "Don't write any files") do |v|
        @dry_run = v
      end

      opts.on("--[no-]diff", "Print changes for changed docs") do |d|
        @diff = d
      end

      opts.on("--rdoc-store=PATH", "Path to the RDoc store to get the documentation from") do |p|
        @store_path = p
      end
    end
    parser.parse!
    if argv.empty?
      puts parser
      return -1
    end

    if Gem::Version.new(RUBY_VERSION) < Gem::Version.new('2.7')
      puts "Warning! Detected RUBY_VERSION=#{RUBY_VERSION}."
      puts "This script uses the current Ruby version to parse documentation from."
      puts "If you're seeing a large diff, it might be because all existing docstrings were generated using Ruby 2.7."
    end

    store.load_all # ensure cross-referencing can find everything
    store.complete(:private)

    argv.each do |dir|
      if File.exist?(dir) && File.extname(dir) == '.rbi'
        process_file!(RBIFile.new(dir))
        next
      end

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
