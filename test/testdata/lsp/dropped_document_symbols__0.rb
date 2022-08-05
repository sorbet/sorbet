# typed: strict

class PublicOuterface
end

# Move the declaration down so locs change for `Bottom` between this file
# and the other files in this testcase.
module Top::Upper::Lower::Bottom
  class Private::Basement::Primus
  end
  class PublicInterface
  end
end
