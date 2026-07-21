# typed: true

module Root::B
  class Foo
    def uses_private_constant
      Root::Bar.new
    end
  end
end
