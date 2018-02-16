#!/bin/sh

MIN_HB_VER=1.7.2
MIN_FT_VER=21.0.15

if [ ! -x "$(command -v pkg-config)" ]; then
	echo "ERROR: pkg-config not installed or not detected";
	exit 1;
fi

if [ $(pkg-config --exists harfbuzz) ]; then
	echo "ERROR: pkg-config cannot find harfbuzz";
	exit 1;
fi

if [ $(pkg-config --atleast-version=$MIN_HB_VER harfbuzz) ]; then
	echo "ERROR: harfbuzz library version is not >=" $MIN_HB_VER;
	exit 1;
fi

if [ $(pkg-config --exists freetype2) ]; then
	echo "ERROR: pkg-config cannot find freetype";
	exit 1;
fi

if [ $(pkg-config --atleast-version=$MIN_FT_VER freetype2) ]; then
	echo "ERROR: freetype library version is not >=" $MIN_FT_VER;
	exit 1;
fi

if [ ! -x "$(command -v bindgen)" ]; then
	echo "ERROR: bindgen is not installed or not detected. Run 'cargo install bindgen'";
	exit 1;
fi

if [ ! -x "$(command -v rustup)" ]; then
	echo "ERROR: rustup is not installed or not detected.";
	exit 1;
fi

set -x
echo "bindgen" \
	"--raw-line '#![allow(dead_code)]'" \
	"--raw-line '#![allow(non_upper_case_globals)]'" \
	"--raw-line '#![allow(non_camel_case_types)]'" \
	"--raw-line '#![allow(non_snake_case)]'" \
	"--raw-line 'use freetype::freetype::*;'" \
	"--whitelist-type 'hb_.*'" \
	"--whitelist-function 'hb_.*'" \
	"--whitelist-var 'hb_.*'" \
	"--blacklist-type 'FT_.*'" \
	"--no-prepend-enum-name" \
	"--rustfmt-bindings" \
	"-o src/hb_raw.rs" \
	"hb_bind.h" \
	"--" \
	"$(pkg-config --cflags-only-I harfbuzz) $(pkg-config --cflags-only-I freetype2)" \
	| rustup run stable sh