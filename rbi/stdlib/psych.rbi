# typed: true

module Psych
  module Streaming
    module ClassMethods
      def new(io)
      end
    end

    def start(encoding = _)
    end
  end

  class Set < ::Hash

  end

  class Parser
    class Mark

    end

    ANY = T.let(T.unsafe(nil), Integer)

    UTF8 = T.let(T.unsafe(nil), Integer)

    UTF16LE = T.let(T.unsafe(nil), Integer)

    UTF16BE = T.let(T.unsafe(nil), Integer)

    def external_encoding=(_)
    end

    def handler=(_)
    end

    def parse(*_)
    end

    def mark()
    end

    def handler()
    end
  end

  class BadAlias < ::Psych::Exception

  end

  class DisallowedClass < ::Psych::Exception

  end

  class Exception < ::RuntimeError

  end

  module Visitors
    class YAMLTree < ::Psych::Visitors::Visitor
      class Registrar
        def id_for(target)
        end

        def register(target, node)
        end

        def node_for(target)
        end

        def key?(target)
        end
      end

      def tree()
      end

      def accept(target)
      end

      def <<(object)
      end

      def finished()
      end

      def push(object)
      end

      def finish()
      end

      def start(encoding = _)
      end

      def started()
      end

      def finished?()
      end

      def started?()
      end

      def visit_Psych_Omap(o)
      end

      def visit_Hash(o)
      end

      def visit_Encoding(o)
      end

      def visit_Object(o)
      end

      def visit_Delegator(o)
      end

      def visit_Struct(o)
      end

      def visit_Exception(o)
      end

      def visit_NameError(o)
      end

      def visit_Regexp(o)
      end

      def visit_DateTime(o)
      end

      def visit_Time(o)
      end

      def visit_Rational(o)
      end

      def visit_Complex(o)
      end

      def visit_Integer(o)
      end

      def visit_TrueClass(o)
      end

      def visit_FalseClass(o)
      end

      def visit_Date(o)
      end

      def visit_Float(o)
      end

      def visit_BigDecimal(o)
      end

      def visit_String(o)
      end

      def visit_Module(o)
      end

      def visit_Class(o)
      end

      def visit_Range(o)
      end

      def visit_Psych_Set(o)
      end

      def visit_Array(o)
      end

      def visit_Enumerator(o)
      end

      def visit_NilClass(o)
      end

      def visit_Symbol(o)
      end

      def visit_BasicObject(o)
      end
    end

    class Emitter < ::Psych::Visitors::Visitor
      def visit_Psych_Nodes_Scalar(o)
      end

      def visit_Psych_Nodes_Sequence(o)
      end

      def visit_Psych_Nodes_Mapping(o)
      end

      def visit_Psych_Nodes_Document(o)
      end

      def visit_Psych_Nodes_Stream(o)
      end

      def visit_Psych_Nodes_Alias(o)
      end
    end

    class JSONTree < ::Psych::Visitors::YAMLTree
      include(Psych::JSON::RubyEvents)

      def accept(target)
      end
    end

    class DepthFirst < ::Psych::Visitors::Visitor

    end

    class ToRuby < ::Psych::Visitors::Visitor
      SHOVEL = T.let(T.unsafe(nil), String)

      def accept(target)
      end

      def visit_Psych_Nodes_Scalar(o)
      end

      def class_loader()
      end

      def visit_Psych_Nodes_Sequence(o)
      end

      def visit_Psych_Nodes_Mapping(o)
      end

      def visit_Psych_Nodes_Document(o)
      end

      def visit_Psych_Nodes_Stream(o)
      end

      def visit_Psych_Nodes_Alias(o)
      end
    end

    class NoAliasRuby < ::Psych::Visitors::ToRuby
      def visit_Psych_Nodes_Alias(o)
      end
    end

    class Visitor
      DISPATCH = T.let(T.unsafe(nil), Hash)

      def accept(target)
      end
    end
  end

  class Emitter < ::Psych::Handler
    def start_stream(_)
    end

    def end_stream()
    end

    def start_document(_, _, _)
    end

    def end_document(_)
    end

    def alias(_)
    end

    def scalar(_, _, _, _, _, _)
    end

    def start_sequence(_, _, _, _)
    end

    def end_sequence()
    end

    def start_mapping(_, _, _, _)
    end

    def end_mapping()
    end

    def canonical()
    end

    def canonical=(_)
    end

    def indentation()
    end

    def indentation=(_)
    end

    def line_width()
    end

    def line_width=(_)
    end
  end

  class Coder
    def implicit()
    end

    def map=(map)
    end

    def implicit=(_)
    end

    def type()
    end

    def style()
    end

    def scalar(*args)
    end

    def [](k)
    end

    def []=(k, v)
    end

    def tag=(_)
    end

    def style=(_)
    end

    def map(tag = _, style = _)
    end

    def represent_scalar(tag, value)
    end

    def represent_seq(tag, list)
    end

    def represent_map(tag, map)
    end

    def represent_object(tag, obj)
    end

    def object=(_)
    end

    def object()
    end

    def scalar=(value)
    end

    def add(k, v)
    end

    def tag()
    end

    def seq()
    end

    def seq=(list)
    end
  end

  module JSON
    module YAMLEvents
      def start_mapping(anchor, tag, implicit, style)
      end

      def start_document(version, tag_directives, implicit)
      end

      def end_document(implicit_end = _)
      end

      def scalar(value, anchor, tag, plain, quoted, style)
      end

      def start_sequence(anchor, tag, implicit, style)
      end
    end

    class TreeBuilder < ::Psych::TreeBuilder
      include(Psych::JSON::YAMLEvents)
    end

    module RubyEvents
      def visit_DateTime(o)
      end

      def visit_Symbol(o)
      end

      def visit_Time(o)
      end

      def visit_String(o)
      end
    end

    class Stream < ::Psych::Visitors::JSONTree
      include(Psych::Streaming)
      extend(Psych::Streaming::ClassMethods)

      class Emitter < ::Psych::Stream::Emitter
        include(Psych::JSON::YAMLEvents)
      end
    end
  end

  VERSION = T.let(T.unsafe(nil), String)

  LIBYAML_VERSION = T.let(T.unsafe(nil), String)

  class FALLBACK < ::Struct
    def to_ruby()
    end

    def to_ruby=(_)
    end
  end

  class Stream < ::Psych::Visitors::YAMLTree
    include(Psych::Streaming)
    extend(Psych::Streaming::ClassMethods)

    class Emitter < ::Psych::Emitter
      def streaming?()
      end

      def end_document(implicit_end = _)
      end
    end
  end

  module Handlers
    class DocumentStream < ::Psych::TreeBuilder
      def end_document(implicit_end = _)
      end

      def start_document(version, tag_directives, implicit)
      end
    end
  end

  class Handler
    class DumperOptions
      def canonical()
      end

      def canonical=(_)
      end

      def indentation()
      end

      def indentation=(_)
      end

      def line_width()
      end

      def line_width=(_)
      end
    end

    OPTIONS = T.let(T.unsafe(nil), Psych::Handler::DumperOptions)

    EVENTS = T.let(T.unsafe(nil), Array)

    def empty()
    end

    def start_stream(encoding)
    end

    def end_stream()
    end

    def start_document(version, tag_directives, implicit)
    end

    def end_document(implicit)
    end

    def alias(anchor)
    end

    def scalar(value, anchor, tag, plain, quoted, style)
    end

    def start_sequence(anchor, tag, implicit, style)
    end

    def end_sequence()
    end

    def start_mapping(anchor, tag, implicit, style)
    end

    def end_mapping()
    end

    def event_location(start_line, start_column, end_line, end_column)
    end

    def streaming?()
    end
  end

  class ClassLoader
    REGEXP = T.let(T.unsafe(nil), String)

    BIG_DECIMAL = T.let(T.unsafe(nil), String)

    COMPLEX = T.let(T.unsafe(nil), String)

    DATE = T.let(T.unsafe(nil), String)

    DATE_TIME = T.let(T.unsafe(nil), String)

    EXCEPTION = T.let(T.unsafe(nil), String)

    OBJECT = T.let(T.unsafe(nil), String)

    PSYCH_OMAP = T.let(T.unsafe(nil), String)

    PSYCH_SET = T.let(T.unsafe(nil), String)

    RANGE = T.let(T.unsafe(nil), String)

    RATIONAL = T.let(T.unsafe(nil), String)

    STRUCT = T.let(T.unsafe(nil), String)

    SYMBOL = T.let(T.unsafe(nil), String)

    CACHE = T.let(T.unsafe(nil), Hash)

    class Restricted < ::Psych::ClassLoader
      def symbolize(sym)
      end
    end

    def complex()
    end

    def date_time()
    end

    def big_decimal()
    end

    def psych_omap()
    end

    def object()
    end

    def rational()
    end

    def psych_set()
    end

    def struct()
    end

    def load(klassname)
    end

    def date()
    end

    def range()
    end

    def symbol()
    end

    def exception()
    end

    def symbolize(sym)
    end

    def regexp()
    end
  end

  class ScalarScanner
    INTEGER = T.let(T.unsafe(nil), Regexp)

    TIME = T.let(T.unsafe(nil), Regexp)

    FLOAT = T.let(T.unsafe(nil), Regexp)

    def parse_int(string)
    end

    def class_loader()
    end

    def tokenize(string)
    end

    def parse_time(string)
    end
  end

  class SyntaxError < ::Psych::Exception
    def file()
    end

    def line()
    end

    def column()
    end

    def offset()
    end

    def context()
    end

    def problem()
    end
  end

  class TreeBuilder < ::Psych::Handler
    def root()
    end

    def start_stream(encoding)
    end

    def end_stream()
    end

    def start_document(version, tag_directives, implicit)
    end

    def end_document(implicit_end = _)
    end

    def alias(anchor)
    end

    def scalar(value, anchor, tag, plain, quoted, style)
    end

    def start_sequence(anchor, tag, implicit, style)
    end

    def end_sequence()
    end

    def start_mapping(anchor, tag, implicit, style)
    end

    def end_mapping()
    end

    def event_location(start_line, start_column, end_line, end_column)
    end
  end

  class Omap < ::Hash

  end

  module Nodes
    class Document < ::Psych::Nodes::Node
      def tag_directives()
      end

      def implicit()
      end

      def implicit_end()
      end

      def root()
      end

      def tag_directives=(_)
      end

      def implicit=(_)
      end

      def implicit_end=(_)
      end

      def version()
      end

      def version=(_)
      end
    end

    class Alias < ::Psych::Nodes::Node
      def anchor()
      end

      def anchor=(_)
      end
    end

    class Scalar < ::Psych::Nodes::Node
      LITERAL = T.let(T.unsafe(nil), Integer)

      FOLDED = T.let(T.unsafe(nil), Integer)

      ANY = T.let(T.unsafe(nil), Integer)

      PLAIN = T.let(T.unsafe(nil), Integer)

      SINGLE_QUOTED = T.let(T.unsafe(nil), Integer)

      DOUBLE_QUOTED = T.let(T.unsafe(nil), Integer)

      def anchor()
      end

      def style()
      end

      def anchor=(_)
      end

      def tag=(_)
      end

      def value()
      end

      def style=(_)
      end

      def tag()
      end

      def quoted()
      end

      def value=(_)
      end

      def plain()
      end

      def plain=(_)
      end

      def quoted=(_)
      end
    end

    class Sequence < ::Psych::Nodes::Node
      ANY = T.let(T.unsafe(nil), Integer)

      BLOCK = T.let(T.unsafe(nil), Integer)

      FLOW = T.let(T.unsafe(nil), Integer)

      def anchor=(_)
      end

      def tag()
      end

      def tag=(_)
      end

      def style=(_)
      end

      def implicit()
      end

      def implicit=(_)
      end

      def anchor()
      end

      def style()
      end
    end

    class Stream < ::Psych::Nodes::Node
      ANY = T.let(T.unsafe(nil), Integer)

      UTF8 = T.let(T.unsafe(nil), Integer)

      UTF16LE = T.let(T.unsafe(nil), Integer)

      UTF16BE = T.let(T.unsafe(nil), Integer)

      def encoding()
      end

      def encoding=(_)
      end
    end

    class Mapping < ::Psych::Nodes::Node
      ANY = T.let(T.unsafe(nil), Integer)

      BLOCK = T.let(T.unsafe(nil), Integer)

      FLOW = T.let(T.unsafe(nil), Integer)

      def anchor=(_)
      end

      def tag()
      end

      def tag=(_)
      end

      def style=(_)
      end

      def implicit()
      end

      def implicit=(_)
      end

      def anchor()
      end

      def style()
      end
    end

    class Node
      include(Enumerable)

      def start_line()
      end

      def start_column()
      end

      def end_line()
      end

      def end_column()
      end

      def transform()
      end

      def children()
      end

      def yaml(io = _, options = _)
      end

      def tag()
      end

      def to_ruby()
      end

      def start_line=(_)
      end

      def start_column=(_)
      end

      def end_line=(_)
      end

      def each(&block)
      end

      def end_column=(_)
      end

      def to_yaml(io = _, options = _)
      end
    end
  end
end
