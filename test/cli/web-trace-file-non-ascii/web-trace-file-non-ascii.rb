# typed: true

class HundredValEnum < T::Enum
  enums do
    Val001 = new
    Val002 = new
    Val003 = new
    Val004 = new
    Val005 = new
    Val006 = new
    Val007 = new
    Val008 = new
    Val009 = new
    Val010 = new
    Val011 = new
    Val012 = new
    Val013 = new
    Val014 = new
    Val015 = new
    Val016 = new
    Val017 = new
    Val018 = new
    Val019 = new
    Val020 = new
  end

  # This string literal contains U+2019, which is a non-ascii character.
  # We used to essentially have something like this at Stripe,
  # and it would crash us when trying to serialize a web trace file
  # (because the method name was in one of the args)
  it '’' do
    # The enum + case are here just to make sure that typechecking this method
    # takes longer than the threshold below which we throw away timers.
    case self
    when Val001 then 1001
    when Val002 then 1002
    when Val003 then 1003
    when Val004 then 1004
    when Val005 then 1005
    when Val006 then 1006
    when Val007 then 1007
    when Val008 then 1008
    when Val009 then 1009
    when Val010 then 1010
    when Val011 then 1011
    when Val012 then 1012
    when Val013 then 1013
    when Val014 then 1014
    when Val015 then 1015
    when Val016 then 1016
    when Val017 then 1017
    when Val018 then 1018
    when Val019 then 1019
    when Val020 then 1020
    else
      T.absurd(self)
    end
  end
end

