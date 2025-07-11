# typed: strict
# enable-experimental-rbs-comments: true

# let

letErr1 = ARGV #: Int
               #  ^^^ error: Unable to resolve constant `Int`

letErr2 = ARGV #: -
               #  ^ error: Failed to parse RBS type (unexpected token for simple type)

letErr3 = ARGV #:
               # ^ error: Failed to parse RBS type (unexpected token for simple type)

letErr4 = ARGV #: Integer
               #  ^^^^^^^ error: Argument does not have asserted type `Integer`

letErr5 = ARGV #:Integer
               # ^^^^^^^ error: Argument does not have asserted type `Integer`

letErr6 = ARGV#:Integer
              # ^^^^^^^ error: Argument does not have asserted type `Integer`

# cast

castErr1 = ARGV.first #: as
                      #  ^^ error: Failed to parse RBS type (unexpected token for simple type)

castErr2 = ARGV.first #: as -> String
                      #     ^^ error: Failed to parse RBS type (unexpected token for simple type)

castErr3 = ARGV.first #: as Int
                      #     ^^^ error: Unable to resolve constant `Int`
                      #     ^^^ error: Please use `T.unsafe` to cast to `T.untyped`

ARGV #:
#      ^ error: Failed to parse RBS type (unexpected token for simple type)

ARGV #: as
#       ^^ error: Failed to parse RBS type (unexpected token for simple type)

ARGV #: !nil
#       ^ error: Failed to parse RBS type (unexpected token for simple type)
