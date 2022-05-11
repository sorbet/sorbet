# frozen_string_literal: true
# typed: strict

class Root::A
  extend T::Sig

  sig {void}
  def main
    RBI::Foo.one
    GlobalFoo.foo
    RBI::Bar.one
    GlobalBar.foo # error: `GlobalBar` resolves but is not exported from `RBI`
  end
end
