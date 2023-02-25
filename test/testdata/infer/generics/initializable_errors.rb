# typed: true

module A
  extend T::Generic

  initializable!('') # error: Invalid param, must be a :symbol
  #              ^^ error: Expected `Symbol` but found `String("")` for argument `variance`
  #              ^^ error: Expected `Symbol` but found `String("")` for argument `variance`
end

module B
  extend T::Generic

  initializable!(:nope) # error: Invalid variance kind, only `:out` and `:in` are supported
end

module C
  extend T::Generic

  initializable! { {nope: Integer} }
  #                 ^^^^ error: Unknown key `nope` provided in block to `type_member`
end
