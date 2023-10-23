#!/usr/bin/env python3
#
#   parsing - Parsing extensions
#
#   Copyright (C) 2023 by Matt Roberts.
#   License: GNU GPL3 (www.gnu.org)
#
#

# system modules
import time
import sys
import os


#
#  isroger(s)
#
def isroger(s):
	if not s:
		return False
	s = s.upper()
	return s == 'RRR' or s == 'RR73' or (s[0] == 'R' and isreport(s))


#
#  isreport(s)
#
def isreport(s):
	if not s:
		return False
	s = s.upper()
	if len(s) == 3:
		return s[0] in '+-' and s[1:].isdigit()
	elif len(s) == 4:
		return s[0] == 'R' and s[1] in '+-' and s[2:].isdigit()
	return False


#
#  is73(s)
#
def is73(s):
	if not s:
		return False
	s = s.upper()
	return s == 'RR73' or s == '73' or s == 'TU73'


#
#  isgrid(s)
#
def isgrid(s):
	if not s:
		return False
	s = s.upper()
	if s == 'RR73' or s == 'TU73':
		return False
	return len(s) == 4 and s[0].isalpha() and s[1].isalpha() and s[2].isdigit() and s[3].isdigit()


#
#  iscall(s)
#
def iscall(s):
	if not s:
		return False
	if s.upper() == 'RR73':
		return False
	if isgrid(s):
		return False

	digits = 0
	letters = 0
	slashes = 0
	other = 0
	if s.startswith('<') and s.endswith('>'):
		s = s[1:-1]
	for ch in s:
		if ch.isdigit():
			digits += 1
		elif ch.isalpha():
			letters += 1
		elif ch == '/':
			slashes += 1
		else:
			other += 1

	return other == 0 and digits >= 1 and letters >= 2;


#
#  basecall(s) - return call with prefixes and suffixes removed
#
def basecall(s):
	if not iscall(s):
		return None
	s = s.upper()
	if s.startswith('<') and s.endswith('>'):
		s = s[1:-1]
	parts = s.split('/')
	if len(parts) == 1:
		return s
	calls = [ ]
	for part in parts:
		if iscall(part):
			calls.append(part)
	if len(calls) == 0:
		return None
	if len(calls) == 1:
		return calls[0]
	result = ""
	for call in calls:
		if len(call) >= len(result):
			result = call
	if not result:
		return None
	return result


#
#  entry point - just unit tests
#
if __name__ == '__main__':
	for i in [ 'kk5jy', 'n5osl', 'n7ul', 'rr73', 'rr7a', 'em16', 'FO33' ]:
		print("isgrid(%s) = %s" % (i, isgrid(i)))
		print("iscall(%s) = %s" % (i, iscall(i)))
	print("---------------------")
	for i in [ '+13', '-05', 'R+03', 'r-3', 'RRR', 'RR73', 'RR72', '73' ]:
		print("isroger(%s)  = %s" % (i, isroger(i)))
		print("isreport(%s) = %s" % (i, isreport(i)))
		print("is73(%s)     = %s" % (i, is73(i)))
	print("---------------------")
	for i in [ 'kk5jy', 'n5osl', 'n7ul', 'kk5jy/r', 'n5osl/3', 'n7ul/qrp', 've3/ab0cd', '<ab0cd>', '<ab0cd/33>', "<...>" ]:
		print("basecall(%s) = %s" % (i, basecall(i)))

# EOF
