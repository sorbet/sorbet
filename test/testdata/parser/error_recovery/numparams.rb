# typed: true

[1,2,3].map { |x| _1 }
                # ^^ error: can't use numbered params when ordinary

# recover
successful_parse = 10

[1,2,3].map { _1.times { _1 }}
                       # ^^ error: numbered parameter is already used in an outer scope
