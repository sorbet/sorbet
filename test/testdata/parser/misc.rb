# typed: false
# contains miscellaneous syntactic features in order of implementation
# in our parser.

alias afoo bar

# various LHSs
@iv = 1
@@cv = 1
$gv = 1

x.var, y = nil, nil
baaaar, naaar = zaaaz
xaaaaz = yayayaya, tutututu


# `begin` keyword

begin; end
begin; nil; end
begin; a; b; end

# block argument
def bfoo(&x); end

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


# next
next
next 1
next 1,2

# defined
defined?(X)


# zsuper
super

# kwargs
def kwfoo(x:,y:1,**z); end

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

# optarg, restarg
def optfoo(x=1, *y); end

# pair and pair_quoted
{x => y, "foo": 1}

BEGIN{foo} # error: Unsupported node type `Preexe`
END{bar} # error: Unsupported node type `Postexe`

# rationals
4r
5ri

# rescue, resbody
begin
rescue E=>x
  nil
end

# splat, splat_mlhs
*x = *y

# symbols
:"foo#{bar}"
%i{sym}


# ternary
x ? 1 : 7

undef x, y # error: Unsupported node type `Undef`

# words
%w{a b}
%W{a b}

# shellout (xstring)
%x{true}

# shadowarg
proc{|;x|}

# Whacky parsing edge case around keyword break and blocks
break cfoo 1 do end


# bare * args
def sfoo(*); end
def ssfoo(**); end
