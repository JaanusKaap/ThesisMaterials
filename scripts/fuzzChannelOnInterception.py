import pykd
import uuid
import sys
import random

xorList = [1, 2, 4, 8, 16, 32, 64, 128]

def byteArray2ByteBuffer(arr):
	ret = ""
	for x in arr:
		ret += chr(x)
	return ret

def checkValidChannel(addr):
	return (pykd.loadQWords(addr, 1)[0] == addr)

def getChannelPtr(channel):
	if isinstance(channel, int) and not checkValidChannel(channel):
		return None
	if isinstance(channel, str):
		header = pykd.getOffset("vmbkmclr!KmclChannelList")
		nextPtr = pykd.loadQWords(header, 1)[0]
		if header == nextPtr:
			return None
			
		while nextPtr != header:
			base = nextPtr - 0x7A0
			if not checkValidChannel(base):
				return None
			interfaceInstanceGuid = uuid.UUID(bytes_le=byteArray2ByteBuffer(pykd.loadBytes(base + 0x62C, 16)))
			if str(interfaceInstanceGuid) == channel:
				return base
			nextPtr = pykd.loadQWords(nextPtr, 1)[0]
		return None
		
def fuzz(locationName, addr, size, count, physical):
	for x in xrange(count):
		loc = random.randint(addr, addr+size-1)
		val = pykd.loadBytes(loc, 1, physical)[0]
		newVal = val ^ xorList[random.randint(0, 7)]
		pykd.writeBytes(loc, [newVal], physical)
		if physical:
			print "[%s]PHYSICAL: @ 0x%X   0x%02X -> 0x%02X" % (locationName, loc, val, newVal)
		else:
			print "[%s]VIRTUAL: @ 0x%X   0x%02X -> 0x%02X" % (locationName, loc, val, newVal)
	
if len(sys.argv) == 1:
	print "Missing channel address/instance GUID"
	exit()
	
if len(sys.argv) == 2:
	print "Missing max number of requests"
	exit()
	
if len(sys.argv) == 3:
	print "Missing target selection (1, 2 or 3)"
	exit()
	
maxCount = int(sys.argv[2])
fuzzTarget = int(sys.argv[3])
fuzzTargetMain = (fuzzTarget & 0x1) > 0
fuzzTargetMdl = (fuzzTarget & 0x2) > 0
	
if sys.argv[1].startswith("0x"):
	channel = getChannelPtr(int(sys.argv[1][2:], 16))
else:
	channel = getChannelPtr(sys.argv[1])
	
if channel is None:
	print "Could not find channel"
	
pykd.dbgCommand("bc *")
print "Channel found at 0x%X" % channel
packetCallback = pykd.loadQWords(channel + 0x710, 1)[0]
print "packet callback @ 0x%X (%s)" % (packetCallback, pykd.findSymbol(packetCallback))
pykd.dbgCommand("bp 0x%X" % packetCallback)

packets = {}
breakAddr = pykd.getOffset("nt!DbgBreakPointWithStatus")
extAddr = pykd.getOffset("vmbkmclr!VmbChannelPacketGetExternalData")
if fuzzTargetMdl:
	pykd.dbgCommand("bp 0x%X" % extAddr)

cnt = 0
while maxCount == 0 or cnt < maxCount:
	try: 
		pykd.dbgCommand("gh")
		if pykd.reg("rip") != packetCallback and pykd.reg("rip") != extAddr:
			if pykd.reg("rip") == breakAddr:
				break
			continue
		if pykd.reg("rip") == packetCallback:
			buf = pykd.reg("r8")
			bufSize = pykd.reg("r9")
			extDataFlag = pykd.loadQWords(pykd.reg("rsp") + 5*8, 1)[0] & 1
			print "%04d>Packet @ 0x%X sized 0x%X with flag 0x%X" % (cnt, buf, bufSize, extDataFlag)
			if fuzzTargetMain:
				fuzz("MAIN", buf, bufSize, random.randint(1, int(bufSize/10)), False)
				cnt += 1
			if fuzzTargetMdl and extDataFlag > 0:
				packets[pykd.reg("rdx")] = (buf, bufSize)
			
		if pykd.reg("rip") == extAddr and pykd.reg("rcx") in packets:
			packet = pykd.reg("rcx")
			ppmdl = pykd.reg("r8")
			pykd.dbgCommand("bd *")
			pykd.dbgCommand("gu")
			pykd.dbgCommand("be *")
			(buf, bufSize) = packets[packet]
			print "%04d>Call to handler with buffer @ 0x%X with size 0x%X and external data:" % (cnt, buf, bufSize)	
			cnt += 1		
			pmdl = pykd.loadQWords(ppmdl, 1)[0]
			while True:
				if pmdl == 0:
					break
				mdl_next = pykd.loadQWords(pmdl, 1)[0]
				mdl_struct_size = pykd.loadWords(pmdl + 0x8, 1)[0]
				mdl_offset = pykd.loadDWords(pmdl + 0x2C, 1)[0]
				mdl_size = pykd.loadDWords(pmdl + 0x28, 1)[0]			
				if mdl_size > 0x10000:
					break
				if mdl_offset != 0:
					break			
				addr = pmdl+0x30
				mdl_size_tmp = mdl_size
				while addr < pmdl+mdl_struct_size:
					print "0x%X < 0x%X" % (addr, pmdl+mdl_struct_size)
					buf = pykd.loadQWords(addr, 1)[0]
					fuzz("MDL", buf, min(0x1000, mdl_size_tmp), 5, True)
					mdl_size_tmp -= 0x1000			
					addr += 0x8
				if mdl_next == 0:
					break
				pmdl = mdl_next
			del packets[packet]
	except:
		raise
		pass
			
		