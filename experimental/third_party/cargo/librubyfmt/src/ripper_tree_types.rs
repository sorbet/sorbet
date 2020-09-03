#![allow(clippy::wrong_self_convention)]

#[cfg(debug_assertions)]
use log::debug;
use ripper_deserialize::RipperDeserialize;
use serde::*;

use crate::types::LineNumber;

fn ident_as_cc(i: String, lc: LineCol) -> CallChainElement {
    CallChainElement::IdentOrOpOrKeywordOrConst(IdentOrOpOrKeywordOrConst::Ident(Ident::new(i, lc)))
}

fn args_as_cc(an: ArgNode) -> CallChainElement {
    CallChainElement::ArgsAddStarOrExpressionList(normalize_args(an))
}

macro_rules! def_tag {
    ($tag_name:ident) => {
        def_tag!($tag_name, stringify!($tag_name));
    };

    ($tag_name:ident, $tag:expr) => {
        #[derive(Serialize, Debug, Clone)]
        #[allow(non_camel_case_types)]
        pub struct $tag_name;
        impl<'de> Deserialize<'de> for $tag_name {
            fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
            where
                D: Deserializer<'de>,
            {
                struct TagVisitor;

                impl<'de> de::Visitor<'de> for TagVisitor {
                    type Value = ();

                    fn expecting(
                        &self,
                        f: &mut std::fmt::Formatter<'_>,
                    ) -> Result<(), std::fmt::Error> {
                        write!(f, $tag)
                    }

                    fn visit_str<E>(self, s: &str) -> Result<Self::Value, E>
                    where
                        E: de::Error,
                    {
                        if s == $tag {
                            #[cfg(debug_assertions)]
                            {
                                debug!("accepted at {:?} {:?}", s, $tag);
                            }
                            Ok(())
                        } else {
                            #[cfg(debug_assertions)]
                            {
                                debug!("rejectd at {:?} {:?}", s, $tag);
                            }
                            Err(E::custom("mismatched tag"))
                        }
                    }
                }

                let _tag = deserializer.deserialize_str(TagVisitor)?;
                Ok($tag_name)
            }
        }
    };
}

def_tag!(program_tag, "program");
#[derive(Deserialize, Debug, Clone)]
pub struct Program(pub program_tag, pub Vec<Expression>);

def_tag!(undeserializable, "oiqjweoifjqwoeifjwqoiefjqwoiej");
#[derive(Deserialize, Debug, Clone)]
pub struct ToProc(pub undeserializable, pub Box<Expression>);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum Expression {
    ToProc(ToProc),
    Class(Class),
    If(If),
    Unary(Unary),
    VoidStmt(VoidStmt),
    Def(Def),
    Defs(Defs),
    VCall(VCall),
    Ident(Ident),
    Params(Box<Params>),
    MethodCall(MethodCall),
    Call(Call),
    CommandCall(CommandCall),
    MethodAddArg(MethodAddArg),
    Int(Int),
    BareAssocHash(BareAssocHash),
    Symbol(Symbol),
    SymbolLiteral(SymbolLiteral),
    DynaSymbol(DynaSymbol),
    Begin(Begin),
    BeginBlock(BeginBlock),
    EndBlock(EndBlock),
    Paren(ParenExpr),
    Dot2(Dot2),
    Dot3(Dot3),
    Alias(Alias),
    Array(Array),
    StringLiteral(StringLiteral),
    XStringLiteral(XStringLiteral),
    VarRef(VarRef),
    Assign(Assign),
    MAssign(MAssign),
    Const(Const),
    Command(Command),
    ConstPathRef(ConstPathRef),
    Defined(Defined),
    TopConstRef(TopConstRef),
    RescueMod(RescueMod),
    MRHSAddStar(MRHSAddStar),
    Next(Next),
    StringConcat(StringConcat),
    Super(Super),
    Kw(Kw),
    Undef(Undef),
    Binary(Binary),
    Float(Float),
    Aref(Aref),
    Char(Char),
    Module(Module),
    Return(Return),
    Return0(Return0),
    Hash(Hash),
    RegexpLiteral(RegexpLiteral),
    Backref(Backref),
    Yield(Yield),
    MethodAddBlock(MethodAddBlock),
    While(While),
    WhileMod(WhileMod),
    UntilMod(UntilMod),
    IfMod(IfMod),
    UnlessMod(UnlessMod),
    Case(Case),
    Retry(Retry),
    SClass(SClass),
    Break(Break),
    StabbyLambda(StabbyLambda),
    Imaginary(Imaginary),
    Rational(Rational),
    MLhs(MLhs),
    Until(Until),
    For(For),
    IfOp(IfOp),
    OpAssign(OpAssign),
    Unless(Unless),
    ZSuper(ZSuper),
    Yield0(Yield0),
}

def_tag!(mlhs_tag, "mlhs");
#[derive(Debug, Clone)]
pub struct MLhs(pub Vec<MLhsInner>);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum MLhsInner {
    VarField(VarField),
    Field(Field),
    RestParam(RestParam),
    Ident(Ident),
    MLhs(Box<MLhs>),
}

impl<'de> Deserialize<'de> for MLhs {
    fn deserialize<D>(deserializer: D) -> Result<MLhs, D::Error>
    where
        D: Deserializer<'de>,
    {
        struct MLhsVisitor;

        impl<'de> de::Visitor<'de> for MLhsVisitor {
            type Value = MLhs;

            fn expecting(&self, f: &mut std::fmt::Formatter<'_>) -> Result<(), std::fmt::Error> {
                write!(f, "[mlhs, (expression)*]")
            }

            fn visit_seq<A>(self, mut seq: A) -> Result<Self::Value, A::Error>
            where
                A: de::SeqAccess<'de>,
            {
                seq.next_element::<mlhs_tag>()?
                    .ok_or_else(|| de::Error::invalid_length(0, &self))?;
                Deserialize::deserialize(de::value::SeqAccessDeserializer::new(&mut seq)).map(MLhs)
            }
        }

        deserializer.deserialize_seq(MLhsVisitor)
    }
}

def_tag!(zsuper_tag, "zsuper");
#[derive(Deserialize, Debug, Clone)]
pub struct ZSuper(zsuper_tag);

impl ZSuper {
    fn into_call_chain(self) -> Vec<CallChainElement> {
        vec![ident_as_cc("super".to_string(), LineCol::unknown())]
    }
}

def_tag!(yield0_tag, "yield0");
#[derive(Deserialize, Debug, Clone)]
pub struct Yield0(yield0_tag);

impl Yield0 {
    fn into_call_chain(self) -> Vec<CallChainElement> {
        vec![ident_as_cc("yield".to_string(), LineCol::unknown())]
    }
}

def_tag!(if_tag, "if");
#[derive(Deserialize, Debug, Clone)]
pub struct If(
    pub if_tag,
    pub Box<Expression>,
    pub Vec<Expression>,
    pub Option<ElsifOrElse>,
);

def_tag!(unless_tag, "unless");
#[derive(Deserialize, Debug, Clone)]
pub struct Unless(
    pub unless_tag,
    pub Box<Expression>,
    pub Vec<Expression>,
    pub Option<Else>,
);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum ElsifOrElse {
    Elsif(Elsif),
    Else(Else),
}

def_tag!(elsif_tag, "elsif");
#[derive(Deserialize, Debug, Clone)]
pub struct Elsif(
    pub elsif_tag,
    pub Box<Expression>,
    pub Vec<Expression>,
    pub Option<Box<ElsifOrElse>>,
);

def_tag!(else_tag, "else");
#[derive(Deserialize, Debug, Clone)]
pub struct Else(pub else_tag, pub Vec<Expression>);

def_tag!(undef_tag, "undef");
#[derive(Deserialize, Debug, Clone)]
pub struct Undef(pub undef_tag, pub Vec<SymbolLiteral>);

def_tag!(string_concat_tag, "string_concat");
#[derive(Deserialize, Debug, Clone)]
pub struct StringConcat(
    pub string_concat_tag,
    pub StringConcatOrStringLiteral,
    pub StringLiteral,
);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum StringConcatOrStringLiteral {
    StringConcat(Box<StringConcat>),
    StringLiteral(StringLiteral),
}

def_tag!(mrhs_add_star_tag, "mrhs_add_star");
#[derive(Deserialize, Debug, Clone)]
pub struct MRHSAddStar(
    pub mrhs_add_star_tag,
    pub MRHSNewFromArgsOrEmpty,
    pub Box<Expression>,
);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum MRHSNewFromArgsOrEmpty {
    MRHSNewFromArgs(MRHSNewFromArgs),
    Empty(Vec<Expression>),
}

def_tag!(mrhs_new_from_args_tag, "mrhs_new_from_args");
#[derive(Deserialize, Debug, Clone)]
pub struct MRHSNewFromArgs(
    pub mrhs_new_from_args_tag,
    pub ArgsAddStarOrExpressionList,
    #[serde(default)]
    /// This will be none if only two expressions are given and the last is a
    /// splat. For example, `rescue A, *B`
    pub Option<Box<Expression>>,
);

def_tag!(rescue_mod_tag, "rescue_mod");
#[derive(Deserialize, Debug, Clone)]
pub struct RescueMod(pub rescue_mod_tag, pub Box<Expression>, pub Box<Expression>);

def_tag!(defined_tag, "defined");
#[derive(Deserialize, Debug, Clone)]
pub struct Defined(pub defined_tag, pub Box<Expression>);

def_tag!(top_const_ref_tag, "top_const_ref");
#[derive(Deserialize, Debug, Clone)]
pub struct TopConstRef(pub top_const_ref_tag, pub Const);

def_tag!(top_const_field_tag, "top_const_field");
#[derive(Deserialize, Debug, Clone)]
pub struct TopConstField(pub top_const_field_tag, pub Const);

def_tag!(const_path_ref_tag, "const_path_ref");
#[derive(Deserialize, Debug, Clone)]
pub struct ConstPathRef(pub const_path_ref_tag, pub Box<Expression>, pub Const);

def_tag!(const_ref_tag, "const_ref");
#[derive(Deserialize, Debug, Clone)]
pub struct ConstRef(pub const_ref_tag, pub Const);

def_tag!(command_tag, "command");
#[derive(Deserialize, Debug, Clone)]
pub struct Command(
    pub command_tag,
    pub IdentOrConst,
    pub ArgsAddBlockOrExpressionList,
);

impl Command {
    pub fn into_call_chain(self) -> Vec<CallChainElement> {
        let io = (self.1).into_ident_or_op_or_keyword_or_const();
        let (s, lc) = io.to_def_parts();
        vec![
            ident_as_cc(s, lc),
            CallChainElement::ArgsAddStarOrExpressionList(
                normalize_args_add_block_or_expression_list(self.2),
            ),
        ]
    }
}

impl ToMethodCall for Command {
    fn to_method_call(self) -> MethodCall {
        MethodCall::new(
            vec![],
            (self.1).into_ident_or_op_or_keyword_or_const(),
            false,
            normalize_args_add_block_or_expression_list(self.2),
        )
    }
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum ArgsAddBlockOrExpressionList {
    ArgsAddBlock(ArgsAddBlock),
    ExpressionList(Vec<Expression>),
}

def_tag!(assign_tag, "assign");
#[derive(Deserialize, Debug, Clone)]
pub struct Assign(
    pub assign_tag,
    pub Assignable,
    pub ExpressionOrMRHSNewFromArgs,
);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum ExpressionOrMRHSNewFromArgs {
    Expression(Box<Expression>),
    MRHSNewFromArgs(MRHSNewFromArgs),
}

def_tag!(massign_tag, "massign");
#[derive(Deserialize, Debug, Clone)]
pub struct MAssign(pub massign_tag, pub AssignableListOrMLhs, pub MRHSOrArray);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum AssignableListOrMLhs {
    AssignableList(Vec<Assignable>),
    MLhs(MLhs),
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum MRHSOrArray {
    MRHS(MRHS),
    Array(Array),
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum IdentOrVarField {
    Ident(Ident),
    VarField(VarField),
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum Assignable {
    VarField(VarField),
    ConstPathField(ConstPathField),
    RestParam(RestParam),
    TopConstField(TopConstField),
    ArefField(ArefField),
    Field(Field),
    // 2.6+
    Ident(Ident),
}

def_tag!(begin_block, "BEGIN");
#[derive(Deserialize, Debug, Clone)]
pub struct BeginBlock(pub begin_block, pub Vec<Expression>);

def_tag!(end_block, "END");
#[derive(Deserialize, Debug, Clone)]
pub struct EndBlock(pub end_block, pub Vec<Expression>);

def_tag!(aref_field_tag, "aref_field");
#[derive(Deserialize, Debug, Clone)]
pub struct ArefField(pub aref_field_tag, pub Box<Expression>, pub ArgsAddBlock);

def_tag!(const_path_field_tag, "const_path_field");
#[derive(Deserialize, Debug, Clone)]
pub struct ConstPathField(pub const_path_field_tag, pub Box<Expression>, pub Const);

def_tag!(var_field_tag, "var_field");
#[derive(Deserialize, Debug, Clone)]
pub struct VarField(pub var_field_tag, pub VarRefType);

def_tag!(field_tag, "field");
#[derive(Deserialize, Debug, Clone)]
pub struct Field(
    pub field_tag,
    pub Box<Expression>,
    pub DotTypeOrOp,
    pub Ident,
);

def_tag!(var_ref_tag, "var_ref");
#[derive(Deserialize, Debug, Clone)]
pub struct VarRef(pub var_ref_tag, pub VarRefType);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum VarRefType {
    GVar(GVar),
    IVar(IVar),
    CVar(CVar),
    Ident(Ident),
    Const(Const),
    Kw(Kw),
}

def_tag!(gvar_tag, "@gvar");
#[derive(Deserialize, Debug, Clone)]
pub struct GVar(pub gvar_tag, pub String, pub LineCol);

def_tag!(ivar_tag, "@ivar");
#[derive(Deserialize, Debug, Clone)]
pub struct IVar(pub ivar_tag, pub String, pub LineCol);

def_tag!(cvar_tag, "@cvar");
#[derive(Deserialize, Debug, Clone)]
pub struct CVar(pub cvar_tag, pub String, pub LineCol);

def_tag!(heredoc_string_literal_tag, "heredoc_string_literal");
#[derive(Deserialize, Debug, Clone)]
pub struct HeredocStringLiteral(pub heredoc_string_literal_tag, pub (String, String));

def_tag!(string_literal_tag, "string_literal");
#[derive(RipperDeserialize, Debug, Clone)]
pub enum StringLiteral {
    Normal(string_literal_tag, StringContent),
    Heredoc(string_literal_tag, HeredocStringLiteral, StringContent),
}

def_tag!(xstring_literal_tag, "xstring_literal");
#[derive(Deserialize, Debug, Clone)]
pub struct XStringLiteral(pub xstring_literal_tag, pub Vec<StringContentPart>);

def_tag!(dyna_symbol_tag, "dyna_symbol");
#[derive(Deserialize, Debug, Clone)]
pub struct DynaSymbol(pub dyna_symbol_tag, pub StringContentOrStringContentParts);

impl DynaSymbol {
    pub fn to_string_literal(self) -> StringLiteral {
        match self.1 {
            StringContentOrStringContentParts::StringContent(sc) => {
                StringLiteral::Normal(string_literal_tag, sc)
            }
            StringContentOrStringContentParts::StringContentParts(scp) => {
                StringLiteral::Normal(string_literal_tag, StringContent(string_content_tag, scp))
            }
        }
    }
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum StringContentOrStringContentParts {
    StringContent(StringContent),
    StringContentParts(Vec<StringContentPart>),
}

def_tag!(tstring_content_tag, "@tstring_content");
#[derive(Deserialize, Debug, Clone)]
pub struct TStringContent(pub tstring_content_tag, pub String, pub LineCol);

def_tag!(string_embexpr_tag, "string_embexpr");
#[derive(Deserialize, Debug, Clone)]
pub struct StringEmbexpr(pub string_embexpr_tag, pub Vec<Expression>);

def_tag!(string_dvar_tag, "string_dvar");
#[derive(Deserialize, Debug, Clone)]
pub struct StringDVar(pub string_dvar_tag, pub Box<Expression>);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum StringContentPart {
    TStringContent(TStringContent),
    StringEmbexpr(StringEmbexpr),
    StringDVar(StringDVar),
}

def_tag!(string_content_tag, "string_content");
#[derive(Debug, Clone)]
pub struct StringContent(pub string_content_tag, pub Vec<StringContentPart>);

impl<'de> Deserialize<'de> for StringContent {
    fn deserialize<D>(deserializer: D) -> Result<StringContent, D::Error>
    where
        D: Deserializer<'de>,
    {
        struct StringContentVisitor;

        impl<'de> de::Visitor<'de> for StringContentVisitor {
            type Value = StringContent;

            fn expecting(&self, f: &mut std::fmt::Formatter<'_>) -> Result<(), std::fmt::Error> {
                write!(
                    f,
                    "[string_content, (tstring_content, string_embexpr, string_dvar)*]"
                )
            }

            fn visit_seq<A>(self, mut seq: A) -> Result<Self::Value, A::Error>
            where
                A: de::SeqAccess<'de>,
            {
                let tag = seq
                    .next_element()?
                    .ok_or_else(|| de::Error::invalid_length(0, &self))?;
                let elements =
                    Deserialize::deserialize(de::value::SeqAccessDeserializer::new(&mut seq))?;
                Ok(StringContent(tag, elements))
            }
        }

        deserializer.deserialize_seq(StringContentVisitor)
    }
}

def_tag!(array_tag, "array");
#[derive(Deserialize, Debug, Clone)]
pub struct Array(
    pub array_tag,
    pub SimpleArrayOrPercentArray,
    pub Option<LineCol>,
);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum SimpleArrayOrPercentArray {
    SimpleArray(Option<ArgsAddStarOrExpressionList>),
    LowerPercentArray((String, Vec<TStringContent>, LineCol)),
    UpperPercentArray((String, Vec<Vec<StringContentPart>>, LineCol)),
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum ArgsAddStarOrExpressionList {
    ExpressionList(Vec<Expression>),
    ArgsAddStar(ArgsAddStar),
}

impl ArgsAddStarOrExpressionList {
    pub fn is_empty(&self) -> bool {
        if let ArgsAddStarOrExpressionList::ExpressionList(el) = self {
            if el.is_empty() {
                return true;
            }
        }

        false
    }

    pub fn empty() -> Self {
        ArgsAddStarOrExpressionList::ExpressionList(vec![])
    }
}

def_tag!(args_add_star_tag, "args_add_star");
#[derive(Debug, Clone)]
pub struct ArgsAddStar(
    pub args_add_star_tag,
    pub Box<ArgsAddStarOrExpressionList>,
    pub Box<Expression>,
    pub Vec<Expression>,
);

impl<'de> Deserialize<'de> for ArgsAddStar {
    fn deserialize<D>(deserializer: D) -> Result<ArgsAddStar, D::Error>
    where
        D: Deserializer<'de>,
    {
        struct ArgsAddStarVisitor;

        impl<'de> de::Visitor<'de> for ArgsAddStarVisitor {
            type Value = ArgsAddStar;

            fn expecting(&self, f: &mut std::fmt::Formatter<'_>) -> Result<(), std::fmt::Error> {
                write!(
                    f,
                    "[args_add_star, [expression*], expression, expression*] or [args_add_star, [args_add_star, ...], expression, expression*"
                )
            }

            fn visit_seq<A>(self, mut seq: A) -> Result<Self::Value, A::Error>
            where
                A: de::SeqAccess<'de>,
            {
                let (tag, left_expressions, star_expression) =
                    Deserialize::deserialize(de::value::SeqAccessDeserializer::new(&mut seq))?;
                let right_expressions =
                    Deserialize::deserialize(de::value::SeqAccessDeserializer::new(&mut seq))?;

                Ok(ArgsAddStar(
                    tag,
                    left_expressions,
                    star_expression,
                    right_expressions,
                ))
            }
        }

        deserializer.deserialize_seq(ArgsAddStarVisitor)
    }
}

def_tag!(alias_tag, "alias");
#[derive(Deserialize, Debug, Clone)]
pub struct Alias(pub alias_tag, pub SymbolLiteral, pub SymbolLiteral);

def_tag!(paren_expr_tag, "paren");
#[derive(Deserialize, Debug, Clone)]
pub struct ParenExpr(pub paren_expr_tag, pub Vec<Expression>);

def_tag!(dot2_tag, "dot2");
#[derive(Deserialize, Debug, Clone)]
pub struct Dot2(
    pub dot2_tag,
    pub Option<Box<Expression>>,
    pub Option<Box<Expression>>,
);

def_tag!(dot3_tag, "dot3");
#[derive(Deserialize, Debug, Clone)]
pub struct Dot3(
    pub dot3_tag,
    pub Option<Box<Expression>>,
    pub Option<Box<Expression>>,
);

def_tag!(void_stmt_tag, "void_stmt");
#[derive(Deserialize, Debug, Clone)]
pub struct VoidStmt(void_stmt_tag);

def_tag!(def_tag, "def");
#[derive(Deserialize, Debug, Clone)]
pub struct Def(
    pub def_tag,
    pub IdentOrOpOrKeywordOrConst,
    pub ParenOrParams,
    pub Box<BodyStmt>,
);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum IdentOrOpOrKeywordOrConst {
    Ident(Ident),
    Op((op_tag, String, LineCol)),
    Keyword(Kw),
    Const(Const),
}

impl IdentOrOpOrKeywordOrConst {
    pub fn to_def_parts(self) -> (String, LineCol) {
        match self {
            Self::Ident(Ident(_, string, linecol)) => (string, linecol),
            Self::Op((_, string, linecol)) => (string, linecol),
            Self::Keyword(Kw(_, string, linecol)) => (string, linecol),
            Self::Const(Const(_, string, linecol)) => (string, linecol),
        }
    }

    pub fn into_ident(self) -> Ident {
        let (s, lc) = self.to_def_parts();
        Ident::new(s, lc)
    }

    pub fn get_name(&self) -> String {
        self.clone().to_def_parts().0
    }
}

def_tag!(begin_tag, "begin");
#[derive(Deserialize, Debug, Clone)]
pub struct Begin(pub begin_tag, pub Box<BodyStmt>);

def_tag!(bodystmt_tag, "bodystmt");
#[derive(Deserialize, Debug, Clone)]
pub struct BodyStmt(
    pub bodystmt_tag,
    pub Vec<Expression>,
    pub Option<Rescue>,
    pub Option<RescueElseOrExpressionList>,
    pub Option<Ensure>,
);

// deals with 2.6, where else is a vec expression and not an else
#[derive(RipperDeserialize, Debug, Clone)]
pub enum RescueElseOrExpressionList {
    RescueElse(RescueElse),
    ExpressionList(Vec<Expression>),
}

def_tag!(rescue_tag, "rescue");
#[derive(Deserialize, Debug, Clone)]
pub struct Rescue(
    pub rescue_tag,
    pub Option<MRHS>,
    pub Option<Assignable>,
    pub Option<Vec<Expression>>,
    pub Option<Box<Rescue>>,
);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum MRHS {
    Single(Box<Expression>),
    SingleAsArray(Vec<Expression>),
    MRHSNewFromArgs(MRHSNewFromArgs),
    MRHSAddStar(MRHSAddStar),
    Array(Array),
}

def_tag!(rescue_else_tag, "else");
#[derive(Deserialize, Debug, Clone)]
pub struct RescueElse(pub rescue_else_tag, pub Option<Vec<Expression>>);

def_tag!(ensure_tag, "ensure");
#[derive(Deserialize, Debug, Clone)]
pub struct Ensure(pub ensure_tag, pub Option<Vec<Expression>>);

def_tag!(const_tag, "@const");
#[derive(Deserialize, Debug, Clone)]
pub struct Const(pub const_tag, pub String, pub LineCol);

def_tag!(ident_tag, "@ident");
#[derive(Deserialize, Debug, Clone)]
pub struct Ident(pub ident_tag, pub String, pub LineCol);

impl Ident {
    pub fn new(s: String, l: LineCol) -> Self {
        Ident(ident_tag, s, l)
    }
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum ParenOrParams {
    Paren(Paren),
    Params(Box<Params>),
}

impl ParenOrParams {
    pub fn is_present(&self) -> bool {
        match self {
            ParenOrParams::Paren(p) => p.is_present(),
            ParenOrParams::Params(p) => p.is_present(),
        }
    }
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum IdentOrMLhs {
    Ident(Ident),
    MLhs(MLhs),
}

def_tag!(paren_tag, "paren");
#[derive(Deserialize, Debug, Clone)]
pub struct Paren(pub paren_tag, pub Box<Params>);

impl Paren {
    fn is_present(&self) -> bool {
        (self.1).is_present()
    }
}

def_tag!(params_tag, "params");
#[derive(Deserialize, Debug, Clone)]
pub struct Params(
    pub params_tag,
    pub Option<Vec<IdentOrMLhs>>,
    pub Option<Vec<(Ident, Expression)>>,
    pub Option<RestParamOr0OrExcessedComma>,
    pub Option<Vec<IdentOrMLhs>>,
    pub Option<Vec<(Label, ExpressionOrFalse)>>,
    pub Option<KwRestParam>,
    pub Option<BlockArg>,
);

impl Params {
    fn is_present(&self) -> bool {
        (self.1).is_some()
            || (self.2).is_some()
            || (self.3).is_some()
            || (self.4).is_some()
            || (self.5).is_some()
            || (self.6).is_some()
            || (self.7).is_some()
    }
}

// on ruby 2.5 and 2.6 the params lists for blocks (only), permit a trailing
// comma (presumably because of f params). Params lists for functions do
// not allow this.
//
// valid:
// ```ruby
// lambda { |x,| }
// lambda { |x, ;f }
// ```
// not valid:
// ``` ruby
// def foo(x,)
// end
// ```
//
// this causes the parser to parse the *params* node differently, even though
// the wrapping structure is a block var:
//
// on 2.5:
//
// rr 'lambda { |x,| }'
// [:program,
//  [[:method_add_block,
//    [:method_add_arg, [:fcall, [:@ident, "lambda", [1, 0]]], []],
//    [:brace_block,
//     [:block_var,
//      [:params, [[:@ident, "x", [1, 10]]], nil, 0, nil, nil, nil, nil],
//      false],
//     [[:void_stmt]]]]]]
// on 2.6:
//
// [:program,
//  [[:method_add_block,
//    [:method_add_arg, [:fcall, [:@ident, "lambda", [1, 0]]], []],
//    [:brace_block,
//     [:block_var,
//      [:params,
//       [[:@ident, "x", [1, 10]]],
//       nil,
//       [:excessed_comma],
//       nil,
//       nil,
//       nil,
//       nil],
//      false],
//     [[:void_stmt]]]]]]
// this difference is in the "rest_args" position, and on 2.5 is a literal
// integer 0 and on 2.6 a unit parser tag [:excessed_comma]. These nodes don't
// appear to cause any semantic difference in the program.
// So:
//   the Zero deserialzer deals with the 2.5 case, and the ExcessedComma node
//   deals with the 2.6 case, I will note that I tried to collapse them in to
//   a single representative node, but that didn't work with the serde setup
//   we have for some reason.
#[derive(RipperDeserialize, Debug, Clone)]
pub enum RestParamOr0OrExcessedComma {
    Zero(i64),
    RestParam(RestParam),
    ExcessedComma(ExcessedComma),
}

def_tag!(excessed_comma_tag, "excessed_comma");
#[derive(Deserialize, Debug, Clone)]
pub struct ExcessedComma(excessed_comma_tag);

impl Params {
    pub fn non_null_positions(&self) -> Vec<bool> {
        vec![
            (self.1).is_some(),
            (self.2).is_some(),
            (self.3).is_some(),
            (self.4).is_some(),
            (self.5).is_some(),
            (self.6).is_some(),
            (self.7).is_some(),
        ]
    }
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum ExpressionOrFalse {
    Expression(Expression),
    False(bool),
}

def_tag!(rest_param_tag, "rest_param");
#[derive(Deserialize, Debug, Clone)]
pub struct RestParam(pub rest_param_tag, pub Option<IdentOrVarField>);

def_tag!(kw_rest_param_tag, "kwrest_param");
#[derive(Deserialize, Debug, Clone)]
pub struct KwRestParam(pub kw_rest_param_tag, pub Option<Ident>);

def_tag!(blockarg_tag, "blockarg");
#[derive(Deserialize, Debug, Clone)]
pub struct BlockArg(pub blockarg_tag, pub Ident);

#[derive(Deserialize, Debug, Clone)]
pub struct LineCol(pub LineNumber, pub u64);

impl LineCol {
    fn unknown() -> Self {
        LineCol(0, 0)
    }
}

pub fn normalize_arg_paren(ap: ArgParen) -> ArgsAddStarOrExpressionList {
    match *ap.1 {
        ArgNode::Null(_) => ArgsAddStarOrExpressionList::ExpressionList(vec![]),
        ae => normalize_args(ae),
    }
}

pub fn normalize_args_add_block_or_expression_list(
    aab: ArgsAddBlockOrExpressionList,
) -> ArgsAddStarOrExpressionList {
    match aab {
        ArgsAddBlockOrExpressionList::ExpressionList(el) => {
            ArgsAddStarOrExpressionList::ExpressionList(el)
        }
        ArgsAddBlockOrExpressionList::ArgsAddBlock(aab) => normalize_args_add_block(aab),
    }
}
pub fn normalize_args_add_block(aab: ArgsAddBlock) -> ArgsAddStarOrExpressionList {
    // .1 is expression list
    // .2 is block
    match aab.2 {
        ToProcExpr::NotPresent(_) => (aab.1).into_args_add_star_or_expression_list(),
        ToProcExpr::Present(e) => {
            let trailing_expr_as_vec = vec![Expression::ToProc(ToProc(undeserializable, e))];

            match (aab.1).into_args_add_star_or_expression_list() {
                ArgsAddStarOrExpressionList::ExpressionList(items) => {
                    ArgsAddStarOrExpressionList::ExpressionList(
                        vec![items, trailing_expr_as_vec].concat(),
                    )
                }
                ArgsAddStarOrExpressionList::ArgsAddStar(aas) => {
                    let mut new_aas = aas;
                    new_aas.3 = vec![new_aas.3, trailing_expr_as_vec].concat();
                    ArgsAddStarOrExpressionList::ArgsAddStar(new_aas)
                }
            }
        }
    }
}

pub fn normalize_args(arg_node: ArgNode) -> ArgsAddStarOrExpressionList {
    match arg_node {
        ArgNode::ArgParen(ap) => normalize_arg_paren(ap),
        ArgNode::ArgsAddBlock(aab) => normalize_args_add_block(aab),
        ArgNode::ArgsAddStar(aas) => ArgsAddStarOrExpressionList::ArgsAddStar(aas),
        ArgNode::Exprs(exprs) => ArgsAddStarOrExpressionList::ExpressionList(exprs),
        ArgNode::Const(c) => {
            ArgsAddStarOrExpressionList::ExpressionList(vec![Expression::Const(c)])
        }
        ArgNode::Ident(c) => {
            ArgsAddStarOrExpressionList::ExpressionList(vec![Expression::Ident(c)])
        }
        ArgNode::Null(_) => panic!("should never be called with null"),
    }
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum ArgNode {
    ArgParen(ArgParen),
    ArgsAddBlock(ArgsAddBlock),
    ArgsAddStar(ArgsAddStar),
    Exprs(Vec<Expression>),
    Const(Const),
    Ident(Ident),
    Null(Option<String>),
}

def_tag!(arg_paren_tag, "arg_paren");
#[derive(Deserialize, Debug, Clone)]
pub struct ArgParen(pub arg_paren_tag, pub Box<ArgNode>);

// See: https://dev.to/penelope_zone/understanding-ruby-s-block-proc-parsing-4a89
#[derive(RipperDeserialize, Debug, Clone)]
pub enum ToProcExpr {
    NotPresent(bool),
    Present(Box<Expression>),
}

// ArgsAddBlock
def_tag!(args_add_block_tag, "args_add_block");
#[derive(Deserialize, Debug, Clone)]
pub struct ArgsAddBlock(
    pub args_add_block_tag,
    pub ArgsAddBlockInner,
    pub ToProcExpr,
);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum AABParen {
    Paren((paren_tag, Box<Expression>)),
    Expression(Expression),
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum ArgsAddBlockInner {
    Parens(Vec<AABParen>),
    ArgsAddStarOrExpressionList(ArgsAddStarOrExpressionList),
}

impl ArgsAddBlockInner {
    pub fn into_args_add_star_or_expression_list(self) -> ArgsAddStarOrExpressionList {
        match self {
            ArgsAddBlockInner::ArgsAddStarOrExpressionList(a) => a,
            ArgsAddBlockInner::Parens(ps) => {
                let el = ps
                    .into_iter()
                    .map(|aabp| match aabp {
                        AABParen::Paren(p) => *p.1,
                        AABParen::Expression(e) => e,
                    })
                    .collect();
                ArgsAddStarOrExpressionList::ExpressionList(el)
            }
        }
    }
}

def_tag!(int_tag, "@int");
#[derive(Deserialize, Debug, Clone)]
pub struct Int(pub int_tag, pub String, pub LineCol);

def_tag!(bare_assoc_hash_tag, "bare_assoc_hash");
#[derive(Deserialize, Debug, Clone)]
pub struct BareAssocHash(pub bare_assoc_hash_tag, pub Vec<AssocNewOrAssocSplat>);

def_tag!(hash_tag, "hash");
#[derive(Deserialize, Debug, Clone)]
pub struct Hash(pub hash_tag, pub Option<AssocListFromArgs>, pub LineCol);

def_tag!(assoclist_from_args_tag, "assoclist_from_args");
#[derive(Deserialize, Debug, Clone)]
pub struct AssocListFromArgs(pub assoclist_from_args_tag, pub Vec<AssocNewOrAssocSplat>);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum AssocNewOrAssocSplat {
    AssocNew(Box<AssocNew>),
    AssocSplat(AssocSplat),
}

def_tag!(assoc_new_tag, "assoc_new");
#[derive(Deserialize, Debug, Clone)]
pub struct AssocNew(pub assoc_new_tag, pub AssocKey, pub Expression);

def_tag!(assoc_splat_tag, "assoc_splat");
#[derive(Deserialize, Debug, Clone)]
pub struct AssocSplat(pub assoc_splat_tag, pub Expression);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum AssocKey {
    Label(Label),
    Expression(Expression),
}

def_tag!(label_tag, "@label");
#[derive(Deserialize, Debug, Clone)]
pub struct Label(pub label_tag, pub String, pub LineCol);

def_tag!(symbol_literal_tag, "symbol_literal");
#[derive(Deserialize, Debug, Clone)]
pub struct SymbolLiteral(pub symbol_literal_tag, pub SymbolOrBare);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum SymbolOrBare {
    Ident(Ident),
    Op(Op),
    Kw(Kw),
    Symbol(Symbol),
    GVar(GVar),
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum IdentOrConst {
    Ident(Ident),
    Const(Const),
}

impl IdentOrConst {
    pub fn into_ident_or_op_or_keyword_or_const(self) -> IdentOrOpOrKeywordOrConst {
        match self {
            IdentOrConst::Ident(i) => IdentOrOpOrKeywordOrConst::Ident(i),
            IdentOrConst::Const(c) => IdentOrOpOrKeywordOrConst::Const(c),
        }
    }
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum IdentOrConstOrKwOrOpOrIvarOrGvar {
    Ident(Ident),
    Const(Const),
    Keyword(Kw),
    Op(Op),
    IVar(IVar),
    GVar(GVar),
}

def_tag!(symbol_tag, "symbol");
#[derive(Deserialize, Debug, Clone)]
pub struct Symbol(pub symbol_tag, pub IdentOrConstOrKwOrOpOrIvarOrGvar);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum CallLeft {
    Paren(ParenExpr),
    SingleParen(paren_tag, Box<Expression>),
    Call(Call),
    FCall(FCall),
    VCall(VCall),
    MethodAddArg(MethodAddArg),
    MethodAddBlock(MethodAddBlock),
    VarRef(VarRef),
    Super(Super),
    ZSuper(ZSuper),
    Next(Next),
    Yield(Yield),
    Yield0(Yield0),
    Command(Command),
    CommandCall(CommandCall),
    Expression(Box<Expression>),
}

impl CallLeft {
    pub fn into_call_chain(self) -> Vec<CallChainElement> {
        match self {
            CallLeft::Paren(p) => vec![CallChainElement::Paren(p)],
            CallLeft::SingleParen(_, e) => {
                vec![CallChainElement::Paren(ParenExpr(paren_expr_tag, vec![*e]))]
            }
            CallLeft::FCall(FCall(_, ic)) => vec![CallChainElement::IdentOrOpOrKeywordOrConst(
                ic.into_ident_or_op_or_keyword_or_const(),
            )],
            CallLeft::VCall(VCall(_, ic)) => vec![CallChainElement::IdentOrOpOrKeywordOrConst(
                ic.into_ident_or_op_or_keyword_or_const(),
            )],
            CallLeft::Call(Call(_, left, dot, name)) => {
                let mut res = left.into_call_chain();
                res.push(CallChainElement::DotTypeOrOp(dot));

                if let CallMethodName::IdentOrOpOrKeywordOrConst(ic) = name {
                    res.push(CallChainElement::IdentOrOpOrKeywordOrConst(ic));
                }
                res
            }
            CallLeft::MethodAddArg(MethodAddArg(_, left, an)) => {
                let mut res = left.into_call_chain();
                res.push(args_as_cc(an));
                res
            }
            CallLeft::MethodAddBlock(MethodAddBlock(_, left, block)) => {
                let mut res = left.into_call_chain();
                res.append(&mut vec![CallChainElement::Block(block)]);
                res
            }
            CallLeft::VarRef(v) => vec![CallChainElement::VarRef(v)],
            CallLeft::Super(s) => s.into_call_chain(),
            CallLeft::ZSuper(zs) => zs.into_call_chain(),
            CallLeft::Next(next) => next.into_call_chain(),
            CallLeft::Yield(y) => y.into_call_chain(),
            CallLeft::Yield0(y) => y.into_call_chain(),
            CallLeft::Command(c) => c.into_call_chain(),
            CallLeft::CommandCall(c) => c.into_call_chain(),
            CallLeft::Expression(e) => vec![CallChainElement::Expression(e)],
        }
    }
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum CallChainElement {
    IdentOrOpOrKeywordOrConst(IdentOrOpOrKeywordOrConst),
    Block(Block),
    VarRef(VarRef),
    ArgsAddStarOrExpressionList(ArgsAddStarOrExpressionList),
    DotTypeOrOp(DotTypeOrOp),
    Paren(ParenExpr),
    Expression(Box<Expression>),
}

pub type DotCall = call_tag;

def_tag!(method_add_arg_tag, "method_add_arg");
#[derive(Deserialize, Debug, Clone)]
pub struct MethodAddArg(pub method_add_arg_tag, pub Box<CallLeft>, pub ArgNode);

impl ToMethodCall for MethodAddArg {
    fn to_method_call(self) -> MethodCall {
        let mut orig_chain = (self.1).into_call_chain();
        let last = orig_chain
            .pop()
            .expect("cannot be empty with method add arg");
        if let CallChainElement::IdentOrOpOrKeywordOrConst(n) = last {
            MethodCall::new(orig_chain, n, true, normalize_args(self.2))
        } else {
            MethodCall::new(
                orig_chain,
                IdentOrOpOrKeywordOrConst::Ident(Ident::new(".()".to_string(), LineCol::unknown())),
                true,
                normalize_args(self.2),
            )
        }
    }
}

def_tag!(method_add_block_tag, "method_add_block");
#[derive(Deserialize, Debug, Clone)]
pub struct MethodAddBlock(method_add_block_tag, pub Box<CallLeft>, pub Block);

def_tag!(fcall_tag, "fcall");
#[derive(Deserialize, Debug, Clone)]
pub struct FCall(pub fcall_tag, pub IdentOrConst);

def_tag!(vcall);
#[derive(Deserialize, Debug, Clone)]
pub struct VCall(pub vcall, pub IdentOrConst);

pub trait ToMethodCall {
    fn to_method_call(self) -> MethodCall;
}

impl ToMethodCall for VCall {
    fn to_method_call(self) -> MethodCall {
        MethodCall::new(
            vec![],
            (self.1).into_ident_or_op_or_keyword_or_const(),
            false,
            ArgsAddStarOrExpressionList::ExpressionList(vec![]),
        )
    }
}

// isn't parsable, but we do create it in our "normalized tree"
def_tag!(method_call_tag, "method_call");
#[derive(Deserialize, Debug, Clone)]
pub struct MethodCall(
    pub method_call_tag,
    // call chain
    pub Vec<CallChainElement>,
    // method name
    pub IdentOrOpOrKeywordOrConst,
    // original used parens
    pub bool,
    // args
    pub ArgsAddStarOrExpressionList,
);

impl MethodCall {
    pub fn new(
        chain: Vec<CallChainElement>,
        name: IdentOrOpOrKeywordOrConst,
        use_parens: bool,
        args: ArgsAddStarOrExpressionList,
    ) -> Self {
        MethodCall(method_call_tag, chain, name, use_parens, args)
    }
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum CallMethodName {
    IdentOrOpOrKeywordOrConst(IdentOrOpOrKeywordOrConst),
    DotCall(DotCall),
}

def_tag!(call_tag, "call");
#[derive(Deserialize, Debug, Clone)]
pub struct Call(
    pub call_tag,
    pub Box<CallLeft>,
    pub DotTypeOrOp,
    pub CallMethodName,
);

impl ToMethodCall for Call {
    fn to_method_call(self) -> MethodCall {
        let mut chain = (self.1).into_call_chain();
        let method_name = match self.3 {
            CallMethodName::IdentOrOpOrKeywordOrConst(i) => i,
            CallMethodName::DotCall(_) => {
                IdentOrOpOrKeywordOrConst::Ident(Ident::new("".to_string(), LineCol::unknown()))
            }
        };
        chain.push(CallChainElement::DotTypeOrOp(self.2));
        MethodCall::new(
            chain,
            method_name,
            false,
            ArgsAddStarOrExpressionList::empty(),
        )
    }
}

def_tag!(command_call_tag, "command_call");
#[derive(Deserialize, Debug, Clone)]
pub struct CommandCall(
    command_call_tag,
    pub Box<CallLeft>,
    pub DotTypeOrOp,
    pub IdentOrOpOrKeywordOrConst,
    pub ArgNode,
);

impl CommandCall {
    pub fn into_call_chain(self) -> Vec<CallChainElement> {
        let mut recur = (self.1).into_call_chain();
        recur.push(CallChainElement::DotTypeOrOp(self.2));
        recur.push(CallChainElement::IdentOrOpOrKeywordOrConst(self.3));
        recur.push(args_as_cc(self.4));
        recur
    }
}

impl ToMethodCall for CommandCall {
    fn to_method_call(self) -> MethodCall {
        let mut chain = (self.1).into_call_chain();
        chain.push(CallChainElement::DotTypeOrOp(self.2));

        MethodCall::new(chain, self.3, false, normalize_args(self.4))
    }
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum DotType {
    Dot(Dot),
    LonelyOperator(LonelyOperator),
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum DotTypeOrOp {
    DotType(DotType),
    Period(Period),
    ColonColon(ColonColon),
    Op(Op),
    StringDot(String),
}

def_tag!(period_tag, "@period");
#[derive(Deserialize, Debug, Clone)]
pub struct Period(pub period_tag, pub String, pub LineCol);

def_tag!(equals_tag, "==");
#[derive(Deserialize, Debug, Clone)]
pub struct Equals(pub equals_tag);

def_tag!(dot_tag, ".");
#[derive(Deserialize, Debug, Clone)]
pub struct Dot(pub dot_tag);

def_tag!(colon_colon_tag, "::");
#[derive(Deserialize, Debug, Clone)]
pub struct ColonColon(pub colon_colon_tag);

def_tag!(lonely_operator_tag, "&.");
#[derive(Deserialize, Debug, Clone)]
pub struct LonelyOperator(pub lonely_operator_tag);

def_tag!(op_tag, "@op");
#[derive(Deserialize, Debug, Clone)]
pub struct Op(pub op_tag, pub Operator, pub LineCol);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum Operator {
    Equals(Equals),
    Dot(Dot),
    LonelyOperator(LonelyOperator),
    StringOperator(String),
}

def_tag!(opassign_tag, "opassign");
#[derive(Deserialize, Debug, Clone)]
pub struct OpAssign(
    pub opassign_tag,
    pub Assignable,
    pub Op,
    pub Box<Expression>,
);

def_tag!(next_tag, "next");
#[derive(Deserialize, Debug, Clone)]
pub struct Next(pub next_tag, pub ArgsAddBlockOrExpressionList);

impl Next {
    pub fn into_call_chain(self) -> Vec<CallChainElement> {
        vec![
            ident_as_cc("next".to_string(), LineCol::unknown()),
            CallChainElement::ArgsAddStarOrExpressionList(
                normalize_args_add_block_or_expression_list(self.1),
            ),
        ]
    }
}

def_tag!(if_mod_tag, "if_mod");
#[derive(Deserialize, Debug, Clone)]
pub struct IfMod(pub if_mod_tag, pub Box<Expression>, pub Box<Expression>);

def_tag!(unless_mod_tag, "unless_mod");
#[derive(Deserialize, Debug, Clone)]
pub struct UnlessMod(pub unless_mod_tag, pub Box<Expression>, pub Box<Expression>);

#[derive(Debug, Clone)]
pub enum UnaryType {
    Not,
    Negative,
    Positive,
    BooleanNot,
}

impl<'de> Deserialize<'de> for UnaryType {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        match Deserialize::deserialize(deserializer)? {
            "not" => Ok(Self::Not),
            "-@" => Ok(Self::Negative),
            "+@" => Ok(Self::Positive),
            "!" => Ok(Self::BooleanNot),
            s => Err(de::Error::invalid_value(
                de::Unexpected::Str(s),
                &"not, -@, +@, or !",
            )),
        }
    }
}

def_tag!(unary_tag, "unary");
#[derive(Deserialize, Debug, Clone)]
pub struct Unary(pub unary_tag, pub UnaryType, pub Box<Expression>);

def_tag!(super_tag, "super");
#[derive(Deserialize, Debug, Clone)]
pub struct Super(pub super_tag, pub ArgNode, pub LineCol);

impl Super {
    pub fn into_call_chain(self) -> Vec<CallChainElement> {
        vec![ident_as_cc("super".to_string(), self.2), args_as_cc(self.1)]
    }
}

impl ToMethodCall for Super {
    fn to_method_call(self) -> MethodCall {
        MethodCall::new(
            vec![],
            IdentOrOpOrKeywordOrConst::Ident(Ident::new("super".to_string(), self.2)),
            true,
            normalize_args(self.1),
        )
    }
}

def_tag!(kw_tag, "@kw");
#[derive(Deserialize, Debug, Clone)]
pub struct Kw(pub kw_tag, pub String, pub LineCol);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum ConstPathRefOrConstRef {
    ConstPathRef(ConstPathRef),
    ConstRef(ConstRef),
}

def_tag!(class_tag, "class");
#[derive(Deserialize, Debug, Clone)]
pub struct Class(
    pub class_tag,
    pub ConstPathRefOrConstRef,
    pub Option<Box<Expression>>,
    pub Box<BodyStmt>,
);

def_tag!(module_tag, "module");
#[derive(Deserialize, Debug, Clone)]
pub struct Module(
    pub module_tag,
    pub ConstPathRefOrConstRef,
    pub Box<BodyStmt>,
);

def_tag!(defs_tag, "defs");
#[derive(Deserialize, Debug, Clone)]
pub struct Defs(
    pub defs_tag,
    pub Singleton,
    pub DotOrColon,
    pub IdentOrOpOrKeywordOrConst,
    pub ParenOrParams,
    pub Box<BodyStmt>,
);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum IdentOrKw {
    Ident(Ident),
    Kw(Kw),
}

#[derive(RipperDeserialize, Debug, Clone)]
pub enum Singleton {
    VarRef(VarRef),
    Paren(ParenExpr),
}

// can only occur in defs, Op is always `::`
#[derive(RipperDeserialize, Debug, Clone)]
pub enum DotOrColon {
    Period(Period),
    Op(Operator),
}

def_tag!(binary_tag, "binary");
#[derive(Deserialize, Debug, Clone)]
pub struct Binary(
    pub binary_tag,
    pub Box<Expression>,
    pub String,
    pub Box<Expression>,
);

def_tag!(float_tag, "@float");
#[derive(Deserialize, Debug, Clone)]
pub struct Float(float_tag, pub String, pub LineCol);

def_tag!(aref_tag, "aref");
#[derive(Deserialize, Debug, Clone)]
pub struct Aref(aref_tag, pub Box<Expression>, pub Option<ArgNode>);

def_tag!(char_tag, "@CHAR");
#[derive(Deserialize, Debug, Clone)]
pub struct Char(char_tag, pub String, pub LineCol);

def_tag!(return_tag, "return");
#[derive(Deserialize, Debug, Clone)]
pub struct Return(return_tag, pub ArgNode, pub LineCol);

def_tag!(return0_tag, "return0");
#[derive(Deserialize, Debug, Clone)]
pub struct Return0(return0_tag);

def_tag!(regexp_literal_tag, "regexp_literal");
#[derive(Deserialize, Debug, Clone)]
pub struct RegexpLiteral(
    regexp_literal_tag,
    pub Vec<StringContentPart>,
    pub RegexpEnd,
);

def_tag!(regexp_end_tag, "@regexp_end");
#[derive(Deserialize, Debug, Clone)]
pub struct RegexpEnd(regexp_end_tag, pub String, pub LineCol, pub String);

def_tag!(backref_tag, "@backref");
#[derive(Deserialize, Debug, Clone)]
pub struct Backref(backref_tag, pub String, pub LineCol);

def_tag!(yield_tag, "yield");
#[derive(Deserialize, Debug, Clone)]
pub struct Yield(yield_tag, pub ParenOrArgsAddBlock, pub LineCol);

impl Yield {
    fn into_call_chain(self) -> Vec<CallChainElement> {
        let arg = (self.1).into_arg_node();
        vec![ident_as_cc("yield".to_string(), self.2), args_as_cc(arg)]
    }
}

impl ToMethodCall for Yield {
    fn to_method_call(self) -> MethodCall {
        let used_parens = (self.1).is_paren();
        MethodCall::new(
            vec![],
            IdentOrOpOrKeywordOrConst::Ident(Ident::new("yield".to_string(), self.2)),
            used_parens,
            normalize_args((self.1).into_arg_node()),
        )
    }
}

def_tag!(break_tag, "break");
#[derive(Deserialize, Debug, Clone)]
pub struct Break(break_tag, pub ParenOrArgsAddBlock, pub LineCol);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum ParenOrArgsAddBlock {
    YieldParen(YieldParen),
    ArgsAddBlock(ArgsAddBlock),
    Empty(Vec<()>),
}

impl ParenOrArgsAddBlock {
    fn is_paren(&self) -> bool {
        match self {
            ParenOrArgsAddBlock::YieldParen(_) => true,
            _ => false,
        }
    }

    fn into_arg_node(self) -> ArgNode {
        match self {
            ParenOrArgsAddBlock::YieldParen(yp) => *yp.1,
            ParenOrArgsAddBlock::ArgsAddBlock(aab) => ArgNode::ArgsAddBlock(aab),
            ParenOrArgsAddBlock::Empty(_) => ArgNode::Null(None),
        }
    }
}

def_tag!(yield_paren_tag, "paren");
#[derive(Deserialize, Debug, Clone)]
pub struct YieldParen(yield_paren_tag, pub Box<ArgNode>);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum Block {
    BraceBlock(BraceBlock),
    DoBlock(DoBlock),
}

// block local variables are a nightmare, they can be false, nil, or an array
// of idents:
//
// 1. nil if params are not present, and block local variables are also not
//    specified
// 2. false if params are present, and block local variables are not present
// 3. a vec of idents either if params are or are not present, and block local
//    variables are present
#[derive(RipperDeserialize, Debug, Clone)]
pub enum BlockLocalVariables {
    EmptyBecauseParamsWerePresent(bool),
    NilBecauseParamsWereNotPresent(Option<()>),
    Present(Vec<Ident>),
}

def_tag!(block_var_tag, "block_var");
#[derive(Deserialize, Debug, Clone)]
pub struct BlockVar(
    block_var_tag,
    pub Option<Box<Params>>,
    pub BlockLocalVariables,
);

def_tag!(do_block_tag, "do_block");
#[derive(Deserialize, Debug, Clone)]
pub struct DoBlock(do_block_tag, pub Option<BlockVar>, pub Box<BodyStmt>);

def_tag!(brace_block_tag, "brace_block");
#[derive(Deserialize, Debug, Clone)]
pub struct BraceBlock(brace_block_tag, pub Option<BlockVar>, pub Vec<Expression>);

def_tag!(while_tag, "while");
#[derive(Deserialize, Debug, Clone)]
pub struct While(while_tag, pub Box<Expression>, pub Vec<Expression>);

def_tag!(until_tag, "until");
#[derive(Deserialize, Debug, Clone)]
pub struct Until(until_tag, pub Box<Expression>, pub Vec<Expression>);

def_tag!(while_mod_tag, "while_mod");
#[derive(Deserialize, Debug, Clone)]
pub struct WhileMod(while_mod_tag, pub Box<Expression>, pub Box<Expression>);

def_tag!(until_mod_tag, "until_mod");
#[derive(Deserialize, Debug, Clone)]
pub struct UntilMod(until_mod_tag, pub Box<Expression>, pub Box<Expression>);

def_tag!(case_tag, "case");
#[derive(Deserialize, Debug, Clone)]
pub struct Case(case_tag, pub Option<Box<Expression>>, pub When, pub LineCol);

def_tag!(when_tag, "when");
#[derive(Deserialize, Debug, Clone)]
pub struct When(
    when_tag,
    pub ArgsAddStarOrExpressionList,
    pub Vec<Expression>,
    pub Option<Box<WhenOrElse>>,
    pub LineCol,
);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum WhenOrElse {
    When(When),
    Else(CaseElse),
}

def_tag!(case_else_tag, "else");
#[derive(Deserialize, Debug, Clone)]
pub struct CaseElse(case_else_tag, pub Vec<Expression>);

def_tag!(retry_tag, "retry");
#[derive(Deserialize, Debug, Clone)]
pub struct Retry(retry_tag);

def_tag!(sclass_tag, "sclass");
#[derive(Deserialize, Debug, Clone)]
pub struct SClass(sclass_tag, pub Box<Expression>, pub Box<BodyStmt>);

// some constructs were expressionlist in 2.5 and bodystmt in 2.6 so this
// deals with both cases
#[derive(RipperDeserialize, Debug, Clone)]
pub enum ExpressionListOrBodyStmt {
    ExpresionList(Vec<Expression>),
    BodyStmt(Box<BodyStmt>),
}

def_tag!(stabby_lambda_tag, "lambda");
#[derive(Deserialize, Debug, Clone)]
pub struct StabbyLambda(
    stabby_lambda_tag,
    pub ParenOrParams,
    pub String,
    pub ExpressionListOrBodyStmt,
    pub LineCol,
);

def_tag!(imaginary_tag, "@imaginary");
#[derive(Deserialize, Debug, Clone)]
pub struct Imaginary(imaginary_tag, pub String, pub LineCol);

def_tag!(rational_tag, "@rational");
#[derive(Deserialize, Debug, Clone)]
pub struct Rational(rational_tag, pub String, pub LineCol);

def_tag!(for_tag, "for");
#[derive(Deserialize, Debug, Clone)]
pub struct For(
    for_tag,
    pub VarFieldOrVarFields,
    pub Box<Expression>,
    pub Vec<Expression>,
);

#[derive(RipperDeserialize, Debug, Clone)]
pub enum VarFieldOrVarFields {
    VarField(VarField),
    VarFields(Vec<VarField>),
}

// ternary
def_tag!(ifop_tag, "ifop");
#[derive(Deserialize, Debug, Clone)]
pub struct IfOp(
    ifop_tag,
    pub Box<Expression>,
    pub Box<Expression>,
    pub Box<Expression>,
);
