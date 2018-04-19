# typed: strict
def foo
  begin
    return 1
  rescue
    return 2
  end
  deadcode
end
