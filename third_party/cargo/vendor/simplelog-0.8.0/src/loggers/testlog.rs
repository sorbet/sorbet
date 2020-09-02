// Copyright 2016 Victor Brekenfeld
//
// Licensed under the Apache License, Version 2.0, <LICENSE-APACHE or
// http://apache.org/licenses/LICENSE-2.0> or the MIT license <LICENSE-MIT or
// http://opensource.org/licenses/MIT>, at your option. This file may not be
// copied, modified, or distributed except according to those terms.

//! Module providing the TestLogger Implementation

use super::logging::should_skip;
use crate::{Config, SharedLogger, LevelPadding};
use log::{set_boxed_logger, set_max_level, LevelFilter, Log, Metadata, Record, SetLoggerError};

use std::thread;

/// The TestLogger struct. Provides a very basic Logger implementation that may be captured by cargo.
pub struct TestLogger {
    level: LevelFilter,
    config: Config,
}

impl TestLogger {
    /// init function. Globally initializes the TestLogger as the one and only used log facility.
    ///
    /// Takes the desired `Level` and `Config` as arguments. They cannot be changed later on.
    /// Fails if another Logger was already initialized.
    ///
    /// # Examples
    /// ```
    /// # extern crate simplelog;
    /// # use simplelog::*;
    /// # fn main() {
    /// #[cfg(not(test))]
    /// // another logger
    /// # let _ = TestLogger::init(LevelFilter::Info, Config::default());
    /// #[cfg(test)]
    /// let _ = TestLogger::init(LevelFilter::Info, Config::default());
    /// # }
    /// ```
    pub fn init(log_level: LevelFilter, config: Config) -> Result<(), SetLoggerError> {
        set_max_level(log_level.clone());
        set_boxed_logger(TestLogger::new(log_level, config))
    }

    /// allows to create a new logger, that can be independently used, no matter what is globally set.
    ///
    /// no macros are provided for this case and you probably
    /// dont want to use this function, but `init()`, if you dont want to build a `CombinedLogger`.
    ///
    /// Takes the desired `Level` and `Config` as arguments. They cannot be changed later on.
    ///
    /// # Examples
    /// ```
    /// # extern crate simplelog;
    /// # use simplelog::*;
    /// # fn main() {
    /// #[cfg(not(test))]
    /// // another logger
    /// # let test_logger = TestLogger::new(LevelFilter::Info, Config::default());
    /// #[cfg(test)]
    /// let test_logger = TestLogger::new(LevelFilter::Info, Config::default());
    /// # }
    /// ```
    pub fn new(log_level: LevelFilter, config: Config) -> Box<TestLogger> {
        Box::new(TestLogger {
            level: log_level,
            config,
        })
    }
}

impl Log for TestLogger {
    fn enabled(&self, metadata: &Metadata<'_>) -> bool {
        metadata.level() <= self.level
    }

    fn log(&self, record: &Record<'_>) {
        if self.enabled(record.metadata()) {
            let _ = log(&self.config, record);
        }
    }

    fn flush(&self) {}
}

impl SharedLogger for TestLogger {
    fn level(&self) -> LevelFilter {
        self.level
    }

    fn config(&self) -> Option<&Config> {
        Some(&self.config)
    }

    fn as_log(self: Box<Self>) -> Box<dyn Log> {
        Box::new(*self)
    }
}

#[inline(always)]
pub fn log(config: &Config, record: &Record<'_>) {
    if should_skip(&config, &record) {
        return;
    }

    if config.time <= record.level() && config.time != LevelFilter::Off {
        write_time(config);
    }

    if config.level <= record.level() && config.level != LevelFilter::Off {
        write_level(record, config);
    }

    if config.thread < record.level() && config.thread != LevelFilter::Off {
        write_thread_id();
    }

    if config.target <= record.level() && config.target != LevelFilter::Off {
        write_target(record);
    }

    if config.location <= record.level() && config.location != LevelFilter::Off {
        write_location(record);
    }

    write_args(record);
}

#[inline(always)]
pub fn write_time(config: &Config) {
    let cur_time = if config.time_local {
        chrono::Local::now().naive_local() + config.time_offset
    } else {
        chrono::Utc::now().naive_utc() + config.time_offset
    };
    print!(
        "{} ",
        cur_time.format(&*config.time_format)
    );
}

#[inline(always)]
pub fn write_level(record: &Record<'_>, config: &Config) {
    match config.level_padding {
        LevelPadding::Left => print!("[{: >5}] ", record.level()),
        LevelPadding::Right => print!("[{: <5}] ", record.level()),
        LevelPadding::Off => print!("[{}] ", record.level()),
    };
}

#[inline(always)]
pub fn write_thread_id() {
    let id = format!("{:?}", thread::current().id());
    let id = id.replace("ThreadId(", "");
    let id = id.replace(")", "");
    print!("({}) ", id);
}

#[inline(always)]
pub fn write_target(record: &Record<'_>) {
    print!("{}: ", record.target());
}

#[inline(always)]
pub fn write_location(record: &Record<'_>) {
    let file = record.file().unwrap_or("<unknown>");
    if let Some(line) = record.line() {
        print!("[{}:{}] ", file, line);
    } else {
        print!("[{}:<unknown>] ", file);
    }
}

#[inline(always)]
pub fn write_args(record: &Record<'_>) {
    println!("{}", record.args());
}
