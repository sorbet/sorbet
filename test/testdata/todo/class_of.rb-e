# typed: true

module Mixin; end
class Foo
    include Mixin;
end

class Main
    extend T::Sig

    sig {params(a: T.class_of(Mixin)).returns(T.class_of(Mixin))}
    def bar(a)
        a
    end

    def main
        bar(Mixin)
        bar(Foo) # error: Expected `T.class_of(Mixin)` but found `T.class_of(Foo)` for argument `a`
                 # TODO: RUBYPLAT-504
    end
end
