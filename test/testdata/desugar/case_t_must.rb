# typed: true
extend T::Sig

sig {params(x: Integer).void}
def one_when(x)
  case x
  when 1 then 2
  end.even? # error: Method `even?` does not exist on `NilClass` component of `T.nilable(Integer)`
end

sig {params(x: Integer).void}
def two_whens(x)
  case x
  when 1 then 2
  when 3 then 4
  end.even? # error: Method `even?` does not exist on `NilClass` component of `T.nilable(Integer)`
end
