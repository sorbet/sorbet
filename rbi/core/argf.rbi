# typed: __STDLIB_INTERNAL

# `ARGF` is a stream designed for use in scripts that process files given as
# command-line arguments or passed in via STDIN.
#
# The arguments passed to your script are stored in the `ARGV`
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html), one argument per
# element. `ARGF` assumes that any arguments that aren't filenames have been
# removed from `ARGV`. For example:
#
# ```
# $ ruby argf.rb --verbose file1 file2
#
# ARGV  #=> ["--verbose", "file1", "file2"]
# option = ARGV.shift #=> "--verbose"
# ARGV  #=> ["file1", "file2"]
# ```
#
# You can now use `ARGF` to work with a concatenation of each of these named
# files. For instance, `ARGF.read` will return the contents of *file1* followed
# by the contents of *file2*.
#
# After a file in `ARGV` has been read `ARGF` removes it from the
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html). Thus, after all
# files have been read `ARGV` will be empty.
#
# You can manipulate `ARGV` yourself to control what `ARGF` operates on. If you
# remove a file from `ARGV`, it is ignored by `ARGF`; if you add files to
# `ARGV`, they are treated as if they were named on the command line. For
# example:
#
# ```ruby
# ARGV.replace ["file1"]
# ARGF.readlines # Returns the contents of file1 as an Array
# ARGV           #=> []
# ARGV.replace ["file2", "file3"]
# ARGF.read      # Returns the contents of file2 and file3
# ```
#
# If `ARGV` is empty, `ARGF` acts as if it contained STDIN, i.e. the data piped
# to your script. For example:
#
# ```
# $ echo "glark" | ruby -e 'p ARGF.read'
# "glark\n"
# ```
module ARGF
  include Enumerable

  extend T::Generic
  Elem = type_member {{fixed: String}}

  # ARGF.to_s  -> String
  #
  # Returns "ARGF".
  sig { returns(String) }
  def self.to_s(); end

  # ARGF.argv  -> ARGV
  #
  # Returns the +ARGV+ array, which contains the arguments passed to your
  # script, one per element.
  #
  # For example:
  #
  #     $ ruby argf.rb -v glark.txt
  #
  #     ARGF.argv   #=> ["-v", "glark.txt"]
  sig { returns(T::Array[String]) }
  def self.argv(); end

  # ARGF.fileno    -> integer
  #
  # Returns an integer representing the numeric file descriptor for
  # the current file. Raises an +ArgumentError+ if there isn't a current file.
  #
  #    ARGF.fileno    #=> 3
  sig { returns(Integer) }
  def self.fileno(); end

  # ARGF.to_i      -> integer
  #
  # Returns an integer representing the numeric file descriptor for
  # the current file. Raises an +ArgumentError+ if there isn't a current file.
  #
  #    ARGF.fileno    #=> 3
  sig { returns(Integer) }
  def self.to_i(); end

  # ARGF.to_io     -> IO
  #
  # Returns an +IO+ object representing the current file. This will be a
  # +File+ object unless the current file is a stream such as STDIN.
  #
  # For example:
  #
  #    ARGF.to_io    #=> #<File:glark.txt>
  #    ARGF.to_io    #=> #<IO:<STDIN>>
  sig { returns(IO) }
  def self.to_io(); end

  # ARGF.to_write_io  -> io
  #
  # Returns IO instance tied to _ARGF_ for writing if inplace mode is
  # enabled.
  sig { returns(IO) }
  def self.to_write_io(); end

  # ARGF.each(sep=$/)             {|line| block }  -> ARGF
  # ARGF.each(sep=$/, limit)      {|line| block }  -> ARGF
  # ARGF.each(...)                                 -> an_enumerator
  # ARGF.each_line(sep=$/)        {|line| block }  -> ARGF
  # ARGF.each_line(sep=$/, limit) {|line| block }  -> ARGF
  # ARGF.each_line(...)                            -> an_enumerator
  #
  # Returns an enumerator which iterates over each line (separated by _sep_,
  # which defaults to your platform's newline character) of each file in
  # +ARGV+. If a block is supplied, each line in turn will be yielded to the
  # block, otherwise an enumerator is returned.
  # The optional _limit_ argument is an +Integer+ specifying the maximum
  # length of each line; longer lines will be split according to this limit.
  #
  # This method allows you to treat the files supplied on the command line as
  # a single file consisting of the concatenation of each named file. After
  # the last line of the first file has been returned, the first line of the
  # second file is returned. The +ARGF.filename+ and +ARGF.lineno+ methods can
  # be used to determine the filename and line number, respectively, of the
  # current line.
  #
  # For example, the following code prints out each line of each named file
  # prefixed with its line number, displaying the filename once per file:
  #
  #    ARGF.each_line do |line|
  #      puts ARGF.filename if ARGF.lineno == 1
  #      puts "#{ARGF.lineno}: #{line}"
  #    end
  sig do
    params(
      blk: T.proc.params(arg0: String).returns(BasicObject),
    )
      .returns(T::Array[String])
  end
  sig {returns(T::Enumerator[String])}
  def self.each(&blk); end

  # ARGF.each_line(sep=$/)        {|line| block }  -> ARGF
  # ARGF.each_line(sep=$/, limit) {|line| block }  -> ARGF
  # ARGF.each_line(...)                            -> an_enumerator
  #
  # Returns an enumerator which iterates over each line (separated by _sep_,
  # which defaults to your platform's newline character) of each file in
  # +ARGV+. If a block is supplied, each line in turn will be yielded to the
  # block, otherwise an enumerator is returned.
  # The optional _limit_ argument is an +Integer+ specifying the maximum
  # length of each line; longer lines will be split according to this limit.
  #
  # This method allows you to treat the files supplied on the command line as
  # a single file consisting of the concatenation of each named file. After
  # the last line of the first file has been returned, the first line of the
  # second file is returned. The +ARGF.filename+ and +ARGF.lineno+ methods can
  # be used to determine the filename and line number, respectively, of the
  # current line.
  #
  # For example, the following code prints out each line of each named file
  # prefixed with its line number, displaying the filename once per file:
  #
  #    ARGF.each_line do |line|
  #      puts ARGF.filename if ARGF.lineno == 1
  #      puts "#{ARGF.lineno}: #{line}"
  #    end
  sig {params(several_variants: T.any(Integer, String), blk: T.nilable(T.proc.params(arg: String).void)).returns(ARGF)}
  sig {returns(T::Enumerator[String])}
  def self.each_line(*several_variants, &blk); end

  # ARGF.bytes     {|byte| block }  -> ARGF
  # ARGF.bytes                      -> an_enumerator
  # ARGF.each_byte {|byte| block }  -> ARGF
  # ARGF.each_byte                  -> an_enumerator
  #
  # Iterates over each byte of each file in +ARGV+.
  # A byte is returned as an +Integer+ in the range 0..255.
  #
  # This method allows you to treat the files supplied on the command line as
  # a single file consisting of the concatenation of each named file. After
  # the last byte of the first file has been returned, the first byte of the
  # second file is returned. The +ARGF.filename+ method can be used to
  # determine the filename of the current byte.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # For example:
  #
  #    ARGF.bytes.to_a  #=> [35, 32, ... 95, 10]
  def self.each_byte(*several_variants, &blk)
    #This is a stub, used for indexing
  end
  # ARGF.each_char {|char| block }  -> ARGF
  # ARGF.each_char                  -> an_enumerator
  #
  # Iterates over each character of each file in +ARGF+.
  #
  # This method allows you to treat the files supplied on the command line as
  # a single file consisting of the concatenation of each named file. After
  # the last character of the first file has been returned, the first
  # character of the second file is returned. The +ARGF.filename+ method can
  # be used to determine the name of the file in which the current character
  # appears.
  #
  # If no block is given, an enumerator is returned instead.
  def self.each_char(*several_variants, &blk)
    #This is a stub, used for indexing
  end
  # ARGF.each_codepoint {|codepoint| block }  -> ARGF
  # ARGF.each_codepoint                       -> an_enumerator
  #
  # Iterates over each codepoint of each file in +ARGF+.
  #
  # This method allows you to treat the files supplied on the command line as
  # a single file consisting of the concatenation of each named file. After
  # the last codepoint of the first file has been returned, the first
  # codepoint of the second file is returned. The +ARGF.filename+ method can
  # be used to determine the name of the file in which the current codepoint
  # appears.
  #
  # If no block is given, an enumerator is returned instead.
  def self.each_codepoint(*several_variants, &blk)
    #This is a stub, used for indexing
  end
  # ARGF.read([length [, outbuf]])    -> string, outbuf, or nil
  #
  # Reads _length_ bytes from ARGF. The files named on the command line
  # are concatenated and treated as a single file by this method, so when
  # called without arguments the contents of this pseudo file are returned in
  # their entirety.
  #
  # _length_ must be a non-negative integer or +nil+.
  #
  # If _length_ is a positive integer, +read+ tries to read
  # _length_ bytes without any conversion (binary mode).
  # It returns +nil+ if an EOF is encountered before anything can be read.
  # Fewer than _length_ bytes are returned if an EOF is encountered during
  # the read.
  # In the case of an integer _length_, the resulting string is always
  # in ASCII-8BIT encoding.
  #
  # If _length_ is omitted or is +nil+, it reads until EOF
  # and the encoding conversion is applied, if applicable.
  # A string is returned even if EOF is encountered before any data is read.
  #
  # If _length_ is zero, it returns an empty string (<code>""</code>).
  #
  # If the optional _outbuf_ argument is present,
  # it must reference a String, which will receive the data.
  # The _outbuf_ will contain only the received data after the method call
  # even if it is not empty at the beginning.
  #
  # For example:
  #
  #    $ echo "small" > small.txt
  #    $ echo "large" > large.txt
  #    $ ./glark.rb small.txt large.txt
  #
  #    ARGF.read      #=> "small\nlarge"
  #    ARGF.read(200) #=> "small\nlarge"
  #    ARGF.read(2)   #=> "sm"
  #    ARGF.read(0)   #=> ""
  #
  # Note that this method behaves like the fread() function in C.
  # This means it retries to invoke read(2) system calls to read data
  # with the specified length.
  # If you need the behavior like a single read(2) system call,
  # consider ARGF#readpartial or ARGF#read_nonblock.
  def self.read(p1 = v1, p2 = v2)
    #This is a stub, used for indexing
  end
  # ARGF.readpartial(maxlen)              -> string
  # ARGF.readpartial(maxlen, outbuf)      -> outbuf
  #
  # Reads at most _maxlen_ bytes from the ARGF stream.
  #
  # If the optional _outbuf_ argument is present,
  # it must reference a String, which will receive the data.
  # The _outbuf_ will contain only the received data after the method call
  # even if it is not empty at the beginning.
  #
  # It raises <code>EOFError</code> on end of ARGF stream.
  # Since ARGF stream is a concatenation of multiple files,
  # internally EOF is occur for each file.
  # ARGF.readpartial returns empty strings for EOFs except the last one and
  # raises <code>EOFError</code> for the last one.
  def self.readpartial(*several_variants)
    #This is a stub, used for indexing
  end
  # ARGF.read_nonblock(maxlen[, options])              -> string
  # ARGF.read_nonblock(maxlen, outbuf[, options])      -> outbuf
  #
  # Reads at most _maxlen_ bytes from the ARGF stream in non-blocking mode.
  def self.read_nonblock(*several_variants)
    #This is a stub, used for indexing
  end
  # ARGF.readlines(sep=$/)     -> array
  # ARGF.readlines(limit)      -> array
  # ARGF.readlines(sep, limit) -> array
  #
  # Reads +ARGF+'s current file in its entirety, returning an +Array+ of its
  # lines, one line per element. Lines are assumed to be separated by _sep_.
  #
  #    lines = ARGF.readlines
  #    lines[0]                #=> "This is line one\n"
  def self.readlines(*several_variants)
    #This is a stub, used for indexing
  end
  # ARGF.to_a(sep=$/)     -> array
  # ARGF.to_a(limit)      -> array
  # ARGF.to_a(sep, limit) -> array
  #
  # Reads +ARGF+'s current file in its entirety, returning an +Array+ of its
  # lines, one line per element. Lines are assumed to be separated by _sep_.
  #
  #    lines = ARGF.readlines
  #    lines[0]                #=> "This is line one\n"
  def self.to_a(*several_variants)
    #This is a stub, used for indexing
  end
  # ARGF.gets(sep=$/ [, getline_args])     -> string or nil
  # ARGF.gets(limit [, getline_args])      -> string or nil
  # ARGF.gets(sep, limit [, getline_args]) -> string or nil
  #
  # Returns the next line from the current file in +ARGF+.
  #
  # By default lines are assumed to be separated by <code>$/</code>;
  # to use a different character as a separator, supply it as a +String+
  # for the _sep_ argument.
  #
  # The optional _limit_ argument specifies how many characters of each line
  # to return. By default all characters are returned.
  #
  # See IO.readlines for details about getline_args.
  def self.gets(*several_variants)
    #This is a stub, used for indexing
  end
  # ARGF.readline(sep=$/)     -> string
  # ARGF.readline(limit)      -> string
  # ARGF.readline(sep, limit) -> string
  #
  # Returns the next line from the current file in +ARGF+.
  #
  # By default lines are assumed to be separated by <code>$/</code>;
  # to use a different character as a separator, supply it as a +String+
  # for the _sep_ argument.
  #
  # The optional _limit_ argument specifies how many characters of each line
  # to return. By default all characters are returned.
  #
  # An +EOFError+ is raised at the end of the file.
  def self.readline(*several_variants)
    #This is a stub, used for indexing
  end
  # ARGF.getc  -> String or nil
  #
  # Reads the next character from +ARGF+ and returns it as a +String+. Returns
  # +nil+ at the end of the stream.
  #
  # +ARGF+ treats the files named on the command line as a single file created
  # by concatenating their contents. After returning the last character of the
  # first file, it returns the first character of the second file, and so on.
  #
  # For example:
  #
  #    $ echo "foo" > file
  #    $ ruby argf.rb file
  #
  #    ARGF.getc  #=> "f"
  #    ARGF.getc  #=> "o"
  #    ARGF.getc  #=> "o"
  #    ARGF.getc  #=> "\n"
  #    ARGF.getc  #=> nil
  #    ARGF.getc  #=> nil
  def self.getc()
    #This is a stub, used for indexing
  end
  # ARGF.getbyte  -> Integer or nil
  #
  # Gets the next 8-bit byte (0..255) from +ARGF+. Returns +nil+ if called at
  # the end of the stream.
  #
  # For example:
  #
  #    $ echo "foo" > file
  #    $ ruby argf.rb file
  #
  #    ARGF.getbyte #=> 102
  #    ARGF.getbyte #=> 111
  #    ARGF.getbyte #=> 111
  #    ARGF.getbyte #=> 10
  #    ARGF.getbyte #=> nil
  def self.getbyte()
    #This is a stub, used for indexing
  end
  # ARGF.readchar  -> String or nil
  #
  # Reads the next character from +ARGF+ and returns it as a +String+. Raises
  # an +EOFError+ after the last character of the last file has been read.
  #
  # For example:
  #
  #    $ echo "foo" > file
  #    $ ruby argf.rb file
  #
  #    ARGF.readchar  #=> "f"
  #    ARGF.readchar  #=> "o"
  #    ARGF.readchar  #=> "o"
  #    ARGF.readchar  #=> "\n"
  #    ARGF.readchar  #=> end of file reached (EOFError)
  def self.readchar()
    #This is a stub, used for indexing
  end
  # ARGF.readbyte  -> Integer
  #
  # Reads the next 8-bit byte from ARGF and returns it as an +Integer+. Raises
  # an +EOFError+ after the last byte of the last file has been read.
  #
  # For example:
  #
  #    $ echo "foo" > file
  #    $ ruby argf.rb file
  #
  #    ARGF.readbyte  #=> 102
  #    ARGF.readbyte  #=> 111
  #    ARGF.readbyte  #=> 111
  #    ARGF.readbyte  #=> 10
  #    ARGF.readbyte  #=> end of file reached (EOFError)
  def self.readbyte()
    #This is a stub, used for indexing
  end
  # ARGF.tell  -> Integer
  #
  # Returns the current offset (in bytes) of the current file in +ARGF+.
  #
  #    ARGF.pos    #=> 0
  #    ARGF.gets   #=> "This is line one\n"
  #    ARGF.pos    #=> 17
  def self.tell()
    #This is a stub, used for indexing
  end
  # ARGF.seek(amount, whence=IO::SEEK_SET)  -> 0
  #
  # Seeks to offset _amount_ (an +Integer+) in the +ARGF+ stream according to
  # the value of _whence_. See IO#seek for further details.
  def self.seek(amount, whence=IO::SEEK_SET)
    #This is a stub, used for indexing
  end
  # ARGF.rewind   -> 0
  #
  # Positions the current file to the beginning of input, resetting
  # +ARGF.lineno+ to zero.
  #
  #    ARGF.readline   #=> "This is line one\n"
  #    ARGF.rewind     #=> 0
  #    ARGF.lineno     #=> 0
  #    ARGF.readline   #=> "This is line one\n"
  def self.rewind()
    #This is a stub, used for indexing
  end
  # ARGF.pos   -> Integer
  #
  # Returns the current offset (in bytes) of the current file in +ARGF+.
  #
  #    ARGF.pos    #=> 0
  #    ARGF.gets   #=> "This is line one\n"
  #    ARGF.pos    #=> 17
  def self.pos()
    #This is a stub, used for indexing
  end
  # ARGF.pos = position  -> Integer
  #
  # Seeks to the position given by _position_ (in bytes) in +ARGF+.
  #
  # For example:
  #
  #     ARGF.pos = 17
  #     ARGF.gets   #=> "This is line two\n"
  def self.pos= position
    #This is a stub, used for indexing
  end
  # ARGF.eof?  -> true or false
  # ARGF.eof   -> true or false
  #
  # Returns true if the current file in +ARGF+ is at end of file, i.e. it has
  # no data to read. The stream must be opened for reading or an +IOError+
  # will be raised.
  #
  #    $ echo "eof" | ruby argf.rb
  #
  #    ARGF.eof?                 #=> false
  #    3.times { ARGF.readchar }
  #    ARGF.eof?                 #=> false
  #    ARGF.readchar             #=> "\n"
  #    ARGF.eof?                 #=> true
  def self.eof()
    #This is a stub, used for indexing
  end
  # ARGF.eof?  -> true or false
  #
  # Returns true if the current file in +ARGF+ is at end of file, i.e. it has
  # no data to read. The stream must be opened for reading or an +IOError+
  # will be raised.
  #
  #    $ echo "eof" | ruby argf.rb
  #
  #    ARGF.eof?                 #=> false
  #    3.times { ARGF.readchar }
  #    ARGF.eof?                 #=> false
  #    ARGF.readchar             #=> "\n"
  #    ARGF.eof?                 #=> true
  def self.eof?()
    #This is a stub, used for indexing
  end
  # ARGF.binmode  -> ARGF
  #
  # Puts +ARGF+ into binary mode. Once a stream is in binary mode, it cannot
  # be reset to non-binary mode. This option has the following effects:
  #
  # *  Newline conversion is disabled.
  # *  Encoding conversion is disabled.
  # *  Content is treated as ASCII-8BIT.
  def self.binmode()
    #This is a stub, used for indexing
  end
  # ARGF.binmode?  -> true or false
  #
  # Returns true if +ARGF+ is being read in binary mode; false otherwise.
  # To enable binary mode use +ARGF.binmode+.
  #
  # For example:
  #
  #    ARGF.binmode?  #=> false
  #    ARGF.binmode
  #    ARGF.binmode?  #=> true
  def self.binmode?()
    #This is a stub, used for indexing
  end
  # ARGF.write(string)   -> integer
  #
  # Writes _string_ if inplace mode.
  def self.write(string)
    #This is a stub, used for indexing
  end
  # ios.print               -> nil
  # ios.print(obj, ...)     -> nil
  #
  # Writes the given object(s) to <em>ios</em>. Returns +nil+.
  #
  # The stream must be opened for writing.
  # Each given object that isn't a string will be converted by calling
  # its <code>to_s</code> method.
  # When called without arguments, prints the contents of <code>$_</code>.
  #
  # If the output field separator (<code>$,</code>) is not +nil+,
  # it is inserted between objects.
  # If the output record separator (<code>$\\</code>) is not +nil+,
  # it is appended to the output.
  #
  #    $stdout.print("This is ", 100, " percent.\n")
  #
  # <em>produces:</em>
  #
  #    This is 100 percent.
  def self.print(*several_variants)
    #This is a stub, used for indexing
  end
  # ios.putc(obj)    -> obj
  #
  # If <i>obj</i> is <code>Numeric</code>, write the character whose code is
  # the least-significant byte of <i>obj</i>.
  # If <i>obj</i> is <code>String</code>, write the first character
  # of <i>obj</i> to <em>ios</em>.
  # Otherwise, raise <code>TypeError</code>.
  #
  #    $stdout.putc "A"
  #    $stdout.putc 65
  #
  # <em>produces:</em>
  #
  #    AA
  def self.putc(obj)
    #This is a stub, used for indexing
  end
  # ios.puts(obj, ...)    -> nil
  #
  # Writes the given object(s) to <em>ios</em>.
  # Writes a newline after any that do not already end
  # with a newline sequence. Returns +nil+.
  #
  # The stream must be opened for writing.
  # If called with an array argument, writes each element on a new line.
  # Each given object that isn't a string or array will be converted
  # by calling its +to_s+ method.
  # If called without arguments, outputs a single newline.
  #
  #    $stdout.puts("this", "is", ["a", "test"])
  #
  # <em>produces:</em>
  #
  #    this
  #    is
  #    a
  #    test
  #
  # Note that +puts+ always uses newlines and is not affected
  # by the output record separator (<code>$\\</code>).
  def self.puts(obj='', *arg)
    #This is a stub, used for indexing
  end
  # ios.printf(format_string [, obj, ...])   -> nil
  #
  # Formats and writes to <em>ios</em>, converting parameters under
  # control of the format string. See <code>Kernel#sprintf</code>
  # for details.
  def self.printf(*args)
    #This is a stub, used for indexing
  end
  # ARGF.filename  -> String
  #
  # Returns the current filename. "-" is returned when the current file is
  # STDIN.
  #
  # For example:
  #
  #    $ echo "foo" > foo
  #    $ echo "bar" > bar
  #    $ echo "glark" > glark
  #
  #    $ ruby argf.rb foo bar glark
  #
  #    ARGF.filename  #=> "foo"
  #    ARGF.read(5)   #=> "foo\nb"
  #    ARGF.filename  #=> "bar"
  #    ARGF.skip
  #    ARGF.filename  #=> "glark"
  def self.filename()
    #This is a stub, used for indexing
  end
  # ARGF.path      -> String
  #
  # Returns the current filename. "-" is returned when the current file is
  # STDIN.
  #
  # For example:
  #
  #    $ echo "foo" > foo
  #    $ echo "bar" > bar
  #    $ echo "glark" > glark
  #
  #    $ ruby argf.rb foo bar glark
  #
  #    ARGF.filename  #=> "foo"
  #    ARGF.read(5)   #=> "foo\nb"
  #    ARGF.filename  #=> "bar"
  #    ARGF.skip
  #    ARGF.filename  #=> "glark"
  def self.path()
    #This is a stub, used for indexing
  end
  # ARGF.file  -> IO or File object
  #
  # Returns the current file as an +IO+ or +File+ object.
  # <code>$stdin</code> is returned when the current file is STDIN.
  #
  # For example:
  #
  #    $ echo "foo" > foo
  #    $ echo "bar" > bar
  #
  #    $ ruby argf.rb foo bar
  #
  #    ARGF.file      #=> #<File:foo>
  #    ARGF.read(5)   #=> "foo\nb"
  #    ARGF.file      #=> #<File:bar>
  def self.file()
    #This is a stub, used for indexing
  end
  # ARGF.skip  -> ARGF
  #
  # Sets the current file to the next file in ARGV. If there aren't any more
  # files it has no effect.
  #
  # For example:
  #
  #    $ ruby argf.rb foo bar
  #    ARGF.filename  #=> "foo"
  #    ARGF.skip
  #    ARGF.filename  #=> "bar"
  def self.skip()
    #This is a stub, used for indexing
  end
  # ARGF.close  -> ARGF
  #
  # Closes the current file and skips to the next file in ARGV. If there are
  # no more files to open, just closes the current file. +STDIN+ will not be
  # closed.
  #
  # For example:
  #
  #    $ ruby argf.rb foo bar
  #
  #    ARGF.filename  #=> "foo"
  #    ARGF.close
  #    ARGF.filename  #=> "bar"
  #    ARGF.close
  def self.close()
    #This is a stub, used for indexing
  end
  # ARGF.closed?  -> true or false
  #
  # Returns _true_ if the current file has been closed; _false_ otherwise. Use
  # +ARGF.close+ to actually close the current file.
  def self.closed?()
    #This is a stub, used for indexing
  end
  # ARGF.lineno  -> integer
  #
  # Returns the current line number of ARGF as a whole. This value
  # can be set manually with +ARGF.lineno=+.
  #
  # For example:
  #
  #     ARGF.lineno   #=> 0
  #     ARGF.readline #=> "This is line 1\n"
  #     ARGF.lineno   #=> 1
  def self.lineno()
    #This is a stub, used for indexing
  end
  # ARGF.lineno = integer  -> integer
  #
  # Sets the line number of +ARGF+ as a whole to the given +Integer+.
  #
  # +ARGF+ sets the line number automatically as you read data, so normally
  # you will not need to set it explicitly. To access the current line number
  # use +ARGF.lineno+.
  #
  # For example:
  #
  #     ARGF.lineno      #=> 0
  #     ARGF.readline    #=> "This is line 1\n"
  #     ARGF.lineno      #=> 1
  #     ARGF.lineno = 0  #=> 0
  #     ARGF.lineno      #=> 0
  def self.lineno= integer
    #This is a stub, used for indexing
  end
  # ARGF.inplace_mode  -> String
  #
  # Returns the file extension appended to the names of modified files under
  # in-place edit mode. This value can be set using +ARGF.inplace_mode=+ or
  # passing the +-i+ switch to the Ruby binary.
  def self.inplace_mode()
    #This is a stub, used for indexing
  end
  # ARGF.inplace_mode = ext  -> ARGF
  #
  # Sets the filename extension for in-place editing mode to the given String.
  # Each file being edited has this value appended to its filename. The
  # modified file is saved under this new name.
  #
  # For example:
  #
  #     $ ruby argf.rb file.txt
  #
  #     ARGF.inplace_mode = '.bak'
  #     ARGF.each_line do |line|
  #       print line.sub("foo","bar")
  #     end
  #
  # Each line of _file.txt_ has the first occurrence of "foo" replaced with
  # "bar", then the new line is written out to _file.txt.bak_.
  def self.inplace_mode= ext
    #This is a stub, used for indexing
  end
  # ARGF.external_encoding   -> encoding
  #
  # Returns the external encoding for files read from +ARGF+ as an +Encoding+
  # object. The external encoding is the encoding of the text as stored in a
  # file. Contrast with +ARGF.internal_encoding+, which is the encoding used
  # to represent this text within Ruby.
  #
  # To set the external encoding use +ARGF.set_encoding+.
  #
  # For example:
  #
  #    ARGF.external_encoding  #=>  #<Encoding:UTF-8>
  def self.external_encoding()
    #This is a stub, used for indexing
  end
  # ARGF.internal_encoding   -> encoding
  #
  # Returns the internal encoding for strings read from +ARGF+ as an
  # +Encoding+ object.
  #
  # If +ARGF.set_encoding+ has been called with two encoding names, the second
  # is returned. Otherwise, if +Encoding.default_external+ has been set, that
  # value is returned. Failing that, if a default external encoding was
  # specified on the command-line, that value is used. If the encoding is
  # unknown, +nil+ is returned.
  def self.internal_encoding()
    #This is a stub, used for indexing
  end
  # ARGF.set_encoding(ext_enc)                -> ARGF
  # ARGF.set_encoding("ext_enc:int_enc")      -> ARGF
  # ARGF.set_encoding(ext_enc, int_enc)       -> ARGF
  # ARGF.set_encoding("ext_enc:int_enc", opt) -> ARGF
  # ARGF.set_encoding(ext_enc, int_enc, opt)  -> ARGF
  #
  # If single argument is specified, strings read from ARGF are tagged with
  # the encoding specified.
  #
  # If two encoding names separated by a colon are given, e.g. "ascii:utf-8",
  # the read string is converted from the first encoding (external encoding)
  # to the second encoding (internal encoding), then tagged with the second
  # encoding.
  #
  # If two arguments are specified, they must be encoding objects or encoding
  # names. Again, the first specifies the external encoding; the second
  # specifies the internal encoding.
  #
  # If the external encoding and the internal encoding are specified, the
  # optional +Hash+ argument can be used to adjust the conversion process. The
  # structure of this hash is explained in the String#encode documentation.
  #
  # For example:
  #
  #     ARGF.set_encoding('ascii')         # Tag the input as US-ASCII text
  #     ARGF.set_encoding(Encoding::UTF_8) # Tag the input as UTF-8 text
  #     ARGF.set_encoding('utf-8','ascii') # Transcode the input from US-ASCII
  #                                        # to UTF-8.
  def self.set_encoding(*several_variants)
    #This is a stub, used for indexing
  end
  def self.inspect()
    #This is a stub, used for indexing
  end
end
