# typed: true

module Forwardable
  FILTER_EXCEPTION = T.let(T.unsafe(nil), String)

  FORWARDABLE_VERSION = T.let(T.unsafe(nil), String)

  def def_delegators(accessor, *methods)
  end

  def def_instance_delegator(accessor, method, ali = _)
  end

  def def_instance_delegators(accessor, *methods)
  end

  def def_delegator(accessor, method, ali = _)
  end

  def delegate(hash)
  end

  def instance_delegate(hash)
  end
end
