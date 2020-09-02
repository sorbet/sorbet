use crate::line_tokens::LineToken;

#[derive(Debug, Clone)]
struct DelimiterPair {
    open: String,
    close: String,
}

impl DelimiterPair {
    fn new(open: String, close: String) -> Self {
        DelimiterPair { open, close }
    }
}

#[derive(Debug, Clone)]
pub struct BreakableDelims {
    single_line: DelimiterPair,
    multi_line: DelimiterPair,
}

impl BreakableDelims {
    pub fn for_method_call() -> Self {
        BreakableDelims {
            single_line: DelimiterPair::new("(".to_string(), ")".to_string()),
            multi_line: DelimiterPair::new("(".to_string(), ")".to_string()),
        }
    }

    pub fn for_return_kw() -> Self {
        BreakableDelims {
            single_line: DelimiterPair::new(" ".to_string(), "".to_string()),
            multi_line: DelimiterPair::new(" [".to_string(), "]".to_string()),
        }
    }

    pub fn for_kw() -> Self {
        BreakableDelims {
            single_line: DelimiterPair::new(" ".to_string(), "".to_string()),
            multi_line: DelimiterPair::new("(".to_string(), ")".to_string()),
        }
    }

    pub fn for_block_params() -> Self {
        BreakableDelims {
            single_line: DelimiterPair::new(" |".to_string(), "|".to_string()),
            multi_line: DelimiterPair::new(" |".to_string(), "|".to_string()),
        }
    }

    pub fn for_array() -> Self {
        BreakableDelims {
            single_line: DelimiterPair::new("[".to_string(), "]".to_string()),
            multi_line: DelimiterPair::new("[".to_string(), "]".to_string()),
        }
    }

    pub fn for_when() -> Self {
        BreakableDelims {
            single_line: DelimiterPair::new(" ".to_string(), "".to_string()),
            multi_line: DelimiterPair::new("".to_string(), "".to_string()),
        }
    }

    pub fn for_hash() -> Self {
        BreakableDelims {
            single_line: DelimiterPair::new("{".to_string(), "}".to_string()),
            multi_line: DelimiterPair::new("{".to_string(), "}".to_string()),
        }
    }

    pub fn single_line_open(&self) -> LineToken {
        LineToken::Delim {
            contents: self.single_line.open.clone(),
        }
    }

    pub fn single_line_close(&self) -> LineToken {
        LineToken::Delim {
            contents: self.single_line.close.clone(),
        }
    }

    pub fn multi_line_open(&self) -> LineToken {
        LineToken::Delim {
            contents: self.multi_line.open.clone(),
        }
    }

    pub fn multi_line_close(&self) -> LineToken {
        LineToken::Delim {
            contents: self.multi_line.close.clone(),
        }
    }
}
