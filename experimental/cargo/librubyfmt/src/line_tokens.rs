use crate::render_targets::{BreakableEntry, ConvertType, LineTokenTarget};

#[derive(Debug, Clone)]
pub enum LineToken {
    // this is all bodil's fault
    CollapsingNewLine,
    HardNewLine,
    SoftNewline,
    Indent { depth: u32 },
    SoftIndent { depth: u32 },
    Keyword { keyword: String },
    DefKeyword,
    ClassKeyword,
    ModuleKeyword,
    DoKeyword,
    ModKeyword { contents: String },
    ConditionalKeyword { contents: String },
    DirectPart { part: String },
    CommaSpace,
    Comma,
    Space,
    Dot,
    ColonColon,
    LonelyOperator,
    OpenSquareBracket,
    CloseSquareBracket,
    OpenCurlyBracket,
    CloseCurlyBracket,
    OpenParen,
    CloseParen,
    BreakableEntry(BreakableEntry),
    Op { op: String },
    DoubleQuote,
    LTStringContent { content: String },
    SingleSlash,
    Comment { contents: String },
    Delim { contents: String },
    End,
}

impl LineToken {
    pub fn into_single_line(self) -> LineToken {
        match self {
            Self::CollapsingNewLine => LineToken::DirectPart {
                part: "".to_string(),
            },
            Self::SoftNewline => LineToken::Space,
            Self::SoftIndent { .. } => LineToken::DirectPart {
                part: "".to_string(),
            },
            x => x,
        }
    }

    pub fn into_multi_line(self) -> LineToken {
        self
    }

    pub fn is_indent(&self) -> bool {
        match self {
            Self::Indent { .. } => true,
            _ => false,
        }
    }

    pub fn is_comment(&self) -> bool {
        match self {
            Self::Comment { .. } => true,
            _ => false,
        }
    }

    pub fn is_newline(&self) -> bool {
        match self {
            Self::HardNewLine => true,
            Self::SoftNewline => true,
            Self::CollapsingNewLine => true,
            Self::DirectPart { part } => {
                if part == "\n" {
                    panic!("shouldn't ever have a single newline direct part");
                } else {
                    false
                }
            }
            _ => false,
        }
    }

    pub fn into_ruby(self) -> String {
        match self {
            Self::CollapsingNewLine => "\n".to_string(),
            Self::HardNewLine => "\n".to_string(),
            Self::SoftNewline => "\n".to_string(),
            Self::Indent { depth } => (0..depth).map(|_| ' ').collect(),
            Self::SoftIndent { depth } => (0..depth).map(|_| ' ').collect(),
            Self::Keyword { keyword } => keyword,
            Self::ModKeyword { contents } => contents,
            Self::ConditionalKeyword { contents } => contents,
            Self::DoKeyword => "do".to_string(),
            Self::ClassKeyword => "class".to_string(),
            Self::DefKeyword => "def".to_string(),
            Self::ModuleKeyword => "module".to_string(),
            Self::DirectPart { part } => part,
            Self::CommaSpace => ", ".to_string(),
            Self::Comma => ",".to_string(),
            Self::Space => " ".to_string(),
            Self::Dot => ".".to_string(),
            Self::ColonColon => "::".to_string(),
            Self::LonelyOperator => "&.".to_string(),
            Self::OpenSquareBracket => "[".to_string(),
            Self::CloseSquareBracket => "]".to_string(),
            Self::OpenCurlyBracket => "{".to_string(),
            Self::CloseCurlyBracket => "}".to_string(),
            Self::OpenParen => "(".to_string(),
            Self::CloseParen => ")".to_string(),
            Self::BreakableEntry(be) => be
                .into_tokens(ConvertType::SingleLine)
                .into_iter()
                .fold("".to_string(), |accum, tok| {
                    format!("{}{}", accum, tok.into_ruby())
                }),
            Self::Op { op } => op,
            Self::DoubleQuote => "\"".to_string(),
            Self::LTStringContent { content } => content,
            Self::SingleSlash => "\\".to_string(),
            Self::Comment { contents } => format!("{}\n", contents),
            Self::Delim { contents } => contents,
            Self::End => "end".to_string(),
        }
    }

    pub fn is_in_need_of_a_trailing_blankline(&self) -> bool {
        self.is_conditional_spaced_token() && !self.is_block_closing_token()
    }

    pub fn is_block_closing_token(&self) -> bool {
        match self {
            Self::End => true,
            Self::DirectPart { part } => part == "}" || part == "]",
            _ => false,
        }
    }

    pub fn is_conditional_spaced_token(&self) -> bool {
        match self {
            Self::ConditionalKeyword { contents } => !(contents == "else" || contents == "elsif"),
            _ => true,
        }
    }

    pub fn is_single_line_breakable_garbage(&self) -> bool {
        match self {
            Self::Comma => true,
            Self::Space => true,
            Self::SoftNewline => true,
            Self::DirectPart { part } => (part == &"".to_string()),
            _ => false,
        }
    }
}
