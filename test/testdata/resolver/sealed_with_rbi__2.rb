# typed: false

class D
  include M # error-with-dupes: `M` is sealed and cannot be included in `D`

  define_method(:from_d) do
  end
end
