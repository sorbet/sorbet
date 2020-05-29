# typed: __STDLIB_INTERNAL

# Kanji Converter for Ruby.
module Kconv
  # [`ASCII`](https://docs.ruby-lang.org/en/2.6.0/Kconv.html#ASCII)
  ASCII = T.let(T.unsafe(nil), Encoding)

  # [`BINARY`](https://docs.ruby-lang.org/en/2.6.0/Kconv.html#BINARY)
  BINARY = T.let(T.unsafe(nil), Encoding)

  # EUC-JP
  EUC = T.let(T.unsafe(nil), Encoding)

  # ISO-2022-JP
  JIS = T.let(T.unsafe(nil), Encoding)

  # Shift\_JIS
  SJIS = T.let(T.unsafe(nil), Encoding)

  # UTF-16
  UTF16 = T.let(T.unsafe(nil), Encoding)

  # UTF-32
  UTF32 = T.let(T.unsafe(nil), Encoding)

  # UTF-8
  UTF8 = T.let(T.unsafe(nil), Encoding)

  # Guess input encoding by
  # [`NKF.guess`](https://docs.ruby-lang.org/en/2.6.0/NKF.html#method-c-guess)
  sig { params(str: String).returns(Encoding) }
  def self.guess(str); end

  # Returns whether input encoding is EUC-JP or not.
  #
  # **Note** don't expect this return value is
  # [`MatchData`](https://docs.ruby-lang.org/en/2.6.0/MatchData.html).
  sig { params(str: String).returns(T::Boolean) }
  def self.iseuc(str); end

  # Returns whether input encoding is ISO-2022-JP or not.
  sig { params(str: String).returns(T::Boolean) }
  def self.isjis(str); end

  # Returns whether input encoding is Shift\_JIS or not.
  sig { params(str: String).returns(T::Boolean) }
  def self.issjis(str); end

  # Returns whether input encoding is UTF-8 or not.
  sig { params(str: String).returns(T::Boolean) }
  def self.isutf8(str); end

  # Convert `str` to `to_enc`. `to_enc` and `from_enc` are given as constants of
  # [`Kconv`](https://docs.ruby-lang.org/en/2.6.0/Kconv.html) or
  # [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html) objects.
  sig { params(str: String, to_enc: Encoding, from_enc: Encoding).returns(String) }
  def self.kconv(str, to_enc, from_enc = to_enc); end

  # Convert `str` to EUC-JP
  sig { params(str: String).returns(String) }
  def self.toeuc(str); end

  # Convert `str` to ISO-2022-JP
  sig { params(str: String).returns(String) }
  def self.tojis(str); end

  # Convert `self` to locale encoding
  sig { params(str: String).returns(String) }
  def self.tolocale(str); end

  # Convert `str` to Shift\_JIS
  sig { params(str: String).returns(String) }
  def self.tosjis(str); end

  # Convert `str` to UTF-16
  sig { params(str: String).returns(String) }
  def self.toutf16(str); end

  # Convert `str` to UTF-32
  sig { params(str: String).returns(String) }
  def self.toutf32(str); end

  # Convert `str` to UTF-8
  sig { params(str: String).returns(String) }
  def self.toutf8(str); end
end
