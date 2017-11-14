pub type Id = usize;

const NULL_ID: Id = 0;

pub struct IdArena<T> {
    vec: Vec<T>,
}

impl<T> IdArena<T> {
    pub fn new() -> Self {
        IdArena { vec: Vec::new() }
    }

    pub fn insert(&mut self, value: Option<T>) -> Id {
        match value {
            None => NULL_ID,
            Some(x) => {
                let id = self.vec.len() + 1;
                self.vec.push(x);
                id
            }
        }
    }

    /// Panics if id is out of bounds
    pub fn get(&self, id: Id) -> Option<&T> {
        if id == NULL_ID {
            None
        } else {
            Some(&self.vec[id - 1])
        }
    }
}
