# typed: strict
def main
  @a = 3 # error: Use of undeclared variable `@a`
# ^^
#  @b = 3 # error: Use of undeclared variable `@b`
## ^^^
#  @c = 3 # error: Use of undeclared variable `@c`
##  ^^
end
