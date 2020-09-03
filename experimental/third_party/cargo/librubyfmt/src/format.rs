use crate::delimiters::BreakableDelims;
use crate::parser_state::{FormattingContext, ParserState};
use crate::ripper_tree_types::*;
use log::debug;

pub fn format_def(ps: &mut ParserState, def: Def) {
    let def_expression = (def.1).to_def_parts();

    let body = def.3;
    ps.on_line((def_expression.1).0);
    if ps.at_start_of_line() {
        ps.emit_indent();
    }
    ps.emit_def(def_expression.0);
    format_paren_or_params(ps, def.2);

    ps.with_formatting_context(FormattingContext::Def, |ps| {
        ps.new_block(|ps| {
            ps.emit_newline();
            ps.with_start_of_line(true, |ps| {
                format_bodystmt(ps, body);
            });
        });
    });

    ps.with_start_of_line(true, |ps| {
        ps.wind_line_forward();
        ps.emit_end();
    });
    ps.emit_newline();
}

pub fn inner_format_params(ps: &mut ParserState, params: Box<Params>) {
    let non_null_positions = params.non_null_positions();
    //def foo(a, b=nil, *args, d, e:, **kwargs, &blk)
    //        ^  ^___^  ^___^  ^  ^    ^_____^   ^
    //        |    |      |    |  |      |       |
    //        |    |      |    |  |      |    block_arg
    //        |    |      |    |  |      |
    //        |    |      |    |  |  kwrest_params
    //        |    |      |    |  |
    //        |    |      |    | kwargs
    //        |    |      |    |
    //        |    |      | more_required_params
    //        |    |      |
    //        |    |  rest_params
    //        |    |
    //        | optional params
    //        |
    //    required params
    let required_params = (params.1).unwrap_or_default();
    let optional_params = (params.2).unwrap_or_default();
    let rest_param = params.3;
    let more_required_params = (params.4).unwrap_or_default();
    let kwargs = (params.5).unwrap_or_default();
    let kwrest_params = params.6;
    let block_arg = params.7;

    let formats: Vec<Box<dyn FnOnce(&mut ParserState) -> bool>> = vec![
        Box::new(move |ps: &mut ParserState| format_required_params(ps, required_params)),
        Box::new(move |ps: &mut ParserState| format_optional_params(ps, optional_params)),
        Box::new(move |ps: &mut ParserState| format_rest_param(ps, rest_param)),
        Box::new(move |ps: &mut ParserState| format_required_params(ps, more_required_params)),
        Box::new(move |ps: &mut ParserState| format_kwargs(ps, kwargs)),
        Box::new(move |ps: &mut ParserState| format_kwrest_params(ps, kwrest_params)),
        Box::new(move |ps: &mut ParserState| format_block_arg(ps, block_arg)),
    ];

    for (idx, format_fn) in formats.into_iter().enumerate() {
        let did_emit = format_fn(ps);
        let have_more = non_null_positions[idx + 1..].iter().any(|&v| v);

        if did_emit && have_more {
            ps.emit_comma();
            ps.emit_soft_newline();
        }
    }
}

pub fn format_blockvar(ps: &mut ParserState, bv: BlockVar) {
    let f_params = match bv.2 {
        BlockLocalVariables::Present(v) => Some(v),
        _ => None,
    };

    let params = bv.1;

    let have_any_params = match &params {
        Some(params) => params.non_null_positions().iter().any(|&v| v) || f_params.is_some(),
        None => f_params.is_some(),
    };

    if !have_any_params {
        return;
    }

    ps.breakable_of(BreakableDelims::for_block_params(), |ps| {
        if let Some(params) = params {
            inner_format_params(ps, params);
        }

        match f_params {
            None => {}
            Some(f_params) => {
                if !f_params.is_empty() {
                    ps.emit_ident(";".to_string());

                    ps.with_start_of_line(false, |ps| {
                        format_list_like_thing_items(
                            ps,
                            f_params.into_iter().map(Expression::Ident).collect(),
                            true,
                        );
                    });
                }
            }
        }
        ps.emit_collapsing_newline();
    });
}

pub fn format_params(ps: &mut ParserState, params: Box<Params>, delims: BreakableDelims) {
    let have_any_params = params.non_null_positions().iter().any(|&x| x);
    if !have_any_params {
        return;
    }

    ps.breakable_of(delims, |ps| {
        inner_format_params(ps, params);
        ps.emit_collapsing_newline();
    });
}

pub fn format_kwrest_params(ps: &mut ParserState, kwrest_params: Option<KwRestParam>) -> bool {
    if kwrest_params.is_none() {
        return false;
    }

    ps.with_start_of_line(false, |ps| {
        ps.emit_soft_indent();
        ps.emit_ident("**".to_string());
        let ident = (kwrest_params.unwrap()).1;
        if let Some(ident) = ident {
            format_ident(ps, ident);
        }
    });
    true
}

pub fn format_block_arg(ps: &mut ParserState, block_arg: Option<BlockArg>) -> bool {
    if block_arg.is_none() {
        return false;
    }

    ps.with_start_of_line(false, |ps| {
        ps.emit_soft_indent();
        ps.emit_ident("&".to_string());
        format_ident(ps, block_arg.unwrap().1);
    });

    true
}

pub fn format_kwargs(ps: &mut ParserState, kwargs: Vec<(Label, ExpressionOrFalse)>) -> bool {
    if kwargs.is_empty() {
        return false;
    }

    ps.with_start_of_line(false, |ps| {
        let len = kwargs.len();
        for (idx, (label, expr_or_false)) in kwargs.into_iter().enumerate() {
            ps.emit_soft_indent();
            handle_string_and_linecol(ps, label.1, label.2);

            match expr_or_false {
                ExpressionOrFalse::Expression(e) => {
                    ps.emit_space();
                    format_expression(ps, e);
                }
                ExpressionOrFalse::False(_) => {}
            }
            emit_params_separator(ps, idx, len);
        }
    });

    true
}

pub fn format_rest_param(
    ps: &mut ParserState,
    rest_param: Option<RestParamOr0OrExcessedComma>,
) -> bool {
    match rest_param {
        None => false,
        Some(RestParamOr0OrExcessedComma::ExcessedComma(_)) => false,
        Some(RestParamOr0OrExcessedComma::Zero(_)) => false,
        Some(RestParamOr0OrExcessedComma::RestParam(rp)) => {
            ps.emit_soft_indent();
            ps.emit_ident("*".to_string());
            ps.with_start_of_line(false, |ps| {
                match rp.1 {
                    Some(IdentOrVarField::Ident(i)) => {
                        format_ident(ps, i);
                    }
                    Some(IdentOrVarField::VarField(vf)) => {
                        format_var_field(ps, vf);
                    }
                    None => {
                        // deliberately do nothing
                    }
                }
            });

            true
        }
    }
}

pub fn format_optional_params(
    ps: &mut ParserState,
    optional_params: Vec<(Ident, Expression)>,
) -> bool {
    if optional_params.is_empty() {
        return false;
    }

    ps.with_start_of_line(false, |ps| {
        let len = optional_params.len();
        for (idx, (left, right)) in optional_params.into_iter().enumerate() {
            ps.emit_soft_indent();
            format_ident(ps, left);
            ps.emit_ident(" = ".to_string());
            format_expression(ps, right);
            emit_params_separator(ps, idx, len);
        }
    });

    true
}

pub fn format_mlhs(ps: &mut ParserState, mlhs: MLhs) {
    ps.emit_open_paren();

    ps.with_start_of_line(false, |ps| {
        let mut first = true;
        for inner in mlhs.0 {
            if !first {
                ps.emit_comma_space();
            }
            first = false;

            match inner {
                MLhsInner::Field(f) => format_field(ps, f),
                MLhsInner::Ident(i) => format_ident(ps, i),
                MLhsInner::RestParam(rp) => {
                    format_rest_param(ps, Some(RestParamOr0OrExcessedComma::RestParam(rp)));
                }
                MLhsInner::VarField(vf) => format_var_field(ps, vf),
                MLhsInner::MLhs(mlhs) => format_mlhs(ps, *mlhs),
            }
        }
    });

    ps.emit_close_paren();
}

pub fn format_required_params(ps: &mut ParserState, required_params: Vec<IdentOrMLhs>) -> bool {
    if required_params.is_empty() {
        return false;
    }

    ps.with_start_of_line(false, |ps| {
        let len = required_params.len();
        for (idx, ident) in required_params.into_iter().enumerate() {
            ps.emit_soft_indent();
            match ident {
                IdentOrMLhs::Ident(ident) => format_ident(ps, ident),
                IdentOrMLhs::MLhs(mlhs) => format_mlhs(ps, mlhs),
            }
            emit_params_separator(ps, idx, len);
        }
    });
    true
}

pub fn emit_params_separator(ps: &mut ParserState, index: usize, length: usize) {
    if index != length - 1 {
        ps.emit_comma();
        ps.emit_soft_newline();
    }
}

pub fn format_bodystmt(ps: &mut ParserState, bodystmt: Box<BodyStmt>) {
    let expressions = bodystmt.1;
    let rescue_part = bodystmt.2;
    let else_part = bodystmt.3;
    let ensure_part = bodystmt.4;

    for expression in expressions {
        format_expression(ps, expression);
    }

    format_rescue(ps, rescue_part);
    format_else(ps, else_part);
    format_ensure(ps, ensure_part);
}

pub fn format_mrhs(ps: &mut ParserState, mrhs: Option<MRHS>) {
    match mrhs {
        None => {}
        Some(MRHS::Single(expr)) => {
            format_expression(ps, *expr);
        }
        Some(MRHS::SingleAsArray(exprs)) => {
            if exprs.len() != 1 {
                panic!("this should be impossible, bug in the ruby parser?");
            }
            format_expression(
                ps,
                exprs
                    .into_iter()
                    .next()
                    .expect("we checked there's one item"),
            );
        }
        Some(MRHS::MRHSNewFromArgs(mnfa)) => {
            format_mrhs_new_from_args(ps, mnfa);
        }
        Some(MRHS::MRHSAddStar(mas)) => {
            format_mrhs_add_star(ps, mas);
        }
        Some(MRHS::Array(array)) => {
            format_array(ps, array);
        }
    }
}

pub fn format_rescue_capture(ps: &mut ParserState, rescue_capture: Option<Assignable>) {
    match rescue_capture {
        None => {}
        Some(expr) => {
            ps.emit_space();
            ps.emit_ident("=>".to_string());
            ps.emit_space();
            format_assignable(ps, expr);
        }
    }
}

pub fn format_rescue(ps: &mut ParserState, rescue_part: Option<Rescue>) {
    match rescue_part {
        None => {}
        Some(Rescue(_, class, capture, expressions, more_rescue)) => {
            ps.dedent(|ps| {
                ps.emit_indent();
                ps.emit_rescue();
                ps.with_start_of_line(false, |ps| {
                    if class.is_some() || capture.is_some() {
                        ps.emit_space();
                    }

                    format_mrhs(ps, class);
                    format_rescue_capture(ps, capture);
                });
            });

            match expressions {
                None => {}
                Some(expressions) => {
                    ps.emit_newline();
                    for expression in expressions {
                        format_expression(ps, expression);
                    }
                }
            }

            format_rescue(ps, more_rescue.map(|v| *v));
        }
    }
}

pub fn format_else(ps: &mut ParserState, else_part: Option<RescueElseOrExpressionList>) {
    match else_part {
        None => {}
        Some(RescueElseOrExpressionList::ExpressionList(exprs)) => {
            ps.dedent(|ps| {
                ps.emit_indent();
                ps.emit_else();
                ps.wind_line_forward();
            });
            ps.emit_newline();
            ps.with_start_of_line(true, |ps| {
                for expr in exprs {
                    format_expression(ps, expr);
                }
            });
        }
        Some(RescueElseOrExpressionList::RescueElse(re)) => {
            ps.dedent(|ps| {
                ps.emit_indent();
                ps.emit_else();
            });

            match re.1 {
                None => {}
                Some(exprs) => {
                    ps.wind_line_forward();
                    ps.emit_newline();
                    ps.wind_line_forward();
                    ps.with_start_of_line(true, |ps| {
                        for expr in exprs {
                            format_expression(ps, expr);
                        }
                    });
                }
            }
        }
    }
}

pub fn format_ensure(ps: &mut ParserState, ensure_part: Option<Ensure>) {
    match ensure_part {
        None => {}
        Some(e) => {
            ps.dedent(|ps| {
                ps.wind_line_forward();
                ps.emit_indent();
                ps.emit_ensure();
            });

            match e.1 {
                None => {}
                Some(exprs) => {
                    ps.emit_newline();
                    ps.with_start_of_line(true, |ps| {
                        for expr in exprs {
                            format_expression(ps, expr);
                        }
                    });
                }
            }
        }
    }
}

pub fn use_parens_for_method_call(
    ps: &ParserState,
    method: &IdentOrOpOrKeywordOrConst,
    args: &ArgsAddStarOrExpressionList,
    original_used_parens: bool,
    context: FormattingContext,
) -> bool {
    let name = method.get_name();
    debug!("name: {:?}", name);
    if name.starts_with("attr_") && context == FormattingContext::ClassOrModule {
        return false;
    }

    if name == "return" || name == "raise" || name == "yield" || name == "break" {
        if ps.current_formatting_context() == FormattingContext::Binary {
            return true;
        }
        match args {
            ArgsAddStarOrExpressionList::ArgsAddStar(_) => return true,
            _ => return false,
        }
    }

    if name == "super" || name == "require" {
        return original_used_parens;
    }

    if args.is_empty() {
        return false;
    }

    if context == FormattingContext::ClassOrModule && !original_used_parens {
        return false;
    }

    true
}

pub fn format_dot_type(ps: &mut ParserState, dt: DotType) {
    match dt {
        DotType::Dot(_) => ps.emit_dot(),
        DotType::LonelyOperator(_) => ps.emit_lonely_operator(),
    }
}

pub fn format_dot(ps: &mut ParserState, dot: DotTypeOrOp) {
    match dot {
        DotTypeOrOp::DotType(dt) => format_dot_type(ps, dt),
        DotTypeOrOp::Op(op) => {
            let lc = op.2;
            ps.on_line(lc.0);
            match op.1 {
                Operator::Dot(dot) => format_dot_type(ps, DotType::Dot(dot)),
                Operator::LonelyOperator(dot) => format_dot_type(ps, DotType::LonelyOperator(dot)),
                Operator::StringOperator(string) => ps.emit_ident(string),
                x => panic!(
                    "should be impossible, dot position operator parsed as not a dot, {:?}",
                    x
                ),
            }
        }
        DotTypeOrOp::Period(_) => {
            ps.emit_dot();
        }
        DotTypeOrOp::ColonColon(_) => {
            ps.emit_colon_colon();
        }
        DotTypeOrOp::StringDot(s) => {
            ps.emit_ident(s);
        }
    }
}

pub fn format_method_call(ps: &mut ParserState, method_call: MethodCall) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    let (chain, method, original_used_parens, args) =
        (method_call.1, method_call.2, method_call.3, method_call.4);

    debug!("method call!!");
    let use_parens = use_parens_for_method_call(
        ps,
        &method,
        &args,
        original_used_parens,
        ps.current_formatting_context(),
    );

    ps.with_start_of_line(false, |ps| {
        format_call_chain(ps, chain);
        format_ident(ps, method.into_ident());

        let delims = if use_parens {
            BreakableDelims::for_method_call()
        } else {
            BreakableDelims::for_kw()
        };

        if !args.is_empty() {
            ps.breakable_of(delims, |ps| {
                ps.with_formatting_context(FormattingContext::ArgsList, |ps| {
                    format_list_like_thing(ps, args, false);
                    ps.emit_collapsing_newline();
                });
            });
        } else if use_parens {
            ps.emit_open_paren();
            ps.emit_close_paren();
        }
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SpecialCase {
    NoSpecialCase,
    NoLeadingTrailingCollectionMarkers,
}

pub fn format_list_like_thing_items(
    ps: &mut ParserState,
    args: Vec<Expression>,
    single_line: bool,
) -> bool {
    let mut emitted_args = false;
    let args_count = args.len();

    ps.magic_handle_comments_for_mulitiline_arrays(|ps| {
        for (idx, expr) in args.into_iter().enumerate() {
            // this raise was present in the ruby source code of rubyfmt
            // but I'm pretty sure it's categorically impossible now. Thanks
            // type system
            //raise "this is bad" if expr[0] == :tstring_content

            if single_line {
                format_expression(ps, expr);
                if idx != args_count - 1 {
                    ps.emit_comma_space();
                }
            } else {
                ps.emit_soft_indent();
                ps.with_start_of_line(false, |ps| {
                    match expr {
                        Expression::BareAssocHash(bah) => format_assocs(
                            ps,
                            bah.1,
                            SpecialCase::NoLeadingTrailingCollectionMarkers,
                        ),
                        expr => format_expression(ps, expr),
                    }
                    if idx != args_count - 1 {
                        ps.emit_comma();
                        ps.emit_soft_newline();
                    }
                });
            };
            emitted_args = true;
        }
    });
    emitted_args
}

pub fn format_ident(ps: &mut ParserState, ident: Ident) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    handle_string_and_linecol(ps, ident.1, ident.2);

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_const(ps: &mut ParserState, c: Const) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    handle_string_and_linecol(ps, c.1, c.2);

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_int(ps: &mut ParserState, int: Int) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    handle_string_and_linecol(ps, int.1, int.2);

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_bare_assoc_hash(ps: &mut ParserState, bah: BareAssocHash) {
    format_assocs(ps, bah.1, SpecialCase::NoSpecialCase)
}

pub fn format_alias(ps: &mut ParserState, alias: Alias) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.emit_ident("alias ".to_string());

    ps.with_start_of_line(false, |ps| {
        format_symbol_literal(ps, alias.1);
        ps.emit_space();
        format_symbol_literal(ps, alias.2);
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_op(ps: &mut ParserState, op: Op) {
    match op.1 {
        Operator::Equals(_) => ps.emit_ident("==".to_string()),
        Operator::Dot(_) => ps.emit_dot(),
        Operator::LonelyOperator(_) => ps.emit_lonely_operator(),
        Operator::StringOperator(s) => ps.emit_ident(s),
    }
}

pub fn format_kw(ps: &mut ParserState, kw: Kw) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    handle_string_and_linecol(ps, kw.1, kw.2);

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_symbol(ps: &mut ParserState, symbol: Symbol) {
    ps.emit_ident(":".to_string());
    match symbol.1 {
        IdentOrConstOrKwOrOpOrIvarOrGvar::Ident(i) => format_ident(ps, i),
        IdentOrConstOrKwOrOpOrIvarOrGvar::Const(c) => format_const(ps, c),
        IdentOrConstOrKwOrOpOrIvarOrGvar::Keyword(kw) => format_kw(ps, kw),
        IdentOrConstOrKwOrOpOrIvarOrGvar::Op(op) => format_op(ps, op),
        IdentOrConstOrKwOrOpOrIvarOrGvar::IVar(ivar) => {
            format_var_ref_type(ps, VarRefType::IVar(ivar))
        }
        IdentOrConstOrKwOrOpOrIvarOrGvar::GVar(gvar) => {
            format_var_ref_type(ps, VarRefType::GVar(gvar))
        }
    }
}

pub fn format_symbol_literal(ps: &mut ParserState, symbol_literal: SymbolLiteral) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| match symbol_literal.1 {
        SymbolOrBare::Ident(ident) => format_ident(ps, ident),
        SymbolOrBare::Kw(kw) => format_kw(ps, kw),
        SymbolOrBare::Op(op) => format_op(ps, op),
        SymbolOrBare::Symbol(symbol) => format_symbol(ps, symbol),
        SymbolOrBare::GVar(gvar) => format_var_ref_type(ps, VarRefType::GVar(gvar)),
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_assocs(ps: &mut ParserState, assocs: Vec<AssocNewOrAssocSplat>, sc: SpecialCase) {
    let len = assocs.len();
    for (idx, assoc) in assocs.into_iter().enumerate() {
        if sc != SpecialCase::NoLeadingTrailingCollectionMarkers || idx != 0 {
            ps.emit_soft_indent();
        }
        ps.with_start_of_line(false, |ps| match assoc {
            AssocNewOrAssocSplat::AssocNew(new) => {
                match new.1 {
                    AssocKey::Label(label) => {
                        handle_string_and_linecol(ps, label.1, label.2);
                        ps.emit_space();
                    }
                    AssocKey::Expression(expression) => {
                        format_expression(ps, expression);
                        ps.emit_space();
                        ps.emit_ident("=>".to_string());
                        ps.emit_space();
                    }
                }
                format_expression(ps, new.2);
            }
            AssocNewOrAssocSplat::AssocSplat(splat) => {
                ps.emit_ident("**".to_string());
                format_expression(ps, splat.1);
            }
        });
        if idx != len - 1 {
            ps.emit_comma();
        }
        ps.emit_soft_newline();
    }
}

pub fn format_begin(ps: &mut ParserState, begin: Begin) {
    if ps.at_start_of_line() {
        ps.emit_indent()
    }

    ps.wind_line_forward();
    ps.emit_begin();
    ps.emit_newline();
    ps.new_block(|ps| {
        ps.with_start_of_line(true, |ps| format_bodystmt(ps, begin.1));
    });

    ps.with_start_of_line(true, |ps| {
        ps.emit_end();
    });
    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_begin_block(ps: &mut ParserState, begin: BeginBlock) {
    if ps.at_start_of_line() {
        ps.emit_indent()
    }

    ps.wind_line_forward();
    ps.emit_begin_block();
    ps.emit_space();
    ps.emit_open_curly_bracket();
    ps.emit_newline();
    ps.new_block(|ps| {
        ps.with_start_of_line(true, |ps| {
            for expr in begin.1 {
                format_expression(ps, expr);
            }
        });
    });

    ps.with_start_of_line(true, |ps| {
        ps.emit_close_curly_bracket();
    });
    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_end_block(ps: &mut ParserState, end: EndBlock) {
    if ps.at_start_of_line() {
        ps.emit_indent()
    }

    ps.wind_line_forward();
    ps.emit_end_block();
    ps.emit_space();
    ps.emit_open_curly_bracket();
    ps.emit_newline();

    ps.new_block(|ps| {
        ps.with_start_of_line(true, |ps| {
            for expr in end.1 {
                format_expression(ps, expr);
            }
        });
    });

    ps.with_start_of_line(true, |ps| {
        ps.emit_close_curly_bracket();
    });
    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn normalize(e: Expression) -> Expression {
    match e {
        Expression::VCall(v) => Expression::MethodCall(v.to_method_call()),
        Expression::MethodAddArg(maa) => Expression::MethodCall(maa.to_method_call()),
        Expression::Command(command) => Expression::MethodCall(command.to_method_call()),
        Expression::CommandCall(call) => Expression::MethodCall(call.to_method_call()),
        Expression::Call(call) => Expression::MethodCall(call.to_method_call()),
        Expression::Super(sup) => Expression::MethodCall(sup.to_method_call()),
        e => e,
    }
}

pub fn format_void_stmt(_ps: &mut ParserState, _void: VoidStmt) {
    // deliberately does nothing
}

pub fn format_paren(ps: &mut ParserState, paren: ParenExpr) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }
    ps.emit_open_paren();

    if paren.1.len() == 1 {
        let p = (paren.1)
            .into_iter()
            .next()
            .expect("we know this isn't empty");
        ps.with_start_of_line(false, |ps| format_expression(ps, p));
    } else {
        ps.emit_newline();
        ps.new_block(|ps| {
            ps.with_start_of_line(true, |ps| {
                for expr in (paren.1).into_iter() {
                    format_expression(ps, expr);
                }
            });
        });
    }

    ps.emit_close_paren();
    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_dot2(ps: &mut ParserState, dot2: Dot2) {
    format_dot2_or_3(ps, "..".to_string(), dot2.1, dot2.2);
}

pub fn format_dot3(ps: &mut ParserState, dot3: Dot3) {
    format_dot2_or_3(ps, "...".to_string(), dot3.1, dot3.2);
}

pub fn format_dot2_or_3(
    ps: &mut ParserState,
    dots: String,
    left: Option<Box<Expression>>,
    right: Option<Box<Expression>>,
) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        if let Some(expr) = left {
            format_expression(ps, *expr)
        }

        ps.emit_ident(dots);

        if let Some(expr) = right {
            format_expression(ps, *expr)
        }
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn percent_symbol_for(tag: String) -> String {
    match tag.as_ref() {
        "qsymbols" => "%i".to_string(),
        "qwords" => "%w".to_string(),
        "symbols" => "%I".to_string(),
        "words" => "%W".to_string(),
        _ => panic!("got invalid percent symbol"),
    }
}

pub fn format_percent_array(ps: &mut ParserState, tag: String, parts: Vec<Vec<StringContentPart>>) {
    ps.emit_ident(percent_symbol_for(tag));
    ps.with_start_of_line(false, |ps| {
        ps.breakable_of(BreakableDelims::for_array(), |ps| {
            let parts_length = parts.len();
            for (idx, part) in parts.into_iter().enumerate() {
                ps.emit_soft_indent();
                format_inner_string(ps, part, StringType::Array);
                if idx != parts_length - 1 {
                    ps.emit_soft_newline();
                }
            }
            ps.emit_collapsing_newline();
        });
    });
}

pub fn format_array(ps: &mut ParserState, array: Array) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    if let Some(location) = array.2 {
        ps.on_line(location.0);
    }

    match array.1 {
        SimpleArrayOrPercentArray::SimpleArray(a) => format_array_fast_path(ps, a),
        SimpleArrayOrPercentArray::LowerPercentArray(pa) => {
            ps.on_line((pa.2).0);
            format_percent_array(
                ps,
                pa.0,
                pa.1.into_iter()
                    .map(|v| vec![StringContentPart::TStringContent(v)])
                    .collect(),
            );
        }
        SimpleArrayOrPercentArray::UpperPercentArray(pa) => {
            ps.on_line((pa.2).0);
            format_percent_array(ps, pa.0, pa.1);
        }
    }

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_array_fast_path(ps: &mut ParserState, a: Option<ArgsAddStarOrExpressionList>) {
    match a {
        None => {
            ps.emit_open_square_bracket();
            ps.emit_close_square_bracket();
        }
        Some(a) => {
            ps.breakable_of(BreakableDelims::for_array(), |ps| {
                format_list_like_thing(ps, a, false);
                ps.emit_collapsing_newline();

                if ps.last_breakable_is_multiline() {
                    ps.wind_line_forward();
                }
            });
        }
    }
}

pub fn format_list_like_thing(
    ps: &mut ParserState,
    a: ArgsAddStarOrExpressionList,
    single_line: bool,
) -> bool {
    match a {
        ArgsAddStarOrExpressionList::ArgsAddStar(aas) => {
            let left = aas.1;
            let star = aas.2;
            let right = aas.3;
            let mut emitted_args = format_list_like_thing(ps, *left, single_line);

            if single_line {
                // if we're single line, our predecessor didn't emit a trailing comma
                // space because rubyfmt terminates single line arg lists without the
                // trailer so emit one here
                if emitted_args {
                    ps.emit_comma_space();
                }
            } else {
                // similarly if we're multi line, we emit a newline but not an indent
                // at the end our formatting spree, because we might be at a terminator
                // so fix up the indent
                if emitted_args {
                    ps.emit_comma();
                    ps.emit_soft_newline();
                }
                ps.emit_soft_indent();
            }

            emitted_args = true;

            ps.with_start_of_line(false, |ps| {
                ps.emit_ident("*".to_string());
                format_expression(ps, *star);

                for expr in right {
                    emit_intermediate_array_separator(ps, single_line);
                    format_expression(ps, expr);
                }
            });

            emitted_args
        }
        ArgsAddStarOrExpressionList::ExpressionList(el) => {
            format_list_like_thing_items(ps, el, single_line)
        }
    }
}

pub fn emit_intermediate_array_separator(ps: &mut ParserState, single_line: bool) {
    if single_line {
        ps.emit_comma_space();
    } else {
        ps.emit_comma();
        ps.emit_soft_newline();
        ps.emit_soft_indent();
    }
}

#[derive(PartialEq, Debug)]
pub enum StringType {
    Quoted,
    Heredoc,
    Array,
    Regexp,
}

pub fn format_inner_string(ps: &mut ParserState, parts: Vec<StringContentPart>, tipe: StringType) {
    let mut peekable = parts.into_iter().peekable();
    while peekable.peek().is_some() {
        let part = peekable.next().expect("we peeked");
        match part {
            StringContentPart::TStringContent(t) => {
                if tipe != StringType::Heredoc {
                    ps.on_line((t.2).0);
                }
                ps.emit_string_content(t.1);
            }
            StringContentPart::StringEmbexpr(e) => {
                ps.emit_string_content("#{".to_string());
                ps.with_start_of_line(false, |ps| {
                    let expr = ((e.1).into_iter()).next().expect("should not be empty");
                    format_expression(ps, expr);
                });
                ps.emit_string_content("}".to_string());

                let on_line_skip = tipe == StringType::Heredoc
                    && match peekable.peek() {
                        Some(StringContentPart::TStringContent(TStringContent(_, s, _))) => {
                            s.starts_with('\n')
                        }
                        _ => false,
                    };
                if on_line_skip {
                    ps.render_heredocs(true)
                }
            }
            StringContentPart::StringDVar(dv) => {
                ps.emit_string_content("#{".to_string());
                ps.with_start_of_line(false, |ps| {
                    let expr = *(dv.1);
                    format_expression(ps, expr);
                });
                ps.emit_string_content("}".to_string());
            }
        }
    }
}

pub fn format_heredoc_string_literal(
    ps: &mut ParserState,
    hd: HeredocStringLiteral,
    parts: Vec<StringContentPart>,
) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_surpress_comments(true, |ps| {
        let heredoc_type = (hd.1).0;
        let heredoc_symbol = (hd.1).1;
        ps.emit_ident(heredoc_type.clone());
        ps.emit_ident(heredoc_symbol.clone());

        ps.push_heredoc_content(heredoc_symbol, heredoc_type.contains('~'), parts);
    });

    if ps.at_start_of_line() && !ps.is_absorbing_indents() {
        ps.emit_newline();
    }
}

pub fn format_string_literal(ps: &mut ParserState, sl: StringLiteral) {
    match sl {
        StringLiteral::Heredoc(_, hd, StringContent(_, parts)) => {
            format_heredoc_string_literal(ps, hd, parts)
        }
        StringLiteral::Normal(_, StringContent(_, parts)) => {
            if ps.at_start_of_line() {
                ps.emit_indent();
            }

            ps.emit_double_quote();
            format_inner_string(ps, parts, StringType::Quoted);
            ps.emit_double_quote();

            if ps.at_start_of_line() {
                ps.emit_newline();
            }
        }
    }
}

pub fn format_xstring_literal(ps: &mut ParserState, xsl: XStringLiteral) {
    let parts = xsl.1;

    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.emit_ident("`".to_string());
    format_inner_string(ps, parts, StringType::Quoted);
    ps.emit_ident("`".to_string());

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_const_path_field(ps: &mut ParserState, cf: ConstPathField) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        format_expression(ps, *cf.1);
        ps.emit_colon_colon();
        format_const(ps, cf.2);
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_top_const_field(ps: &mut ParserState, tcf: TopConstField) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        ps.emit_colon_colon();
        format_const(ps, tcf.1);
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_var_field(ps: &mut ParserState, vf: VarField) {
    let left = vf.1;
    format_var_ref_type(ps, left);
}

pub fn format_aref_field(ps: &mut ParserState, af: ArefField) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        format_expression(ps, *af.1);
        ps.emit_open_square_bracket();
        let aab = af.2;
        match aab.2 {
            ToProcExpr::Present(_) => {
                panic!("got a to_proc in an aref_field, should be impossible");
            }
            ToProcExpr::NotPresent(_) => {
                format_list_like_thing(ps, (aab.1).into_args_add_star_or_expression_list(), true);
            }
        }
        ps.emit_close_square_bracket();
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_field(ps: &mut ParserState, f: Field) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        format_expression(ps, *f.1);
        format_dot(ps, f.2);
        format_ident(ps, f.3);
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_assignable(ps: &mut ParserState, v: Assignable) {
    match v {
        Assignable::VarField(vf) => {
            format_var_field(ps, vf);
        }
        Assignable::ConstPathField(cf) => {
            format_const_path_field(ps, cf);
        }
        Assignable::RestParam(rp) => {
            format_rest_param(ps, Some(RestParamOr0OrExcessedComma::RestParam(rp)));
        }
        Assignable::TopConstField(tcf) => {
            format_top_const_field(ps, tcf);
        }
        Assignable::ArefField(af) => {
            format_aref_field(ps, af);
        }
        Assignable::Field(field) => {
            format_field(ps, field);
        }
        Assignable::Ident(ident) => {
            format_ident(ps, ident);
        }
    }
}

pub fn format_assign(ps: &mut ParserState, assign: Assign) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        format_assignable(ps, assign.1);
        let right = assign.2;

        ps.emit_space();
        ps.emit_op("=".to_string());
        ps.emit_space();

        ps.with_formatting_context(FormattingContext::Assign, |ps| match right {
            ExpressionOrMRHSNewFromArgs::Expression(e) => format_expression(ps, *e),
            ExpressionOrMRHSNewFromArgs::MRHSNewFromArgs(m) => format_mrhs_new_from_args(ps, m),
        });
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_massign(ps: &mut ParserState, massign: MAssign) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        match massign.1 {
            AssignableListOrMLhs::AssignableList(al) => {
                let length = al.len();
                for (idx, v) in al.into_iter().enumerate() {
                    format_assignable(ps, v);
                    let last = idx == length - 1;
                    if !last {
                        ps.emit_comma_space();
                    }
                    if length == 1 {
                        ps.emit_comma();
                    }
                }
            }
            AssignableListOrMLhs::MLhs(mlhs) => format_mlhs(ps, mlhs),
        }
        ps.emit_space();
        ps.emit_ident("=".to_string());
        ps.emit_space();
        match massign.2 {
            MRHSOrArray::MRHS(mrhs) => {
                format_mrhs(ps, Some(mrhs));
            }
            MRHSOrArray::Array(array) => {
                format_array(ps, array);
            }
        }
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_var_ref_type(ps: &mut ParserState, vr: VarRefType) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    match vr {
        VarRefType::CVar(c) => handle_string_and_linecol(ps, c.1, c.2),
        VarRefType::GVar(g) => handle_string_and_linecol(ps, g.1, g.2),
        VarRefType::IVar(i) => handle_string_and_linecol(ps, i.1, i.2),
        VarRefType::Ident(i) => handle_string_and_linecol(ps, i.1, i.2),
        VarRefType::Const(c) => handle_string_and_linecol(ps, c.1, c.2),
        VarRefType::Kw(kw) => handle_string_and_linecol(ps, kw.1, kw.2),
    }

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn handle_string_and_linecol(ps: &mut ParserState, ident: String, lc: LineCol) {
    ps.on_line(lc.0);
    ps.emit_ident(ident);
}

pub fn format_var_ref(ps: &mut ParserState, vr: VarRef) {
    format_var_ref_type(ps, vr.1);
}

pub fn format_const_path_ref(ps: &mut ParserState, cpr: ConstPathRef) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        format_expression(ps, *cpr.1);
        ps.emit_colon_colon();
        format_const(ps, cpr.2);
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_top_const_ref(ps: &mut ParserState, tcr: TopConstRef) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        ps.emit_colon_colon();
        format_const(ps, tcr.1);
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_defined(ps: &mut ParserState, defined: Defined) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        ps.emit_ident("defined?".to_string());
        ps.emit_open_paren();
        format_expression(ps, *defined.1);
        ps.emit_close_paren();
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_rescue_mod(ps: &mut ParserState, rescue_mod: RescueMod) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        format_expression(ps, *rescue_mod.1);
        ps.emit_space();
        ps.emit_rescue();
        ps.emit_space();
        format_expression(ps, *rescue_mod.2);
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_mrhs_new_from_args(ps: &mut ParserState, mnfa: MRHSNewFromArgs) {
    format_list_like_thing(ps, mnfa.1, true);

    if let Some(expr) = mnfa.2 {
        ps.emit_comma_space();
        format_expression(ps, *expr);
    }
}

pub fn format_mrhs_add_star(ps: &mut ParserState, mrhs: MRHSAddStar) {
    let first = mrhs.1;
    let second = mrhs.2;
    ps.with_start_of_line(false, |ps| {
        match first {
            MRHSNewFromArgsOrEmpty::Empty(e) => {
                if !e.is_empty() {
                    panic!("this should be impossible, got non-empty mrhs empty");
                }
            }
            MRHSNewFromArgsOrEmpty::MRHSNewFromArgs(mnfa) => {
                format_mrhs_new_from_args(ps, mnfa);
                ps.emit_comma_space();
            }
        }
        ps.emit_ident("*".to_string());
        ps.with_start_of_line(false, |ps| {
            format_expression(ps, *second);
        });
    });
}

pub fn format_next(ps: &mut ParserState, next: Next) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        ps.emit_ident("next".to_string());
        match next.1 {
            ArgsAddBlockOrExpressionList::ExpressionList(e) => {
                if !e.is_empty() {
                    ps.emit_space();
                    format_list_like_thing_items(ps, e, true);
                }
            }
            ArgsAddBlockOrExpressionList::ArgsAddBlock(aab) => match aab.2 {
                ToProcExpr::Present(_) => {
                    panic!("got a block in a next, should be impossible");
                }
                ToProcExpr::NotPresent(_) => {
                    ps.emit_space();
                    format_list_like_thing(
                        ps,
                        (aab.1).into_args_add_star_or_expression_list(),
                        true,
                    );
                }
            },
        }
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_unary(ps: &mut ParserState, unary: Unary) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        match unary.1 {
            UnaryType::Not => {
                ps.emit_ident("not".to_string());
                ps.emit_space();
            }
            UnaryType::Positive => {
                ps.emit_ident("+".to_string());
            }
            UnaryType::Negative => {
                ps.emit_ident("-".to_string());
            }
            UnaryType::BooleanNot => {
                ps.emit_ident("!".to_string());
            }
        }

        format_expression(ps, *unary.2);
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_string_concat(ps: &mut ParserState, sc: StringConcat) {
    ps.with_absorbing_indent_block(|ps| {
        let nested = sc.1;
        let sl = sc.2;

        ps.with_start_of_line(false, |ps| {
            match nested {
                StringConcatOrStringLiteral::StringConcat(sc) => format_string_concat(ps, *sc),
                StringConcatOrStringLiteral::StringLiteral(sl) => format_string_literal(ps, sl),
            }

            ps.emit_space();
            ps.emit_slash();
            ps.emit_newline();

            ps.emit_indent();
            format_string_literal(ps, sl);
        });
    });
    if ps.at_start_of_line() && !ps.is_absorbing_indents() {
        ps.emit_newline();
    }
}

pub fn format_dyna_symbol(ps: &mut ParserState, ds: DynaSymbol) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.emit_ident(":".to_string());
    ps.with_start_of_line(false, |ps| {
        format_string_literal(ps, ds.to_string_literal());
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_undef(ps: &mut ParserState, undef: Undef) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.emit_ident("undef ".to_string());
    let length = undef.1.len();
    for (idx, literal) in undef.1.into_iter().enumerate() {
        ps.with_start_of_line(false, |ps| format_symbol_literal(ps, literal));
        if idx != length - 1 {
            ps.emit_comma_space();
        }
    }

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_defs(ps: &mut ParserState, defs: Defs) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    let singleton = defs.1;
    let ident_or_kw = defs.3;
    let paren_or_params = defs.4;
    let bodystmt = defs.5;

    ps.emit_def_keyword();
    ps.emit_space();

    ps.with_start_of_line(false, |ps| {
        match singleton {
            Singleton::VarRef(vr) => {
                format_var_ref(ps, vr);
            }
            Singleton::Paren(pe) => {
                format_paren(ps, pe);
            }
        }

        ps.emit_dot();
        let (ident, linecol) = ident_or_kw.to_def_parts();
        handle_string_and_linecol(ps, ident, linecol);
        format_paren_or_params(ps, paren_or_params);
        ps.emit_newline();
    });

    ps.with_formatting_context(FormattingContext::Def, |ps| {
        ps.new_block(|ps| {
            ps.with_start_of_line(true, |ps| {
                format_bodystmt(ps, bodystmt);
            });
        });
    });

    ps.emit_end();

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_paren_or_params(ps: &mut ParserState, pp: ParenOrParams) {
    let params = match pp {
        ParenOrParams::Paren(p) => p.1,
        ParenOrParams::Params(p) => p,
    };
    format_params(ps, params, BreakableDelims::for_method_call());
}

pub fn format_class(ps: &mut ParserState, class: Class) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    let class_name = class.1;
    let inherit = class.2;
    let bodystmt = class.3;

    ps.emit_class_keyword();
    ps.with_start_of_line(false, |ps| {
        ps.emit_space();

        match class_name {
            ConstPathRefOrConstRef::ConstPathRef(cpr) => {
                format_const_path_ref(ps, cpr);
            }
            ConstPathRefOrConstRef::ConstRef(cr) => {
                handle_string_and_linecol(ps, (cr.1).1, (cr.1).2);
            }
        }

        if inherit.is_some() {
            let inherit_expression = *(inherit.expect("We checked it is some"));
            ps.emit_ident(" < ".to_string());
            format_expression(ps, inherit_expression);
        }
    });

    ps.new_block(|ps| {
        ps.with_start_of_line(true, |ps| {
            ps.with_formatting_context(FormattingContext::ClassOrModule, |ps| {
                ps.emit_newline();
                format_bodystmt(ps, bodystmt);
            });
        });
    });

    ps.emit_end();
    ps.wind_line_forward();
    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_module(ps: &mut ParserState, module: Module) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    let module_name = module.1;
    let bodystmt = module.2;

    ps.emit_module_keyword();
    ps.with_start_of_line(false, |ps| {
        ps.emit_space();

        match module_name {
            ConstPathRefOrConstRef::ConstPathRef(cpr) => {
                format_const_path_ref(ps, cpr);
            }
            ConstPathRefOrConstRef::ConstRef(cr) => {
                handle_string_and_linecol(ps, (cr.1).1, (cr.1).2);
            }
        }
    });

    ps.new_block(|ps| {
        ps.with_start_of_line(true, |ps| {
            ps.with_formatting_context(FormattingContext::ClassOrModule, |ps| {
                ps.emit_newline();
                format_bodystmt(ps, bodystmt);
            });
        });
    });

    ps.with_start_of_line(true, |ps| {
        ps.emit_end();
    });
    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_conditional(
    ps: &mut ParserState,
    cond_expr: Expression,
    body: Vec<Expression>,
    kw: String,
    tail: Option<ElsifOrElse>,
) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }
    ps.emit_conditional_keyword(kw);
    ps.emit_space();
    ps.with_start_of_line(false, |ps| {
        format_expression(ps, cond_expr);
    });

    ps.with_start_of_line(true, |ps| {
        ps.new_block(|ps| {
            ps.emit_newline();
            for expr in body.into_iter() {
                format_expression(ps, expr);
            }
        });
    });
    ps.with_start_of_line(true, |ps| match tail {
        None => {}
        Some(ElsifOrElse::Elsif(elsif)) => {
            format_conditional(
                ps,
                *elsif.1,
                elsif.2,
                "elsif".to_string(),
                (elsif.3).map(|v| *v),
            );
        }
        Some(ElsifOrElse::Else(els)) => {
            ps.wind_line_forward();
            ps.emit_indent();
            ps.emit_else();
            ps.emit_newline();
            ps.with_start_of_line(true, |ps| {
                ps.new_block(|ps| {
                    for expr in els.1 {
                        format_expression(ps, expr);
                    }
                });
            });
        }
    });
}

pub fn format_if(ps: &mut ParserState, ifs: If) {
    format_conditional(ps, *ifs.1, ifs.2, "if".to_string(), ifs.3);
    ps.with_start_of_line(true, |ps| {
        ps.wind_line_forward();
        ps.emit_end();
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_unless(ps: &mut ParserState, unless: Unless) {
    format_conditional(
        ps,
        *unless.1,
        unless.2,
        "unless".to_string(),
        (unless.3).map(ElsifOrElse::Else),
    );
    ps.with_start_of_line(true, |ps| {
        ps.emit_end();
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_binary(ps: &mut ParserState, binary: Binary) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_formatting_context(FormattingContext::Binary, |ps| {
        ps.with_start_of_line(false, |ps| {
            format_expression(ps, *binary.1);
            ps.emit_space();
            ps.emit_ident(binary.2);
            ps.emit_space();
            format_expression(ps, *binary.3);
        });
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_float(ps: &mut ParserState, float: Float) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    handle_string_and_linecol(ps, float.1, float.2);

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_aref(ps: &mut ParserState, aref: Aref) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        format_expression(ps, *aref.1);
        ps.emit_open_square_bracket();
        match aref.2 {
            None => {}
            Some(arg_node) => {
                let args_list = normalize_args(arg_node);
                ps.with_formatting_context(FormattingContext::ArgsList, |ps| {
                    format_list_like_thing(ps, args_list, true);
                });
            }
        }
        ps.emit_close_square_bracket();
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_char(ps: &mut ParserState, c: Char) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.emit_double_quote();
    ps.on_line((c.2).0);
    ps.emit_ident(c.1[1..].to_string());
    ps.emit_double_quote();

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_hash(ps: &mut ParserState, hash: Hash) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }
    ps.on_line((hash.2).0);

    match hash.1 {
        None => ps.emit_ident("{}".to_string()),
        Some(assoc_list_from_args) => {
            ps.breakable_of(BreakableDelims::for_hash(), |ps| {
                format_assocs(ps, assoc_list_from_args.1, SpecialCase::NoSpecialCase);
            });
        }
    };

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_regexp_literal(ps: &mut ParserState, regexp: RegexpLiteral) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    let parts = regexp.1;
    let start_delimiter = (regexp.2).3;
    let end_delimiter = (regexp.2).1;

    ps.emit_ident(start_delimiter);
    format_inner_string(ps, parts, StringType::Regexp);
    handle_string_and_linecol(ps, end_delimiter, (regexp.2).2);

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_backref(ps: &mut ParserState, backref: Backref) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    handle_string_and_linecol(ps, backref.1, backref.2);

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

fn format_call_chain(ps: &mut ParserState, cc: Vec<CallChainElement>) {
    for cc_elem in cc.into_iter() {
        match cc_elem {
            CallChainElement::Paren(p) => format_paren(ps, p),
            CallChainElement::IdentOrOpOrKeywordOrConst(i) => format_ident(ps, i.into_ident()),
            CallChainElement::Block(b) => {
                ps.emit_space();
                format_block(ps, b);
            }
            CallChainElement::VarRef(vr) => format_var_ref(ps, vr),
            CallChainElement::ArgsAddStarOrExpressionList(aas) => {
                if !aas.is_empty() {
                    ps.breakable_of(BreakableDelims::for_method_call(), |ps| {
                        format_list_like_thing(ps, aas, false);
                    });
                }
            }
            CallChainElement::DotTypeOrOp(d) => format_dot(ps, d),
            CallChainElement::Expression(e) => format_expression(ps, *e),
        }
    }
}

pub fn format_block(ps: &mut ParserState, b: Block) {
    match b {
        Block::BraceBlock(bb) => format_brace_block(ps, bb),
        Block::DoBlock(db) => format_do_block(ps, db),
    }
}

pub fn format_method_add_block(ps: &mut ParserState, mab: MethodAddBlock) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    let chain = (mab.1).into_call_chain();

    ps.with_start_of_line(false, |ps| {
        format_call_chain(ps, chain);
    });

    // safe to unconditionally emit a space here, we don't have to worry
    // about not having a block, method_add_block can only be parsed if we
    // do in fact have a block
    ps.emit_space();

    match mab.2 {
        Block::DoBlock(do_block) => format_do_block(ps, do_block),
        Block::BraceBlock(brace_block) => format_brace_block(ps, brace_block),
    }

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_brace_block(ps: &mut ParserState, brace_block: BraceBlock) {
    let bv = brace_block.1;
    let body = brace_block.2;

    let new_body = body.clone();

    let is_multiline = ps.will_render_as_multiline(|next_ps| {
        next_ps.new_block(|next_ps| {
            next_ps.with_start_of_line(true, |next_ps| {
                next_ps.with_formatting_context(FormattingContext::CurlyBlock, |next_ps| {
                    for expr in new_body.into_iter() {
                        format_expression(next_ps, expr);
                    }
                });
            });
        });
    });

    ps.emit_ident("{".to_string());

    if let Some(bv) = bv {
        format_blockvar(ps, bv);
    }

    ps.new_block(|ps| {
        ps.with_start_of_line(is_multiline, |ps| {
            if is_multiline {
                ps.emit_newline();
            } else {
                ps.emit_space();
            }
            for expr in body.into_iter() {
                format_expression(ps, expr);
            }
        });
    });

    if is_multiline {
        ps.emit_indent();
    } else {
        ps.emit_space();
    }

    ps.emit_ident("}".to_string());
}

pub fn format_do_block(ps: &mut ParserState, do_block: DoBlock) {
    ps.emit_do_keyword();

    let bv = do_block.1;
    let body = do_block.2;

    if let Some(bv) = bv {
        format_blockvar(ps, bv)
    }

    ps.new_block(|ps| {
        ps.with_start_of_line(true, |ps| {
            ps.emit_newline();
            format_bodystmt(ps, body);
        });
    });

    ps.with_start_of_line(true, |ps| ps.emit_end());
}

pub fn format_kw_with_args(
    ps: &mut ParserState,
    args: ParenOrArgsAddBlock,
    kw: String,
    linecol: LineCol,
) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.emit_keyword(kw);
    let yield_args = match args {
        ParenOrArgsAddBlock::YieldParen(p) => {
            ps.emit_space();
            let arg = *p.1;
            match arg {
                ArgNode::ArgsAddBlock(aab) => aab,
                _ => panic!("should not have anything other than aab in yield"),
            }
        }
        ParenOrArgsAddBlock::ArgsAddBlock(aab) => {
            ps.emit_space();
            aab
        }
        ParenOrArgsAddBlock::Empty(v) => {
            if !v.is_empty() {
                panic!("got non empty empty in break/yield");
            };
            ArgsAddBlock(
                args_add_block_tag,
                ArgsAddBlockInner::ArgsAddStarOrExpressionList(
                    ArgsAddStarOrExpressionList::ExpressionList(vec![]),
                ),
                ToProcExpr::NotPresent(false),
            )
        }
    };
    ps.on_line(linecol.0);

    ps.with_start_of_line(false, |ps| {
        format_list_like_thing(
            ps,
            (yield_args.1).into_args_add_star_or_expression_list(),
            true,
        );
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_while(
    ps: &mut ParserState,
    conditional: Box<Expression>,
    exprs: Vec<Expression>,
    kw: String,
) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        ps.emit_keyword(kw);
        ps.emit_space();
        format_expression(ps, *conditional);
        ps.emit_newline();
        ps.new_block(|ps| {
            ps.with_start_of_line(true, |ps| {
                for expr in exprs {
                    format_expression(ps, expr);
                }
            });
        });
    });

    ps.emit_end();

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_mod_statement(
    ps: &mut ParserState,
    conditional: Box<Expression>,
    body: Box<Expression>,
    name: String,
) {
    let new_body = body.clone();

    let is_multiline = ps.will_render_as_multiline(|next_ps| {
        let exprs = match *new_body {
            Expression::Paren(p) => p.1,
            e => vec![e],
        };

        for expr in exprs {
            format_expression(next_ps, expr);
        }
    });

    if is_multiline {
        let exps = match *body {
            Expression::Paren(ParenExpr(_, exps)) => exps,
            x => vec![x],
        };
        format_conditional(ps, *conditional, exps, name, None);

        ps.with_start_of_line(true, |ps| {
            ps.emit_end();
        });

        if ps.at_start_of_line() {
            ps.emit_newline();
        }
    } else {
        if ps.at_start_of_line() {
            ps.emit_indent();
        }

        ps.with_start_of_line(false, |ps| {
            format_expression(ps, *body);

            ps.emit_mod_keyword(format!(" {} ", name));
            format_expression(ps, *conditional);
        });

        if ps.at_start_of_line() {
            ps.emit_newline();
        }
    }
}

pub fn format_when_or_else(ps: &mut ParserState, tail: WhenOrElse) {
    match tail {
        WhenOrElse::When(when) => {
            let conditionals = when.1;
            let body = when.2;
            let tail = when.3;
            let linecol = when.4;
            ps.on_line(linecol.0);
            ps.emit_indent();
            ps.emit_when_keyword();

            ps.with_start_of_line(false, |ps| {
                ps.breakable_of(BreakableDelims::for_when(), |ps| {
                    format_list_like_thing(ps, conditionals, false);
                });
            });

            ps.emit_newline();
            ps.new_block(|ps| {
                ps.with_start_of_line(true, |ps| {
                    for expr in body {
                        format_expression(ps, expr);
                    }
                });
            });

            if let Some(tail) = tail {
                format_when_or_else(ps, *tail);
            }
        }
        WhenOrElse::Else(e) => {
            ps.emit_indent();
            ps.emit_else();
            ps.emit_newline();

            ps.new_block(|ps| {
                ps.with_start_of_line(true, |ps| {
                    for expr in e.1 {
                        format_expression(ps, expr);
                    }
                });
            });
        }
    }
}

pub fn format_case(ps: &mut ParserState, case: Case) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }
    ps.on_line((case.3).0);

    ps.emit_case_keyword();

    let case_expr = case.1;
    let tail = case.2;

    if let Some(e) = case_expr {
        ps.with_start_of_line(false, |ps| {
            ps.emit_space();
            format_expression(ps, *e)
        });
    }

    ps.emit_newline();
    ps.with_start_of_line(true, |ps| {
        format_when_or_else(ps, WhenOrElse::When(tail));
        ps.emit_end();
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_retry(ps: &mut ParserState, _r: Retry) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.emit_keyword("retry".to_string());

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_sclass(ps: &mut ParserState, sc: SClass) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    let expr = sc.1;
    let body = sc.2;

    ps.with_start_of_line(false, |ps| {
        ps.emit_keyword("class".into());
        ps.emit_space();
        ps.emit_ident("<<".to_string());
        ps.emit_space();
        format_expression(ps, *expr);
        ps.emit_newline();
        ps.new_block(|ps| {
            ps.with_start_of_line(true, |ps| {
                format_bodystmt(ps, body);
            });
        });
    });
    ps.with_start_of_line(true, |ps| {
        ps.emit_end();
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_stabby_lambda(ps: &mut ParserState, sl: StabbyLambda) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    let params = sl.1;

    let tpe = sl.2;
    debug_assert!(tpe == "do" || tpe == "curly");

    let body = sl.3;
    let linecol = sl.4;
    ps.on_line(linecol.0);
    ps.with_start_of_line(false, |ps| {
        ps.emit_keyword("->".to_string());
        if params.is_present() {
            ps.emit_space();
        }
        format_paren_or_params(ps, params);

        let (open_delim, close_delim) = if tpe == "do" {
            ("do".to_string(), "end".to_string())
        } else {
            ("{".to_string(), "}".to_string())
        };

        match body {
            ExpressionListOrBodyStmt::ExpresionList(bud) => {
                let mut b = bud;
                //lambdas typically are a single statement, so line breaking them would
                //be masochistic
                if tpe == "curly" && b.len() == 1 {
                    ps.emit_ident(" { ".to_string());
                    format_expression(ps, b.remove(0));
                    ps.emit_ident(" }".to_string());
                } else {
                    ps.emit_space();
                    ps.emit_ident(open_delim);
                    ps.emit_newline();
                    ps.new_block(|ps| {
                        ps.with_start_of_line(true, |ps| {
                            for expr in b.into_iter() {
                                format_expression(ps, expr);
                            }
                        });
                    });
                    ps.emit_ident(close_delim);
                }
            }
            ExpressionListOrBodyStmt::BodyStmt(bs) => {
                ps.emit_space();
                ps.emit_ident(open_delim);
                ps.emit_newline();
                ps.new_block(|ps| {
                    ps.with_start_of_line(true, |ps| {
                        format_bodystmt(ps, bs);
                    });
                });
                ps.emit_ident(close_delim);
            }
        }
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_imaginary(ps: &mut ParserState, imaginary: Imaginary) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    handle_string_and_linecol(ps, imaginary.1, imaginary.2);

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_rational(ps: &mut ParserState, rational: Rational) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    handle_string_and_linecol(ps, rational.1, rational.2);

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_for(ps: &mut ParserState, forloop: For) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    let variables = forloop.1;
    let collection = forloop.2;
    let body = forloop.3;

    ps.with_start_of_line(false, |ps| {
        ps.emit_keyword("for".to_string());
        ps.emit_space();
        match variables {
            VarFieldOrVarFields::VarField(vf) => {
                format_var_field(ps, vf);
            }
            VarFieldOrVarFields::VarFields(vfs) => {
                let len = vfs.len();
                for (idx, expr) in vfs.into_iter().enumerate() {
                    format_var_field(ps, expr);
                    if idx != len - 1 {
                        ps.emit_comma_space();
                    }
                }
            }
        }

        ps.emit_space();
        ps.emit_keyword("in".to_string());
        ps.emit_space();
        format_expression(ps, *collection);
        ps.emit_newline();
        ps.new_block(|ps| {
            ps.with_start_of_line(true, |ps| {
                for expr in body.into_iter() {
                    format_expression(ps, expr);
                }
            });
        });
    });
    ps.with_start_of_line(true, |ps| {
        ps.emit_end();
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_ifop(ps: &mut ParserState, ifop: IfOp) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        format_expression(ps, *ifop.1);
        ps.emit_space();
        ps.emit_keyword("?".to_string());
        ps.emit_space();
        format_expression(ps, *ifop.2);
        ps.emit_space();
        ps.emit_keyword(":".to_string());
        ps.emit_space();
        format_expression(ps, *ifop.3);
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_return0(ps: &mut ParserState, _r0: Return0) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.emit_keyword("return".to_string());

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_opassign(ps: &mut ParserState, opassign: OpAssign) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.with_start_of_line(false, |ps| {
        format_assignable(ps, opassign.1);
        ps.emit_space();
        format_op(ps, opassign.2);
        ps.emit_space();
        format_expression(ps, *opassign.3);
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}
pub fn format_to_proc(ps: &mut ParserState, e: Box<Expression>) {
    ps.emit_ident("&".to_string());
    ps.with_start_of_line(false, |ps| format_expression(ps, *e));
}

pub fn format_zsuper(ps: &mut ParserState) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.emit_keyword("super".to_string());

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_yield0(ps: &mut ParserState) {
    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    ps.emit_keyword("yield".to_string());

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_yield(ps: &mut ParserState, y: Yield) {
    format_method_call(ps, y.to_method_call())
}

pub fn format_return(ps: &mut ParserState, ret: Return) {
    let args = ret.1;
    let line = (ret.2).0;
    ps.on_line(line);

    if ps.at_start_of_line() {
        ps.emit_indent();
    }

    let args = normalize_args(args);
    ps.emit_keyword("return".to_string());

    ps.with_start_of_line(false, |ps| {
        if !args.is_empty() {
            match args {
                ArgsAddStarOrExpressionList::ArgsAddStar(aas) => {
                    format_bare_return_args(ps, ArgsAddStarOrExpressionList::ArgsAddStar(aas));
                }
                ArgsAddStarOrExpressionList::ExpressionList(mut el) => {
                    if el.len() == 1 {
                        let element = el.remove(0);
                        match element {
                            Expression::Array(Array(array_tag, contents, linecol)) => {
                                ps.emit_space();
                                format_array(ps, Array(array_tag, contents, linecol));
                            }
                            elem => {
                                ps.emit_space();
                                format_expression(ps, elem);
                            }
                        }
                    } else {
                        format_bare_return_args(
                            ps,
                            ArgsAddStarOrExpressionList::ExpressionList(el),
                        );
                    }
                }
            }
        }
    });

    if ps.at_start_of_line() {
        ps.emit_newline();
    }
}

pub fn format_bare_return_args(ps: &mut ParserState, args: ArgsAddStarOrExpressionList) {
    ps.breakable_of(BreakableDelims::for_return_kw(), |ps| {
        ps.with_formatting_context(FormattingContext::ArgsList, |ps| {
            format_list_like_thing(ps, args, false);
            ps.emit_collapsing_newline();
        });
    });
}

pub fn format_expression(ps: &mut ParserState, expression: Expression) {
    let expression = normalize(expression);
    match expression {
        Expression::Def(def) => format_def(ps, def),
        Expression::MethodCall(mc) => format_method_call(ps, mc),
        Expression::Ident(ident) => format_ident(ps, ident),
        Expression::Int(int) => format_int(ps, int),
        Expression::BareAssocHash(bah) => format_bare_assoc_hash(ps, bah),
        Expression::Begin(begin) => format_begin(ps, begin),
        Expression::VoidStmt(void) => format_void_stmt(ps, void),
        Expression::Paren(paren) => format_paren(ps, paren),
        Expression::Dot2(dot2) => format_dot2(ps, dot2),
        Expression::Dot3(dot3) => format_dot3(ps, dot3),
        Expression::SymbolLiteral(sl) => format_symbol_literal(ps, sl),
        Expression::Alias(alias) => format_alias(ps, alias),
        Expression::Array(array) => format_array(ps, array),
        Expression::StringLiteral(sl) => format_string_literal(ps, sl),
        Expression::XStringLiteral(xsl) => format_xstring_literal(ps, xsl),
        Expression::Assign(assign) => format_assign(ps, assign),
        Expression::VarRef(vr) => format_var_ref(ps, vr),
        Expression::ConstPathRef(cpr) => format_const_path_ref(ps, cpr),
        Expression::TopConstRef(tcr) => format_top_const_ref(ps, tcr),
        Expression::Defined(defined) => format_defined(ps, defined),
        Expression::RescueMod(rescue_mod) => format_rescue_mod(ps, rescue_mod),
        Expression::MRHSAddStar(mrhs) => format_mrhs_add_star(ps, mrhs),
        Expression::MAssign(massign) => format_massign(ps, massign),
        Expression::Next(next) => format_next(ps, next),
        Expression::Unary(unary) => format_unary(ps, unary),
        Expression::StringConcat(sc) => format_string_concat(ps, sc),
        Expression::DynaSymbol(ds) => format_dyna_symbol(ps, ds),
        Expression::Undef(undef) => format_undef(ps, undef),
        Expression::Class(class) => format_class(ps, class),
        Expression::Defs(defs) => format_defs(ps, defs),
        Expression::If(ifs) => format_if(ps, ifs),
        Expression::Binary(binary) => format_binary(ps, binary),
        Expression::Float(float) => format_float(ps, float),
        Expression::Aref(aref) => format_aref(ps, aref),
        Expression::Char(c) => format_char(ps, c),
        Expression::Module(m) => format_module(ps, m),
        Expression::Hash(h) => format_hash(ps, h),
        Expression::RegexpLiteral(regexp) => format_regexp_literal(ps, regexp),
        Expression::Backref(backref) => format_backref(ps, backref),
        Expression::Yield(y) => format_yield(ps, y),
        Expression::Break(b) => format_kw_with_args(ps, b.1, "break".to_string(), b.2),
        Expression::MethodAddBlock(mab) => format_method_add_block(ps, mab),
        Expression::While(w) => format_while(ps, w.1, w.2, "while".to_string()),
        Expression::Until(u) => format_while(ps, u.1, u.2, "until".to_string()),
        Expression::WhileMod(wm) => format_mod_statement(ps, wm.1, wm.2, "while".to_string()),
        Expression::UntilMod(um) => format_mod_statement(ps, um.1, um.2, "until".to_string()),
        Expression::IfMod(wm) => format_mod_statement(ps, wm.1, wm.2, "if".to_string()),
        Expression::UnlessMod(um) => format_mod_statement(ps, um.1, um.2, "unless".to_string()),
        Expression::Case(c) => format_case(ps, c),
        Expression::Retry(r) => format_retry(ps, r),
        Expression::SClass(sc) => format_sclass(ps, sc),
        Expression::StabbyLambda(sl) => format_stabby_lambda(ps, sl),
        Expression::Rational(rational) => format_rational(ps, rational),
        Expression::Imaginary(imaginary) => format_imaginary(ps, imaginary),
        Expression::MLhs(mlhs) => format_mlhs(ps, mlhs),
        Expression::For(forloop) => format_for(ps, forloop),
        Expression::IfOp(ifop) => format_ifop(ps, ifop),
        Expression::Return0(r) => format_return0(ps, r),
        Expression::OpAssign(op) => format_opassign(ps, op),
        Expression::Unless(u) => format_unless(ps, u),
        Expression::ToProc(ToProc(_, e)) => format_to_proc(ps, e),
        Expression::ZSuper(..) => format_zsuper(ps),
        Expression::Yield0(..) => format_yield0(ps),
        Expression::Return(ret) => format_return(ps, ret),
        Expression::BeginBlock(begin) => format_begin_block(ps, begin),
        Expression::EndBlock(end) => format_end_block(ps, end),
        e => {
            panic!("got unknown token: {:?}", e);
        }
    }
}

pub fn format_program(ps: &mut ParserState, program: Program) {
    ps.flush_start_of_file_comments();
    debug!("{:?}", program);
    for expression in program.1 {
        format_expression(ps, expression);
    }
}
