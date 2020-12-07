import pykd
import uuid

header = pykd.getOffset("vmbkmclr!KmclChannelList")
nextPtr = pykd.loadQWords(header, 1)[0]

if header == nextPtr:
	print "No channels found!"
	exit()
	
def byteArray2ByteBuffer(arr):
	ret = ""
	for x in arr:
		ret += chr(x)
	return ret

while nextPtr != header:
	base = nextPtr - 0x7A0
	print "Channel at 0x%X" % base 		
	if pykd.loadQWords(base, 1)[0] !=  base:
		print "  INVALID CHANNEL"
		exit()
		
	
	pipe = pykd.loadBytes(base + 0x640, 1)[0]
		
	interfaceTypeGuid = uuid.UUID(bytes_le=byteArray2ByteBuffer(pykd.loadBytes(base + 0x61C, 16)))
	interfaceInstanceGuid = uuid.UUID(bytes_le=byteArray2ByteBuffer(pykd.loadBytes(base + 0x62C, 16)))
	vmIdGuid = uuid.UUID(bytes_le=byteArray2ByteBuffer(pykd.loadBytes(base + 0x650, 16)))
	
	pointer = pykd.loadQWords(base + 0x838, 1)[0]
	primaryChannel = pykd.loadQWords(base + 0xAD8, 1)[0]
	parentDeviceObj = pykd.loadQWords(base + 0xB40, 1)[0]
	subchannelIndex = pykd.loadWords(base + 0xB50, 1)[0]
		
	packetCallback = pykd.loadQWords(base + 0x710, 1)[0]
	completeCallback = pykd.loadQWords(base + 0x718, 1)[0]
	channelOpened = pykd.loadQWords(base + 0x738, 1)[0]
	channelClosed = pykd.loadQWords(base + 0x740, 1)[0]
	channelSuspended = pykd.loadQWords(base + 0x748, 1)[0]
	channelStarted = pykd.loadQWords(base + 0x750, 1)[0]
	channelPostStarted = pykd.loadQWords(base + 0x758, 1)[0]
	
	
	if pipe > 0:
		print "  --Pipe--"
	else:
		print "  --Normal channel--"
	print "  Interface type: %s" % str(interfaceTypeGuid)
	print "  Interface instance: %s" % str(interfaceInstanceGuid)
	print "  VM id: %s" % str(vmIdGuid)
	print "  Pointer: 0x%X" % pointer
	print "  Parent Device Object: 0x%X" % parentDeviceObj
	print "  Primary channel: 0x%X" % primaryChannel
	print "  Sub channel index: 0x%X" % subchannelIndex
	print "  Callbacks"
	if packetCallback > 0:
		print "    packet callback = 0x%X (%s)" % (packetCallback, pykd.findSymbol(packetCallback))
	if completeCallback > 0:
		print "    packet completion callback = 0x%X (%s)" % (completeCallback, pykd.findSymbol(completeCallback))
	if channelOpened > 0:
		print "    channel opened = 0x%X (%s)" % (channelOpened, pykd.findSymbol(channelOpened))
	if channelClosed > 0:
		print "    channel close = 0x%X (%s)" % (channelClosed, pykd.findSymbol(channelClosed))
	if channelSuspended > 0:
		print "    channel suspended = 0x%X (%s)" % (channelSuspended, pykd.findSymbol(channelSuspended))
	if channelStarted > 0:
		print "    channel started = 0x%X (%s)" % (channelStarted, pykd.findSymbol(channelStarted))
	if channelPostStarted > 0:
		print "    channel post started = 0x%X (%s)" % (channelPostStarted, pykd.findSymbol(channelPostStarted))
	
	nextPtr = pykd.loadQWords(nextPtr, 1)[0]
	print "\n"