# typed: __STDLIB_INTERNAL

# This module provides access to the [zlib library](http://zlib.net).
# [`Zlib`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html) is designed to be a
# portable, free, general-purpose, legally unencumbered -- that is, not covered
# by any patents -- lossless data-compression library for use on virtually any
# computer hardware and operating system.
#
# The zlib compression library provides in-memory compression and decompression
# functions, including integrity checks of the uncompressed data.
#
# The zlib compressed data format is described in RFC 1950, which is a wrapper
# around a deflate stream which is described in RFC 1951.
#
# The library also supports reading and writing files in gzip (.gz) format with
# an interface similar to that of
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html). The gzip format is
# described in RFC 1952 which is also a wrapper around a deflate stream.
#
# The zlib format was designed to be compact and fast for use in memory and on
# communications channels. The gzip format was designed for single-file
# compression on file systems, has a larger header than zlib to maintain
# directory information, and uses a different, slower check method than zlib.
#
# See your system's zlib.h for further information about zlib
#
# ## Sample usage
#
# Using the wrapper to compress strings with default parameters is quite simple:
#
# ```ruby
# require "zlib"
#
# data_to_compress = File.read("don_quixote.txt")
#
# puts "Input size: #{data_to_compress.size}"
# #=> Input size: 2347740
#
# data_compressed = Zlib::Deflate.deflate(data_to_compress)
#
# puts "Compressed size: #{data_compressed.size}"
# #=> Compressed size: 887238
#
# uncompressed_data = Zlib::Inflate.inflate(data_compressed)
#
# puts "Uncompressed data is: #{uncompressed_data}"
# #=> Uncompressed data is: The Project Gutenberg EBook of Don Quixote...
# ```
#
# ## [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) tree
#
# *   [`Zlib::Deflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Deflate.html)
# *   [`Zlib::Inflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Inflate.html)
# *   [`Zlib::ZStream`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html)
# *   [`Zlib::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Error.html)
#     *   [`Zlib::StreamEnd`](https://docs.ruby-lang.org/en/2.7.0/Zlib/StreamEnd.html)
#     *   [`Zlib::NeedDict`](https://docs.ruby-lang.org/en/2.7.0/Zlib/NeedDict.html)
#     *   [`Zlib::DataError`](https://docs.ruby-lang.org/en/2.7.0/Zlib/DataError.html)
#     *   [`Zlib::StreamError`](https://docs.ruby-lang.org/en/2.7.0/Zlib/StreamError.html)
#     *   [`Zlib::MemError`](https://docs.ruby-lang.org/en/2.7.0/Zlib/MemError.html)
#     *   [`Zlib::BufError`](https://docs.ruby-lang.org/en/2.7.0/Zlib/BufError.html)
#     *   [`Zlib::VersionError`](https://docs.ruby-lang.org/en/2.7.0/Zlib/VersionError.html)
#
#
#
# (if you have GZIP\_SUPPORT)
# *   [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
# *   [`Zlib::GzipWriter`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html)
# *   [`Zlib::GzipFile`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html)
# *   [`Zlib::GzipFile::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile/Error.html)
#     *   [`Zlib::GzipFile::LengthError`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile/LengthError.html)
#     *   [`Zlib::GzipFile::CRCError`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile/CRCError.html)
#     *   [`Zlib::GzipFile::NoFooter`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile/NoFooter.html)
module Zlib
  # Represents text data as guessed by deflate.
  #
  # NOTE: The underlying constant Z\_ASCII was deprecated in favor of Z\_TEXT in
  # zlib 1.2.2.  New applications should not use this constant.
  #
  # See
  # [`Zlib::Deflate#data_type`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-data_type).
  ASCII = T.let(T.unsafe(nil), Integer)

  # Slowest compression level, but with the best space savings.
  BEST_COMPRESSION = T.let(T.unsafe(nil), Integer)

  # Fastest compression level, but with the lowest space savings.
  BEST_SPEED = T.let(T.unsafe(nil), Integer)

  # Represents binary data as guessed by deflate.
  #
  # See
  # [`Zlib::Deflate#data_type`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-data_type).
  BINARY = T.let(T.unsafe(nil), Integer)

  # Default compression level which is a good trade-off between space and time
  DEFAULT_COMPRESSION = T.let(T.unsafe(nil), Integer)

  # Default deflate strategy which is used for normal data.
  DEFAULT_STRATEGY = T.let(T.unsafe(nil), Integer)

  # The default memory level for allocating zlib deflate compression state.
  DEF_MEM_LEVEL = T.let(T.unsafe(nil), Integer)

  # Deflate strategy for data produced by a filter (or predictor). The effect of
  # [`FILTERED`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#FILTERED) is to
  # force more Huffman codes and less string matching; it is somewhat
  # intermediate between
  # [`DEFAULT_STRATEGY`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#DEFAULT_STRATEGY)
  # and
  # [`HUFFMAN_ONLY`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#HUFFMAN_ONLY).
  # Filtered data consists mostly of small values with a somewhat random
  # distribution.
  FILTERED = T.let(T.unsafe(nil), Integer)

  # Processes all pending input and flushes pending output.
  FINISH = T.let(T.unsafe(nil), Integer)

  # Deflate strategy which prevents the use of dynamic Huffman codes, allowing
  # for a simpler decoder for specialized applications.
  FIXED = T.let(T.unsafe(nil), Integer)

  # Flushes all output as with
  # [`SYNC_FLUSH`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#SYNC_FLUSH),
  # and the compression state is reset so that decompression can restart from
  # this point if previous compressed data has been damaged or if random access
  # is desired. Like
  # [`SYNC_FLUSH`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#SYNC_FLUSH),
  # using
  # [`FULL_FLUSH`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#FULL_FLUSH) too
  # often can seriously degrade compression.
  FULL_FLUSH = T.let(T.unsafe(nil), Integer)

  # Deflate strategy which uses Huffman codes only (no string matching).
  HUFFMAN_ONLY = T.let(T.unsafe(nil), Integer)

  # The maximum memory level for allocating zlib deflate compression state.
  MAX_MEM_LEVEL = T.let(T.unsafe(nil), Integer)

  # The maximum size of the zlib history buffer. Note that zlib allows larger
  # values to enable different inflate modes. See
  # [`Zlib::Inflate.new`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Inflate.html#method-c-new)
  # for details.
  MAX_WBITS = T.let(T.unsafe(nil), Integer)

  # No compression, passes through data untouched. Use this for appending
  # pre-compressed data to a deflate stream.
  NO_COMPRESSION = T.let(T.unsafe(nil), Integer)

  # [`NO_FLUSH`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#NO_FLUSH) is the
  # default flush method and allows deflate to decide how much data to
  # accumulate before producing output in order to maximize compression.
  NO_FLUSH = T.let(T.unsafe(nil), Integer)

  # OS code for Amiga hosts
  OS_AMIGA = T.let(T.unsafe(nil), Integer)

  # OS code for Atari hosts
  OS_ATARI = T.let(T.unsafe(nil), Integer)

  # The OS code of current host
  OS_CODE = T.let(T.unsafe(nil), Integer)

  # OS code for CP/M hosts
  OS_CPM = T.let(T.unsafe(nil), Integer)

  # OS code for Mac OS hosts
  OS_MACOS = T.let(T.unsafe(nil), Integer)

  # OS code for MSDOS hosts
  OS_MSDOS = T.let(T.unsafe(nil), Integer)

  # OS code for OS2 hosts
  OS_OS2 = T.let(T.unsafe(nil), Integer)

  # OS code for QDOS hosts
  OS_QDOS = T.let(T.unsafe(nil), Integer)

  # OS code for RISC OS hosts
  OS_RISCOS = T.let(T.unsafe(nil), Integer)

  # OS code for TOPS-20 hosts
  OS_TOPS20 = T.let(T.unsafe(nil), Integer)

  # OS code for UNIX hosts
  OS_UNIX = T.let(T.unsafe(nil), Integer)

  # OS code for unknown hosts
  OS_UNKNOWN = T.let(T.unsafe(nil), Integer)

  # OS code for VM OS hosts
  OS_VMCMS = T.let(T.unsafe(nil), Integer)

  # OS code for VMS hosts
  OS_VMS = T.let(T.unsafe(nil), Integer)

  # OS code for Win32 hosts
  OS_WIN32 = T.let(T.unsafe(nil), Integer)

  # OS code for Z-System hosts
  OS_ZSYSTEM = T.let(T.unsafe(nil), Integer)

  # Deflate compression strategy designed to be almost as fast as
  # [`HUFFMAN_ONLY`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#HUFFMAN_ONLY),
  # but give better compression for PNG image data.
  RLE = T.let(T.unsafe(nil), Integer)

  # The [`SYNC_FLUSH`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#SYNC_FLUSH)
  # method flushes all pending output to the output buffer and the output is
  # aligned on a byte boundary. Flushing may degrade compression so it should be
  # used only when necessary, such as at a request or response boundary for a
  # network stream.
  SYNC_FLUSH = T.let(T.unsafe(nil), Integer)

  # Represents text data as guessed by deflate.
  #
  # See
  # [`Zlib::Deflate#data_type`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-data_type).
  TEXT = T.let(T.unsafe(nil), Integer)

  # Represents an unknown data type as guessed by deflate.
  #
  # See
  # [`Zlib::Deflate#data_type`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-data_type).
  UNKNOWN = T.let(T.unsafe(nil), Integer)

  # The Ruby/zlib version string.
  VERSION = T.let(T.unsafe(nil), String)

  # The string which represents the version of zlib.h
  ZLIB_VERSION = T.let(T.unsafe(nil), String)

  # Calculates Adler-32 checksum for `string`, and returns updated value of
  # `adler`. If `string` is omitted, it returns the Adler-32 initial value. If
  # `adler` is omitted, it assumes that the initial value is given to `adler`.
  #
  # Example usage:
  #
  # ```ruby
  # require "zlib"
  #
  # data = "foo"
  # puts "Adler32 checksum: #{Zlib.adler32(data).to_s(16)}"
  # #=> Adler32 checksum: 2820145
  # ```
  def self.adler32(*_); end

  # Combine two Adler-32 check values in to one. `alder1` is the first Adler-32
  # value, `adler2` is the second Adler-32 value. `len2` is the length of the
  # string used to generate `adler2`.
  def self.adler32_combine(_, _, _); end

  # Calculates CRC checksum for `string`, and returns updated value of `crc`. If
  # `string` is omitted, it returns the CRC initial value. If `crc` is omitted,
  # it assumes that the initial value is given to `crc`.
  #
  # FIXME: expression.
  def self.crc32(*_); end

  # Combine two CRC-32 check values in to one. `crc1` is the first CRC-32 value,
  # `crc2` is the second CRC-32 value. `len2` is the length of the string used
  # to generate `crc2`.
  def self.crc32_combine(_, _, _); end

  # Returns the table for calculating CRC checksum as an array.
  def self.crc_table; end

  # Compresses the given `string`. Valid values of level are
  # [`Zlib::NO_COMPRESSION`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#NO_COMPRESSION),
  # [`Zlib::BEST_SPEED`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#BEST_SPEED),
  # [`Zlib::BEST_COMPRESSION`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#BEST_COMPRESSION),
  # [`Zlib::DEFAULT_COMPRESSION`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#DEFAULT_COMPRESSION),
  # or an integer from 0 to 9.
  #
  # This method is almost equivalent to the following code:
  #
  # ```ruby
  # def deflate(string, level)
  #   z = Zlib::Deflate.new(level)
  #   dst = z.deflate(string, Zlib::FINISH)
  #   z.close
  #   dst
  # end
  # ```
  #
  # See also
  # [`Zlib.inflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#method-c-inflate)
  def self.deflate(*_); end

  # Decode the given gzipped `string`.
  #
  # This method is almost equivalent to the following code:
  #
  # ```ruby
  # def gunzip(string)
  #   sio = StringIO.new(string)
  #   gz = Zlib::GzipReader.new(sio, encoding: Encoding::ASCII_8BIT)
  #   gz.read
  # ensure
  #   gz&.close
  # end
  # ```
  #
  # See also
  # [`Zlib.gzip`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#method-c-gzip)
  def self.gunzip(_); end

  # Gzip the given `string`. Valid values of level are
  # [`Zlib::NO_COMPRESSION`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#NO_COMPRESSION),
  # [`Zlib::BEST_SPEED`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#BEST_SPEED),
  # [`Zlib::BEST_COMPRESSION`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#BEST_COMPRESSION),
  # [`Zlib::DEFAULT_COMPRESSION`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#DEFAULT_COMPRESSION)
  # (default), or an integer from 0 to 9.
  #
  # This method is almost equivalent to the following code:
  #
  # ```ruby
  # def gzip(string, level: nil, strategy: nil)
  #   sio = StringIO.new
  #   sio.binmode
  #   gz = Zlib::GzipWriter.new(sio, level, strategy)
  #   gz.write(string)
  #   gz.close
  #   sio.string
  # end
  # ```
  #
  # See also
  # [`Zlib.gunzip`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#method-c-gunzip)
  def self.gzip(*_); end

  # Decompresses `string`. Raises a
  # [`Zlib::NeedDict`](https://docs.ruby-lang.org/en/2.7.0/Zlib/NeedDict.html)
  # exception if a preset dictionary is needed for decompression.
  #
  # This method is almost equivalent to the following code:
  #
  # ```ruby
  # def inflate(string)
  #   zstream = Zlib::Inflate.new
  #   buf = zstream.inflate(string)
  #   zstream.finish
  #   zstream.close
  #   buf
  # end
  # ```
  #
  # See also
  # [`Zlib.deflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#method-c-deflate)
  def self.inflate(_); end

  # Returns the string which represents the version of zlib library.
  def self.zlib_version; end
end

# Subclass of
# [`Zlib::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Error.html) when zlib
# returns a Z\_BUF\_ERROR.
#
# Usually if no progress is possible.
class Zlib::BufError < ::Zlib::Error; end

# Subclass of
# [`Zlib::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Error.html) when zlib
# returns a Z\_DATA\_ERROR.
#
# Usually if a stream was prematurely freed.
class Zlib::DataError < ::Zlib::Error; end

# [`Zlib::Deflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Deflate.html) is
# the class for compressing data. See
# [`Zlib::ZStream`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html) for
# more information.
class Zlib::Deflate < ::Zlib::ZStream
  # Creates a new deflate stream for compression. If a given argument is nil,
  # the default value of that argument is used.
  #
  # The `level` sets the compression level for the deflate stream between 0 (no
  # compression) and 9 (best compression). The following constants have been
  # defined to make code more readable:
  #
  # *   Zlib::DEFAULT\_COMPRESSION
  # *   Zlib::NO\_COMPRESSION
  # *   Zlib::BEST\_SPEED
  # *   Zlib::BEST\_COMPRESSION
  #
  #
  # See http://www.zlib.net/manual.html#Constants for further information.
  #
  # The `window_bits` sets the size of the history buffer and should be between
  # 8 and 15. Larger values of this parameter result in better compression at
  # the expense of memory usage.
  #
  # The `mem_level` specifies how much memory should be allocated for the
  # internal compression state. 1 uses minimum memory but is slow and reduces
  # compression ratio while 9 uses maximum memory for optimal speed. The default
  # value is 8. Two constants are defined:
  #
  # *   Zlib::DEF\_MEM\_LEVEL
  # *   Zlib::MAX\_MEM\_LEVEL
  #
  #
  # The `strategy` sets the deflate compression strategy. The following
  # strategies are available:
  #
  # Zlib::DEFAULT\_STRATEGY
  # :   For normal data
  # Zlib::FILTERED
  # :   For data produced by a filter or predictor
  # Zlib::FIXED
  # :   Prevents dynamic Huffman codes
  # Zlib::HUFFMAN\_ONLY
  # :   Prevents string matching
  # Zlib::RLE
  # :   Designed for better compression of PNG image data
  #
  #
  # See the constants for further description.
  #
  # ## Examples
  #
  # ### Basic
  #
  # ```ruby
  # open "compressed.file", "w+" do |io|
  #   io << Zlib::Deflate.new.deflate(File.read("big.file"))
  # end
  # ```
  #
  # ### Custom compression
  #
  # ```ruby
  # open "compressed.file", "w+" do |compressed_io|
  #   deflate = Zlib::Deflate.new(Zlib::BEST_COMPRESSION,
  #                               Zlib::MAX_WBITS,
  #                               Zlib::MAX_MEM_LEVEL,
  #                               Zlib::HUFFMAN_ONLY)
  #
  #   begin
  #     open "big.file" do |big_io|
  #       until big_io.eof? do
  #         compressed_io << zd.deflate(big_io.read(16384))
  #       end
  #     end
  #   ensure
  #     deflate.close
  #   end
  # end
  # ```
  #
  # While this example will work, for best optimization review the flags for
  # your specific time, memory usage and output space requirements.
  def self.new(*_); end

  # Inputs `string` into the deflate stream just like
  # [`Zlib::Deflate#deflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Deflate.html#method-i-deflate),
  # but returns the
  # [`Zlib::Deflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Deflate.html)
  # object itself. The output from the stream is preserved in output buffer.
  def <<(_); end

  # Inputs `string` into the deflate stream and returns the output from the
  # stream. On calling this method, both the input and the output buffers of the
  # stream are flushed. If `string` is nil, this method finishes the stream,
  # just like
  # [`Zlib::ZStream#finish`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-finish).
  #
  # If a block is given consecutive deflated chunks from the `string` are
  # yielded to the block and `nil` is returned.
  #
  # The `flush` parameter specifies the flush mode. The following constants may
  # be used:
  #
  # Zlib::NO\_FLUSH
  # :   The default
  # Zlib::SYNC\_FLUSH
  # :   Flushes the output to a byte boundary
  # Zlib::FULL\_FLUSH
  # :   SYNC\_FLUSH + resets the compression state
  # Zlib::FINISH
  # :   Pending input is processed, pending output is flushed.
  #
  #
  # See the constants for further description.
  def deflate(*_); end

  # This method is equivalent to `deflate('', flush)`. This method is just
  # provided to improve the readability of your Ruby program. If a block is
  # given chunks of deflate output are yielded to the block until the buffer is
  # flushed.
  #
  # See
  # [`Zlib::Deflate#deflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Deflate.html#method-i-deflate)
  # for detail on the `flush` constants NO\_FLUSH, SYNC\_FLUSH, FULL\_FLUSH and
  # FINISH.
  def flush(*_); end

  # Changes the parameters of the deflate stream to allow changes between
  # different types of data that require different types of compression. Any
  # unprocessed data is flushed before changing the params.
  #
  # See
  # [`Zlib::Deflate.new`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Deflate.html#method-c-new)
  # for a description of `level` and `strategy`.
  def params(_, _); end

  # Sets the preset dictionary and returns `string`. This method is available
  # just only after
  # [`Zlib::Deflate.new`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Deflate.html#method-c-new)
  # or
  # [`Zlib::ZStream#reset`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-reset)
  # method was called. See zlib.h for details.
  #
  # Can raise errors of Z\_STREAM\_ERROR if a parameter is invalid (such as NULL
  # dictionary) or the stream state is inconsistent, Z\_DATA\_ERROR if the given
  # dictionary doesn't match the expected one (incorrect adler32 value)
  def set_dictionary(_); end

  # Compresses the given `string`. Valid values of level are
  # Zlib::NO\_COMPRESSION, Zlib::BEST\_SPEED, Zlib::BEST\_COMPRESSION,
  # Zlib::DEFAULT\_COMPRESSION, or an integer from 0 to 9.
  #
  # This method is almost equivalent to the following code:
  #
  # ```ruby
  # def deflate(string, level)
  #   z = Zlib::Deflate.new(level)
  #   dst = z.deflate(string, Zlib::FINISH)
  #   z.close
  #   dst
  # end
  # ```
  #
  # See also
  # [`Zlib.inflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#method-c-inflate)
  def self.deflate(*_); end
end

# The superclass for all exceptions raised by Ruby/zlib.
#
# The following exceptions are defined as subclasses of
# [`Zlib::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Error.html). These
# exceptions are raised when zlib library functions return with an error status.
#
# *   [`Zlib::StreamEnd`](https://docs.ruby-lang.org/en/2.7.0/Zlib/StreamEnd.html)
# *   [`Zlib::NeedDict`](https://docs.ruby-lang.org/en/2.7.0/Zlib/NeedDict.html)
# *   [`Zlib::DataError`](https://docs.ruby-lang.org/en/2.7.0/Zlib/DataError.html)
# *   [`Zlib::StreamError`](https://docs.ruby-lang.org/en/2.7.0/Zlib/StreamError.html)
# *   [`Zlib::MemError`](https://docs.ruby-lang.org/en/2.7.0/Zlib/MemError.html)
# *   [`Zlib::BufError`](https://docs.ruby-lang.org/en/2.7.0/Zlib/BufError.html)
# *   [`Zlib::VersionError`](https://docs.ruby-lang.org/en/2.7.0/Zlib/VersionError.html)
class Zlib::Error < ::StandardError; end

# [`Zlib::GzipFile`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html) is
# an abstract class for handling a gzip formatted compressed file. The
# operations are defined in the subclasses,
# [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
# for reading, and
# [`Zlib::GzipWriter`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html)
# for writing.
#
# GzipReader should be used by associating an
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html), or IO-like, object.
#
# ## [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) Catalogue
#
# *   [`::wrap`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-c-wrap)
# *   [`::open`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-c-open)
#     ([`Zlib::GzipReader::open`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-c-open)
#     and
#     [`Zlib::GzipWriter::open`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html#method-c-open))
# *   [`close`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-close)
# *   [`closed?`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-closed-3F)
# *   [`comment`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-comment)
# *   comment=
#     ([`Zlib::GzipWriter#comment=`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html#method-i-comment-3D))
# *   [`crc`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-crc)
# *   eof?
#     ([`Zlib::GzipReader#eof?`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-eof-3F))
# *   [`finish`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-finish)
# *   [`level`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-level)
# *   lineno
#     ([`Zlib::GzipReader#lineno`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-lineno))
# *   lineno=
#     ([`Zlib::GzipReader#lineno=`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-lineno-3D))
# *   [`mtime`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-mtime)
# *   mtime=
#     ([`Zlib::GzipWriter#mtime=`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html#method-i-mtime-3D))
# *   [`orig_name`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-orig_name)
# *   [`orig_name`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-orig_name)
#     ([`Zlib::GzipWriter#orig_name=`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html#method-i-orig_name-3D))
# *   [`os_code`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-os_code)
# *   path (when the underlying
#     [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) supports path)
# *   [`sync`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-sync)
# *   [`sync=`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-sync-3D)
# *   [`to_io`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-to_io)
#
#
# (due to internal structure, documentation may appear under
# [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
# or
# [`Zlib::GzipWriter`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html))
class Zlib::GzipFile
  # Closes the
  # [`GzipFile`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html) object.
  # This method calls close method of the associated
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object. Returns the
  # associated [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object.
  def close; end

  # Same as
  # [`IO#closed?`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-closed-3F)
  def closed?; end

  # Returns comments recorded in the gzip file header, or nil if the comments is
  # not present.
  def comment; end

  # Returns CRC value of the uncompressed data.
  def crc; end

  # Closes the
  # [`GzipFile`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html) object.
  # Unlike
  # [`Zlib::GzipFile#close`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-close),
  # this method never calls the close method of the associated
  # [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object. Returns the
  # associated [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html) object.
  def finish; end

  # Returns compression level.
  def level; end

  # Returns last modification time recorded in the gzip file header.
  def mtime; end

  # Returns original filename recorded in the gzip file header, or `nil` if
  # original filename is not present.
  def orig_name; end

  # Returns OS code number recorded in the gzip file header.
  def os_code; end

  # Same as
  # [`IO#sync`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-sync)
  def sync; end

  # Same as [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html). If flag is
  # `true`, the associated [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object must respond to the `flush` method. While `sync` mode is `true`, the
  # compression ratio decreases sharply.
  def sync=(_); end

  # Same as [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  def to_io; end

  # Creates a GzipReader or GzipWriter associated with `io`, passing in any
  # necessary extra options, and executes the block with the newly created
  # object just like
  # [`File.open`](https://docs.ruby-lang.org/en/2.7.0/File.html#method-c-open).
  #
  # The [`GzipFile`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html)
  # object will be closed automatically after executing the block. If you want
  # to keep the associated [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object open, you may call
  # [`Zlib::GzipFile#finish`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-finish)
  # method in the block.
  def self.wrap(*_, &blk); end
end

# Raised when the CRC checksum recorded in gzip file footer is not equivalent to
# the CRC checksum of the actual uncompressed data.
class Zlib::GzipFile::CRCError < ::Zlib::GzipFile::Error; end

# Base class of errors that occur when processing GZIP files.
class Zlib::GzipFile::Error < ::Zlib::Error
  # input gzipped string
  def input; end

  # Constructs a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) of
  # the GzipFile [`Error`](https://docs.ruby-lang.org/en/2.7.0/Error.html)
  def inspect; end
end

# Raised when the data length recorded in the gzip file footer is not equivalent
# to the length of the actual uncompressed data.
class Zlib::GzipFile::LengthError < ::Zlib::GzipFile::Error; end

# Raised when gzip file footer is not found.
class Zlib::GzipFile::NoFooter < ::Zlib::GzipFile::Error; end

# [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
# is the class for reading a gzipped file.
# [`GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
# should be used as an [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html), or
# -IO-like, object.
#
# ```ruby
# Zlib::GzipReader.open('hoge.gz') {|gz|
#   print gz.read
# }
#
# File.open('hoge.gz') do |f|
#   gz = Zlib::GzipReader.new(f)
#   print gz.read
#   gz.close
# end
# ```
#
# ## [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) Catalogue
#
# The following methods in
# [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
# are just like their counterparts in
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html), but they raise
# [`Zlib::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Error.html) or
# [`Zlib::GzipFile::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile/Error.html)
# exception if an error was found in the gzip file.
# *   [`each`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-each)
# *   [`each_line`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-each_line)
# *   [`each_byte`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-each_byte)
# *   [`gets`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-gets)
# *   [`getc`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-getc)
# *   [`lineno`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-lineno)
# *   [`lineno=`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-lineno-3D)
# *   [`read`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-read)
# *   [`readchar`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-readchar)
# *   [`readline`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-readline)
# *   [`readlines`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-readlines)
# *   [`ungetc`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-ungetc)
#
#
# Be careful of the footer of the gzip file. A gzip file has the checksum of
# pre-compressed data in its footer.
# [`GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
# checks all uncompressed data against that checksum at the following cases, and
# if it fails, raises `Zlib::GzipFile::NoFooter`, `Zlib::GzipFile::CRCError`, or
# `Zlib::GzipFile::LengthError` exception.
#
# *   When an reading request is received beyond the end of file (the end of
#     compressed data). That is, when
#     [`Zlib::GzipReader#read`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-read),
#     [`Zlib::GzipReader#gets`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-gets),
#     or some other methods for reading returns nil.
# *   When
#     [`Zlib::GzipFile#close`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-close)
#     method is called after the object reaches the end of file.
# *   When
#     [`Zlib::GzipReader#unused`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-i-unused)
#     method is called after the object reaches the end of file.
#
#
# The rest of the methods are adequately described in their own documentation.
class Zlib::GzipReader < ::Zlib::GzipFile
  include(::Enumerable)

  Elem = type_member(:out)

  # Creates a
  # [`GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # object associated with `io`. The
  # [`GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # object reads gzipped data from `io`, and parses/decompresses it. The `io`
  # must have a `read` method that behaves same as the
  # [`IO#read`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-read).
  #
  # The `options` hash may be used to set the encoding of the data.
  # `:external_encoding`, `:internal_encoding` and `:encoding` may be set as in
  # [`IO::new`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-new).
  #
  # If the gzip file header is incorrect, raises an
  # [`Zlib::GzipFile::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile/Error.html)
  # exception.
  def self.new(*_); end

  # This is a deprecated alias for `each_byte`.
  def bytes; end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def each(*_); end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def each_byte; end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def each_char; end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def each_line(*_); end

  # Returns `true` or `false` whether the stream has reached the end.
  def eof; end

  # Returns `true` or `false` whether the stream has reached the end.
  def eof?; end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def external_encoding; end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def getbyte; end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def getc; end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def gets(*_); end

  # The line number of the last row read from this file.
  def lineno; end

  # Specify line number of the last row read from this file.
  def lineno=(_); end

  # This is a deprecated alias for `each_line`.
  def lines(*_); end

  # Total number of output bytes output so far.
  def pos; end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def read(*_); end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def readbyte; end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def readchar; end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def readline(*_); end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def readlines(*_); end

  # Reads at most *maxlen* bytes from the gzipped stream but it blocks only if
  # *gzipreader* has no data immediately available. If the optional *outbuf*
  # argument is present, it must reference a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html), which will
  # receive the data. It raises `EOFError` on end of file.
  def readpartial(*_); end

  # Resets the position of the file pointer to the point created the
  # [`GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # object. The associated [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html)
  # object needs to respond to the `seek` method.
  def rewind; end

  # Total number of output bytes output so far.
  def tell; end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def ungetbyte(_); end

  # See
  # [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # documentation for a description.
  def ungetc(_); end

  # Returns the rest of the data which had read for parsing gzip format, or
  # `nil` if the whole gzip file is not parsed yet.
  def unused; end

  # Opens a file specified by `filename` as a gzipped file, and returns a
  # [`GzipReader`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html)
  # object associated with that file. Further details of this method are in
  # [`Zlib::GzipReader.new`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipReader.html#method-c-new)
  # and ZLib::GzipFile.wrap.
  def self.open(*_, &blk); end
end

# [`Zlib::GzipWriter`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html)
# is a class for writing gzipped files.
# [`GzipWriter`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html)
# should be used with an instance of
# [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html), or IO-like, object.
#
# Following two example generate the same result.
#
# ```ruby
# Zlib::GzipWriter.open('hoge.gz') do |gz|
#   gz.write 'jugemu jugemu gokou no surikire...'
# end
#
# File.open('hoge.gz', 'w') do |f|
#   gz = Zlib::GzipWriter.new(f)
#   gz.write 'jugemu jugemu gokou no surikire...'
#   gz.close
# end
# ```
#
# To make like gzip(1) does, run following:
#
# ```ruby
# orig = 'hoge.txt'
# Zlib::GzipWriter.open('hoge.gz') do |gz|
#   gz.mtime = File.mtime(orig)
#   gz.orig_name = orig
#   gz.write IO.binread(orig)
# end
# ```
#
# NOTE: Due to the limitation of Ruby's finalizer, you must explicitly close
# [`GzipWriter`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html)
# objects by
# [`Zlib::GzipWriter#close`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-i-close)
# etc. Otherwise,
# [`GzipWriter`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html) will
# be not able to write the gzip footer and will generate a broken gzip file.
class Zlib::GzipWriter < ::Zlib::GzipFile
  # Creates a
  # [`GzipWriter`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html)
  # object associated with `io`. `level` and `strategy` should be the same as
  # the arguments of
  # [`Zlib::Deflate.new`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Deflate.html#method-c-new).
  # The [`GzipWriter`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html)
  # object writes gzipped data to `io`. `io` must respond to the `write` method
  # that behaves the same as
  # [`IO#write`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-write).
  #
  # The `options` hash may be used to set the encoding of the data.
  # `:external_encoding`, `:internal_encoding` and `:encoding` may be set as in
  # [`IO::new`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-c-new).
  def self.new(*_); end

  # Same as [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  def <<(_); end

  # Specify the comment (`str`) in the gzip header.
  def comment=(_); end

  # Flushes all the internal buffers of the
  # [`GzipWriter`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html)
  # object. The meaning of `flush` is same as in
  # [`Zlib::Deflate#deflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Deflate.html#method-i-deflate).
  # `Zlib::SYNC_FLUSH` is used if `flush` is omitted. It is no use giving flush
  # `Zlib::NO_FLUSH`.
  def flush(*_); end

  # Specify the modification time (`mtime`) in the gzip header. Using an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  #
  # Setting the mtime in the gzip header does not effect the mtime of the file
  # generated. Different utilities that expand the gzipped files may use the
  # mtime header. For example the gunzip utility can use the `-N' flag which
  # will set the resultant file's mtime to the value in the header. By default
  # many tools will set the mtime of the expanded file to the mtime of the
  # gzipped file, not the mtime in the header.
  #
  # If you do not set an mtime, the default value will be the time when
  # compression started. Setting a value of 0 indicates no time stamp is
  # available.
  def mtime=(_); end

  # Specify the original name (`str`) in the gzip header.
  def orig_name=(_); end

  # Total number of input bytes read so far.
  def pos; end

  # Same as [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  def print(*_); end

  # Same as [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  def printf(*_); end

  # Same as [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  def putc(_); end

  # Same as [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  def puts(*_); end

  # Total number of input bytes read so far.
  def tell; end

  # Same as [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  def write(*_); end

  # Opens a file specified by `filename` for writing gzip compressed data, and
  # returns a
  # [`GzipWriter`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html)
  # object associated with that file. Further details of this method are found
  # in
  # [`Zlib::GzipWriter.new`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipWriter.html#method-c-new)
  # and
  # [`Zlib::GzipFile.wrap`](https://docs.ruby-lang.org/en/2.7.0/Zlib/GzipFile.html#method-c-wrap).
  def self.open(*_, &blk); end
end

# Zlib:Inflate is the class for decompressing compressed data. Unlike
# [`Zlib::Deflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Deflate.html), an
# instance of this class is not able to duplicate (clone, dup) itself.
class Zlib::Inflate < ::Zlib::ZStream
  # Creates a new inflate stream for decompression. `window_bits` sets the size
  # of the history buffer and can have the following values:
  #
  # 0
  # :   Have inflate use the window size from the zlib header of the compressed
  #     stream.
  #
  # (8..15)
  # :   Overrides the window size of the inflate header in the compressed
  #     stream. The window size must be greater than or equal to the window size
  #     of the compressed stream.
  #
  # Greater than 15
  # :   Add 32 to window\_bits to enable zlib and gzip decoding with automatic
  #     header detection, or add 16 to decode only the gzip format (a
  #     [`Zlib::DataError`](https://docs.ruby-lang.org/en/2.7.0/Zlib/DataError.html)
  #     will be raised for a non-gzip stream).
  #
  # (-8..-15)
  # :   Enables raw deflate mode which will not generate a check value, and will
  #     not look for any check values for comparison at the end of the stream.
  #
  #     This is for use with other formats that use the deflate compressed data
  #     format such as zip which provide their own check values.
  #
  #
  # ## Example
  #
  # ```ruby
  # open "compressed.file" do |compressed_io|
  #   zi = Zlib::Inflate.new(Zlib::MAX_WBITS + 32)
  #
  #   begin
  #     open "uncompressed.file", "w+" do |uncompressed_io|
  #       uncompressed_io << zi.inflate(compressed_io.read)
  #     end
  #   ensure
  #     zi.close
  #   end
  # end
  # ```
  def self.new(*_); end

  # Same as [`IO`](https://docs.ruby-lang.org/en/2.7.0/IO.html).
  def <<(_); end

  # Provide the inflate stream with a dictionary that may be required in the
  # future. Multiple dictionaries may be provided. The inflate stream will
  # automatically choose the correct user-provided dictionary based on the
  # stream's required dictionary.
  def add_dictionary(_); end

  # Inputs `deflate_string` into the inflate stream and returns the output from
  # the stream. Calling this method, both the input and the output buffer of the
  # stream are flushed. If string is `nil`, this method finishes the stream,
  # just like
  # [`Zlib::ZStream#finish`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-finish).
  #
  # If a block is given consecutive inflated chunks from the `deflate_string`
  # are yielded to the block and `nil` is returned.
  #
  # Raises a
  # [`Zlib::NeedDict`](https://docs.ruby-lang.org/en/2.7.0/Zlib/NeedDict.html)
  # exception if a preset dictionary is needed to decompress.
  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the dictionary by
  # [`Zlib::Inflate#set_dictionary`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Inflate.html#method-i-set_dictionary)
  # and then call this method again with an empty string to flush the stream:
  #
  # ```ruby
  # inflater = Zlib::Inflate.new
  #
  # begin
  #   out = inflater.inflate compressed
  # rescue Zlib::NeedDict
  #   # ensure the dictionary matches the stream's required dictionary
  #   raise unless inflater.adler == Zlib.adler32(dictionary)
  #
  #   inflater.set_dictionary dictionary
  #   inflater.inflate ''
  # end
  #
  # # ...
  #
  # inflater.close
  # ```
  #
  # See also
  # [`Zlib::Inflate.new`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Inflate.html#method-c-new)
  def inflate(_); end

  # Sets the preset dictionary and returns `string`. This method is available
  # just only after a
  # [`Zlib::NeedDict`](https://docs.ruby-lang.org/en/2.7.0/Zlib/NeedDict.html)
  # exception was raised. See zlib.h for details.
  def set_dictionary(_); end

  # Inputs `string` into the end of input buffer and skips data until a full
  # flush point can be found. If the point is found in the buffer, this method
  # flushes the buffer and returns false. Otherwise it returns `true` and the
  # following data of full flush point is preserved in the buffer.
  def sync(_); end

  # Quoted verbatim from original documentation:
  #
  # ```ruby
  # What is this?
  # ```
  #
  # `:)`
  def sync_point?; end

  # Decompresses `string`. Raises a
  # [`Zlib::NeedDict`](https://docs.ruby-lang.org/en/2.7.0/Zlib/NeedDict.html)
  # exception if a preset dictionary is needed for decompression.
  #
  # This method is almost equivalent to the following code:
  #
  # ```ruby
  # def inflate(string)
  #   zstream = Zlib::Inflate.new
  #   buf = zstream.inflate(string)
  #   zstream.finish
  #   zstream.close
  #   buf
  # end
  # ```
  #
  # See also
  # [`Zlib.deflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib.html#method-c-deflate)
  def self.inflate(_); end
end

# Subclass of
# [`Zlib::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Error.html)
#
# When zlib returns a Z\_MEM\_ERROR, usually if there was not enough memory.
class Zlib::MemError < ::Zlib::Error; end

# Subclass of
# [`Zlib::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Error.html)
#
# When zlib returns a Z\_NEED\_DICT if a preset dictionary is needed at this
# point.
#
# Used by
# [`Zlib::Inflate.inflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Inflate.html#method-c-inflate)
# and `Zlib.inflate`
class Zlib::NeedDict < ::Zlib::Error; end

# Subclass of
# [`Zlib::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Error.html)
#
# When zlib returns a Z\_STREAM\_END is return if the end of the compressed data
# has been reached and all uncompressed out put has been produced.
class Zlib::StreamEnd < ::Zlib::Error; end

# Subclass of
# [`Zlib::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Error.html)
#
# When zlib returns a Z\_STREAM\_ERROR, usually if the stream state was
# inconsistent.
class Zlib::StreamError < ::Zlib::Error; end

# Subclass of
# [`Zlib::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Error.html)
#
# When zlib returns a Z\_VERSION\_ERROR, usually if the zlib library version is
# incompatible with the version assumed by the caller.
class Zlib::VersionError < ::Zlib::Error; end

# [`Zlib::ZStream`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html) is
# the abstract class for the stream which handles the compressed data. The
# operations are defined in the subclasses:
# [`Zlib::Deflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Deflate.html) for
# compression, and
# [`Zlib::Inflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Inflate.html) for
# decompression.
#
# An instance of
# [`Zlib::ZStream`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html) has
# one stream (struct zstream in the source) and two variable-length buffers
# which associated to the input (next\_in) of the stream and the output
# (next\_out) of the stream. In this document, "input buffer" means the buffer
# for input, and "output buffer" means the buffer for output.
#
# [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html) input into an instance
# of [`Zlib::ZStream`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html)
# are temporally stored into the end of input buffer, and then data in input
# buffer are processed from the beginning of the buffer until no more output
# from the stream is produced (i.e. until
# [`avail_out`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-avail_out)
# > 0 after processing). During processing, output buffer is allocated and
# expanded automatically to hold all output data.
#
# Some particular instance methods consume the data in output buffer and return
# them as a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
#
# Here is an ascii art for describing above:
#
# ```
# +================ an instance of Zlib::ZStream ================+
# ||                                                            ||
# ||     +--------+          +-------+          +--------+      ||
# ||  +--| output |<---------|zstream|<---------| input  |<--+  ||
# ||  |  | buffer |  next_out+-------+next_in   | buffer |   |  ||
# ||  |  +--------+                             +--------+   |  ||
# ||  |                                                      |  ||
# +===|======================================================|===+
#     |                                                      |
#     v                                                      |
# "output data"                                         "input data"
# ```
#
# If an error occurs during processing input buffer, an exception which is a
# subclass of
# [`Zlib::Error`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Error.html) is
# raised. At that time, both input and output buffer keep their conditions at
# the time when the error occurs.
#
# ## [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) Catalogue
#
# Many of the methods in this class are fairly low-level and unlikely to be of
# interest to users. In fact, users are unlikely to use this class directly;
# rather they will be interested in
# [`Zlib::Inflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Inflate.html) and
# [`Zlib::Deflate`](https://docs.ruby-lang.org/en/2.7.0/Zlib/Deflate.html).
#
# The higher level methods are listed below.
#
# *   [`total_in`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-total_in)
# *   [`total_out`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-total_out)
# *   [`data_type`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-data_type)
# *   [`adler`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-adler)
# *   [`reset`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-reset)
# *   [`finish`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-finish)
# *   [`finished?`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-finished-3F)
# *   [`close`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-close)
# *   [`closed?`](https://docs.ruby-lang.org/en/2.7.0/Zlib/ZStream.html#method-i-closed-3F)
class Zlib::ZStream
  # Returns the adler-32 checksum.
  def adler; end

  # Returns bytes of data in the input buffer. Normally, returns 0.
  def avail_in; end

  # Returns number of bytes of free spaces in output buffer. Because the free
  # space is allocated automatically, this method returns 0 normally.
  def avail_out; end

  # Allocates `size` bytes of free space in the output buffer. If there are more
  # than `size` bytes already in the buffer, the buffer is truncated. Because
  # free space is allocated automatically, you usually don't need to use this
  # method.
  def avail_out=(_); end

  # Closes the stream. All operations on the closed stream will raise an
  # exception.
  def close; end

  # Returns true if the stream is closed.
  def closed?; end

  # Guesses the type of the data which have been inputted into the stream. The
  # returned value is either `BINARY`, `ASCII`, or `UNKNOWN`.
  def data_type; end

  # Closes the stream. All operations on the closed stream will raise an
  # exception.
  def end; end

  # Returns true if the stream is closed.
  def ended?; end

  # Finishes the stream and flushes output buffer. If a block is given each
  # chunk is yielded to the block until the input buffer has been flushed to the
  # output buffer.
  def finish; end

  # Returns true if the stream is finished.
  def finished?; end

  def flush_next_in; end

  # Flushes output buffer and returns all data in that buffer. If a block is
  # given each chunk is yielded to the block until the current output buffer has
  # been flushed.
  def flush_next_out; end

  # Resets and initializes the stream. All data in both input and output buffer
  # are discarded.
  def reset; end

  # Returns true if the stream is finished.
  def stream_end?; end

  # Returns the total bytes of the input data to the stream. FIXME
  def total_in; end

  # Returns the total bytes of the output data from the stream. FIXME
  def total_out; end
end
