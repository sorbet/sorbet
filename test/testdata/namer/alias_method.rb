# typed: true
module Alias
  def f
  end

  alias f_alias f
  alias_method :f_alias_1, :f

  alias_method # error: Not enough arguments provided for method `Module#alias_method`. Expected: `2`, got: `0`
  alias_method :f
  #            ^^ error: Not enough arguments provided for method `Module#alias_method`. Expected: `2`, got: `1`
  alias_method :f, 8 # error: Expected `Symbol` but found `Integer(8)` for argument `old_name`
  alias_method :bad_alias, :nonexistent_method
  #                        ^^^^^^^^^^^^^^^^^^^ error: Can't make method alias from `bad_alias` to non existing method `nonexistent_method`

  def alias_user
    f_alias_1
    # Should work without error, even though the alias is bad.
    bad_alias
  end
end
