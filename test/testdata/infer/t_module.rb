# typed: true
extend T::Sig

sig { params(x: T.all(T::Props, Kernel)).void }
def example(x)
  klass = x.class
  T.reveal_type(klass) # error: `T.untyped`
end

sig { params(mod: T::Module[T.anything]).void }
def example2(mod)
  if mod < T::Props
    T.reveal_type(mod) # error: `T::Module[T::Props]`

    if mod.is_a?(Class)
      T.reveal_type(mod) # error: `T::Class[T::Props]`
      inst = mod.new
      T.reveal_type(inst) # error: `T::Props`
    end
  end

  if mod < T::Props::Serializable && mod.is_a?(T::Props::Serializable::ClassMethods)
    T.reveal_type(mod) # error: `T.all(T::Module[T::Props::Serializable], T::Props::Serializable::ClassMethods)`

    if mod.is_a?(Class)
      T.reveal_type(mod) # error: `T.all(T::Class[T::Props::Serializable], T::Props::Serializable::ClassMethods)`
      inst = mod.new
      T.reveal_type(inst) # error: `T::Props::Serializable`
    end
  end
end

class NoAttachedClassRequired < Module
  # this file has a typed level of true, so has_attached_class! is not required
end
