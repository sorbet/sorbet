# frozen_string_literal: true

# typed: true

class Foo
  extend T::Sig
  prop :bar, String, factory: -> { Bar.new }
# ^^^^ hover: sig {returns(String)}
#                    ^^^^^^^ hover: Symbol(:"factory")
#                                  ^^^ hover: T.class_of(Foo::Bar)
#                                  ^^^ usage: bar_class
#                                      ^^^ hover: def initialize; end
#                                      ^^^ usage: bar_class_initialize

  class Bar
#       ^^^ def: bar_class
    def initialize
#       ^^^^^^^^^^ def: bar_class_initialize
    end
  end
end
