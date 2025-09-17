# typed: true

class A
  def foo; @foo; end

  def foo=(foo)
    @foo = foo
  end
end

a = A.new
a.fooo = 12
# ^^^^ error: Setter method `fooo=` does not exist on `A`

a.fooo
# ^^^^ error: Method `fooo` does not exist on `A`
