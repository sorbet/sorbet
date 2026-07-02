# typed: true

module Root::B
  class Foo
    def uses_private_constant
      Root::Bar.new
    # ^^^^^^^^^ error: `Root::Bar` resolves but is not exported
    end
  end
end
