# typed: strict
class TestAttr
  extend T::Sig

  sig {returns(String).checked(:always)}
  attr_accessor :str1

  sig {returns(String).checked(:always)}
  attr_reader :str2

  sig {params(str3:String).returns(String).checked(:always)}
  attr_writer :str3

  sig {returns(String).on_failure(:raise)}
  attr_accessor :str4

  sig {returns(String).checked(:always).on_failure(:raise)}
  attr_accessor :str5

  sig {void.checked(:always)}
  attr_reader :str6

  sig {params(str7:String).void.checked(:always).on_failure(:raise)}
  attr_writer :str7

  sig {void.on_failure(:raise)}
  attr_accessor :str8 # error: The method `str8=` does not have a `sig`

  sig {params(str9: T.nilable(String)).returns(T.nilable(String)).checked(:never)}
  attr_writer :str9

  sig {void}
  def initialize
    @str1 = T.let('', String)
    @str2 = T.let('', String)
    @str3 = T.let('', String)
    @str4 = T.let('', String)
    @str5 = T.let('', String)
    @str6 = T.let('', String)
    @str7 = T.let('', String)
    @str8 = T.let('', String)
    @str9 = ''
  end

end
