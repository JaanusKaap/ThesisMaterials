import pykd
import uuid
import sys
import os

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
	else:
		return channel
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
		
def getChannelGuid(channel):
	if not checkValidChannel(channel):
		return None
	interfaceInstanceGuid = uuid.UUID(bytes_le=byteArray2ByteBuffer(pykd.loadBytes(channel + 0x62C, 16)))
	return str(interfaceInstanceGuid)
	
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
	
if sys.argv[1] == "ALL":
	channel = "ALL"
else:
	if sys.argv[1].startswith("0x"):
		channel = getChannelPtr(int(sys.argv[1][2:], 16))
	else:
		channel = getChannelPtr(sys.argv[1])
	
if channel is None:
	print "Could not find channel"
	
pykd.dbgCommand("bc *")
if channel == "ALL":
	print "All channel recorded"
else:
	print "Channel found at 0x%X" % channel

packets = {}
breakAddr = pykd.getOffset("nt!DbgBreakPointWithStatus")
readAddr = pykd.getOffset("vmbusr!PipeTryReadSingle")
pykd.dbgCommand("bp 0x%X" % readAddr)

cnt = 0
while maxCount == 0 or cnt < maxCount:
	pykd.dbgCommand("gh")			
	if pykd.reg("rip") != readAddr:
		continue
	if pykd.reg("rip") == breakAddr:
		break
	
	pipeStruct = pykd.reg("rcx")
	irp = pykd.reg("rdx")
	pykd.dbgCommand("bd *")
	pykd.dbgCommand("gu")
	pykd.dbgCommand("be *")
	reqChannel = pykd.loadQWords(pipeStruct+0x100, 1)[0]
	if reqChannel == 0 or (channel != "ALL" and reqChannel != channel):
		print "Skipping request to channel 0x%X" % reqChannel
		continue
	guid = getChannelGuid(reqChannel)	
	respLen = pykd.loadQWords(irp+0x38, 1)[0]
	if respLen == 0:
		print "Skipping request because size 0"
		continue
	respData = pykd.loadQWords(pykd.loadQWords(irp+0x8, 1)[0]+0x18, 1)[0]
	bufData = byteArray2ByteBuffer(pykd.loadBytes(respData, respLen))
	
	if not os.path.isdir("%s\\%s" % (outDir, guid)):  
		os.mkdir("%s\\%s" % (outDir, guid))
	
	print "#0x%03X: Logging 0x%X bytes from 0x%X at channel %s" % (cnt, respLen, respData, guid)
	f = open("%s\\%s\\data%04X.bin" % (outDir, guid, cnt), "wb")
	f.write(bufData)
	f.close()
	cnt += 1
	
	