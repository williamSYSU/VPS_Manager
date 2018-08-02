netstat -anp |grep 'ESTABLISHED' |grep 'python' |grep 'tcp' |awk '{print $4"\t"$5}' |awk -F "::ffff:" '{print $2"\t"$3}' |awk -F ":" '{print "port: "$2}' |sort -u
