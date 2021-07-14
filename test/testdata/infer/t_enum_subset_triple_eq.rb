# typed: true

extend T::Sig

# (1) Define an interface / module
module DayOfWeek
  extend T::Helpers
  sealed!
end

class Weekday < T::Enum
  # (2) include DayOfWeek when defining the Weekday enum
  include DayOfWeek

  enums do
    Monday = new
    Tuesday = new
    Wednesday = new
    Thursday = new
    Friday = new
  end
end

class Weekend < T::Enum
  # (3) ditto
  include DayOfWeek

  enums do
    Saturday = new
    Sunday = new
  end
end

sig {params(d: DayOfWeek, wk: Weekday, wn: Weekend).void}
def f(d, wk, wn)
  case d
  when Weekday::Monday
    T.reveal_type(Weekday::Monday===d)     # error: Revealed type: `TrueClass`
    T.reveal_type(d===Weekday::Monday)     # error: Revealed type: `TrueClass`
    T.reveal_type(wk===d)                  # error: Revealed type: `T::Boolean`
    T.reveal_type(d===wk)                  # error: Revealed type: `T::Boolean`
    T.reveal_type(wn===d)                  # error: Revealed type: `FalseClass`
    T.reveal_type(d===wn)                  # error: Revealed type: `FalseClass`
  when Weekend
    T.reveal_type(Weekday::Monday===d)     # error: Revealed type: `FalseClass`
    T.reveal_type(d===Weekday::Monday)     # error: Revealed type: `FalseClass`
    T.reveal_type(wk===d)                  # error: Revealed type: `FalseClass`
    T.reveal_type(d===wk)                  # error: Revealed type: `FalseClass`
    T.reveal_type(wn===d)                  # error: Revealed type: `T::Boolean`
    T.reveal_type(d===wn)                  # error: Revealed type: `T::Boolean`
    case d
    when Weekend::Saturday
      T.reveal_type(Weekday::Monday===d)   # error: Revealed type: `FalseClass`
      T.reveal_type(Weekend::Saturday===d) # error: Revealed type: `TrueClass`
      T.reveal_type(Weekend::Sunday===d)   # error: Revealed type: `FalseClass`
      T.reveal_type(wk===d)                # error: Revealed type: `FalseClass`
      T.reveal_type(wn===d)                # error: Revealed type: `T::Boolean`
    end
  end
end
