use crate::delimiters::BreakableDelims;
use crate::line_tokens::LineToken;
use crate::types::{ColNumber, LineNumber};
use std::collections::HashSet;

fn insert_at<T>(idx: usize, target: &mut Vec<T>, input: &mut Vec<T>) {
    let mut tail = target.split_off(idx);
    target.append(input);
    target.append(&mut tail);
}

#[derive(Copy, Clone, Debug)]
pub enum ConvertType {
    MultiLine,
    SingleLine,
}

pub trait LineTokenTarget {
    fn push(&mut self, lt: LineToken);
    fn insert_at(&mut self, idx: usize, tokens: &mut Vec<LineToken>);
    fn into_tokens(self, ct: ConvertType) -> Vec<LineToken>;
    fn last_token_is_a_newline(&self) -> bool;
    fn index_of_prev_hard_newline(&self) -> Option<usize>;
}

#[derive(Debug, Default, Clone)]
pub struct BaseQueue {
    tokens: Vec<LineToken>,
}

impl LineTokenTarget for BaseQueue {
    fn push(&mut self, lt: LineToken) {
        self.tokens.push(lt)
    }

    fn insert_at(&mut self, idx: usize, tokens: &mut Vec<LineToken>) {
        insert_at(idx, &mut self.tokens, tokens)
    }

    fn into_tokens(self, _ct: ConvertType) -> Vec<LineToken> {
        self.tokens
    }

    fn last_token_is_a_newline(&self) -> bool {
        self.tokens.last().map(|x| x.is_newline()).unwrap_or(false)
    }

    fn index_of_prev_hard_newline(&self) -> Option<usize> {
        self.tokens
            .iter()
            .rposition(|v| v.is_newline() || v.is_comment())
    }
}

#[derive(Debug, Clone)]
pub struct BreakableEntry {
    spaces: ColNumber,
    tokens: Vec<LineToken>,
    line_numbers: HashSet<LineNumber>,
    delims: BreakableDelims,
}

impl LineTokenTarget for BreakableEntry {
    fn push(&mut self, lt: LineToken) {
        self.tokens.push(lt);
    }

    fn insert_at(&mut self, idx: usize, tokens: &mut Vec<LineToken>) {
        insert_at(idx, &mut self.tokens, tokens)
    }

    fn into_tokens(self, ct: ConvertType) -> Vec<LineToken> {
        let mut tokens = self.tokens;
        match ct {
            ConvertType::MultiLine => {
                tokens = tokens.into_iter().map(|t| t.into_multi_line()).collect();
                tokens.insert(0, self.delims.multi_line_open());
                tokens.push(self.delims.multi_line_close());
            }
            ConvertType::SingleLine => {
                tokens = tokens.into_iter().map(|t| t.into_single_line()).collect();
                tokens.insert(0, self.delims.single_line_open());
                tokens.push(self.delims.single_line_close());
            }
        }
        tokens
    }

    fn last_token_is_a_newline(&self) -> bool {
        match self.tokens.last() {
            Some(x) => x.is_newline(),
            _ => false,
        }
    }

    fn index_of_prev_hard_newline(&self) -> Option<usize> {
        self.tokens
            .iter()
            .rposition(|v| v.is_newline() || v.is_comment())
    }
}

impl BreakableEntry {
    pub fn new(spaces: ColNumber, delims: BreakableDelims) -> Self {
        BreakableEntry {
            spaces,
            tokens: Vec::new(),
            line_numbers: HashSet::new(),
            delims,
        }
    }

    pub fn single_line_string_length(&self) -> usize {
        self.tokens
            .iter()
            .map(|tok| tok.clone().into_single_line())
            .map(|tok| tok.into_ruby().len())
            .sum()
    }

    pub fn push_line_number(&mut self, number: LineNumber) {
        self.line_numbers.insert(number);
    }

    pub fn is_multiline(&self) -> bool {
        self.line_numbers.len() > 1
    }
}
