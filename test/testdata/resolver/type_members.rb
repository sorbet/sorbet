# @typed

class CovariantNotAllowed
  Elem = T.type(:in) # error: can only have invariant type members
end

class Invalids
  Exp = T.type(1+1) # error: Invalid param, must be a :symbol
  Baz = T.type(:baz) # error: Invalid variance kind, only :out and :in are supported
  Mama = T.type("mama") # error: Invalid param, must be a :symbol
  One = T.type(1) # error: Invalid param, must be a :symbol
  ArrOne = T.type([1]) # error: Invalid param, must be a :symbol
  BadArg = T.type(junk: 1) # error: Missing required param :fixed
end


class Parent
  Elem = T.type
end

class GoodChild < Parent
  Elem = T.type
  My = T.type
end

class BadChild1 < Parent
  My = T.type
  Elem = T.type # error: Type members in wrong order
end

class BadChild2 < Parent # error: should be declared again
  My = T.type
end

class BadChild3 < Parent
  Elem = 3 # error: Type variable <constant:Elem> needs to be declared as `= T.type(SOMETHING)`
end
