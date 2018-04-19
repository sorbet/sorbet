# typed: strict
def foo
  begin
    1
  rescue
  else
    if 2
      3
    end
  end
end
