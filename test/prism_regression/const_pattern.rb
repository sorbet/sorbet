# typed: false

case nil
in      [          ] then "Empty ArrayPattern"
in      [1, 2, 3, 4] then "ArrayPattern"

in Point[          ] then "ConstPattern with nested empty ArrayPattern"
in Point[1, 2, 3, 4] then "ConstPattern with nested ArrayPattern"

in Point(          ) then "ConstPattern with nested empty ArrayPattern"
in Point(1, 2, 3, 4) then "ConstPattern with nested ArrayPattern"

in       x: Integer, y: Integer  then "HashPattern with 2 pairs"
in      {x: Integer, y: Integer} then "HashPattern with 2 pairs"
in Point[x: Integer, y: Integer] then "ConstPattern with nested HashPattern with 2 pairs"
in Point(x: Integer, y: Integer) then "ConstPattern with nested HashPattern with 2 pairs"

end
