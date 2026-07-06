# typed: true

module Root::B::Test
  class BTest
    def test_b
      Root::A.new.foo
    # ^^^^^^^^^^^^^^^ error: Method `foo` on `Root::A` is package-private
      Root::A.foo
    # ^^^^^^^^^^^ error: Method `foo` on `T.class_of(Root::A)` is package-private
    end
  end
end
