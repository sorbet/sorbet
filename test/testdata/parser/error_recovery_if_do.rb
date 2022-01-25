# typed: true

class A
  if 'thing' do
  #          ^^ error: unexpected token "do"
  end
end
