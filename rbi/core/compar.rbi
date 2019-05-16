# typed: core

module Comparable
  sig { params(other: T.untyped).returns(T::Boolean) }
  def <(other); end

  sig { params(other: T.untyped).returns(T::Boolean) }
  def <=(other); end

  sig { params(other: T.untyped).returns(T::Boolean) }
  def ==(other); end

  sig { params(other: T.untyped).returns(T::Boolean) }
  def >(other); end

  sig { params(other: T.untyped).returns(T::Boolean) }
  def >=(other); end

  sig { params(min: T.untyped, max: T.untyped).returns(T::Boolean) }
  def between?(min, max); end

  sig { params(min: T.untyped, max: T.untyped).returns(T.untyped) }
  def clamp(min, max); end
end
