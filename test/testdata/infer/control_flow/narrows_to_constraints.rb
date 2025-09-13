# typed: true

class Super; end

class Self < Super
  extend T::Sig

  sig { returns(T::Boolean).narrows_to(String) }
  def test_unrelated_class
    false
  end

  sig { returns(T::Boolean).narrows_to(Super) }
  def test_superclass
    false
  end

  sig { returns(T::Boolean).narrows_to(Self) }
  def test_self
    false
  end

  sig { returns(T::Boolean).narrows_to(Sibling) }
  def test_sibling
    false
  end

  sig { returns(T::Boolean).narrows_to(Sub) }
  def test_subclass
    false
  end

  sig { returns(TrueClass).narrows_to(Self) }
  def test_trueclass
    true
  end

  sig { returns(FalseClass).narrows_to(Self) }
  def test_falseclass
    false
  end

  sig { returns(String).narrows_to(Self) } # error: Malformed `sig`: `narrows_to(Self)` can only be used with methods that return `T::Boolean`, `TrueClass`, or `FalseClass`, not `String
  def test_unrelated_return
    "foo"
  end

  sig { returns(T::Boolean).narrows_to(a: Self) } # error: `narrows_to` does not accept keyword arguments
  def test_kargs
    true
  end

  sig { returns(T::Boolean).narrows_to(Self, Sub) } # error: Too many arguments provided for method `T::Private::Methods::DeclBuilder#narrows_to`. Expected: `1`, got: `2`
  def test_multi_args
    true
  end

  sig { returns(T::Boolean).narrows_to } # error: Not enough arguments provided for method `T::Private::Methods::DeclBuilder#narrows_to`. Expected: `1`, got: `0`
  def test_no_args
    true
  end
end

class Sibling < Self; end

class Sub < Self; end
