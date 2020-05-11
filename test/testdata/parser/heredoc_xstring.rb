# typed: true

p <<~`E`
      pwd
  E

puts <<~`E`
    a
    rather
  long command
        with bad
    indentation
    E
