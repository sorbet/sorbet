# typed: __STDLIB_INTERNAL

# # Trigonometric and transcendental functions for complex numbers.
#
# [`CMath`](https://docs.ruby-lang.org/en/2.6.0/CMath.html) is a library that
# provides trigonometric and transcendental functions for complex numbers. The
# functions in this module accept integers, floating-point numbers or complex
# numbers as arguments.
#
# Note that the selection of functions is similar, but not identical, to that in
# module math. The reason for having two modules is that some users aren't
# interested in complex numbers, and perhaps don't even know what they are. They
# would rather have
# [`Math.sqrt(-1)`](https://docs.ruby-lang.org/en/2.6.0/Math.html#method-c-sqrt)
# raise an exception than return a complex number.
#
# For more information you can see
# [`Complex`](https://docs.ruby-lang.org/en/2.6.0/Complex.html) class.
#
# ## Usage
#
# To start using this library, simply require cmath library:
#
# ```ruby
# require "cmath"
# ```
#
# # Trigonometric and transcendental functions for complex numbers.
#
# [`CMath`](https://docs.ruby-lang.org/en/2.6.0/CMath.html) is a library that
# provides trigonometric and transcendental functions for complex numbers. The
# functions in this module accept integers, floating-point numbers or complex
# numbers as arguments.
#
# Note that the selection of functions is similar, but not identical, to that in
# module math. The reason for having two modules is that some users aren't
# interested in complex numbers, and perhaps don't even know what they are. They
# would rather have
# [`Math.sqrt(-1)`](https://docs.ruby-lang.org/en/2.6.0/Math.html#method-c-sqrt)
# raise an exception than return a complex number.
#
# For more information you can see
# [`Complex`](https://docs.ruby-lang.org/en/2.6.0/Complex.html) class.
#
# ## Usage
#
# To start using this library, simply require cmath library:
#
# ```ruby
# require "cmath"
# ```
#
# # Trigonometric and transcendental functions for complex numbers.
#
# [`CMath`](https://docs.ruby-lang.org/en/2.6.0/CMath.html) is a library that
# provides trigonometric and transcendental functions for complex numbers. The
# functions in this module accept integers, floating-point numbers or complex
# numbers as arguments.
#
# Note that the selection of functions is similar, but not identical, to that in
# module math. The reason for having two modules is that some users aren't
# interested in complex numbers, and perhaps don't even know what they are. They
# would rather have
# [`Math.sqrt(-1)`](https://docs.ruby-lang.org/en/2.6.0/Math.html#method-c-sqrt)
# raise an exception than return a complex number.
#
# For more information you can see
# [`Complex`](https://docs.ruby-lang.org/en/2.6.0/Complex.html) class.
#
# ## Usage
#
# To start using this library, simply require cmath library:
#
# ```ruby
# require "cmath"
# ```
#
# # Trigonometric and transcendental functions for complex numbers.
#
# [`CMath`](https://docs.ruby-lang.org/en/2.6.0/CMath.html) is a library that
# provides trigonometric and transcendental functions for complex numbers. The
# functions in this module accept integers, floating-point numbers or complex
# numbers as arguments.
#
# Note that the selection of functions is similar, but not identical, to that in
# module math. The reason for having two modules is that some users aren't
# interested in complex numbers, and perhaps don't even know what they are. They
# would rather have
# [`Math.sqrt(-1)`](https://docs.ruby-lang.org/en/2.6.0/Math.html#method-c-sqrt)
# raise an exception than return a complex number.
#
# For more information you can see
# [`Complex`](https://docs.ruby-lang.org/en/2.6.0/Complex.html) class.
#
# ## Usage
#
# To start using this library, simply require cmath library:
#
# ```ruby
# require "cmath"
# ```
module CMath
  include(::Math)

  # Returns the arc cosine of `z`
  #
  # ```ruby
  # CMath.acos(1 + 1i) #=> (0.9045568943023813-1.0612750619050357i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.acos(z); end

  # returns the inverse hyperbolic cosine of `z`
  #
  # ```ruby
  # CMath.acosh(1 + 1i) #=> (1.0612750619050357+0.9045568943023813i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.acosh(z); end

  # Returns the arc sine of `z`
  #
  # ```ruby
  # CMath.asin(1 + 1i) #=> (0.6662394324925153+1.0612750619050355i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.asin(z); end

  # returns the inverse hyperbolic sine of `z`
  #
  # ```ruby
  # CMath.asinh(1 + 1i) #=> (1.0612750619050357+0.6662394324925153i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.asinh(z); end

  # Returns the arc tangent of `z`
  #
  # ```ruby
  # CMath.atan(1 + 1i) #=> (1.0172219678978514+0.4023594781085251i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.atan(z); end

  # returns the arc tangent of `y` divided by `x` using the signs of `y` and `x`
  # to determine the quadrant
  #
  # ```ruby
  # CMath.atan2(1 + 1i, 0) #=> (1.5707963267948966+0.0i)
  # ```
  sig { params(y: Numeric, x: Numeric).returns(Float) }
  def self.atan2(y, x); end

  # returns the inverse hyperbolic tangent of `z`
  #
  # ```ruby
  # CMath.atanh(1 + 1i) #=> (0.4023594781085251+1.0172219678978514i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.atanh(z); end

  # Returns the principal value of the cube root of `z`
  #
  # ```ruby
  # CMath.cbrt(1 + 4i) #=> (1.449461632813119+0.6858152562177092i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.cbrt(z); end

  # Returns the cosine of `z`, where `z` is given in radians
  #
  # ```ruby
  # CMath.cos(1 + 1i) #=> (0.8337300251311491-0.9888977057628651i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.cos(z); end

  # Returns the hyperbolic cosine of `z`, where `z` is given in radians
  #
  # ```ruby
  # CMath.cosh(1 + 1i) #=> (0.8337300251311491+0.9888977057628651i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.cosh(z); end

  # Math::E raised to the `z` power
  #
  # ```ruby
  # CMath.exp(1.i * Math::PI) #=> (-1.0+1.2246467991473532e-16i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.exp(z); end

  # Returns the natural logarithm of
  # [`Complex`](https://docs.ruby-lang.org/en/2.6.0/Complex.html). If a second
  # argument is given, it will be the base of logarithm.
  #
  # ```ruby
  # CMath.log(1 + 4i)     #=> (1.416606672028108+1.3258176636680326i)
  # CMath.log(1 + 4i, 10) #=> (0.6152244606891369+0.5757952953408879i)
  # ```
  sig { params(z: Numeric, b: Numeric).returns(Float) }
  def self.log(z, b = ::Math::E); end

  # Returns the base 10 logarithm of `z`
  #
  # ```ruby
  # CMath.log10(-1) #=> (0.0+1.3643763538418412i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.log10(z); end

  # Returns the base 2 logarithm of `z`
  #
  # ```
  # CMath.log2(-1) => (0.0+4.532360141827194i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.log2(z); end

  # Returns the sine of `z`, where `z` is given in radians
  #
  # ```ruby
  # CMath.sin(1 + 1i) #=> (1.2984575814159773+0.6349639147847361i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.sin(z); end

  # Returns the hyperbolic sine of `z`, where `z` is given in radians
  #
  # ```ruby
  # CMath.sinh(1 + 1i) #=> (0.6349639147847361+1.2984575814159773i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.sinh(z); end

  # Returns the non-negative square root of
  # [`Complex`](https://docs.ruby-lang.org/en/2.6.0/Complex.html).
  #
  # ```ruby
  # CMath.sqrt(-1 + 0i) #=> 0.0+1.0i
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.sqrt(z); end

  # Returns the tangent of `z`, where `z` is given in radians
  #
  # ```ruby
  # CMath.tan(1 + 1i) #=> (0.27175258531951174+1.0839233273386943i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.tan(z); end

  # Returns the hyperbolic tangent of `z`, where `z` is given in radians
  #
  # ```ruby
  # CMath.tanh(1 + 1i) #=> (1.0839233273386943+0.27175258531951174i)
  # ```
  sig { params(z: Numeric).returns(Float) }
  def self.tanh(z); end
end
