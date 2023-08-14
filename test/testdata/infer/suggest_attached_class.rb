# typed: strict

class Foo
  def self.generate # error: does not have a `sig`
    self.new
  end
end
