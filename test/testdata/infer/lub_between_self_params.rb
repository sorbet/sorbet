# typed: true
# disable-fast-path: true
         class QueryProfile < Hash
#        ^^^^^^^^^^^^^^^^^^^^^^^^^ noerror Type `K` declared by parent `Hash` must be re-declared in `QueryProfile`
#        ^^^^^^^^^^^^^^^^^^^^^^^^^ noerror Type `Elem` declared by parent `Hash` must be re-declared in `QueryProfile`
				   V = type_member
           def explain_for(which)
             self["explain_#{which}"] || self[:"explain_#{which}"]
#                 ^^^^^^^^^^^^^^^^^^ error: Expected `QueryProfile::V` but found `String` for argument `arg0`
#                                             ^^^^^^^^^^^^^^^^^^^ error: Expected `QueryProfile::V` but found `Symbol` for argument `arg0`
           end
         end

class Parent
  extend T::Generic
  extend T::Sig
  A = type_member

  sig {params(k:A).void}
  def [](k)
  end
end
class Child < Parent
  A = type_member
  def foo(which)
    self["foo"] || self["bar"]
#        ^^^^^ error: Expected `Child::A` but found `String("foo")` for argument `k`
#              ^^^^ error: Branching on `void` value
#                  ^^^^^^^^^^^ error: This code is unreachable
  end
end
