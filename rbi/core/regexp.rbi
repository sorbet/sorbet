# typed: __STDLIB_INTERNAL

class Regexp < Object
  EXTENDED = T.let(T.unsafe(nil), Integer)
  FIXEDENCODING = T.let(T.unsafe(nil), Integer)
  IGNORECASE = T.let(T.unsafe(nil), Integer)
  MULTILINE = T.let(T.unsafe(nil), Integer)
  NOENCODING = T.let(T.unsafe(nil), Integer)

  # Escapes any characters that would have special meaning in a regular
  # expression. Returns a new escaped string, or self if no characters are
  # escaped. For any string, `Regexp.new(Regexp.escape( str ))=~ str` will
  # be true.
  # 
  # ```ruby
  # Regexp.escape('\*?{}.')   #=> \\\*\?\{\}\.
  # ```
  sig do
    params(
        arg0: T.any(String, Symbol),
    )
    .returns(String)
  end
  def self.escape(arg0); end

  sig {returns(MatchData)}
  sig do
    params(
        arg0: Integer,
    )
    .returns(String)
  end
  def self.last_match(arg0=T.unsafe(nil)); end

  # Try to convert *obj* into a [Regexp](Regexp.downloaded.ruby_doc) , using
  # to\_regexp method. Returns converted regexp or nil if *obj* cannot be
  # converted for any reason.
  # 
  # ```ruby
  # Regexp.try_convert(/re/)         #=> /re/
  # Regexp.try_convert("re")         #=> nil
  # 
  # o = Object.new
  # Regexp.try_convert(o)            #=> nil
  # def o.to_regexp() /foo/ end
  # Regexp.try_convert(o)            #=> /foo/
  # ```
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T.nilable(Regexp))
  end
  def self.try_convert(obj); end

  # Equality—Two regexps are equal if their patterns are identical, they
  # have the same character set code, and their `casefold?` values are the
  # same.
  # 
  # ```ruby
  # /abc/  == /abc/x   #=> false
  # /abc/  == /abc/i   #=> false
  # /abc/  == /abc/u   #=> false
  # /abc/u == /abc/n   #=> false
  # ```
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(other); end

  # Case Equality—Used in case statements.
  # 
  # ```ruby
  # a = "HELLO"
  # case a
  # when /\A[a-z]*\z/; print "Lower case\n"
  # when /\A[A-Z]*\z/; print "Upper case\n"
  # else;              print "Mixed case\n"
  # end
  # #=> "Upper case"
  # ```
  # 
  # Following a regular expression literal with the
  # [===](Regexp.downloaded.ruby_doc#method-i-3D-3D-3D) operator allows you
  # to compare against a
  # [String](https://ruby-doc.org/core-2.6.3/String.html) .
  # 
  # ```ruby
  # /^[a-z]*$/ === "HELLO" #=> false
  # /^[A-Z]*$/ === "HELLO" #=> true
  # ```
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(other); end

  sig do
    params(
        str: T.nilable(String),
    )
    .returns(T.nilable(Integer))
  end
  def =~(str); end

  # Returns the value of the case-insensitive flag.
  # 
  # ```ruby
  # /a/.casefold?           #=> false
  # /a/i.casefold?          #=> true
  # /(?i:a)/.casefold?      #=> false
  # ```
  sig {returns(T::Boolean)}
  def casefold?(); end

  # Returns the [Encoding](https://ruby-doc.org/core-2.6.3/Encoding.html)
  # object that represents the encoding of obj.
  sig {returns(Encoding)}
  def encoding(); end

  sig {returns(T::Boolean)}
  def fixed_encoding?(); end

  # Produce a hash based on the text and options of this regular expression.
  # 
  # See also Object\#hash.
  sig {returns(Integer)}
  def hash(); end

  sig do
    params(
        arg0: String,
        options: BasicObject,
        kcode: String,
    )
    .returns(Object)
  end
  sig do
    params(
        arg0: Regexp,
    )
    .void
  end
  def initialize(arg0, options=T.unsafe(nil), kcode=T.unsafe(nil)); end

  # Produce a nicely formatted string-version of *rxp* . Perhaps
  # surprisingly, `#inspect` actually produces the more natural version of
  # the string than `#to_s` .
  # 
  # ```ruby
  # /ab+c/ix.inspect        #=> "/ab+c/ix"
  # ```
  sig {returns(String)}
  def inspect(); end

  # Returns a `MatchData` object describing the match, or `nil` if there was
  # no match. This is equivalent to retrieving the value of the special
  # variable `$~` following a normal match. If the second parameter is
  # present, it specifies the position in the string to begin the search.
  # 
  # ```ruby
  # /(.)(.)(.)/.match("abc")[2]   #=> "b"
  # /(.)(.)/.match("abc", 1)[2]   #=> "c"
  # ```
  # 
  # If a block is given, invoke the block with
  # [MatchData](https://ruby-doc.org/core-2.6.3/MatchData.html) if match
  # succeed, so that you can write
  # 
  # ```ruby
  # /M(.*)/.match("Matz") do |m|
  #   puts m[0]
  #   puts m[1]
  # end
  # ```
  # 
  # instead of
  # 
  # ```ruby
  # if m = /M(.*)/.match("Matz")
  #   puts m[0]
  #   puts m[1]
  # end
  # ```
  # 
  # The return value is a value from block execution in this case.
  sig do
    params(
        arg0: T.nilable(String),
        arg1: Integer,
    )
    .returns(T.nilable(MatchData))
  end
  def match(arg0, arg1=T.unsafe(nil)); end

  sig {returns(T::Hash[String, T::Array[Integer]])}
  def named_captures(); end

  sig {returns(T::Array[String])}
  def names(); end

  # Returns the set of bits corresponding to the options used when creating
  # this [Regexp](Regexp.downloaded.ruby_doc) (see `Regexp::new` for
  # details. Note that additional bits may be set in the returned options:
  # these are used internally by the regular expression code. These extra
  # bits are ignored if the options are passed to `Regexp::new` .
  # 
  # ```ruby
  # Regexp::IGNORECASE                  #=> 1
  # Regexp::EXTENDED                    #=> 2
  # Regexp::MULTILINE                   #=> 4
  # 
  # /cat/.options                       #=> 0
  # /cat/ix.options                     #=> 3
  # Regexp.new('cat', true).options     #=> 1
  # /\xa1\xa2/e.options                 #=> 16
  # 
  # r = /cat/ix
  # Regexp.new(r.source, r.options)     #=> /cat/ix
  # ```
  sig {returns(Integer)}
  def options(); end

  # Returns the original string of the pattern.
  # 
  # ```ruby
  # /ab+c/ix.source #=> "ab+c"
  # ```
  # 
  # Note that escape sequences are retained as is.
  # 
  # ```ruby
  # /\x20\+/.source  #=> "\\x20\\+"
  # ```
  sig {returns(String)}
  def source(); end

  # Returns a string containing the regular expression and its options
  # (using the `(?opts:source)` notation. This string can be fed back in to
  # `Regexp::new` to a regular expression with the same semantics as the
  # original. (However, `Regexp#==` may not return true when comparing the
  # two, as the source of the regular expression itself may differ, as the
  # example shows). `Regexp#inspect` produces a generally more readable
  # version of *rxp* .
  # 
  # ```ruby
  # r1 = /ab+c/ix           #=> /ab+c/ix
  # s1 = r1.to_s            #=> "(?ix-m:ab+c)"
  # r2 = Regexp.new(s1)     #=> /(?ix-m:ab+c)/
  # r1 == r2                #=> false
  # r1.source               #=> "ab+c"
  # r2.source               #=> "(?ix-m:ab+c)"
  # ```
  sig {returns(String)}
  def to_s(); end

  sig {returns(T.nilable(Integer))}
  def ~(); end

  # Alias for `Regexp.new`
  sig do
    params(
        arg0: String,
        options: BasicObject,
        kcode: String,
    )
    .returns(T.self_type)
  end
  sig do
    params(
        arg0: Regexp,
    )
    .returns(T.self_type)
  end
  def self.compile(arg0, options=T.unsafe(nil), kcode=T.unsafe(nil)); end

  # Escapes any characters that would have special meaning in a regular
  # expression. Returns a new escaped string, or self if no characters are
  # escaped. For any string, `Regexp.new(Regexp.escape( str ))=~ str` will
  # be true.
  # 
  # ```ruby
  # Regexp.escape('\*?{}.')   #=> \\\*\?\{\}\.
  # ```
  sig do
    params(
        arg0: T.any(String, Symbol),
    )
    .returns(String)
  end
  def self.quote(arg0); end

  # Equality—Two regexps are equal if their patterns are identical, they
  # have the same character set code, and their `casefold?` values are the
  # same.
  # 
  # ```ruby
  # /abc/  == /abc/x   #=> false
  # /abc/  == /abc/i   #=> false
  # /abc/  == /abc/u   #=> false
  # /abc/u == /abc/n   #=> false
  # ```
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def eql?(other); end
end
