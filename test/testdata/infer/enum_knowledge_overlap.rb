# typed: strict

class Purpose < T::Enum
  enums do
    A = new
    B = new
    C = new
    D = new
  end
end

extend T::Sig

sig {params(purpose: Purpose).void}
def overlapping_groups(purpose)
  first = purpose == Purpose::A || purpose == Purpose::B
  second = purpose == Purpose::B || purpose == Purpose::C

  if first
    if second
      T.reveal_type(purpose) # error: `Purpose::B`
    else
      T.reveal_type(purpose) # error: `Purpose::A`
    end
  end

  if !first && !second
    T.reveal_type(purpose) # error: `Purpose::D`
  end

  if first && purpose == Purpose::D
    T.absurd(purpose)
  end
end
