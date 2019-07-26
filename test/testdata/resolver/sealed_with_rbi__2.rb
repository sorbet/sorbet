# typed: false

class D
  include M # `M` is sealed and cannot be included in `D`

  define_method(:from_d) do
  end
end
