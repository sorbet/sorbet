use std::ops::Range;

use crate::line_tokens::LineToken;
use crate::types::{ColNumber, LineNumber};

#[derive(Debug)]
pub struct CommentBlock {
    span: Range<LineNumber>,
    comments: Vec<String>,
}

impl CommentBlock {
    pub fn new(span: Range<LineNumber>, comments: Vec<String>) -> Self {
        CommentBlock { span, comments }
    }

    pub fn following_line_number(&self) -> LineNumber {
        self.span.end
    }

    pub fn add_line(&mut self, line: String) {
        self.span.end += 1;
        self.comments.push(line);
    }

    pub fn into_line_tokens(self) -> Vec<LineToken> {
        self.comments
            .into_iter()
            .map(|c| LineToken::Comment { contents: c })
            .collect()
    }

    pub fn apply_spaces(mut self, indent_depth: ColNumber) -> Self {
        for comment in &mut self.comments {
            *comment = str::repeat(" ", indent_depth as _) + comment;
        }
        self
    }

    pub fn has_comments(&self) -> bool {
        !self.comments.is_empty()
    }

    pub fn len(&self) -> usize {
        self.comments.len()
    }
}

pub trait Merge<Other = Self> {
    fn merge(&mut self, other: Other);
}

impl Merge for CommentBlock {
    fn merge(&mut self, mut other: CommentBlock) {
        self.comments.append(&mut other.comments);
    }
}

impl Merge<CommentBlock> for Option<CommentBlock> {
    fn merge(&mut self, other: CommentBlock) {
        if let Some(this) = self {
            this.merge(other)
        } else {
            *self = Some(other)
        }
    }
}
