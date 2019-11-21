# typed: true
class Record
  def self.flatfile; end
  def self.from(*_); end
  def self.pattern(*_); end
  def self.field(*_); end
end
class Flatfile < Record
  flatfile do
    from   1..2, :foo
    pattern(/A-Za-z/, :bar)
    field :baz
  end
end

t = Flatfile.new
t.foo = t.foo + 1
t.bar = t.bar + 1
t.baz = t.baz + 1
