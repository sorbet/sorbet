# typed: true

def test(...)
  [1,2,3].each do
    foo(...) do end
  #     ^^^         parser-error: both block argument and literal block are passed
  # ^^^^^^^^^^^^^^^ error: Splats are only supported where the size
  end
end
