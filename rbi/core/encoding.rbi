# typed: __STDLIB_INTERNAL

# An [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) instance
# represents a character encoding usable in Ruby. It is defined as a constant
# under the [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html)
# namespace. It has a name and optionally, aliases:
#
# ```ruby
# Encoding::ISO_8859_1.name
# #=> "ISO-8859-1"
#
# Encoding::ISO_8859_1.names
# #=> ["ISO-8859-1", "ISO8859-1"]
# ```
#
# Ruby methods dealing with encodings return or accept
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) instances as
# arguments (when a method accepts an
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) instance as an
# argument, it can be passed an
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) name or alias
# instead).
#
# ```ruby
# "some string".encoding
# #=> #<Encoding:UTF-8>
#
# string = "some string".encode(Encoding::ISO_8859_1)
# #=> "some string"
# string.encoding
# #=> #<Encoding:ISO-8859-1>
#
# "some string".encode "ISO-8859-1"
# #=> "some string"
# ```
#
# `Encoding::ASCII_8BIT` is a special encoding that is usually used for a byte
# string, not a character string. But as the name insists, its characters in the
# range of ASCII are considered as ASCII characters. This is useful when you use
# ASCII-8BIT characters with other ASCII compatible characters.
#
# ## Changing an encoding
#
# The associated [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html)
# of a [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) can be
# changed in two different ways.
#
# First, it is possible to set the
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) of a string to
# a new [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) without
# changing the internal byte representation of the string, with
# [`String#force_encoding`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-force_encoding).
# This is how you can tell Ruby the correct encoding of a string.
#
# ```ruby
# string
# #=> "R\xC3\xA9sum\xC3\xA9"
# string.encoding
# #=> #<Encoding:ISO-8859-1>
# string.force_encoding(Encoding::UTF_8)
# #=> "R\u00E9sum\u00E9"
# ```
#
# Second, it is possible to transcode a string, i.e. translate its internal byte
# representation to another encoding. Its associated encoding is also set to the
# other encoding. See
# [`String#encode`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-encode)
# for the various forms of transcoding, and the
# [`Encoding::Converter`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html)
# class for additional control over the transcoding process.
#
# ```ruby
# string
# #=> "R\u00E9sum\u00E9"
# string.encoding
# #=> #<Encoding:UTF-8>
# string = string.encode!(Encoding::ISO_8859_1)
# #=> "R\xE9sum\xE9"
# string.encoding
# #=> #<Encoding::ISO-8859-1>
# ```
#
# ## Script encoding
#
# All Ruby script code has an associated
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) which any
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) literal created in
# the source code will be associated to.
#
# The default script encoding is `Encoding::UTF-8` after v2.0, but it can be
# changed by a magic comment on the first line of the source code file (or
# second line, if there is a shebang line on the first). The comment must
# contain the word `coding` or `encoding`, followed by a colon, space and the
# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) name or alias:
#
# ```ruby
# # encoding: UTF-8
#
# "some string".encoding
# #=> #<Encoding:UTF-8>
# ```
#
# The `__ENCODING__` keyword returns the script encoding of the file which the
# keyword is written:
#
# ```ruby
# # encoding: ISO-8859-1
#
# __ENCODING__
# #=> #<Encoding:ISO-8859-1>
# ```
#
# `ruby -K` will change the default locale encoding, but this is not
# recommended. Ruby source files should declare its script encoding by a magic
# comment even when they only depend on US-ASCII strings or regular expressions.
#
# ## Locale encoding
#
# The default encoding of the environment. Usually derived from locale.
#
# see
# [`Encoding.locale_charmap`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-locale_charmap),
# [`Encoding.find`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-find)('locale')
#
# ## Filesystem encoding
#
# The default encoding of strings from the filesystem of the environment. This
# is used for strings of file names or paths.
#
# see
# [`Encoding.find`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-find)('filesystem')
#
# ## External encoding
#
# Each [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object has an
# external encoding which indicates the encoding that Ruby will use to read its
# data. By default Ruby sets the external encoding of an
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object to the default
# external encoding. The default external encoding is set by locale encoding or
# the interpreter `-E` option.
# [`Encoding.default_external`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-default_external)
# returns the current value of the external encoding.
#
# ```
# ENV["LANG"]
# #=> "UTF-8"
# Encoding.default_external
# #=> #<Encoding:UTF-8>
#
# $ ruby -E ISO-8859-1 -e "p Encoding.default_external"
# #<Encoding:ISO-8859-1>
#
# $ LANG=C ruby -e 'p Encoding.default_external'
# #<Encoding:US-ASCII>
# ```
#
# The default external encoding may also be set through
# [`Encoding.default_external=`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-default_external-3D),
# but you should not do this as strings created before and after the change will
# have inconsistent encodings. Instead use `ruby -E` to invoke ruby with the
# correct external encoding.
#
# When you know that the actual encoding of the data of an
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object is not the default
# external encoding, you can reset its external encoding with
# [`IO#set_encoding`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-set_encoding)
# or set it at [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object
# creation (see
# [`IO.new`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-new) options).
#
# ## Internal encoding
#
# To process the data of an [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html)
# object which has an encoding different from its external encoding, you can set
# its internal encoding. Ruby will use this internal encoding to transcode the
# data when it is read from the
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object.
#
# Conversely, when data is written to the
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object it is transcoded
# from the internal encoding to the external encoding of the
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object.
#
# The internal encoding of an
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object can be set with
# [`IO#set_encoding`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-set_encoding)
# or at [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) object creation (see
# [`IO.new`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-new) options).
#
# The internal encoding is optional and when not set, the Ruby default internal
# encoding is used. If not explicitly set this default internal encoding is
# `nil` meaning that by default, no transcoding occurs.
#
# The default internal encoding can be set with the interpreter option `-E`.
# [`Encoding.default_internal`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-default_internal)
# returns the current internal encoding.
#
# ```
# $ ruby -e 'p Encoding.default_internal'
# nil
#
# $ ruby -E ISO-8859-1:UTF-8 -e "p [Encoding.default_external, \
#   Encoding.default_internal]"
# [#<Encoding:ISO-8859-1>, #<Encoding:UTF-8>]
# ```
#
# The default internal encoding may also be set through
# [`Encoding.default_internal=`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-default_internal-3D),
# but you should not do this as strings created before and after the change will
# have inconsistent encodings. Instead use `ruby -E` to invoke ruby with the
# correct internal encoding.
#
# ## [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) encoding example
#
# In the following example a UTF-8 encoded string "Ru00E9sumu00E9" is transcoded
# for output to ISO-8859-1 encoding, then read back in and transcoded to UTF-8:
#
# ```ruby
# string = "R\u00E9sum\u00E9"
#
# open("transcoded.txt", "w:ISO-8859-1") do |io|
#   io.write(string)
# end
#
# puts "raw text:"
# p File.binread("transcoded.txt")
# puts
#
# open("transcoded.txt", "r:ISO-8859-1:UTF-8") do |io|
#   puts "transcoded text:"
#   p io.read
# end
# ```
#
# While writing the file, the internal encoding is not specified as it is only
# necessary for reading. While reading the file both the internal and external
# encoding must be specified to obtain the correct result.
#
# ```
# $ ruby t.rb
# raw text:
# "R\xE9sum\xE9"
#
# transcoded text:
# "R\u00E9sum\u00E9"
# ```
class Encoding < Object
  ANSI_X3_4_1968 = T.let(T.unsafe(nil), Encoding)
  ASCII = T.let(T.unsafe(nil), Encoding)
  ASCII_8BIT = T.let(T.unsafe(nil), Encoding)
  BIG5 = T.let(T.unsafe(nil), Encoding)
  BIG5_HKSCS = T.let(T.unsafe(nil), Encoding)
  BIG5_HKSCS_2008 = T.let(T.unsafe(nil), Encoding)
  BIG5_UAO = T.let(T.unsafe(nil), Encoding)
  BINARY = T.let(T.unsafe(nil), Encoding)
  Big5 = T.let(T.unsafe(nil), Encoding)
  Big5_HKSCS = T.let(T.unsafe(nil), Encoding)
  Big5_HKSCS_2008 = T.let(T.unsafe(nil), Encoding)
  Big5_UAO = T.let(T.unsafe(nil), Encoding)
  CP1250 = T.let(T.unsafe(nil), Encoding)
  CP1251 = T.let(T.unsafe(nil), Encoding)
  CP1252 = T.let(T.unsafe(nil), Encoding)
  CP1253 = T.let(T.unsafe(nil), Encoding)
  CP1254 = T.let(T.unsafe(nil), Encoding)
  CP1255 = T.let(T.unsafe(nil), Encoding)
  CP1256 = T.let(T.unsafe(nil), Encoding)
  CP1257 = T.let(T.unsafe(nil), Encoding)
  CP1258 = T.let(T.unsafe(nil), Encoding)
  CP437 = T.let(T.unsafe(nil), Encoding)
  CP50220 = T.let(T.unsafe(nil), Encoding)
  CP50221 = T.let(T.unsafe(nil), Encoding)
  CP51932 = T.let(T.unsafe(nil), Encoding)
  CP65000 = T.let(T.unsafe(nil), Encoding)
  CP65001 = T.let(T.unsafe(nil), Encoding)
  CP737 = T.let(T.unsafe(nil), Encoding)
  CP775 = T.let(T.unsafe(nil), Encoding)
  CP850 = T.let(T.unsafe(nil), Encoding)
  CP852 = T.let(T.unsafe(nil), Encoding)
  CP855 = T.let(T.unsafe(nil), Encoding)
  CP857 = T.let(T.unsafe(nil), Encoding)
  CP860 = T.let(T.unsafe(nil), Encoding)
  CP861 = T.let(T.unsafe(nil), Encoding)
  CP862 = T.let(T.unsafe(nil), Encoding)
  CP863 = T.let(T.unsafe(nil), Encoding)
  CP864 = T.let(T.unsafe(nil), Encoding)
  CP865 = T.let(T.unsafe(nil), Encoding)
  CP866 = T.let(T.unsafe(nil), Encoding)
  CP869 = T.let(T.unsafe(nil), Encoding)
  CP874 = T.let(T.unsafe(nil), Encoding)
  CP878 = T.let(T.unsafe(nil), Encoding)
  CP932 = T.let(T.unsafe(nil), Encoding)
  CP936 = T.let(T.unsafe(nil), Encoding)
  CP949 = T.let(T.unsafe(nil), Encoding)
  CP950 = T.let(T.unsafe(nil), Encoding)
  CP951 = T.let(T.unsafe(nil), Encoding)
  CSWINDOWS31J = T.let(T.unsafe(nil), Encoding)
  CsWindows31J = T.let(T.unsafe(nil), Encoding)
  EBCDIC_CP_US = T.let(T.unsafe(nil), Encoding)
  EMACS_MULE = T.let(T.unsafe(nil), Encoding)
  EUCCN = T.let(T.unsafe(nil), Encoding)
  EUCJP = T.let(T.unsafe(nil), Encoding)
  EUCJP_MS = T.let(T.unsafe(nil), Encoding)
  EUCKR = T.let(T.unsafe(nil), Encoding)
  EUCTW = T.let(T.unsafe(nil), Encoding)
  EUC_CN = T.let(T.unsafe(nil), Encoding)
  EUC_JISX0213 = T.let(T.unsafe(nil), Encoding)
  EUC_JIS_2004 = T.let(T.unsafe(nil), Encoding)
  EUC_JP = T.let(T.unsafe(nil), Encoding)
  EUC_JP_MS = T.let(T.unsafe(nil), Encoding)
  EUC_KR = T.let(T.unsafe(nil), Encoding)
  EUC_TW = T.let(T.unsafe(nil), Encoding)
  Emacs_Mule = T.let(T.unsafe(nil), Encoding)
  EucCN = T.let(T.unsafe(nil), Encoding)
  EucJP = T.let(T.unsafe(nil), Encoding)
  EucJP_ms = T.let(T.unsafe(nil), Encoding)
  EucKR = T.let(T.unsafe(nil), Encoding)
  EucTW = T.let(T.unsafe(nil), Encoding)
  GB12345 = T.let(T.unsafe(nil), Encoding)
  GB18030 = T.let(T.unsafe(nil), Encoding)
  GB1988 = T.let(T.unsafe(nil), Encoding)
  GB2312 = T.let(T.unsafe(nil), Encoding)
  GBK = T.let(T.unsafe(nil), Encoding)
  IBM037 = T.let(T.unsafe(nil), Encoding)
  IBM437 = T.let(T.unsafe(nil), Encoding)
  IBM737 = T.let(T.unsafe(nil), Encoding)
  IBM775 = T.let(T.unsafe(nil), Encoding)
  IBM850 = T.let(T.unsafe(nil), Encoding)
  IBM852 = T.let(T.unsafe(nil), Encoding)
  IBM855 = T.let(T.unsafe(nil), Encoding)
  IBM857 = T.let(T.unsafe(nil), Encoding)
  IBM860 = T.let(T.unsafe(nil), Encoding)
  IBM861 = T.let(T.unsafe(nil), Encoding)
  IBM862 = T.let(T.unsafe(nil), Encoding)
  IBM863 = T.let(T.unsafe(nil), Encoding)
  IBM864 = T.let(T.unsafe(nil), Encoding)
  IBM865 = T.let(T.unsafe(nil), Encoding)
  IBM866 = T.let(T.unsafe(nil), Encoding)
  IBM869 = T.let(T.unsafe(nil), Encoding)
  ISO2022_JP = T.let(T.unsafe(nil), Encoding)
  ISO2022_JP2 = T.let(T.unsafe(nil), Encoding)
  ISO8859_1 = T.let(T.unsafe(nil), Encoding)
  ISO8859_10 = T.let(T.unsafe(nil), Encoding)
  ISO8859_11 = T.let(T.unsafe(nil), Encoding)
  ISO8859_13 = T.let(T.unsafe(nil), Encoding)
  ISO8859_14 = T.let(T.unsafe(nil), Encoding)
  ISO8859_15 = T.let(T.unsafe(nil), Encoding)
  ISO8859_16 = T.let(T.unsafe(nil), Encoding)
  ISO8859_2 = T.let(T.unsafe(nil), Encoding)
  ISO8859_3 = T.let(T.unsafe(nil), Encoding)
  ISO8859_4 = T.let(T.unsafe(nil), Encoding)
  ISO8859_5 = T.let(T.unsafe(nil), Encoding)
  ISO8859_6 = T.let(T.unsafe(nil), Encoding)
  ISO8859_7 = T.let(T.unsafe(nil), Encoding)
  ISO8859_8 = T.let(T.unsafe(nil), Encoding)
  ISO8859_9 = T.let(T.unsafe(nil), Encoding)
  ISO_2022_JP = T.let(T.unsafe(nil), Encoding)
  ISO_2022_JP_2 = T.let(T.unsafe(nil), Encoding)
  ISO_2022_JP_KDDI = T.let(T.unsafe(nil), Encoding)
  ISO_8859_1 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_10 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_11 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_13 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_14 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_15 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_16 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_2 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_3 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_4 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_5 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_6 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_7 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_8 = T.let(T.unsafe(nil), Encoding)
  ISO_8859_9 = T.let(T.unsafe(nil), Encoding)
  KOI8_R = T.let(T.unsafe(nil), Encoding)
  KOI8_U = T.let(T.unsafe(nil), Encoding)
  MACCENTEURO = T.let(T.unsafe(nil), Encoding)
  MACCROATIAN = T.let(T.unsafe(nil), Encoding)
  MACCYRILLIC = T.let(T.unsafe(nil), Encoding)
  MACGREEK = T.let(T.unsafe(nil), Encoding)
  MACICELAND = T.let(T.unsafe(nil), Encoding)
  MACJAPAN = T.let(T.unsafe(nil), Encoding)
  MACJAPANESE = T.let(T.unsafe(nil), Encoding)
  MACROMAN = T.let(T.unsafe(nil), Encoding)
  MACROMANIA = T.let(T.unsafe(nil), Encoding)
  MACTHAI = T.let(T.unsafe(nil), Encoding)
  MACTURKISH = T.let(T.unsafe(nil), Encoding)
  MACUKRAINE = T.let(T.unsafe(nil), Encoding)
  MacCentEuro = T.let(T.unsafe(nil), Encoding)
  MacCroatian = T.let(T.unsafe(nil), Encoding)
  MacCyrillic = T.let(T.unsafe(nil), Encoding)
  MacGreek = T.let(T.unsafe(nil), Encoding)
  MacIceland = T.let(T.unsafe(nil), Encoding)
  MacJapan = T.let(T.unsafe(nil), Encoding)
  MacJapanese = T.let(T.unsafe(nil), Encoding)
  MacRoman = T.let(T.unsafe(nil), Encoding)
  MacRomania = T.let(T.unsafe(nil), Encoding)
  MacThai = T.let(T.unsafe(nil), Encoding)
  MacTurkish = T.let(T.unsafe(nil), Encoding)
  MacUkraine = T.let(T.unsafe(nil), Encoding)
  PCK = T.let(T.unsafe(nil), Encoding)
  SHIFT_JIS = T.let(T.unsafe(nil), Encoding)
  SJIS = T.let(T.unsafe(nil), Encoding)
  SJIS_DOCOMO = T.let(T.unsafe(nil), Encoding)
  SJIS_DoCoMo = T.let(T.unsafe(nil), Encoding)
  SJIS_KDDI = T.let(T.unsafe(nil), Encoding)
  SJIS_SOFTBANK = T.let(T.unsafe(nil), Encoding)
  SJIS_SoftBank = T.let(T.unsafe(nil), Encoding)
  STATELESS_ISO_2022_JP = T.let(T.unsafe(nil), Encoding)
  STATELESS_ISO_2022_JP_KDDI = T.let(T.unsafe(nil), Encoding)
  Shift_JIS = T.let(T.unsafe(nil), Encoding)
  Stateless_ISO_2022_JP = T.let(T.unsafe(nil), Encoding)
  Stateless_ISO_2022_JP_KDDI = T.let(T.unsafe(nil), Encoding)
  TIS_620 = T.let(T.unsafe(nil), Encoding)
  UCS_2BE = T.let(T.unsafe(nil), Encoding)
  UCS_4BE = T.let(T.unsafe(nil), Encoding)
  UCS_4LE = T.let(T.unsafe(nil), Encoding)
  US_ASCII = T.let(T.unsafe(nil), Encoding)
  UTF8_DOCOMO = T.let(T.unsafe(nil), Encoding)
  UTF8_DoCoMo = T.let(T.unsafe(nil), Encoding)
  UTF8_KDDI = T.let(T.unsafe(nil), Encoding)
  UTF8_MAC = T.let(T.unsafe(nil), Encoding)
  UTF8_SOFTBANK = T.let(T.unsafe(nil), Encoding)
  UTF8_SoftBank = T.let(T.unsafe(nil), Encoding)
  UTF_16 = T.let(T.unsafe(nil), Encoding)
  UTF_16BE = T.let(T.unsafe(nil), Encoding)
  UTF_16LE = T.let(T.unsafe(nil), Encoding)
  UTF_32 = T.let(T.unsafe(nil), Encoding)
  UTF_32BE = T.let(T.unsafe(nil), Encoding)
  UTF_32LE = T.let(T.unsafe(nil), Encoding)
  UTF_7 = T.let(T.unsafe(nil), Encoding)
  UTF_8 = T.let(T.unsafe(nil), Encoding)
  UTF_8_HFS = T.let(T.unsafe(nil), Encoding)
  UTF_8_MAC = T.let(T.unsafe(nil), Encoding)
  WINDOWS_1250 = T.let(T.unsafe(nil), Encoding)
  WINDOWS_1251 = T.let(T.unsafe(nil), Encoding)
  WINDOWS_1252 = T.let(T.unsafe(nil), Encoding)
  WINDOWS_1253 = T.let(T.unsafe(nil), Encoding)
  WINDOWS_1254 = T.let(T.unsafe(nil), Encoding)
  WINDOWS_1255 = T.let(T.unsafe(nil), Encoding)
  WINDOWS_1256 = T.let(T.unsafe(nil), Encoding)
  WINDOWS_1257 = T.let(T.unsafe(nil), Encoding)
  WINDOWS_1258 = T.let(T.unsafe(nil), Encoding)
  WINDOWS_31J = T.let(T.unsafe(nil), Encoding)
  WINDOWS_874 = T.let(T.unsafe(nil), Encoding)
  Windows_1250 = T.let(T.unsafe(nil), Encoding)
  Windows_1251 = T.let(T.unsafe(nil), Encoding)
  Windows_1252 = T.let(T.unsafe(nil), Encoding)
  Windows_1253 = T.let(T.unsafe(nil), Encoding)
  Windows_1254 = T.let(T.unsafe(nil), Encoding)
  Windows_1255 = T.let(T.unsafe(nil), Encoding)
  Windows_1256 = T.let(T.unsafe(nil), Encoding)
  Windows_1257 = T.let(T.unsafe(nil), Encoding)
  Windows_1258 = T.let(T.unsafe(nil), Encoding)
  Windows_31J = T.let(T.unsafe(nil), Encoding)
  Windows_874 = T.let(T.unsafe(nil), Encoding)

  # Returns the hash of available encoding alias and original encoding name.
  #
  # ```
  # Encoding.aliases
  # #=> {"BINARY"=>"ASCII-8BIT", "ASCII"=>"US-ASCII", "ANSI_X3.4-1986"=>"US-ASCII",
  #       "SJIS"=>"Shift_JIS", "eucJP"=>"EUC-JP", "CP932"=>"Windows-31J"}
  # ```
  sig {returns(T::Hash[String, String])}
  def self.aliases(); end

  # Checks the compatibility of two objects.
  #
  # If the objects are both strings they are compatible when they are
  # concatenatable. The encoding of the concatenated string will be returned if
  # they are compatible, nil if they are not.
  #
  # ```ruby
  # Encoding.compatible?("\xa1".force_encoding("iso-8859-1"), "b")
  # #=> #<Encoding:ISO-8859-1>
  #
  # Encoding.compatible?(
  #   "\xa1".force_encoding("iso-8859-1"),
  #   "\xa1\xa1".force_encoding("euc-jp"))
  # #=> nil
  # ```
  #
  # If the objects are non-strings their encodings are compatible when they have
  # an encoding and:
  # *   Either encoding is US-ASCII compatible
  # *   One of the encodings is a 7-bit encoding
  sig do
    params(
        obj1: BasicObject,
        obj2: BasicObject,
    )
    .returns(T.nilable(Encoding))
  end
  def self.compatible?(obj1, obj2); end

  # Returns default external encoding.
  #
  # The default external encoding is used by default for strings created from
  # the following locations:
  #
  # *   [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html)
  # *   [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html) data read from
  #     disk
  # *   [`SDBM`](https://docs.ruby-lang.org/en/2.6.0/SDBM.html)
  # *   [`StringIO`](https://docs.ruby-lang.org/en/2.6.0/StringIO.html)
  # *   [`Zlib::GzipReader`](https://docs.ruby-lang.org/en/2.6.0/Zlib/GzipReader.html)
  # *   [`Zlib::GzipWriter`](https://docs.ruby-lang.org/en/2.6.0/Zlib/GzipWriter.html)
  # *   [`String#inspect`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-inspect)
  # *   [`Regexp#inspect`](https://docs.ruby-lang.org/en/2.6.0/Regexp.html#method-i-inspect)
  #
  #
  # While strings created from these locations will have this encoding, the
  # encoding may not be valid. Be sure to check
  # [`String#valid_encoding?`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-valid_encoding-3F).
  #
  # [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html) data written to disk
  # will be transcoded to the default external encoding when written.
  #
  # The default external encoding is initialized by the locale or -E option.
  sig {returns(Encoding)}
  def self.default_external(); end

  # Sets default external encoding. You should not set
  # [`Encoding::default_external`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-default_external)
  # in ruby code as strings created before changing the value may have a
  # different encoding from strings created after the value was changed.,
  # instead you should use `ruby -E` to invoke ruby with the correct
  # default\_external.
  #
  # See
  # [`Encoding::default_external`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-default_external)
  # for information on how the default external encoding is used.
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: Encoding,
    )
    .returns(Encoding)
  end
  def self.default_external=(arg0); end

  # Returns default internal encoding. Strings will be transcoded to the default
  # internal encoding in the following places if the default internal encoding
  # is not nil:
  #
  # *   [`CSV`](https://docs.ruby-lang.org/en/2.6.0/CSV.html)
  # *   [`Etc.sysconfdir`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-sysconfdir)
  #     and
  #     [`Etc.systmpdir`](https://docs.ruby-lang.org/en/2.6.0/Etc.html#method-c-systmpdir)
  # *   [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html) data read from
  #     disk
  # *   [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html) names from
  #     [`Dir`](https://docs.ruby-lang.org/en/2.6.0/Dir.html)
  # *   [`Integer#chr`](https://docs.ruby-lang.org/en/2.6.0/Integer.html#method-i-chr)
  # *   [`String#inspect`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-inspect)
  #     and
  #     [`Regexp#inspect`](https://docs.ruby-lang.org/en/2.6.0/Regexp.html#method-i-inspect)
  # *   Strings returned from
  #     [`Readline`](https://docs.ruby-lang.org/en/2.6.0/Readline.html)
  # *   Strings returned from
  #     [`SDBM`](https://docs.ruby-lang.org/en/2.6.0/SDBM.html)
  # *   [`Time#zone`](https://docs.ruby-lang.org/en/2.6.0/Time.html#method-i-zone)
  # *   Values from [`ENV`](https://docs.ruby-lang.org/en/2.6.0/ENV.html)
  # *   Values in ARGV including $PROGRAM\_NAME
  #
  #
  # Additionally
  # [`String#encode`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-encode)
  # and
  # [`String#encode!`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-encode-21)
  # use the default internal encoding if no encoding is given.
  #
  # The locale encoding (\_\_ENCODING\_\_), not
  # [`default_internal`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-default_internal),
  # is used as the encoding of created strings.
  #
  # [`Encoding::default_internal`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-default_internal)
  # is initialized by the source file's internal\_encoding or -E option.
  sig {returns(Encoding)}
  def self.default_internal(); end

  # Sets default internal encoding or removes default internal encoding when
  # passed nil. You should not set
  # [`Encoding::default_internal`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-default_internal)
  # in ruby code as strings created before changing the value may have a
  # different encoding from strings created after the change. Instead you should
  # use `ruby -E` to invoke ruby with the correct default\_internal.
  #
  # See
  # [`Encoding::default_internal`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html#method-c-default_internal)
  # for information on how the default internal encoding is used.
  sig do
    params(
        arg0: String,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Encoding,
    )
    .returns(T.nilable(Encoding))
  end
  def self.default_internal=(arg0); end

  # Search the encoding with specified *name*. *name* should be a string.
  #
  # ```ruby
  # Encoding.find("US-ASCII")  #=> #<Encoding:US-ASCII>
  # ```
  #
  # Names which this method accept are encoding names and aliases including
  # following special aliases
  #
  # "external"
  # :   default external encoding
  # "internal"
  # :   default internal encoding
  # "locale"
  # :   locale encoding
  # "filesystem"
  # :   filesystem encoding
  #
  #
  # An [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html)
  # is raised when no encoding with *name*. Only `Encoding.find("internal")`
  # however returns nil when no encoding named "internal", in other words, when
  # Ruby has no default internal encoding.
  sig do
    params(
        arg0: T.any(String, Encoding),
    )
    .returns(Encoding)
  end
  def self.find(arg0); end

  # Returns the list of loaded encodings.
  #
  # ```ruby
  # Encoding.list
  # #=> [#<Encoding:ASCII-8BIT>, #<Encoding:UTF-8>,
  #       #<Encoding:ISO-2022-JP (dummy)>]
  #
  # Encoding.find("US-ASCII")
  # #=> #<Encoding:US-ASCII>
  #
  # Encoding.list
  # #=> [#<Encoding:ASCII-8BIT>, #<Encoding:UTF-8>,
  #       #<Encoding:US-ASCII>, #<Encoding:ISO-2022-JP (dummy)>]
  # ```
  sig {returns(T::Array[Encoding])}
  def self.list(); end

  # Returns the list of available encoding names.
  #
  # ```
  # Encoding.name_list
  # #=> ["US-ASCII", "ASCII-8BIT", "UTF-8",
  #       "ISO-8859-1", "Shift_JIS", "EUC-JP",
  #       "Windows-31J",
  #       "BINARY", "CP932", "eucJP"]
  # ```
  sig {returns(T::Array[String])}
  def self.name_list(); end

  # Returns whether ASCII-compatible or not.
  #
  # ```ruby
  # Encoding::UTF_8.ascii_compatible?     #=> true
  # Encoding::UTF_16BE.ascii_compatible?  #=> false
  # ```
  sig {returns(T::Boolean)}
  def ascii_compatible?(); end

  # Returns true for dummy encodings. A dummy encoding is an encoding for which
  # character handling is not properly implemented. It is used for stateful
  # encodings.
  #
  # ```ruby
  # Encoding::ISO_2022_JP.dummy?       #=> true
  # Encoding::UTF_8.dummy?             #=> false
  # ```
  sig {returns(T::Boolean)}
  def dummy?(); end

  # Returns a string which represents the encoding for programmers.
  #
  # ```ruby
  # Encoding::UTF_8.inspect       #=> "#<Encoding:UTF-8>"
  # Encoding::ISO_2022_JP.inspect #=> "#<Encoding:ISO-2022-JP (dummy)>"
  # ```
  sig {returns(String)}
  def inspect(); end

  # Returns the name of the encoding.
  #
  # ```ruby
  # Encoding::UTF_8.name      #=> "UTF-8"
  # ```
  sig {returns(String)}
  def name(); end

  # Returns the list of name and aliases of the encoding.
  #
  # ```ruby
  # Encoding::WINDOWS_31J.names  #=> ["Windows-31J", "CP932", "csWindows31J"]
  # ```
  sig {returns(T::Array[String])}
  def names(); end

  # Returns a replicated encoding of *enc* whose name is *name*. The new
  # encoding should have the same byte structure of *enc*. If *name* is used by
  # another encoding, raise
  # [`ArgumentError`](https://docs.ruby-lang.org/en/2.6.0/ArgumentError.html).
  sig do
    params(
        name: String,
    )
    .returns(Encoding)
  end
  def replicate(name); end

  # Returns the name of the encoding.
  #
  # ```ruby
  # Encoding::UTF_8.name      #=> "UTF-8"
  # ```
  sig {returns(String)}
  def to_s(); end
end

# [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) conversion
# class.
class Encoding::Converter < Data
  # [`AFTER_OUTPUT`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#AFTER_OUTPUT)
  #
  # Stop converting after some output is complete but before all of the input
  # was consumed. See
  # [`primitive_convert`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#method-i-primitive_convert)
  # for an example.
  AFTER_OUTPUT = T.let(T.unsafe(nil), Integer)
  # [`CRLF_NEWLINE_DECORATOR`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#CRLF_NEWLINE_DECORATOR)
  #
  # Decorator for converting LF to CRLF
  CRLF_NEWLINE_DECORATOR = T.let(T.unsafe(nil), Integer)
  # [`CR_NEWLINE_DECORATOR`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#CR_NEWLINE_DECORATOR)
  #
  # Decorator for converting LF to CR
  CR_NEWLINE_DECORATOR = T.let(T.unsafe(nil), Integer)
  # [`INVALID_MASK`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#INVALID_MASK)
  #
  # Mask for invalid byte sequences
  INVALID_MASK = T.let(T.unsafe(nil), Integer)
  # [`INVALID_REPLACE`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#INVALID_REPLACE)
  #
  # Replace invalid byte sequences
  INVALID_REPLACE = T.let(T.unsafe(nil), Integer)
  # [`PARTIAL_INPUT`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#PARTIAL_INPUT)
  #
  # Indicates the source may be part of a larger string. See
  # [`primitive_convert`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#method-i-primitive_convert)
  # for an example.
  PARTIAL_INPUT = T.let(T.unsafe(nil), Integer)
  # [`UNDEF_HEX_CHARREF`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#UNDEF_HEX_CHARREF)
  #
  # Replace byte sequences that are undefined in the destination encoding with
  # an [`XML`](https://docs.ruby-lang.org/en/2.6.0/XML.html) hexadecimal
  # character reference. This is valid for
  # [`XML`](https://docs.ruby-lang.org/en/2.6.0/XML.html) conversion.
  UNDEF_HEX_CHARREF = T.let(T.unsafe(nil), Integer)
  # [`UNDEF_MASK`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#UNDEF_MASK)
  #
  # Mask for a valid character in the source encoding but no related
  # character(s) in destination encoding.
  UNDEF_MASK = T.let(T.unsafe(nil), Integer)
  # [`UNDEF_REPLACE`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#UNDEF_REPLACE)
  #
  # Replace byte sequences that are undefined in the destination encoding.
  UNDEF_REPLACE = T.let(T.unsafe(nil), Integer)
  # [`UNIVERSAL_NEWLINE_DECORATOR`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#UNIVERSAL_NEWLINE_DECORATOR)
  #
  # Decorator for converting CRLF and CR to LF
  UNIVERSAL_NEWLINE_DECORATOR = T.let(T.unsafe(nil), Integer)
  # [`XML_ATTR_CONTENT_DECORATOR`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#XML_ATTR_CONTENT_DECORATOR)
  #
  # Escape as [`XML`](https://docs.ruby-lang.org/en/2.6.0/XML.html) AttValue
  XML_ATTR_CONTENT_DECORATOR = T.let(T.unsafe(nil), Integer)
  # [`XML_ATTR_QUOTE_DECORATOR`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#XML_ATTR_QUOTE_DECORATOR)
  #
  # Escape as [`XML`](https://docs.ruby-lang.org/en/2.6.0/XML.html) AttValue
  XML_ATTR_QUOTE_DECORATOR = T.let(T.unsafe(nil), Integer)
  # [`XML_TEXT_DECORATOR`](https://docs.ruby-lang.org/en/2.6.0/Encoding/Converter.html#XML_TEXT_DECORATOR)
  #
  # Escape as [`XML`](https://docs.ruby-lang.org/en/2.6.0/XML.html) CharData
  XML_TEXT_DECORATOR = T.let(T.unsafe(nil), Integer)
end

# Raised by [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) and
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) methods when the
# source encoding is incompatible with the target encoding.
class Encoding::CompatibilityError < EncodingError
end

# Raised by transcoding methods when a named encoding does not correspond with a
# known converter.
class Encoding::ConverterNotFoundError < EncodingError
end

# Raised by [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) and
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) methods when the
# string being transcoded contains a byte invalid for the either the source or
# target encoding.
class Encoding::InvalidByteSequenceError < EncodingError
end

# Raised by [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) and
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) methods when a
# transcoding operation fails.
class Encoding::UndefinedConversionError < EncodingError
end
