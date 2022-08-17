# typed: true

class Box
  MyInteger = T.type_alias {Integer} # error: Type aliases are not allowed in generic classes
end
