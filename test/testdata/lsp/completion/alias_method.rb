# typed: true

class A
  def to_this; end

  alias_method :from_this, :to_this
end

A.new.from_thi # error: does not exist
#             ^ completion: from_this
