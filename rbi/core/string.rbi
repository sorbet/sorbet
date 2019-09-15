# typed: __STDLIB_INTERNAL

# A `String` object holds and manipulates an arbitrary sequence of bytes,
# typically representing characters.
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) objects may be
# created using `String::new` or as literals.
#
# Because of aliasing issues, users of strings should be aware of the methods
# that modify the contents of a `String` object. Typically, methods with names
# ending in "!" modify their receiver, while those without a "!" return a new
# `String`. However, there are exceptions, such as `String#[]=`.
class String < Object
  include Comparable

  # Format---Uses *str* as a format specification, and returns the result of
  # applying it to *arg*. If the format specification contains more than one
  # substitution, then *arg* must be an `Array` or `Hash` containing the values
  # to be substituted. See `Kernel::sprintf` for details of the format string.
  #
  # ```ruby
  # "%05d" % 123                              #=> "00123"
  # "%-5s: %016x" % [ "ID", self.object_id ]  #=> "ID   : 00002b054ec93168"
  # "foo = %{foo}" % { :foo => 'bar' }        #=> "foo = bar"
  # ```
  sig do
    params(
        arg0: Object,
    )
    .returns(String)
  end
  def %(arg0); end

  # Copy --- Returns a new
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) containing
  # `integer` copies of the receiver. `integer` must be greater than or equal to
  # 0.
  #
  # ```ruby
  # "Ho! " * 3   #=> "Ho! Ho! Ho! "
  # "Ho! " * 0   #=> ""
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(String)
  end
  def *(arg0); end

  # Concatenation---Returns a new `String` containing *other\_str* concatenated
  # to *str*.
  #
  # ```ruby
  # "Hello from " + self.to_s   #=> "Hello from main"
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def +(arg0); end

  # Appends the given object to *str*. If the object is an `Integer`, it is
  # considered a codepoint and converted to a character before being appended.
  #
  # ```ruby
  # a = "hello "
  # a << "world"   #=> "hello world"
  # a << 33        #=> "hello world!"
  # ```
  #
  # See also
  # [`String#concat`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-concat),
  # which takes multiple arguments.
  sig do
    params(
        arg0: Object,
    )
    .returns(String)
  end
  def <<(arg0); end

  # Comparison---Returns -1, 0, +1, or `nil` depending on whether `string` is
  # less than, equal to, or greater than `other_string`.
  #
  # `nil` is returned if the two values are incomparable.
  #
  # If the strings are of different lengths, and the strings are equal when
  # compared up to the shortest length, then the longer string is considered
  # greater than the shorter one.
  #
  # `<=>` is the basis for the methods `<`, `<=`, `>`, `>=`, and `between?`,
  # included from module
  # [`Comparable`](https://docs.ruby-lang.org/en/2.6.0/Comparable.html). The
  # method String#== does not use Comparable#==.
  #
  # ```ruby
  # "abcdef" <=> "abcde"     #=> 1
  # "abcdef" <=> "abcdef"    #=> 0
  # "abcdef" <=> "abcdefg"   #=> -1
  # "abcdef" <=> "ABCDEF"    #=> 1
  # "abcdef" <=> 1           #=> nil
  # ```
  sig do
    params(
        other: String,
    )
    .returns(T.nilable(Integer))
  end
  def <=>(other); end

  # Equality---Returns whether `str` == `obj`, similar to Object#==.
  #
  # If `obj` is not an instance of
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) but responds to
  # `to_str`, then the two strings are compared using `obj.==`.
  #
  # Otherwise, returns similarly to
  # [`String#eql?`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-eql-3F),
  # comparing length and content.
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  # Equality---Returns whether `str` == `obj`, similar to Object#==.
  #
  # If `obj` is not an instance of
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) but responds to
  # `to_str`, then the two strings are compared using `obj.==`.
  #
  # Otherwise, returns similarly to
  # [`String#eql?`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-eql-3F),
  # comparing length and content.
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(arg0); end

  # Match---If *obj* is a `Regexp`, use it as a pattern to match against
  # *str*,and returns the position the match starts, or `nil` if there is no
  # match. Otherwise, invokes *obj.=~*, passing *str* as an argument. The
  # default `=~` in `Object` returns `nil`.
  #
  # Note: `str =~ regexp` is not the same as `regexp =~ str`. Strings captured
  # from named capture groups are assigned to local variables only in the second
  # case.
  #
  # ```ruby
  # "cat o' 9 tails" =~ /\d/   #=> 7
  # "cat o' 9 tails" =~ 9      #=> nil
  # ```
  sig do
    params(
        arg0: Object,
    )
    .returns(T.nilable(Integer))
  end
  def =~(arg0); end

  # Element Reference --- If passed a single `index`, returns a substring of one
  # character at that index. If passed a `start` index and a `length`, returns a
  # substring containing `length` characters starting at the `start` index. If
  # passed a `range`, its beginning and end are interpreted as offsets
  # delimiting the substring to be returned.
  #
  # In these three cases, if an index is negative, it is counted from the end of
  # the string. For the `start` and `range` cases the starting index is just
  # before a character and an index matching the string's size. Additionally, an
  # empty string is returned when the starting index for a character range is at
  # the end of the string.
  #
  # Returns `nil` if the initial index falls outside the string or the length is
  # negative.
  #
  # If a `Regexp` is supplied, the matching portion of the string is returned.
  # If a `capture` follows the regular expression, which may be a capture group
  # index or name, follows the regular expression that component of the
  # [`MatchData`](https://docs.ruby-lang.org/en/2.6.0/MatchData.html) is
  # returned instead.
  #
  # If a `match_str` is given, that string is returned if it occurs in the
  # string.
  #
  # Returns `nil` if the regular expression does not match or the match string
  # cannot be found.
  #
  # ```ruby
  # a = "hello there"
  #
  # a[1]                   #=> "e"
  # a[2, 3]                #=> "llo"
  # a[2..3]                #=> "ll"
  #
  # a[-3, 2]               #=> "er"
  # a[7..-2]               #=> "her"
  # a[-4..-2]              #=> "her"
  # a[-2..-4]              #=> ""
  #
  # a[11, 0]               #=> ""
  # a[11]                  #=> nil
  # a[12, 0]               #=> nil
  # a[12..-1]              #=> nil
  #
  # a[/[aeiou](.)\1/]      #=> "ell"
  # a[/[aeiou](.)\1/, 0]   #=> "ell"
  # a[/[aeiou](.)\1/, 1]   #=> "l"
  # a[/[aeiou](.)\1/, 2]   #=> nil
  #
  # a[/(?<vowel>[aeiou])(?<non_vowel>[^aeiou])/, "non_vowel"] #=> "l"
  # a[/(?<vowel>[aeiou])(?<non_vowel>[^aeiou])/, "vowel"]     #=> "e"
  #
  # a["lo"]                #=> "lo"
  # a["bye"]               #=> nil
  # ```
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T.any(T::Range[Integer], Regexp),
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: String,
    )
    .returns(T.nilable(String))
  end
  def [](arg0, arg1=T.unsafe(nil)); end

  # Returns true for a string which has only ASCII characters.
  #
  # ```ruby
  # "abc".force_encoding("UTF-8").ascii_only?          #=> true
  # "abc\u{6666}".force_encoding("UTF-8").ascii_only?  #=> false
  # ```
  sig {returns(T::Boolean)}
  def ascii_only?(); end

  # Returns a copied string whose encoding is ASCII-8BIT.
  sig {returns(String)}
  def b(); end

  # Returns an array of bytes in *str*. This is a shorthand for
  # `str.each_byte.to_a`.
  #
  # If a block is given, which is a deprecated form, works the same as
  # `each_byte`.
  sig {returns(Array)}
  def bytes(); end

  # Returns the length of `str` in bytes.
  #
  # ```ruby
  # "\x80\u3042".bytesize  #=> 4
  # "hello".bytesize       #=> 5
  # ```
  sig {returns(Integer)}
  def bytesize(); end

  # Byte Reference---If passed a single `Integer`, returns a substring of one
  # byte at that position. If passed two `Integer` objects, returns a substring
  # starting at the offset given by the first, and a length given by the second.
  # If given a `Range`, a substring containing bytes at offsets given by the
  # range is returned. In all three cases, if an offset is negative, it is
  # counted from the end of *str*. Returns `nil` if the initial offset falls
  # outside the string, the length is negative, or the beginning of the range is
  # greater than the end. The encoding of the resulted string keeps original
  # encoding.
  #
  # ```ruby
  # "hello".byteslice(1)     #=> "e"
  # "hello".byteslice(-1)    #=> "o"
  # "hello".byteslice(1, 2)  #=> "el"
  # "\x80\u3042".byteslice(1, 3) #=> "\u3042"
  # "\x03\u3042\xff".byteslice(1..3) #=> "\u3042"
  # ```
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T::Range[Integer],
    )
    .returns(T.nilable(String))
  end
  def byteslice(arg0, arg1=T.unsafe(nil)); end

  # Returns a copy of *str* with the first character converted to uppercase and
  # the remainder to lowercase.
  #
  # See
  # [`String#downcase`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-downcase)
  # for meaning of `options` and use with different encodings.
  #
  # ```ruby
  # "hello".capitalize    #=> "Hello"
  # "HELLO".capitalize    #=> "Hello"
  # "123ABC".capitalize   #=> "123abc"
  # ```
  sig {returns(String)}
  def capitalize(); end

  # Modifies *str* by converting the first character to uppercase and the
  # remainder to lowercase. Returns `nil` if no changes are made. There is an
  # exception for modern Georgian (mkhedruli/MTAVRULI), where the result is the
  # same as for
  # [`String#downcase`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-downcase),
  # to avoid mixed case.
  #
  # See
  # [`String#downcase`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-downcase)
  # for meaning of `options` and use with different encodings.
  #
  # ```ruby
  # a = "hello"
  # a.capitalize!   #=> "Hello"
  # a               #=> "Hello"
  # a.capitalize!   #=> nil
  # ```
  sig {returns(T.nilable(String))}
  def capitalize!(); end

  # Case-insensitive version of `String#<=>`. Currently, case-insensitivity only
  # works on characters A-Z/a-z, not all of Unicode. This is different from
  # [`String#casecmp?`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-casecmp-3F).
  #
  # ```ruby
  # "aBcDeF".casecmp("abcde")     #=> 1
  # "aBcDeF".casecmp("abcdef")    #=> 0
  # "aBcDeF".casecmp("abcdefg")   #=> -1
  # "abcdef".casecmp("ABCDEF")    #=> 0
  # ```
  #
  # `nil` is returned if the two strings have incompatible encodings, or if
  # `other_str` is not a string.
  #
  # ```ruby
  # "foo".casecmp(2)   #=> nil
  # "\u{e4 f6 fc}".encode("ISO-8859-1").casecmp("\u{c4 d6 dc}")   #=> nil
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(T.nilable(Integer))
  end
  def casecmp(arg0); end

  # Centers `str` in `width`. If `width` is greater than the length of `str`,
  # returns a new [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) of
  # length `width` with `str` centered and padded with `padstr`; otherwise,
  # returns `str`.
  #
  # ```ruby
  # "hello".center(4)         #=> "hello"
  # "hello".center(20)        #=> "       hello        "
  # "hello".center(20, '123') #=> "1231231hello12312312"
  # ```
  sig do
    params(
        arg0: Integer,
        arg1: String,
    )
    .returns(String)
  end
  def center(arg0, arg1=T.unsafe(nil)); end

  # Returns an array of characters in *str*. This is a shorthand for
  # `str.each_char.to_a`.
  #
  # If a block is given, which is a deprecated form, works the same as
  # `each_char`.
  sig {returns(Array)}
  def chars(); end

  # Returns a new `String` with the given record separator removed from the end
  # of *str* (if present). If `$/` has not been changed from the default Ruby
  # record separator, then `chomp` also removes carriage return characters (that
  # is it will remove `\n`, `\r`, and `\r\n`). If `$/` is an empty string, it
  # will remove all trailing newlines from the string.
  #
  # ```ruby
  # "hello".chomp                #=> "hello"
  # "hello\n".chomp              #=> "hello"
  # "hello\r\n".chomp            #=> "hello"
  # "hello\n\r".chomp            #=> "hello\n"
  # "hello\r".chomp              #=> "hello"
  # "hello \n there".chomp       #=> "hello \n there"
  # "hello".chomp("llo")         #=> "he"
  # "hello\r\n\r\n".chomp('')    #=> "hello"
  # "hello\r\n\r\r\n".chomp('')  #=> "hello\r\n\r"
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def chomp(arg0=T.unsafe(nil)); end

  # Modifies *str* in place as described for `String#chomp`, returning *str*, or
  # `nil` if no modifications were made.
  sig do
    params(
        arg0: String,
    )
    .returns(T.nilable(String))
  end
  def chomp!(arg0=T.unsafe(nil)); end

  # Returns a new `String` with the last character removed. If the string ends
  # with `\r\n`, both characters are removed. Applying `chop` to an empty string
  # returns an empty string. `String#chomp` is often a safer alternative, as it
  # leaves the string unchanged if it doesn't end in a record separator.
  #
  # ```ruby
  # "string\r\n".chop   #=> "string"
  # "string\n\r".chop   #=> "string\n"
  # "string\n".chop     #=> "string"
  # "string".chop       #=> "strin"
  # "x".chop.chop       #=> ""
  # ```
  sig {returns(String)}
  def chop(); end

  # Processes *str* as for `String#chop`, returning *str*, or `nil` if *str* is
  # the empty string. See also `String#chomp!`.
  sig {returns(T.nilable(String))}
  def chop!(); end

  # Returns a one-character string at the beginning of the string.
  #
  # ```ruby
  # a = "abcde"
  # a.chr    #=> "a"
  # ```
  sig {returns(String)}
  def chr(); end

  # Makes string empty.
  #
  # ```ruby
  # a = "abcde"
  # a.clear    #=> ""
  # ```
  sig {returns(String)}
  def clear(); end

  # Returns an array of the `Integer` ordinals of the characters in *str*. This
  # is a shorthand for `str.each_codepoint.to_a`.
  #
  # If a block is given, which is a deprecated form, works the same as
  # `each_codepoint`.
  sig {returns(T::Array[Integer])}
  sig do
    params(
        blk: BasicObject,
    )
    .returns(T::Array[Integer])
  end
  def codepoints(&blk); end

  # Concatenates the given object(s) to *str*. If an object is an `Integer`, it
  # is considered a codepoint and converted to a character before concatenation.
  #
  # `concat` can take multiple arguments, and all the arguments are concatenated
  # in order.
  #
  # ```ruby
  # a = "hello "
  # a.concat("world", 33)      #=> "hello world!"
  # a                          #=> "hello world!"
  #
  # b = "sn"
  # b.concat("_", b, "_", b)   #=> "sn_sn_sn"
  # ```
  #
  # See also
  # [`String#<<`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-3C-3C),
  # which takes a single argument.
  sig do
    params(
        arg0: T.any(Integer, Object),
    )
    .returns(String)
  end
  def concat(arg0); end

  # Each `other_str` parameter defines a set of characters to count. The
  # intersection of these sets defines the characters to count in `str`. Any
  # `other_str` that starts with a caret `^` is negated. The sequence `c1-c2`
  # means all characters between c1 and c2. The backslash character `\` can be
  # used to escape `^` or `-` and is otherwise ignored unless it appears at the
  # end of a sequence or the end of a `other_str`.
  #
  # ```ruby
  # a = "hello world"
  # a.count "lo"                   #=> 5
  # a.count "lo", "o"              #=> 2
  # a.count "hello", "^l"          #=> 4
  # a.count "ej-m"                 #=> 4
  #
  # "hello^world".count "\\^aeiou" #=> 4
  # "hello-world".count "a\\-eo"   #=> 4
  #
  # c = "hello world\\r\\n"
  # c.count "\\"                   #=> 2
  # c.count "\\A"                  #=> 0
  # c.count "X-\\w"                #=> 3
  # ```
  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(Integer)
  end
  def count(arg0, *arg1); end

  # Returns the string generated by calling `crypt(3)` standard library function
  # with `str` and `salt_str`, in this order, as its arguments. Please do not
  # use this method any longer. It is legacy; provided only for backward
  # compatibility with ruby scripts in earlier days. It is bad to use in
  # contemporary programs for several reasons:
  #
  # ```
  # * Behaviour of C's <code>crypt(3)</code> depends on the OS it is
  #   run.  The generated string lacks data portability.
  #
  # * On some OSes such as Mac OS, <code>crypt(3)</code> never fails
  #   (i.e. silently ends up in unexpected results).
  #
  # * On some OSes such as Mac OS, <code>crypt(3)</code> is not
  #   thread safe.
  #
  # * So-called "traditional" usage of <code>crypt(3)</code> is very
  #   very very weak.  According to its manpage, Linux's traditional
  #   <code>crypt(3)</code> output has only 2**56 variations; too
  #   easy to brute force today.  And this is the default behaviour.
  #
  # * In order to make things robust some OSes implement so-called
  #   "modular" usage. To go through, you have to do a complex
  #   build-up of the <code>salt_str</code> parameter, by hand.
  #   Failure in generation of a proper salt string tends not to
  #   yield any errors; typos in parameters are normally not
  #   detectable.
  #
  #     * For instance, in the following example, the second invocation
  #       of <code>String#crypt</code> is wrong; it has a typo in
  #       "round=" (lacks "s").  However the call does not fail and
  #       something unexpected is generated.
  #
  #          "foo".crypt("$5$rounds=1000$salt$") # OK, proper usage
  #          "foo".crypt("$5$round=1000$salt$")  # Typo not detected
  #
  # * Even in the "modular" mode, some hash functions are considered
  #   archaic and no longer recommended at all; for instance module
  #   <code>$1$</code> is officially abandoned by its author: see
  #   http://phk.freebsd.dk/sagas/md5crypt_eol.html .  For another
  #   instance module <code>$3$</code> is considered completely
  #   broken: see the manpage of FreeBSD.
  #
  # * On some OS such as Mac OS, there is no modular mode. Yet, as
  #   written above, <code>crypt(3)</code> on Mac OS never fails.
  #   This means even if you build up a proper salt string it
  #   generates a traditional DES hash anyways, and there is no way
  #   for you to be aware of.
  #
  #       "foo".crypt("$5$rounds=1000$salt$") # => "$5fNPQMxC5j6."
  # ```
  #
  # If for some reason you cannot migrate to other secure contemporary password
  # hashing algorithms, install the string-crypt gem and `require
  # 'string/crypt'` to continue using it.
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def crypt(arg0); end

  # Returns a copy of *str* with all characters in the intersection of its
  # arguments deleted. Uses the same rules for building the set of characters as
  # `String#count`.
  #
  # ```ruby
  # "hello".delete "l","lo"        #=> "heo"
  # "hello".delete "lo"            #=> "he"
  # "hello".delete "aeiou", "^e"   #=> "hell"
  # "hello".delete "ej-m"          #=> "ho"
  # ```
  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(String)
  end
  def delete(arg0, *arg1); end

  # Performs a `delete` operation in place, returning *str*, or `nil` if *str*
  # was not modified.
  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  def delete!(arg0, *arg1); end

  sig {params(prefix: String).returns(String)}
  def delete_prefix(prefix); end

  sig {params(prefix: String).returns(T.nilable(String))}
  def delete_prefix!(prefix); end

  sig {params(prefix: String).returns(String)}
  def delete_suffix(prefix); end

  sig {params(prefix: String).returns(T.nilable(String))}
  def delete_suffix!(prefix); end

  # Returns a copy of *str* with all uppercase letters replaced with their
  # lowercase counterparts. Which letters exactly are replaced, and by which
  # other letters, depends on the presence or absence of options, and on the
  # `encoding` of the string.
  #
  # The meaning of the `options` is as follows:
  #
  # No option
  # :   Full Unicode case mapping, suitable for most languages (see :turkic and
  #     :lithuanian options below for exceptions). Context-dependent case
  #     mapping as described in Table 3-14 of the Unicode standard is currently
  #     not supported.
  # :ascii
  # :   Only the ASCII region, i.e. the characters "A" to "Z" and "a" to "z",
  #     are affected. This option cannot be combined with any other option.
  # :turkic
  # :   Full Unicode case mapping, adapted for Turkic languages (Turkish,
  #     Azerbaijani, ...). This means that upper case I is mapped to lower case
  #     dotless i, and so on.
  # :lithuanian
  # :   Currently, just full Unicode case mapping. In the future, full Unicode
  #     case mapping adapted for Lithuanian (keeping the dot on the lower case i
  #     even if there is an accent on top).
  # :fold
  # :   Only available on `downcase` and `downcase!`. Unicode case **folding**,
  #     which is more far-reaching than Unicode case mapping. This option
  #     currently cannot be combined with any other option (i.e. there is
  #     currently no variant for turkic languages).
  #
  #
  # Please note that several assumptions that are valid for ASCII-only case
  # conversions do not hold for more general case conversions. For example, the
  # length of the result may not be the same as the length of the input (neither
  # in characters nor in bytes), some roundtrip assumptions (e.g. str.downcase
  # == str.upcase.downcase) may not apply, and Unicode normalization (i.e.
  # [`String#unicode_normalize`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-unicode_normalize))
  # is not necessarily maintained by case mapping operations.
  #
  # Non-ASCII case mapping/folding is currently supported for UTF-8,
  # UTF-16BE/LE, UTF-32BE/LE, and ISO-8859-1~16 Strings/Symbols. This support
  # will be extended to other encodings.
  #
  # ```ruby
  # "hEllO".downcase   #=> "hello"
  # ```
  sig {returns(String)}
  def downcase(); end

  # Downcases the contents of *str*, returning `nil` if no changes were made.
  #
  # See
  # [`String#downcase`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-downcase)
  # for meaning of `options` and use with different encodings.
  sig {returns(T.nilable(String))}
  def downcase!(); end

  # Produces a version of `str` with all non-printing characters replaced by
  # `\nnn` notation and all special characters escaped.
  #
  # ```ruby
  # "hello \n ''".dump  #=> "\"hello \\n ''\""
  # ```
  sig {returns(String)}
  def dump(); end

  # Passes each byte in *str* to the given block, or returns an enumerator if no
  # block is given.
  #
  # ```ruby
  # "hello".each_byte {|c| print c, ' ' }
  # ```
  #
  # *produces:*
  #
  # ```
  # 104 101 108 108 111
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(String)
  end
  sig {returns(T::Enumerator[Integer])}
  def each_byte(&blk); end

  # Passes each character in *str* to the given block, or returns an enumerator
  # if no block is given.
  #
  # ```ruby
  # "hello".each_char {|c| print c, ' ' }
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # h e l l o
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(String)
  end
  sig {returns(T::Enumerator[String])}
  def each_char(&blk); end

  # Passes the `Integer` ordinal of each character in *str*, also known as a
  # *codepoint* when applied to Unicode strings to the given block. For
  # encodings other than UTF-8/UTF-16(BE|LE)/UTF-32(BE|LE), values are directly
  # derived from the binary representation of each character.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # "hello\u0639".each_codepoint {|c| print c, ' ' }
  # ```
  #
  # *produces:*
  #
  # ```
  # 104 101 108 108 111 1593
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(String)
  end
  sig {returns(T::Enumerator[Integer])}
  def each_codepoint(&blk); end

  # Splits *str* using the supplied parameter as the record separator (`$/` by
  # default), passing each substring in turn to the supplied block. If a
  # zero-length record separator is supplied, the string is split into
  # paragraphs delimited by multiple successive newlines.
  #
  # See
  # [`IO.readlines`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-readlines)
  # for details about getline\_args.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # print "Example one\n"
  # "hello\nworld".each_line {|s| p s}
  # print "Example two\n"
  # "hello\nworld".each_line('l') {|s| p s}
  # print "Example three\n"
  # "hello\n\n\nworld".each_line('') {|s| p s}
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # Example one
  # "hello\n"
  # "world"
  # Example two
  # "hel"
  # "l"
  # "o\nworl"
  # "d"
  # Example three
  # "hello\n\n"
  # "world"
  # ```
  sig do
    params(
        arg0: String,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(String)
  end
  sig do
    params(
        arg0: String,
    )
    .returns(T::Enumerator[String])
  end
  def each_line(arg0=T.unsafe(nil), &blk); end

  # Returns `true` if *str* has a length of zero.
  #
  # ```ruby
  # "hello".empty?   #=> false
  # " ".empty?       #=> false
  # "".empty?        #=> true
  # ```
  sig {returns(T::Boolean)}
  def empty?(); end

  # Returns the [`Encoding`](https://docs.ruby-lang.org/en/2.6.0/Encoding.html)
  # object that represents the encoding of obj.
  sig {returns(Encoding)}
  def encoding(); end

  # Returns true if `str` ends with one of the `suffixes` given.
  #
  # ```ruby
  # "hello".end_with?("ello")               #=> true
  #
  # # returns true if one of the +suffixes+ matches.
  # "hello".end_with?("heaven", "ello")     #=> true
  # "hello".end_with?("heaven", "paradise") #=> false
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(T::Boolean)
  end
  def end_with?(*arg0); end

  # Two strings are equal if they have the same length and content.
  sig do
    params(
        arg0: String,
    )
    .returns(T::Boolean)
  end
  def eql?(arg0); end

  # Changes the encoding to `encoding` and returns self.
  sig do
    params(
        arg0: T.any(String, Encoding),
    )
    .returns(String)
  end
  def force_encoding(arg0); end

  # returns the *index*th byte as an integer.
  sig do
    params(
        arg0: Integer,
    )
    .returns(T.nilable(Integer))
  end
  def getbyte(arg0); end

  # Returns a copy of *str* with *all* occurrences of *pattern* substituted for
  # the second argument. The *pattern* is typically a `Regexp`; if given as a
  # `String`, any regular expression metacharacters it contains will be
  # interpreted literally, e.g. `'\\\d'` will match a backslash followed by 'd',
  # instead of a digit.
  #
  # If *replacement* is a `String` it will be substituted for the matched text.
  # It may contain back-references to the pattern's capture groups of the form
  # `\\\d`, where *d* is a group number, or `\\\k<n>`, where *n* is a group
  # name. If it is a double-quoted string, both back-references must be preceded
  # by an additional backslash. However, within *replacement* the special match
  # variables, such as `$&`, will not refer to the current match.
  #
  # If the second argument is a `Hash`, and the matched text is one of its keys,
  # the corresponding value is the replacement string.
  #
  # In the block form, the current match string is passed in as a parameter, and
  # variables such as `$1`, `$2`, `$``, `$&`, and `$'` will be set
  # appropriately. The value returned by the block will be substituted for the
  # match on each call.
  #
  # The result inherits any tainting in the original string or any supplied
  # replacement string.
  #
  # When neither a block nor a second argument is supplied, an `Enumerator` is
  # returned.
  #
  # ```ruby
  # "hello".gsub(/[aeiou]/, '*')                  #=> "h*ll*"
  # "hello".gsub(/([aeiou])/, '<\1>')             #=> "h<e>ll<o>"
  # "hello".gsub(/./) {|s| s.ord.to_s + ' '}      #=> "104 101 108 108 111 "
  # "hello".gsub(/(?<foo>[aeiou])/, '{\k<foo>}')  #=> "h{e}ll{o}"
  # 'hello'.gsub(/[eo]/, 'e' => 3, 'o' => '*')    #=> "h3ll*"
  # ```
  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: String,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: Hash,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns(T::Enumerator[String])
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns(String)
  end
  def gsub(arg0, arg1=T.unsafe(nil), &blk); end

  # Performs the substitutions of `String#gsub` in place, returning *str*, or
  # `nil` if no substitutions were performed. If no block and no *replacement*
  # is given, an enumerator is returned instead.
  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns(T::Enumerator[String])
  end
  def gsub!(arg0, arg1=T.unsafe(nil), &blk); end

  # Returns a hash based on the string's length, content and encoding.
  #
  # See also Object#hash.
  sig {returns(Integer)}
  def hash(); end

  # Treats leading characters from *str* as a string of hexadecimal digits (with
  # an optional sign and an optional `0x`) and returns the corresponding number.
  # Zero is returned on error.
  #
  # ```ruby
  # "0x0a".hex     #=> 10
  # "-1234".hex    #=> -4660
  # "0".hex        #=> 0
  # "wombat".hex   #=> 0
  # ```
  sig {returns(Integer)}
  def hex(); end

  # Returns `true` if *str* contains the given string or character.
  #
  # ```ruby
  # "hello".include? "lo"   #=> true
  # "hello".include? "ol"   #=> false
  # "hello".include? ?h     #=> true
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(T::Boolean)
  end
  def include?(arg0); end

  # Returns the index of the first occurrence of the given *substring* or
  # pattern (*regexp*) in *str*. Returns `nil` if not found. If the second
  # parameter is present, it specifies the position in the string to begin the
  # search.
  #
  # ```ruby
  # "hello".index('e')             #=> 1
  # "hello".index('lo')            #=> 3
  # "hello".index('a')             #=> nil
  # "hello".index(?e)              #=> 1
  # "hello".index(/[aeiou]/, -3)   #=> 4
  # ```
  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: Integer,
    )
    .returns(T.nilable(Integer))
  end
  def index(arg0, arg1=T.unsafe(nil)); end

  sig do
    params(
        str: String,
    )
    .void
  end
  def initialize(str=T.unsafe(nil)); end

  # Inserts *other\_str* before the character at the given *index*, modifying
  # *str*. Negative indices count from the end of the string, and insert *after*
  # the given character. The intent is insert *aString* so that it starts at the
  # given *index*.
  #
  # ```ruby
  # "abcd".insert(0, 'X')    #=> "Xabcd"
  # "abcd".insert(3, 'X')    #=> "abcXd"
  # "abcd".insert(4, 'X')    #=> "abcdX"
  # "abcd".insert(-3, 'X')   #=> "abXcd"
  # "abcd".insert(-1, 'X')   #=> "abcdX"
  # ```
  sig do
    params(
        arg0: Integer,
        arg1: String,
    )
    .returns(String)
  end
  def insert(arg0, arg1); end

  # Returns a printable version of *str*, surrounded by quote marks, with
  # special characters escaped.
  #
  # ```ruby
  # str = "hello"
  # str[3] = "\b"
  # str.inspect       #=> "\"hel\\bo\""
  # ```
  sig {returns(String)}
  def inspect(); end

  # Returns the `Symbol` corresponding to *str*, creating the symbol if it did
  # not previously exist. See `Symbol#id2name`.
  #
  # ```ruby
  # "Koala".intern         #=> :Koala
  # s = 'cat'.to_sym       #=> :cat
  # s == :cat              #=> true
  # s = '@cat'.to_sym      #=> :@cat
  # s == :@cat             #=> true
  # ```
  #
  # This can also be used to create symbols that cannot be represented using the
  # `:xxx` notation.
  #
  # ```ruby
  # 'cat and dog'.to_sym   #=> :"cat and dog"
  # ```
  sig {returns(Symbol)}
  def intern(); end

  # Returns the character length of *str*.
  sig {returns(Integer)}
  def length(); end

  # Returns an array of lines in *str* split using the supplied record separator
  # (`$/` by default). This is a shorthand for `str.each_line(separator,
  # getline_args).to_a`.
  #
  # See
  # [`IO.readlines`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-c-readlines)
  # for details about getline\_args.
  #
  # ```ruby
  # "hello\nworld\n".lines              #=> ["hello\n", "world\n"]
  # "hello  world".lines(' ')           #=> ["hello ", " ", "world"]
  # "hello\nworld\n".lines(chomp: true) #=> ["hello", "world"]
  # ```
  #
  # If a block is given, which is a deprecated form, works the same as
  # `each_line`.
  sig do
    params(
        arg0: String,
    )
    .returns(T::Array[String])
  end
  def lines(arg0=T.unsafe(nil)); end

  # If *integer* is greater than the length of *str*, returns a new `String` of
  # length *integer* with *str* left justified and padded with *padstr*;
  # otherwise, returns *str*.
  #
  # ```ruby
  # "hello".ljust(4)            #=> "hello"
  # "hello".ljust(20)           #=> "hello               "
  # "hello".ljust(20, '1234')   #=> "hello123412341234123"
  # ```
  sig do
    params(
        arg0: Integer,
        arg1: String,
    )
    .returns(String)
  end
  def ljust(arg0, arg1=T.unsafe(nil)); end

  # Returns a copy of the receiver with leading whitespace removed. See also
  # [`String#rstrip`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-rstrip)
  # and
  # [`String#strip`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-strip).
  #
  # Refer to
  # [`String#strip`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-strip)
  # for the definition of whitespace.
  #
  # ```ruby
  # "  hello  ".lstrip   #=> "hello  "
  # "hello".lstrip       #=> "hello"
  # ```
  sig {returns(String)}
  def lstrip(); end

  # Removes leading whitespace from the receiver. Returns the altered receiver,
  # or `nil` if no change was made. See also
  # [`String#rstrip!`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-rstrip-21)
  # and
  # [`String#strip!`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-strip-21).
  #
  # Refer to
  # [`String#strip`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-strip)
  # for the definition of whitespace.
  #
  # ```ruby
  # "  hello  ".lstrip!  #=> "hello  "
  # "hello  ".lstrip!    #=> nil
  # "hello".lstrip!      #=> nil
  # ```
  sig {returns(T.nilable(String))}
  def lstrip!(); end

  # Converts *pattern* to a `Regexp` (if it isn't already one), then invokes its
  # `match` method on *str*. If the second parameter is present, it specifies
  # the position in the string to begin the search.
  #
  # ```ruby
  # 'hello'.match('(.)\1')      #=> #<MatchData "ll" 1:"l">
  # 'hello'.match('(.)\1')[0]   #=> "ll"
  # 'hello'.match(/(.)\1/)[0]   #=> "ll"
  # 'hello'.match(/(.)\1/, 3)   #=> nil
  # 'hello'.match('xx')         #=> nil
  # ```
  #
  # If a block is given, invoke the block with
  # [`MatchData`](https://docs.ruby-lang.org/en/2.6.0/MatchData.html) if match
  # succeed, so that you can write
  #
  # ```
  # str.match(pat) {|m| ...}
  # ```
  #
  # instead of
  #
  # ```
  # if m = str.match(pat)
  #   ...
  # end
  # ```
  #
  # The return value is a value from block execution in this case.
  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns(T.nilable(MatchData))
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: Integer,
    )
    .returns(T.nilable(MatchData))
  end
  def match(arg0, arg1=T.unsafe(nil)); end

  # Converts *pattern* to a `Regexp` (if it isn't already one), then returns a
  # `true` or `false` indicates whether the regexp is matched *str* or not
  # without updating `$~` and other related variables. If the second parameter
  # is present, it specifies the position in the string to begin the search.
  #
  # ```ruby
  # "Ruby".match?(/R.../)    #=> true
  # "Ruby".match?(/R.../, 1) #=> false
  # "Ruby".match?(/P.../)    #=> false
  # $&                       #=> nil
  # ```
  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: Integer,
    )
    .returns(T::Boolean)
  end
  def match?(arg0, arg1=T.unsafe(nil)); end

  # Returns the successor to *str*. The successor is calculated by incrementing
  # characters starting from the rightmost alphanumeric (or the rightmost
  # character if there are no alphanumerics) in the string. Incrementing a digit
  # always results in another digit, and incrementing a letter results in
  # another letter of the same case. Incrementing nonalphanumerics uses the
  # underlying character set's collating sequence.
  #
  # If the increment generates a "carry," the character to the left of it is
  # incremented. This process repeats until there is no carry, adding an
  # additional character if necessary.
  #
  # ```ruby
  # "abcd".succ        #=> "abce"
  # "THX1138".succ     #=> "THX1139"
  # "<<koala>>".succ   #=> "<<koalb>>"
  # "1999zzz".succ     #=> "2000aaa"
  # "ZZZ9999".succ     #=> "AAAA0000"
  # "***".succ         #=> "**+"
  # ```
  sig {returns(String)}
  def next(); end

  # Equivalent to `String#succ`, but modifies the receiver in place.
  sig {returns(String)}
  def next!(); end

  # Treats leading characters of *str* as a string of octal digits (with an
  # optional sign) and returns the corresponding number. Returns 0 if the
  # conversion fails.
  #
  # ```ruby
  # "123".oct       #=> 83
  # "-377".oct      #=> -255
  # "bad".oct       #=> 0
  # "0377bad".oct   #=> 255
  # ```
  #
  # If `str` starts with `0`, radix indicators are honored. See Kernel#Integer.
  sig {returns(Integer)}
  def oct(); end

  # Returns the `Integer` ordinal of a one-character string.
  #
  # ```ruby
  # "a".ord         #=> 97
  # ```
  sig {returns(Integer)}
  def ord(); end

  # Searches *sep* or pattern (*regexp*) in the string and returns the part
  # before it, the match, and the part after it. If it is not found, returns two
  # empty strings and *str*.
  #
  # ```ruby
  # "hello".partition("l")         #=> ["he", "l", "lo"]
  # "hello".partition("x")         #=> ["hello", "", ""]
  # "hello".partition(/.l/)        #=> ["h", "el", "lo"]
  # ```
  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns([String, String, String])
  end
  def partition(arg0); end

  # Prepend---Prepend the given strings to *str*.
  #
  # ```ruby
  # a = "!"
  # a.prepend("hello ", "world") #=> "hello world!"
  # a                            #=> "hello world!"
  # ```
  #
  # See also
  # [`String#concat`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-concat).
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def prepend(arg0); end

  # Replaces the contents and taintedness of *str* with the corresponding values
  # in *other\_str*.
  #
  # ```ruby
  # s = "hello"         #=> "hello"
  # s.replace "world"   #=> "world"
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def replace(arg0); end

  # Returns a new string with the characters from *str* in reverse order.
  #
  # ```ruby
  # "stressed".reverse   #=> "desserts"
  # ```
  sig {returns(String)}
  def reverse(); end

  # Returns the index of the last occurrence of the given *substring* or pattern
  # (*regexp*) in *str*. Returns `nil` if not found. If the second parameter is
  # present, it specifies the position in the string to end the
  # search---characters beyond this point will not be considered.
  #
  # ```ruby
  # "hello".rindex('e')             #=> 1
  # "hello".rindex('l')             #=> 3
  # "hello".rindex('a')             #=> nil
  # "hello".rindex(?e)              #=> 1
  # "hello".rindex(/[aeiou]/, -2)   #=> 1
  # ```
  sig do
    params(
        arg0: T.any(String, Regexp),
        arg1: Integer,
    )
    .returns(T.nilable(Integer))
  end
  def rindex(arg0, arg1=T.unsafe(nil)); end

  # If *integer* is greater than the length of *str*, returns a new `String` of
  # length *integer* with *str* right justified and padded with *padstr*;
  # otherwise, returns *str*.
  #
  # ```ruby
  # "hello".rjust(4)            #=> "hello"
  # "hello".rjust(20)           #=> "               hello"
  # "hello".rjust(20, '1234')   #=> "123412341234123hello"
  # ```
  sig do
    params(
        arg0: Integer,
        arg1: String,
    )
    .returns(String)
  end
  def rjust(arg0, arg1=T.unsafe(nil)); end

  # Searches *sep* or pattern (*regexp*) in the string from the end of the
  # string, and returns the part before it, the match, and the part after it. If
  # it is not found, returns two empty strings and *str*.
  #
  # ```ruby
  # "hello".rpartition("l")         #=> ["hel", "l", "o"]
  # "hello".rpartition("x")         #=> ["", "", "hello"]
  # "hello".rpartition(/.l/)        #=> ["he", "ll", "o"]
  # ```
  sig do
    params(
        arg0: T.any(String, Regexp),
    )
    .returns([String, String, String])
  end
  def rpartition(arg0); end

  # Returns a copy of the receiver with trailing whitespace removed. See also
  # [`String#lstrip`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-lstrip)
  # and
  # [`String#strip`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-strip).
  #
  # Refer to
  # [`String#strip`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-strip)
  # for the definition of whitespace.
  #
  # ```ruby
  # "  hello  ".rstrip   #=> "  hello"
  # "hello".rstrip       #=> "hello"
  # ```
  sig {returns(String)}
  def rstrip(); end

  # Removes trailing whitespace from the receiver. Returns the altered receiver,
  # or `nil` if no change was made. See also
  # [`String#lstrip!`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-lstrip-21)
  # and
  # [`String#strip!`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-strip-21).
  #
  # Refer to
  # [`String#strip`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-strip)
  # for the definition of whitespace.
  #
  # ```ruby
  # "  hello  ".rstrip!  #=> "  hello"
  # "  hello".rstrip!    #=> nil
  # "hello".rstrip!      #=> nil
  # ```
  sig {returns(String)}
  def rstrip!(); end

  # Both forms iterate through *str*, matching the pattern (which may be a
  # `Regexp` or a `String`). For each match, a result is generated and either
  # added to the result array or passed to the block. If the pattern contains no
  # groups, each individual result consists of the matched string, `$&`. If the
  # pattern contains groups, each individual result is itself an array
  # containing one entry per group.
  #
  # ```ruby
  # a = "cruel world"
  # a.scan(/\w+/)        #=> ["cruel", "world"]
  # a.scan(/.../)        #=> ["cru", "el ", "wor"]
  # a.scan(/(...)/)      #=> [["cru"], ["el "], ["wor"]]
  # a.scan(/(..)(..)/)   #=> [["cr", "ue"], ["l ", "wo"]]
  # ```
  #
  # And the block form:
  #
  # ```ruby
  # a.scan(/\w+/) {|w| print "<<#{w}>> " }
  # print "\n"
  # a.scan(/(.)(.)/) {|x,y| print y, x }
  # print "\n"
  # ```
  #
  # *produces:*
  #
  # ```
  # <<cruel>> <<world>>
  # rceu lowlr
  # ```
  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns(T::Array[T.any(String, T::Array[String])])
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        blk: BasicObject,
    )
    .returns(T::Array[T.any(String, T::Array[String])])
  end
  def scan(arg0, &blk); end

  # If the string is invalid byte sequence then replace invalid bytes with given
  # replacement character, else returns self. If block is given, replace invalid
  # bytes with returned value of the block.
  #
  # ```ruby
  # "abc\u3042\x81".scrub #=> "abc\u3042\uFFFD"
  # "abc\u3042\x81".scrub("*") #=> "abc\u3042*"
  # "abc\u3042\xE3\x80".scrub{|bytes| '<'+bytes.unpack('H*')[0]+'>' } #=> "abc\u3042<e380>"
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: String,
        blk: T.proc.params(arg0: T.untyped).returns(BasicObject),
    )
    .returns(String)
  end
  def scrub(arg0=T.unsafe(nil), &blk); end

  # If the string is invalid byte sequence then replace invalid bytes with given
  # replacement character, else returns self. If block is given, replace invalid
  # bytes with returned value of the block.
  #
  # ```ruby
  # "abc\u3042\x81".scrub! #=> "abc\u3042\uFFFD"
  # "abc\u3042\x81".scrub!("*") #=> "abc\u3042*"
  # "abc\u3042\xE3\x80".scrub!{|bytes| '<'+bytes.unpack('H*')[0]+'>' } #=> "abc\u3042<e380>"
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: String,
        blk: T.proc.params(arg0: T.untyped).returns(BasicObject),
    )
    .returns(String)
  end
  def scrub!(arg0=T.unsafe(nil), &blk); end

  # modifies the *index*th byte as *integer*.
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(Integer)
  end
  def setbyte(arg0, arg1); end

  # Returns the character length of *str*.
  sig {returns(Integer)}
  def size(); end

  # Deletes the specified portion from *str*, and returns the portion deleted.
  #
  # ```ruby
  # string = "this is a string"
  # string.slice!(2)        #=> "i"
  # string.slice!(3..6)     #=> " is "
  # string.slice!(/s.*t/)   #=> "sa st"
  # string.slice!("r")      #=> "r"
  # string                  #=> "thing"
  # ```
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T.any(T::Range[Integer], Regexp),
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: String,
    )
    .returns(T.nilable(String))
  end
  def slice!(arg0, arg1=T.unsafe(nil)); end

  # Divides *str* into substrings based on a delimiter, returning an array of
  # these substrings.
  #
  # If *pattern* is a `String`, then its contents are used as the delimiter when
  # splitting *str*. If *pattern* is a single space, *str* is split on
  # whitespace, with leading and trailing whitespace and runs of contiguous
  # whitespace characters ignored.
  #
  # If *pattern* is a `Regexp`, *str* is divided where the pattern matches.
  # Whenever the pattern matches a zero-length string, *str* is split into
  # individual characters. If *pattern* contains groups, the respective matches
  # will be returned in the array as well.
  #
  # If *pattern* is `nil`, the value of `$;` is used. If `$;` is `nil` (which is
  # the default), *str* is split on whitespace as if ' ' were specified.
  #
  # If the *limit* parameter is omitted, trailing null fields are suppressed. If
  # *limit* is a positive number, at most that number of split substrings will
  # be returned (captured groups will be returned as well, but are not counted
  # towards the limit). If *limit* is `1`, the entire string is returned as the
  # only entry in an array. If negative, there is no limit to the number of
  # fields returned, and trailing null fields are not suppressed.
  #
  # When the input `str` is empty an empty
  # [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) is returned as the
  # string is considered to have no fields to split.
  #
  # ```ruby
  # " now's  the time ".split       #=> ["now's", "the", "time"]
  # " now's  the time ".split(' ')  #=> ["now's", "the", "time"]
  # " now's  the time".split(/ /)   #=> ["", "now's", "", "the", "time"]
  # "1, 2.34,56, 7".split(%r{,\s*}) #=> ["1", "2.34", "56", "7"]
  # "hello".split(//)               #=> ["h", "e", "l", "l", "o"]
  # "hello".split(//, 3)            #=> ["h", "e", "llo"]
  # "hi mom".split(%r{\s*})         #=> ["h", "i", "m", "o", "m"]
  #
  # "mellow yellow".split("ello")   #=> ["m", "w y", "w"]
  # "1,2,,3,4,,".split(',')         #=> ["1", "2", "", "3", "4"]
  # "1,2,,3,4,,".split(',', 4)      #=> ["1", "2", "", "3,4,,"]
  # "1,2,,3,4,,".split(',', -4)     #=> ["1", "2", "", "3", "4", "", ""]
  #
  # "1:2:3".split(/(:)()()/, 2)     #=> ["1", ":", "", "", "2:3"]
  #
  # "".split(',', -1)               #=> []
  # ```
  #
  # If a block is given, invoke the block with each split substring.
  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: Integer,
    )
    .returns(T::Array[String])
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[String])
  end
  def split(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end

  # Builds a set of characters from the *other\_str* parameter(s) using the
  # procedure described for `String#count`. Returns a new string where runs of
  # the same character that occur in this set are replaced by a single
  # character. If no arguments are given, all runs of identical characters are
  # replaced by a single character.
  #
  # ```ruby
  # "yellow moon".squeeze                  #=> "yelow mon"
  # "  now   is  the".squeeze(" ")         #=> " now is the"
  # "putters shoot balls".squeeze("m-z")   #=> "puters shot balls"
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def squeeze(arg0=T.unsafe(nil)); end

  # Squeezes *str* in place, returning either *str*, or `nil` if no changes were
  # made.
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def squeeze!(arg0=T.unsafe(nil)); end

  # Returns true if `str` starts with one of the `prefixes` given. Each of the
  # `prefixes` should be a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) or a
  # [`Regexp`](https://docs.ruby-lang.org/en/2.6.0/Regexp.html).
  #
  # ```ruby
  # "hello".start_with?("hell")               #=> true
  # "hello".start_with?(/H/i)                 #=> true
  #
  # # returns true if one of the prefixes matches.
  # "hello".start_with?("heaven", "hell")     #=> true
  # "hello".start_with?("heaven", "paradise") #=> false
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(T::Boolean)
  end
  def start_with?(*arg0); end

  # Returns a copy of the receiver with leading and trailing whitespace removed.
  #
  # Whitespace is defined as any of the following characters: null, horizontal
  # tab, line feed, vertical tab, form feed, carriage return, space.
  #
  # ```ruby
  # "    hello    ".strip   #=> "hello"
  # "\tgoodbye\r\n".strip   #=> "goodbye"
  # "\x00\t\n\v\f\r ".strip #=> ""
  # "hello".strip           #=> "hello"
  # ```
  sig {returns(String)}
  def strip(); end

  # Removes leading and trailing whitespace from the receiver. Returns the
  # altered receiver, or `nil` if there was no change.
  #
  # Refer to
  # [`String#strip`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-strip)
  # for the definition of whitespace.
  #
  # ```ruby
  # "  hello  ".strip!  #=> "hello"
  # "hello".strip!      #=> nil
  # ```
  sig {returns(String)}
  def strip!(); end

  # Returns a copy of `str` with the *first* occurrence of `pattern` replaced by
  # the second argument. The `pattern` is typically a
  # [`Regexp`](https://docs.ruby-lang.org/en/2.6.0/Regexp.html); if given as a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), any regular
  # expression metacharacters it contains will be interpreted literally, e.g.
  # `'\\\d'` will match a backslash followed by 'd', instead of a digit.
  #
  # If `replacement` is a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) it will be
  # substituted for the matched text. It may contain back-references to the
  # pattern's capture groups of the form `"\\d"`, where *d* is a group number,
  # or `"\\k<n>"`, where *n* is a group name. If it is a double-quoted string,
  # both back-references must be preceded by an additional backslash. However,
  # within `replacement` the special match variables, such as `$&`, will not
  # refer to the current match. If `replacement` is a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) that looks like
  # a pattern's capture group but is actually not a pattern capture group e.g.
  # `"\\'"`, then it will have to be preceded by two backslashes like so
  # `"\\\\'"`.
  #
  # If the second argument is a
  # [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html), and the matched
  # text is one of its keys, the corresponding value is the replacement string.
  #
  # In the block form, the current match string is passed in as a parameter, and
  # variables such as `$1`, `$2`, `$``, `$&`, and `$'` will be set
  # appropriately. The value returned by the block will be substituted for the
  # match on each call.
  #
  # The result inherits any tainting in the original string or any supplied
  # replacement string.
  #
  # ```ruby
  # "hello".sub(/[aeiou]/, '*')                  #=> "h*llo"
  # "hello".sub(/([aeiou])/, '<\1>')             #=> "h<e>llo"
  # "hello".sub(/./) {|s| s.ord.to_s + ' ' }     #=> "104 ello"
  # "hello".sub(/(?<foo>[aeiou])/, '*\k<foo>*')  #=> "h*e*llo"
  # 'Is SHELL your preferred shell?'.sub(/[[:upper:]]{2,}/, ENV)
  #  #=> "Is /bin/bash your preferred shell?"
  # ```
  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: T.any(String, Hash),
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(String)
  end
  def sub(arg0, arg1=T.unsafe(nil), &blk); end

  # Performs the same substitution as
  # [`String#sub`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-sub)
  # in-place.
  #
  # Returns `str` if a substitution was performed or `nil` if no substitution
  # was performed.
  sig do
    params(
        arg0: T.any(Regexp, String),
        arg1: String,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: T.any(Regexp, String),
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(String)
  end
  def sub!(arg0, arg1=T.unsafe(nil), &blk); end

  # Returns the successor to *str*. The successor is calculated by incrementing
  # characters starting from the rightmost alphanumeric (or the rightmost
  # character if there are no alphanumerics) in the string. Incrementing a digit
  # always results in another digit, and incrementing a letter results in
  # another letter of the same case. Incrementing nonalphanumerics uses the
  # underlying character set's collating sequence.
  #
  # If the increment generates a "carry," the character to the left of it is
  # incremented. This process repeats until there is no carry, adding an
  # additional character if necessary.
  #
  # ```ruby
  # "abcd".succ        #=> "abce"
  # "THX1138".succ     #=> "THX1139"
  # "<<koala>>".succ   #=> "<<koalb>>"
  # "1999zzz".succ     #=> "2000aaa"
  # "ZZZ9999".succ     #=> "AAAA0000"
  # "***".succ         #=> "**+"
  # ```
  sig {returns(String)}
  def succ(); end

  # Returns a basic *n*-bit checksum of the characters in *str*, where *n* is
  # the optional `Integer` parameter, defaulting to 16. The result is simply the
  # sum of the binary value of each byte in *str* modulo `2**n - 1`. This is not
  # a particularly good checksum.
  ### sum0 &= (((unsigned long)1)\<\<bits)-1; } sum = LONG2FIX(sum0); } else {
  ### VALUE mod; if (sum0) { sum = rb\_funcall(sum, '+', 1, LONG2FIX(sum0)); }
  ### mod = rb\_funcall(INT2FIX(1), idLTLT, 1, INT2FIX(bits)); mod =
  ### rb\_funcall(mod, '-', 1, INT2FIX(1)); sum = rb\_funcall(sum, '&', 1,
  ### mod); } } return sum; }
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def sum(arg0=T.unsafe(nil)); end

  # Returns a copy of *str* with uppercase alphabetic characters converted to
  # lowercase and lowercase characters converted to uppercase.
  #
  # See
  # [`String#downcase`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-downcase)
  # for meaning of `options` and use with different encodings.
  #
  # ```ruby
  # "Hello".swapcase          #=> "hELLO"
  # "cYbEr_PuNk11".swapcase   #=> "CyBeR_pUnK11"
  # ```
  sig {returns(String)}
  def swapcase(); end

  # Equivalent to `String#swapcase`, but modifies the receiver in place,
  # returning *str*, or `nil` if no changes were made.
  #
  # See
  # [`String#downcase`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-downcase)
  # for meaning of `options` and use with different encodings.
  sig {returns(T.nilable(String))}
  def swapcase!(); end

  # Returns a complex which denotes the string form. The parser ignores leading
  # whitespaces and trailing garbage. Any digit sequences can be separated by an
  # underscore. Returns zero for null or garbage string.
  #
  # ```ruby
  # '9'.to_c           #=> (9+0i)
  # '2.5'.to_c         #=> (2.5+0i)
  # '2.5/1'.to_c       #=> ((5/2)+0i)
  # '-3/2'.to_c        #=> ((-3/2)+0i)
  # '-i'.to_c          #=> (0-1i)
  # '45i'.to_c         #=> (0+45i)
  # '3-4i'.to_c        #=> (3-4i)
  # '-4e2-4e-2i'.to_c  #=> (-400.0-0.04i)
  # '-0.0-0.0i'.to_c   #=> (-0.0-0.0i)
  # '1/2+3/4i'.to_c    #=> ((1/2)+(3/4)*i)
  # 'ruby'.to_c        #=> (0+0i)
  # ```
  #
  # See [`Kernel`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html).Complex.
  sig {returns(Complex)}
  def to_c(); end

  # Returns the result of interpreting leading characters in *str* as a floating
  # point number. Extraneous characters past the end of a valid number are
  # ignored. If there is not a valid number at the start of *str*, `0.0` is
  # returned. This method never raises an exception.
  #
  # ```ruby
  # "123.45e1".to_f        #=> 1234.5
  # "45.67 degrees".to_f   #=> 45.67
  # "thx1138".to_f         #=> 0.0
  # ```
  sig {returns(Float)}
  def to_f(); end

  # Returns the result of interpreting leading characters in *str* as an integer
  # base *base* (between 2 and 36). Extraneous characters past the end of a
  # valid number are ignored. If there is not a valid number at the start of
  # *str*, `0` is returned. This method never raises an exception when *base* is
  # valid.
  #
  # ```ruby
  # "12345".to_i             #=> 12345
  # "99 red balloons".to_i   #=> 99
  # "0a".to_i                #=> 0
  # "0a".to_i(16)            #=> 10
  # "hello".to_i             #=> 0
  # "1100101".to_i(2)        #=> 101
  # "1100101".to_i(8)        #=> 294977
  # "1100101".to_i(10)       #=> 1100101
  # "1100101".to_i(16)       #=> 17826049
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def to_i(arg0=T.unsafe(nil)); end

  # Returns the result of interpreting leading characters in `str` as a
  # rational. Leading whitespace and extraneous characters past the end of a
  # valid number are ignored. Digit sequences can be separated by an underscore.
  # If there is not a valid number at the start of `str`, zero is returned. This
  # method never raises an exception.
  #
  # ```ruby
  # '  2  '.to_r       #=> (2/1)
  # '300/2'.to_r       #=> (150/1)
  # '-9.2'.to_r        #=> (-46/5)
  # '-9.2e2'.to_r      #=> (-920/1)
  # '1_234_567'.to_r   #=> (1234567/1)
  # '21 June 09'.to_r  #=> (21/1)
  # '21/06/09'.to_r    #=> (7/2)
  # 'BWV 1079'.to_r    #=> (0/1)
  # ```
  #
  # NOTE: "0.3".to\_r isn't the same as 0.3.to\_r.  The former is equivalent to
  # "3/10".to\_r, but the latter isn't so.
  #
  # ```ruby
  # "0.3".to_r == 3/10r  #=> true
  # 0.3.to_r   == 3/10r  #=> false
  # ```
  #
  # See also Kernel#Rational.
  sig {returns(Rational)}
  def to_r(); end

  # Returns `self`.
  #
  # If called on a subclass of
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), converts the
  # receiver to a [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html)
  # object.
  sig {returns(String)}
  def to_s(); end

  # Returns `self`.
  #
  # If called on a subclass of
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), converts the
  # receiver to a [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html)
  # object.
  sig {returns(String)}
  def to_str(); end

  # Returns the `Symbol` corresponding to *str*, creating the symbol if it did
  # not previously exist. See `Symbol#id2name`.
  #
  # ```ruby
  # "Koala".intern         #=> :Koala
  # s = 'cat'.to_sym       #=> :cat
  # s == :cat              #=> true
  # s = '@cat'.to_sym      #=> :@cat
  # s == :@cat             #=> true
  # ```
  #
  # This can also be used to create symbols that cannot be represented using the
  # `:xxx` notation.
  #
  # ```ruby
  # 'cat and dog'.to_sym   #=> :"cat and dog"
  # ```
  sig {returns(Symbol)}
  def to_sym(); end

  # Returns a copy of `str` with the characters in `from_str` replaced by the
  # corresponding characters in `to_str`. If `to_str` is shorter than
  # `from_str`, it is padded with its last character in order to maintain the
  # correspondence.
  #
  # ```ruby
  # "hello".tr('el', 'ip')      #=> "hippo"
  # "hello".tr('aeiou', '*')    #=> "h*ll*"
  # "hello".tr('aeiou', 'AA*')  #=> "hAll*"
  # ```
  #
  # Both strings may use the `c1-c2` notation to denote ranges of characters,
  # and `from_str` may start with a `^`, which denotes all characters except
  # those listed.
  #
  # ```ruby
  # "hello".tr('a-y', 'b-z')    #=> "ifmmp"
  # "hello".tr('^aeiou', '*')   #=> "*e**o"
  # ```
  #
  # The backslash character `\` can be used to escape `^` or `-` and is
  # otherwise ignored unless it appears at the end of a range or the end of the
  # `from_str` or `to_str`:
  #
  # ```ruby
  # "hello^world".tr("\\^aeiou", "*") #=> "h*ll**w*rld"
  # "hello-world".tr("a\\-eo", "*")   #=> "h*ll**w*rld"
  #
  # "hello\r\nworld".tr("\r", "")   #=> "hello\nworld"
  # "hello\r\nworld".tr("\\r", "")  #=> "hello\r\nwold"
  # "hello\r\nworld".tr("\\\r", "") #=> "hello\nworld"
  #
  # "X['\\b']".tr("X\\", "")   #=> "['b']"
  # "X['\\b']".tr("X-\\]", "") #=> "'b'"
  # ```
  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(String)
  end
  def tr(arg0, arg1); end

  # Translates *str* in place, using the same rules as `String#tr`. Returns
  # *str*, or `nil` if no changes were made.
  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  def tr!(arg0, arg1); end

  # Processes a copy of *str* as described under `String#tr`, then removes
  # duplicate characters in regions that were affected by the translation.
  #
  # ```ruby
  # "hello".tr_s('l', 'r')     #=> "hero"
  # "hello".tr_s('el', '*')    #=> "h*o"
  # "hello".tr_s('el', 'hx')   #=> "hhxo"
  # ```
  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(String)
  end
  def tr_s(arg0, arg1); end

  # Performs `String#tr_s` processing on *str* in place, returning *str*, or
  # `nil` if no changes were made.
  sig do
    params(
        arg0: String,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  def tr_s!(arg0, arg1); end

  # Decodes *str* (which may contain binary data) according to the format
  # string, returning an array of each value extracted. The format string
  # consists of a sequence of single-character directives, summarized in the
  # table at the end of this entry. Each directive may be followed by a number,
  # indicating the number of times to repeat with this directive. An asterisk
  # ("`*`") will use up all remaining elements. The directives `sSiIlL` may each
  # be followed by an underscore ("`_`") or exclamation mark ("`!`") to use the
  # underlying platform's native size for the specified type; otherwise, it uses
  # a platform-independent consistent size. Spaces are ignored in the format
  # string. See also `String#unpack1`,  `Array#pack`.
  #
  # ```ruby
  # "abc \0\0abc \0\0".unpack('A6Z6')   #=> ["abc", "abc "]
  # "abc \0\0".unpack('a3a3')           #=> ["abc", " \000\000"]
  # "abc \0abc \0".unpack('Z*Z*')       #=> ["abc ", "abc "]
  # "aa".unpack('b8B8')                 #=> ["10000110", "01100001"]
  # "aaa".unpack('h2H2c')               #=> ["16", "61", 97]
  # "\xfe\xff\xfe\xff".unpack('sS')     #=> [-2, 65534]
  # "now=20is".unpack('M*')             #=> ["now is"]
  # "whole".unpack('xax2aX2aX1aX2a')    #=> ["h", "e", "l", "l", "o"]
  # ```
  #
  # This table summarizes the various formats and the Ruby classes returned by
  # each.
  #
  # ```
  # Integer       |         |
  # Directive     | Returns | Meaning
  # ------------------------------------------------------------------
  # C             | Integer | 8-bit unsigned (unsigned char)
  # S             | Integer | 16-bit unsigned, native endian (uint16_t)
  # L             | Integer | 32-bit unsigned, native endian (uint32_t)
  # Q             | Integer | 64-bit unsigned, native endian (uint64_t)
  # J             | Integer | pointer width unsigned, native endian (uintptr_t)
  #               |         |
  # c             | Integer | 8-bit signed (signed char)
  # s             | Integer | 16-bit signed, native endian (int16_t)
  # l             | Integer | 32-bit signed, native endian (int32_t)
  # q             | Integer | 64-bit signed, native endian (int64_t)
  # j             | Integer | pointer width signed, native endian (intptr_t)
  #               |         |
  # S_ S!         | Integer | unsigned short, native endian
  # I I_ I!       | Integer | unsigned int, native endian
  # L_ L!         | Integer | unsigned long, native endian
  # Q_ Q!         | Integer | unsigned long long, native endian (ArgumentError
  #               |         | if the platform has no long long type.)
  # J!            | Integer | uintptr_t, native endian (same with J)
  #               |         |
  # s_ s!         | Integer | signed short, native endian
  # i i_ i!       | Integer | signed int, native endian
  # l_ l!         | Integer | signed long, native endian
  # q_ q!         | Integer | signed long long, native endian (ArgumentError
  #               |         | if the platform has no long long type.)
  # j!            | Integer | intptr_t, native endian (same with j)
  #               |         |
  # S> s> S!> s!> | Integer | same as the directives without ">" except
  # L> l> L!> l!> |         | big endian
  # I!> i!>       |         |
  # Q> q> Q!> q!> |         | "S>" is same as "n"
  # J> j> J!> j!> |         | "L>" is same as "N"
  #               |         |
  # S< s< S!< s!< | Integer | same as the directives without "<" except
  # L< l< L!< l!< |         | little endian
  # I!< i!<       |         |
  # Q< q< Q!< q!< |         | "S<" is same as "v"
  # J< j< J!< j!< |         | "L<" is same as "V"
  #               |         |
  # n             | Integer | 16-bit unsigned, network (big-endian) byte order
  # N             | Integer | 32-bit unsigned, network (big-endian) byte order
  # v             | Integer | 16-bit unsigned, VAX (little-endian) byte order
  # V             | Integer | 32-bit unsigned, VAX (little-endian) byte order
  #               |         |
  # U             | Integer | UTF-8 character
  # w             | Integer | BER-compressed integer (see Array.pack)
  #
  # Float        |         |
  # Directive    | Returns | Meaning
  # -----------------------------------------------------------------
  # D d          | Float   | double-precision, native format
  # F f          | Float   | single-precision, native format
  # E            | Float   | double-precision, little-endian byte order
  # e            | Float   | single-precision, little-endian byte order
  # G            | Float   | double-precision, network (big-endian) byte order
  # g            | Float   | single-precision, network (big-endian) byte order
  #
  # String       |         |
  # Directive    | Returns | Meaning
  # -----------------------------------------------------------------
  # A            | String  | arbitrary binary string (remove trailing nulls and ASCII spaces)
  # a            | String  | arbitrary binary string
  # Z            | String  | null-terminated string
  # B            | String  | bit string (MSB first)
  # b            | String  | bit string (LSB first)
  # H            | String  | hex string (high nibble first)
  # h            | String  | hex string (low nibble first)
  # u            | String  | UU-encoded string
  # M            | String  | quoted-printable, MIME encoding (see RFC2045)
  # m            | String  | base64 encoded string (RFC 2045) (default)
  #              |         | base64 encoded string (RFC 4648) if followed by 0
  # P            | String  | pointer to a structure (fixed-length string)
  # p            | String  | pointer to a null-terminated string
  #
  # Misc.        |         |
  # Directive    | Returns | Meaning
  # -----------------------------------------------------------------
  # @            | ---     | skip to the offset given by the length argument
  # X            | ---     | skip backward one byte
  # x            | ---     | skip forward one byte
  # ```
  #
  # HISTORY
  #
  # *   J, J! j, and j! are available since Ruby 2.3.
  # *   Q\_, Q!, q\_, and q! are available since Ruby 2.1.
  # *   I!<, i!<, I!>, and i!> are available since Ruby 1.9.3.
  sig do
    params(
        arg0: String,
    )
    .returns(T::Array[T.nilable(T.any(Integer, Float, String))])
  end
  def unpack(arg0); end

  # Returns a copy of *str* with all lowercase letters replaced with their
  # uppercase counterparts.
  #
  # See
  # [`String#downcase`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-downcase)
  # for meaning of `options` and use with different encodings.
  #
  # ```ruby
  # "hEllO".upcase   #=> "HELLO"
  # ```
  sig {returns(String)}
  def upcase(); end

  # Upcases the contents of *str*, returning `nil` if no changes were made.
  #
  # See
  # [`String#downcase`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-downcase)
  # for meaning of `options` and use with different encodings.
  sig {returns(T.nilable(String))}
  def upcase!(); end

  # Iterates through successive values, starting at *str* and ending at
  # *other\_str* inclusive, passing each value in turn to the block. The
  # `String#succ` method is used to generate each value. If optional second
  # argument exclusive is omitted or is false, the last value will be included;
  # otherwise it will be excluded.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # "a8".upto("b6") {|s| print s, ' ' }
  # for s in "a8".."b6"
  #   print s, ' '
  # end
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # a8 a9 b0 b1 b2 b3 b4 b5 b6
  # a8 a9 b0 b1 b2 b3 b4 b5 b6
  # ```
  #
  # If *str* and *other\_str* contains only ascii numeric characters, both are
  # recognized as decimal numbers. In addition, the width of string (e.g.
  # leading zeros) is handled appropriately.
  #
  # ```ruby
  # "9".upto("11").to_a   #=> ["9", "10", "11"]
  # "25".upto("5").to_a   #=> []
  # "07".upto("11").to_a  #=> ["07", "08", "09", "10", "11"]
  # ```
  sig do
    type_parameters(:Bool).params(
        arg0: String,
        arg1: T.type_parameter(:Bool),
    )
    .returns(T::Enumerator[String])
  end
  sig do
    type_parameters(:Bool).params(
        arg0: String,
        arg1: T.type_parameter(:Bool),
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(String)
  end
  def upto(arg0, arg1=T.unsafe(nil), &blk); end

  # Returns true for a string which is encoded correctly.
  #
  # ```ruby
  # "\xc2\xa1".force_encoding("UTF-8").valid_encoding?  #=> true
  # "\xc2".force_encoding("UTF-8").valid_encoding?      #=> false
  # "\x80".force_encoding("UTF-8").valid_encoding?      #=> false
  # ```
  sig {returns(T::Boolean)}
  def valid_encoding?(); end

  # Try to convert *obj* into a
  # [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), using
  # [`to_str`](https://docs.ruby-lang.org/en/2.6.0/String.html#method-i-to_str)
  # method. Returns converted string or nil if *obj* cannot be converted for any
  # reason.
  #
  # ```ruby
  # String.try_convert("str")     #=> "str"
  # String.try_convert(/re/)      #=> nil
  # ```
  sig do
    params(
        obj: Object,
    )
    .returns(T.nilable(String))
  end
  def self.try_convert(obj); end

  # Element Reference --- If passed a single `index`, returns a substring of one
  # character at that index. If passed a `start` index and a `length`, returns a
  # substring containing `length` characters starting at the `start` index. If
  # passed a `range`, its beginning and end are interpreted as offsets
  # delimiting the substring to be returned.
  #
  # In these three cases, if an index is negative, it is counted from the end of
  # the string. For the `start` and `range` cases the starting index is just
  # before a character and an index matching the string's size. Additionally, an
  # empty string is returned when the starting index for a character range is at
  # the end of the string.
  #
  # Returns `nil` if the initial index falls outside the string or the length is
  # negative.
  #
  # If a `Regexp` is supplied, the matching portion of the string is returned.
  # If a `capture` follows the regular expression, which may be a capture group
  # index or name, follows the regular expression that component of the
  # [`MatchData`](https://docs.ruby-lang.org/en/2.6.0/MatchData.html) is
  # returned instead.
  #
  # If a `match_str` is given, that string is returned if it occurs in the
  # string.
  #
  # Returns `nil` if the regular expression does not match or the match string
  # cannot be found.
  #
  # ```ruby
  # a = "hello there"
  #
  # a[1]                   #=> "e"
  # a[2, 3]                #=> "llo"
  # a[2..3]                #=> "ll"
  #
  # a[-3, 2]               #=> "er"
  # a[7..-2]               #=> "her"
  # a[-4..-2]              #=> "her"
  # a[-2..-4]              #=> ""
  #
  # a[11, 0]               #=> ""
  # a[11]                  #=> nil
  # a[12, 0]               #=> nil
  # a[12..-1]              #=> nil
  #
  # a[/[aeiou](.)\1/]      #=> "ell"
  # a[/[aeiou](.)\1/, 0]   #=> "ell"
  # a[/[aeiou](.)\1/, 1]   #=> "l"
  # a[/[aeiou](.)\1/, 2]   #=> nil
  #
  # a[/(?<vowel>[aeiou])(?<non_vowel>[^aeiou])/, "non_vowel"] #=> "l"
  # a[/(?<vowel>[aeiou])(?<non_vowel>[^aeiou])/, "vowel"]     #=> "e"
  #
  # a["lo"]                #=> "lo"
  # a["bye"]               #=> nil
  # ```
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T.any(T::Range[Integer], Regexp),
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: String,
    )
    .returns(T.nilable(String))
  end
  def slice(arg0, arg1=T.unsafe(nil)); end
end
