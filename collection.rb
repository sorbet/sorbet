# typed: true
class Module
  include T::Sig
  include T::Generic
end

# Extended example from this post:
# https://lptk.github.io/programming/2019/09/13/type-projection.html

module Collection
  abstract!
  E = type_member
  Index = type_member

  sig { abstract.params(idx: Index).returns(T.nilable(E)) }
  def get(idx); end

  sig { abstract.returns(T::Range[Index]) }
  def indices; end
end

module Sequence
  abstract!
  include Collection
  E = type_member
  Index = type_member { {fixed: Integer} }
end

module Mapping
  abstract!
  include Collection
  E = type_member { {fixed: V} }
  Index = type_member { {fixed: K} }
  K = type_member
  V = type_member
end

class ArraySequence < T::Struct
  include Sequence
  E = type_member
  Index = type_member { {fixed: Integer} }

  prop :underlying, T::Array[E]

  sig { override.params(idx: Index).returns(T.nilable(E)) }
  def get(idx) = underlying[idx]

  sig { override.returns(T::Range[Index]) }
  def indices = (0..underlying.size)
end

module MyMatrix
  abstract!
  include Collection
  E = type_member { {fixed: Float} }
  Index = type_member
  Cols = type_member { {upper: Collection[E, Index]} }
  Row = type_member { {fixed: Cols} }

  # Scala:
  #   type Position = (rows.Index, Cols#Index)
  # would be more like
  #   Position = type_member { {fixed: [Row::Index, Cols::Index]} }
  # because you can do type projection on an arbitrary type
  # whereas in Ruby you can only do :: on classes/modules
  Position = type_member { {fixed: [Index, Index]} }

  sig { abstract.returns(Collection[Row, Index]) }
  def rows; end

  sig { params(p: Position).returns(T.nilable(E)) }
  def get_position(p)
    row = rows.get(p[0])
    return nil unless row
    col = row.get(p[1])
    return col
  end
end

class DenseMatrix < T::Struct
  include MyMatrix
  E = type_member { {fixed: Float} }
  Index = type_member { {fixed: Integer} }
  Cols = type_member { {fixed: Sequence[Float]} }
  Row = type_member { {fixed: Cols} }

  Position = type_member { {fixed: [Index, Index]} }

  prop :rows, Sequence[Sequence[Float]]
  
  sig { override.params(idx: Index).returns(T.nilable(E)) }
  def get(idx)

  end
  sig { override.returns(T::Range[Index]) }
  def indices = (0..rows.size)
end  
  
# abstract class Collection[E] {
#   type Index
#   def get(idx: Index): Option[E]
#   def indices: Iterator[Index]
# }
# abstract class Sequence[E] extends Collection[E] {
#   type Index = Int
# }
# abstract class Mapping[K,V] extends Collection[V] {
#   type Index = K
# }

# // An example Sequence implementation:
# case class ArraySequence[E](underlying: Array[E]) extends Sequence[E] {
#   def get(idx: Index): Option[E] =
#     if (0 <= idx && idx < underlying.length) Some(underlying(idx))
#     else None
#   def indices = Iterator.range(0, underlying.length - 1)
# }

# // Type parameter Cols is required to be a subtype of Collection[Double]:
# abstract class Matrix[Cols <: Collection[Double]] {
#   type Row = Cols{type Index = Cols#Index}
#   val rows: Collection[Row]
#
#   // Type representing a position in the matrix
#   type Position = (rows.Index, Cols#Index)
#
#   def get(p: Position): Option[Double] =
#     rows.get(p._1).flatMap(r => r.get(p._2))
# }

# case class DenseMatrix(rows: Sequence[Sequence[Double]])
#   extends Matrix[Sequence[Double]]

# type SparseArr[E] = Mapping[BigInt, E]
#
# case class SparseMatrix(rows: SparseArr[SparseArr[Double]])
#   extends Matrix[SparseArr[Double]]

