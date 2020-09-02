use crate::{Config, LevelPadding, ThreadPadding, ThreadLogMode};
use chrono;
use log::{LevelFilter, Record};
use std::io::{Error, Write};
use std::thread;

#[inline(always)]
pub fn try_log<W>(config: &Config, record: &Record<'_>, write: &mut W) -> Result<(), Error>
where
    W: Write + Sized,
{
    if should_skip(config, record) {
        return Ok(());
    }

    if config.time <= record.level() && config.time != LevelFilter::Off {
        write_time(write, config)?;
    }

    if config.level <= record.level() && config.level != LevelFilter::Off {
        write_level(record, write, config)?;
    }

    if config.thread <= record.level() && config.thread != LevelFilter::Off {
        match config.thread_log_mode {
            ThreadLogMode::IDs => {
                write_thread_id(write, config)?;
            }
            ThreadLogMode::Names | ThreadLogMode::Both => {
                write_thread_name(write, config)?;
            }
        }
    }

    if config.target <= record.level() && config.target != LevelFilter::Off {
        write_target(record, write)?;
    }

    if config.location <= record.level() && config.location != LevelFilter::Off {
        write_location(record, write)?;
    }

    write_args(record, write)
}

#[inline(always)]
pub fn write_time<W>(write: &mut W, config: &Config) -> Result<(), Error>
where
    W: Write + Sized,
{
    let cur_time = if config.time_local {
        (chrono::Local::now() + config.time_offset).format(&*config.time_format)
    } else {
        (chrono::Utc::now() + config.time_offset).format(&*config.time_format)
    };

    write!(write, "{} ", cur_time)?;
    Ok(())
}

#[inline(always)]
pub fn write_level<W>(record: &Record<'_>, write: &mut W, config: &Config) -> Result<(), Error>
where
    W: Write + Sized,
{
    match config.level_padding {
        LevelPadding::Left => write!(write, "[{: >5}] ", record.level())?,
        LevelPadding::Right => write!(write, "[{: <5}] ", record.level())?,
        LevelPadding::Off => write!(write, "[{}] ", record.level())?,
    };
    Ok(())
}

#[inline(always)]
pub fn write_target<W>(record: &Record<'_>, write: &mut W) -> Result<(), Error>
where
    W: Write + Sized,
{
    write!(write, "{}: ", record.target())?;
    Ok(())
}

#[inline(always)]
pub fn write_location<W>(record: &Record<'_>, write: &mut W) -> Result<(), Error>
where
    W: Write + Sized,
{
    let file = record.file().unwrap_or("<unknown>");
    if let Some(line) = record.line() {
        write!(write, "[{}:{}] ", file, line)?;
    } else {
        write!(write, "[{}:<unknown>] ", file)?;
    }
    Ok(())
}

pub fn write_thread_name<W>(write: &mut W, config: &Config) -> Result<(), Error>
where
    W: Write + Sized,
{
    if let Some(name) = thread::current().name() {
        match config.thread_padding {
            ThreadPadding::Left{0: qty} => {
                write!(write, "({name:>0$}) ", qty, name=name)?;
            }
            ThreadPadding::Right{0: qty} => {
                write!(write, "({name:<0$}) ", qty, name=name)?;
            }
            ThreadPadding::Off => {
                write!(write, "({}) ", name)?;
            }
        }
    } else if config.thread_log_mode == ThreadLogMode::Both {
        write_thread_id(write, config)?;
    }

    Ok(())
}

pub fn write_thread_id<W>(write: &mut W, config: &Config) -> Result<(), Error>
where
    W: Write + Sized,
{
    let id = format!("{:?}", thread::current().id());
    let id = id.replace("ThreadId(", "");
    let id = id.replace(")", "");
    match config.thread_padding {
        ThreadPadding::Left{0: qty} => {
            write!(write, "({id:>0$}) ", qty, id=id)?;
        }
        ThreadPadding::Right{0: qty} => {
            write!(write, "({id:<0$}) ", qty, id=id)?;
        }
        ThreadPadding::Off => {
            write!(write, "({}) ", id)?;
        }
    }
    Ok(())
}

#[inline(always)]
pub fn write_args<W>(record: &Record<'_>, write: &mut W) -> Result<(), Error>
where
    W: Write + Sized,
{
    writeln!(write, "{}", record.args())?;
    Ok(())
}

#[inline(always)]
pub fn should_skip(config: &Config, record: &Record<'_>) -> bool {
    // If a module path and allowed list are available
    match (record.target(), &*config.filter_allow) {
        (path, allowed) if allowed.len() > 0 => {
            // Check that the module path matches at least one allow filter
            if let None = allowed.iter().find(|v| path.starts_with(&***v)) {
                // If not, skip any further writing
                return true;
            }
        }
        _ => {}
    }

    // If a module path and ignore list are available
    match (record.target(), &*config.filter_ignore) {
        (path, ignore) if ignore.len() > 0 => {
            // Check that the module path does not match any ignore filters
            if let Some(_) = ignore.iter().find(|v| path.starts_with(&***v)) {
                // If not, skip any further writing
                return true;
            }
        }
        _ => {}
    }

    return false;
}
