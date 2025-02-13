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
    if 0 .. # parser-error: unexpected token "if"
    end
    if 0 .. # error: Unsupported node type `IFlipflop`
      #  ^^ parser-error: missing arg to ".." operator
      puts 'hello'
    end
    puts(1 ... 1)
    puts(1 ... )
    if 1... # parser-error: unexpected token "if"
    end
    if 1... # error: Unsupported node type `EFlipflop`
      # ^^^ parser-error: missing arg to "..." operator
      puts 'hello'
    end
    puts(2 + 2)
    puts(2 + ) # parser-error: missing arg to "+" operator
    if 2 + # parser-error: missing arg to "+" operator
    end
    if 2 + # parser-error: missing arg to "+" operator
      puts 'hello'
    end
    puts(3 - 3)
    puts(3 - ) # parser-error: missing arg to "-" operator
    if 3 - # parser-error: missing arg to "-" operator
    end
    if 3 - # parser-error: missing arg to "-" operator
      puts 'hello'
    end
    puts(4 * 4)
    puts(4 * ) # parser-error: missing arg to "*" operator
    if 4 * # parser-error: missing arg to "*" operator
    end
    if 4 * # parser-error: missing arg to "*" operator
      puts 'hello'
    end
    puts(5 / 5)
    puts(5 / ) # parser-error: missing arg to "/" operator
    if 5 / # parser-error: missing arg to "/" operator
    end
    if 5 / # parser-error: missing arg to "/" operator
      puts 'hello'
    end
    puts(6 % 6)
    puts(6 % ) # parser-error: missing arg to "%" operator
    if 6 % # parser-error: missing arg to "%" operator
    end
    if 6 % # parser-error: missing arg to "%" operator
      puts 'hello'
    end
    puts(7 ** 7)
    puts(7 ** ) # parser-error: missing arg to "**" operator
    if 7 ** # parser-error: missing arg to "**" operator
    end
    if 7 ** # parser-error: missing arg to "**" operator
      puts 'hello'
    end
    puts(-8 ** 8)
    puts(-8 ** ) # parser-error: missing arg to "**" operator
    if -8 ** # parser-error: missing arg to "**" operator
    end
    if -8 ** # parser-error: missing arg to "**" operator
      puts 'hello'
    end
    puts(+9 ** 9)
    puts(+9 ** ) # parser-error: missing arg to "**" operator
    if +9 ** # parser-error: missing arg to "**" operator
    end
    if +9 ** # parser-error: missing arg to "**" operator
      puts 'hello'
    end
    puts(-10)
    puts(-) # parser-error: missing arg to "unary -" operator
    if - # parser-error: missing arg to "unary -" operator
    end
    if - # parser-error: missing arg to "unary -" operator
      puts 'hello'
    end
    puts(+10)
    puts(+) # parser-error: missing arg to "unary +" operator
    if + # parser-error: missing arg to "unary +" operator
    end
    if + # parser-error: missing arg to "unary +" operator
      puts 'hello'
    end
    puts(10 | 10)
    puts(10 | ) # parser-error: missing arg to "|" operator
    if 10 | # parser-error: missing arg to "|" operator
    end
    if 10 | # parser-error: missing arg to "|" operator
      puts 'hello'
    end
    puts(11 ^ 11)
    puts(11 ^ ) # parser-error: missing arg to "^" operator
    if 11 ^ # parser-error: missing arg to "^" operator
    end
    if 11 ^ # parser-error: missing arg to "^" operator
      puts 'hello'
    end
    puts(12 & 12)
    puts(12 & ) # parser-error: missing arg to "&" operator
    if 12 & # parser-error: missing arg to "&" operator
    end
    if 12 & # parser-error: missing arg to "&" operator
      puts 'hello'
    end
    puts(13 <=> 13)
    puts(13 <=> ) # parser-error: missing arg to "<=>" operator
    if 13 <=> # parser-error: missing arg to "<=>" operator
    end
    if 13 <=> # parser-error: missing arg to "<=>" operator
      puts 'hello'
    end
    puts(14 == 14)
    puts(14 == ) # parser-error: missing arg to "==" operator
    if 14 == # parser-error: missing arg to "==" operator
    end
    if 14 == # parser-error: missing arg to "==" operator
      puts 'hello'
    end
    puts(15 === 15)
    puts(15 === ) # parser-error: missing arg to "===" operator
    if 15 === # parser-error: missing arg to "===" operator
    end
    if 15 === # parser-error: missing arg to "===" operator
      puts 'hello'
    end
    puts(16 != 16)
    puts(16 != ) # parser-error: missing arg to "!=" operator
    if 16 != # parser-error: missing arg to "!=" operator
    end
    if 16 != # parser-error: missing arg to "!=" operator
      puts 'hello'
    end
    puts(/17/ =~ "17")
    puts(/17/ =~ ) # parser-error: missing arg to "=~" operator
    if /17/ =~ # parser-error: missing arg to "=~" operator
    end
    if /17/ =~ # parser-error: missing arg to "=~" operator
      puts 'hello'
    end
    puts(/18/ !~ "eighteen")
    puts(/18/ !~ ) # parser-error: missing arg to "!~" operator
    if /18/ !~ # parser-error: missing arg to "!~" operator
    end
    if /18/ !~ # parser-error: missing arg to "!~" operator
      puts 'hello'
    end
    puts(!t)
    puts(!) # parser-error: missing arg to "!" operator
    if ! # parser-error: missing arg to "!" operator
    end
    if !
      puts 'hello'
    end
    puts(~19)
    puts(~) # parser-error: missing arg to "~" operator
    if ~ # parser-error: missing arg to "~" operator
    end
    if ~ # parser-error: missing arg to "~" operator
      puts 'hello'
    end
    puts(20 << 20)
    puts(20 << ) # parser-error: missing arg to "<<" operator
    if 20 << # parser-error: missing arg to "<<" operator
    end
    if 20 << # parser-error: missing arg to "<<" operator
      puts 'hello'
    end
    puts(21 >> 21)
    puts(21 >> ) # parser-error: missing arg to ">>" operator
    if 21 >> # parser-error: missing arg to ">>" operator
    end
    if 21 >> # parser-error: missing arg to ">>" operator
      puts 'hello'
    end
    puts(t && t)
    puts(t && ) # parser-error: missing arg to "&&" operator
    if t && # parser-error: missing arg to "&&" operator
    end
    if t && # parser-error: missing arg to "&&" operator
      puts 'hello'
    end
    puts(f || f)
    puts(f || ) # parser-error: missing arg to "||" operator
    if f || # parser-error: missing arg to "||" operator
    end
    if f || # parser-error: missing arg to "||" operator
      puts 'hello'
    end
    puts(24 > 24)
    puts(24 > ) # parser-error: missing arg to ">" operator
    if 24 > # parser-error: missing arg to ">" operator
    end
    if 24 > # parser-error: missing arg to ">" operator
      puts 'hello'
    end
    puts(25 < 25)
    puts(25 < ) # parser-error: missing arg to "<" operator
    if 25 < # parser-error: missing arg to "<" operator
    end
    if 25 < # parser-error: missing arg to "<" operator
      puts 'hello'
    end
    puts(26 >= 26)
    puts(26 >= ) # parser-error: missing arg to ">=" operator
    if 26 >= # parser-error: missing arg to ">=" operator
    end
    if 26 >= # parser-error: missing arg to ">=" operator
      puts 'hello'
    end
    puts(27 <= 27)
    puts(27 <= ) # parser-error: missing arg to "<=" operator
    if 27 <= # parser-error: missing arg to "<=" operator
    end
    if 27 <= # parser-error: missing arg to "<=" operator
      puts 'hello'
    end
  end # parser-error: unexpected token "end"
end
