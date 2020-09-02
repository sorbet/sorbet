use crate::line_metadata::LineMetadata;
use crate::line_tokens::*;
#[cfg(debug_assertions)]
use log::debug;
use std::mem;

#[derive(Debug)]
pub enum BlanklineReason {
    ComesAfterEnd,
    Conditional,
    ClassOrModule,
    DoKeyword,
    EndOfRequireBlock,
}

pub struct Intermediary {
    tokens: Vec<LineToken>,
    index_of_last_hard_newline: usize,
    current_line_metadata: LineMetadata,
    previous_line_metadata: Option<LineMetadata>,
}

impl Intermediary {
    pub fn new() -> Self {
        Intermediary {
            tokens: vec![],
            current_line_metadata: LineMetadata::new(),
            previous_line_metadata: None,
            index_of_last_hard_newline: 0,
        }
    }

    pub fn len(&self) -> usize {
        self.tokens.len()
    }

    pub fn last_4(&self) -> Option<(&LineToken, &LineToken, &LineToken, &LineToken)> {
        if self.len() < 4 {
            return None;
        }

        Some((
            &self.tokens[self.len() - 4],
            &self.tokens[self.len() - 3],
            &self.tokens[self.len() - 2],
            &self.tokens[self.len() - 1],
        ))
    }

    pub fn into_tokens(self) -> Vec<LineToken> {
        self.tokens
    }

    pub fn push(&mut self, lt: LineToken) {
        self.debug_assert_newlines();
        let mut do_push = true;

        match &lt {
            LineToken::HardNewLine => {
                if let Some(prev) = &self.previous_line_metadata {
                    if !self.current_line_metadata.has_require() && prev.has_require() {
                        self.insert_trailing_blankline(BlanklineReason::EndOfRequireBlock);
                    }
                }

                let mut md = LineMetadata::new();
                mem::swap(&mut md, &mut self.current_line_metadata);
                self.previous_line_metadata = Some(md);
                self.index_of_last_hard_newline = self.tokens.len();

                if self.tokens.len() >= 2 {
                    if let (Some(&LineToken::HardNewLine), Some(&LineToken::HardNewLine)) = (
                        self.tokens.get(self.index_of_last_hard_newline - 2),
                        self.tokens.get(self.index_of_last_hard_newline - 1),
                    ) {
                        do_push = false;
                        self.index_of_last_hard_newline = self.tokens.len() - 1;
                    }
                }
            }
            LineToken::ModuleKeyword | LineToken::ClassKeyword => {
                self.handle_class_or_module();
            }
            LineToken::DoKeyword => {
                self.handle_do_keyword();
            }
            LineToken::ConditionalKeyword { contents } => self.handle_conditional(contents),
            LineToken::End => self.handle_end(),
            LineToken::DefKeyword => self.handle_def(),
            LineToken::Indent { depth } => {
                self.current_line_metadata.observe_indent_level(*depth);

                if let Some(prev) = &mut self.previous_line_metadata {
                    if LineMetadata::indent_level_increases_between(
                        &prev,
                        &self.current_line_metadata,
                    ) {
                        prev.set_gets_indented()
                    }
                }
            }
            LineToken::DirectPart { part } => {
                if part == "require" && self.tokens.last().map(|t| t.is_indent()).unwrap_or(false) {
                    self.current_line_metadata.set_has_require();
                }
            }
            _ => {}
        }

        if do_push {
            self.tokens.push(lt);
        }
        self.debug_assert_newlines();
    }

    fn handle_end(&mut self) {
        self.current_line_metadata.set_has_end();
    }

    fn handle_def(&mut self) {
        self.current_line_metadata.set_has_def();
    }

    fn handle_do_keyword(&mut self) {
        self.current_line_metadata.set_has_do_keyword();
        if let Some(prev) = &self.previous_line_metadata {
            if prev.wants_spacer_for_conditional() {
                self.insert_trailing_blankline(BlanklineReason::DoKeyword);
            }
        }
    }

    fn handle_class_or_module(&mut self) {
        if let Some(prev) = &self.previous_line_metadata {
            if !prev.gets_indented() {
                self.insert_trailing_blankline(BlanklineReason::ClassOrModule);
            }
        }
    }

    fn handle_conditional(&mut self, cond: &str) {
        self.current_line_metadata.set_has_conditional();
        if let Some(prev) = &self.previous_line_metadata {
            if prev.wants_spacer_for_conditional() && cond == "if" {
                self.insert_trailing_blankline(BlanklineReason::Conditional);
            }
        }
    }

    pub fn clear_breakable_garbage(&mut self) {
        // after running self.tokens looks like this (or some variant):
        // [.., Comma, Space, DirectPart {part: ""}, <close_delimiter>]
        // so we remove items at positions length-2 until there is nothing
        // in that position that is garbage.
        while self.tokens[self.len() - 2].is_single_line_breakable_garbage() {
            self.tokens.remove(self.len() - 2);
        }
    }

    pub fn insert_trailing_blankline(&mut self, _bl: BlanklineReason) {
        match (
            self.tokens.get(self.index_of_last_hard_newline - 1),
            self.tokens.get(self.index_of_last_hard_newline),
        ) {
            (Some(&LineToken::HardNewLine), Some(&LineToken::HardNewLine)) => {}
            (_, _) => {
                #[cfg(debug_assertions)]
                {
                    debug!("{:?}", _bl);
                }
                self.tokens
                    .insert(self.index_of_last_hard_newline, LineToken::HardNewLine);
                self.index_of_last_hard_newline += 1;
                self.debug_assert_newlines();
            }
        }
    }

    #[cfg(debug_assertions)]
    fn debug_assert_newlines(&self) {
        if self.index_of_last_hard_newline == 0 {
            return;
        }
        match self.tokens.get(self.index_of_last_hard_newline) {
            Some(&LineToken::HardNewLine) => {}
            _ => panic!("newlines are fucked"),
        }
    }

    #[cfg(not(debug_assertions))]
    fn debug_assert_newlines(&self) {}
}
