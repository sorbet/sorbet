# typed: strict
def main # error: does not have a `sig`
  @a = 3 # error: The instance variable `@a` must be declared using `T.let` when specifying `# typed: strict`
end
