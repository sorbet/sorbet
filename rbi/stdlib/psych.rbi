# typed: __STDLIB_INTERNAL

###
# # Overview
#
# [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html) is a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) parser and emitter.
# [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html) leverages libyaml
# [Home page: https://pyyaml.org/wiki/LibYAML] or [HG repo:
# https://bitbucket.org/xi/libyaml] for its
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) parsing and emitting
# capabilities. In addition to wrapping libyaml,
# [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html) also knows how to
# serialize and de-serialize most Ruby objects to and from the
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) format.
#
# # I NEED TO PARSE OR EMIT [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) RIGHT NOW!
#
# ```ruby
# # Parse some YAML
# Psych.load("--- foo") # => "foo"
#
# # Emit some YAML
# Psych.dump("foo")     # => "--- foo\n...\n"
# { :a => 'b'}.to_yaml  # => "---\n:a: b\n"
# ```
#
# Got more time on your hands?  Keep on reading!
#
# ## [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) Parsing
#
# [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html) provides a range of
# interfaces for parsing a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document ranging from
# low level to high level, depending on your parsing needs. At the lowest level,
# is an event based parser. Mid level is access to the raw
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) AST, and at the
# highest level is the ability to unmarshal
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) to Ruby objects.
#
# ## [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) Emitting
#
# [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html) provides a range of
# interfaces ranging from low to high level for producing
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) documents. Very
# similar to the [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) parsing
# interfaces, [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html) provides
# at the lowest level, an event based system, mid-level is building a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) AST, and the highest
# level is converting a Ruby object straight to a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document.
#
# ## High-level API
#
# ### Parsing
#
# The high level [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) parser
# provided by [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html) simply
# takes [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) as input and
# returns a Ruby data structure. For information on using the high level parser
# see
# [`Psych.load`](https://docs.ruby-lang.org/en/2.7.0/Psych.html#method-c-load)
#
# #### Reading from a string
#
# ```ruby
# Psych.load("--- a")             # => 'a'
# Psych.load("---\n - a\n - b")   # => ['a', 'b']
# ```
#
# #### Reading from a file
#
# ```ruby
# Psych.load_file("database.yml")
# ```
#
# #### [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) handling
#
# ```ruby
# begin
#   # The second argument changes only the exception contents
#   Psych.parse("--- `", "file.txt")
# rescue Psych::SyntaxError => ex
#   ex.file    # => 'file.txt'
#   ex.message # => "(file.txt): found character that cannot start any token"
# end
# ```
#
# ### Emitting
#
# The high level emitter has the easiest interface.
# [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html) simply takes a Ruby
# data structure and converts it to a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document. See
# [`Psych.dump`](https://docs.ruby-lang.org/en/2.7.0/Psych.html#method-c-dump)
# for more information on dumping a Ruby data structure.
#
# #### Writing to a string
#
# ```ruby
# # Dump an array, get back a YAML string
# Psych.dump(['a', 'b'])  # => "---\n- a\n- b\n"
#
# # Dump an array to an IO object
# Psych.dump(['a', 'b'], StringIO.new)  # => #<StringIO:0x000001009d0890>
#
# # Dump an array with indentation set
# Psych.dump(['a', ['b']], :indentation => 3) # => "---\n- a\n-  - b\n"
#
# # Dump an array to an IO with indentation set
# Psych.dump(['a', ['b']], StringIO.new, :indentation => 3)
# ```
#
# #### Writing to a file
#
# Currently there is no direct API for dumping Ruby structure to file:
#
# ```ruby
# File.open('database.yml', 'w') do |file|
#   file.write(Psych.dump(['a', 'b']))
# end
# ```
#
# ## Mid-level API
#
# ### Parsing
#
# [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html) provides access to
# an AST produced from parsing a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document. This tree is
# built using the
# [`Psych::Parser`](https://docs.ruby-lang.org/en/2.7.0/Psych/Parser.html) and
# [`Psych::TreeBuilder`](https://docs.ruby-lang.org/en/2.7.0/Psych/TreeBuilder.html).
# The AST can be examined and manipulated freely. Please see
# [`Psych::parse_stream`](https://docs.ruby-lang.org/en/2.7.0/Psych.html#method-c-parse_stream),
# [`Psych::Nodes`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes.html), and
# [`Psych::Nodes::Node`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Node.html)
# for more information on dealing with
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) syntax trees.
#
# #### Reading from a string
#
# ```ruby
# # Returns Psych::Nodes::Stream
# Psych.parse_stream("---\n - a\n - b")
#
# # Returns Psych::Nodes::Document
# Psych.parse("---\n - a\n - b")
# ```
#
# #### Reading from a file
#
# ```ruby
# # Returns Psych::Nodes::Stream
# Psych.parse_stream(File.read('database.yml'))
#
# # Returns Psych::Nodes::Document
# Psych.parse_file('database.yml')
# ```
#
# #### [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html) handling
#
# ```ruby
# begin
#   # The second argument changes only the exception contents
#   Psych.parse("--- `", "file.txt")
# rescue Psych::SyntaxError => ex
#   ex.file    # => 'file.txt'
#   ex.message # => "(file.txt): found character that cannot start any token"
# end
# ```
#
# ### Emitting
#
# At the mid level is building an AST. This AST is exactly the same as the AST
# used when parsing a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html)
# document. Users can build an AST by hand and the AST knows how to emit itself
# as a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document. See
# [`Psych::Nodes`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes.html),
# [`Psych::Nodes::Node`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Node.html),
# and
# [`Psych::TreeBuilder`](https://docs.ruby-lang.org/en/2.7.0/Psych/TreeBuilder.html)
# for more information on building a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) AST.
#
# #### Writing to a string
#
# ```ruby
# # We need Psych::Nodes::Stream (not Psych::Nodes::Document)
# stream = Psych.parse_stream("---\n - a\n - b")
#
# stream.to_yaml # => "---\n- a\n- b\n"
# ```
#
# #### Writing to a file
#
# ```ruby
# # We need Psych::Nodes::Stream (not Psych::Nodes::Document)
# stream = Psych.parse_stream(File.read('database.yml'))
#
# File.open('database.yml', 'w') do |file|
#   file.write(stream.to_yaml)
# end
# ```
#
# ## Low-level API
#
# ### Parsing
#
# The lowest level parser should be used when the
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) input is already
# known, and the developer does not want to pay the price of building an AST or
# automatic detection and conversion to Ruby objects. See
# [`Psych::Parser`](https://docs.ruby-lang.org/en/2.7.0/Psych/Parser.html) for
# more information on using the event based parser.
#
# #### Reading to [`Psych::Nodes::Stream`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Stream.html) structure
#
# ```ruby
# parser = Psych::Parser.new(TreeBuilder.new) # => #<Psych::Parser>
# parser = Psych.parser                       # it's an alias for the above
#
# parser.parse("---\n - a\n - b")             # => #<Psych::Parser>
# parser.handler                              # => #<Psych::TreeBuilder>
# parser.handler.root                         # => #<Psych::Nodes::Stream>
# ```
#
# #### Receiving an events stream
#
# ```ruby
# recorder = Psych::Handlers::Recorder.new
# parser = Psych::Parser.new(recorder)
#
# parser.parse("---\n - a\n - b")
# recorder.events # => [list of [event, args] lists]
#                 # event is one of: Psych::Handler::EVENTS
#                 # args are the arguments passed to the event
# ```
#
# ### Emitting
#
# The lowest level emitter is an event based system. Events are sent to a
# [`Psych::Emitter`](https://docs.ruby-lang.org/en/2.7.0/Psych/Emitter.html)
# object. That object knows how to convert the events to a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document. This
# interface should be used when document format is known in advance or speed is
# a concern. See
# [`Psych::Emitter`](https://docs.ruby-lang.org/en/2.7.0/Psych/Emitter.html) for
# more information.
#
# #### Writing to a Ruby structure
#
# ```ruby
# Psych.parser.parse("--- a")       # => #<Psych::Parser>
#
# parser.handler.first              # => #<Psych::Nodes::Stream>
# parser.handler.first.to_ruby      # => ["a"]
#
# parser.handler.root.first         # => #<Psych::Nodes::Document>
# parser.handler.root.first.to_ruby # => "a"
#
# # You can instantiate an Emitter manually
# Psych::Visitors::ToRuby.new.accept(parser.handler.root.first)
# # => "a"
# ```
module Psych
  # The version of libyaml
  # [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html) is using
  LIBYAML_VERSION = T.let(T.unsafe(nil), String)
  # Deprecation guard
  NOT_GIVEN = T.let(T.unsafe(nil), Object)

  ###
  # Load `yaml` in to a Ruby data structure. If multiple documents are provided,
  # the object contained in the first document will be returned. `filename` will
  # be used in the exception message if any exception is raised while parsing.
  # If `yaml` is empty, it returns the specified `fallback` return value, which
  # defaults to `false`.
  #
  # Raises a
  # [`Psych::SyntaxError`](https://docs.ruby-lang.org/en/2.7.0/Psych/SyntaxError.html)
  # when a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) syntax error
  # is detected.
  #
  # Example:
  #
  # ```ruby
  # Psych.load("--- a")             # => 'a'
  # Psych.load("---\n - a\n - b")   # => ['a', 'b']
  #
  # begin
  #   Psych.load("--- `", filename: "file.txt")
  # rescue Psych::SyntaxError => ex
  #   ex.file    # => 'file.txt'
  #   ex.message # => "(file.txt): found character that cannot start any token"
  # end
  # ```
  #
  # When the optional `symbolize_names` keyword argument is set to a true value,
  # returns symbols for keys in
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) objects (default:
  # strings).
  #
  # ```ruby
  # Psych.load("---\n foo: bar")                         # => {"foo"=>"bar"}
  # Psych.load("---\n foo: bar", symbolize_names: true)  # => {:foo=>"bar"}
  # ```
  #
  # Raises a [`TypeError`](https://docs.ruby-lang.org/en/2.7.0/TypeError.html)
  # when `yaml` parameter is
  # [`NilClass`](https://docs.ruby-lang.org/en/2.7.0/NilClass.html)
  #
  # NOTE: This method \*should not\* be used to parse untrusted documents, such
  # as [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) documents that
  # are supplied via user input. Instead, please use the
  # [`safe_load`](https://docs.ruby-lang.org/en/2.7.0/Psych.html#method-c-safe_load)
  # method.
  sig do
    params(
      yaml: T.any(String, StringIO, IO),
      legacy_filename: Object,
      permitted_classes: T::Array[T::Class[T.anything]],
      permitted_symbols: T::Array[Symbol],
      aliases: T::Boolean,
      filename: T.nilable(String),
      fallback: T.untyped,
      symbolize_names: T::Boolean,
      freeze: T::Boolean,
    )
    .returns(T.untyped)
  end
  def self.load(yaml, legacy_filename = T.unsafe(nil), permitted_classes: [Symbol], permitted_symbols: [], aliases: false, filename: T.unsafe(nil), fallback: T.unsafe(nil), symbolize_names: T.unsafe(nil), freeze: T.unsafe(nil)); end

  ###
  # Safely load the yaml string in `yaml`. By default, only the following
  # classes are allowed to be deserialized:
  #
  # *   [`TrueClass`](https://docs.ruby-lang.org/en/2.7.0/TrueClass.html)
  # *   [`FalseClass`](https://docs.ruby-lang.org/en/2.7.0/FalseClass.html)
  # *   [`NilClass`](https://docs.ruby-lang.org/en/2.7.0/NilClass.html)
  # *   [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html)
  # *   [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # *   [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html)
  # *   [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  #
  #
  # Recursive data structures are not allowed by default. Arbitrary classes can
  # be allowed by adding those classes to the `permitted_classes` keyword
  # argument. They are additive. For example, to allow
  # [`Date`](https://docs.ruby-lang.org/en/2.7.0/Date.html) deserialization:
  #
  # ```ruby
  # Psych.safe_load(yaml, permitted_classes: [Date])
  # ```
  #
  # Now the [`Date`](https://docs.ruby-lang.org/en/2.7.0/Date.html) class can be
  # loaded in addition to the classes listed above.
  #
  # Aliases can be explicitly allowed by changing the `aliases` keyword
  # argument. For example:
  #
  # ```ruby
  # x = []
  # x << x
  # yaml = Psych.dump x
  # Psych.safe_load yaml               # => raises an exception
  # Psych.safe_load yaml, aliases: true # => loads the aliases
  # ```
  #
  # A
  # [`Psych::DisallowedClass`](https://docs.ruby-lang.org/en/2.7.0/Psych/DisallowedClass.html)
  # exception will be raised if the yaml contains a class that isn't in the
  # `permitted_classes` list.
  #
  # A
  # [`Psych::BadAlias`](https://docs.ruby-lang.org/en/2.7.0/Psych/BadAlias.html)
  # exception will be raised if the yaml contains aliases but the `aliases`
  # keyword argument is set to false.
  #
  # `filename` will be used in the exception message if any exception is raised
  # while parsing.
  #
  # When the optional `symbolize_names` keyword argument is set to a true value,
  # returns symbols for keys in
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) objects (default:
  # strings).
  #
  # ```ruby
  # Psych.safe_load("---\n foo: bar")                         # => {"foo"=>"bar"}
  # Psych.safe_load("---\n foo: bar", symbolize_names: true)  # => {:foo=>"bar"}
  # ```
  sig do
    params(
      yaml: T.any(String, StringIO, IO),
      legacy_permitted_classes: Object,
      legacy_permitted_symbols: Object,
      legacy_aliases: Object,
      legacy_filename: Object,
      permitted_classes: T::Array[T::Class[T.anything]],
      permitted_symbols: T::Array[Symbol],
      aliases: T::Boolean,
      filename: T.nilable(String),
      fallback: T.untyped,
      symbolize_names: T::Boolean,
      freeze: T::Boolean,
    )
    .returns(T.untyped)
  end
  def self.safe_load(yaml, legacy_permitted_classes = T.unsafe(nil), legacy_permitted_symbols = T.unsafe(nil), legacy_aliases = T.unsafe(nil), legacy_filename = T.unsafe(nil), permitted_classes: T.unsafe(nil), permitted_symbols: T.unsafe(nil), aliases: T.unsafe(nil), filename: T.unsafe(nil), fallback: T.unsafe(nil), symbolize_names: T.unsafe(nil), freeze: T.unsafe(nil)); end

  ###
  # Parse a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) string in
  # `yaml`. Returns the
  # [`Psych::Nodes::Document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Document.html).
  # `filename` is used in the exception message if a
  # [`Psych::SyntaxError`](https://docs.ruby-lang.org/en/2.7.0/Psych/SyntaxError.html)
  # is raised.
  #
  # Raises a
  # [`Psych::SyntaxError`](https://docs.ruby-lang.org/en/2.7.0/Psych/SyntaxError.html)
  # when a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) syntax error
  # is detected.
  #
  # Example:
  #
  # ```ruby
  # Psych.parse("---\n - a\n - b") # => #<Psych::Nodes::Document:0x00>
  #
  # begin
  #   Psych.parse("--- `", filename: "file.txt")
  # rescue Psych::SyntaxError => ex
  #   ex.file    # => 'file.txt'
  #   ex.message # => "(file.txt): found character that cannot start any token"
  # end
  # ```
  #
  # See [`Psych::Nodes`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes.html)
  # for more information about
  # [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) AST.
  sig do
    params(
      yaml: T.any(String, StringIO, IO),
      legacy_filename: Object,
      filename: T.nilable(String),
      fallback: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse(yaml, legacy_filename = T.unsafe(nil), filename: T.unsafe(nil), fallback: T.unsafe(nil)); end

  ###
  # Parse a file at `filename`. Returns the
  # [`Psych::Nodes::Document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Document.html).
  #
  # Raises a
  # [`Psych::SyntaxError`](https://docs.ruby-lang.org/en/2.7.0/Psych/SyntaxError.html)
  # when a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) syntax error
  # is detected.
  sig do
    params(
      filename: String,
      fallback: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.parse_file(filename, fallback: T.unsafe(nil)); end

  ###
  # Returns a default parser
  sig { returns(Psych::Parser) }
  def self.parser; end

  ###
  # Parse a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) string in
  # `yaml`. Returns the
  # [`Psych::Nodes::Stream`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Stream.html).
  # This method can handle multiple
  # [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) documents contained
  # in `yaml`. `filename` is used in the exception message if a
  # [`Psych::SyntaxError`](https://docs.ruby-lang.org/en/2.7.0/Psych/SyntaxError.html)
  # is raised.
  #
  # If a block is given, a
  # [`Psych::Nodes::Document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Document.html)
  # node will be yielded to the block as it's being parsed.
  #
  # Raises a
  # [`Psych::SyntaxError`](https://docs.ruby-lang.org/en/2.7.0/Psych/SyntaxError.html)
  # when a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) syntax error
  # is detected.
  #
  # Example:
  #
  # ```ruby
  # Psych.parse_stream("---\n - a\n - b") # => #<Psych::Nodes::Stream:0x00>
  #
  # Psych.parse_stream("--- a\n--- b") do |node|
  #   node # => #<Psych::Nodes::Document:0x00>
  # end
  #
  # begin
  #   Psych.parse_stream("--- `", filename: "file.txt")
  # rescue Psych::SyntaxError => ex
  #   ex.file    # => 'file.txt'
  #   ex.message # => "(file.txt): found character that cannot start any token"
  # end
  # ```
  #
  # Raises a [`TypeError`](https://docs.ruby-lang.org/en/2.7.0/TypeError.html)
  # when [`NilClass`](https://docs.ruby-lang.org/en/2.7.0/NilClass.html) is
  # passed.
  #
  # See [`Psych::Nodes`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes.html)
  # for more information about
  # [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) AST.
  sig do
    params(
      yaml: T.any(String, StringIO, IO),
      legacy_filename: Object,
      filename: T.nilable(String),
      block: T.nilable(T.proc.params(node: Psych::Nodes::Document).void),
    )
    .returns(Psych::Nodes::Stream)
  end
  def self.parse_stream(yaml, legacy_filename = T.unsafe(nil), filename: T.unsafe(nil), &block); end

  ###
  # Dump Ruby object `o` to a
  # [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) string. Optional
  # `options` may be passed in to control the output format. If an
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object is passed in, the
  # [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) will be dumped to
  # that [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object.
  #
  # Currently supported options are:
  #
  # `:indentation`
  # :   Number of space characters used to indent. Acceptable value should be in
  #     `0..9` range, otherwise option is ignored.
  #
  #     Default: `2`.
  # `:line_width`
  # :   Max character to wrap line at.
  #
  #     Default: `0` (meaning "wrap at 81").
  # `:canonical`
  # :   Write "canonical"
  #     [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) form (very
  #     verbose, yet strictly formal).
  #
  #     Default: `false`.
  # `:header`
  # :   Write `%YAML [version]` at the beginning of document.
  #
  #     Default: `false`.
  #
  #
  # Example:
  #
  # ```ruby
  # # Dump an array, get back a YAML string
  # Psych.dump(['a', 'b'])  # => "---\n- a\n- b\n"
  #
  # # Dump an array to an IO object
  # Psych.dump(['a', 'b'], StringIO.new)  # => #<StringIO:0x000001009d0890>
  #
  # # Dump an array with indentation set
  # Psych.dump(['a', ['b']], indentation: 3) # => "---\n- a\n-  - b\n"
  #
  # # Dump an array to an IO with indentation set
  # Psych.dump(['a', ['b']], StringIO.new, indentation: 3)
  # ```
  sig do
    params(
      o: T.untyped,
      io: T.untyped,
      options: T::Hash[Symbol, T.untyped],
    )
    .returns(T.untyped)
  end
  def self.dump(o, io = T.unsafe(nil), options = T.unsafe(nil)); end

  ###
  # Dump a list of objects as separate documents to a document stream.
  #
  # Example:
  #
  # ```ruby
  # Psych.dump_stream("foo\n  ", {}) # => "--- ! \"foo\\n  \"\n--- {}\n"
  # ```
  sig { params(objects: T.untyped).returns(T.untyped) }
  def self.dump_stream(*objects); end

  ###
  # Dump Ruby `object` to a
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string.
  sig { params(object: T.untyped).returns(String) }
  def self.to_json(object); end

  ###
  # Load multiple documents given in `yaml`. Returns the parsed documents as a
  # list. If a block is given, each document will be converted to Ruby and
  # passed to the block during parsing
  #
  # Example:
  #
  # ```ruby
  # Psych.load_stream("--- foo\n...\n--- bar\n...") # => ['foo', 'bar']
  #
  # list = []
  # Psych.load_stream("--- foo\n...\n--- bar\n...") do |ruby|
  #   list << ruby
  # end
  # list # => ['foo', 'bar']
  # ```
  sig do
    params(
      yaml: T.any(String, StringIO, IO),
      legacy_filename: Object,
      filename: T.nilable(String),
      fallback: T.untyped,
    )
    .returns(T::Array[T.untyped])
  end
  def self.load_stream(yaml, legacy_filename = T.unsafe(nil), filename: T.unsafe(nil), fallback: T.unsafe(nil)); end

  ###
  # Load the document contained in `filename`. Returns the yaml contained in
  # `filename` as a Ruby object, or if the file is empty, it returns the
  # specified `fallback` return value, which defaults to `false`.
  sig { params(filename: T.any(String, Pathname), kwargs: T.untyped).returns(T.untyped) }
  def self.load_file(filename, **kwargs); end

  ###
  # Safely loads the document contained in +filename+.  Returns the yaml contained in
  # +filename+ as a Ruby object, or if the file is empty, it returns
  # the specified +fallback+ return value, which defaults to +false+.
  # See safe_load for options.
  sig { params(filename: T.any(String, Pathname), kwargs: T.untyped).returns(T.untyped) }
  def self.safe_load_file(filename, **kwargs); end
end

class Psych::Exception < RuntimeError
end

class Psych::BadAlias < Psych::Exception
end

class Psych::ClassLoader
  BIG_DECIMAL = ::T.unsafe(nil)
  CACHE = ::T.unsafe(nil)
  COMPLEX = ::T.unsafe(nil)
  DATE = ::T.unsafe(nil)
  DATE_TIME = ::T.unsafe(nil)
  EXCEPTION = ::T.unsafe(nil)
  OBJECT = ::T.unsafe(nil)
  PSYCH_OMAP = ::T.unsafe(nil)
  PSYCH_SET = ::T.unsafe(nil)
  RANGE = ::T.unsafe(nil)
  RATIONAL = ::T.unsafe(nil)
  REGEXP = ::T.unsafe(nil)
  STRUCT = ::T.unsafe(nil)
  SYMBOL = ::T.unsafe(nil)

  def big_decimal(); end

  def complex(); end

  def date(); end

  def date_time(); end

  def exception(); end

  def initialize(); end

  def load(klassname); end

  def object(); end

  def psych_omap(); end

  def psych_set(); end

  def range(); end

  def rational(); end

  def regexp(); end

  def struct(); end

  def symbol(); end

  def symbolize(sym); end
end

class Psych::ClassLoader::Restricted < Psych::ClassLoader

  def initialize(classes, symbols); end

  def symbolize(sym); end
end

# If an object defines `encode_with`, then an instance of
# [`Psych::Coder`](https://docs.ruby-lang.org/en/2.7.0/Psych/Coder.html) will be
# passed to the method when the object is being serialized. The
# [`Coder`](https://docs.ruby-lang.org/en/2.7.0/Psych/Coder.html) automatically
# assumes a
# [`Psych::Nodes::Mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Mapping.html)
# is being emitted. Other objects like Sequence and Scalar may be emitted if
# `seq=` or `scalar=` are called, respectively.
class Psych::Coder

  def [](k); end

  # Also aliased as:
  # [`add`](https://docs.ruby-lang.org/en/2.7.0/Psych/Coder.html#method-i-add)
  def []=(k, v); end

  # Alias for:
  # [`[]=`](https://docs.ruby-lang.org/en/2.7.0/Psych/Coder.html#method-i-5B-5D-3D)
  def add(k, v); end

  def implicit(); end

  def implicit=(implicit); end

  def initialize(tag); end

  # Emit a map. The coder will be yielded to the block.
  def map(tag=T.unsafe(nil), style=T.unsafe(nil)); end

  # Emit a map with `value`
  def map=(map); end

  def object(); end

  def object=(object); end

  # Emit a sequence with `map` and `tag`
  def represent_map(tag, map); end

  # Emit an arbitrary object `obj` and `tag`
  def represent_object(tag, obj); end

  # Emit a scalar with `value` and `tag`
  def represent_scalar(tag, value); end

  # Emit a sequence with `list` and `tag`
  def represent_seq(tag, list); end

  def scalar(*args); end

  # Emit a scalar with `value`
  def scalar=(value); end

  def seq(); end

  # Emit a sequence of `list`
  def seq=(list); end

  def style(); end

  def style=(style); end

  def tag(); end

  def tag=(tag); end

  def type(); end
end

class Psych::DisallowedClass < Psych::Exception

  def initialize(klass_name); end
end

class Psych::Emitter < Psych::Handler

  # Emit an alias with `anchor`.
  #
  # See
  # [`Psych::Handler#alias`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html#method-i-alias)
  def alias(arg0); end

  # Get the output style, canonical or not.
  def canonical(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the output style to
  # canonical, or not.
  def canonical=(canonical); end

  # End a document emission with an `implicit` ending.
  #
  # See
  # [`Psych::Handler#end_document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html#method-i-end_document)
  def end_document(arg0); end

  # Emit the end of a mapping.
  #
  # See
  # [`Psych::Handler#end_mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html#method-i-end_mapping)
  def end_mapping(); end

  # End sequence emission.
  #
  # See
  # [`Psych::Handler#end_sequence`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html#method-i-end_sequence)
  def end_sequence(); end

  # End a stream emission
  #
  # See
  # [`Psych::Handler#end_stream`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html#method-i-end_stream)
  def end_stream(); end

  # Get the indentation level.
  def indentation(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the indentation level
  # to `level`. The level must be less than 10 and greater than 1.
  def indentation=(indentation); end

  def initialize(*arg0); end

  # Get the preferred line width.
  def line_width(); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the preferred line
  # with to `width`.
  def line_width=(line_width); end

  # Emit a scalar with `value`, `anchor`, `tag`, and a `plain` or `quoted`
  # string type with `style`.
  #
  # See
  # [`Psych::Handler#scalar`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html#method-i-scalar)
  def scalar(arg0, arg1, arg2, arg3, arg4, arg5); end

  # Start a document emission with
  # [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) `version`, `tags`,
  # and an `implicit` start.
  #
  # See
  # [`Psych::Handler#start_document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html#method-i-start_document)
  def start_document(arg0, arg1, arg2); end

  # Start emitting a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) map
  # with `anchor`, `tag`, an `implicit` start and end, and `style`.
  #
  # See
  # [`Psych::Handler#start_mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html#method-i-start_mapping)
  def start_mapping(arg0, arg1, arg2, arg3); end

  # Start emitting a sequence with `anchor`, a `tag`, `implicit` sequence start
  # and end, along with `style`.
  #
  # See
  # [`Psych::Handler#start_sequence`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html#method-i-start_sequence)
  def start_sequence(arg0, arg1, arg2, arg3); end

  # Start a stream emission with `encoding`
  #
  # See
  # [`Psych::Handler#start_stream`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html#method-i-start_stream)
  def start_stream(arg0); end
end

# [`Psych::Handler`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html) is
# an abstract base class that defines the events used when dealing with
# [`Psych::Parser`](https://docs.ruby-lang.org/en/2.7.0/Psych/Parser.html).
# Clients who want to use
# [`Psych::Parser`](https://docs.ruby-lang.org/en/2.7.0/Psych/Parser.html)
# should implement a class that inherits from
# [`Psych::Handler`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html) and
# define events that they can handle.
#
# [`Psych::Handler`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html)
# defines all events that
# [`Psych::Parser`](https://docs.ruby-lang.org/en/2.7.0/Psych/Parser.html) can
# possibly send to event handlers.
#
# See [`Psych::Parser`](https://docs.ruby-lang.org/en/2.7.0/Psych/Parser.html)
# for more details
class Psych::Handler
  # Events that a
  # [`Handler`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html) should
  # respond to.
  EVENTS = ::T.unsafe(nil)
  # Default dumping options
  OPTIONS = ::T.unsafe(nil)

  # Called when an alias is found to `anchor`. `anchor` will be the name of the
  # anchor found.
  #
  # ### Example
  #
  # Here we have an example of an array that references itself in YAML:
  #
  # ```
  # --- &ponies
  # - first element
  # - *ponies
  # ```
  #
  # &ponies is the anchor, \*ponies is the alias. In this case, alias is called
  # with "ponies".
  def alias(anchor); end

  # Called when an empty event happens. (Which, as far as I can tell, is never).
  def empty(); end

  # Called with the document ends. `implicit` is a boolean value indicating
  # whether or not the document has an implicit ending.
  #
  # ### Example
  #
  # Given the following YAML:
  #
  # ```
  # ---
  #   hello world
  # ```
  #
  # `implicit` will be true. Given this YAML:
  #
  # ```
  # ---
  #   hello world
  # ...
  # ```
  #
  # `implicit` will be false.
  def end_document(implicit); end

  # Called when a map ends
  def end_mapping(); end

  # Called when a sequence ends.
  def end_sequence(); end

  # Called when the [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html)
  # stream ends
  def end_stream(); end

  # Called before each event with line/column information.
  def event_location(start_line, start_column, end_line, end_column); end

  # Called when a scalar `value` is found. The scalar may have an `anchor`, a
  # `tag`, be implicitly `plain` or implicitly `quoted`
  #
  # `value` is the string value of the scalar `anchor` is an associated anchor
  # or nil `tag` is an associated tag or nil `plain` is a boolean value `quoted`
  # is a boolean value `style` is an integer idicating the string style
  #
  # See the constants in
  # [`Psych::Nodes::Scalar`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Scalar.html)
  # for the possible values of `style`
  #
  # ### Example
  #
  # Here is a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document
  # that exercises most of the possible ways this method can be called:
  #
  # ```
  # ---
  # - !str "foo"
  # - &anchor fun
  # - many
  #   lines
  # - |
  #   many
  #   newlines
  # ```
  #
  # The above [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document
  # contains a list with four strings. Here are the parameters sent to this
  # method in the same order:
  #
  # ```ruby
  # # value               anchor    tag     plain   quoted  style
  # ["foo",               nil,      "!str", false,  false,  3    ]
  # ["fun",               "anchor", nil,    true,   false,  1    ]
  # ["many lines",        nil,      nil,    true,   false,  1    ]
  # ["many\nnewlines\n",  nil,      nil,    false,  true,   4    ]
  # ```
  def scalar(value, anchor, tag, plain, quoted, style); end

  # Called when the document starts with the declared `version`,
  # `tag_directives`, if the document is `implicit`.
  #
  # `version` will be an array of integers indicating the
  # [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) version being dealt
  # with, `tag_directives` is a list of tuples indicating the prefix and suffix
  # of each tag, and `implicit` is a boolean indicating whether the document is
  # started implicitly.
  #
  # ### Example
  #
  # Given the following YAML:
  #
  # ```
  # %YAML 1.1
  # %TAG ! tag:tenderlovemaking.com,2009:
  # --- !squee
  # ```
  #
  # The parameters for
  # [`start_document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html#method-i-start_document)
  # must be this:
  #
  # ```ruby
  # version         # => [1, 1]
  # tag_directives  # => [["!", "tag:tenderlovemaking.com,2009:"]]
  # implicit        # => false
  # ```
  def start_document(version, tag_directives, implicit); end

  # Called when a map starts.
  #
  # `anchor` is the anchor associated with the map or `nil`. `tag` is the tag
  # associated with the map or `nil`. `implicit` is a boolean indicating whether
  # or not the map was implicitly started. `style` is an integer indicating the
  # mapping style.
  #
  # See the constants in
  # [`Psych::Nodes::Mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Mapping.html)
  # for the possible values of `style`.
  #
  # ### Example
  #
  # Here is a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document
  # that exercises most of the possible ways this method can be called:
  #
  # ```
  # ---
  # k: !!map { hello: world }
  # v: &pewpew
  #   hello: world
  # ```
  #
  # The above [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document
  # consists of three maps, an outer map that contains two inner maps. Below is
  # a matrix of the parameters sent in order to represent these three maps:
  #
  # ```ruby
  # # anchor    tag                       implicit  style
  # [nil,       nil,                      true,     1     ]
  # [nil,       "tag:yaml.org,2002:map",  false,    2     ]
  # ["pewpew",  nil,                      true,     1     ]
  # ```
  def start_mapping(anchor, tag, implicit, style); end

  # Called when a sequence is started.
  #
  # `anchor` is the anchor associated with the sequence or nil. `tag` is the tag
  # associated with the sequence or nil. `implicit` a boolean indicating whether
  # or not the sequence was implicitly started. `style` is an integer indicating
  # the list style.
  #
  # See the constants in
  # [`Psych::Nodes::Sequence`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Sequence.html)
  # for the possible values of `style`.
  #
  # ### Example
  #
  # Here is a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document
  # that exercises most of the possible ways this method can be called:
  #
  # ```
  # ---
  # - !!seq [
  #   a
  # ]
  # - &pewpew
  #   - b
  # ```
  #
  # The above [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document
  # consists of three lists, an outer list that contains two inner lists. Here
  # is a matrix of the parameters sent to represent these lists:
  #
  # ```ruby
  # # anchor    tag                       implicit  style
  # [nil,       nil,                      true,     1     ]
  # [nil,       "tag:yaml.org,2002:seq",  false,    2     ]
  # ["pewpew",  nil,                      true,     1     ]
  # ```
  def start_sequence(anchor, tag, implicit, style); end

  # Called with `encoding` when the
  # [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) stream starts. This
  # method is called once per stream. A stream may contain multiple documents.
  #
  # See the constants in
  # [`Psych::Parser`](https://docs.ruby-lang.org/en/2.7.0/Psych/Parser.html) for
  # the possible values of `encoding`.
  def start_stream(encoding); end

  # Is this handler a streaming handler?
  def streaming?(); end
end

# Configuration options for dumping
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html).
class Psych::Handler::DumperOptions

  def canonical(); end

  def canonical=(canonical); end

  def indentation(); end

  def indentation=(indentation); end

  def initialize(); end

  def line_width(); end

  def line_width=(line_width); end
end

module Psych::Handlers
end

class Psych::Handlers::DocumentStream < Psych::TreeBuilder

  def end_document(implicit_end=T.unsafe(nil)); end

  def initialize(&block); end

  def start_document(version, tag_directives, implicit); end
end

module Psych::JSON
end

module Psych::JSON::RubyEvents

  def visit_DateTime(o); end

  def visit_String(o); end

  def visit_Symbol(o); end

  def visit_Time(o); end
end

class Psych::JSON::Stream < Psych::Visitors::JSONTree
  include ::Psych::Streaming
  extend ::Psych::Streaming::ClassMethods
end

class Psych::JSON::Stream::Emitter < Psych::Stream::Emitter
  include ::Psych::JSON::YAMLEvents
end

# [`Psych::JSON::TreeBuilder`](https://docs.ruby-lang.org/en/2.7.0/Psych/JSON/TreeBuilder.html)
# is an event based AST builder. Events are sent to an instance of
# [`Psych::JSON::TreeBuilder`](https://docs.ruby-lang.org/en/2.7.0/Psych/JSON/TreeBuilder.html)
# and a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/Psych/JSON.html) AST is
# constructed.
class Psych::JSON::TreeBuilder < Psych::TreeBuilder
  include ::Psych::JSON::YAMLEvents
end

module Psych::JSON::YAMLEvents

  def end_document(implicit_end=T.unsafe(nil)); end

  def scalar(value, anchor, tag, plain, quoted, style); end

  def start_document(version, tag_directives, implicit); end

  def start_mapping(anchor, tag, implicit, style); end

  def start_sequence(anchor, tag, implicit, style); end
end

# # Overview
#
# When using
# [`Psych.load`](https://docs.ruby-lang.org/en/2.7.0/Psych.html#method-c-load)
# to deserialize a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html)
# document, the document is translated to an intermediary AST. That intermediary
# AST is then translated in to a Ruby object graph.
#
# In the opposite direction, when using
# [`Psych.dump`](https://docs.ruby-lang.org/en/2.7.0/Psych.html#method-c-dump),
# the Ruby object graph is translated to an intermediary AST which is then
# converted to a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html)
# document.
#
# [`Psych::Nodes`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes.html)
# contains all of the classes that make up the nodes of a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) AST. You can manually
# build an AST and use one of the visitors (see
# [`Psych::Visitors`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors.html))
# to convert that AST to either a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document or to a Ruby
# object graph.
#
# Here is an example of building an AST that represents a list with one scalar:
#
# ```ruby
# # Create our nodes
# stream = Psych::Nodes::Stream.new
# doc    = Psych::Nodes::Document.new
# seq    = Psych::Nodes::Sequence.new
# scalar = Psych::Nodes::Scalar.new('foo')
#
# # Build up our tree
# stream.children << doc
# doc.children    << seq
# seq.children    << scalar
# ```
#
# The stream is the root of the tree. We can then convert the tree to YAML:
#
# ```
# stream.to_yaml => "---\n- foo\n"
# ```
#
# Or convert it to Ruby:
#
# ```
# stream.to_ruby => [["foo"]]
# ```
#
# ## [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) AST Requirements
#
# A valid [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) AST **must**
# have one
# [`Psych::Nodes::Stream`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Stream.html)
# at the root. A
# [`Psych::Nodes::Stream`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Stream.html)
# node must have 1 or more
# [`Psych::Nodes::Document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Document.html)
# nodes as children.
#
# [`Psych::Nodes::Document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Document.html)
# nodes must have one and **only** one child. That child may be one of:
#
# *   [`Psych::Nodes::Sequence`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Sequence.html)
# *   [`Psych::Nodes::Mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Mapping.html)
# *   [`Psych::Nodes::Scalar`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Scalar.html)
#
#
# [`Psych::Nodes::Sequence`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Sequence.html)
# and
# [`Psych::Nodes::Mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Mapping.html)
# nodes may have many children, but
# [`Psych::Nodes::Mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Mapping.html)
# nodes should have an even number of children.
#
# All of these are valid children for
# [`Psych::Nodes::Sequence`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Sequence.html)
# and
# [`Psych::Nodes::Mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Mapping.html)
# nodes:
#
# *   [`Psych::Nodes::Sequence`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Sequence.html)
# *   [`Psych::Nodes::Mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Mapping.html)
# *   [`Psych::Nodes::Scalar`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Scalar.html)
# *   [`Psych::Nodes::Alias`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Alias.html)
#
#
# [`Psych::Nodes::Scalar`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Scalar.html)
# and
# [`Psych::Nodes::Alias`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Alias.html)
# are both terminal nodes and should not have any children.
module Psych::Nodes
end

# This class represents a [YAML Alias](http://yaml.org/spec/1.1/#alias). It
# points to an `anchor`.
#
# A
# [`Psych::Nodes::Alias`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Alias.html)
# is a terminal node and may have no children.
class Psych::Nodes::Alias < Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out) {{fixed: T.untyped}}

  def alias?(); end

  # The anchor this alias links to
  def anchor(); end

  # The anchor this alias links to
  def anchor=(anchor); end

  def initialize(anchor); end
end

# This represents a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html)
# [`Document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Document.html).
# This node must be a child of
# [`Psych::Nodes::Stream`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Stream.html).
# A
# [`Psych::Nodes::Document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Document.html)
# must have one child, and that child may be one of the following:
#
# *   [`Psych::Nodes::Sequence`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Sequence.html)
# *   [`Psych::Nodes::Mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Mapping.html)
# *   [`Psych::Nodes::Scalar`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Scalar.html)
class Psych::Nodes::Document < Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out) {{fixed: T.untyped}}

  def document?(); end

  # Was this document implicitly created?
  def implicit(); end

  # Was this document implicitly created?
  def implicit=(implicit); end

  # Is the end of the document implicit?
  def implicit_end(); end

  # Is the end of the document implicit?
  def implicit_end=(implicit_end); end

  def initialize(version=T.unsafe(nil), tag_directives=T.unsafe(nil), implicit=T.unsafe(nil)); end

  # Returns the root node. A
  # [`Document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Document.html)
  # may only have one root node: http://yaml.org/spec/1.1/#id898031
  def root(); end

  # A list of tag directives for this document
  def tag_directives(); end

  # A list of tag directives for this document
  def tag_directives=(tag_directives); end

  # The version of the [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html)
  # document
  def version(); end

  # The version of the [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html)
  # document
  def version=(version); end
end

# This class represents a [YAML Mapping](http://yaml.org/spec/1.1/#mapping).
#
# A
# [`Psych::Nodes::Mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Mapping.html)
# node may have 0 or more children, but must have an even number of children.
# Here are the valid children a
# [`Psych::Nodes::Mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Mapping.html)
# node may have:
#
# *   [`Psych::Nodes::Sequence`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Sequence.html)
# *   [`Psych::Nodes::Mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Mapping.html)
# *   [`Psych::Nodes::Scalar`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Scalar.html)
# *   [`Psych::Nodes::Alias`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Alias.html)
class Psych::Nodes::Mapping < Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out) {{fixed: T.untyped}}

  # Any Map Style
  ANY = ::T.unsafe(nil)
  # Block Map Style
  BLOCK = ::T.unsafe(nil)
  # Flow Map Style
  FLOW = ::T.unsafe(nil)

  # The optional anchor for this mapping
  def anchor(); end

  # The optional anchor for this mapping
  def anchor=(anchor); end

  # Is this an implicit mapping?
  def implicit(); end

  # Is this an implicit mapping?
  def implicit=(implicit); end

  def initialize(anchor=T.unsafe(nil), tag=T.unsafe(nil), implicit=T.unsafe(nil), style=T.unsafe(nil)); end

  def mapping?(); end

  # The style of this mapping
  def style(); end

  # The style of this mapping
  def style=(style); end

  # The optional tag for this mapping
  def tag(); end

  # The optional tag for this mapping
  def tag=(tag); end
end

# The base class for any
# [`Node`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Node.html) in a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) parse tree. This class
# should never be instantiated.
class Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out) {{fixed: T.untyped}}

  def alias?(); end

  # The children of this node
  def children(); end

  def document?(); end

  # Iterate over each node in the tree. Yields each node to `block` depth first.
  def each(&block); end

  # The column number where this node ends
  def end_column(); end

  # The column number where this node ends
  def end_column=(end_column); end

  # The line number where this node ends
  def end_line(); end

  # The line number where this node ends
  def end_line=(end_line); end

  def initialize(); end

  def mapping?(); end

  def scalar?(); end

  def sequence?(); end

  # The column number where this node start
  def start_column(); end

  # The column number where this node start
  def start_column=(start_column); end

  # The line number where this node start
  def start_line(); end

  # The line number where this node start
  def start_line=(start_line); end

  def stream?(); end

  # An associated tag
  def tag(); end

  # Convert this node to Ruby.
  #
  # See also
  # [`Psych::Visitors::ToRuby`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/ToRuby.html)
  #
  # Also aliased as:
  # [`transform`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Node.html#method-i-transform)
  def to_ruby(); end

  # Alias for:
  # [`yaml`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Node.html#method-i-yaml)
  def to_yaml(io=T.unsafe(nil), options=T.unsafe(nil)); end

  # Alias for:
  # [`to_ruby`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Node.html#method-i-to_ruby)
  def transform(); end

  # Convert this node to
  # [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html).
  #
  # See also
  # [`Psych::Visitors::Emitter`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/Emitter.html)
  #
  # Also aliased as:
  # [`to_yaml`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Node.html#method-i-to_yaml)
  def yaml(io=T.unsafe(nil), options=T.unsafe(nil)); end
end

# This class represents a [YAML Scalar](http://yaml.org/spec/1.1/#id858081).
#
# This node type is a terminal node and should not have any children.
class Psych::Nodes::Scalar < Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out) {{fixed: T.untyped}}

  # Any style scalar, the emitter chooses
  ANY = ::T.unsafe(nil)
  # Double quoted style
  DOUBLE_QUOTED = ::T.unsafe(nil)
  # Folded style
  FOLDED = ::T.unsafe(nil)
  # Literal style
  LITERAL = ::T.unsafe(nil)
  # Plain scalar style
  PLAIN = ::T.unsafe(nil)
  # Single quoted style
  SINGLE_QUOTED = ::T.unsafe(nil)

  # The anchor value (if there is one)
  def anchor(); end

  # The anchor value (if there is one)
  def anchor=(anchor); end

  def initialize(value, anchor=T.unsafe(nil), tag=T.unsafe(nil), plain=T.unsafe(nil), quoted=T.unsafe(nil), style=T.unsafe(nil)); end

  # Is this a plain scalar?
  def plain(); end

  # Is this a plain scalar?
  def plain=(plain); end

  # Is this scalar quoted?
  def quoted(); end

  # Is this scalar quoted?
  def quoted=(quoted); end

  def scalar?(); end

  # The style of this scalar
  def style(); end

  # The style of this scalar
  def style=(style); end

  # The tag value (if there is one)
  def tag(); end

  # The tag value (if there is one)
  def tag=(tag); end

  # The scalar value
  def value(); end

  # The scalar value
  def value=(value); end
end

# This class represents a [YAML
# sequence](http://yaml.org/spec/1.1/#sequence/syntax).
#
# A [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) sequence is
# basically a list, and looks like this:
#
# ```
# %YAML 1.1
# ---
# - I am
# - a Sequence
# ```
#
# A [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) sequence may have an
# anchor like this:
#
# ```
# %YAML 1.1
# ---
# &A [
#   "This sequence",
#   "has an anchor"
# ]
# ```
#
# A [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) sequence may also
# have a tag like this:
#
# ```
# %YAML 1.1
# ---
# !!seq [
#   "This sequence",
#   "has a tag"
# ]
# ```
#
# This class represents a sequence in a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document. A
# [`Psych::Nodes::Sequence`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Sequence.html)
# node may have 0 or more children. Valid children for this node are:
#
# *   [`Psych::Nodes::Sequence`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Sequence.html)
# *   [`Psych::Nodes::Mapping`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Mapping.html)
# *   [`Psych::Nodes::Scalar`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Scalar.html)
# *   [`Psych::Nodes::Alias`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Alias.html)
class Psych::Nodes::Sequence < Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out) {{fixed: T.untyped}}

  # Any Styles, emitter chooses
  ANY = ::T.unsafe(nil)
  # Block style sequence
  BLOCK = ::T.unsafe(nil)
  # Flow style sequence
  FLOW = ::T.unsafe(nil)

  # The anchor for this sequence (if any)
  def anchor(); end

  # The anchor for this sequence (if any)
  def anchor=(anchor); end

  # Is this sequence started implicitly?
  def implicit(); end

  # Is this sequence started implicitly?
  def implicit=(implicit); end

  def initialize(anchor=T.unsafe(nil), tag=T.unsafe(nil), implicit=T.unsafe(nil), style=T.unsafe(nil)); end

  def sequence?(); end

  # The sequence style used
  def style(); end

  # The sequence style used
  def style=(style); end

  # The tag name for this sequence (if any)
  def tag(); end

  # The tag name for this sequence (if any)
  def tag=(tag); end
end

# Represents a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) stream.
# This is the root node for any
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) parse tree. This node
# must have one or more child nodes. The only valid child node for a
# [`Psych::Nodes::Stream`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Stream.html)
# node is
# [`Psych::Nodes::Document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Nodes/Document.html).
class Psych::Nodes::Stream < Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out) {{fixed: T.untyped}}

  # Any encoding
  ANY = ::T.unsafe(nil)
  # UTF-16BE encoding
  UTF16BE = ::T.unsafe(nil)
  # UTF-16LE encoding
  UTF16LE = ::T.unsafe(nil)
  # UTF-8 encoding
  UTF8 = ::T.unsafe(nil)

  # The encoding used for this stream
  def encoding(); end

  # The encoding used for this stream
  def encoding=(encoding); end

  def initialize(encoding=T.unsafe(nil)); end

  def stream?(); end
end

class Psych::Omap < Hash
  include Enumerable
  extend T::Generic
  K = type_member(:out)
  V = type_member(:out)
  Elem = type_member(:out) {{fixed: T.untyped}}
end

# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) event parser class.
# This class parses a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html)
# document and calls events on the handler that is passed to the constructor.
# The events can be used for things such as constructing a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) AST or deserializing
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) documents. It can even
# be fed back to
# [`Psych::Emitter`](https://docs.ruby-lang.org/en/2.7.0/Psych/Emitter.html) to
# emit the same document that was parsed.
#
# See [`Psych::Handler`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html)
# for documentation on the events that
# [`Psych::Parser`](https://docs.ruby-lang.org/en/2.7.0/Psych/Parser.html)
# emits.
#
# Here is an example that prints out ever scalar found in a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document:
#
# ```ruby
# # Handler for detecting scalar values
# class ScalarHandler < Psych::Handler
#   def scalar value, anchor, tag, plain, quoted, style
#     puts value
#   end
# end
#
# parser = Psych::Parser.new(ScalarHandler.new)
# parser.parse(yaml_document)
# ```
#
# Here is an example that feeds the parser back in to
# [`Psych::Emitter`](https://docs.ruby-lang.org/en/2.7.0/Psych/Emitter.html).
# The [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document is read
# from STDIN and written back out to STDERR:
#
# ```ruby
# parser = Psych::Parser.new(Psych::Emitter.new($stderr))
# parser.parse($stdin)
# ```
#
# [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html) uses
# [`Psych::Parser`](https://docs.ruby-lang.org/en/2.7.0/Psych/Parser.html) in
# combination with
# [`Psych::TreeBuilder`](https://docs.ruby-lang.org/en/2.7.0/Psych/TreeBuilder.html)
# to construct an AST of the parsed
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document.
class Psych::Parser
  # Let the parser choose the encoding
  ANY = ::T.unsafe(nil)
  # UTF-16-BE [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html)
  # with BOM
  UTF16BE = ::T.unsafe(nil)
  # UTF-16-LE [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html)
  # with BOM
  UTF16LE = ::T.unsafe(nil)
  # UTF-8 [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html)
  UTF8 = ::T.unsafe(nil)

  def external_encoding=(external_encoding); end

  # The handler on which events will be called
  def handler(); end

  # The handler on which events will be called
  def handler=(handler); end

  def initialize(handler=T.unsafe(nil)); end

  # Returns a
  # [`Psych::Parser::Mark`](https://docs.ruby-lang.org/en/2.7.0/Psych/Parser/Mark.html)
  # object that contains line, column, and index information.
  def mark(); end

  # Parse the [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document
  # contained in `yaml`. Events will be called on the handler set on the parser
  # instance.
  #
  # See [`Psych::Parser`](https://docs.ruby-lang.org/en/2.7.0/Psych/Parser.html)
  # and
  # [`Psych::Parser#handler`](https://docs.ruby-lang.org/en/2.7.0/Psych/Parser.html#attribute-i-handler)
  def parse(*arg0); end
end

class Psych::Parser::Mark

  def column(); end

  def column=(arg0); end

  def index(); end

  def index=(arg0); end

  def line(); end

  def line=(arg0); end
end

# Scan scalars for built in types
class Psych::ScalarScanner
  # Taken from http://yaml.org/type/float.html
  FLOAT = ::T.unsafe(nil)
  # Taken from http://yaml.org/type/int.html
  INTEGER = ::T.unsafe(nil)
  # Taken from http://yaml.org/type/timestamp.html
  TIME = ::T.unsafe(nil)

  def class_loader(); end

  def initialize(class_loader); end

  # Parse and return an int from `string`
  def parse_int(string); end

  # Parse and return a [`Time`](https://docs.ruby-lang.org/en/2.7.0/Time.html)
  # from `string`
  def parse_time(string); end

  # Tokenize `string` returning the Ruby object
  def tokenize(string); end
end

class Psych::Set < Hash
  include Enumerable
  extend T::Generic
  K = type_member(:out)
  V = type_member(:out)
  Elem = type_member(:out) {{fixed: T.untyped}}
end

# [`Psych::Stream`](https://docs.ruby-lang.org/en/2.7.0/Psych/Stream.html) is a
# streaming [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) emitter. It
# will not buffer your [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html),
# but send it straight to an
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
#
# Here is an example use:
#
# ```ruby
# stream = Psych::Stream.new($stdout)
# stream.start
# stream.push({:foo => 'bar'})
# stream.finish
# ```
#
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) will be immediately
# emitted to $stdout with no buffering.
#
# [`Psych::Stream#start`](https://docs.ruby-lang.org/en/2.7.0/Psych/Streaming.html#method-i-start)
# will take a block and ensure that
# [`Psych::Stream#finish`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/YAMLTree.html#method-i-finish)
# is called, so you can do this form:
#
# ```ruby
# stream = Psych::Stream.new($stdout)
# stream.start do |em|
#   em.push(:foo => 'bar')
# end
# ```
class Psych::Stream < Psych::Visitors::YAMLTree
  include ::Psych::Streaming
  extend ::Psych::Streaming::ClassMethods
end

class Psych::Stream::Emitter < Psych::Emitter

  def end_document(implicit_end=T.unsafe(nil)); end

  def streaming?(); end
end

module Psych::Streaming

  # Start streaming using `encoding`
  def start(encoding=T.unsafe(nil)); end
end

module Psych::Streaming::ClassMethods

  # Create a new streaming emitter. Emitter will print to `io`. See
  # [`Psych::Stream`](https://docs.ruby-lang.org/en/2.7.0/Psych/Stream.html) for
  # an example.
  def new(io); end
end

class Psych::SyntaxError < Psych::Exception

  def column(); end

  def context(); end

  def file(); end

  def initialize(file, line, col, offset, problem, context); end

  def line(); end

  def offset(); end

  def problem(); end
end

# This class works in conjunction with
# [`Psych::Parser`](https://docs.ruby-lang.org/en/2.7.0/Psych/Parser.html) to
# build an in-memory parse tree that represents a
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) document.
#
# ## Example
#
# ```ruby
# parser = Psych::Parser.new Psych::TreeBuilder.new
# parser.parse('--- foo')
# tree = parser.handler.root
# ```
#
# See [`Psych::Handler`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html)
# for documentation on the event methods used in this class.
class Psych::TreeBuilder < Psych::Handler

  def alias(anchor); end

  # Handles
  # [`end_document`](https://docs.ruby-lang.org/en/2.7.0/Psych/TreeBuilder.html#method-i-end_document)
  # events with `version`, `tag_directives`, and `implicit` styling.
  #
  # See
  # [`Psych::Handler#start_document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html#method-i-start_document)
  def end_document(implicit_end=T.unsafe(nil)); end

  def end_mapping(); end

  def end_sequence(); end

  def end_stream(); end

  def event_location(start_line, start_column, end_line, end_column); end

  def initialize(); end

  # Returns the root node for the built tree
  def root(); end

  def scalar(value, anchor, tag, plain, quoted, style); end

  # Handles
  # [`start_document`](https://docs.ruby-lang.org/en/2.7.0/Psych/TreeBuilder.html#method-i-start_document)
  # events with `version`, `tag_directives`, and `implicit` styling.
  #
  # See
  # [`Psych::Handler#start_document`](https://docs.ruby-lang.org/en/2.7.0/Psych/Handler.html#method-i-start_document)
  def start_document(version, tag_directives, implicit); end

  def start_mapping(anchor, tag, implicit, style); end

  def start_sequence(anchor, tag, implicit, style); end

  def start_stream(encoding); end
end

class Psych::UnsafeYAML < StandardError
end

module Psych::Visitors
end

class Psych::Visitors::DepthFirst < Psych::Visitors::Visitor

  def initialize(block); end
end

class Psych::Visitors::Emitter < Psych::Visitors::Visitor

  def initialize(io, options=T.unsafe(nil)); end

  def visit_Psych_Nodes_Alias(o); end

  def visit_Psych_Nodes_Document(o); end

  def visit_Psych_Nodes_Mapping(o); end

  def visit_Psych_Nodes_Scalar(o); end

  def visit_Psych_Nodes_Sequence(o); end

  def visit_Psych_Nodes_Stream(o); end
end

class Psych::Visitors::JSONTree < Psych::Visitors::YAMLTree
  include ::Psych::JSON::RubyEvents

  def accept(target); end

  def self.create(options=T.unsafe(nil)); end
end

class Psych::Visitors::NoAliasRuby < Psych::Visitors::ToRuby

  def visit_Psych_Nodes_Alias(o); end
end

# This class walks a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html)
# AST, converting each node to Ruby
class Psych::Visitors::ToRuby < Psych::Visitors::Visitor
  SHOVEL = ::T.unsafe(nil)

  def accept(target); end

  def class_loader(); end

  def initialize(ss, class_loader); end

  def visit_Psych_Nodes_Alias(o); end

  def visit_Psych_Nodes_Document(o); end

  def visit_Psych_Nodes_Mapping(o); end

  def visit_Psych_Nodes_Scalar(o); end

  def visit_Psych_Nodes_Sequence(o); end

  def visit_Psych_Nodes_Stream(o); end

  def self.create(); end
end

class Psych::Visitors::Visitor
  DISPATCH = ::T.unsafe(nil)

  def accept(target); end
end

# [`YAMLTree`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/YAMLTree.html)
# builds a [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) ast given a
# Ruby object. For example:
#
# ```ruby
# builder = Psych::Visitors::YAMLTree.new
# builder << { :foo => 'bar' }
# builder.tree # => #<Psych::Nodes::Stream .. }
# ```
class Psych::Visitors::YAMLTree < Psych::Visitors::Visitor

  # Alias for:
  # [`push`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/YAMLTree.html#method-i-push)
  def <<(object); end

  def accept(target); end

  def finish(); end

  def finished(); end

  def finished?(); end

  def initialize(emitter, ss, options); end

  # Also aliased as:
  # [`<<`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/YAMLTree.html#method-i-3C-3C)
  def push(object); end

  def start(encoding=T.unsafe(nil)); end

  def started(); end

  def started?(); end

  def tree(); end

  def visit_Array(o); end

  def visit_BasicObject(o); end

  def visit_BigDecimal(o); end

  def visit_Class(o); end

  def visit_Complex(o); end

  # Alias for:
  # [`visit_Integer`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/YAMLTree.html#method-i-visit_Integer)
  def visit_Date(o); end

  def visit_DateTime(o); end

  # Alias for:
  # [`visit_Object`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/YAMLTree.html#method-i-visit_Object)
  def visit_Delegator(o); end

  def visit_Encoding(o); end

  def visit_Enumerator(o); end

  def visit_Exception(o); end

  # Alias for:
  # [`visit_Integer`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/YAMLTree.html#method-i-visit_Integer)
  def visit_FalseClass(o); end

  def visit_Float(o); end

  def visit_Hash(o); end

  # Also aliased as:
  # [`visit_TrueClass`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/YAMLTree.html#method-i-visit_TrueClass),
  # [`visit_FalseClass`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/YAMLTree.html#method-i-visit_FalseClass),
  # [`visit_Date`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/YAMLTree.html#method-i-visit_Date)
  def visit_Integer(o); end

  def visit_Module(o); end

  def visit_NameError(o); end

  def visit_NilClass(o); end

  # Also aliased as:
  # [`visit_Delegator`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/YAMLTree.html#method-i-visit_Delegator)
  def visit_Object(o); end

  def visit_Psych_Omap(o); end

  def visit_Psych_Set(o); end

  def visit_Range(o); end

  def visit_Rational(o); end

  def visit_Regexp(o); end

  def visit_String(o); end

  def visit_Struct(o); end

  def visit_Symbol(o); end

  def visit_Time(o); end

  # Alias for:
  # [`visit_Integer`](https://docs.ruby-lang.org/en/2.7.0/Psych/Visitors/YAMLTree.html#method-i-visit_Integer)
  def visit_TrueClass(o); end

  def self.create(options=T.unsafe(nil), emitter=T.unsafe(nil)); end
end

class Psych::Visitors::YAMLTree::Registrar

  def id_for(target); end

  def initialize(); end

  def key?(target); end

  def node_for(target); end

  def register(target, node); end
end
