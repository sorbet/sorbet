# typed: true
         class QueryProfile < Hash
#        ^^^^^^^^^^^^^^^^^^^^^^^^^ noerror Type `K` declared by parent `Hash` must be declared again
#        ^^^^^^^^^^^^^^^^^^^^^^^^^ noerror Type `Elem` declared by parent `Hash` must be declared again
				   V = type_member
           def explain_for(which)
             self["explain_#{which}"] || self[:"explain_#{which}"]
#            ^^^^^^^^^^^^^^^^^^^^^^^^ error: `String` doesn't match `QueryProfile::V` for argument `arg0`
#                                        ^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Symbol` doesn't match `QueryProfile::V` for argument `arg0`
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
#   ^^^^^^^^^^^ error: `String("foo")` doesn't match `Child::A` for argument `k`
#                  ^^^^^^^^^^^ error: `String("bar")` doesn't match `Child::A` for argument `k`
  end
end
