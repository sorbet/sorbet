# @typed

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
        bar(Foo) # error: does not match expected type
                 # TODO: RUBYPLAT-504
    end
end
