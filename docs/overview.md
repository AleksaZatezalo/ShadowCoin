# Simplecoin Technical Specification

**Version:** 1.0  
**Last Updated:** October 2025

## Table of Contents

1. [Overview](#overview)
2. [Consensus Mechanism](#consensus-mechanism)
3. [Block Structure](#block-structure)
4. [Transaction Model](#transaction-model)
5. [Cryptographic Primitives](#cryptographic-primitives)
6. [Network Protocol](#network-protocol)
7. [Mining Algorithm](#mining-algorithm)
8. [Wallet Architecture](#wallet-architecture)
9. [RPC Interface](#rpc-interface)
10. [Data Storage](#data-storage)

---

## Overview

Simplecoin is a proof-of-work cryptocurrency implementing fundamental blockchain concepts. The system implements a UTXO-based transaction model with Ed25519 signatures and SHA-256 proof-of-work.

### Design Goals

- **Simplicity**: Minimal dependencies, straightforward codebase
- **Educational Value**: Clear demonstration of cryptocurrency fundamentals
- **Self-Contained**: Single binary for all operations
- **Standards Compliance**: Follows established cryptographic best practices

### Key Parameters

| Parameter | Value | Notes |
|-----------|-------|-------|
| Target Block Time | 120 seconds | 2 minutes |
| Difficulty Adjustment Period | 144 blocks | ~4.8 hours |
| Block Reward (Initial) | 50 SC | Halves every 210,000 blocks |
| Maximum Supply | 21,000,000 SC | Asymptotic limit |
| Address Format | Base58Check | Ed25519 public key derived |
| Signature Scheme | Ed25519 | Fast, secure, deterministic |

---

## Consensus Mechanism

### Proof-of-Work

Simplecoin uses standard double SHA-256 proof-of-work.

**Target Difficulty**: Blocks are valid when `SHA-256(SHA-256(header)) < target`, where target is adjusted every 144 blocks to maintain 120-second average block time.

**Difficulty Adjustment Algorithm**:

```
actual_time = timestamp[block_n] - timestamp[block_n-144]
expected_time = 144 * 120  // 17,280 seconds

adjustment_factor = expected_time / actual_time
new_difficulty = old_difficulty * adjustment_factor

// Clamp adjustment to prevent extreme swings
adjustment_factor = clamp(adjustment_factor, 0.25, 4.0)
```

### Longest Chain Rule

The canonical chain is determined by cumulative proof-of-work:

```
chain_work = Σ(2^256 / (difficulty + 1))
```

Nodes always follow the chain with the highest cumulative work, not merely the longest chain by block count.

### Orphan Handling

Blocks received without their parent are stored in an orphan pool (max 100 blocks). When a missing parent arrives, orphan descendants are recursively validated and potentially integrated into the chain.

---

## Block Structure

### Block Header (80 bytes)

```c
typedef struct BlockHeader {
    uint32_t version;              // Protocol version (currently 1)
    uint8_t  prev_hash[32];        // SHA-256 of previous block header
    uint8_t  merkle_root[32];      // Merkle root of transactions
    uint32_t timestamp;            // Unix timestamp (seconds)
    uint32_t difficulty;           // Compact difficulty target
    uint32_t nonce;                // Mining nonce
} BlockHeader;
```

### Full Block Format

```c
typedef struct Block {
    BlockHeader header;
    uint32_t tx_count;             // Number of transactions
    Transaction *transactions;      // Transaction array
} Block;
```

### Serialization

Blocks are serialized in little-endian format:

```
[version:4][prev_hash:32][merkle_root:32][timestamp:4][difficulty:4]
[nonce:4][tx_count:4][transactions:variable]
```

### Block Hash Calculation

The block hash is the double SHA-256 of the 80-byte header:

```c
uint8_t block_hash[32];
sha256_double(header, 80, block_hash);
```

### Merkle Tree Construction

Transaction hashes are organized in a binary Merkle tree:

1. Hash each transaction: `tx_hash = SHA-256(SHA-256(tx_serialized))`
2. If odd number of transactions, duplicate the last hash
3. Concatenate pairs and hash: `parent = SHA-256(left || right)`
4. Repeat until single root hash remains

The coinbase transaction (block reward) is always the first transaction.

---

## Transaction Model

### UTXO Model

Simplecoin uses an Unspent Transaction Output (UTXO) model. Each transaction consumes existing UTXOs as inputs and creates new UTXOs as outputs.

### Transaction Structure

```c
typedef struct Transaction {
    uint32_t version;              // Transaction version
    uint32_t input_count;          // Number of inputs
    TxInput *inputs;               // Input array
    uint32_t output_count;         // Number of outputs
    TxOutput *outputs;             // Output array
    uint32_t locktime;             // Block height or timestamp lock
} Transaction;

typedef struct TxInput {
    uint8_t prev_tx_hash[32];      // Hash of transaction containing output
    uint32_t output_index;         // Index of output in that transaction
    uint8_t signature[64];         // Ed25519 signature
    uint8_t pubkey[32];            // Ed25519 public key
} TxInput;

typedef struct TxOutput {
    uint64_t amount;               // Amount in base units (1 SC = 10^8 units)
    uint8_t recipient_pubkey[32];  // Recipient's public key
} TxOutput;
```

### Transaction ID

Transaction ID is the double SHA-256 of the serialized transaction:

```c
uint8_t txid[32];
sha256_double(tx_serialized, tx_size, txid);
```

### Transaction Validation

A transaction is valid if:

1. All input references exist and are unspent
2. Each input signature is valid for the transaction hash
3. Sum of inputs ≥ Sum of outputs (difference is miner fee)
4. No duplicate inputs (double-spend prevention)
5. Locktime constraint satisfied
6. Transaction size ≤ 100 KB

**Signature Verification**:

```c
// Message to sign is the transaction with inputs replaced by empty signatures
uint8_t tx_for_signing[...];
prepare_tx_for_signing(tx, input_index, tx_for_signing);

uint8_t tx_hash[32];
sha256_double(tx_for_signing, size, tx_hash);

bool valid = ed25519_verify(signature, tx_hash, 32, pubkey);
```

### Coinbase Transaction

The first transaction in each block is a coinbase transaction:

- No inputs (input count = 0)
- One or more outputs
- Total output value = Block Reward + Transaction Fees
- First 100 bytes of coinbase can contain arbitrary data

**Block Reward Schedule**:

```
reward = 50 * 10^8 / (2^(block_height / 210000))
```

### Transaction Fees

Transaction fee is implicitly defined:

```
fee = Σ(input_amounts) - Σ(output_amounts)
```

Miners prioritize transactions by fee rate (fee/byte). Minimum relay fee is 1000 units/KB.

---

## Cryptographic Primitives

### Hash Functions

**SHA-256**: Used for all hashing operations (block hashes, transaction IDs, merkle trees)

```c
void sha256(const uint8_t *data, size_t len, uint8_t hash[32]);
void sha256_double(const uint8_t *data, size_t len, uint8_t hash[32]);
```

Implementation follows FIPS 180-4 standard.

### Digital Signatures

**Ed25519**: Used for transaction signatures

- Key Generation: 32-byte private key → 32-byte public key
- Signing: Deterministic, produces 64-byte signature
- Verification: Fast, constant-time

```c
void ed25519_create_keypair(uint8_t pubkey[32], uint8_t privkey[64], const uint8_t seed[32]);
void ed25519_sign(uint8_t signature[64], const uint8_t *message, size_t len, const uint8_t privkey[64]);
bool ed25519_verify(const uint8_t signature[64], const uint8_t *message, size_t len, const uint8_t pubkey[32]);
```

### Address Encoding

Addresses are Base58Check encoded public keys:

```
version_byte = 0x1E  // 'S' in Base58
checksum = SHA-256(SHA-256(version_byte || pubkey))[:4]
address = Base58Encode(version_byte || pubkey || checksum)
```

Example address: `S7rK8mQ3vN2pL9xW4hF6jC5tY8uE1sA2nB9vX3cM4dN5e`

---

## Network Protocol

### Peer Discovery

1. **DNS Seeds**: Query `seed.simplecoin.org` for initial peer list
2. **Peer Exchange**: Request peer addresses from connected nodes
3. **Configuration**: Manually specify peers via `-connect` or config file

### Connection Protocol

TCP connections on port 8333 (mainnet). All messages follow this format:

```
[magic:4][command:12][payload_size:4][checksum:4][payload:variable]
```

- **Magic**: `0xD9B4BEF9` (identifies Simplecoin protocol)
- **Command**: ASCII string (null-padded to 12 bytes)
- **Checksum**: First 4 bytes of `SHA-256(SHA-256(payload))`

### Message Types

#### Version Handshake

```c
typedef struct VersionMessage {
    uint32_t version;              // Protocol version (10001)
    uint64_t services;             // Service flags (NODE_NETWORK = 1)
    uint64_t timestamp;            // Current timestamp
    uint8_t addr_recv[26];         // Receiving node's address
    uint8_t addr_from[26];         // Sending node's address
    uint64_t nonce;                // Connection nonce (anti-self)
    char user_agent[256];          // Client identification
    uint32_t start_height;         // Blockchain height
} VersionMessage;
```

Connection flow:
1. Node A → Node B: `version` message
2. Node B → Node A: `version` message
3. Node B → Node A: `verack` acknowledgment
4. Node A → Node B: `verack` acknowledgment

#### Block Propagation

**inv** (inventory): Announce availability of blocks/transactions

```c
typedef struct InvVector {
    uint32_t type;                 // 1=TX, 2=BLOCK
    uint8_t hash[32];              // Object hash
} InvVector;
```

**getdata**: Request specific objects by hash

**block**: Send block data

**getheaders**: Request block headers (for initial sync)

**headers**: Respond with block headers

#### Transaction Propagation

Nodes announce new transactions via `inv` messages. Peers request full transaction data with `getdata` if interested.

### Synchronization

New nodes synchronize using headers-first approach:

1. Request headers from peers (`getheaders`)
2. Validate header chain (proof-of-work, timestamps)
3. Request full blocks for the best chain (`getdata`)
4. Validate transactions and update UTXO set

---

## Mining Algorithm

### Standard Proof-of-Work

The mining algorithm uses double SHA-256:

```c
bool mine_block(BlockHeader *header, uint32_t target_difficulty) {
    uint8_t hash[32];
    
    while (true) {
        // Calculate double SHA-256 of header
        sha256_double((uint8_t*)header, sizeof(BlockHeader), hash);
        
        // Check if hash meets difficulty target
        if (meets_difficulty(hash, target_difficulty)) {
            return true;  // Valid block found
        }
        
        // Increment nonce and try again
        header->nonce++;
        
        // Update timestamp periodically to include new transactions
        if (header->nonce % 1000000 == 0) {
            header->timestamp = current_time();
        }
    }
}
```

### Mining Difficulty

Difficulty is stored as a compact 32-bit representation (same as Bitcoin):

```c
uint32_t difficulty_to_compact(uint256_t difficulty);
uint256_t compact_to_difficulty(uint32_t compact);

bool meets_difficulty(uint8_t hash[32], uint32_t compact_target) {
    uint256_t hash_value = bytes_to_uint256(hash);
    uint256_t target = compact_to_difficulty(compact_target);
    return hash_value < target;
}
```

### Multi-threaded Mining

The miner can utilize multiple CPU cores:

```c
typedef struct MiningThread {
    pthread_t thread_id;
    BlockHeader header_template;
    uint32_t nonce_start;          // Each thread gets different nonce range
    uint32_t nonce_end;
    bool *stop_flag;               // Shared stop signal
    bool *solution_found;          // Shared solution signal
} MiningThread;
```

Each thread works on the same block template but with different nonce ranges to avoid duplicate work.

---

## Wallet Architecture

### Hierarchical Deterministic (HD) Wallet

Simplecoin uses BIP39-style 24-word mnemonic seeds:

```
Entropy (256 bits) → Mnemonic (24 words) → Seed (512 bits) → Master Key
```

### Key Derivation

```c
// Master key from seed
void derive_master_key(const uint8_t seed[64], uint8_t master_private[32], uint8_t master_public[32]);

// Child key derivation (non-hardened)
void derive_child_key(
    const uint8_t parent_private[32],
    uint32_t index,
    uint8_t child_private[32],
    uint8_t child_public[32]
);
```

### Wallet Storage

The `wallet.dat` file contains:

```c
typedef struct WalletData {
    uint32_t version;              // Wallet version
    uint8_t encrypted_seed[64];    // AES-256-GCM encrypted
    uint8_t salt[16];              // Password derivation salt
    uint8_t nonce[12];             // GCM nonce
    uint8_t tag[16];               // GCM authentication tag
    uint32_t key_count;            // Number of derived keys
    WalletKey *keys;               // Derived key metadata
} WalletData;
```

**Encryption**: User password is stretched with Argon2id (10 iterations, 64 MB memory) to derive AES-256 key.

### UTXO Tracking

The wallet maintains a local index of UTXOs belonging to its addresses:

```c
typedef struct WalletUTXO {
    uint8_t txid[32];              // Transaction ID
    uint32_t output_index;         // Output index
    uint64_t amount;               // Amount in base units
    uint8_t pubkey[32];            // Controlling public key
    uint32_t confirmations;        // Block confirmations
    bool spent;                    // Spend status
} WalletUTXO;
```

### Transaction Construction

Building a transaction:

1. **Input Selection**: Choose UTXOs to cover amount + fee (use coin selection algorithm)
2. **Create Outputs**: Recipient output + change output (if needed)
3. **Sign Inputs**: Generate Ed25519 signatures for each input
4. **Broadcast**: Send to network via RPC or P2P

---

## RPC Interface

### JSON-RPC 1.0

The daemon exposes a JSON-RPC interface on port 8332 (configurable).

**Authentication**: HTTP Basic Auth using `rpcuser` and `rpcpassword` from config.

### Request Format

```json
{
    "jsonrpc": "1.0",
    "id": "client-request-1",
    "method": "getblockchaininfo",
    "params": []
}
```

### Response Format

```json
{
    "result": {
        "chain": "main",
        "blocks": 12345,
        "headers": 12345,
        "difficulty": 1024.5,
        "mediantime": 1696339200
    },
    "error": null,
    "id": "client-request-1"
}
```

### Core RPC Methods

#### Blockchain

- `getblockchaininfo` → Chain status
- `getblock <hash>` → Block details
- `getblockbyhash <height>` → Block by height
- `gettransaction <txid>` → Transaction details
- `getrawmempool` → Unconfirmed transactions

#### Wallet

- `createwallet` → Generate new wallet
- `recoverwallet <mnemonic>` → Restore from seed
- `getbalance` → Total balance
- `getaddress [index]` → Get receiving address
- `sendtoaddress <address> <amount>` → Send coins
- `listtransactions [count]` → Transaction history

#### Mining

- `setgenerate <true|false> [threads]` → Control mining
- `getmininginfo` → Mining statistics
- `getblocktemplate` → Template for external miners
- `submitblock <hex>` → Submit mined block

#### Network

- `getpeerinfo` → Connected peers
- `getnetworkinfo` → Network status
- `addnode <ip:port> <add|remove|onetry>` → Manage connections

---

## Data Storage

### Directory Structure

```
~/.Simplecoin/
├── Simplecoin.conf          # Configuration
├── wallet.dat               # Encrypted wallet
├── peers.dat                # Known peers (binary format)
├── data/
│   ├── blocks/
│   │   ├── blk00000.dat    # Block data files (128 MB each)
│   │   ├── blk00001.dat
│   │   └── index/
│   │       └── block.idx   # Block index (hash → file position)
│   ├── chainstate/
│   │   └── utxo.db         # UTXO set (LevelDB-like)
│   └── mempool/
│       └── mempool.dat     # Mempool persistence
└── debug.log                # Application log
```

### Block Storage

Blocks are stored in flat files (`blk*.dat`) with a separate index:

```c
typedef struct BlockIndex {
    uint8_t hash[32];              // Block hash
    uint32_t file_number;          // blk file number
    uint64_t file_offset;          // Byte offset in file
    uint32_t block_size;           // Block size in bytes
    uint32_t height;               // Block height
    uint64_t cumulative_work;      // Total chain work
} BlockIndex;
```

### UTXO Database

The UTXO set is stored as key-value pairs:

**Key**: `txid || output_index` (36 bytes)  
**Value**: `amount || pubkey || height` (44 bytes)

Database operations:

- `get(key)` → Retrieve UTXO
- `put(key, value)` → Add new UTXO
- `delete(key)` → Mark UTXO as spent
- `batch_commit()` → Atomic updates

### Pruning (Future)

Pruning mode will retain only:
- Recent blocks (last N MB)
- UTXO set
- Block headers (full chain)

---

## Security Considerations

### Double-Spend Prevention

1. Transaction pool validation (reject conflicting inputs)
2. Block validation (ensure no double-spends in block)
3. UTXO set atomicity (updates are atomic per block)
4. Reorganization handling (revert and replay on chain switch)

### Network Security

- **Eclipse Attacks**: Mitigated by diverse peer connections and periodic peer rotation
- **Sybil Attacks**: Connection limits (max 125 peers) and proof-of-work requirements
- **51% Attacks**: Inherent to PoW; mitigated by diverse mining participation

### Wallet Security

- **Encryption**: AES-256-GCM with Argon2id key derivation
- **Seed Phrase**: 256-bit entropy, written down offline
- **No Key Reuse**: HD wallet derives unique keys per transaction
- **Secure Deletion**: Memory is zeroed after use (private keys, passwords)

---

## Future Enhancements

1. **Segregated Witness**: Separate signature data to increase block capacity
2. **Lightning Network**: Layer-2 payment channels for instant transactions
3. **Schnorr Signatures**: More efficient multi-signature schemes
4. **UTXO Commitments**: Merkle tree of UTXO set in blocks for fast sync
5. **Bloom Filters**: SPV clients for lightweight wallets

---

## References

- Bitcoin Whitepaper: Nakamoto, S. (2008)
- Ed25519 High-speed high-security signatures: Bernstein, D. et al. (2011)
- Merkle Trees: Merkle, R. (1988)
- FIPS 180-4: Secure Hash Standard (SHA-256)

---

**Document Maintained By**: Simplecoin Core Development Team  
**Feedback**: Submit issues or corrections via the project repository