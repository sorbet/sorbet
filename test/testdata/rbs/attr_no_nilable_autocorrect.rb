# typed: strict
# enable-experimental-rbs-comments: true

# The `T.nilable` autocorrect for a non-nilable attr sig widens the type in place. That would
# produce `#: T.nilable(Integer)` here, which is not valid RBS, so it is not offered.
class RbsAttr
  #: Integer
  attr_accessor :foo
  #              ^^^ error: Use of undeclared variable `@foo`
  #              ^^^ error: The instance variable `@foo` must be declared using `T.let` when specifying `# typed: strict`
end
