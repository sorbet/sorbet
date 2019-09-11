# typed: true
T.assert_type!(
  [1].flat_map {|x| x},
  T::Array[Integer],
)

T.assert_type!(
  [1].flat_map {|x| [x]},
  T::Array[Integer],
)

T.assert_type!(
  [1].flat_map {|x| [[x]]},
  T::Array[T::Array[Integer]],
)

T.assert_type!(
  [1].flat_map {|x| [[[x]]]},
  T::Array[T::Array[T::Array[Integer]]],
)

class A; end
class B; end
class C; end
class D; end
class E; end
class F; end
class G; end

T.assert_type!(
  [1].flat_map do |x|
    case rand
    when 0.1
      A.new
    when 0.2
      B.new
    when 0.3
      [C.new]
    when 0.4
      [D.new]
    when 0.5
      [E.new]
    when 0.6
      [F.new]
    else
      G.new
    end
  end,
  T::Array[T.any(A, B, C, D, E, F, G)],
)
