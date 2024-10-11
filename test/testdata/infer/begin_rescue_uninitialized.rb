# typed: true
extend T::Sig

sig { returns(Integer) }
def returns_int
  raise StandardError.new
end

begin
  x = returns_int
rescue ArgumentError
  x = returns_int
end

x&.even?
