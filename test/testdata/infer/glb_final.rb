# typed: true
extend T::Sig

class FinalClass
  extend T::Helpers
  final!
end
module NonFinalModule; end

class NonFinalClass; end
module FinalModule
  extend T::Helpers
  final!
end

sig {params(x: FinalClass).void}
def example1(x)
  case x
  when NonFinalModule
    puts(x) # error: This code is unreachable
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `FinalClass` wasn't handled
  end
end

sig {params(x: NonFinalClass).void}
def example2(x)
  case x
  when FinalModule
    puts(x) # error: This code is unreachable
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `NonFinalClass` wasn't handled
  end
end

sig do
  params(
    x: T::Array[T.all(FinalClass, NonFinalModule)],
    y: T::Array[T.all(NonFinalClass, FinalModule)]
  ).void
end
def example3(x, y)
  T.reveal_type(x) # error: `T::Array[T.noreturn]`
  T.reveal_type(y) # error: `T::Array[T.noreturn]`
end
