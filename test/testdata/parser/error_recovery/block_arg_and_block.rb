# typed: true

[1,2,3].each do |x|
  foo(&bar) do end
    # ^^^^ error: both block argument and literal block are passed
end
