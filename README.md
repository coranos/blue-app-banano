# blue-app-banano CE

This is the community edition of the Ledger Nano S app for the banano Cryptocoin.

# roadmap
1. develop ledger nano s app (development complete)

   1.1. test ledger blue port of ledger nano app  (development complete)
   

   1.2. get someone to pay for the Ledger Nano app to be listed on Ledger's store (not started)
    
   **ETA: Unknown.**
   
   

   1.3. get someone to pay for the Ledger Blue app to be listed on Ledger's store (not started)
    
   **ETA: Unknown.**
   


2. develop light wallet that connects to both Banano RPC and Ledger Nano S / Ledger Blue (development in progress)

   [ledger-wallet-banano](https://github.com/coranos/ledger-wallet-banano)

   **ETA: Early February.**
   
   

3. integration test light wallet with RPC and Ledger Nano S. (not started)

   **ETA: Late February.**
   


   3.1. integration test light wallet with RPC and Ledger Nano S. (not started)
    
   **ETA: Late February.**
   
   

# development

Run `make load` to build and load the application onto the device.

After installing and running the application, you can run `python demo-BANANO.py` to test signing several transactions over USB.

changed 
	/usr/local/lib/python3.5/dist-packages/ledgerblue/hexParser.py
	