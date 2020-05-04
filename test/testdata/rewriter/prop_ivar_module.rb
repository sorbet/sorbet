# typed: strict

# There should be no errors for accessing instance variables here, because the
# Prop rewriter pass declares them.

module MixesInProps
  include T::Props

  const :in_parent, Integer
end
