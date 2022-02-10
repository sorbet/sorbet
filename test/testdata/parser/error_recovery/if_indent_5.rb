# typed: false

module A::B
  class C1
    def foo
      if # error: unexpected token "if"
    end

    sig {void}
    private def bar; end
  end

  class C2
    def foo
      if x # error: Hint: this "if" token might not be properly closed
    end

    sig {void}
    private def bar; end
  end
end # error: unexpected token "end of file"
