# typed: false

class A
  def g
    t = T.let(true, T::Boolean)
    f = T.let(false, T::Boolean)
    puts (0 .. 0)
    puts (0 .. def) # error: unexpected token ")"
    puts (1 ... 1)
    puts (1 ... def) # error: unexpected token ")"
    puts (2 + 2)
    puts (2 + def) # error: unexpected token ")"
    puts (2 +) # error: unexpected token ")"
    puts (3 - 3)
    puts (3 - def) # error: unexpected token ")"
    puts (3 -) # error: unexpected token ")"
    puts (4 * 4)
    puts (4 * def) # error: unexpected token ")"
    puts (4 *) # error: unexpected token ")"
    puts (5 / 5)
    puts (5 / def) # error: unexpected token ")"
    puts (5 /) # error: unexpected token ")"
    puts (6 % 6)
    puts (6 % def) # error: unexpected token ")"
    puts (6 % ) # error: unexpected token ")"
    puts (7 ** 7)
    puts (7 ** def) # error: unexpected token ")"
    puts (7 **) # error: unexpected token ")"
    puts (-8 ** 8)
    puts (-8 ** def) # error: unexpected token ")"
    puts (-8 **) # error: unexpected token ")"
    puts (+9 ** 9)
    puts (+9 ** def) # error: unexpected token ")"
    puts (+9 **) # error: unexpected token ")"
    ten = 10
    puts (-ten)
    puts (-def) # error: unexpected token ")"
    puts (-) # error: unexpected token ")"
    puts (+def) # error: unexpected token ")"
    puts (+) # error: unexpected token ")"
    puts (+ten)
    puts (10 | 10)
    puts (10 | def) # error: unexpected token ")"
    puts (10 |) # error: unexpected token ")"
    puts (11 ^ 11)
    puts (11 ^ def) # error: unexpected token ")"
    puts (11 ^) # error: unexpected token ")"
    puts (12 & 12)
    puts (12 & def) # error: unexpected token ")"
    puts (12 &) # error: unexpected token ")"
    puts (13 <=> 13)
    puts (13 <=> def) # error: unexpected token ")"
    puts (13 <=>) # error: unexpected token ")"
    puts (14 == 14)
    puts (14 == def) # error: unexpected token ")"
    puts (14 ==) # error: unexpected token ")"
    puts (15 === 15)
    puts (15 === def) # error: unexpected token ")"
    puts (15 ===) # error: unexpected token ")"
    puts (16 != 16)
    puts (16 != def) # error: unexpected token ")"
    puts (16 !=) # error: unexpected token ")"
    puts (/17/ =~ "17")
    puts (/17/ =~ def) # error: unexpected token ")"
    puts (/17/ =~) # error: unexpected token ")"
    puts (/18/ !~ "eighteen")
    puts (/18/ !~ def) # error: unexpected token ")"
    puts (/18/ !~) # error: unexpected token ")"
    puts (!t)
    puts (!def) # error: unexpected token ")"
    puts (!) # error: unexpected token ")"
    nineteen = 19
    puts (~nineteen)
    puts (~def) # error: unexpected token ")"
    puts (~) # error: unexpected token ")"
    puts (20 << 20)
    puts (20 << def) # error: unexpected token ")"
    puts (20 <<) # error: unexpected token ")"
    puts (21 >> 21)
    puts (21 >> def) # error: unexpected token ")"
    puts (21 >>) # error: unexpected token ")"
    puts (t && t)
    puts (t && def) # error: unexpected token ")"
    puts (t &&) # error: unexpected token ")"
    puts (f || f)
    puts (f || def) # error: unexpected token ")"
    puts (f ||) # error: unexpected token ")"
    puts (24 > 24)
    puts (24 > def) # error: unexpected token ")"
    puts (24 >) # error: unexpected token ")"
    puts (25 < 25)
    puts (25 < def) # error: unexpected token ")"
    puts (25 <) # error: unexpected token ")"
    puts (26 >= 26)
    puts (26 >= def) # error: unexpected token ")"
    puts (26 >=) # error: unexpected token ")"
    puts (27 <= 27)
    puts (27 <= def) # error: unexpected token ")"
    puts (27 <=) # error: unexpected token ")"
  end
end
