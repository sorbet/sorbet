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

# block argument
def foo(&x); end

# lambda
lambda {}

case x
when y
  1
else z
end


# character literal
?x

# numeric types
1+4i

0.5

1.5i

# singleton class sugar
def self.classmeth; end

class <<self; end

# booleans
true || false


# loops
for x in arr; end
while 0 != 1; end

# index assignment
x[1] = 0

# break
break
break 1
break 1,2

# defined
defined?(X)


# zsuper
super

# kwargs
def foo(x:,y:1,**z); end

# kwsplat
{**x}

__LINE__


# loops
while true; nil; end
nil while true

until true; nil; end
nil until true

# mlhs1
a, ((x,)) = 1

# nth_ref
$4

# optarg
def foo(x=1); end


# pair and pair_quoted
{x => y, "foo": 1}
