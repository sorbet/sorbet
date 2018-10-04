# typed: true
class Gener
  extend T::Helpers

  HydraResource = T.type_alias(DOES_NOT_EXIST) # error: Unable to resolve constant

  sig do
    params(table: HydraResource)
    .void
  end
  def generate_top_level_table(table)
  end
end
