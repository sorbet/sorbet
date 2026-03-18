# typed: true

class Test0
  class Parent
    extend T::Sig

    sig { params(id: Integer, block: T.proc.void).void }
    def demo(id: 123, &block)
    end
  end

  class Child < Parent
    sig { params(block: T.proc.void).void }
    def demo(&block)
      super(&block)
    end
  end
end

class Test1
  class Parent
    extend T::Sig

    sig { params(block: T.proc.void).void }
    def demo(&block); end
  end

  class Child < Parent
    sig { params(block: T.proc.void).void }
    def demo(&block) = super(&block)
  end
end

class Test2
  class Parent
    extend T::Sig

    sig { params(id: Integer, block: T.proc.void).void }
    def demo(id: 123, &block); end
  end

  class Child < Parent
    sig { params(block: T.proc.void).void }
    def demo(&block) = super(id: 123, &block)
  end
end

class Test3
  class Parent
    extend T::Sig

    sig { params(id: Integer, block: T.proc.void).void }
    def demo(id:, &block); end
  end

  class Child < Parent
    sig { params(block: T.proc.void).void }
    def demo(&block) = super(&block)
    #                        ^ error: Missing required keyword
  end
end

class Test4
  class Parent
    extend T::Sig

    sig { params(id: Integer, block: T.proc.void).void }
    def demo(id = 123, &block); end
  end

  class Child < Parent
    sig { params(block: T.proc.void).void }
    def demo(&block) = super(&block)
  end
end

class Test5
  class Parent
    extend T::Sig

    sig { params(id: Integer, block: T.proc.void).void }
    def demo(id: 123, &block); end
  end

  class Child < Parent
    sig { params(block: T.proc.void).void }
    def demo(&block) = super(123, &block)
    #                        ^^^ error: Too many positional
  end
end

class Test6
  class Parent
    extend T::Sig

    sig { params(id: Integer, block: T.proc.void).void }
    def demo1(id: 123, &block); end
  end

  class Child < Parent
    sig { params(block: T.proc.void).void }
    def demo2(&block) = demo1(&block)
  end
end

class Test7
  class Parent
    extend T::Sig

    sig { params(id: Integer).void }
    def demo(id: 123); end
  end

  class Child < Parent
    sig { void }
    def demo() = super()
  end
end

class Test8
  class Parent
    extend T::Sig

    sig { params(id: Integer).void }
    def demo(id: 123); end
  end

  class Child < Parent
    sig { void }
    def demo() = super
  end
end

class Test9
  class Parent
    extend T::Sig

    sig { params(id: Integer, block: T.proc.void).void }
    def demo(id: 123, &block); end
  end

  class Child < Parent
    sig { params(block: T.proc.void).void }
    def demo(&block) = super {}
  end
end
