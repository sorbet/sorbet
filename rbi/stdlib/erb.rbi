# typed: __STDLIB_INTERNAL

# # [`ERB`](https://docs.ruby-lang.org/en/2.6.0/ERB.html) -- Ruby Templating
#
# ## Introduction
#
# [`ERB`](https://docs.ruby-lang.org/en/2.6.0/ERB.html) provides an easy to use
# but powerful templating system for Ruby. Using
# [`ERB`](https://docs.ruby-lang.org/en/2.6.0/ERB.html), actual Ruby code can be
# added to any plain text document for the purposes of generating document
# information details and/or flow control.
#
# A very simple example is this:
#
# ```ruby
# require 'erb'
#
# x = 42
# template = ERB.new <<-EOF
#   The value of x is: <%= x %>
# EOF
# puts template.result(binding)
# ```
#
# *Prints:* The value of x is: 42
#
# More complex examples are given below.
#
# ## Recognized Tags
#
# [`ERB`](https://docs.ruby-lang.org/en/2.6.0/ERB.html) recognizes certain tags
# in the provided template and converts them based on the rules below:
#
# ```
# <% Ruby code -- inline with output %>
# <%= Ruby expression -- replace with result %>
# <%# comment -- ignored -- useful in testing %>
# % a line of Ruby code -- treated as <% line %> (optional -- see ERB.new)
# %% replaced with % if first thing on a line and % processing is used
# <%% or %%> -- replace with <% or %> respectively
# ```
#
# All other text is passed through
# [`ERB`](https://docs.ruby-lang.org/en/2.6.0/ERB.html) filtering unchanged.
#
# ## Options
#
# There are several settings you can change when you use ERB:
# *   the nature of the tags that are recognized;
# *   the value of `$SAFE` under which the template is run;
# *   the binding used to resolve local variables in the template.
#
#
# See the [`ERB.new`](https://docs.ruby-lang.org/en/2.6.0/ERB.html#method-c-new)
# and
# [`ERB#result`](https://docs.ruby-lang.org/en/2.6.0/ERB.html#method-i-result)
# methods for more detail.
#
# ## Character encodings
#
# [`ERB`](https://docs.ruby-lang.org/en/2.6.0/ERB.html) (or Ruby code generated
# by [`ERB`](https://docs.ruby-lang.org/en/2.6.0/ERB.html)) returns a string in
# the same character encoding as the input string. When the input string has a
# magic comment, however, it returns a string in the encoding specified by the
# magic comment.
#
# ```ruby
# # -*- coding: utf-8 -*-
# require 'erb'
#
# template = ERB.new <<EOF
# <%#-*- coding: Big5 -*-%>
#   \_\_ENCODING\_\_ is <%= \_\_ENCODING\_\_ %>.
# EOF
# puts template.result
# ```
#
# *Prints:* \_*ENCODING*_ is Big5.
#
# ## Examples
#
# ### Plain Text
#
# [`ERB`](https://docs.ruby-lang.org/en/2.6.0/ERB.html) is useful for any
# generic templating situation. Note that in this example, we use the convenient
# "% at start of line" tag, and we quote the template literally with `%q{...}`
# to avoid trouble with the backslash.
#
# ```ruby
# require "erb"
#
# # Create template.
# template = %q{
#   From:  James Edward Gray II <james@grayproductions.net>
#   To:  <%= to %>
#   Subject:  Addressing Needs
#
#   <%= to[/\w+/] %>:
#
#   Just wanted to send a quick note assuring that your needs are being
#   addressed.
#
#   I want you to know that my team will keep working on the issues,
#   especially:
#
#   <%# ignore numerous minor requests -- focus on priorities %>
#   % priorities.each do |priority|
#     * <%= priority %>
#   % end
#
#   Thanks for your patience.
#
#   James Edward Gray II
# }.gsub(/^  /, '')
#
# message = ERB.new(template, trim_mode: "%<>")
#
# # Set up template data.
# to = "Community Spokesman <spokesman@ruby_community.org>"
# priorities = [ "Run Ruby Quiz",
#                "Document Modules",
#                "Answer Questions on Ruby Talk" ]
#
# # Produce result.
# email = message.result
# puts email
# ```
#
# *Generates:*
#
# ```
# From:  James Edward Gray II <james@grayproductions.net>
# To:  Community Spokesman <spokesman@ruby_community.org>
# Subject:  Addressing Needs
#
# Community:
#
# Just wanted to send a quick note assuring that your needs are being addressed.
#
# I want you to know that my team will keep working on the issues, especially:
#
#     * Run Ruby Quiz
#     * Document Modules
#     * Answer Questions on Ruby Talk
#
# Thanks for your patience.
#
# James Edward Gray II
# ```
#
# ### Ruby in HTML
#
# [`ERB`](https://docs.ruby-lang.org/en/2.6.0/ERB.html) is often used in
# `.rhtml` files (HTML with embedded Ruby). Notice the need in this example to
# provide a special binding when the template is run, so that the instance
# variables in the Product object can be resolved.
#
# ```ruby
# require "erb"
#
# # Build template data class.
# class Product
#   def initialize( code, name, desc, cost )
#     @code = code
#     @name = name
#     @desc = desc
#     @cost = cost
#
#     @features = [ ]
#   end
#
#   def add_feature( feature )
#     @features << feature
#   end
#
#   # Support templating of member data.
#   def get_binding
#     binding
#   end
#
#   # ...
# end
#
# # Create template.
# template = %{
#   <html>
#     <head><title>Ruby Toys -- <%= @name %></title></head>
#     <body>
#
#       <h1><%= @name %> (<%= @code %>)</h1>
#       <p><%= @desc %></p>
#
#       <ul>
#         <% @features.each do |f| %>
#           <li><b><%= f %></b></li>
#         <% end %>
#       </ul>
#
#       <p>
#         <% if @cost < 10 %>
#           <b>Only <%= @cost %>!!!</b>
#         <% else %>
#            Call for a price, today!
#         <% end %>
#       </p>
#
#     </body>
#   </html>
# }.gsub(/^  /, '')
#
# rhtml = ERB.new(template)
#
# # Set up template data.
# toy = Product.new( "TZ-1002",
#                    "Rubysapien",
#                    "Geek's Best Friend!  Responds to Ruby commands...",
#                    999.95 )
# toy.add_feature("Listens for verbal commands in the Ruby language!")
# toy.add_feature("Ignores Perl, Java, and all C variants.")
# toy.add_feature("Karate-Chop Action!!!")
# toy.add_feature("Matz signature on left leg.")
# toy.add_feature("Gem studded eyes... Rubies, of course!")
#
# # Produce result.
# rhtml.run(toy.get_binding)
# ```
#
# *Generates (some blank lines removed):*
#
# ```
# <html>
#   <head><title>Ruby Toys -- Rubysapien</title></head>
#   <body>
#
#     <h1>Rubysapien (TZ-1002)</h1>
#     <p>Geek's Best Friend!  Responds to Ruby commands...</p>
#
#     <ul>
#         <li><b>Listens for verbal commands in the Ruby language!</b></li>
#         <li><b>Ignores Perl, Java, and all C variants.</b></li>
#         <li><b>Karate-Chop Action!!!</b></li>
#         <li><b>Matz signature on left leg.</b></li>
#         <li><b>Gem studded eyes... Rubies, of course!</b></li>
#     </ul>
#
#     <p>
#          Call for a price, today!
#     </p>
#
#   </body>
# </html>
# ```
#
# ## Notes
#
# There are a variety of templating solutions available in various Ruby
# projects:
# *   ERB's big brother, eRuby, works the same but is written in C for speed;
# *   Amrita (smart at producing HTML/XML);
# *   cs/Template (written in C for speed);
# *   [`RDoc`](https://docs.ruby-lang.org/en/2.6.0/RDoc.html), distributed with
#     Ruby, uses its own template engine, which can be reused elsewhere;
# *   and others; search [RubyGems.org](https://rubygems.org/) or [The Ruby
#     Toolbox](https://www.ruby-toolbox.com/).
#
#
# Rails, the web application framework, uses
# [`ERB`](https://docs.ruby-lang.org/en/2.6.0/ERB.html) to create views.
class ERB
  Revision = ::T.let(nil, ::T.untyped)

  # Define unnamed class which has *methodname* as instance method, and return
  # it.
  #
  # example:
  #
  # ```ruby
  # class MyClass_
  #   def initialize(arg1, arg2)
  #     @arg1 = arg1;  @arg2 = arg2
  #   end
  # end
  # filename = 'example.rhtml'  # @arg1 and @arg2 are used in example.rhtml
  # erb = ERB.new(File.read(filename))
  # erb.filename = filename
  # MyClass = erb.def_class(MyClass_, 'render()')
  # print MyClass.new('foo', 123).render()
  # ```
  sig do
    params(
      superklass: ::T.untyped,
      methodname: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def def_class(superklass=T.unsafe(nil), methodname=T.unsafe(nil)); end

  sig do
    params(
      mod: ::T.untyped,
      methodname: ::T.untyped,
      fname: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def def_method(mod, methodname, fname=T.unsafe(nil)); end

  sig do
    params(
      methodname: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def def_module(methodname=T.unsafe(nil)); end

  # The encoding to eval
  sig {returns(::T.untyped)}
  def encoding(); end

  # The optional *filename* argument passed to
  # [`Kernel#eval`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-eval)
  # when the [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html) code is run
  sig {returns(::T.untyped)}
  def filename(); end

  # The optional *filename* argument passed to
  # [`Kernel#eval`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-eval)
  # when the [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html) code is run
  sig do
    params(
      filename: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def filename=(filename); end

  sig do
    params(
      str: ::T.untyped,
      safe_level: ::T.untyped,
      trim_mode: ::T.untyped,
      eoutvar: ::T.untyped,
    )
    .void
  end
  def initialize(str, safe_level=T.unsafe(nil), trim_mode=T.unsafe(nil), eoutvar=T.unsafe(nil)); end

  # The optional *lineno* argument passed to
  # [`Kernel#eval`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-eval)
  # when the [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html) code is run
  sig {returns(::T.untyped)}
  def lineno(); end

  # The optional *lineno* argument passed to
  # [`Kernel#eval`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-eval)
  # when the [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html) code is run
  sig do
    params(
      lineno: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def lineno=(lineno); end

  # Sets optional filename and line number that will be used in
  # [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html) code evaluation and
  # error reporting. See also
  # [`filename=`](https://docs.ruby-lang.org/en/2.7.0/ERB.html#attribute-i-filename)
  # and
  # [`lineno=`](https://docs.ruby-lang.org/en/2.7.0/ERB.html#attribute-i-lineno)
  #
  # ```ruby
  # erb = ERB.new('<%= some_x %>')
  # erb.render
  # # undefined local variable or method `some_x'
  # #   from (erb):1
  #
  # erb.location = ['file.erb', 3]
  # # All subsequent error reporting would use new location
  # erb.render
  # # undefined local variable or method `some_x'
  # #   from file.erb:4
  # ```
  sig do
    params(
      location: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def location=(location); end

  # Creates a new compiler for
  # [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html). See ERB::Compiler.new
  # for details
  sig do
    params(
      trim_mode: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def make_compiler(trim_mode); end

  # Executes the generated [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html)
  # code to produce a completed template, returning the results of that code.
  # (See [`ERB::new`](https://docs.ruby-lang.org/en/2.7.0/ERB.html#method-c-new)
  # for details on how this process can be affected by *safe\_level*.)
  #
  # *b* accepts a [`Binding`](https://docs.ruby-lang.org/en/2.7.0/Binding.html)
  # object which is used to set the context of code evaluation.
  sig do
    params(
      b: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def result(b=T.unsafe(nil)); end

  # Render a template on a new toplevel binding with local variables specified
  # by a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) object.
  def result_with_hash(hash); end

  # Generate results and print them. (see
  # [`ERB#result`](https://docs.ruby-lang.org/en/2.7.0/ERB.html#method-i-result))
  sig do
    params(
      b: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def run(b=T.unsafe(nil)); end

  # Can be used to set *eoutvar* as described in
  # [`ERB::new`](https://docs.ruby-lang.org/en/2.7.0/ERB.html#method-c-new).
  # It's probably easier to just use the constructor though, since calling this
  # method requires the setup of an
  # [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html) *compiler* object.
  sig do
    params(
      compiler: ::T.untyped,
      eoutvar: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def set_eoutvar(compiler, eoutvar=T.unsafe(nil)); end

  # The Ruby code generated by
  # [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html)
  sig {returns(::T.untyped)}
  def src(); end

  # Returns revision information for the erb.rb module.
  sig {returns(::T.untyped)}
  def self.version(); end
end

class ERB::Compiler
  sig do
    params(
      out: ::T.untyped,
      content: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_insert_cmd(out, content); end

  sig do
    params(
      out: ::T.untyped,
      content: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_put_cmd(out, content); end

  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def compile(s); end

  sig do
    params(
      stag: ::T.untyped,
      out: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def compile_content(stag, out); end

  sig do
    params(
      etag: ::T.untyped,
      out: ::T.untyped,
      scanner: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def compile_etag(etag, out, scanner); end

  sig do
    params(
      stag: ::T.untyped,
      out: ::T.untyped,
      scanner: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def compile_stag(stag, out, scanner); end

  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def content_dump(s); end

  sig do
    params(
      trim_mode: ::T.untyped,
    )
    .void
  end
  def initialize(trim_mode); end

  sig {returns(::T.untyped)}
  def insert_cmd(); end

  sig do
    params(
      insert_cmd: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def insert_cmd=(insert_cmd); end

  sig do
    params(
      src: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def make_scanner(src); end

  sig {returns(::T.untyped)}
  def percent(); end

  sig {returns(::T.untyped)}
  def post_cmd(); end

  sig do
    params(
      post_cmd: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def post_cmd=(post_cmd); end

  sig {returns(::T.untyped)}
  def pre_cmd(); end

  sig do
    params(
      pre_cmd: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def pre_cmd=(pre_cmd); end

  sig do
    params(
      mode: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def prepare_trim_mode(mode); end

  sig {returns(::T.untyped)}
  def put_cmd(); end

  sig do
    params(
      put_cmd: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def put_cmd=(put_cmd); end

  sig {returns(::T.untyped)}
  def trim_mode(); end
end

class ERB::Compiler::Buffer
  sig {returns(::T.untyped)}
  def close(); end

  sig {returns(::T.untyped)}
  def cr(); end

  sig do
    params(
      compiler: ::T.untyped,
      enc: ::T.untyped,
      frozen: ::T.untyped,
    )
    .void
  end
  def initialize(compiler, enc=T.unsafe(nil), frozen=T.unsafe(nil)); end

  sig do
    params(
      cmd: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def push(cmd); end

  sig {returns(::T.untyped)}
  def script(); end
end

class ERB::Compiler::ExplicitScanner < ERB::Compiler::Scanner
  sig {returns(::T.untyped)}
  def scan(); end
end

class ERB::Compiler::PercentLine
  sig {returns(::T.untyped)}
  def empty?(); end

  sig do
    params(
      str: ::T.untyped,
    )
    .void
  end
  def initialize(str); end

  sig {returns(::T.untyped)}
  def to_s(); end

  sig {returns(::T.untyped)}
  def value(); end
end

class ERB::Compiler::Scanner
  sig {returns(::T.untyped)}
  def etags(); end

  sig do
    params(
      src: ::T.untyped,
      trim_mode: ::T.untyped,
      percent: ::T.untyped,
    )
    .void
  end
  def initialize(src, trim_mode, percent); end

  sig {returns(::T.untyped)}
  def scan(); end

  sig {returns(::T.untyped)}
  def stag(); end

  sig do
    params(
      stag: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def stag=(stag); end

  sig {returns(::T.untyped)}
  def stags(); end

  sig do
    params(
      klass: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.default_scanner=(klass); end

  sig do
    params(
      src: ::T.untyped,
      trim_mode: ::T.untyped,
      percent: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.make_scanner(src, trim_mode, percent); end

  sig do
    params(
      klass: ::T.untyped,
      trim_mode: ::T.untyped,
      percent: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.regist_scanner(klass, trim_mode, percent); end

  sig do
    params(
      klass: ::T.untyped,
      trim_mode: ::T.untyped,
      percent: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.register_scanner(klass, trim_mode, percent); end
end

class ERB::Compiler::SimpleScanner < ERB::Compiler::Scanner
  sig {returns(::T.untyped)}
  def scan(); end
end

class ERB::Compiler::TrimScanner < ERB::Compiler::Scanner
  ERB_STAG = ::T.let(nil, ::T.untyped)

  sig do
    params(
      line: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def explicit_trim_line(line); end

  sig do
    params(
      src: ::T.untyped,
      trim_mode: ::T.untyped,
      percent: ::T.untyped,
    )
    .void
  end
  def initialize(src, trim_mode, percent); end

  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def is_erb_stag?(s); end

  sig do
    params(
      line: ::T.untyped,
      block: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def percent_line(line, &block); end

  sig do
    params(
      block: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def scan(&block); end

  sig do
    params(
      line: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def scan_line(line); end

  sig do
    params(
      line: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def trim_line1(line); end

  sig do
    params(
      line: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def trim_line2(line); end
end

# Utility module to define eRuby script as instance method.
#
# ### Example
#
# example.rhtml:
#
# ```
# <% for item in @items %>
# <b><%= item %></b>
# <% end %>
# ```
#
# example.rb:
#
# ```ruby
# require 'erb'
# class MyClass
#   extend ERB::DefMethod
#   def_erb_method('render()', 'example.rhtml')
#   def initialize(items)
#     @items = items
#   end
# end
# print MyClass.new([10,20,30]).render()
# ```
#
# result:
#
# ```
# <b>10</b>
#
# <b>20</b>
#
# <b>30</b>
# ```
module ERB::DefMethod
  # define *methodname* as instance method of current module, using
  # [`ERB`](https://docs.ruby-lang.org/en/2.7.0/ERB.html) object or eRuby file
  sig do
    params(
      methodname: ::T.untyped,
      erb_or_fname: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.def_erb_method(methodname, erb_or_fname); end
end

# A utility module for conversion routines, often handy in HTML generation.
module ERB::Util
  # Alias for:
  # [`html_escape`](https://docs.ruby-lang.org/en/2.7.0/ERB/Util.html#method-i-html_escape)
  def h(s); end

  # A utility method for escaping HTML tag characters in *s*.
  #
  # ```ruby
  # require "erb"
  # include ERB::Util
  #
  # puts html_escape("is a > 0 & a < 10?")
  # ```
  #
  # *Generates*
  #
  # ```
  # is a &gt; 0 &amp; a &lt; 10?
  # ```
  #
  #
  # Also aliased as:
  # [`h`](https://docs.ruby-lang.org/en/2.7.0/ERB/Util.html#method-i-h)
  def html_escape(s); end

  # Alias for:
  # [`html_escape`](https://docs.ruby-lang.org/en/2.7.0/ERB/Util.html#method-i-html_escape)
  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.h(s); end

  # A utility method for escaping HTML tag characters in *s*.
  #
  # ```ruby
  # require "erb"
  # include ERB::Util
  #
  # puts html_escape("is a > 0 & a < 10?")
  # ```
  #
  # *Generates*
  #
  # ```
  # is a &gt; 0 &amp; a &lt; 10?
  # ```
  #
  #
  # Also aliased as:
  # [`h`](https://docs.ruby-lang.org/en/2.7.0/ERB/Util.html#method-c-h)
  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.html_escape(s); end

  # Alias for:
  # [`url_encode`](https://docs.ruby-lang.org/en/2.7.0/ERB/Util.html#method-i-url_encode)
  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.u(s); end

  # A utility method for encoding the
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) *s* as a URL.
  #
  # ```ruby
  # require "erb"
  # include ERB::Util
  #
  # puts url_encode("Programming Ruby:  The Pragmatic Programmer's Guide")
  # ```
  #
  # *Generates*
  #
  # ```
  # Programming%20Ruby%3A%20%20The%20Pragmatic%20Programmer%27s%20Guide
  # ```
  #
  #
  # Also aliased as:
  # [`u`](https://docs.ruby-lang.org/en/2.7.0/ERB/Util.html#method-c-u)
  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.url_encode(s); end
end
