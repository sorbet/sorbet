module A; end
module B; end
module A;
  include B
end
module B
  include A # error: Circular dependency
end
