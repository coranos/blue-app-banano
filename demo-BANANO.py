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
              +"800000a5"
              +"80000000"
              +"80000000"
              +"80000000")

# sending to AHXSMB19pWytwJ7vzvCw5aWmd1DUniDKRT

# blank tx
textToSign_01 = bytes(("80028000744284F08CA8FE206FE6854E21AA69163FEAFAE248F0A8D53E4FAE700B26122B613FE80B4BC842E82C1C18ABFEEC47EA989E63953BC82AC411F304D13833D52A560ad1d5cb2db69bfe31560ed1909135d6d5fa6a471d35fd9b1424c84ca68b9c40" + bipp44_path + "").decode('hex'))

textToSignArray = [textToSign_01]

dongle = getDongle(True)

print "STARTED resetStatus "
resetStatus = dongle.exchange(bytes(("80030000FF").decode('hex')))
print "SUCCESS resetStatus " + str(resetStatus).encode('hex')

print "STARTED privateKey "
privateKey = dongle.exchange(bytes(("80050000FF"+ bipp44_path).decode('hex')))
actualPrivateKey = str(privateKey).encode('hex').upper()
expectPrivateKey = "55BCF45FFD718D8939FB798AAD5B5C8C55D3F08E42507D0D8479F50AB648639E"
print "ACTUAL privateKey " + actualPrivateKey
print "EXPECT privateKey " + expectPrivateKey
print "MATCH? privateKey " + str(actualPrivateKey == expectPrivateKey)

print "STARTED publicKey "
publicKey = dongle.exchange(bytes(("80040000FF"+ bipp44_path).decode('hex')))
actualPublicKey = str(publicKey).encode('hex').upper()
expectPublicKey = "0AD1D5CB2DB69BFE31560ED1909135D6D5FA6A471D35FD9B1424C84CA68B9C40"
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