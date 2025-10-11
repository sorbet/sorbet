# typed: true

def test(...)
  [1,2,3].each do
    foo(...) do end
  #     ^^^         error: both block argument and literal block are passed
  end
end
