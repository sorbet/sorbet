# typed: strict

module Root::A
  module Sub
    # Resolves through the lexical scope `Root::A`, without naming any package-private component.
    SECRET
  # ^^^^^^ error: `Root::A::SECRET` is `package_private!` in package `Root::A`
  end
end
