# typed: true

describe 'bar' do
  it 'foo' do
    class A
      X = 1
    end
    p(X) # error: Unable to resolve constant `X`
  end
end
