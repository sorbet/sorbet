# typed: true

class A
  extend T::Sig
  sig {returns(T::Array[T.attached_class])}
  def self.make_many
    xs = T::Array[T.attached_class].new
    T.reveal_type(xs) # error: `T::Array[T.attached_class (of A)]`
    3.times do
      x = new
      T.reveal_type(x) # error: `T.attached_class (of A)`
      xs << x
      xs << 'nope' # error: Expected `T.attached_class (of A)` but found `String("nope")`
    end
    xs
  end
end

class Example; end

class B
  extend T::Sig
  sig {params(blk: T.proc.bind(T.class_of(Example)).void).void}
  def self.bind_example(&blk); end

  sig {params(blk: T.proc.bind(T.attached_class).void).void}
  def self.bind_attached(&blk); end
end

B.bind_example do
  example = new
  T.reveal_type(example) # error: `T.attached_class (of Example)`
  examples = T::Array[T.attached_class].new
  T.reveal_type(examples) # error: `T::Array[T.attached_class (of Example)]`
  examples << example
  examples << 'nope'
  #           ^^^^^^ error: Expected `T.attached_class (of Example)` but found `String("nope")`
end

B.bind_attached do
  # There is a duplicate error here, because the resolver pass that looks at
  # `T.let` and the intrinsic report this error
  b = T.let(new, T.attached_class)
  #         ^^^ error: Method `new` does not exist
  #              ^^^^^^^^^^^^^^^^ error: `T.attached_class` may only be used in a singleton class method context
  #              ^^^^^^^^^^^^^^^^ error: `T.attached_class` may only be used in a singleton class method context

  puts(b)
  bs = T::Array[T.attached_class].new
  #             ^^^^^^^^^^^^^^^^ error: `T.attached_class` may only be used in a singleton class method context
  T.reveal_type(bs) # error: `T::Array[T.untyped]`
end
