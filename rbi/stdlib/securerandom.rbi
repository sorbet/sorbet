# typed: true

module SecureRandom
  extend(Random::Formatter)
end

module Random::Formatter
  ALPHANUMERIC = T.let(T.unsafe(nil), Array)

  def rand(*_)
  end

  def uuid()
  end

  def alphanumeric(n = _)
  end

  def random_number(*_)
  end

  def random_bytes(n = _)
  end

  def base64(n = _)
  end

  def urlsafe_base64(n = _, padding = _)
  end

  def hex(n = _)
  end
end
