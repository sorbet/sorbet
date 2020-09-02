use crate::intermediary::{BlanklineReason, Intermediary};
use crate::line_tokens::*;
use crate::render_targets::{BreakableEntry, ConvertType, LineTokenTarget};
#[cfg(debug_assertions)]
use log::debug;
use std::env;
use std::io::{self, Write};

const MAX_LINE_LENGTH: usize = 120;

pub struct RenderQueueWriter {
    tokens: Vec<LineToken>,
}

impl RenderQueueWriter {
    pub fn new(tokens: Vec<LineToken>) -> Self {
        RenderQueueWriter { tokens }
    }

    pub fn write<W: Write>(self, writer: &mut W) -> io::Result<()> {
        let mut accum = Intermediary::new();
        let key = "RUBYFMT_DISABLE_SZUSZ";
        let run = match env::var(key) {
            Err(_) => true,
            Ok(x) => x != "1",
        };
        #[cfg(debug_assertions)]
        {
            debug!("first tokens {:?}", self.tokens);
        }
        if run {
            Self::render_as(
                &mut accum,
                self.tokens
                    .into_iter()
                    .map(|t| t.into_multi_line())
                    .collect(),
            );
            Self::write_final_tokens(writer, accum.into_tokens())
        } else {
            Self::write_final_tokens(writer, self.tokens)
        }
    }

    fn render_as(accum: &mut Intermediary, tokens: Vec<LineToken>) {
        for next_token in tokens.into_iter() {
            match next_token {
                LineToken::BreakableEntry(be) => Self::format_breakable_entry(accum, be),
                x => accum.push(x),
            }

            if accum.len() >= 4 {
                if let (&LineToken::End, &LineToken::HardNewLine, &LineToken::Indent { .. }, x) =
                    accum.last_4().expect("we checked length")
                {
                    if x.is_in_need_of_a_trailing_blankline() {
                        accum.insert_trailing_blankline(BlanklineReason::ComesAfterEnd);
                    }
                }
            }
        }
    }

    fn format_breakable_entry(accum: &mut Intermediary, be: BreakableEntry) {
        let length = be.single_line_string_length();

        if length > MAX_LINE_LENGTH || be.is_multiline() {
            Self::render_as(accum, be.into_tokens(ConvertType::MultiLine));
        } else {
            Self::render_as(accum, be.into_tokens(ConvertType::SingleLine));
            // after running accum looks like this (or some variant):
            // [.., Comma, Space, DirectPart {part: ""}, <close_delimiter>]
            // so we remove items at positions length-2 until there is nothing
            // in that position that is garbage.
            accum.clear_breakable_garbage();
        }
    }

    fn write_final_tokens<W: Write>(writer: &mut W, tokens: Vec<LineToken>) -> io::Result<()> {
        #[cfg(debug_assertions)]
        {
            debug!("final tokens: {:?}", tokens);
        }

        for line_token in tokens.into_iter() {
            let s = line_token.into_ruby();
            write!(writer, "{}", s)?
        }
        Ok(())
    }
}
