# typed: strict

class ::Integer
  # This used to accidentally overwrite the loc of Integer#times's block
  # argument to have an empty Loc.
  def times # error: Refusing to typecheck `Integer#times` against an overloaded signature
  end
end

0.times {}
