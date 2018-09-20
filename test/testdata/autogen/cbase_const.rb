# typed: true

class C
end

module Scope
  C
  ::C
end
