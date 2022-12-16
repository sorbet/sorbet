# typed: true

class A
  extend T::Sig
  1.times do end
    #       ^ completion: (nothing)
  # end
end

class B
  extend T::Sig
  sig do end # error-with-dupes: Malformed `sig`
    #   ^ completion: (nothing)
end
