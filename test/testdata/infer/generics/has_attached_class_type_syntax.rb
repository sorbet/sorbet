# typed: true

class C1
  def foo
    xs = T::Array[T.attached_class].new
    #             ^^^^^^^^^^^^^^^^ error: `T.attached_class` may only be used in singleton methods on classes or instance methods on `has_attached_class!` modules
  end
end

module M1
  def foo
    xs = T::Array[T.attached_class].new
    #             ^^^^^^^^^^^^^^^^ error: `M1` must declare `has_attached_class!` before module instance methods can use `T.attached_class`
  end
end

module M2
  extend T::Generic
  has_attached_class!

  def foo
    xs = T::Array[T.attached_class].new
    T.reveal_type(xs) # error: Revealed type: `T::Array[T.attached_class]`
  end
end

module M3
  def self.foo
    xs = T::Array[T.attached_class].new
    #             ^^^^^^^^^^^^^^^^ error: `T.attached_class` cannot be used in singleton methods on modules, because modules cannot be instantiated
  end
end
