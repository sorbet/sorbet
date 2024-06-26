# typed: true
class Module; include T::Sig; end

class Unset
  extend T::Helpers
  final!
end

sig do
  type_parameters(:T)
    .params(value: T.any(T.type_parameter(:T), Unset))
    .returns(T.type_parameter(:T))
end
def example(value)
  case value
  when Unset
    raise RuntimeError.new
  end

  value
end
