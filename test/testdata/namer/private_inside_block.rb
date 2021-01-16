# typed: true

class Foo
  def self.foo(&blk); end

  foo do
    private

    def bar; end
  end

  def baz; end
end

Foo.new.baz
