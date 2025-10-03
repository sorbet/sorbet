# typed: true
extend T::Sig

class Account
  attr_reader :account_id
end

class A::B::C::D
  extend T::Sig

  sig { params(account: T.any(String, Account)).void }
  def self.example(account:)
    account
  end

  example(account: 0)
  #                ^ error: Expected `T.any(String, Account)`
  #         ^ hover-line: 2 # A::B::C::D.example
  #         ^ hover-line: 3 (kwparam) account: T.any(String, Account)

  account = ""
  example(account:)
  #         ^ hover: String("")

  # Imperfect, does not reimplement calls.cc's hash literal -> kwparam logic
  example({account: ""})
  #         ^ hover: Symbol(:account)

  example(:account)
  #       ^^^^^^^^ error: Too many positional arguments
  #               ^ error: Missing required keyword argument `account`
  #         ^ hover: Symbol(:account)

  example("account")
  #       ^^^^^^^^^ error: Too many positional arguments
  #                ^ error: Missing required keyword argument `account`
  #         ^ hover: String("account")

  example("account" => 0)
  #       ^^^^^^^^^^^^^^ error: method has required keyword parameters
  #         ^ hover: String("account")

  example(does_not_exist: 0)
  #                        ^ error: Missing required keyword argument `account`
  #       ^^^^^^^^^^^^^^^^^ error: Unrecognized keyword argument `does_not_exist`
  #         ^ hover: Symbol(:does_not_exist)

  def self.takes_untyped(arg0:)
  end

  takes_untyped(arg0: "hello, world")
  #              ^ hover-line: 2 # A::B::C::D.takes_untyped
  #              ^ hover-line: 3 (kwparam) arg0: T.untyped

  def self.kwargs_example(**kwargs)
  end

  kwargs_example(kwargs: 0)
  #               ^ hover: Symbol(:kwargs)
end

class A::B::C::E
  extend T::Sig

  sig { params(account: T.any(String, Account)).void }
  #            ^ def: account-other 1 not-def-of-self
  def self.example(account:)
    account
  end
end

class Other
  extend T::Sig

  sig { params(account: String).void }
  def self.example(account:)
    account
  end
end

sig {
  params(x: T.any(T.class_of(A::B::C::E), T.class_of(Other))).void
}
def takes_any(x)
  x.example(account: '')
  #           ^ hover-line: 1 ```ruby
  #           ^ hover-line: 2 # A::B::C::E.example
  #           ^ hover-line: 3 (kwparam) account: T.any(String, Account)
  #           ^ hover-line: 4 # Other.example
  #           ^ hover-line: 5 (kwparam) account: String
  #           ^ hover-line: 6 ```
end
