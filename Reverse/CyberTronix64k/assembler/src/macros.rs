macro_rules! error {
  ($position:expr, $fmt:expr) => ({
    <::std::io::Stderr as ::std::io::Write>::write_fmt(
      &mut ::std::io::stderr(), format_args!(concat!("error: ", $fmt))
    ).expect("Writing to stderr failed");
    <::std::io::Stderr as ::std::io::Write>::write_fmt(
      &mut ::std::io::stderr(), format_args!(" at {}\n", $position)
    ).expect("Writing to stderr failed");
    <::std::io::Stderr as ::std::io::Write>::write_fmt(
      &mut ::std::io::stderr(), format_args!(
        "note: assembler at {}:{}\n",  file!(), line!(),
      ),
    ).expect("Writing to stderr failed");
    ::std::process::exit(1);
  });
  ($position:expr, $fmt:expr, $($arg:tt)*) => ({
    <::std::io::Stderr as ::std::io::Write>::write_fmt(
      &mut ::std::io::stderr(), format_args!(concat!("error: ", $fmt), $($arg)*)
    ).expect("Writing to stderr failed");
    <::std::io::Stderr as ::std::io::Write>::write_fmt(
      &mut ::std::io::stderr(), format_args!(" at {}\n", $position)
    ).expect("Writing to stderr failed");
    <::std::io::Stderr as ::std::io::Write>::write_fmt(
      &mut ::std::io::stderr(), format_args!(
        "note: assembler at {}:{}\n",  file!(), line!(),
      ),
    ).expect("Writing to stderr failed");
    ::std::process::exit(1);
  });
}

// no line number
macro_rules! error_np {
  ($fmt:expr) => ({
    <::std::io::Stderr as ::std::io::Write>::write_fmt(
      &mut ::std::io::stderr(), format_args!(concat!("error: ", $fmt, "\n"))
    ).expect("Writing to stderr failed");
    <::std::io::Stderr as ::std::io::Write>::write_fmt(
      &mut ::std::io::stderr(), format_args!(
        "note: error found here: {}:{}\n",  file!(), line!(),
      ),
    ).expect("Writing to stderr failed");
    ::std::process::exit(1);
  });
  ($fmt:expr, $($arg:tt)*) => ({
    <::std::io::Stderr as ::std::io::Write>::write_fmt(
      &mut ::std::io::stderr(), format_args!(
        concat!("error: ", $fmt, "\n"), $($arg)*
      )
    ).expect("Writing to stderr failed");
    <::std::io::Stderr as ::std::io::Write>::write_fmt(
      &mut ::std::io::stderr(), format_args!(
        "note: error found here: {}:{}\n",  file!(), line!(),
      ),
    ).expect("Writing to stderr failed");
    ::std::process::exit(1);
  });
}
