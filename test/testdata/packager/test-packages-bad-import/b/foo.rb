# typed: true

module Root::B
  class Foo
    def uses_private_constant
      # Not an error: `b/__package.rb` reports its `uses_internals: true`
      # as invalid, but error recovery still honors it.
      Root::Bar.new
    end
  end
end
