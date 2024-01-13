# typed: __STDLIB_INTERNAL

# [`NKF`](https://docs.ruby-lang.org/en/2.7.0/NKF.html) - Ruby extension for
# Network Kanji Filter
#
# ## Description
#
# This is a Ruby Extension version of nkf (Network Kanji Filter). It converts
# the first argument and returns converted result. Conversion details are
# specified by flags as the first argument.
#
# **Nkf** is a yet another kanji code converter among networks, hosts and
# terminals. It converts input kanji code to designated kanji code such as
# ISO-2022-JP, Shift\_JIS, EUC-JP, UTF-8 or UTF-16.
#
# One of the most unique faculty of **nkf** is the guess of the input kanji
# encodings. It currently recognizes ISO-2022-JP, Shift\_JIS, EUC-JP, UTF-8 and
# UTF-16. So users needn't set the input kanji code explicitly.
#
# By default, X0201 kana is converted into X0208 kana. For X0201 kana, SO/SI,
# SSO and ESC-(-I methods are supported. For automatic code detection, nkf
# assumes no X0201 kana in Shift\_JIS. To accept X0201 in Shift\_JIS, use
# **-X**, **-x** or **-S**.
#
# ## Flags
#
# ### -b -u
#
# Output is buffered (DEFAULT), Output is unbuffered.
#
# ### -j -s -e -w -w16 -w32
#
# Output code is ISO-2022-JP (7bit
# [`JIS`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#JIS)), Shift\_JIS,
# EUC-JP, UTF-8N, UTF-16BE, UTF-32BE. Without this option and compile option,
# ISO-2022-JP is assumed.
#
# ### -J -S -E -W -W16 -W32
#
# Input assumption is [`JIS`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#JIS)
# 7 bit, Shift\_JIS, EUC-JP, UTF-8, UTF-16, UTF-32.
#
# #### -J
#
# Assume  [`JIS`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#JIS) input. It
# also accepts EUC-JP. This is the default. This flag does not exclude
# Shift\_JIS.
#
# #### -S
#
# Assume Shift\_JIS and X0201 kana input. It also accepts
# [`JIS`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#JIS). EUC-JP is
# recognized as X0201 kana. Without **-x** flag, X0201 kana (halfwidth kana) is
# converted into X0208.
#
# #### -E
#
# Assume EUC-JP input. It also accepts
# [`JIS`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#JIS). Same as -J.
#
# ### -t
#
# No conversion.
#
# ### -i\_
#
# Output sequence to designate JIS-kanji. (DEFAULT B)
#
# ### -o\_
#
# Output sequence to designate
# [`ASCII`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#ASCII). (DEFAULT B)
#
# ### -r
#
# {de/en}crypt ROT13/47
#
# ### -[h](123) --hiragana --katakana --katakana-hiragana
#
# -h1 --hiragana
# :   Katakana to Hiragana conversion.
#
# -h2 --katakana
# :   Hiragana to Katakana conversion.
#
# -h3 --katakana-hiragana
# :   Katakana to Hiragana and Hiragana to Katakana conversion.
#
#
# ### -T
#
# Text mode output (MS-DOS)
#
# ### -l
#
# ISO8859-1 (Latin-1) support
#
# ### -f[`m` [- `n`]]
#
# Folding on `m` length with `n` margin in a line. Without this option, fold
# length is 60 and fold margin is 10.
#
# ### -F
#
# New line preserving line folding.
#
# ### -[Z](0-3)
#
# Convert X0208 alphabet (Fullwidth Alphabets) to
# [`ASCII`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#ASCII).
#
# -Z -Z0
# :   Convert X0208 alphabet to
#     [`ASCII`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#ASCII).
#
# -Z1
# :   Converts X0208 kankaku to single
#     [`ASCII`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#ASCII) space.
#
# -Z2
# :   Converts X0208 kankaku to double
#     [`ASCII`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#ASCII) spaces.
#
# -Z3
# :   Replacing Fullwidth >, <, ", & into '&gt;', '&lt;', '&quot;', '&amp;' as
#     in HTML.
#
#
# ### -X -x
#
# Assume X0201 kana in MS-Kanji. With **-X** or without this option, X0201 is
# converted into X0208 Kana. With **-x**, try to preserve X0208 kana and do not
# convert X0201 kana to X0208. In
# [`JIS`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#JIS) output, ESC-(-I is
# used. In [`EUC`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#EUC) output, SSO
# is used.
#
# ### -[B](0-2)
#
# Assume broken JIS-Kanji input, which lost ESC. Useful when your site is using
# old B-News Nihongo patch.
#
# -B1
# :   allows any char after ESC-( or ESC-$.
#
# -B2
# :   forces [`ASCII`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#ASCII) after
#     NL.
#
#
# ### -I
#
# Replacing non iso-2022-jp char into a geta character (substitute character in
# Japanese).
#
# ### -d -c
#
# Delete r in line feed, Add r in line feed.
#
# ### -[m](BQN0)
#
# MIME ISO-2022-JP/ISO8859-1 decode. (DEFAULT) To see ISO8859-1 (Latin-1) -l is
# necessary.
#
# -mB
# :   Decode MIME base64 encoded stream. Remove header or other part before
#
# conversion.
#
# -mQ
# :   Decode MIME quoted stream. '\_' in quoted stream is converted to space.
#
# -mN
# :   Non-strict decoding.
#
# It allows line break in the middle of the base64 encoding.
#
# -m0
# :   No MIME decode.
#
#
# ### -M
#
# MIME encode. Header style. All
# [`ASCII`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#ASCII) code and control
# characters are intact. Kanji conversion is performed before encoding, so this
# cannot be used as a picture encoder.
#
# -MB
# :   MIME encode [`Base64`](https://docs.ruby-lang.org/en/2.7.0/Base64.html)
#     stream.
#
# -MQ
# :   Perform quoted encoding.
#
#
# ### -l
#
# Input and output code is ISO8859-1 (Latin-1) and ISO-2022-JP. **-s**, **-e**
# and **-x** are not compatible with this option.
#
# ### -[L](uwm)
#
# new line mode Without this option, nkf doesn't convert line breaks.
#
# -Lu
# :   unix (LF)
#
# -Lw
# :   windows (CRLF)
#
# -Lm
# :   mac (CR)
#
#
# ### --fj --unix --mac --msdos --windows
#
# convert for these system
#
# ### --jis --euc --sjis --mime --base64
#
# convert for named code
#
# ### --jis-input --euc-input --sjis-input --mime-input --base64-input
#
# assume input system
#
# ### --ic=`input codeset` --oc=`output codeset`
#
# [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the input or output
# codeset. [`NKF`](https://docs.ruby-lang.org/en/2.7.0/NKF.html) supports
# following codesets and those codeset name are case insensitive.
#
# ISO-2022-JP
# :   a.k.a. RFC1468, 7bit
#     [`JIS`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#JIS), JUNET
#
# EUC-JP (eucJP-nkf)
# :   a.k.a. AT&T [`JIS`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#JIS),
#     Japanese [`EUC`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#EUC), UJIS
#
# eucJP-ascii
# :   a.k.a. x-eucjp-open-19970715-ascii
#
# eucJP-ms
# :   a.k.a. x-eucjp-open-19970715-ms
#
# CP51932
# :   Microsoft Version of EUC-JP.
#
# Shift\_JIS
# :   [`SJIS`](https://docs.ruby-lang.org/en/2.7.0/NKF.html#SJIS), MS-Kanji
#
# Windows-31J
# :   a.k.a. CP932
#
# UTF-8
# :   same as UTF-8N
#
# UTF-8N
# :   UTF-8 without BOM
#
# UTF-8-BOM
# :   UTF-8 with BOM
#
# UTF-16
# :   same as UTF-16BE
#
# UTF-16BE
# :   UTF-16 Big Endian without BOM
#
# UTF-16BE-BOM
# :   UTF-16 Big Endian with BOM
#
# UTF-16LE
# :   UTF-16 Little Endian without BOM
#
# UTF-16LE-BOM
# :   UTF-16 Little Endian with BOM
#
# UTF-32
# :   same as UTF-32BE
#
# UTF-32BE
# :   UTF-32 Big Endian without BOM
#
# UTF-32BE-BOM
# :   UTF-32 Big Endian with BOM
#
# UTF-32LE
# :   UTF-32 Little Endian without BOM
#
# UTF-32LE-BOM
# :   UTF-32 Little Endian with BOM
#
# UTF8-MAC
# :   NKDed UTF-8, a.k.a. UTF8-NFD (input only)
#
#
# ### --fb-{skip, html, xml, perl, java, subchar}
#
# Specify the way that nkf handles unassigned characters. Without this option,
# --fb-skip is assumed.
#
# ### --prefix= `escape character` `target character` ..
#
# When nkf converts to Shift\_JIS, nkf adds a specified escape character to
# specified 2nd byte of Shift\_JIS characters. 1st byte of argument is the
# escape character and following bytes are target characters.
#
# ### --no-cp932ext
#
# Handle the characters extended in CP932 as unassigned characters.
#
# ## --no-best-fit-chars
#
# When Unicode to Encoded byte conversion, don't convert characters which is not
# round trip safe. When Unicode to Unicode conversion, with this and -x option,
# nkf can be used as UTF converter. (In other words, without this and -x option,
# nkf doesn't save some characters)
#
# When nkf convert string which related to path, you should use this option.
#
# ### --cap-input
#
# Decode hex encoded characters.
#
# ### --url-input
#
# Unescape percent escaped characters.
#
# ### --
#
# Ignore rest of -option.
module NKF
  ASCII = T.let(T.unsafe(nil), Encoding)

  BINARY = T.let(T.unsafe(nil), Encoding)

  EUC = T.let(T.unsafe(nil), Encoding)

  JIS = T.let(T.unsafe(nil), Encoding)

  # Release date of nkf
  NKF_RELEASE_DATE = T.let(T.unsafe(nil), String)

  # Version of nkf
  NKF_VERSION = T.let(T.unsafe(nil), String)

  SJIS = T.let(T.unsafe(nil), Encoding)

  UTF16 = T.let(T.unsafe(nil), Encoding)

  UTF32 = T.let(T.unsafe(nil), Encoding)

  UTF8 = T.let(T.unsafe(nil), Encoding)

  # Full version string of nkf
  VERSION = T.let(T.unsafe(nil), String)

  # Returns guessed encoding of *str* by nkf routine.
  def self.guess(str); end

  # Convert *str* and return converted result. Conversion details are specified
  # by *opt* as [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  #
  # ```ruby
  # require 'nkf'
  # output = NKF.nkf("-s", input)
  # ```
  def self.nkf(opt, str); end
end
