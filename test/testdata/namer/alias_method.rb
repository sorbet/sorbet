# typed: true
module Alias
  def f
  end

  alias f_alias f
  alias_method :f_alias_1, :f

  alias_method # error: Not enough arguments provided for method `Module#alias_method`. Expected: `2`, got: `0`
  alias_method :f # error: Not enough arguments provided for method `Module#alias_method`. Expected: `2`, got: `1`
  alias_method :f, 8 # error: `Integer(8)` doesn't match `Symbol` for argument `old_name`
end
