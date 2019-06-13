# typed: true

extend T::Sig

module A; end
module B; end

sig {params(x: T.all(A, B)).returns(Integer)}
def check_left_all(x)
  case x
  when A
    1
  else
    2 # error: This code is unreachable
  end
end

sig {params(x: T.all(A, B)).returns(Integer)}
def check_right_all(x)
  case x
  when B
    1
  else
    2 # error: This code is unreachable
  end
end

sig {params(x: T.any(A, T.all(A, B))).returns(Integer)}
def check_overlapping_all_part(x)
  case x
  when A
    1
  end
end

sig {params(x: T.any(A, T.all(A, B))).returns(Integer)}
def check_non_overlapping_all_part(x)
  case x
  when B
    1
  else
    2
  end
end
