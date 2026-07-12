BEGIN {
	FS=": "
}
{
	src=$1
	cnt[src]++
}
END {
	for(s in cnt)
		print cnt[s], s
}
