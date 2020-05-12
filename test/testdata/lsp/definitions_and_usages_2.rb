# typed: true
module Bar
     # ^ def: Bar
  CONST = 2
  # ^ def: CONST

  class Steel
      # ^ def: Steel
    def meth(y)
      # ^ def: meth
      y
    end

    def self.meth(x)
           # ^ def: self_meth
                # ^ def: x
      x
    # ^ usage: x
    end
  end
end

local = 131
# ^ def: local
localer = local + 2
# ^ def: localer
        # ^ usage: local
localer2 = localer + 2
# ^ def: localer2
          # ^ usage: localer
local3 = localer + local + 2
# ^ def: local3
        # ^ usage: localer
                   # ^ usage: local
const_to_local = Bar::CONST;
               # ^ usage: Bar
                     # ^ usage: CONST
const_add = Bar::CONST + local
          # ^ usage: Bar
                 # ^ usage: CONST
                         # ^ usage: local
const_add_reverse = local + Bar::CONST
                   # ^ usage: local
                          # ^ usage: Bar
                                 # ^ usage: CONST

Bar::Steel.meth(local)
# ^ usage: Bar
   # ^ usage: Steel
         # ^ usage: self_meth
              # ^ usage: local
puts(Bar::CONST)
   # ^ usage: Bar
          # ^ usage: CONST

Bar::Steel.new.meth(10)
# ^ usage: Bar
   # ^ usage: Steel
             # ^ usage: meth
