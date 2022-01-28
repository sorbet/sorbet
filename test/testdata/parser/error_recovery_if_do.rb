# typed: true

class A
  if 'thing' do
# ^^ error: Unexpected token "if"; did you mean "it"?
  end
end
