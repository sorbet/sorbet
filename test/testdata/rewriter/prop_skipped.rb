# typed: true

class A < T::Struct
  # This prop won't be processed by the Prop dsl, as it contains a shape type
  const :foo, T::Array[{foo: Integer}]
                     # ^^^^^^^^^^^^^^ error: Invalid type in prop type descriptor
end

  A.new(foo: [])
# ^^^^^^^^^^^^^^ error: Too many arguments
