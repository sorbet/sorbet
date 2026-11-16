# typed: true
extend T::Sig

sig {params(x: Integer).returns(Integer)}
def one_when(x)
  case x
  when 1 then 2
  end
end

sig {params(x: Integer).returns(Integer)}
def one_when_no_predicate(x)
  case
  when x == 1 then 2
  end
end

sig {params(x: Integer).returns(Integer)}
def one_when_temp_predicate(x)
  case x + 1
  when 1 then 2
  end
end

sig {params(x: Integer).returns(Integer)}
def two_whens(x)
  case x
  when 1 then 2
  when 3 then 4
  end
end
