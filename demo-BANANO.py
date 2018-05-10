#!/usr/bin/env python
# *******************************************************************************
# *   Ledger Blue
# *   (c) 2016 Ledger
# *
# *  Licensed under the Apache License, Version 2.0 (the "License");
# *  you may not use this file except in compliance with the License.
# *  You may obtain a copy of the License at
# *
# *      http://www.apache.org/licenses/LICENSE-2.0
# *
# *  Unless required by applicable law or agreed to in writing, software
# *  distributed under the License is distributed on an "AS IS" BASIS,
# *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# *  See the License for the specific language governing permissions and
# *  limitations under the License.
# ********************************************************************************
from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
import binascii

bipp44_path = (
    "8000002C"
    + "800000a5"
    + "80000000"
    + "80000000"
    + "80000000")

# sending to AHXSMB19pWytwJ7vzvCw5aWmd1DUniDKRT

# # blank tx
# textToSign_01 = bytes(bytearray.fromhex(
#     "80028000340000000000000000000000000000000000000000000000000000000000000000" + bipp44_path + ""))

# parent(32), link(32), rep(32), balance(16)
# tx with small amount, wide range
textToSign_01 = bytes(bytearray.fromhex(
    "80028000"
    # length
    + "70"

    # parent
    #           1         2         3         4         5         6
    #  1234567890123456789012345678901234567890123456789012345678901234
    + "AA00000000000000000000000000000000000000000000000000000000AA"

    # link
    + "BB00000000000000000000000000000000000000000000000000000000BB"

    # rep
    + "FF00000000000000000000000000000000000000000000000000000000FF"

    # balance
    #                     1         2         3
    # ban        123456789012345678901234567890
    # dec 1334440654591915542993625911497130241
    #           1         2         3
    #  12345678901234567890123456789012
    + "0000000000010101010101010101010101"

    # bip44 path
    + bipp44_path))

# tx with small amount, 0x6d14 error
# textToSign_01 = bytes(bytearray.fromhex(
#     "800280003400000000000000000000FF060102030405000708090A0B0C0D00000000000000" + bipp44_path + ""))

# # tx with small amount, 0x6d14 error, going from 99 to 100.
# textToSign_01 = bytes(bytearray.fromhex(
#     "800280003400000000000000000000FF016302030405000708090A0B0C0D00000000000000" + bipp44_path + ""))

# textToSign_01 = bytes(bytearray.fromhex(
#   "80028000343D6AD425971738FD0C1B8C006B4885D38EDEF802D2ACB8DCAB30933154F6962B" + bipp44_path + ""))
# textToSign_01 = bytes(bytearray.fromhex(
#   "80028000343D6AD425971738FD0C1B8CAA6B4885D38EDEF802D2ACB8DCAB30933154F6962B" + bipp44_path + ""))
# textToSign_01 = bytes(bytearray.fromhex(
#   "80028000343D6AD425971738FD0C1B8CAA6B4885D38EDEF802D2ACB8DCAB30933154F6962B" + bipp44_path + ""))

textToSignArray = [textToSign_01]

dongle = getDongle(True)

#print ("STARTED privateKey ")
#privateKey = dongle.exchange(bytes(bytearray.fromhex("80050000FF"+ bipp44_path)))
#actualPrivateKey = privateKey.hex().upper()
#expectPrivateKey = "55BCF45FFD718D8939FB798AAD5B5C8C55D3F08E42507D0D8479F50AB648639E"
#print ("ACTUAL privateKey " + actualPrivateKey)
#print ("EXPECT privateKey " + expectPrivateKey)
#print ("MATCH? privateKey " + str(actualPrivateKey == expectPrivateKey))

print("STARTED publicKey[1] ")
publicKey = dongle.exchange(
    bytes(bytearray.fromhex("80040000FF" + bipp44_path)))
actualPublicKey = publicKey.hex().upper()
expectPublicKey = "0AD1D5CB2DB69BFE31560ED1909135D6D5FA6A471D35FD9B1424C84CA68B9C40"
print("ACTUAL publicKey[1] " + actualPublicKey)
print("EXPECT publicKey[1] " + expectPublicKey)
print("MATCH? publicKey[1] " + str(actualPublicKey == expectPublicKey))

print("STARTED publicKey[2] ")
publicKey = dongle.exchange(
    bytes(bytearray.fromhex("80040000FF" + bipp44_path)))
actualPublicKey = publicKey.hex().upper()
expectPublicKey = "0AD1D5CB2DB69BFE31560ED1909135D6D5FA6A471D35FD9B1424C84CA68B9C40"
print("ACTUAL publicKey[2] " + actualPublicKey)
print("EXPECT publicKey[2] " + expectPublicKey)
print("MATCH? publicKey[2] " + str(actualPublicKey == expectPublicKey))

# print ("STARTED sending transactions (sign on device)")
#
for textToSign in textToSignArray:
    try:
        offset = 0
        while offset != len(textToSign):
            if (len(textToSign) - offset) > 255:
                chunk = textToSign[offset: offset + 255]
            else:
                chunk = textToSign[offset:]
            if (offset + len(chunk)) == len(textToSign):
                p1 = 0x80
            else:
                p1 = 0x00
            apdu = bytes(bytearray.fromhex("8002")) + p1.to_bytes(1, byteorder='big') \
                + 0x00.to_bytes(1, byteorder='big') + \
                len(chunk).to_bytes(1, byteorder='big') + bytes(chunk)
            signature = dongle.exchange(apdu)
            offset += len(chunk)
        print("signature " + signature.hex().upper())
    except CommException as comm:
        if comm.sw == 0x6985:
            print("Aborted by user")
        else:
            print("Invalid status " + hex(comm.sw))

print("SUCCESS sending transactions ")
