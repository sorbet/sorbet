# typed: true
module A
  CONST = :A
end

module B
  CONST = :B
end

module C
  include A
end

module D
  include B
  include C

  puts CONST
end
