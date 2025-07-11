# typed: true

class A
  def value
    5
  end
end

extend T::Sig

sig { params(avec: T::Array[A]).returns(T.untyped) }
def sumup(avec)
  avec.sum(&:val) # error: does not exist
#               ^ completion: value, ...

  avec.sum(&:)
#          ^ error: no anonymous block parameter
#           ^ error: unexpected token
#            ^ completion: avec, CSV, class, ...
end
