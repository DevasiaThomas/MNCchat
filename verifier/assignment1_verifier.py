#!/usr/bin/python
#
# This file is part of CSE 489/589 PA1: Verifier.
# 
# CSE 489/589 PA1: Verifier is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
# 
# CSE 489/589 PA1: Verifier is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with CSE 489/589 PA1: Verifier. If not, see <http://www.gnu.org/licenses/>.
# 

__author__ = "Swetank Kumar Saha (swetankk@buffalo.edu)"
__copyright__ = "Copyright (C) 2016 Swetank Kumar Saha"
__license__ = "GNU GPL"
__version__ = "3.2"

import argparse

parser = argparse.ArgumentParser(description='CSE 489/589 PA1: Verifier')

requiredArgs = parser.add_argument_group('required named arguments')
requiredArgs.add_argument('-p', '--path', dest='path', type=str, nargs=1, help='path to your binary', required=True)

parser.add_argument('-au', '--author', dest='author', action="store_true", help='Verify AUTHOR command and exit')
parser.add_argument('-i', '--ip', dest='ip', action="store_true", help='Verify IP command and exit')
parser.add_argument('-po', '--port', dest='port', action="store_true", help='Verify PORT command and exit')
parser.add_argument('-l', '--list', dest='list', action="store_true", help='Verify LIST command and exit')
parser.add_argument('-s', '--send', dest='send', action="store_true", help='Verify SEND command and exit')
parser.add_argument('-b', '--broadcast', dest='broadcast', action="store_true", help='Verify BROADCAST command and exit')
parser.add_argument('-st', '--statistics', dest='statistics', action="store_true", help='Verify STATISTICS command and exit')
parser.add_argument('-a', '--all', dest='all', action="store_true", help='Verify all commands and exit')

args = parser.parse_args()

def procStatus(pid):
    for line in open("/proc/%d/status" % pid).readlines():
        if line.startswith("State:"):
            return line.split(":",1)[1].strip().split(' ')[0]
    return None

if __name__ == "__main__":
	
	import sys
	import subprocess
	import time
	import os
	import socket
	import re
	
	def extractOutputSuccess(command, logfile_path):
		logfile = open(logfile_path, 'r')
		log_output = logfile.read()
		logfile.close()

		matches = re.compile('\['+command+':SUCCESS\]\\n(.*?)\['+command+':END\]\\n', re.DOTALL).search(log_output)
		
		if not matches: 
			print "\033[91mNo output detected: Check format/syntax of output.\033[0m"
			return None
		return matches.group(1)

	from assignment1_parser import *

	logfile_path = os.path.dirname(args.path[0])+"/logs/assignment_log_"+socket.getfqdn().split('.')[0]

	def parseOutput(output, parse_func):
		if output:
			sys.stdout.write('\033[94m'+output+'\033[0m')
			sys.stdout.flush()
			print "---------------------------------------"
			print "I parsed the following from the output:"
			print '\033[92m'+parse_func()+'\033[0m'

	#Application Startup
	print
	print '\033[33m'+"Application Startup ..."+'\033[0m',
	sys.stdout.flush()
	s_or_c = 's'
	port = 4242
	command = args.path[0]+" "+s_or_c+" "+str(port)
	process = subprocess.Popen(command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
	time.sleep(4)
	status = procStatus(process.pid)
	if status == 'R' or status == 'S':
		os.system('kill -9 '+str(process.pid))
		print "PASS"
	else:
		print "FAIL"
		exit()


	#AUTHOR
	if args.author or args.all:
		print
		print '\033[33m'+"AUTHOR ..."+'\033[0m',
		sys.stdout.flush()
		s_or_c = 's'
		port = 4242
		expect_command = "expect -f author.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		process = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		time.sleep(3)
		
		print
		print "I got the following output:"
		output = extractOutputSuccess("AUTHOR", logfile_path+"_"+str(port))
		parseOutput(output, lambda: parseAUTHOR(output))

	
	#IP
	if args.ip or args.all:
		print
		print '\033[33m'+"IP ..."+'\033[0m',
		sys.stdout.flush()
		s_or_c = 's'
		port = 4242
		expect_command = "expect -f ip.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		process = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		time.sleep(3)

		print
		print "I got the following output:"
		output = extractOutputSuccess("IP", logfile_path+"_"+str(port))
		parseOutput(output, lambda: parseIP(output))


	#PORT
	if args.port or args.all:
		print
		print '\033[33m'+"PORT ..."+'\033[0m',
		sys.stdout.flush()
		s_or_c = 's'
		port = 4242
		expect_command = "expect -f port.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		process = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		time.sleep(3)

		print
		print "I got the following output:"
		output = extractOutputSuccess("PORT", logfile_path+"_"+str(port))
		parseOutput(output, lambda: parsePORT(output))
	

	#LIST
	if args.list or args.all:
		print
		print '\033[33m'+"LIST ..."+'\033[0m',
		sys.stdout.flush()
		s_or_c = 's'
		port = 4242
		expect_command = "expect -f list_server.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		server = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)

		s_or_c = 'c'
		port = 1111
		expect_command = "expect -f list_client.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		client_1 = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		time.sleep(1)
		port = 1212
		expect_command = "expect -f list_client.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		client_2 = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		time.sleep(1)
		port = 1313
		expect_command = "expect -f list_client.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		client_3 = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		time.sleep(1)
		port = 1414
		expect_command = "expect -f list_client.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		client_4 = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		time.sleep(12)

		os.system('kill -9 '+str(client_1.pid))
		os.system('kill -9 '+str(client_2.pid))
		os.system('kill -9 '+str(client_3.pid))
		os.system('kill -9 '+str(client_4.pid))

		print
		print "I got the following output:"
		print "-On Server-"
		output = extractOutputSuccess("LIST", logfile_path+"_4242")
		parseOutput(output, lambda: parseLIST(output, 'server'))
			
		print "-On Client 1-"
		output = extractOutputSuccess("LIST", logfile_path+"_1111")
		parseOutput(output, lambda: parseLIST(output, 'client1'))

		print "-On Client 2-"
		output = extractOutputSuccess("LIST", logfile_path+"_1212")
		parseOutput(output, lambda: parseLIST(output, 'client2'))

		print "-On Client 3-"
		output = extractOutputSuccess("LIST", logfile_path+"_1313")
		parseOutput(output, lambda: parseLIST(output, 'client3'))

		print "-On Client 4-"
		output = extractOutputSuccess("LIST", logfile_path+"_1414")
		parseOutput(output, lambda: parseLIST(output, 'client4'))


	#SEND
	if args.send or args.all:
		print
		print '\033[33m'+"SEND ..."+'\033[0m',
		sys.stdout.flush()
		s_or_c = 's'
		port = 4242
		expect_command = "expect -f send_server.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		server = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		s_or_c = 'c'
		port = 1111
		expect_command = "expect -f send_client.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		client = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		time.sleep(8)

		os.system('kill -9 '+str(client.pid))

		print
		print "I got the following output:"
		print "-On Server-"
		output = extractOutputSuccess("RELAYED", logfile_path+"_4242")
		parseOutput(output, lambda: parseRELAYED(output))
		print
		print "-On Client-"
		output = extractOutputSuccess("RECEIVED", logfile_path+"_1111")
		parseOutput(output, lambda: parseRECEIVED(output))
		print


	#BROADCAST
	if args.broadcast or args.all:
		print
		print '\033[33m'+"BROADCAST ..."+'\033[0m',
		sys.stdout.flush()
		s_or_c = 's'
		port = 4242
		expect_command = "expect -f broadcast_server.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		server = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		s_or_c = 'c'
		port = 1111
		expect_command = "expect -f broadcast_client_1.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		client_1 = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		port = 1212
		expect_command = "expect -f broadcast_client_2.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		client_2 = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		time.sleep(10)

		os.system('kill -9 '+str(client_1.pid))
		os.system('kill -9 '+str(client_2.pid))

		print
		print "I got the following output:"
		print "-On Server-"
		output = extractOutputSuccess("RELAYED", logfile_path+"_4242")
		parseOutput(output, lambda: parseRELAYED(output))
		print
		print "-On Client 1-"
		output = extractOutputSuccess("RECEIVED", logfile_path+"_1111")
		parseOutput(output, lambda: parseRECEIVED(output))
		print
		print "-On Client 2-"
		output = extractOutputSuccess("RECEIVED", logfile_path+"_1212")
		parseOutput(output, lambda: parseRECEIVED(output))
		print


	#STATISTICS
	if args.statistics or args.all:
		print
		print '\033[33m'+"STATISTICS ..."+'\033[0m',
		sys.stdout.flush()
		s_or_c = 's'
		port = 4242
		expect_command = "expect -f statistics_server.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		server = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		s_or_c = 'c'
		port = 1111
		expect_command = "expect -f statistics_client_1.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		client_1 = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		port = 1212
		expect_command = "expect -f statistics_client_2.exp "+args.path[0]+" "+s_or_c+" "+str(port)
		client_2 = subprocess.Popen(expect_command, shell=True, stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT)
		time.sleep(15)

		os.system('kill -9 '+str(client_1.pid))
		os.system('kill -9 '+str(client_2.pid))

		print
		print "I got the following output:"
		output = extractOutputSuccess("STATISTICS", logfile_path+"_4242")
		parseOutput(output, lambda: parseSTATISTICS(output))
		print
	