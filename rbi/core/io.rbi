# typed: __STDLIB_INTERNAL

# Expect library adds the [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
# instance method
# [`expect`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-expect), which
# does similar act to tcl's expect extension.
#
# In order to use this method, you must require expect:
#
# ```ruby
# require 'expect'
# ```
#
# Please see
# [`expect`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-expect) for
# usage.
# The [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) class is the basis for
# all input and output in Ruby. An I/O stream may be *duplexed* (that is,
# bidirectional), and so may use more than one native operating system stream.
#
# Many of the examples in this section use the
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) class, the only
# standard subclass of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html). The
# two classes are closely associated. Like the
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html) class, the
# [`Socket`](https://docs.ruby-lang.org/en/2.7.0/Socket.html) library subclasses
# from [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) (such as
# [`TCPSocket`](https://docs.ruby-lang.org/en/2.7.0/TCPSocket.html) or
# [`UDPSocket`](https://docs.ruby-lang.org/en/2.7.0/UDPSocket.html)).
#
# The
# [`Kernel#open`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-open)
# method can create an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) (or
# [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html)) object for these
# types of arguments:
#
# *   A plain string represents a filename suitable for the underlying operating
#     system.
#
# *   A string starting with `"|"` indicates a subprocess. The remainder of the
#     string following the `"|"` is invoked as a process with appropriate
#     input/output channels connected to it.
#
# *   A string equal to `"|-"` will create another Ruby instance as a
#     subprocess.
#
#
# The [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) may be opened with
# different file modes (read-only, write-only) and encodings for proper
# conversion. See
# [`IO.new`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-new) for these
# options. See
# [`Kernel#open`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-open)
# for details of the various command formats described above.
#
# [`IO.popen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-popen), the
# [`Open3`](https://docs.ruby-lang.org/en/2.7.0/Open3.html) library, or
# Process#spawn may also be used to communicate with subprocesses through an
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
#
# Ruby will convert pathnames between different operating system conventions if
# possible. For instance, on a Windows system the filename
# `"/gumby/ruby/test.rb"` will be opened as `"\gumby\ruby\test.rb"`. When
# specifying a Windows-style filename in a Ruby string, remember to escape the
# backslashes:
#
# ```ruby
# "C:\\gumby\\ruby\\test.rb"
# ```
#
# Our examples here will use the Unix-style forward slashes;
# File::ALT\_SEPARATOR can be used to get the platform-specific separator
# character.
#
# The global constant [`ARGF`](https://docs.ruby-lang.org/en/2.7.0/ARGF.html)
# (also accessible as `$<`) provides an IO-like stream which allows access to
# all files mentioned on the command line (or STDIN if no files are mentioned).
# [`ARGF#path`](https://docs.ruby-lang.org/en/2.7.0/ARGF.html#method-i-path) and
# its alias
# [`ARGF#filename`](https://docs.ruby-lang.org/en/2.7.0/ARGF.html#method-i-filename)
# are provided to access the name of the file currently being read.
#
# ## io/console
#
# The io/console extension provides methods for interacting with the console.
# The console can be accessed from
# [`IO.console`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-console)
# or the standard input/output/error
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects.
#
# Requiring io/console adds the following methods:
#
# *   [`IO::console`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-console)
# *   [`IO#raw`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-raw)
# *   [`IO#raw!`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-raw-21)
# *   [`IO#cooked`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-cooked)
# *   [`IO#cooked!`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-cooked-21)
# *   [`IO#getch`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-getch)
# *   [`IO#echo=`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-echo-3D)
# *   [`IO#echo?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-echo-3F)
# *   [`IO#noecho`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-noecho)
# *   [`IO#winsize`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-winsize)
# *   [`IO#winsize=`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-winsize-3D)
# *   [`IO#iflush`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-iflush)
# *   [`IO#ioflush`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-ioflush)
# *   [`IO#oflush`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-oflush)
#
#
# Example:
#
# ```ruby
# require 'io/console'
# rows, columns = $stdout.winsize
# puts "Your screen is #{columns} wide and #{rows} tall"
# ```
#
# ## Example Files
#
# Many examples here use these filenames and their corresponding files:
#
# *   `t.txt`: A text-only file that is assumed to exist via:
#
# ```ruby
# text = <<~EOT
#   This is line one.
#   This is the second line.
#   This is the third line.
# EOT
# File.write('t.txt', text)
# ```
#
# *   `t.dat`: A data file that is assumed to exist via:
#
# ```ruby
# data = "\u9990\u9991\u9992\u9993\u9994"
# f = File.open('t.dat', 'wb:UTF-16')
# f.write(data)
# f.close
# ```
#
# *   `t.rus`: A Russian-language text file that is assumed to exist via:
#
# ```ruby
# File.write('t.rus', "\u{442 435 441 442}")
# ```
#
# *   `t.tmp`: A file that is assumed *not* to exist.
#
#
# ## Modes
#
# A number of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) method calls
# must or may specify a *mode* for the stream; the mode determines how stream is
# to be accessible, including:
#
# *   Whether the stream is to be read-only, write-only, or read-write.
# *   Whether the stream is positioned at its beginning or its end.
# *   Whether the stream treats data as text-only or binary.
# *   The external and internal encodings.
#
#
# ### Mode Specified as an [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html)
#
# When `mode` is an integer it must be one or more (combined by bitwise OR (`|`)
# of the modes defined in File::Constants:
#
# *   `File::RDONLY`: Open for reading only.
# *   `File::WRONLY`: Open for writing only.
# *   `File::RDWR`: Open for reading and writing.
# *   `File::APPEND`: Open for appending only.
# *   `File::CREAT`: Create file if it does not exist.
# *   `File::EXCL`: Raise an exception if `File::CREAT` is given and the file
#     exists.
#
#
# Examples:
#
# ```ruby
# File.new('t.txt', File::RDONLY)
# File.new('t.tmp', File::RDWR | File::CREAT | File::EXCL)
# ```
#
# Note: [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html)
# [`IO#set_encoding`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-set_encoding)
# does not allow the mode to be specified as an integer.
#
# ### Mode Specified As a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
#
# When `mode` is a string it must begin with one of the following:
#
# *   `'r'`: Read-only stream, positioned at the beginning; the stream cannot be
#     changed to writable.
# *   `'w'`: Write-only stream, positioned at the beginning; the stream cannot
#     be changed to readable.
# *   `'a'`: Write-only stream, positioned at the end; every write appends to
#     the end; the stream cannot be changed to readable.
# *   `'r+'`: Read-write stream, positioned at the beginning.
# *   `'w+'`: Read-write stream, positioned at the end.
# *   `'a+'`: Read-write stream, positioned at the end.
#
#
# For a writable file stream (that is, any except read-only), the file is
# truncated to zero if it exists, and is created if it does not exist.
#
# Examples:
#
# ```ruby
# File.open('t.txt', 'r')
# File.open('t.tmp', 'w')
# ```
#
# Either of the following may be suffixed to any of the above:
#
# *   `'t'`: Text data; sets the default external encoding to `Encoding::UTF_8`;
#     on Windows, enables conversion between EOL and CRLF.
# *   `'b'`: Binary data; sets the default external encoding to
#     `Encoding::ASCII_8BIT`; on Windows, suppresses conversion between EOL and
#     CRLF.
#
#
# If neither is given, the stream defaults to text data.
#
# Examples:
#
# ```ruby
# File.open('t.txt', 'rt')
# File.open('t.dat', 'rb')
# ```
#
# The following may be suffixed to any writable mode above:
#
# *   `'x'`: Creates the file if it does not exist; raises an exception if the
#     file exists.
#
#
# Example:
#
# ```ruby
# File.open('t.tmp', 'wx')
# ```
#
# Finally, the mode string may specify encodings -- either external encoding
# only or both external and internal encodings -- by appending one or both
# encoding names, separated by colons:
#
# ```ruby
# f = File.new('t.dat', 'rb')
# f.external_encoding # => #<Encoding:ASCII-8BIT>
# f.internal_encoding # => nil
# f = File.new('t.dat', 'rb:UTF-16')
# f.external_encoding # => #<Encoding:UTF-16 (dummy)>
# f.internal_encoding # => nil
# f = File.new('t.dat', 'rb:UTF-16:UTF-16')
# f.external_encoding # => #<Encoding:UTF-16 (dummy)>
# f.internal_encoding # => #<Encoding:UTF-16>
# ```
#
# The numerous encoding names are available in array
# [`Encoding.name_list`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html#method-c-name_list):
#
# ```ruby
# Encoding.name_list.size    # => 175
# Encoding.name_list.take(3) # => ["ASCII-8BIT", "UTF-8", "US-ASCII"]
# ```
#
# ## Encodings
#
# When the external encoding is set, strings read are tagged by that encoding
# when reading, and strings written are converted to that encoding when writing.
#
# When both external and internal encodings are set, strings read are converted
# from external to internal encoding, and strings written are converted from
# internal to external encoding. For further details about transcoding input and
# output, see [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html).
#
# If the external encoding is `'BOM|UTF-8'`, `'BOM|UTF-16LE'` or
# `'BOM|UTF16-BE'`, Ruby checks for a Unicode BOM in the input document to help
# determine the encoding. For UTF-16 encodings the file open mode must be
# binary. If the BOM is found, it is stripped and the external encoding from the
# BOM is used.
#
# Note that the BOM-style encoding option is case insensitive, so 'bom|utf-8' is
# also valid.)
#
# ## Open Options
#
# A number of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) methods accept
# an optional parameter `opts`, which determines how a new stream is to be
# opened:
#
# *   `:mode`: Stream mode.
# *   `:flags`: [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html)
#     file open flags; If `mode` is also given, the two are bitwise-ORed.
# *   `:external_encoding`: External encoding for the stream.
# *   `:internal_encoding`: Internal encoding for the stream. `'-'` is a synonym
#     for the default internal encoding. If the value is `nil` no conversion
#     occurs.
# *   `:encoding`: Specifies external and internal encodings as
#     `'extern:intern'`.
# *   `:textmode`: If a truthy value, specifies the mode as text-only, binary
#     otherwise.
# *   `:binmode`: If a truthy value, specifies the mode as binary, text-only
#     otherwise.
# *   `:autoclose`: If a truthy value, specifies that the `fd` will close when
#     the stream closes; otherwise it remains open.
#
#
# Also available are the options offered in
# [`String#encode`](https://docs.ruby-lang.org/en/2.7.0/String.html#method-i-encode),
# which may control conversion between external internal encoding.
#
# ## Getline Options
#
# A number of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) methods accept
# optional keyword arguments that determine how a stream is to be treated:
#
# *   `:chomp`: If `true`, line separators are omitted; default is  `false`.
#
#
# ## Position
#
# An [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) stream has a
# *position*, which is the non-negative integer offset (in bytes) in the stream
# where the next read or write will occur.
#
# Note that a text stream may have multi-byte characters, so a text stream whose
# position is `n` (*bytes*) may not have `n` *characters* preceding the current
# position -- there may be fewer.
#
# A new stream is initially positioned:
#
# *   At the beginning (position `0`) if its mode is `'r'`, `'w'`, or `'r+'`.
# *   At the end (position `self.size`) if its mode is `'a'`, `'w+'`, or `'a+'`.
#
#
# Methods to query the position:
#
# *   [`IO#tell`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-tell) and
#     its alias
#     [`IO#pos`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pos)
#     return the position for an open stream.
# *   [`IO#eof?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof-3F)
#     and its alias
#     [`IO#eof`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof)
#     return whether the position is at the end of a readable stream.
#
#
# Reading from a stream usually changes its position:
#
# ```ruby
# f = File.open('t.txt')
# f.tell     # => 0
# f.readline # => "This is line one.\n"
# f.tell     # => 19
# f.readline # => "This is the second line.\n"
# f.tell     # => 45
# f.eof?     # => false
# f.readline # => "Here's the third line.\n"
# f.eof?     # => true
# ```
#
# Writing to a stream usually changes its position:
#
# ```ruby
# f = File.open('t.tmp', 'w')
# f.tell         # => 0
# f.write('foo') # => 3
# f.tell         # => 3
# f.write('bar') # => 3
# f.tell         # => 6
# ```
#
# Iterating over a stream usually changes its position:
#
# ```ruby
# f = File.open('t.txt')
# f.each do |line|
#   p "position=#{f.pos} eof?=#{f.eof?} line=#{line}"
# end
# ```
#
# Output:
#
# ```ruby
# "position=19 eof?=false line=This is line one.\n"
# "position=45 eof?=false line=This is the second line.\n"
# "position=70 eof?=true line=This is the third line.\n"
# ```
#
# The position may also be changed by certain other methods:
#
# *   [`IO#pos=`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pos-3D)
#     and [`IO#seek`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-seek)
#     change the position to a specified offset.
# *   [`IO#rewind`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-rewind)
#     changes the position to the beginning.
#
#
# ## Line Number
#
# A readable [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) stream has a
# *line* *number*, which is the non-negative integer line number in the stream
# where the next read will occur.
#
# A new stream is initially has line number `0`.
#
# [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html)
# [`IO#lineno`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-lineno)
# returns the line number.
#
# Reading lines from a stream usually changes its line number:
#
# ```ruby
# f = File.open('t.txt', 'r')
# f.lineno   # => 0
# f.readline # => "This is line one.\n"
# f.lineno   # => 1
# f.readline # => "This is the second line.\n"
# f.lineno   # => 2
# f.readline # => "Here's the third line.\n"
# f.lineno   # => 3
# f.eof?     # => true
# ```
#
# Iterating over lines in a stream usually changes its line number:
#
# ```ruby
# f = File.open('t.txt')
# f.each_line do |line|
#   p "position=#{f.pos} eof?=#{f.eof?} line=#{line}"
# end
# ```
#
# Output:
#
# ```ruby
# "position=19 eof?=false line=This is line one.\n"
# "position=45 eof?=false line=This is the second line.\n"
# "position=70 eof?=true line=This is the third line.\n"
# ```
#
# ## What's Here
#
# First, what's elsewhere.
# [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) IO:
#
# *   Inherits from [class
#     Object](Object.html#class-Object-label-What-27s+Here).
# *   Includes [module
#     Enumerable](Enumerable.html#module-Enumerable-label-What-27s+Here), which
#     provides dozens of additional methods.
#
#
# Here, class [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) provides
# methods that are useful for:
#
# *   [Creating](#class-IO-label-Creating)
# *   [Reading](#class-IO-label-Reading)
# *   [Writing](#class-IO-label-Writing)
# *   [Positioning](#class-IO-label-Positioning)
# *   [Iterating](#class-IO-label-Iterating)
# *   [Settings](#class-IO-label-Settings)
# *   [Querying](#class-IO-label-Querying)
# *   [Buffering](#class-IO-label-Buffering)
# *   [Low-Level Access](#class-IO-label-Low-Level+Access)
# *   [Other](#class-IO-label-Other)
#
#
# ### Creating
#
#     [`::new`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-new) (aliased as [`::for_fd`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-for_fd))
# :       Creates and returns a new
#         [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object for the
#         given integer file descriptor.
#
#     [`::open`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-open)
# :       Creates a new [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
#         object.
#
#     [`::pipe`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-pipe)
# :       Creates a connected pair of reader and writer
#         [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects.
#
#     [`::popen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-popen)
# :       Creates an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object
#         to interact with a subprocess.
#
#     [`::select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
# :       Selects which given
#         [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) instances are
#         ready for reading,
#
#     writing, or have pending exceptions.
#
#
# ### Reading
#
#     [`::binread`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-binread)
# :       Returns a binary string with all or a subset of bytes from the given
#         file.
#
#     [`::read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-read)
# :       Returns a string with all or a subset of bytes from the given file.
#
#     [`::readlines`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-readlines)
# :       Returns an array of strings, which are the lines from the given file.
#
#     [`getbyte`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-getbyte)
# :       Returns the next 8-bit byte read from `self` as an integer.
#
#     [`getc`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-getc)
# :       Returns the next character read from `self` as a string.
#
#     [`gets`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-gets)
# :       Returns the line read from `self`.
#
#     [`pread`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pread)
# :       Returns all or the next *n* bytes read from `self`, not updating the
#         receiver's offset.
#
#     [`read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read)
# :       Returns all remaining or the next *n* bytes read from `self` for a
#         given *n*.
#
#     [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read_nonblock)
# :       the next *n* bytes read from `self` for a given *n*, in non-block
#         mode.
#
#     [`readbyte`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-readbyte)
# :       Returns the next byte read from `self`; same as
#         [`getbyte`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-getbyte),
#         but raises an exception on end-of-file.
#
#     [`readchar`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-readchar)
# :       Returns the next character read from `self`; same as
#         [`getc`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-getc),
#         but raises an exception on end-of-file.
#
#     [`readline`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-readline)
# :       Returns the next line read from `self`; same as getline, but raises an
#         exception of end-of-file.
#
#     [`readlines`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-readlines)
# :       Returns an array of all lines read read from `self`.
#
#     [`readpartial`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-readpartial)
# :       Returns up to the given number of bytes from `self`.
#
#
#
# ### Writing
#
#     [`::binwrite`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-binwrite)
# :       Writes the given string to the file at the given filepath, in binary
#         mode.
#
#     [`::write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-write)
# :       Writes the given string to `self`.
#
#     [:<<](#method-i-3C-3C)
# :       Appends the given string to `self`.
#
#     [`print`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-print)
# :       Prints last read line or given objects to `self`.
#
#     [`printf`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-printf)
# :       Writes to `self` based on the given format string and objects.
#
#     [`putc`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-putc)
# :       Writes a character to `self`.
#
#     [`puts`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-puts)
# :       Writes lines to `self`, making sure line ends with a newline.
#
#     [`pwrite`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pwrite)
# :       Writes the given string at the given offset, not updating the
#         receiver's offset.
#
#     [`write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write)
# :       Writes one or more given strings to `self`.
#
#     [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write_nonblock)
# :       Writes one or more given strings to `self` in non-blocking mode.
#
#
#
# ### Positioning
#
#     [`lineno`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-lineno)
# :       Returns the current line number in `self`.
#
#     [`lineno=`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-lineno-3D)
# :       Sets the line number is `self`.
#
#     [`pos`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pos) (aliased as [`tell`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-tell))
# :       Returns the current byte offset in `self`.
#
#     [`pos=`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pos-3D)
# :       Sets the byte offset in `self`.
#
#     [`reopen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-reopen)
# :       Reassociates `self` with a new or existing
#         [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) stream.
#
#     [`rewind`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-rewind)
# :       Positions `self` to the beginning of input.
#
#     [`seek`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-seek)
# :       Sets the offset for `self` relative to given position.
#
#
#
# ### Iterating
#
#     [`::foreach`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-foreach)
# :       Yields each line of given file to the block.
#
#     [`each`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each) (aliased as [`each_line`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_line))
# :       Calls the given block with each successive line in `self`.
#
#     [`each_byte`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_byte)
# :       Calls the given block with each successive byte in `self` as an
#         integer.
#
#     [`each_char`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_char)
# :       Calls the given block with each successive character in `self` as a
#         string.
#
#     [`each_codepoint`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_codepoint)
# :       Calls the given block with each successive codepoint in `self` as an
#         integer.
#
#
#
# ### Settings
#
#     [`autoclose=`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-autoclose-3D)
# :       Sets whether `self` auto-closes.
#
#     [`binmode`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-binmode)
# :       Sets `self` to binary mode.
#
#     [`close`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-close)
# :       Closes `self`.
#
#     [`close_on_exec=`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-close_on_exec-3D)
# :       Sets the close-on-exec flag.
#
#     [`close_read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-close_read)
# :       Closes `self` for reading.
#
#     [`close_write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-close_write)
# :       Closes `self` for writing.
#
#     [`set_encoding`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-set_encoding)
# :       Sets the encoding for `self`.
#
#     [`set_encoding_by_bom`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-set_encoding_by_bom)
# :       Sets the encoding for `self`, based on its Unicode byte-order-mark.
#
#     [`sync=`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sync-3D)
# :       Sets the sync-mode to the given value.
#
#
#
# ### Querying
#
#     [`autoclose?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-autoclose-3F)
# :       Returns whether `self` auto-closes.
#
#     [`binmode?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-binmode-3F)
# :       Returns whether `self` is in binary mode.
#
#     [`close_on_exec?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-close_on_exec-3F)
# :       Returns the close-on-exec flag for `self`.
#
#     [`closed?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-closed-3F)
# :       Returns whether `self` is closed.
#
#     [`eof?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof-3F) (aliased as [`eof`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof))
# :       Returns whether `self` is at end-of-file.
#
#     [`external_encoding`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-external_encoding)
# :       Returns the external encoding object for `self`.
#
#     [`fileno`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-fileno) (aliased as [`to_i`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-to_i))
# :       Returns the integer file descriptor for `self`
#
#     [`internal_encoding`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-internal_encoding)
# :       Returns the internal encoding object for `self`.
#
#     [`pid`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pid)
# :       Returns the process ID of a child process associated with `self`, if
#         `self` was created by
#         [`::popen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-popen).
#
#     [`stat`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-stat)
# :       Returns the
#         [`File::Stat`](https://docs.ruby-lang.org/en/2.7.0/File/Stat.html)
#         object containing status information for `self`.
#
#     [`sync`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sync)
# :       Returns whether `self` is in sync-mode.
#
#     tty (aliased as [`isatty`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-isatty))
# :       Returns whether `self` is a terminal.
#
#
#
# ### Buffering
#
#     [`fdatasync`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-fdatasync)
# :       Immediately writes all buffered data in `self` to disk.
#
#     [`flush`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-flush)
# :       Flushes any buffered data within `self` to the underlying operating
#         system.
#
#     [`fsync`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-fsync)
# :       Immediately writes all buffered data and attributes in `self` to disk.
#
#     [`ungetbyte`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-ungetbyte)
# :       Prepends buffer for `self` with given integer byte or string.
#
#     [`ungetc`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-ungetc)
# :       Prepends buffer for `self` with given string.
#
#
#
# ### Low-Level Access
#
#     [`::sysopen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-sysopen)
# :       Opens the file given by its path, returning the integer file
#         descriptor.
#
#     [`advise`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-advise)
# :       Announces the intention to access data from `self` in a specific way.
#
#     [`fcntl`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-fcntl)
# :       Passes a low-level command to the file specified by the given file
#         descriptor.
#
#     [`ioctl`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-ioctl)
# :       Passes a low-level command to the device specified by the given file
#         descriptor.
#
#     [`sysread`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sysread)
# :       Returns up to the next *n* bytes read from self using a low-level
#         read.
#
#     [`sysseek`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sysseek)
# :       Sets the offset for `self`.
#
#     [`syswrite`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-syswrite)
# :       Writes the given string to `self` using a low-level write.
#
#
#
# ### Other
#
#     [`::copy_stream`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-copy_stream)
# :       Copies data from a source to a destination, each of which is a
#         filepath or an IO-like object.
#
#     [`::try_convert`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-try_convert)
# :       Returns a new [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
#         object resulting from converting the given object.
#
#     [`inspect`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-inspect)
# :       Returns the string representation of `self`.
class IO < Object
  include File::Constants
  include Enumerable

  extend T::Generic
  Elem = type_member(:out) {{fixed: String}}

  APPEND = T.let(T.unsafe(nil), Integer)
  BINARY = T.let(T.unsafe(nil), Integer)
  CREAT = T.let(T.unsafe(nil), Integer)
  DIRECT = T.let(T.unsafe(nil), Integer)
  DSYNC = T.let(T.unsafe(nil), Integer)
  EXCL = T.let(T.unsafe(nil), Integer)
  FNM_CASEFOLD = T.let(T.unsafe(nil), Integer)
  FNM_DOTMATCH = T.let(T.unsafe(nil), Integer)
  FNM_EXTGLOB = T.let(T.unsafe(nil), Integer)
  FNM_NOESCAPE = T.let(T.unsafe(nil), Integer)
  FNM_PATHNAME = T.let(T.unsafe(nil), Integer)
  FNM_SHORTNAME = T.let(T.unsafe(nil), Integer)
  FNM_SYSCASE = T.let(T.unsafe(nil), Integer)
  LOCK_EX = T.let(T.unsafe(nil), Integer)
  LOCK_NB = T.let(T.unsafe(nil), Integer)
  LOCK_SH = T.let(T.unsafe(nil), Integer)
  LOCK_UN = T.let(T.unsafe(nil), Integer)
  NOATIME = T.let(T.unsafe(nil), Integer)
  NOCTTY = T.let(T.unsafe(nil), Integer)
  NOFOLLOW = T.let(T.unsafe(nil), Integer)
  NONBLOCK = T.let(T.unsafe(nil), Integer)
  NULL = T.let(T.unsafe(nil), String)
  RDONLY = T.let(T.unsafe(nil), Integer)
  RDWR = T.let(T.unsafe(nil), Integer)
  RSYNC = T.let(T.unsafe(nil), Integer)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) I/O position from the
  # current position
  SEEK_CUR = T.let(T.unsafe(nil), Integer)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) I/O position to the
  # next location containing data
  SEEK_DATA = T.let(T.unsafe(nil), Integer)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) I/O position from the
  # end
  SEEK_END = T.let(T.unsafe(nil), Integer)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) I/O position to the
  # next hole
  SEEK_HOLE = T.let(T.unsafe(nil), Integer)
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) I/O position from the
  # beginning
  SEEK_SET = T.let(T.unsafe(nil), Integer)
  SHARE_DELETE = T.let(T.unsafe(nil), Integer)
  SYNC = T.let(T.unsafe(nil), Integer)
  TMPFILE = T.let(T.unsafe(nil), Integer)
  TRUNC = T.let(T.unsafe(nil), Integer)
  WRONLY = T.let(T.unsafe(nil), Integer)

  # Writes the given `object` to `self`, which must be opened for writing (see
  # [Modes](#class-IO-label-Modes)); returns `self`; if `object` is not a
  # string, it is converted via method `to_s`:
  #
  # ```ruby
  # $stdout << 'Hello' << ', ' << 'World!' << "\n"
  # $stdout << 'foo' << :bar << 2 << "\n"
  # ```
  #
  # Output:
  #
  # ```
  # Hello, World!
  # foobar2
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.self_type)
  end
  def <<(arg0); end

  # Announce an intention to access data from the current file in a specific
  # pattern. On platforms that do not support the *posix\_fadvise(2)* system
  # call, this method is a no-op.
  #
  # *advice* is one of the following symbols:
  #
  # :normal
  # :   No advice to give; the default assumption for an open file.
  # :sequential
  # :   The data will be accessed sequentially with lower offsets read before
  #     higher ones.
  # :random
  # :   The data will be accessed in random order.
  # :willneed
  # :   The data will be accessed in the near future.
  # :dontneed
  # :   The data will not be accessed in the near future.
  # :noreuse
  # :   The data will only be accessed once.
  #
  #
  # The semantics of a piece of advice are platform-dependent. See *man 2
  # posix\_fadvise* for details.
  #
  # "data" means the region of the current file that begins at *offset* and
  # extends for *len* bytes. If *len* is 0, the region ends at the last byte of
  # the file. By default, both *offset* and *len* are 0, meaning that the advice
  # applies to the entire file.
  #
  # If an error occurs, one of the following exceptions will be raised:
  #
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html)
  # :   The [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) stream is
  #     closed.
  # Errno::EBADF
  # :   The file descriptor of the current file is invalid.
  # Errno::EINVAL
  # :   An invalid value for *advice* was given.
  # Errno::ESPIPE
  # :   The file descriptor of the current file refers to a FIFO or pipe. (Linux
  #     raises Errno::EINVAL in this case).
  # [`TypeError`](https://docs.ruby-lang.org/en/2.7.0/TypeError.html)
  # :   Either *advice* was not a
  #     [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html), or one of
  #     the other arguments was not an
  #     [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  # [`RangeError`](https://docs.ruby-lang.org/en/2.7.0/RangeError.html)
  # :   One of the arguments given was too big/small.
  #
  # This list is not exhaustive; other [`Errno`](https://docs.ruby-lang.org/en/2.7.0/Errno.html)
  # :   exceptions are also possible.
  sig do
    params(
        arg0: Symbol,
        offset: Integer,
        len: Integer,
    )
    .returns(NilClass)
  end
  def advise(arg0, offset=T.unsafe(nil), len=T.unsafe(nil)); end

  # Sets auto-close flag.
  #
  # ```ruby
  # f = open("/dev/null")
  # IO.for_fd(f.fileno)
  # # ...
  # f.gets # may cause Errno::EBADF
  #
  # f = open("/dev/null")
  # IO.for_fd(f.fileno).autoclose = false
  # # ...
  # f.gets # won't cause Errno::EBADF
  # ```
  sig do
    params(
        arg0: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def autoclose=(arg0); end

  # Returns `true` if the underlying file descriptor of *ios* will be closed
  # automatically at its finalization, otherwise `false`.
  sig {returns(T::Boolean)}
  def autoclose?(); end

  # Puts *ios* into binary mode. Once a stream is in binary mode, it cannot be
  # reset to nonbinary mode.
  #
  # *   newline conversion disabled
  # *   encoding conversion disabled
  # *   content is treated as ASCII-8BIT
  sig {returns(T.self_type)}
  def binmode(); end

  # Returns `true` if *ios* is binmode.
  sig {returns(T::Boolean)}
  def binmode?(); end

  # Closes *ios* and flushes any pending writes to the operating system. The
  # stream is unavailable for any further data operations; an
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) is raised if
  # such an attempt is made. I/O streams are automatically closed when they are
  # claimed by the garbage collector.
  #
  # If *ios* is opened by
  # [`IO.popen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-popen),
  # [`close`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-close) sets
  # `$?`.
  #
  # Calling this method on closed
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object is just ignored
  # since Ruby 2.3.
  sig {returns(NilClass)}
  def close(); end

  # Sets a close-on-exec flag.
  #
  # ```ruby
  # f = open("/dev/null")
  # f.close_on_exec = true
  # system("cat", "/proc/self/fd/#{f.fileno}") # cat: /proc/self/fd/3: No such file or directory
  # f.closed?                #=> false
  # ```
  #
  # Ruby sets close-on-exec flags of all file descriptors by default since Ruby
  # 2.0.0. So you don't need to set by yourself. Also, unsetting a close-on-exec
  # flag can cause file descriptor leak if another thread use fork() and exec()
  # (via system() method for example). If you really needs file descriptor
  # inheritance to child process, use spawn()'s argument such as fd=>fd.
  sig do
    params(
        arg0: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def close_on_exec=(arg0); end

  # Returns `true` if *ios* will be closed on exec.
  #
  # ```ruby
  # f = open("/dev/null")
  # f.close_on_exec?                 #=> false
  # f.close_on_exec = true
  # f.close_on_exec?                 #=> true
  # f.close_on_exec = false
  # f.close_on_exec?                 #=> false
  # ```
  sig {returns(T::Boolean)}
  def close_on_exec?(); end

  # Closes the read end of a duplex I/O stream (i.e., one that contains both a
  # read and a write stream, such as a pipe). Will raise an
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) if the stream
  # is not duplexed.
  #
  # ```ruby
  # f = IO.popen("/bin/sh","r+")
  # f.close_read
  # f.readlines
  # ```
  #
  # *produces:*
  #
  # ```
  # prog.rb:3:in `readlines': not opened for reading (IOError)
  #  from prog.rb:3
  # ```
  #
  # Calling this method on closed
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object is just ignored
  # since Ruby 2.3.
  sig {returns(NilClass)}
  def close_read(); end

  # Closes the write end of a duplex I/O stream (i.e., one that contains both a
  # read and a write stream, such as a pipe). Will raise an
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) if the stream
  # is not duplexed.
  #
  # ```ruby
  # f = IO.popen("/bin/sh","r+")
  # f.close_write
  # f.print "nowhere"
  # ```
  #
  # *produces:*
  #
  # ```
  # prog.rb:3:in `write': not opened for writing (IOError)
  #  from prog.rb:3:in `print'
  #  from prog.rb:3
  # ```
  #
  # Calling this method on closed
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object is just ignored
  # since Ruby 2.3.
  sig {returns(NilClass)}
  def close_write(); end

  # Returns `true` if *ios* is completely closed (for duplex streams, both
  # reader and writer), `false` otherwise.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.close         #=> nil
  # f.closed?       #=> true
  # f = IO.popen("/bin/sh","r+")
  # f.close_write   #=> nil
  # f.closed?       #=> false
  # f.close_read    #=> nil
  # f.closed?       #=> true
  # ```
  sig {returns(T::Boolean)}
  def closed?(); end

  # Executes the block for every line in *ios*, where lines are separated by
  # *sep*. *ios* must be opened for reading or an
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) will be
  # raised.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.each {|line| puts "#{f.lineno}: #{line}" }
  # ```
  #
  # *produces:*
  #
  # ```
  # 1: This is line one
  # 2: This is line two
  # 3: This is line three
  # 4: And so on...
  # ```
  #
  # See
  # [`IO.readlines`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-readlines)
  # for details about getline\_args.
  #
  # Also aliased as:
  # [`each_line`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_line)
  sig do
    params(
        sep: String,
        limit: Integer,
        chomp: T::Boolean,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig do
    params(
        sep: String,
        limit: Integer,
        chomp: T::Boolean,
    )
    .returns(T::Enumerator[String])
  end
  def each(sep=T.unsafe(nil), limit=T.unsafe(nil), chomp: false, &blk); end

  # Calls the given block once for each byte (0..255) in *ios*, passing the byte
  # as an argument. The stream must be opened for reading or an
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) will be
  # raised.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # f = File.new("testfile")
  # checksum = 0
  # f.each_byte {|x| checksum ^= x }   #=> #<File:testfile>
  # checksum                           #=> 12
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def each_byte(&blk); end

  # Calls the given block once for each character in *ios*, passing the
  # character as an argument. The stream must be opened for reading or an
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) will be
  # raised.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.each_char {|c| print c, ' ' }   #=> #<File:testfile>
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[String])}
  def each_char(&blk); end

  # Passes the [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html)
  # ordinal of each character in *ios*, passing the codepoint as an argument.
  # The stream must be opened for reading or an
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) will be
  # raised.
  #
  # If no block is given, an enumerator is returned instead.
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def each_codepoint(&blk); end

  # Returns `true` if the stream is positioned at its end, `false` otherwise;
  # see [Position](#class-IO-label-Position):
  #
  # ```ruby
  # f = File.open('t.txt')
  # f.eof           # => false
  # f.seek(0, :END) # => 0
  # f.eof           # => true
  # ```
  #
  # Raises an exception unless the stream is opened for reading; see
  # [Mode](#class-IO-label-Mode).
  #
  # If `self` is a stream such as pipe or socket, this method blocks until the
  # other end sends some data or closes it:
  #
  # ```ruby
  # r, w = IO.pipe
  # Thread.new { sleep 1; w.close }
  # r.eof? # => true # After 1-second wait.
  #
  # r, w = IO.pipe
  # Thread.new { sleep 1; w.puts "a" }
  # r.eof?  # => false # After 1-second wait.
  #
  # r, w = IO.pipe
  # r.eof?  # blocks forever
  # ```
  #
  # Note that this method reads data to the input byte buffer. So
  # [`IO#sysread`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sysread)
  # may not behave as you intend with
  # [`IO#eof?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof-3F),
  # unless you call
  # [`IO#rewind`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-rewind)
  # first (which is not available for some streams).
  #
  # I#eof? is an alias for
  # [`IO#eof`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof).
  #
  # Also aliased as:
  # [`eof?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof-3F)
  sig {returns(T::Boolean)}
  def eof(); end

  # Returns the [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html)
  # object that represents the encoding of the file. If *io* is in write mode
  # and no encoding is specified, returns `nil`.
  def external_encoding; end

  # Provides a mechanism for issuing low-level commands to control or query
  # file-oriented I/O streams. Arguments and results are platform dependent. If
  # *arg* is a number, its value is passed directly. If it is a string, it is
  # interpreted as a binary sequence of bytes
  # ([`Array#pack`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-pack)
  # might be a useful way to build this string). On Unix platforms, see
  # `fcntl(2)` for details. Not implemented on all platforms.
  sig do
    params(
        integer_cmd: Integer,
        arg: T.any(String, Integer),
    )
    .returns(Integer)
  end
  def fcntl(integer_cmd, arg); end

  # Immediately writes to disk all data buffered in the stream, via the
  # operating system's: `fdatasync(2)`, if supported, otherwise via `fsync(2)`,
  # if supported; otherwise raises an exception.
  sig {returns(T.nilable(Integer))}
  def fdatasync(); end

  # Returns the integer file descriptor for the stream:
  #
  # ```ruby
  # $stdin.fileno             # => 0
  # $stdout.fileno            # => 1
  # $stderr.fileno            # => 2
  # File.open('t.txt').fileno # => 10
  # ```
  #
  # [`IO#to_i`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-to_i) is an
  # alias for
  # [`IO#fileno`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-fileno).
  #
  # Also aliased as:
  # [`to_i`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-to_i)
  sig {returns(Integer)}
  def fileno(); end

  # Flushes data buffered in `self` to the operating system (but does not
  # necessarily flush data buffered in the operating system):
  #
  # ```ruby
  # $stdout.print 'no newline' # Not necessarily flushed.
  # $stdout.flush              # Flushed.
  # ```
  sig {returns(T.self_type)}
  def flush(); end

  # Immediately writes to disk all data buffered in the stream, via the
  # operating system's `fsync(2)`.
  #
  # Note this difference:
  #
  # *   [`IO#sync=`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sync-3D):
  #     Ensures that data is flushed from the stream's internal buffers, but
  #     does not guarantee that the operating system actually writes the data to
  #     disk.
  # *   [`IO#fsync`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-fsync):
  #     Ensures both that data is flushed from internal buffers, and that data
  #     is written to disk.
  #
  #
  # Raises an exception if the operating system does not support `fsync(2)`.
  sig {returns(T.nilable(Integer))}
  def fsync(); end

  # Gets the next 8-bit byte (0..255) from *ios*. Returns `nil` if called at end
  # of file.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.getbyte   #=> 84
  # f.getbyte   #=> 104
  # ```
  sig {returns(T.nilable(Integer))}
  def getbyte(); end

  # Reads a one-character string from *ios*. Returns `nil` if called at end of
  # file.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.getc   #=> "h"
  # f.getc   #=> "e"
  # ```
  sig {returns(T.nilable(String))}
  def getc(); end

  # Reads and returns data from the stream; assigns the return value to `$_`.
  #
  # With no arguments given, returns the next line as determined by line
  # separator `$/`, or `nil` if none:
  #
  # ```ruby
  # f = File.open('t.txt')
  # f.gets # => "This is line one.\n"
  # $_     # => "This is line one.\n"
  # f.gets # => "This is the second line.\n"
  # f.gets # => "This is the third line.\n"
  # f.gets # => nil
  # ```
  #
  # With string argument `sep` given, but not argument `limit`, returns the next
  # line as determined by line separator `sep`, or `nil` if none:
  #
  # ```ruby
  # f = File.open('t.txt')
  # f.gets(' is') # => "This is"
  # f.gets(' is') # => " line one.\nThis is"
  # f.gets(' is') # => " the second line.\nThis is"
  # f.gets(' is') # => " the third line.\n"
  # f.gets(' is') # => nil
  # ```
  #
  # Note two special values for `sep`:
  #
  # *   `nil`: The entire stream is read and returned.
  # *   `''` (empty string): The next "paragraph" is read and returned, the
  #     paragraph separator being two successive line separators.
  #
  #
  # With integer argument `limit` given, returns up to `limit+1` bytes:
  #
  # ```ruby
  # # Text with 1-byte characters.
  # File.open('t.txt') {|f| f.gets(1) } # => "T"
  # File.open('t.txt') {|f| f.gets(2) } # => "Th"
  # File.open('t.txt') {|f| f.gets(3) } # => "Thi"
  # File.open('t.txt') {|f| f.gets(4) } # => "This"
  # # No more than one line.
  # File.open('t.txt') {|f| f.gets(17) } # => "This is line one."
  # File.open('t.txt') {|f| f.gets(18) } # => "This is line one.\n"
  # File.open('t.txt') {|f| f.gets(19) } # => "This is line one.\n"
  #
  # # Text with 2-byte characters, which will not be split.
  # File.open('t.rus') {|f| f.gets(1).size } # => 1
  # File.open('t.rus') {|f| f.gets(2).size } # => 1
  # File.open('t.rus') {|f| f.gets(3).size } # => 2
  # File.open('t.rus') {|f| f.gets(4).size } # => 2
  # ```
  #
  # With arguments `sep` and `limit`, combines the two behaviors above:
  #
  # *   Returns the next line as determined by line separator `sep`, or `nil` if
  #     none.
  # *   But returns no more than `limit+1` bytes.
  #
  #
  # For all forms above, trailing optional keyword arguments may be given; see
  # [Getline Options](#class-IO-label-Getline+Options):
  #
  # ```ruby
  # f = File.open('t.txt')
  # # Chomp the lines.
  # f.gets(chomp: true) # => "This is line one."
  # f.gets(chomp: true) # => "This is the second line."
  # f.gets(chomp: true) # => "This is the third line."
  # f.gets(chomp: true) # => nil
  # ```
  sig do
    params(
        sep: T.nilable(String),
        limit: Integer,
    )
    .returns(T.nilable(String))
  end
  def gets(sep=T.unsafe(nil), limit=T.unsafe(nil)); end

  # Returns a new [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object (a
  # stream) for the given integer file descriptor `fd` and `mode` string. `opt`
  # may be used to specify parts of `mode` in a more readable fashion. See also
  # [`IO.sysopen`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-sysopen)
  # and
  # [`IO.for_fd`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-for_fd).
  #
  # [`IO.new`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-new) is
  # called by various [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html)
  # and [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) opening methods such
  # as [`IO::open`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-open),
  # [`Kernel#open`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-open),
  # and
  # [`File::open`](https://docs.ruby-lang.org/en/2.6.0/File.html#method-c-open).
  #
  # ### Open Mode
  #
  # When `mode` is an integer it must be combination of the modes defined in
  # File::Constants (`File::RDONLY`, `File::WRONLY|File::CREAT`). See the
  # open(2) man page for more information.
  #
  # When `mode` is a string it must be in one of the following forms:
  #
  # ```
  # fmode
  # fmode ":" ext_enc
  # fmode ":" ext_enc ":" int_enc
  # fmode ":" "BOM|UTF-*"
  # ```
  #
  # `fmode` is an [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) open mode
  # string, `ext_enc` is the external encoding for the
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) and `int_enc` is the
  # internal encoding.
  #
  # #### [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) Open Mode
  #
  # Ruby allows the following open modes:
  #
  # ```
  # "r"  Read-only, starts at beginning of file  (default mode).
  #
  # "r+" Read-write, starts at beginning of file.
  #
  # "w"  Write-only, truncates existing file
  #      to zero length or creates a new file for writing.
  #
  # "w+" Read-write, truncates existing file to zero length
  #      or creates a new file for reading and writing.
  #
  # "a"  Write-only, each write call appends data at end of file.
  #      Creates a new file for writing if file does not exist.
  #
  # "a+" Read-write, each write call appends data at end of file.
  #      Creates a new file for reading and writing if file does
  #      not exist.
  # ```
  #
  # The following modes must be used separately, and along with one or more of
  # the modes seen above.
  #
  # ```
  # "b"  Binary file mode
  #      Suppresses EOL <-> CRLF conversion on Windows. And
  #      sets external encoding to ASCII-8BIT unless explicitly
  #      specified.
  #
  # "t"  Text file mode
  # ```
  #
  # The exclusive access mode ("x") can be used together with "w" to ensure the
  # file is created. `Errno::EEXIST` is raised when it already exists. It may
  # not be supported with all kinds of streams (e.g. pipes).
  #
  # When the open mode of original
  # [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) is read only, the mode
  # cannot be changed to be writable. Similarly, the open mode cannot be changed
  # from write only to readable.
  #
  # When such a change is attempted the error is raised in different locations
  # according to the platform.
  #
  # ### [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html)
  #
  # When `ext_enc` is specified, strings read will be tagged by the encoding
  # when reading, and strings output will be converted to the specified encoding
  # when writing.
  #
  # When `ext_enc` and `int_enc` are specified read strings will be converted
  # from `ext_enc` to `int_enc` upon input, and written strings will be
  # converted from `int_enc` to `ext_enc` upon output. See
  # [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) for further
  # details of transcoding on input and output.
  #
  # If "BOM|UTF-8", "BOM|UTF-16LE" or "BOM|UTF16-BE" are used, Ruby checks for a
  # Unicode BOM in the input document to help determine the encoding. For UTF-16
  # encodings the file open mode must be binary. When present, the BOM is
  # stripped and the external encoding from the BOM is used. When the BOM is
  # missing the given Unicode encoding is used as `ext_enc`. (The BOM-set
  # encoding option is case insensitive, so "bom|utf-8" is also valid.)
  #
  # ### Options
  #
  # `opt` can be used instead of `mode` for improved readability. The following
  # keys are supported:
  #
  # :mode
  # :   Same as `mode` parameter
  #
  # :flags
  # :   Specifies file open flags as integer. If `mode` parameter is given, this
  #     parameter will be bitwise-ORed.
  #
  # :external\_encoding
  # :   External encoding for the
  #     [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html).
  #
  # :internal\_encoding
  # :   Internal encoding for the
  #     [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html). "-" is a synonym
  #     for the default internal encoding.
  #
  #     If the value is `nil` no conversion occurs.
  #
  # :encoding
  # :   Specifies external and internal encodings as "extern:intern".
  #
  # :textmode
  # :   If the value is truth value, same as "t" in argument `mode`.
  #
  # :binmode
  # :   If the value is truth value, same as "b" in argument `mode`.
  #
  # :autoclose
  # :   If the value is `false`, the `fd` will be kept open after this
  #     [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) instance gets
  #     finalized.
  #
  #
  # Also, `opt` can have same keys in
  # [`String#encode`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-encode)
  # for controlling conversion between the external encoding and the internal
  # encoding.
  #
  # ### Example 1
  #
  # ```ruby
  # fd = IO.sysopen("/dev/tty", "w")
  # a = IO.new(fd,"w")
  # $stderr.puts "Hello"
  # a.puts "World"
  # ```
  #
  # Produces:
  #
  # ```ruby
  # Hello
  # World
  # ```
  #
  # ### Example 2
  #
  # ```ruby
  # require 'fcntl'
  #
  # fd = STDERR.fcntl(Fcntl::F_DUPFD)
  # io = IO.new(fd, mode: 'w:UTF-16LE', cr_newline: true)
  # io.puts "Hello, World!"
  #
  # fd = STDERR.fcntl(Fcntl::F_DUPFD)
  # io = IO.new(fd, mode: 'w', cr_newline: true,
  #             external_encoding: Encoding::UTF_16LE)
  # io.puts "Hello, World!"
  # ```
  #
  # Both of above print "Hello, World!" in UTF-16LE to standard error output
  # with converting EOL generated by `puts` to CR.
  sig do
    params(
        fd: Integer,
        mode: Integer,
        opt: T.untyped,
    )
    .void
  end
  def initialize(fd, mode=T.unsafe(nil), **opt); end

  # Returns a string representation of `self`:
  #
  # ```ruby
  # f = File.open('t.txt')
  # f.inspect # => "#<File:t.txt>"
  # ```
  sig {returns(String)}
  def inspect(); end

  # Returns the [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html)
  # of the internal string if conversion is specified. Otherwise returns `nil`.
  sig {returns(Encoding)}
  def internal_encoding(); end

  # Provides a mechanism for issuing low-level commands to control or query I/O
  # devices. Arguments and results are platform dependent. If *arg* is a number,
  # its value is passed directly. If it is a string, it is interpreted as a
  # binary sequence of bytes. On Unix platforms, see `ioctl(2)` for details. Not
  # implemented on all platforms.
  sig do
    params(
        integer_cmd: Integer,
        arg: T.any(String, Integer),
    )
    .returns(Integer)
  end
  def ioctl(integer_cmd, arg); end

  # Returns `true` if *ios* is associated with a terminal device (tty), `false`
  # otherwise.
  #
  # ```ruby
  # File.new("testfile").isatty   #=> false
  # File.new("/dev/tty").isatty   #=> true
  # ```
  #
  #
  # Also aliased as:
  # [`tty?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-tty-3F)
  sig {returns(T::Boolean)}
  def isatty(); end

  # Returns the current line number in *ios*. The stream must be opened for
  # reading.
  # [`lineno`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-lineno)
  # counts the number of times
  # [`gets`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-gets) is
  # called rather than the number of newlines encountered. The two values will
  # differ if
  # [`gets`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-gets) is
  # called with a separator other than newline.
  #
  # Methods that use `$/` like
  # [`each`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each), lines
  # and
  # [`readline`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-readline)
  # will also increment
  # [`lineno`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-lineno).
  #
  # See also the `$.` variable.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.lineno   #=> 0
  # f.gets     #=> "This is line one\n"
  # f.lineno   #=> 1
  # f.gets     #=> "This is line two\n"
  # f.lineno   #=> 2
  # ```
  sig {returns(Integer)}
  def lineno(); end

  # Manually sets the current line number to the given value. `$.` is updated
  # only on the next read.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.gets                     #=> "This is line one\n"
  # $.                         #=> 1
  # f.lineno = 1000
  # f.lineno                   #=> 1000
  # $.                         #=> 1         # lineno of last read
  # f.gets                     #=> "This is line two\n"
  # $.                         #=> 1001      # lineno of last read
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def lineno=(arg0); end

  # Reads *maxlen* bytes from *ios* using the pread system call and returns them
  # as a string without modifying the underlying descriptor offset. This is
  # advantageous compared to combining
  # [`IO#seek`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-seek) and
  # [`IO#read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read) in
  # that it is atomic, allowing multiple threads/process to share the same
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object for reading the
  # file at various locations. This bypasses any userspace buffering of the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) layer. If the optional
  # *outbuf* argument is present, it must reference a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), which will
  # receive the data. Raises
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # on error, [`EOFError`](https://docs.ruby-lang.org/en/2.7.0/EOFError.html) at
  # end of file and
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.7.0/NotImplementedError.html)
  # if platform does not implement the system call.
  #
  # ```ruby
  # File.write("testfile", "This is line one\nThis is line two\n")
  # File.open("testfile") do |f|
  #   p f.read           # => "This is line one\nThis is line two\n"
  #   p f.pread(12, 0)   # => "This is line"
  #   p f.pread(9, 8)    # => "line one\n"
  # end
  # ```
  def pread(*_); end

  # Writes the given string to *ios* at *offset* using pwrite() system call.
  # This is advantageous to combining
  # [`IO#seek`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-seek) and
  # [`IO#write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write) in
  # that it is atomic, allowing multiple threads/process to share the same
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object for reading the
  # file at various locations. This bypasses any userspace buffering of the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) layer. Returns the
  # number of bytes written. Raises
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # on error and
  # [`NotImplementedError`](https://docs.ruby-lang.org/en/2.7.0/NotImplementedError.html)
  # if platform does not implement the system call.
  #
  # ```ruby
  # File.open("out", "w") do |f|
  #   f.pwrite("ABCDEF", 3)   #=> 6
  # end
  #
  # File.read("out")          #=> "\u0000\u0000\u0000ABCDEF"
  # ```
  def pwrite(_, _); end

  # With no associated block,
  # [`IO.open`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-open) is a
  # synonym for
  # [`IO.new`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-new). If the
  # optional code block is given, it will be passed `io` as an argument, and the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object will
  # automatically be closed when the block terminates. In this instance,
  # [`IO.open`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-open)
  # returns the value of the block.
  #
  # See [`IO.new`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-new) for
  # a description of the `fd`, `mode` and `opt` parameters.
  sig do
    params(
      fd: T.any(String, Integer),
      mode: T.any(Integer, String),
      opt: T.untyped,
    ).returns(IO)
  end
  sig do
    type_parameters(:U).params(
      fd: T.any(String, Integer),
      mode: T.any(Integer, String),
      opt: T.untyped,
      blk: T.proc.params(io: T.attached_class).returns(T.type_parameter(:U))
    ).returns(T.type_parameter(:U))
  end
  def self.open(fd, mode='r', **opt, &blk); end

  # Returns the process ID of a child process associated with the stream, which
  # will have been set by IO#popen, or `nil` if the stream was not created by
  # IO#popen:
  #
  # ```ruby
  # pipe = IO.popen("-")
  # if pipe
  #   $stderr.puts "In parent, child pid is #{pipe.pid}"
  # else
  #   $stderr.puts "In child, pid is #{$$}"
  # end
  # ```
  #
  # Output:
  #
  # ```
  # In child, pid is 26209
  # In parent, child pid is 26209
  # ```
  sig {returns(Integer)}
  def pid(); end

  # Creates a pair of pipe endpoints (connected to each other) and returns them
  # as a two-element array of
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects: `[` *read\_io*,
  # *write\_io* `]`.
  #
  # If a block is given, the block is called and returns the value of the block.
  # *read\_io* and *write\_io* are sent to the block as arguments. If read\_io
  # and write\_io are not closed when the block exits, they are closed. i.e.
  # closing read\_io and/or write\_io doesn't cause an error.
  #
  # Not available on all platforms.
  #
  # If an encoding (encoding name or encoding object) is specified as an
  # optional argument, read string from pipe is tagged with the encoding
  # specified. If the argument is a colon separated two encoding names "A:B",
  # the read string is converted from encoding A (external encoding) to encoding
  # B (internal encoding), then tagged with B. If two optional arguments are
  # specified, those must be encoding objects or encoding names, and the first
  # one is the external encoding, and the second one is the internal encoding.
  # If the external encoding and the internal encoding is specified, optional
  # hash argument specify the conversion option.
  #
  # In the example below, the two processes close the ends of the pipe that they
  # are not using. This is not just a cosmetic nicety. The read end of a pipe
  # will not generate an end of file condition if there are any writers with the
  # pipe still open. In the case of the parent process, the `rd.read` will never
  # return if it does not first issue a `wr.close`.
  #
  # ```ruby
  # rd, wr = IO.pipe
  #
  # if fork
  #   wr.close
  #   puts "Parent got: <#{rd.read}>"
  #   rd.close
  #   Process.wait
  # else
  #   rd.close
  #   puts "Sending message to parent"
  #   wr.write "Hi Dad"
  #   wr.close
  # end
  # ```
  #
  # *produces:*
  #
  # ```
  # Sending message to parent
  # Parent got: <Hi Dad>
  # ```
  sig do
    params(
      ext_enc: T.nilable(T.any(String, Encoding)),
      int_enc: T.nilable(T.any(String, Encoding)),
      opt: T.nilable(T::Hash[Symbol, String]),
      blk: T.nilable(T.proc.params(read_io: IO, write_io: IO).void)
    ).returns([IO, IO])
  end
  def self.pipe(ext_enc = nil, int_enc = nil, opt = nil, &blk); end

  # Returns the current position (in bytes) in `self` (see
  # [Position](#class-IO-label-Position)):
  #
  # ```ruby
  # f = File.new('t.txt')
  # f.tell     # => 0
  # f.readline # => "This is line one.\n"
  # f.tell     # => 19
  # ```
  #
  # Related:
  # [`IO#pos=`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pos-3D),
  # [`IO#seek`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-seek).
  #
  # [`IO#pos`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pos) is an
  # alias for
  # [`IO#tell`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-tell).
  #
  # Alias for:
  # [`tell`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-tell)
  sig {returns(Integer)}
  def pos(); end

  # Seeks to the given `new_position` (in bytes); see
  # [Position](#class-IO-label-Position):
  #
  # ```ruby
  # f = File.open('t.txt')
  # f.tell     # => 0
  # f.pos = 20 # => 20
  # f.tell     # => 20
  # ```
  #
  # Related:
  # [`IO#seek`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-seek),
  # [`IO#tell`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-tell).
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def pos=(arg0); end

  # Writes the given object(s) to *ios*. Returns `nil`.
  #
  # The stream must be opened for writing. Each given object that isn't a string
  # will be converted by calling its `to_s` method. When called without
  # arguments, prints the contents of `$_`.
  #
  # If the output field separator (`$,`) is not `nil`, it is inserted between
  # objects. If the output record separator (`$\`) is not `nil`, it is appended
  # to the output.
  #
  # ```ruby
  # $stdout.print("This is ", 100, " percent.\n")
  # ```
  #
  # *produces:*
  #
  # ```
  # This is 100 percent.
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def print(*arg0); end

  # Formats and writes to *ios*, converting parameters under control of the
  # format string. See
  # [`Kernel#sprintf`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-sprintf)
  # for details.
  sig do
    params(
        format_string: String,
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def printf(format_string, *arg0); end

  # If *obj* is [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html),
  # write the character whose code is the least-significant byte of *obj*. If
  # *obj* is [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), write
  # the first character of *obj* to *ios*. Otherwise, raise
  # [`TypeError`](https://docs.ruby-lang.org/en/2.7.0/TypeError.html).
  #
  # ```ruby
  # $stdout.putc "A"
  # $stdout.putc 65
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # AA
  # ```
  sig do
    params(
        arg0: T.any(Numeric, String),
    )
    .returns(T.untyped)
  end
  def putc(arg0); end

  # Writes the given object(s) to *ios*. Writes a newline after any that do not
  # already end with a newline sequence. Returns `nil`.
  #
  # The stream must be opened for writing. If called with an array argument,
  # writes each element on a new line. Each given object that isn't a string or
  # array will be converted by calling its `to_s` method. If called without
  # arguments, outputs a single newline.
  #
  # ```ruby
  # $stdout.puts("this", "is", ["a", "test"])
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # this
  # is
  # a
  # test
  # ```
  #
  # Note that `puts` always uses newlines and is not affected by the output
  # record separator (`$\`).
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def puts(*arg0); end

  # Reads bytes from the stream (in binary mode):
  #
  # *   If `maxlen` is `nil`, reads all bytes.
  # *   Otherwise reads `maxlen` bytes, if available.
  # *   Otherwise reads all bytes.
  #
  #
  # Returns a string (either a new string or the given `out_string`) containing
  # the bytes read. The encoding of the string depends on both `maxLen` and
  # `out_string`:
  #
  # *   `maxlen` is `nil`: uses internal encoding of `self` (regardless of
  #     whether `out_string` was given).
  # *   `maxlen` not `nil`:
  #
  #     *   `out_string` given: encoding of `out_string` not modified.
  #     *   `out_string` not given: ASCII-8BIT is used.
  #
  #
  #
  # **Without Argument `out_string`**
  #
  # When argument `out_string` is omitted, the returned value is a new string:
  #
  # ```ruby
  # f = File.new('t.txt')
  # f.read
  # # => "This is line one.\nThis is the second line.\nThis is the third line.\n"
  # f.rewind
  # f.read(40)      # => "This is line one.\r\nThis is the second li"
  # f.read(40)      # => "ne.\r\nThis is the third line.\r\n"
  # f.read(40)      # => nil
  # ```
  #
  # If `maxlen` is zero, returns an empty string.
  #
  # ** With Argument `out_string`**
  #
  # When argument `out_string` is given, the returned value is `out_string`,
  # whose content is replaced:
  #
  # ```ruby
  # f = File.new('t.txt')
  # s = 'foo'      # => "foo"
  # f.read(nil, s) # => "This is line one.\nThis is the second line.\nThis is the third line.\n"
  # s              # => "This is line one.\nThis is the second line.\nThis is the third line.\n"
  # f.rewind
  # s = 'bar'
  # f.read(40, s)  # => "This is line one.\r\nThis is the second li"
  # s              # => "This is line one.\r\nThis is the second li"
  # s = 'baz'
  # f.read(40, s)  # => "ne.\r\nThis is the third line.\r\n"
  # s              # => "ne.\r\nThis is the third line.\r\n"
  # s = 'bat'
  # f.read(40, s)  # => nil
  # s              # => ""
  # ```
  #
  # Note that this method behaves like the fread() function in C. This means it
  # retries to invoke read(2) system calls to read data with the specified
  # maxlen (or until EOF).
  #
  # This behavior is preserved even if the stream is in non-blocking mode. (This
  # method is non-blocking-flag insensitive as other methods.)
  #
  # If you need the behavior like a single read(2) system call, consider
  # [`readpartial`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-readpartial),
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read_nonblock),
  # and
  # [`sysread`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sysread).
  sig do
    params(
        length: Integer,
        outbuf: String,
    )
    .returns(T.nilable(String))
  end
  def read(length=T.unsafe(nil), outbuf=T.unsafe(nil)); end

  # Reads at most *maxlen* bytes from *ios* using the read(2) system call after
  # O\_NONBLOCK is set for the underlying file descriptor.
  #
  # If the optional *outbuf* argument is present, it must reference a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), which will
  # receive the data. The *outbuf* will contain only the received data after the
  # method call even if it is not empty at the beginning.
  #
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read_nonblock)
  # just calls the read(2) system call. It causes all errors the read(2) system
  # call causes: Errno::EWOULDBLOCK, Errno::EINTR, etc. The caller should care
  # such errors.
  #
  # If the exception is Errno::EWOULDBLOCK or Errno::EAGAIN, it is extended by
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html).
  # So
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # can be used to rescue the exceptions for retrying read\_nonblock.
  #
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read_nonblock)
  # causes [`EOFError`](https://docs.ruby-lang.org/en/2.7.0/EOFError.html) on
  # EOF.
  #
  # On some platforms, such as Windows, non-blocking mode is not supported on
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects other than
  # sockets. In such cases, Errno::EBADF will be raised.
  #
  # If the read byte buffer is not empty,
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read_nonblock)
  # reads from the buffer like readpartial. In this case, the read(2) system
  # call is not called.
  #
  # When
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read_nonblock)
  # raises an exception kind of
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html),
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read_nonblock)
  # should not be called until io is readable for avoiding busy loop. This can
  # be done as follows.
  #
  # ```ruby
  # # emulates blocking read (readpartial).
  # begin
  #   result = io.read_nonblock(maxlen)
  # rescue IO::WaitReadable
  #   IO.select([io])
  #   retry
  # end
  # ```
  #
  # Although
  # [`IO#read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read_nonblock)
  # doesn't raise
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html).
  # [`OpenSSL::Buffering#read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-read_nonblock)
  # can raise
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html).
  # If [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) and SSL should be
  # used polymorphically,
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # should be rescued too. See the document of
  # [`OpenSSL::Buffering#read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-read_nonblock)
  # for sample code.
  #
  # Note that this method is identical to readpartial except the non-blocking
  # flag is set.
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read_nonblock)
  # should not raise an
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # exception, but return the symbol `:wait_readable` instead. At EOF, it will
  # return nil instead of raising
  # [`EOFError`](https://docs.ruby-lang.org/en/2.7.0/EOFError.html).
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

  # Reads a byte as with
  # [`IO#getbyte`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-getbyte),
  # but raises an
  # [`EOFError`](https://docs.ruby-lang.org/en/2.7.0/EOFError.html) on end of
  # file.
  sig {returns(Integer)}
  def readbyte(); end

  # Reads a one-character string from *ios*. Raises an
  # [`EOFError`](https://docs.ruby-lang.org/en/2.7.0/EOFError.html) on end of
  # file.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.readchar   #=> "h"
  # f.readchar   #=> "e"
  # ```
  sig {returns(String)}
  def readchar(); end

  # Reads a line as with
  # [`IO#gets`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-gets), but
  # raises an [`EOFError`](https://docs.ruby-lang.org/en/2.7.0/EOFError.html) on
  # end of file.
  sig do
    params(
        sep: String,
        limit: Integer,
        chomp: T::Boolean,
    )
    .returns(String)
  end
  def readline(sep=T.unsafe(nil), limit=T.unsafe(nil), chomp: false); end

  # Reads all of the lines in *ios*, and returns them in an array. Lines are
  # separated by the optional *sep*. If *sep* is `nil`, the rest of the stream
  # is returned as a single record. If the first argument is an integer, or an
  # optional second argument is given, the returning string would not be longer
  # than the given value in bytes. The stream must be opened for reading or an
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) will be
  # raised.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.readlines[0]   #=> "This is line one\n"
  #
  # f = File.new("testfile", chomp: true)
  # f.readlines[0]   #=> "This is line one"
  # ```
  #
  # See
  # [`IO.readlines`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-readlines)
  # for details about getline\_args.
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(T::Array[String])
  end
  def readlines(sep=T.unsafe(nil), limit=T.unsafe(nil)); end

  # Reads up to `maxlen` bytes from the stream; returns a string (either a new
  # string or the given `out_string`). Its encoding is:
  #
  # *   The unchanged encoding of `out_string`, if `out_string` is given.
  # *   ASCII-8BIT, otherwise.
  #
  # *   Contains `maxlen` bytes from the stream, if available.
  # *   Otherwise contains all available bytes, if any available.
  # *   Otherwise is an empty string.
  #
  #
  # With the single non-negative integer argument `maxlen` given, returns a new
  # string:
  #
  # ```ruby
  # f = File.new('t.txt')
  # f.readpartial(30) # => "This is line one.\nThis is the"
  # f.readpartial(30) # => " second line.\nThis is the thi"
  # f.readpartial(30) # => "rd line.\n"
  # f.eof             # => true
  # f.readpartial(30) # Raises EOFError.
  # ```
  #
  # With both argument `maxlen` and string argument `out_string` given, returns
  # modified `out_string`:
  #
  # ```ruby
  # f = File.new('t.txt')
  # s = 'foo'
  # f.readpartial(30, s) # => "This is line one.\nThis is the"
  # s = 'bar'
  # f.readpartial(0, s)  # => ""
  # ```
  #
  # This method is useful for a stream such as a pipe, a socket, or a tty. It
  # blocks only when no data is immediately available. This means that it blocks
  # only when *all* of the following are true:
  #
  # *   The byte buffer in the stream is empty.
  # *   The content of the stream is empty.
  # *   The stream is not at EOF.
  #
  #
  # When blocked, the method waits for either more data or EOF on the stream:
  #
  # *   If more data is read, the method returns the data.
  # *   If EOF is reached, the method raises
  #     [`EOFError`](https://docs.ruby-lang.org/en/2.7.0/EOFError.html).
  #
  #
  # When not blocked, the method responds immediately:
  #
  # *   Returns data from the buffer if there is any.
  # *   Otherwise returns data from the stream if there is any.
  # *   Otherwise raises
  #     [`EOFError`](https://docs.ruby-lang.org/en/2.7.0/EOFError.html) if the
  #     stream has reached EOF.
  #
  #
  # Note that this method is similar to sysread. The differences are:
  #
  # *   If the byte buffer is not empty, read from the byte buffer instead of
  #     "sysread for buffered
  #     [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  #     ([`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html))".
  # *   It doesn't cause Errno::EWOULDBLOCK and Errno::EINTR. When readpartial
  #     meets EWOULDBLOCK and EINTR by read system call, readpartial retries the
  #     system call.
  #
  #
  # The latter means that readpartial is non-blocking-flag insensitive. It
  # blocks on the situation
  # [`IO#sysread`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sysread)
  # causes Errno::EWOULDBLOCK as if the fd is blocking mode.
  #
  # Examples:
  #
  # ```ruby
  # #                        # Returned      Buffer Content    Pipe Content
  # r, w = IO.pipe           #
  # w << 'abc'               #               ""                "abc".
  # r.readpartial(4096)      # => "abc"      ""                ""
  # r.readpartial(4096)      # (Blocks because buffer and pipe are empty.)
  #
  # #                        # Returned      Buffer Content    Pipe Content
  # r, w = IO.pipe           #
  # w << 'abc'               #               ""                "abc"
  # w.close                  #               ""                "abc" EOF
  # r.readpartial(4096)      # => "abc"      ""                 EOF
  # r.readpartial(4096)      # raises EOFError
  #
  # #                        # Returned      Buffer Content    Pipe Content
  # r, w = IO.pipe           #
  # w << "abc\ndef\n"        #               ""                "abc\ndef\n"
  # r.gets                   # => "abc\n"    "def\n"           ""
  # w << "ghi\n"             #               "def\n"           "ghi\n"
  # r.readpartial(4096)      # => "def\n"    ""                "ghi\n"
  # r.readpartial(4096)      # => "ghi\n"    ""                ""
  # ```
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

  # Reassociates *ios* with the I/O stream given in *other\_IO* or to a new
  # stream opened on *path*. This may dynamically change the actual class of
  # this stream. The `mode` and `opt` parameters accept the same values as
  # [`IO.open`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-open).
  #
  # ```ruby
  # f1 = File.new("testfile")
  # f2 = File.new("testfile")
  # f2.readlines[0]   #=> "This is line one\n"
  # f2.reopen(f1)     #=> #<File:testfile>
  # f2.readlines[0]   #=> "This is line one\n"
  # ```
  sig do
    params(
        other_IO_or_path: T.any(IO, Tempfile),
    )
    .returns(IO)
  end
  sig do
    params(
        other_IO_or_path: String,
        mode_str: String,
    )
    .returns(IO)
  end
  def reopen(other_IO_or_path, mode_str=T.unsafe(nil)); end

  # Repositions the stream to its beginning, setting both the position and the
  # line number to zero; see [Position](#class-IO-label-Position) and [Line
  # Number](#class-IO-label-Line+Number):
  #
  # ```ruby
  # f = File.open('t.txt')
  # f.tell     # => 0
  # f.lineno   # => 0
  # f.readline # => "This is line one.\n"
  # f.tell     # => 19
  # f.lineno   # => 1
  # f.rewind   # => 0
  # f.tell     # => 0
  # f.lineno   # => 0
  # ```
  #
  # Note that this method cannot be used with streams such as pipes, ttys, and
  # sockets.
  sig {returns(Integer)}
  def rewind(); end

  # Seeks to the position given by integer `offset` (see
  # [Position](#class-IO-label-Position)) and constant `whence`, which is one
  # of:
  #
  # *   `:CUR` or `IO::SEEK_CUR`: Repositions the stream to its current position
  #     plus the given `offset`:
  #
  # ```ruby
  # f = File.open('t.txt')
  # f.tell            # => 0
  # f.seek(20, :CUR)  # => 0
  # f.tell            # => 20
  # f.seek(-10, :CUR) # => 0
  # f.tell            # => 10
  # ```
  #
  # *   `:END` or `IO::SEEK_END`: Repositions the stream to its end plus the
  #     given `offset`:
  #
  # ```ruby
  # f = File.open('t.txt')
  # f.tell            # => 0
  # f.seek(0, :END)   # => 0  # Repositions to stream end.
  # f.tell            # => 70
  # f.seek(-20, :END) # => 0
  # f.tell            # => 50
  # f.seek(-40, :END) # => 0
  # f.tell            # => 30
  # ```
  #
  # *   `:SET` or `IO:SEEK_SET`: Repositions the stream to the given `offset`:
  #
  # ```ruby
  # f = File.open('t.txt')
  # f.tell            # => 0
  # f.seek(20, :SET) # => 0
  # f.tell           # => 20
  # f.seek(40, :SET) # => 0
  # f.tell           # => 40
  # ```
  #
  #
  # Related:
  # [`IO#pos=`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pos-3D),
  # [`IO#tell`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-tell).
  sig do
    params(
        amount: Integer,
        whence: Integer,
    )
    .returns(Integer)
  end
  def seek(amount, whence=T.unsafe(nil)); end

  # If single argument is specified, read string from io is tagged with the
  # encoding specified. If encoding is a colon separated two encoding names
  # "A:B", the read string is converted from encoding A (external encoding) to
  # encoding B (internal encoding), then tagged with B. If two arguments are
  # specified, those must be encoding objects or encoding names, and the first
  # one is the external encoding, and the second one is the internal encoding.
  # If the external encoding and the internal encoding is specified, optional
  # hash argument specify the conversion option.
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

  # Checks if `ios` starts with a BOM, and then consumes it and sets the
  # external encoding. Returns the result encoding if found, or nil. If `ios` is
  # not binmode or its encoding has been set already, an exception will be
  # raised.
  #
  # ```ruby
  # File.write("bom.txt", "\u{FEFF}abc")
  # ios = File.open("bom.txt", "rb")
  # ios.set_encoding_by_bom    #=>  #<Encoding:UTF-8>
  #
  # File.write("nobom.txt", "abc")
  # ios = File.open("nobom.txt", "rb")
  # ios.set_encoding_by_bom    #=>  nil
  # ```
  sig { returns(T.nilable(Encoding)) }
  def set_encoding_by_bom; end

  # Returns status information for *ios* as an object of type
  # [`File::Stat`](https://docs.ruby-lang.org/en/2.7.0/File/Stat.html).
  #
  # ```ruby
  # f = File.new("testfile")
  # s = f.stat
  # "%o" % s.mode   #=> "100644"
  # s.blksize       #=> 4096
  # s.atime         #=> Wed Apr 09 08:53:54 CDT 2003
  # ```
  sig {returns(File::Stat)}
  def stat(); end

  # Returns the current sync mode of the stream. When sync mode is true, all
  # output is immediately flushed to the underlying operating system and is not
  # buffered by Ruby internally. See also
  # [`fsync`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-fsync).
  #
  # ```ruby
  # f = File.open('t.tmp', 'w')
  # f.sync # => false
  # f.sync = true
  # f.sync # => true
  # ```
  sig {returns(T::Boolean)}
  def sync(); end

  # Sets the *sync* *mode* for the stream to the given value; returns the given
  # value.
  #
  # Values for the sync mode:
  #
  # *   `true`: All output is immediately flushed to the underlying operating
  #     system and is not buffered internally.
  # *   `false`: Output may be buffered internally.
  #
  #
  # Example;
  #
  # ```ruby
  # f = File.open('t.tmp', 'w')
  # f.sync # => false
  # f.sync = true
  # f.sync # => true
  # ```
  #
  # Related:
  # [`IO#fsync`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-fsync).
  sig do
    params(
        arg0: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def sync=(arg0); end

  # Reads *maxlen* bytes from *ios* using a low-level read and returns them as a
  # string. Do not mix with other methods that read from *ios* or you may get
  # unpredictable results.
  #
  # If the optional *outbuf* argument is present, it must reference a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), which will
  # receive the data. The *outbuf* will contain only the received data after the
  # method call even if it is not empty at the beginning.
  #
  # Raises
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # on error and [`EOFError`](https://docs.ruby-lang.org/en/2.7.0/EOFError.html)
  # at end of file.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.sysread(16)   #=> "This is line one"
  # ```
  sig do
    params(
        maxlen: Integer,
        outbuf: String,
    )
    .returns(String)
  end
  def sysread(maxlen, outbuf=T.unsafe(nil)); end

  # Seeks to a given *offset* in the stream according to the value of *whence*
  # (see [`IO#seek`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-seek)
  # for values of *whence*). Returns the new offset into the file.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.sysseek(-13, IO::SEEK_END)   #=> 53
  # f.sysread(10)                  #=> "And so on."
  # ```
  sig do
    params(
        amount: Integer,
        whence: Integer,
    )
    .returns(Integer)
  end
  def sysseek(amount, whence=T.unsafe(nil)); end

  # Writes the given string to *ios* using a low-level write. Returns the number
  # of bytes written. Do not mix with other methods that write to *ios* or you
  # may get unpredictable results. Raises
  # [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html)
  # on error.
  #
  # ```ruby
  # f = File.new("out", "w")
  # f.syswrite("ABCDEF")   #=> 6
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(Integer)
  end
  def syswrite(arg0); end

  # Returns the current position (in bytes) in `self` (see
  # [Position](#class-IO-label-Position)):
  #
  # ```ruby
  # f = File.new('t.txt')
  # f.tell     # => 0
  # f.readline # => "This is line one.\n"
  # f.tell     # => 19
  # ```
  #
  # Related:
  # [`IO#pos=`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pos-3D),
  # [`IO#seek`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-seek).
  #
  # [`IO#pos`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pos) is an
  # alias for
  # [`IO#tell`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-tell).
  #
  # Also aliased as:
  # [`pos`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pos)
  sig {returns(Integer)}
  def tell(); end

  # Returns `self`.
  sig {returns(T.self_type)}
  def to_io(); end

  # Returns `true` if *ios* is associated with a terminal device (tty), `false`
  # otherwise.
  #
  # ```ruby
  # File.new("testfile").isatty   #=> false
  # File.new("/dev/tty").isatty   #=> true
  # ```
  #
  #
  # Alias for:
  # [`isatty`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-isatty)
  sig {returns(T::Boolean)}
  def tty?(); end

  # Pushes back bytes (passed as a parameter) onto *ios*, such that a subsequent
  # buffered read will return it. It is only guaranteed to support a single
  # byte, and only if ungetbyte or ungetc has not already been called on *ios*
  # since the previous read of at least a single byte from *ios*. However, it
  # can support additional bytes if there is space in the internal buffer to
  # allow for it.
  #
  # ```ruby
  # f = File.new("testfile")   #=> #<File:testfile>
  # b = f.getbyte              #=> 0x38
  # f.ungetbyte(b)             #=> nil
  # f.getbyte                  #=> 0x38
  # ```
  #
  # If given an integer, only uses the lower 8 bits of the integer as the byte
  # to push.
  #
  # ```ruby
  # f = File.new("testfile")   #=> #<File:testfile>
  # f.ungetbyte(0x102)         #=> nil
  # f.getbyte                  #=> 0x2
  # ```
  #
  # Calling this method prepends to the existing buffer, even if the method has
  # already been called previously:
  #
  # ```ruby
  # f = File.new("testfile")   #=> #<File:testfile>
  # f.ungetbyte("ab")          #=> nil
  # f.ungetbyte("cd")          #=> nil
  # f.read(5)                  #=> "cdab8"
  # ```
  #
  # Has no effect with unbuffered reads (such as
  # [`IO#sysread`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sysread)).
  sig do
    params(
        arg0: T.any(String, Integer),
    )
    .returns(NilClass)
  end
  def ungetbyte(arg0); end

  # Pushes back characters (passed as a parameter) onto *ios*, such that a
  # subsequent buffered read will return it. It is only guaranteed to support a
  # single byte, and only if ungetbyte or ungetc has not already been called on
  # *ios* since the previous read of at least a single byte from *ios*. However,
  # it can support additional bytes if there is space in the internal buffer to
  # allow for it.
  #
  # ```ruby
  # f = File.new("testfile")   #=> #<File:testfile>
  # c = f.getc                 #=> "8"
  # f.ungetc(c)                #=> nil
  # f.getc                     #=> "8"
  # ```
  #
  # If given an integer, the integer must represent a valid codepoint in the
  # external encoding of *ios*.
  #
  # Calling this method prepends to the existing buffer, even if the method has
  # already been called previously:
  #
  # ```ruby
  # f = File.new("testfile")   #=> #<File:testfile>
  # f.ungetc("ab")             #=> nil
  # f.ungetc("cd")             #=> nil
  # f.read(5)                  #=> "cdab8"
  # ```
  #
  # Has no effect with unbuffered reads (such as
  # [`IO#sysread`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sysread)).
  sig do
    params(
        arg0: String,
    )
    .returns(NilClass)
  end
  def ungetc(arg0); end

  # Writes each of the given `objects` to `self`, which must be opened for
  # writing (see [Modes](#class-IO-label-Modes)); returns the total number bytes
  # written; each of `objects` that is not a string is converted via method
  # `to_s`:
  #
  # ```ruby
  # $stdout.write('Hello', ', ', 'World!', "\n") # => 14
  # $stdout.write('foo', :bar, 2, "\n")          # => 8
  # ```
  #
  # Output:
  #
  # ```
  # Hello, World!
  # foobar2
  # ```
  sig do
    params(
        arg0: Object,
    )
    .returns(Integer)
  end
  def write(arg0); end

  # Opens the file, optionally seeks to the given *offset*, then returns
  # *length* bytes (defaulting to the rest of the file). binread ensures the
  # file is closed before returning. The open mode would be `"rb:ASCII-8BIT"`.
  #
  # If `name` starts with a pipe character (`"|"`) and the receiver is the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) class, a subprocess is
  # created in the same way as
  # [`Kernel#open`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-open),
  # and its output is returned. Consider to use
  # [`File.binread`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-binread)
  # to disable the behavior of subprocess invocation.
  #
  # ```ruby
  # File.binread("testfile")           #=> "This is line one\nThis is line two\nThis is line three\nAnd so on...\n"
  # File.binread("testfile", 20)       #=> "This is line one\nThi"
  # File.binread("testfile", 20, 10)   #=> "ne one\nThis is line "
  # IO.binread("| cat testfile")       #=> "This is line one\nThis is line two\nThis is line three\nAnd so on...\n"
  # ```
  #
  # See also
  # [`IO.read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-read) for
  # details about `name` and open\_args.
  sig do
    params(
        name: String,
        length: Integer,
        offset: Integer,
    )
    .returns(String)
  end
  def self.binread(name, length=T.unsafe(nil), offset=T.unsafe(nil)); end

  # Same as
  # [`IO.write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-write)
  # except opening the file in binary mode and ASCII-8BIT encoding
  # (`"wb:ASCII-8BIT"`).
  #
  # If `name` starts with a pipe character (`"|"`) and the receiver is the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) class, a subprocess is
  # created in the same way as
  # [`Kernel#open`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-open),
  # and its output is returned. Consider to use
  # [`File.binwrite`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-binwrite)
  # to disable the behavior of subprocess invocation.
  #
  # See also
  # [`IO.read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-read) for
  # details about `name` and open\_args.
  sig do
    params(
        name: T.any(String, Pathname),
        arg0: String,
        offset: Integer,
        external_encoding: T.any(String, Encoding),
        internal_encoding: T.any(String, Encoding),
        encoding: T.any(String, Encoding),
        textmode: BasicObject,
        binmode: BasicObject,
        autoclose: BasicObject,
        mode: String,
    )
    .returns(Integer)
  end
  def self.binwrite(name, arg0, offset=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil)); end

  # [`IO.copy_stream`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-copy_stream)
  # copies *src* to *dst*. *src* and *dst* is either a filename or an IO-like
  # object. IO-like object for *src* should have
  # [`readpartial`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-readpartial)
  # or [`read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read)
  # method. IO-like object for *dst* should have
  # [`write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write)
  # method. (Specialized mechanisms, such as sendfile system call, may be used
  # on appropriate situation.)
  #
  # This method returns the number of bytes copied.
  #
  # If optional arguments are not given, the start position of the copy is the
  # beginning of the filename or the current file offset of the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html). The end position of the
  # copy is the end of file.
  #
  # If *copy\_length* is given, No more than *copy\_length* bytes are copied.
  #
  # If *src\_offset* is given, it specifies the start position of the copy.
  #
  # When *src\_offset* is specified and *src* is an
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html),
  # [`IO.copy_stream`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-copy_stream)
  # doesn't move the current file offset.
  sig do
    params(
        src: T.any(String, IO, Tempfile),
        dst: T.any(String, IO, Tempfile),
        copy_length: Integer,
        src_offset: Integer,
    )
    .returns(Integer)
  end
  def self.copy_stream(src, dst, copy_length=T.unsafe(nil), src_offset=T.unsafe(nil)); end

  # Executes the block for every line in the named I/O port, where lines are
  # separated by *sep*.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # If `name` starts with a pipe character (`"|"`) and the receiver is the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) class, a subprocess is
  # created in the same way as
  # [`Kernel#open`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-open),
  # and its output is returned. Consider to use
  # [`File.foreach`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-foreach)
  # to disable the behavior of subprocess invocation.
  #
  # ```ruby
  # File.foreach("testfile") {|x| print "GOT ", x }
  # IO.foreach("| cat testfile") {|x| print "GOT ", x }
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # GOT This is line one
  # GOT This is line two
  # GOT This is line three
  # GOT And so on...
  # ```
  #
  # If the last argument is a hash, it's the keyword argument to open. See
  # [`IO.readlines`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-readlines)
  # for details about getline\_args. And see also
  # [`IO.read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-read) for
  # details about open\_args.
  def self.foreach(*_); end

  ### https://ruby-doc.org/core-2.3.0/IO.html#method-c-popen
  ### This signature is very hard to type. I'm giving up and making it untyped.
  ### As far as I can tell, at least one arg is required, and it must be an array,
  ### but sometimes it's the first arg and sometimes it's the second arg, so
  ### let's just make everything untyped.
  ### Have to declare this as a rest arg, because pay-server runtime
  ### reflection sees it this way. Once it's out of the missing method file, we
  ### can add a better sig here.
  # Runs the specified command as a subprocess; the subprocess's standard input
  # and output will be connected to the returned
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object.
  #
  # The PID of the started process can be obtained by
  # [`IO#pid`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-pid) method.
  #
  # *cmd* is a string or an array as follows.
  #
  # ```
  # cmd:
  #   "-"                                      : fork
  #   commandline                              : command line string which is passed to a shell
  #   [env, cmdname, arg1, ..., opts]          : command name and zero or more arguments (no shell)
  #   [env, [cmdname, argv0], arg1, ..., opts] : command name, argv[0] and zero or more arguments (no shell)
  # (env and opts are optional.)
  # ```
  #
  # If *cmd* is a `String` "`-`", then a new instance of Ruby is started as the
  # subprocess.
  #
  # If *cmd* is an `Array` of `String`, then it will be used as the subprocess's
  # `argv` bypassing a shell. The array can contain a hash at first for
  # environments and a hash at last for options similar to
  # [`spawn`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-spawn).
  #
  # The default mode for the new file object is "r", but *mode* may be set to
  # any of the modes listed in the description for class
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html). The last argument *opt*
  # qualifies *mode*.
  #
  # ```ruby
  # # set IO encoding
  # IO.popen("nkf -e filename", :external_encoding=>"EUC-JP") {|nkf_io|
  #   euc_jp_string = nkf_io.read
  # }
  #
  # # merge standard output and standard error using
  # # spawn option.  See the document of Kernel.spawn.
  # IO.popen(["ls", "/", :err=>[:child, :out]]) {|ls_io|
  #   ls_result_with_error = ls_io.read
  # }
  #
  # # spawn options can be mixed with IO options
  # IO.popen(["ls", "/"], :err=>[:child, :out]) {|ls_io|
  #   ls_result_with_error = ls_io.read
  # }
  # ```
  #
  # Raises exceptions which
  # [`IO.pipe`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-pipe) and
  # [`Kernel.spawn`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-spawn)
  # raise.
  #
  # If a block is given, Ruby will run the command as a child connected to Ruby
  # with a pipe. Ruby's end of the pipe will be passed as a parameter to the
  # block. At the end of block, Ruby closes the pipe and sets `$?`. In this case
  # [`IO.popen`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-popen)
  # returns the value of the block.
  #
  # If a block is given with a *cmd* of "`-`", the block will be run in two
  # separate processes: once in the parent, and once in a child. The parent
  # process will be passed the pipe object as a parameter to the block, the
  # child version of the block will be passed `nil`, and the child's standard in
  # and standard out will be connected to the parent through the pipe. Not
  # available on all platforms.
  #
  # ```ruby
  # f = IO.popen("uname")
  # p f.readlines
  # f.close
  # puts "Parent is #{Process.pid}"
  # IO.popen("date") {|f| puts f.gets }
  # IO.popen("-") {|f| $stderr.puts "#{Process.pid} is here, f is #{f.inspect}"}
  # p $?
  # IO.popen(%w"sed -e s|^|<foo>| -e s&$&;zot;&", "r+") {|f|
  #   f.puts "bar"; f.close_write; puts f.gets
  # }
  # ```
  #
  # *produces:*
  #
  # ```
  # ["Linux\n"]
  # Parent is 21346
  # Thu Jan 15 22:41:19 JST 2009
  # 21346 is here, f is #<IO:fd 3>
  # 21352 is here, f is nil
  # #<Process::Status: pid 21352 exit 0>
  # <foo>bar;zot;
  # ```
  sig do
    params(
        args: T.untyped,
        blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def self.popen(*args, &blk); end

  # Opens the file, optionally seeks to the given `offset`, then returns
  # `length` bytes (defaulting to the rest of the file).
  # [`read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read) ensures
  # the file is closed before returning.
  #
  # If `name` starts with a pipe character (`"|"`) and the receiver is the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) class, a subprocess is
  # created in the same way as
  # [`Kernel#open`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-open),
  # and its output is returned. Consider to use
  # [`File.read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-read) to
  # disable the behavior of subprocess invocation.
  #
  # ### Options
  #
  # The options hash accepts the following keys:
  #
  # :encoding
  # :   string or encoding
  #
  #     Specifies the encoding of the read string. `:encoding` will be ignored
  #     if `length` is specified. See
  #     [`Encoding.aliases`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html#method-c-aliases)
  #     for possible encodings.
  #
  # :mode
  # :   string or integer
  #
  #     Specifies the *mode* argument for open(). It must start with an "r",
  #     otherwise it will cause an error. See
  #     [`IO.new`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-new) for
  #     the list of possible modes.
  #
  # :open\_args
  # :   array
  #
  #     Specifies arguments for open() as an array. This key can not be used in
  #     combination with either `:encoding` or `:mode`.
  #
  #
  # Examples:
  #
  # ```ruby
  # File.read("testfile")            #=> "This is line one\nThis is line two\nThis is line three\nAnd so on...\n"
  # File.read("testfile", 20)        #=> "This is line one\nThi"
  # File.read("testfile", 20, 10)    #=> "ne one\nThis is line "
  # File.read("binfile", mode: "rb") #=> "\xF7\x00\x00\x0E\x12"
  # IO.read("|ls -a")                #=> ".\n..\n"...
  # ```
  sig do
    params(
        name: T.any(String, Tempfile, File, Pathname),
        length: Integer,
        offset: Integer,
        external_encoding: T.any(String, Encoding),
        internal_encoding: T.any(String, Encoding),
        encoding: T.any(String, Encoding),
        textmode: BasicObject,
        binmode: BasicObject,
        autoclose: BasicObject,
        mode: String,
    )
    .returns(String)
  end
  def self.read(name, length=T.unsafe(nil), offset=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil)); end

  # Reads the entire file specified by *name* as individual lines, and returns
  # those lines in an array. Lines are separated by *sep*.
  #
  # If `name` starts with a pipe character (`"|"`) and the receiver is the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) class, a subprocess is
  # created in the same way as
  # [`Kernel#open`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-open),
  # and its output is returned. Consider to use
  # [`File.readlines`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-readlines)
  # to disable the behavior of subprocess invocation.
  #
  # ```ruby
  # a = File.readlines("testfile")
  # a[0]   #=> "This is line one\n"
  #
  # b = File.readlines("testfile", chomp: true)
  # b[0]   #=> "This is line one"
  #
  # IO.readlines("|ls -a")     #=> [".\n", "..\n", ...]
  # ```
  #
  # If the last argument is a hash, it's the keyword argument to open.
  #
  # ### Options for getline
  #
  # The options hash accepts the following keys:
  #
  # :chomp
  # :   When the optional `chomp` keyword argument has a true value, `\n`, `\r`,
  #     and `\r\n` will be removed from the end of each line.
  #
  #
  # See also
  # [`IO.read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-read) for
  # details about `name` and open\_args.
  sig do
    params(
        name: T.any(String, Tempfile, File, Pathname),
        sep: String,
        limit: Integer,
        external_encoding: T.any(String, Encoding),
        internal_encoding: T.any(String, Encoding),
        encoding: T.any(String, Encoding),
        textmode: BasicObject,
        binmode: BasicObject,
        autoclose: BasicObject,
        mode: String,
        chomp: T::Boolean
    )
    .returns(T::Array[String])
  end
  def self.readlines(name, sep=T.unsafe(nil), limit=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil), chomp: T.unsafe(nil)); end

  # Calls select(2) system call. It monitors given arrays of
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects, waits until one
  # or more of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects are
  # ready for reading, are ready for writing, and have pending exceptions
  # respectively, and returns an array that contains arrays of those
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects. It will return
  # `nil` if optional *timeout* value is given and no
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object is ready in
  # *timeout* seconds.
  #
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # peeks the buffer of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # objects for testing readability. If the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) buffer is not empty,
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # immediately notifies readability. This "peek" only happens for
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects. It does not
  # happen for IO-like objects such as
  # [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html).
  #
  # The best way to use
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # is invoking it after nonblocking methods such as
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read_nonblock),
  # [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write_nonblock),
  # etc. The methods raise an exception which is extended by
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # or
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html).
  # The modules notify how the caller should wait with
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select).
  # If
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # is raised, the caller should wait for reading. If
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # is raised, the caller should wait for writing.
  #
  # So, blocking read
  # ([`readpartial`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-readpartial))
  # can be emulated using
  # [`read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read_nonblock)
  # and
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # as follows:
  #
  # ```ruby
  # begin
  #   result = io_like.read_nonblock(maxlen)
  # rescue IO::WaitReadable
  #   IO.select([io_like])
  #   retry
  # rescue IO::WaitWritable
  #   IO.select(nil, [io_like])
  #   retry
  # end
  # ```
  #
  # Especially, the combination of nonblocking methods and
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # is preferred for [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) like
  # objects such as
  # [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html).
  # It has [`to_io`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-to_io)
  # method to return underlying
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object.
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # calls [`to_io`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-to_io)
  # to obtain the file descriptor to wait.
  #
  # This means that readability notified by
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # doesn't mean readability from
  # [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html)
  # object.
  #
  # The most likely situation is that
  # [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html)
  # buffers some data.
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # doesn't see the buffer. So
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # can block when
  # [`OpenSSL::SSL::SSLSocket#readpartial`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-readpartial)
  # doesn't block.
  #
  # However, several more complicated situations exist.
  #
  # SSL is a protocol which is sequence of records. The record consists of
  # multiple bytes. So, the remote side of SSL sends a partial record,
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # notifies readability but
  # [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html)
  # cannot decrypt a byte and
  # [`OpenSSL::SSL::SSLSocket#readpartial`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-readpartial)
  # will block.
  #
  # Also, the remote side can request SSL renegotiation which forces the local
  # SSL engine to write some data. This means
  # [`OpenSSL::SSL::SSLSocket#readpartial`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-readpartial)
  # may invoke
  # [`write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write) system
  # call and it can block. In such a situation,
  # [`OpenSSL::SSL::SSLSocket#read_nonblock`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/Buffering.html#method-i-read_nonblock)
  # raises
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # instead of blocking. So, the caller should wait for ready for writability as
  # above example.
  #
  # The combination of nonblocking methods and
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # is also useful for streams such as tty, pipe socket socket when multiple
  # processes read from a stream.
  #
  # Finally, Linux kernel developers don't guarantee that readability of
  # select(2) means readability of following read(2) even for a single process.
  # See select(2) manual on GNU/Linux system.
  #
  # Invoking
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # before
  # [`IO#readpartial`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-readpartial)
  # works well as usual. However it is not the best way to use
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select).
  #
  # The writability notified by select(2) doesn't show how many bytes are
  # writable.
  # [`IO#write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write)
  # method blocks until given whole string is written. So, `IO#write(two or more
  # bytes)` can block after writability is notified by
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select).
  # [`IO#write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write_nonblock)
  # is required to avoid the blocking.
  #
  # Blocking write
  # ([`write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write)) can
  # be emulated using
  # [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write_nonblock)
  # and
  # [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select)
  # as follows:
  # [`IO::WaitReadable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitReadable.html)
  # should also be rescued for SSL renegotiation in
  # [`OpenSSL::SSL::SSLSocket`](https://docs.ruby-lang.org/en/2.7.0/OpenSSL/SSL/SSLSocket.html).
  #
  # ```ruby
  # while 0 < string.bytesize
  #   begin
  #     written = io_like.write_nonblock(string)
  #   rescue IO::WaitReadable
  #     IO.select([io_like])
  #     retry
  #   rescue IO::WaitWritable
  #     IO.select(nil, [io_like])
  #     retry
  #   end
  #   string = string.byteslice(written..-1)
  # end
  # ```
  #
  # ### Parameters
  # read\_array
  # :   an array of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects
  #     that wait until ready for read
  # write\_array
  # :   an array of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects
  #     that wait until ready for write
  # error\_array
  # :   an array of [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) objects
  #     that wait for exceptions
  # timeout
  # :   a numeric value in second
  #
  #
  # ### Example
  #
  # ```ruby
  # rp, wp = IO.pipe
  # mesg = "ping "
  # 100.times {
  #   # IO.select follows IO#read.  Not the best way to use IO.select.
  #   rs, ws, = IO.select([rp], [wp])
  #   if r = rs[0]
  #     ret = r.read(5)
  #     print ret
  #     case ret
  #     when /ping/
  #       mesg = "pong\n"
  #     when /pong/
  #       mesg = "ping "
  #     end
  #   end
  #   if w = ws[0]
  #     w.write(mesg)
  #   end
  # }
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # ping pong
  # ping pong
  # ping pong
  # (snipped)
  # ping
  # ```
  sig do
    params(
        read_array: T.nilable(T::Array[IO]),
        write_array: T.nilable(T::Array[IO]),
        error_array: T.nilable(T::Array[IO]),
        timeout: T.any(NilClass, Integer, Float),
    )
    .returns(T.nilable(T::Array[T::Array[IO]]))
  end
  def self.select(read_array, write_array=nil, error_array=nil, timeout=nil); end

  # Opens the given path, returning the underlying file descriptor as a
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  #
  # ```ruby
  # IO.sysopen("testfile")   #=> 3
  # ```
  sig do
    params(
        path: T.any(String, Tempfile, File, Pathname),
        mode: String,
        perm: String,
    )
    .returns(Integer)
  end
  def self.sysopen(path, mode=T.unsafe(nil), perm=T.unsafe(nil)); end

  # Attempts to convert `object` into an
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object via method
  # `to_io`; returns the new [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object if successful, or `nil` otherwise:
  #
  # ```ruby
  # IO.try_convert(STDOUT)   # => #<IO:<STDOUT>>
  # IO.try_convert(ARGF)     # => #<IO:<STDIN>>
  # IO.try_convert('STDOUT') # => nil
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.nilable(IO))
  end
  def self.try_convert(arg0); end

  # Opens the file, optionally seeks to the given *offset*, writes *string*,
  # then returns the length written.
  # [`write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write)
  # ensures the file is closed before returning. If *offset* is not given in
  # write mode, the file is truncated. Otherwise, it is not truncated.
  #
  # If `name` starts with a pipe character (`"|"`) and the receiver is the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) class, a subprocess is
  # created in the same way as
  # [`Kernel#open`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-open),
  # and its output is returned. Consider to use
  # [`File.write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-write)
  # to disable the behavior of subprocess invocation.
  #
  # ```ruby
  # File.write("testfile", "0123456789", 20)  #=> 10
  # # File could contain:  "This is line one\nThi0123456789two\nThis is line three\nAnd so on...\n"
  # File.write("testfile", "0123456789")      #=> 10
  # # File would now read: "0123456789"
  # IO.write("|tr a-z A-Z", "abc")            #=> 3
  # # Prints "ABC" to the standard output
  # ```
  #
  # If the last argument is a hash, it specifies options for the internal
  # open(). It accepts the following keys:
  #
  # :encoding
  # :   string or encoding
  #
  #     Specifies the encoding of the read string. See
  #     [`Encoding.aliases`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html#method-c-aliases)
  #     for possible encodings.
  #
  # :mode
  # :   string or integer
  #
  #     Specifies the *mode* argument for open(). It must start with "w", "a",
  #     or "r+", otherwise it will cause an error. See
  #     [`IO.new`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-new) for
  #     the list of possible modes.
  #
  # :perm
  # :   integer
  #
  #     Specifies the *perm* argument for open().
  #
  # :open\_args
  # :   array
  #
  #     Specifies arguments for open() as an array. This key can not be used in
  #     combination with other keys.
  #
  #
  # See also
  # [`IO.read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-read) for
  # details about `name` and open\_args.
  sig do
    params(
        name: T.any(String, Tempfile, File, Pathname),
        string: Object,
        offset: Integer,
        external_encoding: T.any(String, Encoding),
        internal_encoding: T.any(String, Encoding),
        encoding: T.any(String, Encoding),
        textmode: BasicObject,
        binmode: BasicObject,
        autoclose: BasicObject,
        mode: String,
        perm: Integer
    )
    .returns(Integer)
  end
  def self.write(name, string, offset=T.unsafe(nil), external_encoding: T.unsafe(nil), internal_encoding: T.unsafe(nil), encoding: T.unsafe(nil), textmode: T.unsafe(nil), binmode: T.unsafe(nil), autoclose: T.unsafe(nil), mode: T.unsafe(nil), perm: T.unsafe(nil)); end

  # Synonym for
  # [`IO.new`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-new).
  sig do
    params(
        fd: Integer,
        mode: Integer,
        opt: Integer,
    )
    .returns(T.self_type)
  end
  def self.for_fd(fd, mode=T.unsafe(nil), opt=T.unsafe(nil)); end

  # This is a deprecated alias for
  # [`each_byte`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_byte).
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def bytes(&blk); end

  # This is a deprecated alias for
  # [`each_char`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_char).
  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[String])}
  def chars(&blk); end

  # This is a deprecated alias for
  # [`each_codepoint`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_codepoint).
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Integer])}
  def codepoints(&blk); end

  # Executes the block for every line in *ios*, where lines are separated by
  # *sep*. *ios* must be opened for reading or an
  # [`IOError`](https://docs.ruby-lang.org/en/2.7.0/IOError.html) will be
  # raised.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # f = File.new("testfile")
  # f.each {|line| puts "#{f.lineno}: #{line}" }
  # ```
  #
  # *produces:*
  #
  # ```
  # 1: This is line one
  # 2: This is line two
  # 3: This is line three
  # 4: And so on...
  # ```
  #
  # See
  # [`IO.readlines`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-readlines)
  # for details about getline\_args.
  #
  # Alias for:
  # [`each`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each)
  sig do
    params(
        sep: String,
        limit: Integer,
        chomp: T::Boolean,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig do
    params(
        sep: String,
        limit: Integer,
        chomp: T::Boolean,
    )
    .returns(T::Enumerator[String])
  end
  def each_line(sep=T.unsafe(nil), limit=T.unsafe(nil), chomp: false, &blk); end

  # Returns `true` if the stream is positioned at its end, `false` otherwise;
  # see [Position](#class-IO-label-Position):
  #
  # ```ruby
  # f = File.open('t.txt')
  # f.eof           # => false
  # f.seek(0, :END) # => 0
  # f.eof           # => true
  # ```
  #
  # Raises an exception unless the stream is opened for reading; see
  # [Mode](#class-IO-label-Mode).
  #
  # If `self` is a stream such as pipe or socket, this method blocks until the
  # other end sends some data or closes it:
  #
  # ```ruby
  # r, w = IO.pipe
  # Thread.new { sleep 1; w.close }
  # r.eof? # => true # After 1-second wait.
  #
  # r, w = IO.pipe
  # Thread.new { sleep 1; w.puts "a" }
  # r.eof?  # => false # After 1-second wait.
  #
  # r, w = IO.pipe
  # r.eof?  # blocks forever
  # ```
  #
  # Note that this method reads data to the input byte buffer. So
  # [`IO#sysread`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sysread)
  # may not behave as you intend with
  # [`IO#eof?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof-3F),
  # unless you call
  # [`IO#rewind`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-rewind)
  # first (which is not available for some streams).
  #
  # I#eof? is an alias for
  # [`IO#eof`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof).
  #
  # Alias for: [`eof`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-eof)
  sig {returns(T::Boolean)}
  def eof?(); end

  # This is a deprecated alias for
  # [`each_line`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_line).
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
  def lines(sep=T.unsafe(nil), limit=T.unsafe(nil), &blk); end

  # Returns the integer file descriptor for the stream:
  #
  # ```ruby
  # $stdin.fileno             # => 0
  # $stdout.fileno            # => 1
  # $stderr.fileno            # => 2
  # File.open('t.txt').fileno # => 10
  # ```
  #
  # [`IO#to_i`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-to_i) is an
  # alias for
  # [`IO#fileno`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-fileno).
  #
  # Alias for:
  # [`fileno`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-fileno)
  sig {returns(Integer)}
  def to_i(); end

  # Writes the given string to *ios* using the write(2) system call after
  # O\_NONBLOCK is set for the underlying file descriptor.
  #
  # It returns the number of bytes written.
  #
  # [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write_nonblock)
  # just calls the write(2) system call. It causes all errors the write(2)
  # system call causes: Errno::EWOULDBLOCK, Errno::EINTR, etc. The result may
  # also be smaller than string.length (partial write). The caller should care
  # such errors and partial write.
  #
  # If the exception is Errno::EWOULDBLOCK or Errno::EAGAIN, it is extended by
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html).
  # So
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # can be used to rescue the exceptions for retrying write\_nonblock.
  #
  # ```ruby
  # # Creates a pipe.
  # r, w = IO.pipe
  #
  # # write_nonblock writes only 65536 bytes and return 65536.
  # # (The pipe size is 65536 bytes on this environment.)
  # s = "a" * 100000
  # p w.write_nonblock(s)     #=> 65536
  #
  # # write_nonblock cannot write a byte and raise EWOULDBLOCK (EAGAIN).
  # p w.write_nonblock("b")   # Resource temporarily unavailable (Errno::EAGAIN)
  # ```
  #
  # If the write buffer is not empty, it is flushed at first.
  #
  # When
  # [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write_nonblock)
  # raises an exception kind of
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html),
  # [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write_nonblock)
  # should not be called until io is writable for avoiding busy loop. This can
  # be done as follows.
  #
  # ```ruby
  # begin
  #   result = io.write_nonblock(string)
  # rescue IO::WaitWritable, Errno::EINTR
  #   IO.select(nil, [io])
  #   retry
  # end
  # ```
  #
  # Note that this doesn't guarantee to write all data in string. The length
  # written is reported as result and it should be checked later.
  #
  # On some platforms such as Windows,
  # [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write_nonblock)
  # is not supported according to the kind of the
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object. In such cases,
  # [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write_nonblock)
  # raises `Errno::EBADF`.
  #
  # By specifying a keyword argument *exception* to `false`, you can indicate
  # that
  # [`write_nonblock`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write_nonblock)
  # should not raise an
  # [`IO::WaitWritable`](https://docs.ruby-lang.org/en/2.7.0/IO/WaitWritable.html)
  # exception, but return the symbol `:wait_writable` instead.
  sig { params(string: Object, exception: T::Boolean).returns(Integer) }
  def write_nonblock(string, exception: true); end
end

# exception to wait for reading by EAGAIN. see
# [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select).
class IO::EAGAINWaitReadable < Errno::EAGAIN
  include IO::WaitReadable
  Errno = T.let(nil, Integer)
end

# exception to wait for writing by EAGAIN. see
# [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select).
class IO::EAGAINWaitWritable < Errno::EAGAIN
  include IO::WaitWritable
  Errno = T.let(nil, Integer)
end

# exception to wait for reading by EINPROGRESS. see
# [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select).
class IO::EINPROGRESSWaitReadable < Errno::EINPROGRESS
  include IO::WaitReadable
  Errno = T.let(nil, Integer)
end

# exception to wait for writing by EINPROGRESS. see
# [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select).
class IO::EINPROGRESSWaitWritable < Errno::EINPROGRESS
  include IO::WaitWritable
  Errno = T.let(nil, Integer)
end

# exception to wait for reading. see
# [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select).
module IO::WaitReadable
end

# exception to wait for writing. see
# [`IO.select`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-select).
module IO::WaitWritable
end
