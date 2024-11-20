# typed: true

class A
  extend T::Sig
  sig { params(x: Integer).void }
  def self.example(x)
    puts(x + 1)
  end
end

begin
  A.
  # ^ completion: example, ...
  # ^ apply-completion: [B] item: 0

  x = nil # error: does not exist

  A.
  # ^ completion: example, ...
  # ^ apply-completion: [A] item: 0

end # error: unexpected token "end"
