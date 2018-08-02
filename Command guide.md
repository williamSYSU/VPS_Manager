# Command Guide
Some command you need to monitor the user or manage your user.

## Auto Manager Command
* Add user with time, and restart SSR
```bash
manager -add MONTH | tee -a server.log
manager -refresh
```
* Add batch of user with specific number (not activate, will not expired)
```bash
manager -batch NUMBER | tee -a server.log
manager	-refresh
```
* Delete user by port number, then need to back and refresh SSR
```bash
manager -delete PORT_NUM | tee -a server.log
manager -back | tee -a server.log
manager	-refresh
```
* Activate port by port number and time, then refresh SSR
```bash
manager -activate PORT_NUM MONTH | tee -a server.log
manager	-refresh
```
* Prolong use of port by port number and time, then refresh SSR
```bash
manager -prolong PORT_NUM MONTH | tee -a server.log
manager	-refresh
```
* Change password by port number, then refresh
```bash
manager -change PORT_NUM | tee -a server.log
manager -refresh
```
* Get information of port by port number
```bash
manager -info PORT_NUM
```
* Back up config file (*/etc/shadowsocks.json* to */root/vps-server/shadowsocks-backup.json*)
```bash
manager -back
```
* Scan config file to check the expired port
```bash
manager -scan
```
* Count total users
```bash
manager -count
```
* Recount total users (Set config file correctly)
```bash
manager -recount
```
* Count ports that not been activated
```bash
manager -falsecount
```
* Delete all ports that not been activated
```bash
manager -deletefalse
```
* Restart SSR
```bash
manager -refresh
```
* Show help
```bash
manager -help
```

## Monitor connection
* 显示所有进出链接
```bash
netstat -anp |grep 'ESTABLISHED' |grep 'python'
```
* 仅显示链接服务器的用户连接（后面加grep '端口号'查对应端口的IP）
```bash
netstat -anp |grep 'ESTABLISHED' |grep 'python' |grep 'tcp'
```
* 仅显示链接服务器的用户连接数量
```bash
netstat -anp |grep 'ESTABLISHED' |grep 'python' |grep 'tcp' |wc -l
```
* 仅显示链接服务器的用户连接并写入到文件
```bash
netstat -anp |grep 'ESTABLISHED' |grep 'python' |grep 'tcp'>>/root/log.txt
```

---
* 如果你是多用户版(多个端口)的服务端，那么你可以用这个命令
* 显示当前链接服务器的用户的SS端口
```bash
netstat -anp |grep 'ESTABLISHED' |grep 'python' |grep 'tcp' | grep '::ffff:' |awk '{print $4}' |sort -u
```
* 显示当前链接服务器的用户的SS端口数量
```bash
netstat -anp |grep 'ESTABLISHED' |grep 'python' |grep 'tcp' | grep '::ffff:' |awk '{print $4}' |sort -u |wc -l
```
* *显示当前所有链接SS的端口对应的用户IP* (见currentuser.sh)
```bash
netstat -anp |grep 'ESTABLISHED' |grep 'python' |grep 'tcp' |awk '{print $4"\t"$5}' |awk -F "::ffff:" '{print $2"\t"$3}' |awk -F ":" '{print "port: "$2}' |sort -u
```
* 显示当前所有链接SS的用户IP数量
```bash
netstat -anp |grep 'ESTABLISHED' |grep 'python' |grep 'tcp' |awk '{print $5}' |awk -F ":" '{print $1}' |sort -u |wc -l
```
