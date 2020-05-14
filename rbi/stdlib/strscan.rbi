# typed: __STDLIB_INTERNAL
# [`StringScanner`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html)
# provides for lexical scanning operations on a
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html). Here is an
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
# [`scan`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-scan)
# for a pattern (a regular expression), the match must occur at the character
# after the scan pointer. If you use
# [`scan_until`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-scan_until),
# then the match can occur anywhere after the scan pointer. In both cases, the
# scan pointer moves *just beyond* the last character of the match, ready to
# scan again from the next character onwards. This is demonstrated by the
# example above.
#
# ## [`Method`](https://docs.ruby-lang.org/en/2.6.0/Method.html) Categories
#
# There are other methods besides the plain scanners. You can look ahead in the
# string without actually scanning. You can access the most recent match. You
# can modify the string being scanned, reset or terminate the scanner, find out
# or change the position of the scan pointer, skip ahead, and so on.
#
# ### Advancing the Scan Pointer
#
# *   [`getch`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-getch)
# *   [`get_byte`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-get_byte)
# *   [`scan`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-scan)
# *   [`scan_until`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-scan_until)
# *   [`skip`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-skip)
# *   [`skip_until`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-skip_until)
#
#
# ### Looking Ahead
#
# *   [`check`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-check)
# *   [`check_until`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-check_until)
# *   [`exist?`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-exist-3F)
# *   [`match?`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-match-3F)
# *   [`peek`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-peek)
#
#
# ### Finding Where we Are
#
# *   [`beginning_of_line?`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-beginning_of_line-3F)
#     (#bol?)
# *   [`eos?`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-eos-3F)
# *   [`rest?`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-rest-3F)
# *   [`rest_size`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-rest_size)
# *   [`pos`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-pos)
#
#
# ### Setting Where we Are
#
# *   [`reset`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-reset)
# *   [`terminate`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-terminate)
# *   [`pos=`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-pos-3D)
#
#
# ### Match [`Data`](https://docs.ruby-lang.org/en/2.6.0/Data.html)
#
# *   [`matched`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-matched)
# *   [`matched?`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-matched-3F)
# *   [`matched_size`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-matched_size)
#
#
# :
# *   [`pre_match`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-pre_match)
# *   [`post_match`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-post_match)
#
#
# ### Miscellaneous
#
# *   <<
# *   [`concat`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-concat)
# *   [`string`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-string)
# *   [`string=`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-string-3D)
# *   [`unscan`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html#method-i-unscan)
#
#
# There are aliases to several of the methods.
class StringScanner < Object
  # Creates a new
  # [`StringScanner`](https://docs.ruby-lang.org/en/2.6.0/StringScanner.html)
  # object to scan over the given `string`. `dup` argument is obsolete and not
  # used now.
  sig do
    params(
        arg0: String,
        arg1: T::Boolean,
    )
    .returns(StringScanner)
  end
  def self.new(arg0, arg1=T.unsafe(nil)); end

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
  sig {returns(String)}
  def getch(); end

  # Tries to match with `pattern` at the current position. If there's a match,
  # the scanner advances the "scan pointer" and returns the matched string.
  # Otherwise, the scanner returns `nil`.
  #
  # ```ruby
  # s = StringScanner.new('test string')
  # p s.scan(/\w+/)   # -> "test"
  # p s.scan(/\w+/)   # -> nil
  # p s.scan(/\s+/)   # -> " "
  # p s.scan(/\w+/)   # -> "string"
  # p s.scan(/./)     # -> nil
  # ```
  sig do
    params(
        arg0: Regexp,
    )
    .returns(String)
  end
  def scan(arg0); end
end

class StringScanner::Error < StandardError
end
