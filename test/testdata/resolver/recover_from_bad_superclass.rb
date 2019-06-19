# typed: true

def make_a_class
  Class.new
end

class A < 1 # error: Superclasses must only contain constant literals
end
T.let(A, Class)
A.new.class

DynamicParent = make_a_class
class B < DynamicParent # error: Superclasses and mixins may only use class aliases like `A = Integer`
end
T.let(B, Class)
B.new.class

Alias = T.type_alias(Object)
class C < Alias # error: Superclasses and mixins may not be type aliases
end
T.let(C, Class)
C.new.class

class D < D # error: Circular dependency: `D` is a parent of itself
end
T.let(D, Class)
D.new.class

class F < G
end
T.let(F, Class)
F.new.class

class G < F # error: Circular dependency: `G` and `F` are declared as parents of each other
end
T.let(G, Class)
G.new.class
