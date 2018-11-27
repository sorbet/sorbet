# typed: true

class WeakRef < ::Delegator
  class RefError < ::StandardError

  end

  def __setobj__(obj)
  end

  def __getobj__()
  end

  def weakref_alive?()
  end
end
