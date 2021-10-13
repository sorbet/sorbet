# typed: true

class ContainerType
  extend T::Sig

  sig { returns(T::Array[StringPair]) }
  def to_ary
    [StringPair.new, StringPair.new]
  end
end

class StringPair
  extend T::Sig

  sig { returns(T::Array[String]) }
  def to_ary
    ["1", "2"]
  end
end

T.reveal_type([1].flat_map {|x| x}) # error: Revealed type: `T::Array[Integer]`
T.reveal_type([1].flat_map {|x| [x]}) # error: Revealed type: `T::Array[Integer]`
T.reveal_type([1].flat_map {|x| [[x]]}) # error: Revealed type: `T::Array[[Integer]]`
T.reveal_type([1].flat_map {|x| [[[x]]]}) # error: Revealed type: `T::Array[[[Integer]]]`
T.reveal_type([1].flat_map {|x| ContainerType.new }) # error: Revealed type: `T::Array[StringPair]`

class A; end
class B; end
class C; end
class D; end
class E; end
class F; end
class G; end

T.reveal_type(
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
  end
) # error: Revealed type: `T::Array[T.any(A, B, C, D, E, F, G)]`
