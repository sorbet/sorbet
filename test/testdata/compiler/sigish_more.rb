# frozen_string_literal: true
# typed: true
# compiled: true

class DeclBuilder
  def params(params)
    self
  end

  def void
    self
  end
end

module Sigish
  T::Sig::WithoutRuntime.sig {params(blk: T.proc.bind(DeclBuilder).params(arg0: T.untyped).void).void}
  def sig(&blk)
    DeclBuilder.new.instance_exec(&blk)
  end
end

class A
  extend Sigish

  sig {params(a: Integer).void}
  def foo(a)
    puts a
    a
  end
end

A.new.foo(1)

