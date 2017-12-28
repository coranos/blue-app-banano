#!/usr/bin/env python
#*******************************************************************************
#*   Ledger Blue
#*   (c) 2016 Ledger
#*
#*  Licensed under the Apache License, Version 2.0 (the "License");
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*      http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an "AS IS" BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#********************************************************************************
from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
from secp256k1 import PublicKey

bipp44_path = (
               "8000002C"
              +"80000378"
              +"80000000"
              +"80000000"
              +"80000000")

# sending to AHXSMB19pWytwJ7vzvCw5aWmd1DUniDKRT

# blank tx
textToSign_01 = bytes(("0000" + bipp44_path + "").decode('hex'))

textToSignArray = [textToSign_01]

dongle = getDongle(True)

print "STARTED resetStatus "
resetStatus = dongle.exchange(bytes(("80030000FF"+ bipp44_path).decode('hex')))
print "SUCCESS resetStatus " + str(resetStatus).encode('hex')

print "STARTED privateKey "
publicKey = dongle.exchange(bytes(("80050000FF"+ bipp44_path).decode('hex')))
print "SUCCESS privateKey " + str(publicKey).encode('hex')
print "EXPECT  privateKey " + "0000000000000000000000000000000000000000000000000000000000000000"
#print "EXPECT  privateKey " + "17799D6ED28AB940A23B04C2EAE5CA393FACE3C79ABB84B8C5F9069C697D25A4"

print "STARTED publicKey "
publicKey = dongle.exchange(bytes(("80040000FF"+ bipp44_path).decode('hex')))
actualPublicKey = str(publicKey).encode('hex').upper()
expectPublicKey = "19D3D919475DEED4696B5D13018151D1AF88B2BD3BCFF048B45031C1F36D1858"
print "ACTUAL publicKey " + actualPublicKey
print "EXPECT publicKey " + expectPublicKey
print "MATCH? publicKey " + str(actualPublicKey == expectPublicKey)

print "STARTED sending transactions (sign on device)"

for textToSign in textToSignArray:
	try:
		offset = 0
		while offset <> len(textToSign):
			if (len(textToSign) - offset) > 255:
				chunk = textToSign[offset : offset + 255] 
			else:
				chunk = textToSign[offset:]
			if (offset + len(chunk)) == len(textToSign):
				p1 = 0x80
			else:
				p1 = 0x00
			apdu = bytes("8002".decode('hex')) + chr(p1) + chr(0x00) + chr(len(chunk)) + bytes(chunk)
			signature = dongle.exchange(apdu)
			offset += len(chunk)  	
		print "signature " + str(signature).encode('hex')
	except CommException as comm:
		if comm.sw == 0x6985:
			print "Aborted by user"
		else:
			print "Invalid status " + hex(comm.sw)

print "SUCCESS sending transactions "
