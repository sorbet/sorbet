# typed: true
class Module; include T::Sig; end

module A
  sig {
    params(blk: T.proc.returns(T.anything))
      .returns(IFoo)
  }
  def foo(&blk)
    Foo.new
  end
end

module IFoo; end
class Foo
  include IFoo
end

module B
  sig {
    type_parameters(:U)
      .params(blk: T.proc.returns(T.type_parameter(:U)))
      .returns(T.type_parameter(:U))
  }
  def foo(&blk)
    yield
  end
end

# NOTE: At time of writing, this suffers from this bug:
# https://github.com/sorbet/sorbet/issues/5409

sig { params(x: T.any(A, B)).void }
def example_any(x)
  res = x.foo {
    #      ^ hover-line: 2 # A#foo:
    #      ^ hover-line: 3 sig { params(blk: T.proc.returns(T.anything)).returns(IFoo) }
    #      ^ hover-line: 4 def foo(&blk); end
    #      ^ hover-line: 5 # B#foo:
    #      ^ hover-line: 6 sig do
    #      ^ hover-line: 7   type_parameters(:U)
    #      ^ hover-line: 8   .params(
    #      ^ hover-line: 9     blk: T.proc.returns(T.type_parameter(:U))
    #      ^ hover-line: 10   )
    #      ^ hover-line: 11   .returns(T.type_parameter(:U))
    #      ^ hover-line: 12 end
    #      ^ hover-line: 13 def foo(&blk); end
    #      ^ hover-line: 15 # result type:
    #      ^ hover-line: 16 IFoo
    ""
  }
  # WRONG!
  T.reveal_type(res) # error: `IFoo`
end

sig { params(x: T.all(A, B)).void }
def example_all(x)
  res = x.foo {
    #     ^ hover-line: 2 # A#foo:
    #     ^ hover-line: 3 sig { params(blk: T.proc.returns(T.anything)).returns(IFoo) }
    #     ^ hover-line: 4 def foo(&blk); end
    #     ^ hover-line: 5 # B#foo:
    #     ^ hover-line: 6 sig do
    #     ^ hover-line: 7   type_parameters(:U)
    #     ^ hover-line: 8   .params(
    #     ^ hover-line: 9     blk: T.proc.returns(T.type_parameter(:U))
    #     ^ hover-line: 10   )
    #     ^ hover-line: 11   .returns(T.type_parameter(:U))
    #     ^ hover-line: 12 end
    #     ^ hover-line: 13 def foo(&blk); end
    #     ^ hover-line: 15 # result type:
    #     ^ hover-line: 16 IFoo
    ""
  }
  # WRONG!
  T.reveal_type(res) # error: `IFoo`
end
