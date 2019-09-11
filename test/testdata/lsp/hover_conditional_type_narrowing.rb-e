# typed: true

def main
  a = T.let(nil, T.nilable(String))
  if !a.nil?
    puts a
       # ^ hover: String
  else
    puts a
       # ^ hover: NilClass
  end
  b = T.must(a)
# ^ hover: String
end
