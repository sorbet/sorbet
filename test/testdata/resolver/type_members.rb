# @typed

class CovariantNotAllowed
  Elem = T.type(:in) # error: can only have invariant type members
end

class Invalids
  Exp = T.type(1+1) # error: Invalid type definition
  Baz = T.type(:baz) # error: Invalid variance kind, only :out and :in are supported
  Mama = T.type("mama") # error: Invalid type definition
  One = T.type(1) # error: Invalid type definition
  ArrOne = T.type([1]) # error: Invalid type definition
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
