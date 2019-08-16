# typed: __STDLIB_INTERNAL

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
  #     Encoding.aliases
  #     #=> {"BINARY"=>"ASCII-8BIT", "ASCII"=>"US-ASCII", "ANSI_X3.4-1986"=>"US-ASCII",
  #           "SJIS"=>"Shift_JIS", "eucJP"=>"EUC-JP", "CP932"=>"Windows-31J"}
  sig {returns(T::Hash[String, String])}
  def self.aliases(); end

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
  # The default external encoding is used by default for strings created
  # from the following locations:
  #
  #   - CSV
  #
  #   - [File](https://ruby-doc.org/core-2.6.3/File.html) data read from
  #     disk
  #
  #   - SDBM
  #
  #   - StringIO
  #
  #   - Zlib::GzipReader
  #
  #   - Zlib::GzipWriter
  #
  #   - [String\#inspect](https://ruby-doc.org/core-2.6.3/String.html#method-i-inspect)
  #
  #   - [Regexp\#inspect](https://ruby-doc.org/core-2.6.3/Regexp.html#method-i-inspect)
  #
  # While strings created from these locations will have this encoding, the
  # encoding may not be valid. Be sure to check
  # [String\#valid\_encoding?](https://ruby-doc.org/core-2.6.3/String.html#method-i-valid_encoding-3F)
  # .
  #
  # [File](https://ruby-doc.org/core-2.6.3/File.html) data written to disk
  # will be transcoded to the default external encoding when written.
  #
  # The default external encoding is initialized by the locale or -E option.
  sig {returns(Encoding)}
  def self.default_external(); end

  # Sets default external encoding. You should not set
  # [::default\_external](Encoding.downloaded.ruby_doc#method-c-default_external)
  # in ruby code as strings created before changing the value may have a
  # different encoding from strings created after the value was changed.,
  # instead you should use `ruby -E` to invoke ruby with the correct
  # default\_external.
  #
  # See
  # [::default\_external](Encoding.downloaded.ruby_doc#method-c-default_external)
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

  # Returns default internal encoding. Strings will be transcoded to the
  # default internal encoding in the following places if the default
  # internal encoding is not nil:
  #
  #   - CSV
  #
  #   - Etc.sysconfdir and Etc.systmpdir
  #
  #   - [File](https://ruby-doc.org/core-2.6.3/File.html) data read from
  #     disk
  #
  #   - [File](https://ruby-doc.org/core-2.6.3/File.html) names from
  #     [Dir](https://ruby-doc.org/core-2.6.3/Dir.html)
  #
  #   - [Integer\#chr](https://ruby-doc.org/core-2.6.3/Integer.html#method-i-chr)
  #
  #   - [String\#inspect](https://ruby-doc.org/core-2.6.3/String.html#method-i-inspect)
  #     and
  #     [Regexp\#inspect](https://ruby-doc.org/core-2.6.3/Regexp.html#method-i-inspect)
  #
  #   - Strings returned from Readline
  #
  #   - Strings returned from SDBM
  #
  #   - [Time\#zone](https://ruby-doc.org/core-2.6.3/Time.html#method-i-zone)
  #
  #   - Values from [ENV](https://ruby-doc.org/core-2.6.3/ENV.html)
  #
  #   - Values in ARGV including $PROGRAM\_NAME
  #
  # Additionally
  # [String\#encode](https://ruby-doc.org/core-2.6.3/String.html#method-i-encode)
  # and
  # [String\#encode\!](https://ruby-doc.org/core-2.6.3/String.html#method-i-encode-21)
  # use the default internal encoding if no encoding is given.
  #
  # The locale encoding (\_\_ENCODING\_\_), not
  # [::default\_internal](Encoding.downloaded.ruby_doc#method-c-default_internal)
  # , is used as the encoding of created strings.
  #
  # [::default\_internal](Encoding.downloaded.ruby_doc#method-c-default_internal)
  # is initialized by the source file's internal\_encoding or -E option.
  sig {returns(Encoding)}
  def self.default_internal(); end

  # Sets default internal encoding or removes default internal encoding when
  # passed nil. You should not set
  # [::default\_internal](Encoding.downloaded.ruby_doc#method-c-default_internal)
  # in ruby code as strings created before changing the value may have a
  # different encoding from strings created after the change. Instead you
  # should use `ruby -E` to invoke ruby with the correct default\_internal.
  #
  # See
  # [::default\_internal](Encoding.downloaded.ruby_doc#method-c-default_internal)
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

  sig do
    params(
        arg0: T.any(String, Encoding),
    )
    .returns(Encoding)
  end
  def self.find(arg0); end

  sig {returns(T::Array[Encoding])}
  def self.list(); end

  # Returns the list of available encoding names.
  #
  #     Encoding.name_list
  #     #=> ["US-ASCII", "ASCII-8BIT", "UTF-8",
  #           "ISO-8859-1", "Shift_JIS", "EUC-JP",
  #           "Windows-31J",
  #           "BINARY", "CP932", "eucJP"]
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

  # Returns true for dummy encodings. A dummy encoding is an encoding for
  # which character handling is not properly implemented. It is used for
  # stateful encodings.
  #
  # ```ruby
  # Encoding::ISO_2022_JP.dummy?       #=> true
  # Encoding::UTF_8.dummy?             #=> false
  # ```
  sig {returns(T::Boolean)}
  def dummy?(); end

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

  # Returns a replicated encoding of *enc* whose name is *name* . The new
  # encoding should have the same byte structure of *enc* . If *name* is
  # used by another encoding, raise
  # [ArgumentError](https://ruby-doc.org/core-2.6.3/ArgumentError.html).
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

class Encoding::Converter < Data
  AFTER_OUTPUT = T.let(T.unsafe(nil), Integer)
  CRLF_NEWLINE_DECORATOR = T.let(T.unsafe(nil), Integer)
  CR_NEWLINE_DECORATOR = T.let(T.unsafe(nil), Integer)
  INVALID_MASK = T.let(T.unsafe(nil), Integer)
  INVALID_REPLACE = T.let(T.unsafe(nil), Integer)
  PARTIAL_INPUT = T.let(T.unsafe(nil), Integer)
  UNDEF_HEX_CHARREF = T.let(T.unsafe(nil), Integer)
  UNDEF_MASK = T.let(T.unsafe(nil), Integer)
  UNDEF_REPLACE = T.let(T.unsafe(nil), Integer)
  UNIVERSAL_NEWLINE_DECORATOR = T.let(T.unsafe(nil), Integer)
  XML_ATTR_CONTENT_DECORATOR = T.let(T.unsafe(nil), Integer)
  XML_ATTR_QUOTE_DECORATOR = T.let(T.unsafe(nil), Integer)
  XML_TEXT_DECORATOR = T.let(T.unsafe(nil), Integer)
end

class Encoding::CompatibilityError < EncodingError
end

class Encoding::ConverterNotFoundError < EncodingError
end

class Encoding::InvalidByteSequenceError < EncodingError
end

class Encoding::UndefinedConversionError < EncodingError
end
