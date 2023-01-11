# typed: true

# Most partially typed keywords show up as SendResponse internally, so we tap
# into method name completion to service keywords. There are some, though, that
# don't come back as a SendResponse:
#
#   IdentResponse:
#     __ENCODING__
#     __LINE__
#     __FILE__
#   ConstantResponse:
#     BEGIN
#     END
#
# Rather than also tap into ident / const responses to service keyword
# completion for those keywords, we just drop them on the floor (which is fine,
# because these keywords are comparably rare).

# `alias_method` is not a keyword
alia # error: does not exist
#   ^ alias, alias_method

an # error: does not exist
# ^ and, ancestors

begi # error: does not exist
#   ^ completion: begin

brea # error: does not exist
#   ^ completion: break

cas # error: does not exist
#  ^ completion: case

clas # error: does not exist
#   ^ completion: class, ...

de # error: does not exist
# ^ completion: def, defined?, ...

defined # error: does not exist
#      ^ completion: defined?, ...

d # error: does not exist
#^ completion: def, defined?, do, ...

# `else` is more common--be sure it comes before `ensure`
els # error: does not exist
#  ^ completion: else, elsif

elsi # error: does not exist
#   ^ completion: elsif

# `end` is more common--be sure it comes before `ensure`
en # error: does not exist
# ^ completion: end, ensure, ...

ensur # error: does not exist
#    ^ completion: ensure

fals # error: does not exist
#   ^ completion: false

fo # error: does not exist
# ^ completion: for, ...

# `i` is a prefix of two keywords
i # error: does not exist
#^ completion: if, in, ...

modul # error: does not exist
#    ^ completion: module, ...

nex # error: does not exist
#  ^ completion: next

ni # error: does not exist
# ^ completion: nil, ...

no # error: does not exist
# ^ completion: not

o # error: does not exist
#^ completion: or, ...

red # error: does not exist
#  ^ completion: redo

rescu # error: does not exist
#    ^ completion: rescue

retr # error: does not exist
#   ^ completion: retry

retur # error: does not exist
#    ^ completion: return

sel # error: does not exist
#  ^ completion: self, ...

supe # error: does not exist
#   ^ completion: super, superclass

# then is special; see below

tru # error: does not exist
#  ^ completion: true, ...

# undef is special; see below

unles # error: does not exist
#    ^ completion: unless

unti # error: does not exist
#   ^ completion: until

whe # error: does not exist
#  ^ completion: when

whil # error: does not exist
#   ^ completion: while

yiel # error: does not exist
#   ^ completion: yield, ...

# then is both a keyword and a method
# 1) if ... then ... else ... end
# 2) A.new.then
the # error: does not exist
#  ^ completion: then, then

# ... so when keywords aren't allowed, only the method is suggested
class A; end
A.new.the # error: does not exist
#        ^ completion: then

# undef_method comes before undef because undef is a method on Kernel
# but undef_method is a method on Module (Module < Object < Kernel)
unde # error: does not exist
#   ^ completion: undef_method, undef
