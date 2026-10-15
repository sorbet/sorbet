# typed: true

String::RejectedClass # error: Unable to resolve constant `RejectedClass`
String::REJECTED_FIELD # error: Unable to resolve constant `REJECTED_FIELD`
String::ValidClass
T.let(String::VALID_FIELD, Integer)
