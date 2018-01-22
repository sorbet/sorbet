# @typed

class CovariantNotAllowed
  type_decl +T # error: can only have invariant type members
end

class Invalids
  type_decl t # error: Invalid type definition
  type_decl 1+1 # error: Invalid type definition
  type_decl :baz # error: Invalid type definition
  type_decl "mama" # error: Invalid type definition
  type_decl 1 # error: Invalid type definition
  type_decl [1] # error: Invalid type definition
end


class Parent
  type_decl T
end

class GoodChild < Parent
  type_decl T
  type_decl My
end

class BadChild1 < Parent
  type_decl My
  type_decl T # error: Type members in wrong order
end

class BadChild2 < Parent # error: should be declared again
  type_decl My
end
