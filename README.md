# blue-app-banano CE

This is the community edition of the Ledger Nano S app for the banano Cryptocoin.

accounts are in base32 format, 64 characters long.
valid characters are
	13456789 ABCDEFGH
	IJKMNOPQ RSTUWXYZ

ban_14pjtq7kufnuzrroe5
pjk4amdopozbo6g9bozpf
jab8abkmaq941 hkourxtg

ui is 128x32.
at 10 pixels high we can display 3 lines, 21 characters.
that gives us 6 pixels width per character, for a total width of 126.


# roadmap
1. develop ledger nano s app (started, signing works, workign on UI)   

2. find a light wallet that can be used with bananos (found: [bananovault](https://github.com/coranos/bananovault))

3. integration test light wallet with RPC and Ledger Nano S. (started)


# development

Run `make load` to build and load the application onto the device.

After installing and running the application, you can run `python demo-BANANO.py` to test signing several transactions over USB.

changed
	/usr/local/lib/python3.5/dist-packages/ledgerblue/hexParser.py
