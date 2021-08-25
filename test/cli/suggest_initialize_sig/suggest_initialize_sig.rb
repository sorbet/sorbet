# typed: true

class Foo
  extend T::Sig

  sig {params(x: Integer).returns(Integer).on_failure(:soft, notify: 'sorbet')}
  def initialize(x)
    @x = x
  end
end