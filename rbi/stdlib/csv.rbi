# typed: true

class CSV
  include(Enumerable)
  extend(Forwardable)

  DateMatcher = T.let(T.unsafe(nil), Regexp)

  DateTimeMatcher = T.let(T.unsafe(nil), Regexp)

  ConverterEncoding = T.let(T.unsafe(nil), Encoding)

  Converters = T.let(T.unsafe(nil), Hash)

  class Row
    include(Enumerable)
    extend(Forwardable)

    def <<(arg)
    end

    def delete(header_or_index, minimum_index = _)
    end

    def index(header, minimum_index = _)
    end

    def fields(*headers_and_or_indices)
    end

    def ==(other)
    end

    def header_row?()
    end

    def field_row?()
    end

    def to_s(**options)
    end

    def [](header_or_index, minimum_index = _)
    end

    def []=(*args)
    end

    def to_hash()
    end

    def field(header_or_index, minimum_index = _)
    end

    def has_key?(header)
    end

    def empty?(*args, &block)
    end

    def fetch(header, *varargs)
    end

    def push(*args)
    end

    def key?(header)
    end

    def field?(data)
    end

    def header?(name)
    end

    def include?(name)
    end

    def headers()
    end

    def to_csv(**options)
    end

    def member?(header)
    end

    def values_at(*headers_and_or_indices)
    end

    def delete_if(&block)
    end

    def inspect()
    end

    def length(*args, &block)
    end

    def size(*args, &block)
    end

    def each(&block)
    end
  end

  HeaderConverters = T.let(T.unsafe(nil), Hash)

  class Table
    include(Enumerable)
    extend(Forwardable)

    def <<(row_or_array)
    end

    def delete(index_or_header)
    end

    def ==(other)
    end

    def to_a()
    end

    def to_s(write_headers: _, **options)
    end

    def [](index_or_header)
    end

    def mode()
    end

    def []=(index_or_header, value)
    end

    def empty?(*args, &block)
    end

    def push(*rows)
    end

    def to_csv(write_headers: _, **options)
    end

    def headers()
    end

    def by_col()
    end

    def by_col!()
    end

    def by_col_or_row()
    end

    def by_col_or_row!()
    end

    def by_row()
    end

    def by_row!()
    end

    def values_at(*indices_or_headers)
    end

    def delete_if(&block)
    end

    def inspect()
    end

    def length(*args, &block)
    end

    def size(*args, &block)
    end

    def each(&block)
    end
  end

  DEFAULT_OPTIONS = T.let(T.unsafe(nil), Hash)

  VERSION = T.let(T.unsafe(nil), String)

  class MalformedCSVError < ::RuntimeError

  end

  class FieldInfo < ::Struct
    def index=(_)
    end

    def line()
    end

    def line=(_)
    end

    def header()
    end

    def header=(_)
    end

    def index()
    end
  end

  def field_size_limit()
  end

  def converters()
  end

  def quote_char()
  end

  def lineno()
  end

  def header_converters()
  end

  def sync=(*args, &block)
  end

  def string(*args, &block)
  end

  def skip_lines()
  end

  def sync(*args, &block)
  end

  def seek(*args, &block)
  end

  def tell(*args, &block)
  end

  def <<(row)
  end

  def rewind()
  end

  def pos(*args, &block)
  end

  def pos=(*args, &block)
  end

  def eof(*args, &block)
  end

  def eof?(*args, &block)
  end

  def close(*args, &block)
  end

  def closed?(*args, &block)
  end

  def close_read(*args, &block)
  end

  def close_write(*args, &block)
  end

  def isatty(*args, &block)
  end

  def tty?(*args, &block)
  end

  def binmode?(*args, &block)
  end

  def headers()
  end

  def ioctl(*args, &block)
  end

  def fcntl(*args, &block)
  end

  def pid(*args, &block)
  end

  def external_encoding(*args, &block)
  end

  def internal_encoding(*args, &block)
  end

  def header_convert(name = _, &converter)
  end

  def convert(name = _, &converter)
  end

  def unconverted_fields?()
  end

  def return_headers?()
  end

  def write_headers?()
  end

  def skip_blanks?()
  end

  def force_quotes?()
  end

  def liberal_parsing?()
  end

  def shift()
  end

  def inspect()
  end

  def add_row(row)
  end

  def gets()
  end

  def each()
  end

  def read()
  end

  def binmode(*args, &block)
  end

  def to_io(*args, &block)
  end

  def to_i(*args, &block)
  end

  def header_row?()
  end

  def flush(*args, &block)
  end

  def encoding()
  end

  def stat(*args, &block)
  end

  def truncate(*args, &block)
  end

  def puts(row)
  end

  def readline()
  end

  def readlines()
  end

  def line()
  end

  def flock(*args, &block)
  end

  def reopen(*args, &block)
  end

  def fileno(*args, &block)
  end

  def fsync(*args, &block)
  end

  def col_sep()
  end

  def row_sep()
  end

  def path(*args, &block)
  end
end
