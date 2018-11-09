# typed: true

# classes and their singleton class get names
class A; end

# inheritance
class B < A
end

# modules (and their singletons)
module M; end

class D
  # include module
  include M
end
