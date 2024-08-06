# typed: true
extend T::Sig

sig do
  params(
    mod1: T.all(T::Module[T::Props::Serializable], T::Props::Serializable::ClassMethods, T::Class[T.anything]),
    # mod2: T.all(T::Class[T.anything], T::Module[T::Props::Serializable], T::Props::Serializable::ClassMethods)
  ).void
end
# def collapse(mod1, mod2)
def collapse(mod1)
  T.reveal_type(mod1)
  # T.reveal_type(mod2)
end


