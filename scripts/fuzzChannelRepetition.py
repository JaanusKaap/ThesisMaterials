import pykd
import random

xorList = [1, 2, 4, 8, 16, 32, 64, 128]
allRegs = ["rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rip", "rsp", "rbp", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"]

def storeRegs():
	global allRegs
	data = {}
	for reg in allRegs:
		data[reg] = pykd.reg(reg)
	return data
	
def restoreRegs(data):
	global allRegs
	for reg in allRegs:
		pykd.setReg(reg, data[reg])
		
def fuzz(locationName, data, count):
	for x in xrange(count):
		loc = random.randint(0, len(data)-1)
		val = data[loc]
		newVal = val ^ xorList[random.randint(0, 7)]
		data[loc] = newVal
		print "[%s] 0x%X : 0x%02X -> 0x%02X" % (locationName, loc, val, newVal)
	return data
		
def fuzzInput(bufferLoc, bufferData):
	fuzzData = bufferData[:]
	fuzzData = fuzz("MAIN", fuzzData, random.randint(1, 8))
	pykd.writeBytes(bufferLoc, fuzzData)

channel = pykd.reg("rcx")
packet = pykd.reg("rdx")
packetData = pykd.loadBytes(packet, 0x880)
buffer = pykd.reg("r8")
bufferLen = pykd.reg("r9")
bufferData = pykd.loadBytes(buffer, bufferLen)
mdlExist = (pykd.loadQWords(pykd.reg("rsp") + 5*8, 1)[0] & 1) > 0
packetComplete = pykd.getOffset("vmbkmclr!VmbChannelPacketComplete")
packetDeferToPassive = pykd.getOffset("vmbkmclr!VmbChannelPacketDeferToPassive")
packetDeferUntilPolled = pykd.getOffset("vmbkmclr!VmbChannelPacketDeferUntilPolled")
start_irql = pykd.dbgCommand("!irql")
maxRequests = 1

if pykd.loadQWords(channel, 1)[0] != channel:
	print "Channel parameter (from rcx) does not seem to point to channel structure"
if not pykd.isValid(packet):
	print "Packet parameter (from rdx) does not seem to point to valid addess"
if not pykd.isValid(buffer) or not pykd.isValid(buffer + bufferLen):
	print "Buffer parameter range 0x%X - 0x%X (from r8 to r8+r9) does not seem to point to valid addess" % (buffer, buffer + bufferLen)

print "Trying to repeat-fuzz the request with irql: %s" % start_irql
data = storeRegs()
pykd.dbgCommand("bc *")
pykd.dbgCommand("bp 0x%X" % packetComplete)
pykd.dbgCommand("bp 0x%X" % packetDeferToPassive)
pykd.dbgCommand("bp 0x%X" % packetDeferUntilPolled)


for count in xrange(maxRequests):
	if count % 25 == 0:
		print packetData
	fuzzInput(buffer, bufferData)
	print "**************"
	print "**** Fuzz iteration %d" % (count+1)
	print "**************"
	print pykd.dbgCommand("gh")	
	print "**************"
	print "**  %s" % pykd.dbgCommand("!irql")
	print "**************"
	rip = pykd.reg("rip")
	if rip != packetComplete and rip != packetDeferToPassive and rip != packetDeferUntilPolled:
		print "Unknown break location, stopping fuzzing"
		break
	if count == maxRequests-1:
		break
	if pykd.reg("rcx") != packet:
		continue	
	locStr = pykd.findSymbol(rip)
	print "\n\n>>>>> %s" % locStr
	print "***************************"
	pykd.writeBytes(packet, packetData)
	restoreRegs(data)

pykd.dbgCommand("bc *")
print "DONE"	