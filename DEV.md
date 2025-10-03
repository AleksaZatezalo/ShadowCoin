0. Create makefile & install dependencies
1. crypto/hash.c/h
   └─ Implement SHA-256
   └─ Add Argon2id wrapper
   └─ Write unit tests
2. crypto/signature.c/h
   └─ Implement Ed25519 signing/verification
   └─ Add key generation
   └─ Write unit tests
3. wallet/wallet.c/h (basic)
   └─ BIP39 seed phrase generation
   └─ Key derivation (BIP32/BIP44)
   └─ Address generation
   └─ Write unit tests
4. blockchain/transaction.c/h
   └─ Transaction structure
   └─ Transaction serialization/deserialization
   └─ Input/output handling
   └─ Signature verification for transactions
   └─ Write unit tests
5. blockchain/block.c/h
   └─ Block structure
   └─ Block serialization/deserialization
   └─ Merkle root calculation
   └─ Block validation (signature checks, merkle root)
   └─ Write unit tests
6. blockchain/chain.c/h (basic)
   └─ Genesis block creation
   └─ Block linking (prev_hash validation)
   └─ Basic chain validation
   └─ In-memory chain storage for now
   └─ Write unit tests
7. wallet/balance.c/h
   └─ UTXO set management
   └─ Balance calculation
   └─ Finding unspent outputs for spending
   └─ Write unit tests
8. mining/difficulty.c/h
   └─ Difficulty target calculation
   └─ Difficulty adjustment algorithm
   └─ Target to bits conversion
   └─ Write unit tests
9. mining/miner.c/h
   └─ Proof-of-work implementation (Simplehash)
   └─ Block template creation
   └─ Nonce iteration
   └─ Multi-threaded mining
   └─ Write unit tests
10. blockchain/chain.c/h (complete)
    └─ Persistent storage (save/load blockchain)
    └─ Chain reorganization
    └─ Orphan block handling
    └─ Write unit tests
11. cli/parser.c/h
    └─ Command-line argument parsing
    └─ Command routing
    └─ Basic input validation
    └─ Write unit tests
12. main.c (basic)
    └─ Entry point
    └─ Initialize subsystems
    └─ Route to CLI commands
    └─ Clean shutdown
13. rpc/server.c/h
    └─ HTTP server setup
    └─ JSON-RPC request parsing
    └─ Response formatting
    └─ Authentication
    └─ Write unit tests
14. rpc/commands.c/h
    └─ Implement RPC methods
    └─ Map to internal functions
    └─ Error handling
    └─ Write unit tests
15. daemon.c/h (basic)
    └─ Background daemon mode
    └─ RPC server integration
    └─ Signal handling
16. network/protocol.c/h
    └─ Message serialization
    └─ Message types (version, inv, getdata, etc.)
    └─ Message parsing
    └─ Write unit tests
17. network/peer.c/h
    └─ TCP connection handling
    └─ Peer discovery (DNS seeds, hardcoded)
    └─ Peer connection management
    └─ Message sending/receiving
    └─ Handshake protocol
    └─ Write integration tests
18. daemon.c/h (complete)
    └─ Network thread integration
    └─ Block propagation
    └─ Transaction propagation
    └─ Mempool management
    └─ Sync protocol