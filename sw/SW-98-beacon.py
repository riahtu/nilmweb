#!/usr/bin/python
#==================================================
# SW-98-beacon.py
# 2016-04-21
#==================================================
import time
import httplib
import os
#==================================================
# Read configuration
#==================================================
print 'Version 2016-08-18'
g_debug     = 'YES';
g_server    = '78.67.160.17'
g_sercon    = 'config.nabton.com'
g_path      = '/sxndata/index.php'
g_ipaddress = 'x.x.x.x'
g_delay     = 10
g_name      = 'uniqueName'
#==================================================
os.system("ifconfig > ipaddress.work")
file = open('ipaddress.work','r')
for line in file:
    if 'Bcast' in line:
        words=line.split(' ')
        work=words[11].split(':')
        g_ipaddress = work[1] 
        print 'local ipaddress ' + g_ipaddress
        split_ip = g_ipaddress.split('.')
        last_ip = split_ip[4]
        print 'last ip: ' + last_ip

req = g_path+ '?mid=5'+'&appid=98'+'&ip=' + g_ipaddress + '&name=' + g_name  
     
while 1:
	conn = httplib.HTTPConnection(g_server)
	try:
		conn.request("GET", req)
		try:
			r1 = conn.getresponse()
			if g_debug == 'YES':
                		print ("Server Response:-_- %s %s " % (r1.status, r1.reason))
            		data1 = r1.read()
            		if g_debug == 'YES':
                		print data1
        	except:
            		print '-_- No response from nb server'
    	except:
        	print '-_- Not able to connect to server '+g_server
    	conn.close()
    	time.sleep(g_delay)
# End of file
