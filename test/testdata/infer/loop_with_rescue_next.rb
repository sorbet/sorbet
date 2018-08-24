# typed: strict
2.times do
  begin
    return 1
  rescue
    next
  end
end
