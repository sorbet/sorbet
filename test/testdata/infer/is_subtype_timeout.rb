# typed: true
#
module Half; end;

class A01; end
class A02; end
class A03; end
class A04; end
class A05; end
class A06; end
class A07; end
class A08; end
class A09; end
class A10; end
class A11; end
class A12; end
class A13; end
class A14; end
class A15; end
class A16; end
class A17; end
class A18; end
class A19; end
class A20; end
class A21; end
class A22; end
class A23; end
class A24; end
class A25; end
class A26; end
class A27; end
class A28; end
class A29; end
class A30; end
class A31; end
class A32; end
class A33; end
class A34; end
class A35; end
class A36; end
class A37; end
class A38; end
class A39; end
class A40; end
class A41; end
class A42; end
class A43; end
class A44; end
class A45; end
class A46; end
class A47; end
class A48; end
class A49; end
class A50; end

PM = T.type_alias do
        T.any(
A01,
A02,
A03,
A04,
A05,
A06,
A07,
A08,
A09,
A10,
A11,
A12,
A13,
A14,
A15,
A16,
A17,
A18,
A19,
A20,
A22,
A23,
A24,
A25,
A26,
A27,
A28,
A29,
A30,
A31,
A32,
A33,
A34,
A35,
A36,
A37,
A38,
A39,
A40,
A41,
A42,
A43,
A44,
A45,
A46,
A47,
A48,
A49,
A50
)
end


extend T::Sig
 sig do
         params(pm: T.all(PM, Half))
      .void
    end
    def used_to_take_forever_to_typecheck(pm)
        T.assert_type!(pm, Half)
end
