# typed: strict
module Alias
  def f
  end

  alias f_alias f
  alias_method :f_alias_1, :f

  alias_method        # error: Wrong number of arguments
  alias_method :f     # error: Wrong number of arguments
  alias_method :f, 8  # error: arguments must be symbol literals
end
