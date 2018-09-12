# typed: strict
def main # error: does not have a `sig`
  @a = 3 # error: Use of undeclared variable `@a`
# ^^
#  @b = 3 # error: Use of undeclared variable `@b`
## ^^^
#  @c = 3 # error: Use of undeclared variable `@c`
##  ^^
end
