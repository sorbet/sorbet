# typed: strict
# enable-experimental-rbs-comments: true

# correct to required positional

#: (Integer) -> void
def req_pos1(x); end

#: (Integer x) -> void
def req_pos2(x); end

#: (?Integer) -> void
#    ^^^^^^^ error: Argument kind mismatch for `x`, method declares `positional`, but RBS signature declares `optional positional`
def req_pos3(x); end

#: (?Integer x) -> void
#    ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `positional`, but RBS signature declares `optional positional`
def req_pos4(x); end

#: (*Integer) -> void
#    ^^^^^^^ error: Argument kind mismatch for `x`, method declares `positional`, but RBS signature declares `rest positional`
def req_pos5(x); end

#: (*Integer x) -> void
#    ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `positional`, but RBS signature declares `rest positional`
def req_pos6(x); end

#: (x: Integer) -> void
#   ^^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `positional`, but RBS signature declares `keyword`
def req_pos7(x); end

#: (?x: Integer) -> void
#    ^^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `positional`, but RBS signature declares `optional keyword`
def req_pos8(x); end

#: (**Integer) -> void
#     ^^^^^^^ error: Argument kind mismatch for `x`, method declares `positional`, but RBS signature declares `rest keyword`
def req_pos9(x); end

#: (**Integer x) -> void
#     ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `positional`, but RBS signature declares `rest keyword`
def req_pos10(x); end

# correct to optional positional

#: (Integer) -> void
#   ^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional positional`, but RBS signature declares `positional`
def opt_pos1(x = 42); end

#: (Integer x) -> void
#   ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional positional`, but RBS signature declares `positional`
def opt_pos2(x = 42); end

#: (?Integer) -> void
def opt_pos3(x = 42); end

#: (?Integer x) -> void
def opt_pos4(x = 42); end

#: (*Integer) -> void
#    ^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional positional`, but RBS signature declares `rest positional`
def opt_pos5(x = 42); end

#: (*Integer x) -> void
#    ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional positional`, but RBS signature declares `rest positional`
def opt_pos6(x = 42); end

#: (x: Integer) -> void
#   ^^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional positional`, but RBS signature declares `keyword`
def opt_pos7(x = 42); end

#: (?x: Integer) -> void
#    ^^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional positional`, but RBS signature declares `optional keyword`
def opt_pos8(x = 42); end

#: (**Integer) -> void
#     ^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional positional`, but RBS signature declares `rest keyword`
def opt_pos9(x = 42); end

#: (**Integer x) -> void
#     ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional positional`, but RBS signature declares `rest keyword`
def opt_pos10(x = 42); end

# correct to rest positional

#: (Integer) -> void
#   ^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest positional`, but RBS signature declares `positional`
def rest_pos1(*x); end

#: (Integer x) -> void
#   ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest positional`, but RBS signature declares `positional`
def rest_pos2(*x); end

#: (?Integer) -> void
#    ^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest positional`, but RBS signature declares `optional positional`
def rest_pos3(*x); end

#: (?Integer x) -> void
#    ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest positional`, but RBS signature declares `optional positional`
def rest_pos4(*x); end

#: (*Integer) -> void
def rest_pos5(*x); end

#: (*Integer x) -> void
def rest_pos6(*x); end

#: (x: Integer) -> void
#   ^^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest positional`, but RBS signature declares `keyword`
def rest_pos7(*x); end

#: (?x: Integer) -> void
#    ^^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest positional`, but RBS signature declares `optional keyword`
def rest_pos8(*x); end

#: (**Integer) -> void
#     ^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest positional`, but RBS signature declares `rest keyword`
def rest_pos9(*x); end

#: (**Integer x) -> void
#     ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest positional`, but RBS signature declares `rest keyword`
def rest_pos10(*x); end

# correct to required keyword

#: (Integer) -> void
#   ^^^^^^^ error: Argument kind mismatch for `x`, method declares `keyword`, but RBS signature declares `positional`
def req_kw1(x:); end

#: (Integer x) -> void
#   ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `keyword`, but RBS signature declares `positional`
def req_kw2(x:); end

#: (?Integer) -> void
#    ^^^^^^^ error: Argument kind mismatch for `x`, method declares `keyword`, but RBS signature declares `optional positional`
def req_kw3(x:); end

#: (?Integer x) -> void
#    ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `keyword`, but RBS signature declares `optional positional`
def req_kw4(x:); end

#: (*Integer) -> void
#    ^^^^^^^ error: Argument kind mismatch for `x`, method declares `keyword`, but RBS signature declares `rest positional`
def req_kw5(x:); end

#: (*Integer x) -> void
#    ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `keyword`, but RBS signature declares `rest positional`
def req_kw6(x:); end

#: (x: Integer) -> void
def req_kw7(x:); end

#: (?x: Integer) -> void
#    ^^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `keyword`, but RBS signature declares `optional keyword`
def req_kw8(x:); end

#: (**Integer) -> void
#     ^^^^^^^ error: Argument kind mismatch for `x`, method declares `keyword`, but RBS signature declares `rest keyword`
def req_kw9(x:); end

#: (**Integer x) -> void
#     ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `keyword`, but RBS signature declares `rest keyword`
def req_kw10(x:); end

# correct to optional keyword

#: (Integer) -> void
#   ^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional keyword`, but RBS signature declares `positional`
def opt_kw1(x: 42); end

#: (Integer x) -> void
#   ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional keyword`, but RBS signature declares `positional`
def opt_kw2(x: 42); end

#: (?Integer) -> void
#    ^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional keyword`, but RBS signature declares `optional positional`
def opt_kw3(x: 42); end

#: (?Integer x) -> void
#    ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional keyword`, but RBS signature declares `optional positional`
def opt_kw4(x: 42); end

#: (*Integer) -> void
#    ^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional keyword`, but RBS signature declares `rest positional`
def opt_kw5(x: 42); end

#: (*Integer x) -> void
#    ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional keyword`, but RBS signature declares `rest positional`
def opt_kw6(x: 42); end

#: (x: Integer) -> void
#   ^^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional keyword`, but RBS signature declares `keyword`
def opt_kw7(x: 42); end

#: (?x: Integer) -> void
def opt_kw8(x: 42); end

#: (**Integer) -> void
#     ^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional keyword`, but RBS signature declares `rest keyword`
def opt_kw9(x: 42); end

#: (**Integer x) -> void
#     ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `optional keyword`, but RBS signature declares `rest keyword`
def opt_kw10(x: 42); end

# correct to rest keyword

#: (Integer) -> void
#   ^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest keyword`, but RBS signature declares `positional`
def rest_kw1(**x); end

#: (Integer x) -> void
#   ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest keyword`, but RBS signature declares `positional`
def rest_kw2(**x); end

#: (?Integer) -> void
#    ^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest keyword`, but RBS signature declares `optional positional`
def rest_kw3(**x); end

#: (?Integer x) -> void
#    ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest keyword`, but RBS signature declares `optional positional`
def rest_kw4(**x); end

#: (*Integer) -> void
#    ^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest keyword`, but RBS signature declares `rest positional`
def rest_kw5(**x); end

#: (*Integer x) -> void
#    ^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest keyword`, but RBS signature declares `rest positional`
def rest_kw6(**x); end

#: (x: Integer) -> void
#   ^^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest keyword`, but RBS signature declares `keyword`
def rest_kw7(**x); end

#: (?x: Integer) -> void
#    ^^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `rest keyword`, but RBS signature declares `optional keyword`
def rest_kw8(**x); end

#: (**Integer) -> void
def rest_kw9(**x); end

#: (**Integer x) -> void
def rest_kw10(**x); end

# correct multiple

#: (?Integer, y: Integer, **Integer) -> void
#    ^^^^^^^ error: Argument kind mismatch for `x`, method declares `positional`, but RBS signature declares `optional positional`
#             ^^^^^^^^^^ error: Argument kind mismatch for `y`, method declares `optional positional`, but RBS signature declares `keyword`
#                           ^^^^^^^ error: Argument kind mismatch for `z`, method declares `rest positional`, but RBS signature declares `rest keyword`
def multiple1(x, y = 42, *z); end

# do not correct blocks

#: (^-> void) -> void
#   ^^^^^^^^ error: Argument kind mismatch for `x`, method declares `block`, but RBS signature declares `positional`
def block1(&x); end

#: { -> void } -> void
#  ^^^^^^^^^^^ error: Argument kind mismatch for `x`, method declares `positional`, but RBS signature declares `block`
def block2(x); end
