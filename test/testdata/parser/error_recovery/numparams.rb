# typed: true

[1,2,3].map { |x| _1 }
                # ^^ error: numbered parameters are not allowed when an ordinary parameter is defined

# recover
successful_parse = 10

[1,2,3].map { _1.times { _1 }}
                       # ^^ error: numbered parameter is already used in an outer scope
