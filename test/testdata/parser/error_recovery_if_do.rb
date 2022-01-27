# typed: true

class A
  if 'thing' do
# ^^ error: Unexpected use of "if"; did you mean to use "it"?
  end
end
