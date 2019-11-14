# typed: true
class Record
  def self.ff_from(*_); end
  def self.ff_pattern(*_); end
  def self.ff_field(*_); end
end
class Flatfile < Record
  ff_from   1..2, :foo
  ff_pattern(/A-Za-z/, :bar)
  ff_field :baz
end

t = Flatfile.new
t.foo = t.foo + 1
t.bar = t.bar + 1
t.baz = t.baz + 1
