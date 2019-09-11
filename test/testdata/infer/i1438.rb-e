# typed: true
class A
  extend T::Sig

  sig {params(x: T::Boolean).void}
  def testing(x)
    bar = !x
    zap = !bar || 123

    if x
      raise
    end

    bar
  end
end
