module Opus::Types
  def any
  end
end

class String
  standard_method(
    {
      _: Opus::Types.any(Integer),
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def getbyte
  end
end