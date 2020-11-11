# typed: false
# The fast path causes duplicate errors.

# a penultimate class
class PenC
  extend T::Helpers
  penultimate!
end

class BadInherit < PenC # error: `PenC` was declared as penultimate but its child `BadInherit` was not declared final
end

class OkayInherit < PenC
  extend T::Helpers
  final!
end


# a penultimate module
module PenM
  extend T::Helpers
  penultimate!
end

class BadInclude
  include PenM  # error: `BadInclude` is not final, but included the penultimate module `PenM`
end

class OkayInclude
  include PenM
  extend T::Helpers
  final!
end



class BadExtend
  extend PenM  # error: `BadExtend` is not final, but extended the penultimate module `PenM`
end

class OkayExtend
  extend PenM
  extend T::Helpers
  final!
end
