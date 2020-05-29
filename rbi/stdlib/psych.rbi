# typed: __STDLIB_INTERNAL

###
# = Overview
#
# Psych is a YAML parser and emitter.
# Psych leverages libyaml [Home page: https://pyyaml.org/wiki/LibYAML]
# or [HG repo: https://bitbucket.org/xi/libyaml] for its YAML parsing
# and emitting capabilities. In addition to wrapping libyaml, Psych also
# knows how to serialize and de-serialize most Ruby objects to and from
# the YAML format.
#
# = I NEED TO PARSE OR EMIT YAML RIGHT NOW!
#
#   # Parse some YAML
#   Psych.load("--- foo") # => "foo"
#
#   # Emit some YAML
#   Psych.dump("foo")     # => "--- foo\n...\n"
#   { :a => 'b'}.to_yaml  # => "---\n:a: b\n"
#
# Got more time on your hands?  Keep on reading!
#
# == YAML Parsing
#
# Psych provides a range of interfaces for parsing a YAML document ranging from
# low level to high level, depending on your parsing needs.  At the lowest
# level, is an event based parser.  Mid level is access to the raw YAML AST,
# and at the highest level is the ability to unmarshal YAML to Ruby objects.
#
# == YAML Emitting
#
# Psych provides a range of interfaces ranging from low to high level for
# producing YAML documents.  Very similar to the YAML parsing interfaces, Psych
# provides at the lowest level, an event based system, mid-level is building
# a YAML AST, and the highest level is converting a Ruby object straight to
# a YAML document.
#
# == High-level API
#
# === Parsing
#
# The high level YAML parser provided by Psych simply takes YAML as input and
# returns a Ruby data structure.  For information on using the high level parser
# see Psych.load
#
# ==== Reading from a string
#
#   Psych.load("--- a")             # => 'a'
#   Psych.load("---\n - a\n - b")   # => ['a', 'b']
#
# ==== Reading from a file
#
#   Psych.load_file("database.yml")
#
# ==== Exception handling
#
#   begin
#     # The second argument changes only the exception contents
#     Psych.parse("--- `", "file.txt")
#   rescue Psych::SyntaxError => ex
#     ex.file    # => 'file.txt'
#     ex.message # => "(file.txt): found character that cannot start any token"
#   end
#
# === Emitting
#
# The high level emitter has the easiest interface.  Psych simply takes a Ruby
# data structure and converts it to a YAML document.  See Psych.dump for more
# information on dumping a Ruby data structure.
#
# ==== Writing to a string
#
#   # Dump an array, get back a YAML string
#   Psych.dump(['a', 'b'])  # => "---\n- a\n- b\n"
#
#   # Dump an array to an IO object
#   Psych.dump(['a', 'b'], StringIO.new)  # => #<StringIO:0x000001009d0890>
#
#   # Dump an array with indentation set
#   Psych.dump(['a', ['b']], :indentation => 3) # => "---\n- a\n-  - b\n"
#
#   # Dump an array to an IO with indentation set
#   Psych.dump(['a', ['b']], StringIO.new, :indentation => 3)
#
# ==== Writing to a file
#
# Currently there is no direct API for dumping Ruby structure to file:
#
#   File.open('database.yml', 'w') do |file|
#     file.write(Psych.dump(['a', 'b']))
#   end
#
# == Mid-level API
#
# === Parsing
#
# Psych provides access to an AST produced from parsing a YAML document.  This
# tree is built using the Psych::Parser and Psych::TreeBuilder.  The AST can
# be examined and manipulated freely.  Please see Psych::parse_stream,
# Psych::Nodes, and Psych::Nodes::Node for more information on dealing with
# YAML syntax trees.
#
# ==== Reading from a string
#
#   # Returns Psych::Nodes::Stream
#   Psych.parse_stream("---\n - a\n - b")
#
#   # Returns Psych::Nodes::Document
#   Psych.parse("---\n - a\n - b")
#
# ==== Reading from a file
#
#   # Returns Psych::Nodes::Stream
#   Psych.parse_stream(File.read('database.yml'))
#
#   # Returns Psych::Nodes::Document
#   Psych.parse_file('database.yml')
#
# ==== Exception handling
#
#   begin
#     # The second argument changes only the exception contents
#     Psych.parse("--- `", "file.txt")
#   rescue Psych::SyntaxError => ex
#     ex.file    # => 'file.txt'
#     ex.message # => "(file.txt): found character that cannot start any token"
#   end
#
# === Emitting
#
# At the mid level is building an AST.  This AST is exactly the same as the AST
# used when parsing a YAML document.  Users can build an AST by hand and the
# AST knows how to emit itself as a YAML document.  See Psych::Nodes,
# Psych::Nodes::Node, and Psych::TreeBuilder for more information on building
# a YAML AST.
#
# ==== Writing to a string
#
#   # We need Psych::Nodes::Stream (not Psych::Nodes::Document)
#   stream = Psych.parse_stream("---\n - a\n - b")
#
#   stream.to_yaml # => "---\n- a\n- b\n"
#
# ==== Writing to a file
#
#   # We need Psych::Nodes::Stream (not Psych::Nodes::Document)
#   stream = Psych.parse_stream(File.read('database.yml'))
#
#   File.open('database.yml', 'w') do |file|
#     file.write(stream.to_yaml)
#   end
#
# == Low-level API
#
# === Parsing
#
# The lowest level parser should be used when the YAML input is already known,
# and the developer does not want to pay the price of building an AST or
# automatic detection and conversion to Ruby objects.  See Psych::Parser for
# more information on using the event based parser.
#
# ==== Reading to Psych::Nodes::Stream structure
#
#   parser = Psych::Parser.new(TreeBuilder.new) # => #<Psych::Parser>
#   parser = Psych.parser                       # it's an alias for the above
#
#   parser.parse("---\n - a\n - b")             # => #<Psych::Parser>
#   parser.handler                              # => #<Psych::TreeBuilder>
#   parser.handler.root                         # => #<Psych::Nodes::Stream>
#
# ==== Receiving an events stream
#
#   recorder = Psych::Handlers::Recorder.new
#   parser = Psych::Parser.new(recorder)
#
#   parser.parse("---\n - a\n - b")
#   recorder.events # => [list of [event, args] lists]
#                   # event is one of: Psych::Handler::EVENTS
#                   # args are the arguments passed to the event
#
# === Emitting
#
# The lowest level emitter is an event based system.  Events are sent to a
# Psych::Emitter object.  That object knows how to convert the events to a YAML
# document.  This interface should be used when document format is known in
# advance or speed is a concern.  See Psych::Emitter for more information.
#
# ==== Writing to a Ruby structure
#
#   Psych.parser.parse("--- a")       # => #<Psych::Parser>
#
#   parser.handler.first              # => #<Psych::Nodes::Stream>
#   parser.handler.first.to_ruby      # => ["a"]
#
#   parser.handler.root.first         # => #<Psych::Nodes::Document>
#   parser.handler.root.first.to_ruby # => "a"
#
#   # You can instantiate an Emitter manually
#   Psych::Visitors::ToRuby.new.accept(parser.handler.root.first)
#   # => "a"

module Psych
  # The version of libyaml Psych is using
  LIBYAML_VERSION = T.let(T.unsafe(nil), String)
  # Deprecation guard
  NOT_GIVEN = T.let(T.unsafe(nil), Object)

  ###
  # Load +yaml+ in to a Ruby data structure.  If multiple documents are
  # provided, the object contained in the first document will be returned.
  # +filename+ will be used in the exception message if any exception
  # is raised while parsing.  If +yaml+ is empty, it returns
  # the specified +fallback+ return value, which defaults to +false+.
  #
  # Raises a Psych::SyntaxError when a YAML syntax error is detected.
  #
  # Example:
  #
  #   Psych.load("--- a")             # => 'a'
  #   Psych.load("---\n - a\n - b")   # => ['a', 'b']
  #
  #   begin
  #     Psych.load("--- `", filename: "file.txt")
  #   rescue Psych::SyntaxError => ex
  #     ex.file    # => 'file.txt'
  #     ex.message # => "(file.txt): found character that cannot start any token"
  #   end
  #
  # When the optional +symbolize_names+ keyword argument is set to a
  # true value, returns symbols for keys in Hash objects (default: strings).
  #
  #   Psych.load("---\n foo: bar")                         # => {"foo"=>"bar"}
  #   Psych.load("---\n foo: bar", symbolize_names: true)  # => {:foo=>"bar"}
  #
  # Raises a TypeError when `yaml` parameter is NilClass
  #
  # NOTE: This method *should not* be used to parse untrusted documents, such as
  # YAML documents that are supplied via user input.  Instead, please use the
  # safe_load method.
  #
  sig do
    params(
      yaml: T.any(String, StringIO, IO),
      legacy_filename: Object,
      filename: T.nilable(String),
      fallback: T.untyped,
      symbolize_names: T::Boolean,
    )
    .returns(T.untyped)
  end
  def self.load(yaml, legacy_filename = T.unsafe(nil), filename: T.unsafe(nil), fallback: T.unsafe(nil), symbolize_names: T.unsafe(nil)); end

  ###
  # Safely load the yaml string in +yaml+.  By default, only the following
  # classes are allowed to be deserialized:
  #
  # * TrueClass
  # * FalseClass
  # * NilClass
  # * Numeric
  # * String
  # * Array
  # * Hash
  #
  # Recursive data structures are not allowed by default.  Arbitrary classes
  # can be allowed by adding those classes to the +permitted_classes+ keyword argument.  They are
  # additive.  For example, to allow Date deserialization:
  #
  #   Psych.safe_load(yaml, permitted_classes: [Date])
  #
  # Now the Date class can be loaded in addition to the classes listed above.
  #
  # Aliases can be explicitly allowed by changing the +aliases+ keyword argument.
  # For example:
  #
  #   x = []
  #   x << x
  #   yaml = Psych.dump x
  #   Psych.safe_load yaml               # => raises an exception
  #   Psych.safe_load yaml, aliases: true # => loads the aliases
  #
  # A Psych::DisallowedClass exception will be raised if the yaml contains a
  # class that isn't in the +permitted_classes+ list.
  #
  # A Psych::BadAlias exception will be raised if the yaml contains aliases
  # but the +aliases+ keyword argument is set to false.
  #
  # +filename+ will be used in the exception message if any exception is raised
  # while parsing.
  #
  # When the optional +symbolize_names+ keyword argument is set to a
  # true value, returns symbols for keys in Hash objects (default: strings).
  #
  #   Psych.safe_load("---\n foo: bar")                         # => {"foo"=>"bar"}
  #   Psych.safe_load("---\n foo: bar", symbolize_names: true)  # => {:foo=>"bar"}
  #
  sig do
    params(
      yaml: T.any(String, StringIO, IO),
      legacy_permitted_classes: Object,
      legacy_permitted_symbols: Object,
      legacy_aliases: Object,
      legacy_filename: Object,
      permitted_classes: T::Array[Class],
      permitted_symbols: T::Array[Symbol],
      aliases: T::Boolean,
      filename: T.nilable(String),
      fallback: T.untyped,
      symbolize_names: T::Boolean,
    )
    .returns(T.untyped)
  end
  def self.safe_load(yaml, legacy_permitted_classes = T.unsafe(nil), legacy_permitted_symbols = T.unsafe(nil), legacy_aliases = T.unsafe(nil), legacy_filename = T.unsafe(nil), permitted_classes: T.unsafe(nil), permitted_symbols: T.unsafe(nil), aliases: T.unsafe(nil), filename: T.unsafe(nil), fallback: T.unsafe(nil), symbolize_names: T.unsafe(nil)); end

  ###
  # Parse a YAML string in +yaml+.  Returns the Psych::Nodes::Document.
  # +filename+ is used in the exception message if a Psych::SyntaxError is
  # raised.
  #
  # Raises a Psych::SyntaxError when a YAML syntax error is detected.
  #
  # Example:
  #
  #   Psych.parse("---\n - a\n - b") # => #<Psych::Nodes::Document:0x00>
  #
  #   begin
  #     Psych.parse("--- `", filename: "file.txt")
  #   rescue Psych::SyntaxError => ex
  #     ex.file    # => 'file.txt'
  #     ex.message # => "(file.txt): found character that cannot start any token"
  #   end
  #
  # See Psych::Nodes for more information about YAML AST.
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
  # Parse a file at +filename+. Returns the Psych::Nodes::Document.
  #
  # Raises a Psych::SyntaxError when a YAML syntax error is detected.
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
  # Parse a YAML string in +yaml+.  Returns the Psych::Nodes::Stream.
  # This method can handle multiple YAML documents contained in +yaml+.
  # +filename+ is used in the exception message if a Psych::SyntaxError is
  # raised.
  #
  # If a block is given, a Psych::Nodes::Document node will be yielded to the
  # block as it's being parsed.
  #
  # Raises a Psych::SyntaxError when a YAML syntax error is detected.
  #
  # Example:
  #
  #   Psych.parse_stream("---\n - a\n - b") # => #<Psych::Nodes::Stream:0x00>
  #
  #   Psych.parse_stream("--- a\n--- b") do |node|
  #     node # => #<Psych::Nodes::Document:0x00>
  #   end
  #
  #   begin
  #     Psych.parse_stream("--- `", filename: "file.txt")
  #   rescue Psych::SyntaxError => ex
  #     ex.file    # => 'file.txt'
  #     ex.message # => "(file.txt): found character that cannot start any token"
  #   end
  #
  # Raises a TypeError when NilClass is passed.
  #
  # See Psych::Nodes for more information about YAML AST.
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
  # call-seq:
  #   Psych.dump(o)               -> string of yaml
  #   Psych.dump(o, options)      -> string of yaml
  #   Psych.dump(o, io)           -> io object passed in
  #   Psych.dump(o, io, options)  -> io object passed in
  #
  # Dump Ruby object +o+ to a YAML string.  Optional +options+ may be passed in
  # to control the output format.  If an IO object is passed in, the YAML will
  # be dumped to that IO object.
  #
  # Currently supported options are:
  #
  # [<tt>:indentation</tt>]   Number of space characters used to indent.
  #                           Acceptable value should be in <tt>0..9</tt> range,
  #                           otherwise option is ignored.
  #
  #                           Default: <tt>2</tt>.
  # [<tt>:line_width</tt>]    Max character to wrap line at.
  #
  #                           Default: <tt>0</tt> (meaning "wrap at 81").
  # [<tt>:canonical</tt>]     Write "canonical" YAML form (very verbose, yet
  #                           strictly formal).
  #
  #                           Default: <tt>false</tt>.
  # [<tt>:header</tt>]        Write <tt>%YAML [version]</tt> at the beginning of document.
  #
  #                           Default: <tt>false</tt>.
  #
  # Example:
  #
  #   # Dump an array, get back a YAML string
  #   Psych.dump(['a', 'b'])  # => "---\n- a\n- b\n"
  #
  #   # Dump an array to an IO object
  #   Psych.dump(['a', 'b'], StringIO.new)  # => #<StringIO:0x000001009d0890>
  #
  #   # Dump an array with indentation set
  #   Psych.dump(['a', ['b']], indentation: 3) # => "---\n- a\n-  - b\n"
  #
  #   # Dump an array to an IO with indentation set
  #   Psych.dump(['a', ['b']], StringIO.new, indentation: 3)
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
  #   Psych.dump_stream("foo\n  ", {}) # => "--- ! \"foo\\n  \"\n--- {}\n"
  sig { params(objects: T.untyped).returns(T.untyped) }
  def self.dump_stream(*objects); end

  ###
  # Dump Ruby +object+ to a JSON string.
  sig { params(object: T.untyped).returns(String) }
  def self.to_json(object); end

  ###
  # Load multiple documents given in +yaml+.  Returns the parsed documents
  # as a list.  If a block is given, each document will be converted to Ruby
  # and passed to the block during parsing
  #
  # Example:
  #
  #   Psych.load_stream("--- foo\n...\n--- bar\n...") # => ['foo', 'bar']
  #
  #   list = []
  #   Psych.load_stream("--- foo\n...\n--- bar\n...") do |ruby|
  #     list << ruby
  #   end
  #   list # => ['foo', 'bar']
  #
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
  # Load the document contained in +filename+.  Returns the yaml contained in
  # +filename+ as a Ruby object, or if the file is empty, it returns
  # the specified +fallback+ return value, which defaults to +false+.
  sig { params(filename: T.any(String, Pathname), fallback: T.untyped).returns(T.untyped) }
  def self.load_file(filename, fallback: false); end
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

class Psych::Coder

  def [](k); end


  def []=(k, v); end


  def add(k, v); end


  def implicit(); end


  def implicit=(implicit); end


  def initialize(tag); end


  def map(tag=T.unsafe(nil), style=T.unsafe(nil)); end


  def map=(map); end


  def object(); end


  def object=(object); end


  def represent_map(tag, map); end


  def represent_object(tag, obj); end


  def represent_scalar(tag, value); end


  def represent_seq(tag, list); end


  def scalar(*args); end


  def scalar=(value); end


  def seq(); end


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

  def alias(_); end


  def canonical(); end


  def canonical=(canonical); end


  def end_document(_); end


  def end_mapping(); end


  def end_sequence(); end


  def end_stream(); end


  def indentation(); end


  def indentation=(indentation); end


  def initialize(*_); end


  def line_width(); end


  def line_width=(line_width); end


  def scalar(_, _1, _2, _3, _4, _5); end


  def start_document(_, _1, _2); end


  def start_mapping(_, _1, _2, _3); end


  def start_sequence(_, _1, _2, _3); end


  def start_stream(_); end
end

class Psych::Handler
  EVENTS = ::T.unsafe(nil)
  OPTIONS = ::T.unsafe(nil)


  def alias(anchor); end


  def empty(); end


  def end_document(implicit); end


  def end_mapping(); end


  def end_sequence(); end


  def end_stream(); end


  def event_location(start_line, start_column, end_line, end_column); end


  def scalar(value, anchor, tag, plain, quoted, style); end


  def start_document(version, tag_directives, implicit); end


  def start_mapping(anchor, tag, implicit, style); end


  def start_sequence(anchor, tag, implicit, style); end


  def start_stream(encoding); end


  def streaming?(); end
end

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

module Psych::Nodes
end

class Psych::Nodes::Alias < Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)


  def alias?(); end


  def anchor(); end


  def anchor=(anchor); end


  def initialize(anchor); end
end

class Psych::Nodes::Document < Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)


  def document?(); end


  def implicit(); end


  def implicit=(implicit); end


  def implicit_end(); end


  def implicit_end=(implicit_end); end


  def initialize(version=T.unsafe(nil), tag_directives=T.unsafe(nil), implicit=T.unsafe(nil)); end


  def root(); end


  def tag_directives(); end


  def tag_directives=(tag_directives); end


  def version(); end


  def version=(version); end
end

class Psych::Nodes::Mapping < Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)

  ANY = ::T.unsafe(nil)
  BLOCK = ::T.unsafe(nil)
  FLOW = ::T.unsafe(nil)


  def anchor(); end


  def anchor=(anchor); end


  def implicit(); end


  def implicit=(implicit); end


  def initialize(anchor=T.unsafe(nil), tag=T.unsafe(nil), implicit=T.unsafe(nil), style=T.unsafe(nil)); end


  def mapping?(); end


  def style(); end


  def style=(style); end


  def tag(); end


  def tag=(tag); end
end

class Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)


  def alias?(); end


  def children(); end


  def document?(); end


  def each(&block); end


  def end_column(); end


  def end_column=(end_column); end


  def end_line(); end


  def end_line=(end_line); end


  def initialize(); end


  def mapping?(); end


  def scalar?(); end


  def sequence?(); end


  def start_column(); end


  def start_column=(start_column); end


  def start_line(); end


  def start_line=(start_line); end


  def stream?(); end


  def tag(); end


  def to_ruby(); end


  def to_yaml(io=T.unsafe(nil), options=T.unsafe(nil)); end


  def transform(); end


  def yaml(io=T.unsafe(nil), options=T.unsafe(nil)); end
end

class Psych::Nodes::Scalar < Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)

  ANY = ::T.unsafe(nil)
  DOUBLE_QUOTED = ::T.unsafe(nil)
  FOLDED = ::T.unsafe(nil)
  LITERAL = ::T.unsafe(nil)
  PLAIN = ::T.unsafe(nil)
  SINGLE_QUOTED = ::T.unsafe(nil)


  def anchor(); end


  def anchor=(anchor); end


  def initialize(value, anchor=T.unsafe(nil), tag=T.unsafe(nil), plain=T.unsafe(nil), quoted=T.unsafe(nil), style=T.unsafe(nil)); end


  def plain(); end


  def plain=(plain); end


  def quoted(); end


  def quoted=(quoted); end


  def scalar?(); end


  def style(); end


  def style=(style); end


  def tag(); end


  def tag=(tag); end


  def value(); end


  def value=(value); end
end

class Psych::Nodes::Sequence < Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)

  ANY = ::T.unsafe(nil)
  BLOCK = ::T.unsafe(nil)
  FLOW = ::T.unsafe(nil)


  def anchor(); end


  def anchor=(anchor); end


  def implicit(); end


  def implicit=(implicit); end


  def initialize(anchor=T.unsafe(nil), tag=T.unsafe(nil), implicit=T.unsafe(nil), style=T.unsafe(nil)); end


  def sequence?(); end


  def style(); end


  def style=(style); end


  def tag(); end


  def tag=(tag); end
end

class Psych::Nodes::Stream < Psych::Nodes::Node
  include Enumerable
  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)

  ANY = ::T.unsafe(nil)
  UTF16BE = ::T.unsafe(nil)
  UTF16LE = ::T.unsafe(nil)
  UTF8 = ::T.unsafe(nil)


  def encoding(); end


  def encoding=(encoding); end


  def initialize(encoding=T.unsafe(nil)); end


  def stream?(); end
end

class Psych::Omap < Hash
  include Enumerable
  extend T::Generic
  K = type_member(:out)
  V = type_member(:out)
  Elem = type_member(:out, fixed: T.untyped)
end

class Psych::Parser
  ANY = ::T.unsafe(nil)
  UTF16BE = ::T.unsafe(nil)
  UTF16LE = ::T.unsafe(nil)
  UTF8 = ::T.unsafe(nil)


  def external_encoding=(external_encoding); end


  def handler(); end


  def handler=(handler); end


  def initialize(handler=T.unsafe(nil)); end


  def mark(); end


  def parse(*_); end
end

class Psych::Parser::Mark

  def column(); end


  def column=(_); end


  def index(); end


  def index=(_); end


  def line(); end


  def line=(_); end
end

class Psych::ScalarScanner
  FLOAT = ::T.unsafe(nil)
  INTEGER = ::T.unsafe(nil)
  TIME = ::T.unsafe(nil)


  def class_loader(); end


  def initialize(class_loader); end


  def parse_int(string); end


  def parse_time(string); end


  def tokenize(string); end
end

class Psych::Set < Hash
  include Enumerable
  extend T::Generic
  K = type_member(:out)
  V = type_member(:out)
  Elem = type_member(:out, fixed: T.untyped)
end

class Psych::Stream < Psych::Visitors::YAMLTree
  include ::Psych::Streaming
  extend ::Psych::Streaming::ClassMethods
end

class Psych::Stream::Emitter < Psych::Emitter

  def end_document(implicit_end=T.unsafe(nil)); end


  def streaming?(); end
end

module Psych::Streaming

  def start(encoding=T.unsafe(nil)); end
end

module Psych::Streaming::ClassMethods

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

class Psych::TreeBuilder < Psych::Handler

  def alias(anchor); end


  def end_document(implicit_end=T.unsafe(nil)); end


  def end_mapping(); end


  def end_sequence(); end


  def end_stream(); end


  def event_location(start_line, start_column, end_line, end_column); end


  def initialize(); end


  def root(); end


  def scalar(value, anchor, tag, plain, quoted, style); end


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

class Psych::Visitors::YAMLTree < Psych::Visitors::Visitor

  def <<(object); end


  def accept(target); end


  def finish(); end


  def finished(); end


  def finished?(); end


  def initialize(emitter, ss, options); end


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


  def visit_Date(o); end


  def visit_DateTime(o); end


  def visit_Delegator(o); end


  def visit_Encoding(o); end


  def visit_Enumerator(o); end


  def visit_Exception(o); end


  def visit_FalseClass(o); end


  def visit_Float(o); end


  def visit_Hash(o); end


  def visit_Integer(o); end


  def visit_Module(o); end


  def visit_NameError(o); end


  def visit_NilClass(o); end


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
