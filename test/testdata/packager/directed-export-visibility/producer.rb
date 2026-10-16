# typed: strict
# stratum: 0
module Producer
  # Producer application API
  class Public
    #   ^^^^^^ def: public
  end
  class Original
    class Nested; end
  end
  Alias = Original
  Value = 1
  Type = T.type_alias {Integer}
end
