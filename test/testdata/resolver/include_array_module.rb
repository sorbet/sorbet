# typed: true

# You don't want to write code like this, but it used to result in a crash.
class Module # error-with-dupes: Missing definition for abstract method `Enumerable#each`
  include Array
  #       ^^^^^ error: Only modules can be `include`d, but `Array` is a class

  include String
  #       ^^^^^^ error: Only modules can be `include`d, but `String` is a class

  include Hash
  #       ^^^^ error: Only modules can be `include`d, but `Hash` is a class
end
