#[derive(Debug)]
pub struct LineMetadata {
    gets_indented: bool,
    conditional: bool,
    end: bool,
    def: bool,
    do_keyword: bool,
    indent_level: Option<u32>,
    require: bool,
}

impl LineMetadata {
    pub fn indent_level_increases_between(prev: &LineMetadata, current: &LineMetadata) -> bool {
        prev.indent_level < current.indent_level
    }

    pub fn new() -> Self {
        LineMetadata {
            gets_indented: false,
            conditional: false,
            end: false,
            def: false,
            do_keyword: false,
            indent_level: None,
            require: false,
        }
    }

    pub fn set_has_require(&mut self) {
        self.require = true;
    }

    pub fn has_require(&self) -> bool {
        self.require
    }

    pub fn observe_indent_level(&mut self, level: u32) {
        if self.indent_level.is_some() {
            panic!("indent_level should be impossible");
        }

        self.indent_level = Some(level);
    }

    pub fn gets_indented(&self) -> bool {
        self.gets_indented
    }

    pub fn set_gets_indented(&mut self) {
        self.gets_indented = true;
    }

    pub fn set_has_conditional(&mut self) {
        self.conditional = true;
    }

    pub fn set_has_end(&mut self) {
        self.end = true;
    }

    pub fn set_has_def(&mut self) {
        self.def = true;
    }

    pub fn set_has_do_keyword(&mut self) {
        self.do_keyword = true;
    }

    pub fn wants_spacer_for_conditional(&self) -> bool {
        !(self.conditional || self.gets_indented || self.end || self.def || self.do_keyword)
    }
}
