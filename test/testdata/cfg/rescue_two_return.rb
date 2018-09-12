# typed: true
def foo
  begin
    return 1
  rescue
    return 2
  end
  deadcode # error: This code is unreachable
end
