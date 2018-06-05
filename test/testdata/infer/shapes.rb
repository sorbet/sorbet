# typed: true
x = {
  str: "hi",
  'int' => 17,
  sym: :foo,
}

T.assert_type!(x[:str], String)
T.assert_type!(x['int'], Integer)
T.assert_type!(x[:sym], Symbol)
T.assert_type!(x[1.0], NilClass)
