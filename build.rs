extern crate bindgen;
extern crate pkg_config;

use std::path::PathBuf;
use std::process::Command;

fn main() {
	let hb = pkg_config::Config::new()
		.atleast_version("1.7.2")
		.statik(true)
		.probe("harfbuzz")
		.unwrap();
	let mut hb_libs_dedup = hb.libs.clone();
	hb_libs_dedup.sort();
	hb_libs_dedup.dedup();
	hb_libs_dedup
		.iter()
		.for_each(|l| println!("cargo:rustc-link-lib={}", l));
	hb.include_paths
		.iter()
		.for_each(|p| println!("cargo:include={}", p.display()));

	let ft = pkg_config::Config::new()
		.atleast_version("2.8.1")
		.statik(true)
		.probe("freetype2")
		.unwrap();
	ft.libs
		.iter()
		.for_each(|l| println!("cargo:rustc-link-lib={}", l));
	ft.include_paths
		.iter()
		.for_each(|p| println!("cargo:include={}", p.display()));

	let hb_includes = hb.include_paths
		.iter()
		.map(|p| "-I".to_string() + AsRef::<str>::as_ref(&p.to_string_lossy()))
		.collect::<Vec<String>>();
	let ft_includes = ft.include_paths
		.iter()
		.map(|p| "-I".to_string() + AsRef::<str>::as_ref(&p.to_string_lossy()))
		.collect::<Vec<String>>();
	let bindings = bindgen::Builder::default()
		.header("hb_bind.h")
		.raw_line("#![allow(dead_code)]")
		.raw_line("#![allow(non_upper_case_globals)]")
		.raw_line("#![allow(non_camel_case_types)]")
		.raw_line("#![allow(non_snake_case)]")
		.raw_line("use freetype::freetype::*;")
		.whitelist_type("hb_.*")
		.whitelist_function("hb_.*")
		.whitelist_var("hb_.*")
		.blacklist_type("FT_.*")
		.prepend_enum_name(false)
		.clang_args(hb_includes)
		.clang_args(ft_includes)
		.generate()
		.expect("Unable to generate harfbuzz bindings");

	let mut out = PathBuf::from("src");
	out.push("hb_raw.rs");
	bindings
		.write_to_file(out)
		.expect("Failed to write harfbuzz bindings");
}
