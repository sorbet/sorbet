# contains miscellaneous syntactic features in order of implementation
# in our parser.

alias foo bar

# various LHSs
@iv = 1
@@cv = 1
$gv = 1

x.var, y = nil, nil


# `begin` keyword

begin; end
begin; nil; end
begin; a; b; end
