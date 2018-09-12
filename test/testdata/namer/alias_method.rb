# typed: true
module Alias
  def f
  end

  alias f_alias f
  alias_method :f_alias_1, :f

  alias_method        # error: MULTI
  alias_method :f     # error: MULTI
  alias_method :f, 8  # error: MULTI
end
