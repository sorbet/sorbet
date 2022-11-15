# typed: true

class Gener
  extend T::Sig

  HydraResource = T.type_alias { DOES_NOT_EXIST }
 #                               ^^^^^^^^^^^^^^ error: Unable to resolve constant
# ^^^^^^^^^^^^^ error: Unable to resolve right hand side of type alias
  sig do
    params(table: HydraResource)
      .void
  end
  def generate_top_level_table(table); end
  C = 1
end
