for path in $(find resources/ -type f); do
	file=$(echo $path | sed 's,.*/,,g')
	folder=$(echo $path | sed "s,$file,,g")
	echo build/include/$folder$file.hh
done

