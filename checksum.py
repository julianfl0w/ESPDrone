import sys

#print 'Number of arguments:', len(sys.argv), 'arguments.'
#print 'Argument List:', str(sys.argv)

theString = str(sys.argv[1])
result = ord(theString[0]);

for i in range(1, len(theString)):
	result = result ^ ord(theString[i])

CK_A = 0
CK_B = 0
for i in range(0, len(theString)):
	CK_A = (CK_A + ord(theString[i])) & 0xFF
	CK_B = (CK_B + CK_A) & 0xFF

print hex(result)
#print hex(CK_A)
#print hex(CK_B)

