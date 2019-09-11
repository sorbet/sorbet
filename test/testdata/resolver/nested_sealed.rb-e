# typed: true

module Result
  extend T::Helpers
  sealed!
end

class Ok
  include Result
end

module Error
  include Result

  extend T::Helpers
  sealed!
end

class Bad1
  include Error
end

class Bad2
  include Error
end

extend T::Sig

sig {params(x: Result).void}
def nested_inheritance_flattened_case(x)
  case x
  when Ok
    T.reveal_type(x) # error: Revealed type: `Ok`
  when Bad1
    T.reveal_type(x) # error: Revealed type: `Bad1`
  when Bad2
    T.reveal_type(x) # error: Revealed type: `Bad2`
  else
    T.absurd(x)
  end
end

sig {params(x: Result).void}
def nested_inheritance_flattened_case_missing(x)
  case x
  when Ok
    T.reveal_type(x) # error: Revealed type: `Ok`
  when Bad1
    T.reveal_type(x) # error: Revealed type: `Bad1`
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `Bad2` wasn't handled
  end
end

sig {params(x: Result).void}
def nested_inheritance_nested_case(x)
  case x
  when Ok
    T.reveal_type(x) # error: Revealed type: `Ok`
  when Error
    case x
    when Bad1
      T.reveal_type(x) # error: Revealed type: `Bad1`
    when Bad2
      T.reveal_type(x) # error: Revealed type: `Bad2`
    else
      T.absurd(x)
    end
  else
    T.absurd(x)
  end
end
