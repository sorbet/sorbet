# typed: true
extend T::Sig

module Left; end
module Right; end
class Parent; end

sig {params(x: T.all(T.class_of(Parent), T::Class[Left])).void}
def example1(x)
  T.reveal_type(x) # error: `T.class_of(Parent)[T.all(Parent, Left)]`
end

sig {params(x: T.all(T.class_of(Parent), T::Class[Right])).void}
def example2(x)
  T.reveal_type(x) # error: `T.class_of(Parent)[T.all(Parent, Right)]`
  example1(x)
  #        ^ error: `T.class_of(Parent)[T.all(Parent, Left)]` but found `T.class_of(Parent)[T.all(Parent, Right)]` for argument `x`
end
