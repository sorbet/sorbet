# typed: true

class A
  # There's no `T::Sig` in scope here, because the user forgot or didn't want
  # to put `extend T::Sig` (not required in RBI files), which means we can't
  # give autocompletion results.
  #
  # NOTE that codebases which have `class Module; include T::Sig; end`, are able
  # to get sig completion here

  sig
  #  ^ completion: (nothing)
  def foo; end
end

class B
  extend T::Sig

  sig
  #  ^ completion: sig
  #  ^ apply-completion: [A] item: 0
  def foo(x); end
end

class C
  sig {returns(T.noreturn)}
  #               ^ hover: For more information, see https://sorbet.org/docs/noreturn
  def always_raise; end
end

class D
  sig {returns(T.noret)} # error: Unsupported method `T.noret`
  #                   ^ completion: noreturn
  def foo; end
end
