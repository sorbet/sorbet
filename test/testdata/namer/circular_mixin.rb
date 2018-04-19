# typed: strict
module A; end
module B; end
module A;
  include B
end
module B
  include A # error: Circular dependency
end

module A
  include A # error: Circular dependency
end
