import pykd
import uuid
import sys

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
	
if len(sys.argv) == 1:
	print "Missing channel address/instance GUID"
	exit()
	
if len(sys.argv) == 2:
	print "Missing output directory"
	exit()
	
if len(sys.argv) == 3:
	print "Missing max number of requests"
	exit()
	
outDir = sys.argv[2]
maxCount = int(sys.argv[3])
logExtData = False
if len(sys.argv) > 4 and int(sys.argv[4]) > 0:
	logExtData = True
	
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
if logExtData:
	pykd.dbgCommand("bp 0x%X" % extAddr)

cnt = 0
while maxCount == 0 or cnt < maxCount:
	pykd.dbgCommand("gh")
	if pykd.reg("rip") != packetCallback and pykd.reg("rip") != extAddr:
		if pykd.reg("rip") == breakAddr:
			break
		continue
	if pykd.reg("rip") == packetCallback:
		buf = pykd.reg("r8")
		bufSize = pykd.reg("r9")
		bufData = byteArray2ByteBuffer(pykd.loadBytes(buf, bufSize))
		extDataFlag = pykd.loadQWords(pykd.reg("rsp") + 5*8, 1)[0] & 1
		if extDataFlag == 0 or not logExtData:
			print "Call to handler with buffer @ 0x%X with size 0x%X and no external data" % (buf, bufSize)	
			f = open("%s\\data%04X.bin" % (outDir, cnt), "wb")
			f.write(bufData)
			f.close()
			cnt += 1
		else:
			packets[pykd.reg("rdx")] = (buf, bufSize)
	if pykd.reg("rip") == extAddr and pykd.reg("rcx") in packets:
		packet = pykd.reg("rcx")
		ppmdl = pykd.reg("r8")
		pykd.dbgCommand("bd *")
		pykd.dbgCommand("gu")
		pykd.dbgCommand("be *")
		(buf, bufSize) = packets[packet]
		print "Call to handler with buffer @ 0x%X with size 0x%X and external data:" % (buf, bufSize)	
		f = open("%s\\data%04X.bin" % (outDir, cnt), "wb")
		f.write(bufData)
		f.close()
		f = open("%s\\mdl%04X.bin" % (outDir, cnt), "wb")
		cnt += 1		
		pmdl = pykd.loadQWords(ppmdl, 1)[0]
		while True:
			mdl_next = pykd.loadQWords(pmdl, 1)[0]
			mdl_struct_size = pykd.loadWords(pmdl + 0x8, 1)[0]
			mdl_offset = pykd.loadDWords(pmdl + 0x2C, 1)[0]
			mdl_size = pykd.loadDWords(pmdl + 0x28, 1)[0]
			
			if mdl_offset != 0:
				print "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX: 0x%X" % mdl_offset
				break
			
			addr = pmdl+0x30
			mdl_size_tmp = mdl_size
			while addr < pmdl+mdl_struct_size:
				buf = pykd.loadQWords(addr, 1)[0]
				bufData = byteArray2ByteBuffer(pykd.loadBytes(buf * 0x1000, min(0x1000, mdl_size_tmp), True))
				mdl_size_tmp -= 0x1000
				f.write(bufData)
				addr += 0x8
			
			print "  MDL buffer @ 0x%X with size 0x%X" % (pmdl, mdl_size)
			if mdl_next == 0:
				break
			pmdl = mdl_next
		f.close()
		del packets[packet]
			
		