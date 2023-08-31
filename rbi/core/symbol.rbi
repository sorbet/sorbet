# typed: __STDLIB_INTERNAL

# [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html) objects represent
# names inside the Ruby interpreter. They are generated using the `:name` and
# `:"string"` literals syntax, and by the various `to_sym` methods. The same
# [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html) object will be
# created for a given name or string for the duration of a program's execution,
# regardless of the context or meaning of that name. Thus if `Fred` is a
# constant in one context, a method in another, and a class in a third, the
# [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html) `:Fred` will be
# the same object in all three contexts.
#
# ```ruby
# module One
#   class Fred
#   end
#   $f1 = :Fred
# end
# module Two
#   Fred = 1
#   $f2 = :Fred
# end
# def Fred()
# end
# $f3 = :Fred
# $f1.object_id   #=> 2514190
# $f2.object_id   #=> 2514190
# $f3.object_id   #=> 2514190
# ```
class Symbol < Object
  include Comparable

  # Returns an array of all the symbols currently in Ruby's symbol table.
  #
  # ```
  # Symbol.all_symbols.size    #=> 903
  # Symbol.all_symbols[1,20]   #=> [:floor, :ARGV, :Binding, :symlink,
  #                                 :chown, :EOFError, :$;, :String,
  #                                 :LOCK_SH, :"setuid?", :$<,
  #                                 :default_proc, :compact, :extend,
  #                                 :Tms, :getwd, :$=, :ThreadGroup,
  #                                 :wait2, :$>]
  # ```
  sig {returns(T::Array[Symbol])}
  def self.all_symbols(); end

  # Compares `symbol` with `other_symbol` after calling
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html#method-i-to_s) on
  # each of the symbols. Returns -1, 0, +1, or `nil` depending on whether
  # `symbol` is less than, equal to, or greater than `other_symbol`.
  #
  # `nil` is returned if the two values are incomparable.
  #
  # See String#<=> for more information.
  sig do
    params(
        other: Object,
    )
    .returns(T.nilable(Integer))
  end
  def <=>(other); end

  # Equality---If *sym* and *obj* are exactly the same symbol, returns `true`.
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(obj); end

  # Equality---If *sym* and *obj* are exactly the same symbol, returns `true`.
  sig {params(obj: T.anything).returns(T::Boolean)}
  def ===(obj); end

  # Returns `sym.to_s =~ obj`.
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T.nilable(Integer))
  end
  def =~(obj); end

  # Returns `sym.to_s[]`.
  sig do
    params(
        idx_or_range: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        idx_or_range: Integer,
        n: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        idx_or_range: T::Range[Integer],
    )
    .returns(String)
  end
  def [](idx_or_range, n=T.unsafe(nil)); end

  # Same as `sym.to_s.capitalize.intern`.
  sig {returns(Symbol)}
  def capitalize(); end

  # Case-insensitive version of Symbol#<=>. Currently, case-insensitivity only
  # works on characters A-Z/a-z, not all of Unicode. This is different from
  # [`Symbol#casecmp?`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html#method-i-casecmp-3F).
  #
  # ```ruby
  # :aBcDeF.casecmp(:abcde)     #=> 1
  # :aBcDeF.casecmp(:abcdef)    #=> 0
  # :aBcDeF.casecmp(:abcdefg)   #=> -1
  # :abcdef.casecmp(:ABCDEF)    #=> 0
  # ```
  #
  # `nil` is returned if the two symbols have incompatible encodings, or if
  # `other_symbol` is not a symbol.
  #
  # ```ruby
  # :foo.casecmp(2)   #=> nil
  # "\u{e4 f6 fc}".encode("ISO-8859-1").to_sym.casecmp(:"\u{c4 d6 dc}")   #=> nil
  # ```
  sig do
    params(
        other: Symbol,
    )
    .returns(T.nilable(Integer))
  end
  def casecmp(other); end

  # Returns `true` if `sym` and `other_symbol` are equal after Unicode case
  # folding, `false` if they are not equal.
  #
  # ```ruby
  # :aBcDeF.casecmp?(:abcde)     #=> false
  # :aBcDeF.casecmp?(:abcdef)    #=> true
  # :aBcDeF.casecmp?(:abcdefg)   #=> false
  # :abcdef.casecmp?(:ABCDEF)    #=> true
  # :"\u{e4 f6 fc}".casecmp?(:"\u{c4 d6 dc}")   #=> true
  # ```
  #
  # `nil` is returned if the two symbols have incompatible encodings, or if
  # `other_symbol` is not a symbol.
  #
  # ```ruby
  # :foo.casecmp?(2)   #=> nil
  # "\u{e4 f6 fc}".encode("ISO-8859-1").to_sym.casecmp?(:"\u{c4 d6 dc}")   #=> nil
  # ```
  sig do
    params(
        other: Symbol,
    )
    .returns(T.nilable(T::Boolean))
  end
  def casecmp?(other); end

  # Same as `sym.to_s.downcase.intern`.
  sig {returns(Symbol)}
  def downcase(); end

  # Returns whether *sym* is :"" or not.
  sig {returns(T::Boolean)}
  def empty?(); end

  # Returns the [`Encoding`](https://docs.ruby-lang.org/en/2.7.0/Encoding.html)
  # object that represents the encoding of *sym*.
  sig {returns(Encoding)}
  def encoding(); end

  # Returns true if `sym` ends with one of the `suffixes` given.
  #
  # ```ruby
  # :hello.end_with?("ello")               #=> true
  #
  # # returns true if one of the +suffixes+ matches.
  # :hello.end_with?("heaven", "ello")     #=> true
  # :hello.end_with?("heaven", "paradise") #=> false
  # ```
  sig do
    params(
        arg0: String,
    )
    .returns(T::Boolean)
  end
  def end_with?(*arg0); end

  # Returns the name or string corresponding to *sym*.
  #
  # ```ruby
  # :fred.id2name   #=> "fred"
  # :ginger.to_s    #=> "ginger"
  # ```
  sig {returns(String)}
  def id2name(); end

  # Returns the representation of *sym* as a symbol literal.
  #
  # ```ruby
  # :fred.inspect   #=> ":fred"
  # ```
  sig {returns(String)}
  def inspect(); end

  # In general, `to_sym` returns the
  # [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html) corresponding to
  # an object. As *sym* is already a symbol, `self` is returned in this case.
  sig {returns(T.self_type)}
  def intern(); end

  # Same as `sym.to_s.length`.
  sig {returns(Integer)}
  def length(); end

  # Returns `sym.to_s.match`.
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T.nilable(MatchData))
  end
  def match(obj); end

  # Returns `sym.to_s.match?`.
  sig do
    params(
        args: T.untyped
    ).returns(T::Boolean)
  end
  def match?(*args); end

  # Returns the name or string corresponding to *sym*. Unlike `to_s`, the returned string is frozen.
  sig {returns(String)}
  def name(); end

  # Same as `sym.to_s.succ.intern`.
  sig {returns(Symbol)}
  def next(); end

  # Same as `sym.to_s.succ.intern`.
  sig {returns(Symbol)}
  def succ(); end

  # Same as `sym.to_s.swapcase.intern`.
  sig {returns(Symbol)}
  def swapcase(); end

  # Returns a *Proc* object which responds to the given method by *sym*.
  #
  # ```ruby
  # (1..3).collect(&:to_s)  #=> ["1", "2", "3"]
  # ```
  sig {returns(Proc)}
  def to_proc(); end

  # Same as `sym.to_s.upcase.intern`.
  sig {returns(Symbol)}
  def upcase(); end

  # Same as `sym.to_s.length`.
  sig {returns(Integer)}
  def size(); end

  # Returns `sym.to_s[]`.
  sig do
    params(
        idx_or_range: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        idx_or_range: Integer,
        n: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        idx_or_range: T::Range[Integer],
    )
    .returns(String)
  end
  def slice(idx_or_range, n=T.unsafe(nil)); end

  # Returns true if `sym` starts with one of the `prefixes` given. Each of the
  # `prefixes` should be a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or a
  # [`Regexp`](https://docs.ruby-lang.org/en/2.7.0/Regexp.html).
  #
  # ```ruby
  # :hello.start_with?("hell")               #=> true
  # :hello.start_with?(/H/i)                 #=> true
  #
  # # returns true if one of the prefixes matches.
  # :hello.start_with?("heaven", "hell")     #=> true
  # :hello.start_with?("heaven", "paradise") #=> false
  # ```
  sig do
    params(
        arg0: T.any(String, Regexp),
    )
    .returns(T::Boolean)
  end
  def start_with?(*arg0); end

  # Returns the name or string corresponding to *sym*.
  #
  # ```ruby
  # :fred.id2name   #=> "fred"
  # :ginger.to_s    #=> "ginger"
  # ```
  sig {returns(String)}
  def to_s(); end

  # In general, `to_sym` returns the
  # [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html) corresponding to
  # an object. As *sym* is already a symbol, `self` is returned in this case.
  sig {returns(T.self_type)}
  def to_sym(); end
end
