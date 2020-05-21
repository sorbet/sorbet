# typed: __STDLIB_INTERNAL

# The set of all prime numbers.
#
# ## Example
#
# ```ruby
# Prime.each(100) do |prime|
#   p prime  #=> 2, 3, 5, 7, 11, ...., 97
# end
# ```
#
# [`Prime`](https://docs.ruby-lang.org/en/2.6.0/Prime.html) is Enumerable:
#
# ```ruby
# Prime.first 5 # => [2, 3, 5, 7, 11]
# ```
#
# ## Retrieving the instance
#
# For convenience, each instance method of `Prime`.instance can be accessed as a
# class method of `Prime`.
#
# e.g.
#
# ```ruby
# Prime.instance.prime?(2)  #=> true
# Prime.prime?(2)           #=> true
# ```
#
# ## Generators
#
# A "generator" provides an implementation of enumerating pseudo-prime numbers
# and it remembers the position of enumeration and upper bound. Furthermore, it
# is an external iterator of prime enumeration which is compatible with an
# [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html).
#
# `Prime`::`PseudoPrimeGenerator` is the base class for generators. There are
# few implementations of generator.
#
# `Prime`::`EratosthenesGenerator`
# :   Uses eratosthenes' sieve.
# `Prime`::`TrialDivisionGenerator`
# :   Uses the trial division method.
# `Prime`::`Generator23`
# :   Generates all positive integers which are not divisible by either 2 or 3.
#     This sequence is very bad as a pseudo-prime sequence. But this is faster
#     and uses much less memory than the other generators. So, it is suitable
#     for factorizing an integer which is not large but has many prime factors.
#     e.g. for
#     [`Prime#prime?`](https://docs.ruby-lang.org/en/2.6.0/Prime.html#method-i-prime-3F)
#     .
class Prime
  include(::Singleton)
  include(::Enumerable)

  Elem = type_member(:out)

  # Iterates the given block over all prime numbers.
  #
  # ## Parameters
  #
  # `ubound`
  # :   Optional. An arbitrary positive number. The upper bound of enumeration.
  #     The method enumerates prime numbers infinitely if `ubound` is nil.
  # `generator`
  # :   Optional. An implementation of pseudo-prime generator.
  #
  #
  # ## Return value
  #
  # An evaluated value of the given block at the last time. Or an enumerator
  # which is compatible to an `Enumerator` if no block given.
  #
  # ## Description
  #
  # Calls `block` once for each prime number, passing the prime as a parameter.
  #
  # `ubound`
  # :   Upper bound of prime numbers. The iterator stops after it yields all
  #     prime numbers p <= `ubound`.
  def each(ubound = nil, generator = Prime::EratosthenesGenerator.new, &block); end

  # Returns `true` if `obj` is an
  # [`Integer`](https://docs.ruby-lang.org/en/2.6.0/Integer.html) and is prime.
  # Also returns true if `obj` is a Module that is an ancestor of `Prime`.
  # Otherwise returns false.
  def include?(obj); end

  # Re-composes a prime factorization and returns the product.
  #
  # ## Parameters
  # `pd`
  # :   [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html) of pairs of
  #     integers. The each internal pair consists of a prime number -- a prime
  #     factor -- and a natural number -- an exponent.
  #
  #
  # ## Example
  # For `[[p_1, e_1], [p_2, e_2], ...., [p_n, e_n]]`, it returns:
  #
  # ```
  # p_1**e_1 * p_2**e_2 * .... * p_n**e_n.
  #
  # Prime.int_from_prime_division([[2,2], [3,1]])  #=> 12
  # ```
  def int_from_prime_division(pd); end

  # Returns true if `value` is a prime number, else returns false.
  #
  # ## Parameters
  #
  # `value`
  # :   an arbitrary integer to be checked.
  # `generator`
  # :   optional. A pseudo-prime generator.
  def prime?(value, generator = Prime::Generator23.new); end

  # Returns the factorization of `value`.
  #
  # ## Parameters
  # `value`
  # :   An arbitrary integer.
  # `generator`
  # :   Optional. A pseudo-prime generator. `generator`.succ must return the
  #     next pseudo-prime number in the ascending order. It must generate all
  #     prime numbers, but may also generate non prime numbers too.
  #
  #
  # ### Exceptions
  # `ZeroDivisionError`
  # :   when `value` is zero.
  #
  #
  # ## Example
  # For an arbitrary integer:
  #
  # ```
  # n = p_1**e_1 * p_2**e_2 * .... * p_n**e_n,
  # ```
  #
  # [`prime_division`](https://docs.ruby-lang.org/en/2.6.0/Prime.html#method-i-prime_division)(n)
  # returns:
  #
  # ```
  # [[p_1, e_1], [p_2, e_2], ...., [p_n, e_n]].
  #
  # Prime.prime_division(12) #=> [[2,2], [3,1]]
  # ```
  def prime_division(value, generator = Prime::Generator23.new); end

  # Returns true if `value` is a prime number, else returns false.
  #
  # ## Parameters
  #
  # `value`
  # :   an arbitrary integer to be checked.
  # `generator`
  # :   optional. A pseudo-prime generator.
  def self.prime?(value, generator = Prime::Generator23.new); end

  # Returns the factorization of `value`.
  #
  # ## Parameters
  # `value`
  # :   An arbitrary integer.
  # `generator`
  # :   Optional. A pseudo-prime generator. `generator`.succ must return the
  #     next pseudo-prime number in the ascending order. It must generate all
  #     prime numbers, but may also generate non prime numbers too.
  #
  #
  # ### Exceptions
  # `ZeroDivisionError`
  # :   when `value` is zero.
  #
  #
  # ## Example
  # For an arbitrary integer:
  #
  # ```
  # n = p_1**e_1 * p_2**e_2 * .... * p_n**e_n,
  # ```
  #
  # [`prime_division`](https://docs.ruby-lang.org/en/2.6.0/Prime.html#method-i-prime_division)(n)
  # returns:
  #
  # ```
  # [[p_1, e_1], [p_2, e_2], ...., [p_n, e_n]].
  #
  # Prime.prime_division(12) #=> [[2,2], [3,1]]
  # ```
  def self.prime_division(value, generator = Prime::Generator23.new); end
end

# An implementation of `PseudoPrimeGenerator`.
#
# Uses `EratosthenesSieve`.
class Prime::EratosthenesGenerator < ::Prime::PseudoPrimeGenerator
  Elem = type_member(:out)

  # Alias for:
  # [`succ`](https://docs.ruby-lang.org/en/2.6.0/Prime/EratosthenesGenerator.html#method-i-succ)
  def next; end

  def rewind; end

  # Also aliased as:
  # [`next`](https://docs.ruby-lang.org/en/2.6.0/Prime/EratosthenesGenerator.html#method-i-next)
  def succ; end

  def self.new; end
end

# Internal use. An implementation of Eratosthenes' sieve
class Prime::EratosthenesSieve
  include(::Singleton)
  extend(::Singleton::SingletonClassMethods)

  def get_nth_prime(n); end

  def self.new; end
end

# Generates all integers which are greater than 2 and are not divisible by
# either 2 or 3.
#
# This is a pseudo-prime generator, suitable on checking primality of an integer
# by brute force method.
class Prime::Generator23 < ::Prime::PseudoPrimeGenerator
  Elem = type_member(:out)

  # Alias for:
  # [`succ`](https://docs.ruby-lang.org/en/2.6.0/Prime/Generator23.html#method-i-succ)
  def next; end

  def rewind; end

  # Also aliased as:
  # [`next`](https://docs.ruby-lang.org/en/2.6.0/Prime/Generator23.html#method-i-next)
  def succ; end

  def self.new; end
end

# An abstract class for enumerating pseudo-prime numbers.
#
# Concrete subclasses should override succ, next, rewind.
class Prime::PseudoPrimeGenerator
  include(::Enumerable)

  Elem = type_member(:out)

  # Iterates the given block for each prime number.
  def each; end

  # alias of `succ`.
  def next; end

  # Rewinds the internal position for enumeration.
  #
  # See `Enumerator`#rewind.
  def rewind; end

  def size; end

  # returns the next pseudo-prime number, and move the internal position
  # forward.
  #
  # `PseudoPrimeGenerator`#succ raises `NotImplementedError`.
  def succ; end

  def upper_bound; end

  def upper_bound=(ubound); end

  # see `Enumerator`#with\_index.
  def with_index(offset = 0); end

  # see `Enumerator`#with\_object.
  def with_object(obj); end

  def self.new(ubound = nil); end
end

# Internal use. An implementation of prime table by trial division method.
class Prime::TrialDivision
  include(::Singleton)
  extend(::Singleton::SingletonClassMethods)

  # Returns the +index+th prime number.
  #
  # `index` is a 0-based index.
  def [](index); end

  def self.new; end
end

# An implementation of `PseudoPrimeGenerator` which uses a prime table generated
# by trial division.
class Prime::TrialDivisionGenerator < ::Prime::PseudoPrimeGenerator
  Elem = type_member(:out)

  # Alias for:
  # [`succ`](https://docs.ruby-lang.org/en/2.6.0/Prime/TrialDivisionGenerator.html#method-i-succ)
  def next; end

  def rewind; end

  # Also aliased as:
  # [`next`](https://docs.ruby-lang.org/en/2.6.0/Prime/TrialDivisionGenerator.html#method-i-next)
  def succ; end

  def self.new; end
end
