BEGIN{
	FS=":"
	OFS=""
	gfile="/etc/group"
	#print "debug"
	#print ""
	while((getline < gfile) > 0){
		gname=$1
		gid=$3
		usrs=$4
		split(usrs, users, ",")
		for(i in users){
			usr = users[i]
			ugroup[usr] = ((ugroup[usr] == "" ? "" : (ugroup[usr] ",")) gname)
	#		printf "%s ", usr
		}
	#	print ""
		idgroup[gid]=gname
	}
	#print "end debug"
	#print ""
	close(gfile)
	printf "%-10s %-20s %s\n\n", "UID", "USERNAME", "GROUPS" > "/root/p2-2/p4.log"
}
{
	if($NF=="/usr/sbin/nologin"){
		uname=$1
		gid=$4
		uid=$3
		groups = idgroup[gid] (ugroup[uname]=="" ? "" : ("," ugroup[uname]))
		printf "%-10s %-20s %s\n", uid, uname, groups | "sort -k1 -rn >> /root/p2-2/p4.log"
	}
}

