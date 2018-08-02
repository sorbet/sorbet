# typed: true

CSV.new(StringIO.new)
CSV.new(T.must(File.open('foo')))
CSV.new(StringIO.new, {opt: 'bar'})
CSV.new

CSV(StringIO.new)
CSV(T.must(File.open('foo')))
CSV(StringIO.new, {foo: 'bar'})
CSV()

T.let(
  T.must(CSV.parse("a,b,c\nd,e,f")),
  T::Array[T::Array[T.nilable(String)]],
)
CSV.parse("a,b,c\n", {foo: 'bar'})
CSV.parse("a,b,c\nd,e,f") {|r| T.let(r, T::Array[T.nilable(String)])}

CSV.parse_line("ab,cd")
CSV.parse_line("ab,cd", {foo: 'bar'})

CSV.read("path.csv")
CSV.read("path.csv", {foo: 'bar'})

T.let(
  T.must(CSV(StringIO.new('a,b,c')).readline),
  T::Array[T.nilable(String)],
)
