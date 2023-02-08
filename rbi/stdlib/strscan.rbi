# typed: __STDLIB_INTERNAL
# [`StringScanner`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html)
# provides for lexical scanning operations on a
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html). Here is an
# example of its usage:
#
# ```ruby
# s = StringScanner.new('This is an example string')
# s.eos?               # -> false
#
# p s.scan(/\w+/)      # -> "This"
# p s.scan(/\w+/)      # -> nil
# p s.scan(/\s+/)      # -> " "
# p s.scan(/\s+/)      # -> nil
# p s.scan(/\w+/)      # -> "is"
# s.eos?               # -> false
#
# p s.scan(/\s+/)      # -> " "
# p s.scan(/\w+/)      # -> "an"
# p s.scan(/\s+/)      # -> " "
# p s.scan(/\w+/)      # -> "example"
# p s.scan(/\s+/)      # -> " "
# p s.scan(/\w+/)      # -> "string"
# s.eos?               # -> true
#
# p s.scan(/\s+/)      # -> nil
# p s.scan(/\w+/)      # -> nil
# ```
#
# Scanning a string means remembering the position of a *scan pointer*, which is
# just an index. The point of scanning is to move forward a bit at a time, so
# matches are sought after the scan pointer; usually immediately after it.
#
# Given the string "test string", here are the pertinent scan pointer positions:
#
# ```
#   t e s t   s t r i n g
# 0 1 2 ...             1
#                       0
# ```
#
# When you
# [`scan`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-scan)
# for a pattern (a regular expression), the match must occur at the character
# after the scan pointer. If you use
# [`scan_until`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-scan_until),
# then the match can occur anywhere after the scan pointer. In both cases, the
# scan pointer moves *just beyond* the last character of the match, ready to
# scan again from the next character onwards. This is demonstrated by the
# example above.
#
# ## [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) Categories
#
# There are other methods besides the plain scanners. You can look ahead in the
# string without actually scanning. You can access the most recent match. You
# can modify the string being scanned, reset or terminate the scanner, find out
# or change the position of the scan pointer, skip ahead, and so on.
#
# ### Advancing the Scan Pointer
#
# *   [`getch`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-getch)
# *   [`get_byte`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-get_byte)
# *   [`scan`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-scan)
# *   [`scan_until`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-scan_until)
# *   [`skip`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-skip)
# *   [`skip_until`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-skip_until)
#
#
# ### Looking Ahead
#
# *   [`check`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-check)
# *   [`check_until`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-check_until)
# *   [`exist?`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-exist-3F)
# *   [`match?`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-match-3F)
# *   [`peek`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-peek)
#
#
# ### Finding Where we Are
#
# *   [`beginning_of_line?`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-beginning_of_line-3F)
#     (bol?)
# *   [`eos?`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-eos-3F)
# *   [`rest?`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-rest-3F)
# *   [`rest_size`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-rest_size)
# *   [`pos`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-pos)
#
#
# ### Setting Where we Are
#
# *   [`reset`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-reset)
# *   [`terminate`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-terminate)
# *   [`pos=`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-pos-3D)
#
#
# ### Match [`Data`](https://docs.ruby-lang.org/en/2.7.0/Data.html)
#
# *   [`matched`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-matched)
# *   [`matched?`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-matched-3F)
# *   [`matched_size`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-matched_size)
#
#
# :
# *   [`pre_match`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-pre_match)
# *   [`post_match`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-post_match)
#
#
# ### Miscellaneous
#
# *   <<
# *   [`concat`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-concat)
# *   [`string`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-string)
# *   [`string=`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-string-3D)
# *   [`unscan`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-unscan)
#
#
# There are aliases to several of the methods.
class StringScanner < Object
  # Creates a new
  # [`StringScanner`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html)
  # object to scan over the given `string`.
  #
  # If `fixed_anchor` is `true`, `\A` always matches the beginning of the
  # string. Otherwise, `\A` always matches the current position.
  #
  # `dup` argument is obsolete and not used now.
  sig do
    params(
        arg0: String,
        arg1: T::Boolean,
    )
    .returns(StringScanner)
  end
  def self.new(arg0, arg1=T.unsafe(nil)); end

  # Appends `str` to the string being scanned. This method does not affect scan
  # pointer.
  #
  # ```ruby
  # s = StringScanner.new("Fri Dec 12 1975 14:39")
  # s.scan(/Fri /)
  # s << " +1000 GMT"
  # s.string            # -> "Fri Dec 12 1975 14:39 +1000 GMT"
  # s.scan(/Dec/)       # -> "Dec"
  # ```
  def <<(_); end

  # Returns the n-th subgroup in the most recent match.
  #
  # ```ruby
  # s = StringScanner.new("Fri Dec 12 1975 14:39")
  # s.scan(/(\w+) (\w+) (\d+) /)       # -> "Fri Dec 12 "
  # s[0]                               # -> "Fri Dec 12 "
  # s[1]                               # -> "Fri"
  # s[2]                               # -> "Dec"
  # s[3]                               # -> "12"
  # s.post_match                       # -> "1975 14:39"
  # s.pre_match                        # -> ""
  #
  # s.reset
  # s.scan(/(?<wday>\w+) (?<month>\w+) (?<day>\d+) /)       # -> "Fri Dec 12 "
  # s[0]                               # -> "Fri Dec 12 "
  # s[1]                               # -> "Fri"
  # s[2]                               # -> "Dec"
  # s[3]                               # -> "12"
  # s[:wday]                           # -> "Fri"
  # s[:month]                          # -> "Dec"
  # s[:day]                            # -> "12"
  # s.post_match                       # -> "1975 14:39"
  # s.pre_match                        # -> ""
  # ```
  def [](_); end

  # Returns `true` iff the scan pointer is at the beginning of the line.
  #
  # ```ruby
  # s = StringScanner.new("test\ntest\n")
  # s.bol?           # => true
  # s.scan(/te/)
  # s.bol?           # => false
  # s.scan(/st\n/)
  # s.bol?           # => true
  # s.terminate
  # s.bol?           # => true
  # ```
  def beginning_of_line?; end

  # Returns the subgroups in the most recent match (not including the full
  # match). If nothing was priorly matched, it returns nil.
  #
  # ```ruby
  # s = StringScanner.new("Fri Dec 12 1975 14:39")
  # s.scan(/(\w+) (\w+) (\d+) /)       # -> "Fri Dec 12 "
  # s.captures                         # -> ["Fri", "Dec", "12"]
  # s.scan(/(\w+) (\w+) (\d+) /)       # -> nil
  # s.captures                         # -> nil
  # ```
  def captures; end

  # Returns the character position of the scan pointer. In the 'reset' position,
  # this value is zero. In the 'terminated' position (i.e. the string is
  # exhausted), this value is the size of the string.
  #
  # In short, it's a 0-based index into the string.
  #
  # ```ruby
  # s = StringScanner.new("abcädeföghi")
  # s.charpos           # -> 0
  # s.scan_until(/ä/)   # -> "abcä"
  # s.pos               # -> 5
  # s.charpos           # -> 4
  # ```
  def charpos; end

  # This returns the value that
  # [`scan`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-scan)
  # would return, without advancing the scan pointer. The match register is
  # affected, though.
  #
  # ```ruby
  # s = StringScanner.new("Fri Dec 12 1975 14:39")
  # s.check /Fri/               # -> "Fri"
  # s.pos                       # -> 0
  # s.matched                   # -> "Fri"
  # s.check /12/                # -> nil
  # s.matched                   # -> nil
  # ```
  #
  # Mnemonic: it "checks" to see whether a
  # [`scan`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-scan)
  # will return a value.
  def check(_); end

  # This returns the value that
  # [`scan_until`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-scan_until)
  # would return, without advancing the scan pointer. The match register is
  # affected, though.
  #
  # ```ruby
  # s = StringScanner.new("Fri Dec 12 1975 14:39")
  # s.check_until /12/          # -> "Fri Dec 12"
  # s.pos                       # -> 0
  # s.matched                   # -> 12
  # ```
  #
  # Mnemonic: it "checks" to see whether a
  # [`scan_until`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-scan_until)
  # will return a value.
  def check_until(_); end

  # Equivalent to
  # [`terminate`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-terminate).
  # This method is obsolete; use
  # [`terminate`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-terminate)
  # instead.
  def clear; end

  # Appends `str` to the string being scanned. This method does not affect scan
  # pointer.
  #
  # ```ruby
  # s = StringScanner.new("Fri Dec 12 1975 14:39")
  # s.scan(/Fri /)
  # s << " +1000 GMT"
  # s.string            # -> "Fri Dec 12 1975 14:39 +1000 GMT"
  # s.scan(/Dec/)       # -> "Dec"
  # ```
  def concat(_); end

  # Equivalent to
  # [`eos?`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-eos-3F).
  # This method is obsolete, use
  # [`eos?`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-eos-3F)
  # instead.
  def empty?; end

  # Looks *ahead* to see if the `pattern` exists *anywhere* in the string,
  # without advancing the scan pointer. This predicates whether a
  # [`scan_until`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-scan_until)
  # will return a value.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # s.exist? /s/            # -> 3
  # s.scan /test/           # -> "test"
  # s.exist? /s/            # -> 2
  # s.exist? /e/            # -> nil
  # ```
  def exist?(_); end

  # Scans one byte and returns it. This method is not multibyte character
  # sensitive. See also:
  # [`getch`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-getch).
  #
  # ```ruby
  # s = StringScanner.new('ab')
  # s.get_byte         # => "a"
  # s.get_byte         # => "b"
  # s.get_byte         # => nil
  #
  # $KCODE = 'EUC'
  # s = StringScanner.new("\244\242")
  # s.get_byte         # => "\244"
  # s.get_byte         # => "\242"
  # s.get_byte         # => nil
  # ```
  def get_byte; end

  # Equivalent to
  # [`get_byte`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-get_byte).
  # This method is obsolete; use
  # [`get_byte`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-get_byte)
  # instead.
  def getbyte; end

  # Returns `true` if the scan pointer is at the end of the string.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # p s.eos?          # => false
  # s.scan(/test/)
  # p s.eos?          # => false
  # s.terminate
  # p s.eos?          # => true
  # ```
  sig {returns(T::Boolean)}
  def eos?(); end

  # Scans one character and returns it. This method is multibyte character
  # sensitive.
  #
  # ```ruby
  # s = StringScanner.new("ab")
  # s.getch           # => "a"
  # s.getch           # => "b"
  # s.getch           # => nil
  #
  # $KCODE = 'EUC'
  # s = StringScanner.new("\244\242")
  # s.getch           # => "\244\242"   # Japanese hira-kana "A" in EUC-JP
  # s.getch           # => nil
  # ```
  sig {returns(T.nilable(String))}
  def getch(); end

  # Returns a string that represents the
  # [`StringScanner`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html)
  # object, showing:
  # *   the current position
  # *   the size of the string
  # *   the characters surrounding the scan pointer
  #
  #     s =
  #     [`StringScanner.new`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-c-new)("Fri
  #     Dec 12 1975 14:39") s.inspect            # -> '#<StringScanner 0/21 @
  #     "Fri D...">' s.scan\_until /12/    # -> "Fri Dec 12" s.inspect
  #     # -> '#<StringScanner 10/21 "...ec 12" @ " 1975...">'
  def inspect; end

  # Tests whether the given `pattern` is matched from the current scan pointer.
  # Returns the length of the match, or `nil`. The scan pointer is not advanced.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # p s.match?(/\w+/)   # -> 4
  # p s.match?(/\w+/)   # -> 4
  # p s.match?("test")  # -> 4
  # p s.match?(/\s+/)   # -> nil
  # ```
  def match?(_); end

  # Returns the last matched string.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # s.match?(/\w+/)     # -> 4
  # s.matched           # -> "test"
  # ```
  def matched; end

  # Returns `true` iff the last match was successful.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # s.match?(/\w+/)     # => 4
  # s.matched?          # => true
  # s.match?(/\d+/)     # => nil
  # s.matched?          # => false
  # ```
  def matched?; end

  # Returns the size of the most recent match (see
  # [`matched`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-matched)),
  # or `nil` if there was no recent match.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # s.check /\w+/           # -> "test"
  # s.matched_size          # -> 4
  # s.check /\d+/           # -> nil
  # s.matched_size          # -> nil
  # ```
  def matched_size; end

  # Extracts a string corresponding to `string[pos,len]`, without advancing the
  # scan pointer.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # s.peek(7)          # => "test st"
  # s.peek(7)          # => "test st"
  # ```
  def peek(_); end

  # Equivalent to
  # [`peek`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-peek).
  # This method is obsolete; use
  # [`peek`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-peek)
  # instead.
  def peep(_); end

  # Returns the byte position of the scan pointer. In the 'reset' position, this
  # value is zero. In the 'terminated' position (i.e. the string is exhausted),
  # this value is the bytesize of the string.
  #
  # In short, it's a 0-based index into bytes of the string.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # s.pos               # -> 0
  # s.scan_until /str/  # -> "test str"
  # s.pos               # -> 8
  # s.terminate         # -> #<StringScanner fin>
  # s.pos               # -> 11
  # ```
  def pointer; end

  # Sets the byte position of the scan pointer.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # s.pos = 7            # -> 7
  # s.rest               # -> "ring"
  # ```
  def pointer=(_); end

  # Returns the byte position of the scan pointer. In the 'reset' position, this
  # value is zero. In the 'terminated' position (i.e. the string is exhausted),
  # this value is the bytesize of the string.
  #
  # In short, it's a 0-based index into bytes of the string.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # s.pos               # -> 0
  # s.scan_until /str/  # -> "test str"
  # s.pos               # -> 8
  # s.terminate         # -> #<StringScanner fin>
  # s.pos               # -> 11
  # ```
  def pos; end

  # Sets the byte position of the scan pointer.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # s.pos = 7            # -> 7
  # s.rest               # -> "ring"
  # ```
  def pos=(_); end

  # Returns the ***post**-match* (in the regular expression sense) of the last
  # scan.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # s.scan(/\w+/)           # -> "test"
  # s.scan(/\s+/)           # -> " "
  # s.pre_match             # -> "test"
  # s.post_match            # -> "string"
  # ```
  def post_match; end

  # Returns the ***pre**-match* (in the regular expression sense) of the last
  # scan.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # s.scan(/\w+/)           # -> "test"
  # s.scan(/\s+/)           # -> " "
  # s.pre_match             # -> "test"
  # s.post_match            # -> "string"
  # ```
  def pre_match; end

  # Reset the scan pointer (index 0) and clear matching data.
  def reset; end

  # Returns the "rest" of the string (i.e. everything after the scan pointer).
  # If there is no more data (eos? = true), it returns `""`.
  def rest; end

  # Returns true iff there is more data in the string. See
  # [`eos?`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-eos-3F).
  # This method is obsolete; use
  # [`eos?`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-eos-3F)
  # instead.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # s.eos?              # These two
  # s.rest?             # are opposites.
  # ```
  def rest?; end

  # `s.rest_size` is equivalent to `s.rest.size`.
  def rest_size; end

  # `s.restsize` is equivalent to `s.rest_size`. This method is obsolete; use
  # [`rest_size`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-rest_size)
  # instead.
  def restsize; end

  # Tries to match with `pattern` at the current position. If there's a match,
  # the scanner advances the "scan pointer" and returns the matched string.
  # Otherwise, the scanner returns `nil`.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # p s.scan(/\w+/)   # -> "test"
  # p s.scan(/\w+/)   # -> nil
  # p s.scan(/\s+/)   # -> " "
  # p s.scan("str")   # -> "str"
  # p s.scan(/\w+/)   # -> "ing"
  # p s.scan(/./)     # -> nil
  # ```
  sig {params(pattern: T.any(Regexp, String)).returns(T.nilable(String))}
  def scan(pattern); end

  # Tests whether the given `pattern` is matched from the current scan pointer.
  # Advances the scan pointer if `advance_pointer_p` is true. Returns the
  # matched string if `return_string_p` is true. The match register is affected.
  #
  # "full" means "#scan with full parameters".
  def scan_full(_, _, _); end

  # Scans the string *until* the `pattern` is matched. Returns the substring up
  # to and including the end of the match, advancing the scan pointer to that
  # location. If there is no match, `nil` is returned.
  #
  # ```ruby
  # s = StringScanner.new("Fri Dec 12 1975 14:39")
  # s.scan_until(/1/)        # -> "Fri Dec 1"
  # s.pre_match              # -> "Fri Dec "
  # s.scan_until(/XYZ/)      # -> nil
  # ```
  def scan_until(_); end

  # Scans the string *until* the `pattern` is matched. Advances the scan pointer
  # if `advance_pointer_p`, otherwise not. Returns the matched string if
  # `return_string_p` is true, otherwise returns the number of bytes advanced.
  # This method does affect the match register.
  def search_full(_, _, _); end

  # Returns the amount of subgroups in the most recent match. The full match
  # counts as a subgroup.
  #
  # ```ruby
  # s = StringScanner.new("Fri Dec 12 1975 14:39")
  # s.scan(/(\w+) (\w+) (\d+) /)       # -> "Fri Dec 12 "
  # s.size                             # -> 4
  # ```
  def size; end

  # Attempts to skip over the given `pattern` beginning with the scan pointer.
  # If it matches, the scan pointer is advanced to the end of the match, and the
  # length of the match is returned. Otherwise, `nil` is returned.
  #
  # It's similar to
  # [`scan`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-scan),
  # but without returning the matched string.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # p s.skip(/\w+/)   # -> 4
  # p s.skip(/\w+/)   # -> nil
  # p s.skip(/\s+/)   # -> 1
  # p s.skip("st")    # -> 2
  # p s.skip(/\w+/)   # -> 4
  # p s.skip(/./)     # -> nil
  # ```
  def skip(_); end

  # Advances the scan pointer until `pattern` is matched and consumed. Returns
  # the number of bytes advanced, or `nil` if no match was found.
  #
  # Look ahead to match `pattern`, and advance the scan pointer to the *end* of
  # the match. Return the number of characters advanced, or `nil` if the match
  # was unsuccessful.
  #
  # It's similar to
  # [`scan_until`](https://docs.ruby-lang.org/en/2.7.0/StringScanner.html#method-i-scan_until),
  # but without returning the intervening string.
  #
  # ```ruby
  # s = StringScanner.new("Fri Dec 12 1975 14:39")
  # s.skip_until /12/           # -> 10
  # s                           #
  # ```
  def skip_until(_); end

  # Returns the string being scanned.
  def string; end

  # Changes the string being scanned to `str` and resets the scanner. Returns
  # `str`.
  def string=(_); end

  # Sets the scan pointer to the end of the string and clear matching data.
  def terminate; end

  # Sets the scan pointer to the previous position. Only one previous position
  # is remembered, and it changes with each scanning operation.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # s.scan(/\w+/)        # => "test"
  # s.unscan
  # s.scan(/../)         # => "te"
  # s.scan(/\d/)         # => nil
  # s.unscan             # ScanError: unscan failed: previous match record not exist
  # ```
  def unscan; end

  # Returns the subgroups in the most recent match at the given indices. If
  # nothing was priorly matched, it returns nil.
  #
  # ```ruby
  # s = StringScanner.new("Fri Dec 12 1975 14:39")
  # s.scan(/(\w+) (\w+) (\d+) /)       # -> "Fri Dec 12 "
  # s.values_at 0, -1, 5, 2            # -> ["Fri Dec 12 ", "12", nil, "Dec"]
  # s.scan(/(\w+) (\w+) (\d+) /)       # -> nil
  # s.values_at 0, -1, 5, 2            # -> nil
  # ```
  def values_at(*_); end

  # This method is defined for backward compatibility.
  def self.must_C_version; end
end

class StringScanner::Error < StandardError
end
