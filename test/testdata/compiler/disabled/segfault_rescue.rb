# typed: true

def foo
  [1].each do
    begin
    rescue
    end
  end
end
