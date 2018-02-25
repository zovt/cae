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
}
