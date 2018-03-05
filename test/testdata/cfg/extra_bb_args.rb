# @typed

def main
  name = T.let(nil, T.nilable(String))
  return "missing name" if name.nil?
  name.include?("foo")
end
