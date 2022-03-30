# typed: true

class BaseClass; end

class A < BaseClass; end
class B < BaseClass; end
class C < BaseClass; end
class D < BaseClass; end
class E < BaseClass; end
class F < BaseClass; end
class G < BaseClass; end
class H < BaseClass; end
class Not; end

def f
  x = T.let([A, B, C, D, E, F, G, H, Not].to_set.freeze, T::Set[T.class_of(BaseClass)])
  return x
end
