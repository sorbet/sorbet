# frozen_string_literal: true
# typed: strict

class Root::A
  extend T::Sig

  sig {void}
  def main
    RBI::Foo.one
    GlobalFoo.foo
    RBI::Bar.one
    # Not exported, but also not owned by any package:
    GlobalBar.foo
  end
end
