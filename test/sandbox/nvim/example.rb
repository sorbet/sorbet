# typed: strict
class Module; include T::Sig; end

class A
  sig {params(x: Integer).void}
  def main(x)
    if x.even?
      uts(x)
    end
  end
end
