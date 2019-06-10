# typed: true

extend T::Sig

sig {params(x: T.any(Integer, String)).returns(Integer)}
def exhaustive_all_cases(x)
  ret = case x
  when Integer
    x
  when String
    x.to_i
  end
  T.reveal_type(ret) # error: Revealed type: `Integer`
  ret
end

sig {params(x: T.any(Integer, String)).returns(Integer)}
def exhaustive_some_cases(x)
  ret = case x
  when Integer
    x
  else
    3
  end
  T.reveal_type(ret) # error: Revealed type: `Integer`
  ret
end

sig {params(x: T.any(Integer, String)).returns(Integer)}
def exhaustive_unreachable_case(x)
  ret = case x
  when String
    x.to_i
  when Integer
    x
  else
    3 # error: This code is unreachable
  end
  T.reveal_type(ret) # error: Revealed type: `Integer`
  ret
end

sig {params(x: T.any(Integer, String)).returns(Integer)}
def non_exhaustive_some_cases(x) # error: Returning value that does not conform to method result type
  case x
  when Integer
    x
  end
end
