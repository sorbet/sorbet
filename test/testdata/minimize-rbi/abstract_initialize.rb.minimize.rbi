# typed: true

class AbstractFoo
  extend T::Helpers

  # This method will actually be defined at runtime in files that use `abstract!`
  # (The method is designed to `raise` unconditionally.)
  def initialize(*args, &blk)
  end
end
