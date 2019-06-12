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
def exhaustive_else(x)
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
def exhaustive_unreachable_else(x)
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
def exhaustive_unreachable_when_before(x)
  ret = case x
  when Symbol
    3 # error: This code is unreachable
  when String
    x.to_i
  when Integer
    x
  end
  T.reveal_type(ret) # error: Revealed type: `Integer`
  ret
end

sig {params(x: T.any(Integer, String)).returns(Integer)}
def exhaustive_unreachable_when_after(x)
  ret = case x
  when String
    x.to_i
  when Integer
    x
  when Symbol # error: This code is unreachable
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

sig {params(x: T.any(Integer, String)).returns(Integer)}
def non_exhaustive_some_cases_unreachable_when(x) # error: Returning value that does not conform to method result type
  case x
  when Integer
    x
  when Symbol
    3 # error: This code is unreachable
  end
end
