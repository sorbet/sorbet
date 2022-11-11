# typed: true
require "matrix"

class Foo
  extend T::Sig

  sig { params(matrix: Matrix).void }
  def initialize(matrix:)
    @matrix = matrix
  end
end
