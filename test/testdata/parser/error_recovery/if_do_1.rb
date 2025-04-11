# typed: true

class A
  if 'thing' do
# ^^ parser-error: Unexpected token "if"; did you mean "it"?
  end
end
