# typed: strict

# Note: We use versions below to denote the field (2) and method (1) defined by the accessor
class Item < T::Struct
  extend T::Sig

  const :make, String
# ^^^^^^^^^^^^^^^^^^^ def: make 1
  #      ^^^^ def: make 2


  prop :model, String
# ^^^^^^^^^^^^^^^^^^^ def: model 1
  #     ^^^^^ def: model 2

  sig {void}
  def write_accessors
    self.model = "newmodel"
  #      ^^^^^ usage: model 1
  end

  sig {void}
  def read_accessors
    self.make
  #      ^^^^ usage: make 1
    self.model
  #      ^^^^^ usage: model 1
  end

  sig {void}
  def read_fields
    @make
    #^^^^ usage: make 2
    @model
    #^^^^^ usage: model 2
  end
end

class ItemMetadata
  extend T::Sig
  
  sig {returns(String)}
  attr_reader :serial
# ^^^^^^^^^^^^^^^^^^^ def: serial 1
  #            ^^^^^^ def: serial 2

  sig {params(owner: String).returns(String)}
  attr_writer :owner
# ^^^^^^^^^^^^^^^^^^ def: owner 1
  #            ^^^^^ def: owner 2

  sig {returns(String)}
  attr_accessor :region
# ^^^^^^^^^^^^^^^^^^^^^ def: region 1
  #              ^^^^^^ def: region 2

  sig {void}
  def initialize
    @serial = T.let("", String)
    #^^^^^^ usage: serial 2
    @owner = T.let("", String)
    #^^^^^ usage: owner 2
    @region = T.let("", String)
    #^^^^^^ usage: region 2
  end

  sig {void}
  def write_accessors
    self.owner = "garfield"
  #      ^^^^^ usage: owner 1
    self.region = "USA"
  #      ^^^^^^ usage: region 1
  end

  sig {void}
  def read_accessors
    self.serial
  #      ^^^^^^ usage: serial 1
    self.region
  #      ^^^^^^ usage: region 1
  end

  sig {void}
  def read_fields
    @serial
    #^^^^^^ usage: serial 2
    @owner
    #^^^^^ usage: owner 2
    @region
    #^^^^^^ usage: region 2
  end
end

