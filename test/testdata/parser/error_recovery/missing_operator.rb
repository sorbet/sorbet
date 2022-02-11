# typed: false

module Opus::Log
  def self.info; end
end

class A
  def g
    t = T.let(true, T::Boolean)
    f = T.let(false, T::Boolean)
    puts(0 .. 0)
    puts(0 .. )
    if 0 .. # error: unexpected token "if"
    end
    if 0 .. # error: Unsupported node type `IFlipflop`
      #  ^^ error: missing arg to ".." operator
      puts 'hello'
    end
    puts(1 ... 1)
    puts(1 ... )
    if 1... # error: unexpected token "if"
    end
    if 1... # error: Unsupported node type `EFlipflop`
      # ^^^ error: missing arg to "..." operator
      puts 'hello'
    end
    puts(2 + 2)
    puts(2 + ) # error: missing arg to "+" operator
    if 2 + # error: missing arg to "+" operator
    end
    if 2 + # error: missing arg to "+" operator
      puts 'hello'
    end
    puts(3 - 3)
    puts(3 - ) # error: missing arg to "-" operator
    if 3 - # error: missing arg to "-" operator
    end
    if 3 - # error: missing arg to "-" operator
      puts 'hello'
    end
    puts(4 * 4)
    puts(4 * ) # error: missing arg to "*" operator
    if 4 * # error: missing arg to "*" operator
    end
    if 4 * # error: missing arg to "*" operator
      puts 'hello'
    end
    puts(5 / 5)
    puts(5 / ) # error: missing arg to "/" operator
    if 5 / # error: missing arg to "/" operator
    end
    if 5 / # error: missing arg to "/" operator
      puts 'hello'
    end
    puts(6 % 6)
    puts(6 % ) # error: missing arg to "%" operator
    if 6 % # error: missing arg to "%" operator
    end
    if 6 % # error: missing arg to "%" operator
      puts 'hello'
    end
    puts(7 ** 7)
    puts(7 ** ) # error: missing arg to "**" operator
    if 7 ** # error: missing arg to "**" operator
    end
    if 7 ** # error: missing arg to "**" operator
      puts 'hello'
    end
    puts(-8 ** 8)
    puts(-8 ** ) # error: missing arg to "**" operator
    if -8 ** # error: missing arg to "**" operator
    end
    if -8 ** # error: missing arg to "**" operator
      puts 'hello'
    end
    puts(+9 ** 9)
    puts(+9 ** ) # error: missing arg to "**" operator
    if +9 ** # error: missing arg to "**" operator
    end
    if +9 ** # error: missing arg to "**" operator
      puts 'hello'
    end
    puts(-10)
    puts(-) # error: missing arg to "unary -" operator
    if - # error: missing arg to "unary -" operator
    end
    if - # error: missing arg to "unary -" operator
      puts 'hello'
    end
    puts(+10)
    puts(+) # error: missing arg to "unary +" operator
    if + # error: missing arg to "unary +" operator
    end
    if + # error: missing arg to "unary +" operator
      puts 'hello'
    end
    puts(10 | 10)
    puts(10 | ) # error: missing arg to "|" operator
    if 10 | # error: missing arg to "|" operator
    end
    if 10 | # error: missing arg to "|" operator
      puts 'hello'
    end
    puts(11 ^ 11)
    puts(11 ^ ) # error: missing arg to "^" operator
    if 11 ^ # error: missing arg to "^" operator
    end
    if 11 ^ # error: missing arg to "^" operator
      puts 'hello'
    end
    puts(12 & 12)
    puts(12 & ) # error: missing arg to "&" operator
    if 12 & # error: missing arg to "&" operator
    end
    if 12 & # error: missing arg to "&" operator
      puts 'hello'
    end
    puts(13 <=> 13)
    puts(13 <=> ) # error: missing arg to "<=>" operator
    if 13 <=> # error: missing arg to "<=>" operator
    end
    if 13 <=> # error: missing arg to "<=>" operator
      puts 'hello'
    end
    puts(14 == 14)
    puts(14 == ) # error: missing arg to "==" operator
    if 14 == # error: missing arg to "==" operator
    end
    if 14 == # error: missing arg to "==" operator
      puts 'hello'
    end
    puts(15 === 15)
    puts(15 === ) # error: missing arg to "===" operator
    if 15 === # error: missing arg to "===" operator
    end
    if 15 === # error: missing arg to "===" operator
      puts 'hello'
    end
    puts(16 != 16)
    puts(16 != ) # error: missing arg to "!=" operator
    if 16 != # error: missing arg to "!=" operator
    end
    if 16 != # error: missing arg to "!=" operator
      puts 'hello'
    end
    puts(/17/ =~ "17")
    puts(/17/ =~ ) # error: missing arg to "=~" operator
    if /17/ =~ # error: missing arg to "=~" operator
    end
    if /17/ =~ # error: missing arg to "=~" operator
      puts 'hello'
    end
    puts(/18/ !~ "eighteen")
    puts(/18/ !~ ) # error: missing arg to "!~" operator
    if /18/ !~ # error: missing arg to "!~" operator
    end
    if /18/ !~ # error: missing arg to "!~" operator
      puts 'hello'
    end
    puts(!t)
    puts(!) # error: missing arg to "!" operator
    if ! # error: missing arg to "!" operator
    end
    if !
      puts 'hello'
    end
    puts(~19)
    puts(~) # error: missing arg to "~" operator
    if ~ # error: missing arg to "~" operator
    end
    if ~ # error: missing arg to "~" operator
      puts 'hello'
    end
    puts(20 << 20)
    puts(20 << ) # error: missing arg to "<<" operator
    if 20 << # error: missing arg to "<<" operator
    end
    if 20 << # error: missing arg to "<<" operator
      puts 'hello'
    end
    puts(21 >> 21)
    puts(21 >> ) # error: missing arg to ">>" operator
    if 21 >> # error: missing arg to ">>" operator
    end
    if 21 >> # error: missing arg to ">>" operator
      puts 'hello'
    end
    puts(t && t)
    puts(t && ) # error: missing arg to "&&" operator
    if t && # error: missing arg to "&&" operator
    end
    if t && # error: missing arg to "&&" operator
      puts 'hello'
    end
    puts(f || f)
    puts(f || ) # error: missing arg to "||" operator
    if f || # error: missing arg to "||" operator
    end
    if f || # error: missing arg to "||" operator
      puts 'hello'
    end
    puts(24 > 24)
    puts(24 > ) # error: missing arg to ">" operator
    if 24 > # error: missing arg to ">" operator
    end
    if 24 > # error: missing arg to ">" operator
      puts 'hello'
    end
    puts(25 < 25)
    puts(25 < ) # error: missing arg to "<" operator
    if 25 < # error: missing arg to "<" operator
    end
    if 25 < # error: missing arg to "<" operator
      puts 'hello'
    end
    puts(26 >= 26)
    puts(26 >= ) # error: missing arg to ">=" operator
    if 26 >= # error: missing arg to ">=" operator
    end
    if 26 >= # error: missing arg to ">=" operator
      puts 'hello'
    end
    puts(27 <= 27)
    puts(27 <= ) # error: missing arg to "<=" operator
    if 27 <= # error: missing arg to "<=" operator
    end
    if 27 <= # error: missing arg to "<=" operator
      puts 'hello'
    end
  end # error: unexpected token "end"
end
