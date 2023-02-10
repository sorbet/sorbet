# typed: strict
extend T::Sig

class Left
  extend T::Sig
  sig {void}
  private def foo; end
end

class Right
  extend T::Sig
  sig {void}
  def foo; end
end

sig {params(left_or_right: T.any(Left, Right)).void}
def test_left_or_right(left_or_right)
  left_or_right.foo # error: Non-private call to private method `foo` on `Left` component of `T.any(Left, Right)`
end

sig {params(right_or_left: T.any(Right, Left)).void}
def test_right_or_left(right_or_left)
  right_or_left.foo # error: Non-private call to private method `foo` on `Left` component of `T.any(Right, Left)`
end

module PrivateBar
  extend T::Sig
  sig {void}
  private def bar; end
end

module PublicQux
  extend T::Sig
  sig {void}
  def qux; end
end

sig {params(bar_and_qux: T.all(PrivateBar, PublicQux)).void}
def test_bar_and_qux(bar_and_qux)
  bar_and_qux.bar # error: Non-private call to private method `bar` on `PrivateBar` component of `T.all(PrivateBar, PublicQux)`
end

sig {params(qux_and_bar: T.all(PublicQux, PrivateBar)).void}
def test_qux_and_bar(qux_and_bar)
  qux_and_bar.bar # error: Non-private call to private method `bar` on `PrivateBar` component of `T.all(PublicQux, PrivateBar)`
end

