# typed: strict

class ::Integer
  # This used to accidentally overwrite the loc of Integer#times's block
  # argument to have an empty Loc.
  def times
  end
end

0.times {}
