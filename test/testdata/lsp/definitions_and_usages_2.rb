# typed: true
module Bar
  CONST = 2
  # ^ def: CONST
  
  def self.meth(x)
    x
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
                     # ^ usage: CONST
const_add = Bar::CONST + local
                 # ^ usage: CONST
                         # ^ usage: local
const_add_reverse = local + Bar::CONST
                   # ^ usage: local
                                 # ^ usage: CONST

Bar.meth(local)
        # ^ usage: local
puts(Bar::CONST)
          # ^ usage: CONST
