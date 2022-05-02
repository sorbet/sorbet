# typed: strict
class Upper
  extend T::Sig
  extend T::Generic

  Elem = type_template {{upper: Upper}}

  sig {params(e: Elem).void}
  def self.elem(e); end
end

class Fixed
  extend T::Sig
  extend T::Generic

  Elem = type_template {{fixed: Fixed}}

  sig {params(e: Elem).void}
  def self.elem(e); end
end

# When types are used without specifying an instantion for their parameters,
# sorbet defaults them according to their variance: :in parameters are
# defaulted to their lower bound, :invariant(default) parameters are defaulted
# to T.untyped, and :out parameters are defaulted to their upper bound.
# One exception to this rule is fixed type parameters, who are treated as
# having the same upper and lower bound internally, and thus can be defaulted
# to that type exactly.
#
# As classes in sorbet are only allowed to have :invariant type parameters
# (like in csharp), the Elem type template in your Upper class is being
# defaulted to T.untyped at use sites when no instantiation is supplied.
# Unfortunately there is not currently any syntax for specifying the
# instantiation of a type_template, so using non-fixed type_templates
# is only really advisable in situations where you plan to inherit from a
# class, and re-declare its type_templates as fixed.

Upper.elem(1)
Fixed.elem(1)
