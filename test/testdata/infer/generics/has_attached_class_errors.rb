# typed: true

module A
  extend T::Generic

  has_attached_class!('') # error: Invalid param, must be a :symbol
  #                   ^^ error: Expected `Symbol` but found `String("")` for argument `variance`
  #                   ^^ error: Expected `Symbol` but found `String("")` for argument `variance`
end

module B
  extend T::Generic

  has_attached_class!(:nope) # error: Invalid variance kind, only `:out` and `:in` are supported
end

module C
  extend T::Generic

  has_attached_class! { {nope: Integer} }
  #                      ^^^^ error: Unknown key `nope` provided in block to `type_member`
end
