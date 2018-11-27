# typed: true

module ExceptionForMatrix
  extend(Exception2MessageMapper)

  class ErrOperationNotDefined < ::StandardError

  end

  class ErrOperationNotImplemented < ::StandardError

  end

  class ErrDimensionMismatch < ::StandardError

  end

  class ErrNotRegular < ::StandardError

  end

  def Fail(err = _, *rest)
  end

  def Raise(err = _, *rest)
  end
end

class Matrix
  include(Matrix::CoercionHelper)
  include(ExceptionForMatrix)
  include(Enumerable)
  extend(Matrix::ConversionHelper)
  extend(Exception2MessageMapper)

  class LUPDecomposition
    include(Matrix::ConversionHelper)

    def p()
    end

    def singular?()
    end

    def determinant()
    end

    def to_ary()
    end

    def pivots()
    end

    def solve(b)
    end

    def det()
    end

    def l()
    end

    def to_a()
    end

    def u()
    end
  end

  module CoercionHelper

  end

  module ConversionHelper

  end

  SELECTORS = T.let(T.unsafe(nil), Hash)

  class Scalar < ::Numeric
    include(Matrix::CoercionHelper)
    include(ExceptionForMatrix)
    extend(Exception2MessageMapper)

    def /(other)
    end

    def Raise(err = _, *rest)
    end

    def **(other)
    end

    def *(other)
    end

    def +(other)
    end

    def Fail(err = _, *rest)
    end

    def -(other)
    end
  end

  class EigenvalueDecomposition
    def v_inv()
    end

    def to_ary()
    end

    def to_a()
    end

    def d()
    end

    def v()
    end

    def eigenvector_matrix()
    end

    def eigenvector_matrix_inv()
    end

    def eigenvalues()
    end

    def eigenvectors()
    end

    def eigenvalue_matrix()
    end
  end

  def *(m)
  end

  def +(m)
  end

  def transpose()
  end

  def -(m)
  end

  def /(other)
  end

  def coerce(other)
  end

  def row(i, &block)
  end

  def row_count()
  end

  def column_count()
  end

  def find_index(*args)
  end

  def real?()
  end

  def zero?()
  end

  def collect(&block)
  end

  def map(&block)
  end

  def rect()
  end

  def real()
  end

  def column(j)
  end

  def round(ndigits = _)
  end

  def vstack(*matrices)
  end

  def imaginary()
  end

  def hstack(*matrices)
  end

  def conjugate()
  end

  def combine(*matrices, &block)
  end

  def Raise(err = _, *rest)
  end

  def t()
  end

  def element(i, j)
  end

  def row_size()
  end

  def column_size()
  end

  def imag()
  end

  def each_with_index(which = _)
  end

  def trace()
  end

  def rectangular()
  end

  def minor(*param)
  end

  def conj()
  end

  def first_minor(row, column)
  end

  def cofactor(row, column)
  end

  def square?()
  end

  def +@()
  end

  def adjugate()
  end

  def laplace_expansion(row: _, column: _)
  end

  def **(other)
  end

  def cofactor_expansion(row: _, column: _)
  end

  def index(*args)
  end

  def diagonal?()
  end

  def lower_triangular?()
  end

  def normal?()
  end

  def hermitian?()
  end

  def determinant()
  end

  def ==(other)
  end

  def orthogonal?()
  end

  def [](i, j)
  end

  def regular?()
  end

  def permutation?()
  end

  def symmetric?()
  end

  def unitary?()
  end

  def singular?()
  end

  def empty?()
  end

  def eql?(other)
  end

  def upper_triangular?()
  end

  def inverse()
  end

  def hadamard_product(m)
  end

  def entrywise_product(m)
  end

  def inv()
  end

  def -@()
  end

  def eigensystem()
  end

  def det()
  end

  def component(i, j)
  end

  def inspect()
  end

  def determinant_e()
  end

  def det_e()
  end

  def rank()
  end

  def rank_e()
  end

  def eigen()
  end

  def each(which = _)
  end

  def lup()
  end

  def lup_decomposition()
  end

  def row_vectors()
  end

  def column_vectors()
  end

  def to_matrix()
  end

  def elements_to_f()
  end

  def elements_to_i()
  end

  def elements_to_r()
  end

  def tr()
  end

  def to_a()
  end

  def to_s()
  end

  def Fail(err = _, *rest)
  end

  def hash()
  end

  def clone()
  end
end
