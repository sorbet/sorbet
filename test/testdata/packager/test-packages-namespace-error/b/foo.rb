# typed: true

module Root::B
  class Foo
    RA = Root::A
    #    ^^^^^^^ error: `Root::A` is defined in a test namespace
  end
end
