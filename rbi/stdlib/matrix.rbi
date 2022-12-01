# typed: __STDLIB_INTERNAL

# The `Vector` class represents a mathematical vector, which is useful in its
# own right, and also constitutes a row or column of a
# [`Matrix`](https://docs.ruby-lang.org/en/2.7.0/Matrix.html).
#
# ## [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) Catalogue
#
# To create a Vector:
# *   [`Vector.[](*array)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-c-5B-5D)
# *   [`Vector.elements`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-c-elements)(array,
#     copy = true)
# *   [`Vector.basis`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-c-basis)(size:
#     n, index: k)
# *   [`Vector.zero(n)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-c-zero)
#
#
# To access elements:
# *   [`[](i)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-5B-5D)
#
#
# To set elements:
# *   [`[]=`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-5B-5D-3D)(i,
#     v)
#
#
# To enumerate the elements:
# *   [`each2(v)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-each2)
# *   [`collect2(v)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-collect2)
#
#
# Properties of vectors:
# *   [`angle_with(v)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-angle_with)
# *   [`Vector.independent?(*vs)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-c-independent-3F)
# *   [`independent?(*vs)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-independent-3F)
# *   [`zero?`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-zero-3F)
#
#
# [`Vector`](https://docs.ruby-lang.org/en/2.7.0/Vector.html) arithmetic:
# *   [`*(x)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-2A) "is
#     matrix or number"
# *   [`+(v)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-2B)
# *   [`-(v)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-2D)
# *   #/(v)
# *   [`+`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-2B)@
# *   [`-`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-2D)@
#
#
# [`Vector`](https://docs.ruby-lang.org/en/2.7.0/Vector.html) functions:
# *   [`inner_product(v)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-inner_product),
#     dot(v)
# *   [`cross_product(v)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-cross_product),
#     cross(v)
# *   [`collect`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-collect)
# *   [`collect!`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-collect-21)
# *   [`magnitude`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-magnitude)
# *   [`map`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-map)
# *   [`map!`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-map-21)
# *   [`map2(v)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-map2)
# *   [`norm`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-norm)
# *   [`normalize`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-normalize)
# *   [`r`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-r)
# *   [`round`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-round)
# *   [`size`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-size)
#
#
# Conversion to other data types:
# *   [`covector`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-covector)
# *   [`to_a`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-to_a)
# *   [`coerce(other)`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-coerce)
#
#
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) representations:
# *   [`to_s`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-to_s)
# *   [`inspect`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-inspect)
class Vector
  include ::Enumerable

  Elem = type_member {{ fixed: T.untyped }}

  # [`Vector.new`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-c-new)
  # is private; use Vector[] or
  # [`Vector.elements`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-c-elements)
  # to create.
  def self.new(array); end

  # Creates a [`Vector`](https://docs.ruby-lang.org/en/2.7.0/Vector.html) from a
  # list of elements.
  #
  # ```
  # Vector[7, 4, ...]
  # ```
  def self.[](*array); end

  # Returns a standard basis `n`-vector, where k is the index.
  #
  # ```
  # Vector.basis(size:, index:) # => Vector[0, 1, 0]
  # ```
  def self.basis(size:, index:); end

  # Creates a vector from an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html). The optional
  # second argument specifies whether the array itself or a copy is used
  # internally.
  def self.elements(array, copy = true); end

  # Returns `true` iff all of vectors are linearly independent.
  #
  # ```
  # Vector.independent?(Vector[1,0], Vector[0,1])
  #   => true
  #
  # Vector.independent?(Vector[1,2], Vector[2,4])
  #   => false
  # ```
  def self.independent?(*vs); end

  # Return a zero vector.
  #
  # ```
  # Vector.zero(3) => Vector[0, 0, 0]
  # ```
  def self.zero(size); end

  # Multiplies the vector by `x`, where `x` is a number or a matrix.
  def *(x); end

  # [`Vector`](https://docs.ruby-lang.org/en/2.7.0/Vector.html) addition.
  def +(x); end

  def +@; end

  # [`Vector`](https://docs.ruby-lang.org/en/2.7.0/Vector.html) subtraction.
  def -(x); end

  def -@; end

  # [`Vector`](https://docs.ruby-lang.org/en/2.7.0/Vector.html) division.
  def /(x); end

  # Returns `true` iff the two vectors have the same elements in the same order.
  def ==(other); end

  # Returns an angle with another vector. Result is within the [0..Math::PI].
  #
  # ```ruby
  # Vector[1,0].angle_with(Vector[0,1])
  # # => Math::PI / 2
  # ```
  def angle_with(v); end

  def clone(); end

  # The coerce method provides support for Ruby type coercion. This coercion
  # mechanism is used by Ruby to handle mixed-type numeric operations: it is
  # intended to find a compatible common type between the two operands of the
  # operator. See also
  # [`Numeric#coerce`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-coerce).
  def coerce(other); end

  # Like
  # [`Array#collect`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-collect).
  #
  # Also aliased as:
  # [`map`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-map)
  def collect(); end

  # Collects (as in
  # [`Enumerable#collect`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-collect))
  # over the elements of this vector and `v` in conjunction.
  def collect2(v); end

  # Alias for:
  # [`[]`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-5B-5D)
  def component(i); end

  def convector(); end

  # Alias for:
  # [`cross_product`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-cross_product)
  def cross(*vs); end

  # Returns the cross product of this vector with the others.
  #
  # ```ruby
  # Vector[1, 0, 0].cross_product Vector[0, 1, 0]   => Vector[0, 0, 1]
  # ```
  #
  # It is generalized to other dimensions to return a vector perpendicular to
  # the arguments.
  #
  # ```ruby
  # Vector[1, 2].cross_product # => Vector[-2, 1]
  # Vector[1, 0, 0, 0].cross_product(
  #    Vector[0, 1, 0, 0],
  #    Vector[0, 0, 1, 0]
  # )  #=> Vector[0, 0, 0, 1]
  # ```
  #
  #
  # Also aliased as:
  # [`cross`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-cross)
  def cross_product(*vs); end

  # Alias for:
  # [`inner_product`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-inner_product)
  def dot(v); end

  # Iterate over the elements of this vector
  def each(&block); end

  # Iterate over the elements of this vector and `v` in conjunction.
  def each2(v); end

  # Alias for:
  # [`[]`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-5B-5D)
  def element(i); end

  def elements_to_f(); end

  def elements_to_i(); end

  def elements_to_r(); end

  def eql?(other); end

  # Returns a hash-code for the vector.
  def hash(); end

  # Returns `true` iff all of vectors are linearly independent.
  #
  # ```
  # Vector[1,0].independent?(Vector[0,1])
  #   => true
  #
  # Vector[1,2].independent?(Vector[2,4])
  #   => false
  # ```
  def independent?(*vs); end

  # Returns the inner product of this vector with the other.
  #
  # ```ruby
  # Vector[4,7].inner_product Vector[10,1]  => 47
  # ```
  #
  #
  # Also aliased as:
  # [`dot`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-dot)
  def inner_product(v); end

  # Overrides
  # [`Object#inspect`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-inspect)
  def inspect(); end

  # Returns the modulus (Pythagorean distance) of the vector.
  #
  # ```
  # Vector[5,8,2].r => 9.643650761
  # ```
  #
  #
  # Also aliased as:
  # [`r`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-r),
  # [`norm`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-norm)
  def magnitude(); end

  # Alias for:
  # [`collect`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-collect)
  def map(); end

  # Like
  # [`Vector#collect2`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-collect2),
  # but returns a [`Vector`](https://docs.ruby-lang.org/en/2.7.0/Vector.html)
  # instead of an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
  def map2(v); end

  # Alias for:
  # [`magnitude`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-magnitude)
  def norm(); end

  # Returns a new vector with the same direction but with norm 1.
  #
  # ```
  # v = Vector[5,8,2].normalize
  # # => Vector[0.5184758473652127, 0.8295613557843402, 0.20739033894608505]
  # v.norm => 1.0
  # ```
  def normalize(); end

  # Alias for:
  # [`magnitude`](https://docs.ruby-lang.org/en/2.7.0/Vector.html#method-i-magnitude)
  def r(); end

  # Returns a vector with entries rounded to the given precision (see
  # [`Float#round`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-round))
  def round(ndigits = 0); end

  # Returns the number of elements in the vector.
  def size(); end

  # Returns the elements of the vector in an array.
  def to_a(); end

  # Return a single-column matrix from this vector
  def to_matrix(); end

  # Overrides
  # [`Object#to_s`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-to_s)
  def to_s(); end

  # Returns `true` iff all elements are zero.
  def zero?(); end
end

# The `Matrix` class represents a mathematical matrix. It provides methods for
# creating matrices, operating on them arithmetically and algebraically, and
# determining their mathematical properties such as trace, rank, inverse,
# determinant, or eigensystem.
class Matrix
  include ::Enumerable

  Elem = type_member {{ fixed: T.untyped }}

  # Yields all elements of the matrix, starting with those of the first row, or
  # returns an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) if no
  # block given. Elements can be restricted by passing an argument:
  # *   :all (default): yields all elements
  # *   :diagonal: yields only elements on the diagonal
  # *   :off\_diagonal: yields all elements except on the diagonal
  # *   :lower: yields only elements on or below the diagonal
  # *   :strict\_lower: yields only elements below the diagonal
  # *   :strict\_upper: yields only elements above the diagonal
  # *   :upper: yields only elements on or above the diagonal
  #
  #     Matrix[ [1,2], [3,4] ].each { |e| puts e }
  #
  # ```ruby
  # # => prints the numbers 1 to 4
  # ```
  #
  #     Matrix[ [1,2], [3,4] ].each(:strict\_lower).to\_a # => [3]
  def each(which=:all, &block); end

  # [`Matrix.new`](https://docs.ruby-lang.org/en/2.7.0/Matrix.html#method-c-new)
  # is private; use
  # [`Matrix.rows`](https://docs.ruby-lang.org/en/2.7.0/Matrix.html#method-c-rows),
  # columns, [], etc... to create.
  def self.new(rows, column_count = rows[0].size); end

  # Alias for:
  # [`identity`](https://docs.ruby-lang.org/en/2.7.0/Matrix.html#method-c-identity)
  def self.I(n); end

  # Creates a matrix where each argument is a row.
  #
  # ```
  # Matrix[ [25, 93], [-1, 66] ]
  #    =>  25 93
  #        -1 66
  # ```
  def self.[](*rows); end

  # Creates a matrix of size `row_count` x `column_count`. It fills the values
  # by calling the given block, passing the current row and column. Returns an
  # enumerator if no block is given.
  #
  # ```
  # m = Matrix.build(2, 4) {|row, col| col - row }
  #   => Matrix[[0, 1, 2, 3], [-1, 0, 1, 2]]
  # m = Matrix.build(3) { rand }
  #   => a 3x3 matrix with random elements
  # ```
  def self.build(row_count, column_count = row_count); end

  # Creates a single-column matrix where the values of that column are as given
  # in `column`.
  #
  # ```
  # Matrix.column_vector([4,5,6])
  #   => 4
  #      5
  #      6
  # ```
  def self.column_vector(column); end

  # Creates a matrix using `columns` as an array of column vectors.
  #
  # ```
  # Matrix.columns([[25, 93], [-1, 66]])
  #    =>  25 -1
  #        93 66
  # ```
  def self.columns(columns); end

  # Create a matrix by combining matrices entrywise, using the given block
  #
  # ```ruby
  # x = Matrix[[6, 6], [4, 4]]
  # y = Matrix[[1, 2], [3, 4]]
  # Matrix.combine(x, y) {|a, b| a - b} # => Matrix[[5, 4], [1, 0]]
  # ```
  def self.combine(*matrices); end

  # Creates a matrix where the diagonal elements are composed of `values`.
  #
  # ```
  # Matrix.diagonal(9, 5, -3)
  #   =>  9  0  0
  #       0  5  0
  #       0  0 -3
  # ```
  def self.diagonal(*values); end

  # Creates a empty matrix of `row_count` x `column_count`. At least one of
  # `row_count` or `column_count` must be 0.
  #
  # ```
  # m = Matrix.empty(2, 0)
  # m == Matrix[ [], [] ]
  #   => true
  # n = Matrix.empty(0, 3)
  # n == Matrix.columns([ [], [], [] ])
  #   => true
  # m * n
  #   => Matrix[[0, 0, 0], [0, 0, 0]]
  # ```
  def self.empty(row_count = 0,  column_count = 0); end

  # Create a matrix by stacking matrices horizontally
  #
  # ```ruby
  # x = Matrix[[1, 2], [3, 4]]
  # y = Matrix[[5, 6], [7, 8]]
  # Matrix.hstack(x, y) # => Matrix[[1, 2, 5, 6], [3, 4, 7, 8]]
  # ```
  def self.hstack(x, *matrices); end

  # Creates an `n` by `n` identity matrix.
  #
  # ```
  # Matrix.identity(2)
  #   => 1 0
  #      0 1
  # ```
  #
  #
  # Also aliased as:
  # [`unit`](https://docs.ruby-lang.org/en/2.7.0/Matrix.html#method-c-unit),
  # [`I`](https://docs.ruby-lang.org/en/2.7.0/Matrix.html#method-c-I)
  def self.identity(n); end

  # Creates a single-row matrix where the values of that row are as given in
  # `row`.
  #
  # ```
  # Matrix.row_vector([4,5,6])
  #   => 4 5 6
  # ```
  def self.row_vector(row); end

  # Creates a matrix where `rows` is an array of arrays, each of which is a row
  # of the matrix. If the optional argument `copy` is false, use the given
  # arrays as the internal structure of the matrix without copying.
  #
  # ```
  # Matrix.rows([[25, 93], [-1, 66]])
  #    =>  25 93
  #        -1 66
  # ```
  def self.rows(rows, copy = true); end

  # Creates an `n` by `n` diagonal matrix where each diagonal element is
  # `value`.
  #
  # ```
  # Matrix.scalar(2, 5)
  #   => 5 0
  #      0 5
  # ```
  def self.scalar(n, value); end

  # Alias for:
  # [`identity`](https://docs.ruby-lang.org/en/2.7.0/Matrix.html#method-c-identity)
  def self.unit(n); end

  # Create a matrix by stacking matrices vertically
  #
  # ```ruby
  # x = Matrix[[1, 2], [3, 4]]
  # y = Matrix[[5, 6], [7, 8]]
  # Matrix.vstack(x, y) # => Matrix[[1, 2], [3, 4], [5, 6], [7, 8]]
  # ```
  def self.vstack(x, *matrices); end

  # Creates a zero matrix.
  #
  # ```
  # Matrix.zero(2)
  #   => 0 0
  #      0 0
  # ```
  def self.zero(row_count, column_couint = row_count); end

  # Create a matrix by combining matrices entrywise, using the given block
  #
  # ```ruby
  # x = Matrix[[6, 6], [4, 4]]
  # y = Matrix[[1, 2], [3, 4]]
  # Matrix.combine(x, y) {|a, b| a - b} # => Matrix[[5, 4], [1, 0]]
  # ```
  def self.combine(*matrices, &block); end
end

# Eigenvalues and eigenvectors of a real matrix.
#
# Computes the eigenvalues and eigenvectors of a matrix A.
#
# If A is diagonalizable, this provides matrices V and D such that A =
# V\*D\*V.inv, where D is the diagonal matrix with entries equal to the
# eigenvalues and V is formed by the eigenvectors.
#
# If A is symmetric, then V is orthogonal and thus A = V\*D\*V.t
class Matrix::EigenvalueDecomposition
  # Constructs the eigenvalue decomposition for a square matrix `A`
  def self.new(a); end

  # Alias for:
  # [`eigenvalue_matrix`](https://docs.ruby-lang.org/en/2.7.0/Matrix/EigenvalueDecomposition.html#method-i-eigenvalue_matrix)
  def d; end

  # Returns the block diagonal eigenvalue matrix `D`
  #
  # Also aliased as:
  # [`d`](https://docs.ruby-lang.org/en/2.7.0/Matrix/EigenvalueDecomposition.html#method-i-d)
  def eigenvalue_matrix; end

  # Returns the eigenvalues in an array
  def eigenvalues; end

  # Returns the eigenvector matrix `V`
  #
  # Also aliased as:
  # [`v`](https://docs.ruby-lang.org/en/2.7.0/Matrix/EigenvalueDecomposition.html#method-i-v)
  def eigenvector_matrix; end

  # Returns the inverse of the eigenvector matrix `V`
  #
  # Also aliased as:
  # [`v_inv`](https://docs.ruby-lang.org/en/2.7.0/Matrix/EigenvalueDecomposition.html#method-i-v_inv)
  def eigenvector_matrix_inv; end

  # Returns an array of the eigenvectors
  def eigenvectors; end

  # Alias for:
  # [`to_ary`](https://docs.ruby-lang.org/en/2.7.0/Matrix/EigenvalueDecomposition.html#method-i-to_ary)
  def to_a; end

  # Returns [eigenvector\_matrix,
  # [`eigenvalue_matrix`](https://docs.ruby-lang.org/en/2.7.0/Matrix/EigenvalueDecomposition.html#method-i-eigenvalue_matrix),
  # [`eigenvector_matrix_inv`](https://docs.ruby-lang.org/en/2.7.0/Matrix/EigenvalueDecomposition.html#method-i-eigenvector_matrix_inv)]
  #
  # Also aliased as:
  # [`to_a`](https://docs.ruby-lang.org/en/2.7.0/Matrix/EigenvalueDecomposition.html#method-i-to_a)
  def to_ary; end

  # Alias for:
  # [`eigenvector_matrix`](https://docs.ruby-lang.org/en/2.7.0/Matrix/EigenvalueDecomposition.html#method-i-eigenvector_matrix)
  def v; end

  # Alias for:
  # [`eigenvector_matrix_inv`](https://docs.ruby-lang.org/en/2.7.0/Matrix/EigenvalueDecomposition.html#method-i-eigenvector_matrix_inv)
  def v_inv; end
end
