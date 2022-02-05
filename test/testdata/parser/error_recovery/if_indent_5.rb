# typed: false

module A::B
  class C1
    def foo
      if
    end # error: unexpected token "end"

    sig {void}
    private def bar; end
  end

  class C2
    def foo
      if x
    end # error: Closing "end" token was not indented as far as "if" token

    sig {void}
    private def bar; end
  end
end

