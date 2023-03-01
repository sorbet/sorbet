# typed: __STDLIB_INTERNAL

# Pseudo I/O on [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# object, with interface corresponding to
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
#
# Commonly used to simulate `$stdio` or `$stderr`
#
# ### Examples
#
# ```ruby
# require 'stringio'
#
# # Writing stream emulation
# io = StringIO.new
# io.puts "Hello World"
# io.string #=> "Hello World\n"
#
# # Reading stream emulation
# io = StringIO.new "first\nsecond\nlast\n"
# io.getc #=> "f"
# io.gets #=> "irst\n"
# io.read #=> "second\nlast\n"
# ```
class StringIO
  include Enumerable
  extend T::Generic

  sig do
    params(
      string: String,
      mode: T.nilable(String)
    ).void
  end
  def initialize(string="", mode="rw"); end

  # Equivalent to
  # [`StringIO.new`](https://docs.ruby-lang.org/en/2.7.0/StringIO.html#method-c-new)
  # except that when it is called with a block, it yields with the new instance
  # and closes it, and returns the result which returned from the block.
  sig do
    type_parameters(:U)
    .params(
      string: String,
      mode: T.nilable(String),
      blk: T.proc.params(arg: StringIO).returns(T.type_parameter(:U)),
    ).returns(T.type_parameter(:U))
  end
  def self.open(string="", mode="rw", &blk); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.self_type)
  end
  def <<(arg0); end

  # Puts stream into binary mode. See
  # [`IO#binmode`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-binmode).
  sig {returns(T.self_type)}
  def binmode(); end

  # Closes a [`StringIO`](https://docs.ruby-lang.org/en/2.7.0/StringIO.html).
  # The stream is unavailable for any further data operations; an `IOError` is
  # raised if such an attempt is made.
  sig {returns(NilClass)}
  def close(); end

  # Closes the read end of a
  # [`StringIO`](https://docs.ruby-lang.org/en/2.7.0/StringIO.html). Will raise
  # an `IOError` if the receiver is not readable.
  sig {returns(NilClass)}
  def close_read(); end

  # Closes the write end of a
  # [`StringIO`](https://docs.ruby-lang.org/en/2.7.0/StringIO.html). Will raise
  # an  `IOError` if the receiver is not writeable.
  sig {returns(NilClass)}
  def close_write(); end

  # Returns `true` if the stream is completely closed, `false` otherwise.
  sig {returns(T::Boolean)}
  def closed?(); end

  # Returns `true` if the stream is not readable, `false` otherwise.
  sig {returns(T::Boolean)}
  def closed_read?(); end

  # Returns `true` if the stream is not writable, `false` otherwise.
  sig {returns(T::Boolean)}
  def closed_write?(); end

  # See [`IO#each`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each).
  sig do
    params(
        sep: String,
        limit: Integer,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(T::Enumerator[String])
  end
  def each(sep=T.unsafe(nil), limit=T.unsafe(nil), &blk); end

  # See
  # [`IO#each_byte`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_byte).
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def each_byte(&blk); end

  # See
  # [`IO#each_char`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_char).
  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[String])}
  def each_char(&blk); end

  # See
  # [`IO#each_codepoint`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_codepoint).
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def each_codepoint(&blk); end

  # Returns true if the stream is at the end of the data (underlying string).
  # The stream must be opened for reading or an `IOError` will be raised.
  sig {returns(T::Boolean)}
  def eof(); end

  # Raises
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.7.0/NotImplementedError.html).
  sig do
    params(
        integer_cmd: Integer,
        arg: T.any(String, Integer),
    )
    .returns(Integer)
  end
  def fcntl(integer_cmd, arg); end

  # Returns `nil`. Just for compatibility to
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  sig {returns(NilClass)}
  def fileno(); end

  # Returns an object itself. Just for compatibility to
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  sig {returns(T.self_type)}
  def flush(); end

  # Returns 0. Just for compatibility to
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  sig {returns(T.nilable(Integer))}
  def fsync(); end

  # See
  # [`IO#getbyte`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-getbyte).
  sig {returns(T.nilable(Integer))}
  def getbyte(); end

  # See [`IO#getc`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-getc).
  sig {returns(T.nilable(String))}
  def getc(); end

  # See [`IO#gets`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-gets).
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(T.nilable(String))
  end
  def gets(sep=T.unsafe(nil), limit=T.unsafe(nil)); end

  # Returns the [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html)
  # of the internal string if conversion is specified. Otherwise returns `nil`.
  sig {returns(Encoding)}
  def internal_encoding(); end

  # Returns the [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html)
  # object that represents the encoding of the file. If the stream is write mode
  # and no encoding is specified, returns `nil`.
  sig {returns(Encoding)}
  def external_encoding(); end

  # Returns `false`. Just for compatibility to
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  sig {returns(T::Boolean)}
  def isatty(); end

  # Returns the size of the buffer string.
  def length; end

  # Returns the current line number. The stream must be opened for reading.
  # `lineno` counts the number of times  `gets` is called, rather than the
  # number of newlines  encountered. The two values will differ if `gets` is
  # called with a separator other than newline. See also the  `$.` variable.
  sig {returns(Integer)}
  def lineno(); end

  # Manually sets the current line number to the given value. `$.` is updated
  # only on the next read.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def lineno=(arg0); end

  # Returns `nil`. Just for compatibility to
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  sig {returns(NilClass)}
  def pid(); end

  # Returns the current offset (in bytes).
  sig {returns(Integer)}
  def pos(); end

  # Seeks to the given position (in bytes).
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def pos=(arg0); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def print(*arg0); end

  sig do
    params(
        format_string: String,
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def printf(format_string, *arg0); end

  # See [`IO#putc`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-putc).
  sig do
    params(
        arg0: T.any(Numeric, String),
    )
    .returns(T.untyped)
  end
  def putc(arg0); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def puts(*arg0); end

  # See [`IO#read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read).
  sig do
    params(
        length: Integer,
        outbuf: String,
    )
    .returns(T.nilable(String))
  end
  def read(length=T.unsafe(nil), outbuf=T.unsafe(nil)); end

  sig do
    params(
        len: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        len: Integer,
        buf: String,
    )
    .returns(String)
  end
  def read_nonblock(len, buf=T.unsafe(nil)); end

  sig {returns(Integer)}
  def readbyte(); end

  sig {returns(String)}
  def readchar(); end

  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(String)
  end
  def readline(sep=T.unsafe(nil), limit=T.unsafe(nil)); end

  # See
  # [`IO#readlines`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-readlines).
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(T::Array[String])
  end
  def readlines(sep=T.unsafe(nil), limit=T.unsafe(nil)); end

  sig do
    params(
        maxlen: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        maxlen: Integer,
        outbuf: String,
    )
    .returns(String)
  end
  def readpartial(maxlen, outbuf=T.unsafe(nil)); end

  # Reinitializes the stream with the given *other\_StrIO* or *string* and
  # *mode* (see StringIO#new).
  sig do
    params(
        other: StringIO,
    )
    .returns(T.self_type)
  end
  sig do
    params(
        other: String,
        mode_str: String,
    )
    .returns(T.self_type)
  end
  def reopen(other, mode_str=T.unsafe(nil)); end

  # Positions the stream to the beginning of input, resetting `lineno` to zero.
  sig {returns(Integer)}
  def rewind(); end

  # Seeks to a given offset *amount* in the stream according to the value of
  # *whence* (see
  # [`IO#seek`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-seek)).
  sig do
    params(
        amount: Integer,
        whence: Integer,
    )
    .returns(Integer)
  end
  def seek(amount, whence=T.unsafe(nil)); end

  # Specify the encoding of the
  # [`StringIO`](https://docs.ruby-lang.org/en/2.7.0/StringIO.html) as
  # *ext\_enc*. Use the default external encoding if *ext\_enc* is nil. 2nd
  # argument *int\_enc* and optional hash *opt* argument are ignored; they are
  # for API compatibility to
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  sig do
    params(
        ext_or_ext_int_enc: T.any(String, Encoding),
    )
    .returns(T.self_type)
  end
  sig do
    params(
        ext_or_ext_int_enc: T.any(String, Encoding),
        int_enc: T.any(String, Encoding),
    )
    .returns(T.self_type)
  end
  def set_encoding(ext_or_ext_int_enc=T.unsafe(nil), int_enc=T.unsafe(nil)); end

  # Returns underlying
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) object, the
  # subject of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  sig {returns(String)}
  def string(); end

  # Changes underlying
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) object, the
  # subject of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  sig {params(str: String).returns(String)}
  def string=(str); end

  # Returns the size of the buffer string.
  sig {returns(Integer)}
  def size(); end

  # Returns `true` always.
  sig {returns(T::Boolean)}
  def sync(); end

  # Returns the argument unchanged. Just for compatibility to
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  sig do
    params(
        arg0: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def sync=(arg0); end

  sig do
    params(
        maxlen: Integer,
        outbuf: String,
    )
    .returns(String)
  end
  def sysread(maxlen, outbuf); end

  sig do
    params(
        arg0: String,
    )
    .returns(Integer)
  end
  def syswrite(arg0); end

  # Returns the current offset (in bytes).
  sig {returns(Integer)}
  def tell(); end

  # Truncates the buffer string to at most *integer* bytes. The stream must be
  # opened for writing.
  def truncate(_); end

  # Returns `false`. Just for compatibility to
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  sig {returns(T::Boolean)}
  def tty?(); end

  # See
  # [`IO#ungetbyte`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-ungetbyte)
  sig do
    params(
        arg0: T.any(String, Integer),
    )
    .returns(NilClass)
  end
  def ungetbyte(arg0); end

  # Pushes back one character (passed as a parameter) such that a subsequent
  # buffered read will return it. There is no limitation for multiple pushbacks
  # including pushing back behind the beginning of the buffer string.
  sig do
    params(
        arg0: String,
    )
    .returns(NilClass)
  end
  def ungetc(arg0); end

  # Appends the given string to the underlying buffer string. The stream must be
  # opened for writing. If the argument is not a string, it will be converted to
  # a string using `to_s`. Returns the number of bytes written. See
  # [`IO#write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write).
  sig do
    params(
        arg0: String,
    )
    .returns(Integer)
  end
  def write(arg0); end

  # See [`IO#each`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each).
  sig do
    params(
        sep: String,
        limit: Integer,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(T::Enumerator[String])
  end
  def each_line(sep=T.unsafe(nil), limit=T.unsafe(nil), &blk); end

  # Returns true if the stream is at the end of the data (underlying string).
  # The stream must be opened for reading or an `IOError` will be raised.
  sig {returns(T::Boolean)}
  def eof?(); end
end

# Pseudo I/O on [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
# object, with interface corresponding to
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
#
# Commonly used to simulate `$stdio` or `$stderr`
#
# ### Examples
#
# ```ruby
# require 'stringio'
#
# # Writing stream emulation
# io = StringIO.new
# io.puts "Hello World"
# io.string #=> "Hello World\n"
#
# # Reading stream emulation
# io = StringIO.new "first\nsecond\nlast\n"
# io.getc #=> "f"
# io.gets #=> "irst\n"
# io.read #=> "second\nlast\n"
# ```
class StringIO < Data
  include Enumerable

  extend T::Generic
  Elem = type_member(:out) {{fixed: String}}
end
