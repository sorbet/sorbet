# typed: false

module A::B
  class C1
    def foo
      if # parser-error: unexpected token "if"
    end

    sig {void}
    private def bar; end
  end

  class C2
    def foo
      if x # parser-error: Hint: this "if" token might not be properly closed
    end

    sig {void}
    private def bar; end
  end
end # parser-error: unexpected token "end of file"
