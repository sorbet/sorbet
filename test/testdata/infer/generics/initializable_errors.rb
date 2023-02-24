# typed: true

module A
  extend T::Generic

  initializable!('') # error: Invalid param, must be a :symbol
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

module D
  extend T::Generic

  initializable!(:out, :extra)
  #                 ^^^^ error: Unknown key `nope` provided in block to `type_member`
end
