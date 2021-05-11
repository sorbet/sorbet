# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

# length is not the only method that we need to be careful about, but it's a
# reasonable starting place.

class Stringlike < String
  def length
    28
  end
end

sig {params(string: String).returns(Integer)}
def ref(string)
  string.length
end

s = Stringlike.new("default")

p ref(s)
