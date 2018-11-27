# typed: true

class GetoptLong
  ORDERINGS = T.let(T.unsafe(nil), Array)

  REQUIRE_ORDER = T.let(T.unsafe(nil), Integer)

  PERMUTE = T.let(T.unsafe(nil), Integer)

  RETURN_IN_ORDER = T.let(T.unsafe(nil), Integer)

  ARGUMENT_FLAGS = T.let(T.unsafe(nil), Array)

  NO_ARGUMENT = T.let(T.unsafe(nil), Integer)

  REQUIRED_ARGUMENT = T.let(T.unsafe(nil), Integer)

  OPTIONAL_ARGUMENT = T.let(T.unsafe(nil), Integer)

  STATUS_YET = T.let(T.unsafe(nil), Integer)

  STATUS_STARTED = T.let(T.unsafe(nil), Integer)

  STATUS_TERMINATED = T.let(T.unsafe(nil), Integer)

  class AmbiguousOption < ::GetoptLong::Error

  end

  class NeedlessArgument < ::GetoptLong::Error

  end

  class Error < ::StandardError

  end

  class MissingArgument < ::GetoptLong::Error

  end

  class InvalidOption < ::GetoptLong::Error

  end

  def ordering()
  end

  def quiet()
  end

  def quiet?()
  end

  def terminated?()
  end

  def error?()
  end

  def get()
  end

  def error_message()
  end

  def get_option()
  end

  def each_option()
  end

  def quiet=(_)
  end

  def error()
  end

  def each()
  end

  def set_options(*arguments)
  end

  def ordering=(ordering)
  end

  def terminate()
  end
end
