#!/bin/sh

for path in $@; do
	file=$(echo $path | sed 's,.*/,,g')
	folder=$(echo $path | sed "s,$file,,g")
	underscore_file=$(echo $file | tr '.' '_')
	underscore_path=$(echo $path | tr './' '_')
	output_header=build/include/$folder$file.hh
	echo "Building $output_header..."
	mkdir -p build/include/$folder
	: > $output_header
	echo "#pragma once" >> $output_header
	echo "#include <string>" >> $output_header
	xxd -i $path | sed \
		-e "s,$underscore_path,$underscore_file,g;"\
		-e "s,unsigned char,std::vector<unsigned char> const,g;"\
		-e "s,\[\],,g;"\
		-e "/unsigned int/d;" >> $output_header
done
