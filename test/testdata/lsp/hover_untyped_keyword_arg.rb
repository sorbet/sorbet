# typed: true

module Test
  def foo(bar: nil)
    #     ^^^ hover: T.untyped
  end
end
