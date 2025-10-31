# typed: true

class Parent
  extend T::Sig
  sig {returns(T::Array[T.attached_class])}
  def self.make_many
    T.attached_class.to_s # error: Call to method `to_s` on `T.attached_class (of Parent)` mistakes a type for a value
    xs = T::Array[T.attached_class].new
    T.reveal_type(xs) # error: `T::Array[T.attached_class (of Parent)]`
    3.times do
      x = new
      T.reveal_type(x) # error: `T.attached_class (of Parent)`
      xs << x
      xs << 'nope' # error: Expected `T.attached_class (of Parent)` but found `String("nope")`
    end
    xs
  end
end

class Child < Parent; end

T.reveal_type(Parent.make_many) # error: Revealed type: `T::Array[Parent]`
T.reveal_type(Child.make_many) # error: Revealed type: `T::Array[Child]`

class Example; end

class B
  extend T::Sig
  sig {params(blk: T.proc.bind(T.class_of(Example)).void).void}
  def self.bind_example(&blk); end

  sig {params(blk: T.proc.bind(T.attached_class).void).void}
  def self.bind_attached(&blk); end

  sig {params(blk: T.proc.bind(T.self_type).void).void}
  def self.bind_self(&blk); end
end

B.bind_example do
  T.reveal_type(self) # error: `T.class_of(Example)`
  example = new
  T.reveal_type(example) # error: `Example`
  examples = T::Array[T.attached_class].new
  T.reveal_type(examples) # error: `T::Array[T.attached_class (of Example)]`
  examples << example
  #           ^^^^^^^ error: Expected `T.attached_class (of Example)` but found `Example`
  examples << 'nope'
  #           ^^^^^^ error: Expected `T.attached_class (of Example)` but found `String("nope")`
end

B.bind_attached do
  T.reveal_type(self) # error: `B`
  # There is a duplicate error here, because the resolver pass that looks at
  # `T.let` and the intrinsic report this error
  b = T.let(new, T.attached_class)
  #         ^^^ error: Method `new` does not exist
  #              ^^^^^^^^^^^^^^^^ error: `T.attached_class` may only be used in singleton methods on classes or instance methods on `has_attached_class!` modules
  #              ^^^^^^^^^^^^^^^^ error: `T.attached_class` may only be used in singleton methods on classes or instance methods on `has_attached_class!` modules

  puts(b)
  bs = T::Array[T.attached_class].new
  #             ^^^^^^^^^^^^^^^^ error: `T.attached_class` may only be used in singleton methods on classes or instance methods on `has_attached_class!` modules
  T.reveal_type(bs) # error: `T::Array[T.untyped]`
end

B.bind_self do
  T.reveal_type(self) # error: `T.class_of(B)`
  # The `T.let` type gets parsed in resolver (before T.proc.bind)
  b = T.let(new, T.attached_class)
  #              ^^^^^^^^^^^^^^^^ error: `T.attached_class` may only be used in singleton methods on classes or instance methods on `has_attached_class!` modules

  puts(b)
  # The `T.let` type here gets handled by calls.cc
  bs = T::Array[T.attached_class].new
  T.reveal_type(bs) # error: `T::Array[T.attached_class (of B)]`
end
