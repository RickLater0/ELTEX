BEGIN{
	FS="\n"
	RS="\n\n"
	OFS="; "
}
{
	cl = "CL"
	sd = "START"
	ed = "END"
	for(i = 1; i <= NF; i++){
		if($i ~ /^Commandline:/) cl = $i
		else if($i ~ /^Start-Date:/) sd = $i
		else if($i ~ /^End-Date:/) ed = $i
	}
	print cl, sd, ed
}
