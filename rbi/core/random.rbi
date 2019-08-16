# typed: __STDLIB_INTERNAL

# [Random](Random) provides an interface to Ruby's
# pseudo-random number generator, or PRNG. The PRNG produces a
# deterministic sequence of bits which approximate true randomness. The
# sequence may be represented by integers, floats, or binary strings.
# 
# The generator may be initialized with either a system-generated or
# user-supplied seed value by using
# [::srand](Random#method-c-srand) .
# 
# The class method [\#rand](Random#method-i-rand)
# provides the base functionality of
# [Kernel\#rand](https://ruby-doc.org/core-2.6.3/Kernel.html#method-i-rand)
# along with better handling of floating point values. These are both
# interfaces to Random::DEFAULT, the Ruby system PRNG.
# 
# [::new](Random#method-c-new) will create a new PRNG
# with a state independent of Random::DEFAULT, allowing multiple
# generators with different seed values or sequence positions to exist
# simultaneously. [Random](Random) objects can be
# marshaled, allowing sequences to be saved and resumed.
# 
# PRNGs are currently implemented as a modified Mersenne Twister with a
# period of 2\*\*19937-1.
class Random < Object
  include Random::Formatter

  DEFAULT = T.let(T.unsafe(nil), Random)

  # Returns true if the two generators have the same internal state,
  # otherwise false. Equivalent generators will return the same sequence of
  # pseudo-random numbers. Two generators will generally have the same state
  # only if they were initialized with the same seed
  # 
  # ```ruby
  # Random.new == Random.new             # => false
  # Random.new(1234) == Random.new(1234) # => true
  # ```
  # 
  # and have the same invocation history.
  # 
  # ```ruby
  # prng1 = Random.new(1234)
  # prng2 = Random.new(1234)
  # prng1 == prng2 # => true
  # 
  # prng1.rand     # => 0.1915194503788923
  # prng1 == prng2 # => false
  # 
  # prng2.rand     # => 0.1915194503788923
  # prng1 == prng2 # => true
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  # Returns a random binary string containing `size` bytes.
  # 
  # ```ruby
  # random_string = Random.new.bytes(10) # => "\xD7:R\xAB?\x83\xCE\xFAkO"
  # random_string.size                   # => 10
  # ```
  sig do
    params(
        size: Integer,
    )
    .returns(String)
  end
  def bytes(size); end

  sig do
    params(
        seed: Integer,
    )
    .void
  end
  def initialize(seed=T.unsafe(nil)); end

  # When `max` is an [Integer](https://ruby-doc.org/core-2.6.3/Integer.html)
  # , `rand` returns a random integer greater than or equal to zero and less
  # than `max` . Unlike
  # [Kernel\#rand](https://ruby-doc.org/core-2.6.3/Kernel.html#method-i-rand)
  # , when `max` is a negative integer or zero, `rand` raises an
  # [ArgumentError](https://ruby-doc.org/core-2.6.3/ArgumentError.html) .
  # 
  # ```ruby
  # prng = Random.new
  # prng.rand(100)       # => 42
  # ```
  # 
  # When `max` is a [Float](https://ruby-doc.org/core-2.6.3/Float.html) ,
  # `rand` returns a random floating point number between 0.0 and `max`,
  # including 0.0 and excluding `max` .
  # 
  # ```ruby
  # prng.rand(1.5)       # => 1.4600282860034115
  # ```
  # 
  # When `max` is a [Range](https://ruby-doc.org/core-2.6.3/Range.html) ,
  # `rand` returns a random number where range.member?(number) == true.
  # 
  # ```ruby
  # prng.rand(5..9)      # => one of [5, 6, 7, 8, 9]
  # prng.rand(5...9)     # => one of [5, 6, 7, 8]
  # prng.rand(5.0..9.0)  # => between 5.0 and 9.0, including 9.0
  # prng.rand(5.0...9.0) # => between 5.0 and 9.0, excluding 9.0
  # ```
  # 
  # Both the beginning and ending values of the range must respond to
  # subtract ( `-` ) and add ( `+` )methods, or rand will raise an
  # [ArgumentError](https://ruby-doc.org/core-2.6.3/ArgumentError.html) .
  sig do
    params(
        max: T.any(Integer, T::Range[Integer]),
    )
    .returns(Integer)
  end
  sig do
    params(
        max: T.any(Float, T::Range[Float]),
    )
    .returns(Float)
  end
  def rand(max=T.unsafe(nil)); end

  # Returns the seed value used to initialize the generator. This may be
  # used to initialize another generator with the same state at a later
  # time, causing it to produce the same sequence of numbers.
  # 
  # ```ruby
  # prng1 = Random.new(1234)
  # prng1.seed       #=> 1234
  # prng1.rand(100)  #=> 47
  # 
  # prng2 = Random.new(prng1.seed)
  # prng2.rand(100)  #=> 47
  # ```
  sig {returns(Integer)}
  def seed(); end

  # Returns an arbitrary seed value. This is used by
  # [::new](Random.downloaded.ruby_doc#method-c-new) when no seed value is
  # specified as an argument.
  # 
  # ```ruby
  # Random.new_seed  #=> 115032730400174366788466674494640623225
  # ```
  sig {returns(Integer)}
  def self.new_seed(); end

  # Alias of Random::DEFAULT.rand.
  sig do
    params(
        max: Integer,
    )
    .returns(Numeric)
  end
  def self.rand(max=T.unsafe(nil)); end

  # Seeds the system pseudo-random number generator, Random::DEFAULT, with
  # `number` . The previous seed value is returned.
  # 
  # If `number` is omitted, seeds the generator using a source of entropy
  # provided by the operating system, if available (/dev/urandom on Unix
  # systems or the RSA cryptographic provider on Windows), which is then
  # combined with the time, the process id, and a sequence number.
  # 
  # srand may be used to ensure repeatable sequences of pseudo-random
  # numbers between different runs of the program. By setting the seed to a
  # known value, programs can be made deterministic during testing.
  # 
  # ```ruby
  # srand 1234               # => 268519324636777531569100071560086917274
  # [ rand, rand ]           # => [0.1915194503788923, 0.6221087710398319]
  # [ rand(10), rand(1000) ] # => [4, 664]
  # srand 1234               # => 1234
  # [ rand, rand ]           # => [0.1915194503788923, 0.6221087710398319]
  # ```
  sig do
    params(
        number: Integer,
    )
    .returns(Numeric)
  end
  def self.srand(number=T.unsafe(nil)); end
end

module Random::Formatter
  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(String)
  end
  def base64(n=nil); end

  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(String)
  end
  def hex(n=nil); end

  sig {returns(Float)}
  sig do
    params(
      n: T.nilable(Float)
    )
    .returns(Float)
  end
  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(Integer)
  end
  sig do
    params(
      n: T.nilable(Numeric)
    )
    .returns(Numeric)
  end
  sig do
    params(
      n: T.nilable(T::Range[Float])
    )
    .returns(Float)
  end
  sig do
    params(
      n: T.nilable(T::Range[Integer])
    )
    .returns(Integer)
  end
  sig do
    params(
      n: T.nilable(T::Range[Numeric])
    )
    .returns(Numeric)
  end
  def rand(n=nil); end

  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(String)
  end
  def random_bytes(n=nil); end

  sig {returns(Float)}
  sig do
    params(
      n: T.nilable(Float)
    )
    .returns(Float)
  end
  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(Integer)
  end
  sig do
    params(
      n: T.nilable(Numeric)
    )
    .returns(Numeric)
  end
  sig do
    params(
      n: T.nilable(T::Range[Float])
    )
    .returns(Float)
  end
  sig do
    params(
      n: T.nilable(T::Range[Integer])
    )
    .returns(Integer)
  end
  sig do
    params(
      n: T.nilable(T::Range[Numeric])
    )
    .returns(Numeric)
  end
  def random_number(n=nil); end

  sig do
    params(
      n: T.nilable(Integer),
      padding: T::Boolean
    )
    .returns(String)
  end
  def urlsafe_base64(n=nil, padding=false); end

  sig {returns(String)}
  def uuid(); end
end
