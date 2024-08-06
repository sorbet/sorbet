# typed: true
extend T::Sig

sig { params(x: T.all(T::Props, Kernel)).void }
def example(x)
  klass = x.class
  T.reveal_type(klass)
end

T.reveal_type(0.class)

sig { params(mod: Module).void }
def example2(mod)
  if mod < T::Props
    T.reveal_type(mod)

    if mod.is_a?(Class)
      T.reveal_type(mod)
      inst = mod.new
      T.reveal_type(inst)
    end
  end

  if mod < T::Props::Serializable && mod.is_a?(T::Props::Serializable::ClassMethods)
    T.reveal_type(mod)

    if mod.is_a?(Class)
      T.reveal_type(mod)
      inst = mod.new
      T.reveal_type(inst)
    end
  end

end
