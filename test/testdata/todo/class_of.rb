# typed: strict

module Mixin; end
class Foo
    include Mixin;
end

class Main
    sig(a: T.class_of(Mixin)).returns(T.class_of(Mixin))
    def bar(a)
        a
    end

    def main
        bar(Mixin)
        bar(Foo) # error: `<Class:Foo>` doesn't match `<Class:Mixin>` for argument `a`
                 # TODO: RUBYPLAT-504
    end
end
