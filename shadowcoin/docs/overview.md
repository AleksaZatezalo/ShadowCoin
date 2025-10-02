# Shadowcoin Documentation

Complete technical documentation for the Shadowcoin cryptocurrency.

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Architecture Overview](#architecture-overview)
3. [Command Reference](#command-reference)
4. [RPC API Reference](#rpc-api-reference)
5. [Protocol Specification](#protocol-specification)
6. [Mining Guide](#mining-guide)
7. [Wallet Management](#wallet-management)
8. [Network Protocol](#network-protocol)
9. [Building from Source](#building-from-source)
10. [Developer Guide](#developer-guide)

---

## Getting Started

### Installation

Download the latest release or build from source:

```bash
git clone https://github.com/shadowcoin/shadowcoin.git
cd shadowcoin
make
sudo make install
```

### Quick Start

```bash
# Start the node
shadowcoind -daemon

# Create a wallet
shadowcoind createwallet

# Get your address
shadowcoind getaddress

# Check balance
shadowcoind getbalance

# Start mining
shadowcoind setgenerate true 4
```

### Data Directory

Default locations:
- Linux: `~/.shadowcoin/`
- macOS: `~/Library/Application Support/Shadowcoin/`
- Windows: `%APPDATA%\Shadowcoin\`

---

## Architecture Overview

### Component Hierarchy

```
shadowcoind
├── Main Entry Point
│   └── Command Router
├── Daemon Mode
│   ├── Blockchain Manager
│   ├── Network Layer
│   ├── Mining Engine
│   ├── Wallet Manager
│   └── RPC Server
└── CLI Mode
    └── RPC Client
```

### Data Flow

1. **Transaction Creation**: Wallet → Transaction Pool → Block Template
2. **Block Mining**: Mining Engine → Proof-of-Work → Block Validation
3. **Block Propagation**: Network Layer → Peer Validation → Chain Integration
4. **Balance Updates**: Chain Integration → UTXO Set → Wallet Balance

### Threading Model

- **Main Thread**: Event loop, RPC server
- **Network Thread**: P2P message handling, peer management
- **Mining Threads**: Proof-of-work computation (configurable count)
- **Validation Thread**: Block and transaction verification

---

## Command Reference

### Node Control

#### `shadowcoind`
Start the daemon in foreground mode.

**Options:**
- `-daemon` - Run in background
- `-port=<port>` - P2P port (default: 8333)
- `-rpcport=<port>` - RPC port (default: 8332)
- `-datadir=<dir>` - Data directory path
- `-connect=<ip:port>` - Connect to specific peer
- `-maxconnections=<n>` - Maximum peer connections (default: 125)

**Example:**
```bash
shadowcoind -daemon -port=8333 -maxconnections=50
```

#### `stop`
Shut down the daemon gracefully.

```bash
shadowcoind stop
```

#### `getinfo`
Get general information about the node.

**Returns:**
- Version number
- Protocol version
- Wallet version
- Current block height
- Connection count
- Mining status

```bash
shadowcoind getinfo
```

### Wallet Commands

#### `createwallet`
Create a new wallet with generated keypair and 24-word BIP39 seed phrase.

**Returns:**
- 24-word seed phrase (WRITE THIS DOWN - shown only once)
- Public address
- Success confirmation

**Example:**
```bash
shadowcoind createwallet
```

**Output:**
```
Seed Phrase (WRITE THIS DOWN AND STORE SECURELY):
abandon ability able about above absent absorb abstract absurd abuse access accident
account accuse achieve acid acoustic acquire across act action actor actress actual

Your Address: SHD1qxyz...abc
Wallet created successfully.

WARNING: Write down your seed phrase and store it in a secure location.
This is the ONLY way to recover your wallet if wallet.dat is lost.
```

#### `recoverwallet`
Recover wallet from 24-word BIP39 seed phrase.

**Interactive prompt** will ask for seed phrase (space-separated words).

**Example:**
```bash
shadowcoind recoverwallet
```

**Prompt:**
```
Enter your 24-word seed phrase:
> abandon ability able about above absent absorb abstract absurd abuse access accident account accuse achieve acid acoustic acquire across act action actor actress actual

Wallet recovered successfully.
Your Address: SHD1qxyz...abc
```

#### `getaddress`
Get the current wallet's public address.

```bash
shadowcoind getaddress
```

#### `getbalance`
Get confirmed wallet balance.

**Options:**
- `minconf=<n>` - Minimum confirmations (default: 1)

```bash
shadowcoind getbalance
shadowcoind getbalance minconf=6
```

#### `sendtoaddress <address> <amount>`
Send coins to an address.

**Parameters:**
- `address` - Recipient's public address
- `amount` - Amount in coins (decimal)

**Returns:**
- Transaction ID

```bash
shadowcoind sendtoaddress SHD1qxyz...abc 10.5
```

#### `listtransactions`
List recent transactions.

**Options:**
- `count=<n>` - Number of transactions (default: 10)
- `skip=<n>` - Skip first n transactions (default: 0)

```bash
shadowcoind listtransactions count=20
```

#### `backupwallet <destination>`
Backup wallet.dat file to specified location.

**Note:** The seed phrase is a more portable backup method. This command backs up the encrypted wallet file.

```bash
shadowcoind backupwallet ~/shadowcoin-backup.dat
```

#### `encryptwallet <passphrase>`
Encrypt wallet with passphrase.

⚠️ **Warning**: This requires daemon restart.

```bash
shadowcoind encryptwallet "my secure passphrase"
```

#### `walletpassphrase <passphrase> <timeout>`
Unlock encrypted wallet for specified seconds.

```bash
shadowcoind walletpassphrase "my secure passphrase" 300
```

### Blockchain Commands

#### `getblockchaininfo`
Get blockchain status information.

**Returns:**
- Chain height
- Best block hash
- Difficulty
- Verification progress
- Chain work

```bash
shadowcoind getblockchaininfo
```

#### `getblock <hash>`
Get block details by hash.

**Returns:**
- Block header information
- Transaction list
- Confirmations

```bash
shadowcoind getblock 000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f
```

#### `getblockbyhash <height>`
Get block by height number.

```bash
shadowcoind getblockbyhash 12345
```

#### `getblockhash <height>`
Get block hash at specific height.

```bash
shadowcoind getblockhash 100
```

#### `gettransaction <txid>`
Get transaction details.

```bash
shadowcoind gettransaction a1b2c3d4...
```

#### `getmempoolinfo`
Get mempool (transaction pool) information.

**Returns:**
- Transaction count
- Memory usage
- Minimum fee rate

```bash
shadowcoind getmempoolinfo
```

### Mining Commands

#### `setgenerate <true|false> [threads]`
Enable or disable mining.

**Parameters:**
- `true|false` - Enable/disable mining
- `threads` - Number of mining threads (optional)

```bash
shadowcoind setgenerate true 4
shadowcoind setgenerate false
```

#### `getmininginfo`
Get mining statistics.

**Returns:**
- Mining status
- Hashrate
- Difficulty
- Block height
- Network hashrate estimate

```bash
shadowcoind getmininginfo
```

#### `getdifficulty`
Get current mining difficulty.

```bash
shadowcoind getdifficulty
```

#### `getblocktemplate`
Get block template for mining.

```bash
shadowcoind getblocktemplate
```

#### `submitblock <hexdata>`
Submit mined block.

```bash
shadowcoind submitblock 0000002000...
```

### Network Commands

#### `getpeerinfo`
Get information about connected peers.

**Returns:**
- Peer IP addresses
- Connection time
- Protocol version
- Bytes sent/received

```bash
shadowcoind getpeerinfo
```

#### `getnetworkinfo`
Get network status.

**Returns:**
- Protocol version
- Local services
- Connection count
- Network active status

```bash
shadowcoind getnetworkinfo
```

#### `addnode <ip:port> <add|remove|onetry>`
Manually control peer connections.

```bash
shadowcoind addnode 192.168.1.100:8333 add
shadowcoind addnode 192.168.1.100:8333 remove
```

#### `getconnectioncount`
Get number of connections to other nodes.

```bash
shadowcoind getconnectioncount
```

---

## RPC API Reference

### Connection

All RPC calls use JSON-RPC 1.0 over HTTP.

**Endpoint:** `http://localhost:8332/`

**Authentication:** Basic HTTP authentication using credentials from `shadowcoin.conf`

### Request Format

```json
{
  "jsonrpc": "1.0",
  "id": "curltest",
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
    "bestblockhash": "000000000019d6689c...",
    "difficulty": 1.0
  },
  "error": null,
  "id": "curltest"
}
```

### Example Calls

#### Using curl

```bash
curl --user rpcuser:rpcpass \
  --data-binary '{"jsonrpc":"1.0","id":"1","method":"getinfo","params":[]}' \
  -H 'content-type: text/plain;' \
  http://127.0.0.1:8332/
```

#### Using Python

```python
import requests
import json

def rpc_call(method, params=[]):
    url = "http://localhost:8332/"
    headers = {'content-type': 'text/plain'}
    auth = ('rpcuser', 'rpcpass')
    
    payload = {
        "jsonrpc": "1.0",
        "id": "python",
        "method": method,
        "params": params
    }
    
    response = requests.post(url, data=json.dumps(payload), 
                           headers=headers, auth=auth)
    return response.json()

# Get blockchain info
result = rpc_call("getblockchaininfo")
print(result['result'])
```

### Error Codes

- `-1` - Generic error
- `-3` - Invalid amount
- `-4` - Wallet error
- `-5` - Invalid address
- `-6` - Insufficient funds
- `-8` - Transaction rejected
- `-32600` - Invalid request
- `-32601` - Method not found

---

## Protocol Specification

### Consensus Rules

#### Block Validation
- Block size: Maximum 2MB
- Block time: Target 2 minutes (120 seconds)
- Difficulty adjustment: Every 144 blocks
- Maximum transactions per block: 10,000

#### Transaction Validation
- Minimum output: 0.00000001 SHD
- Maximum output: 21,000,000 SHD
- Transaction version: 1
- Signature algorithm: Ed25519

#### Block Reward Schedule
- Initial reward: 50 SHD
- Halving interval: 210,000 blocks (~8 months)
- Maximum supply: 21,000,000 SHD

### Proof-of-Work Algorithm

**Algorithm Name:** Shadowhash

**Steps:**
1. Serialize block header (80 bytes)
2. Compute SHA-256 hash
3. Use hash as seed for Argon2id (64MB memory, 4 iterations)
4. Extract 16 memory indices from Argon2id output
5. Read random 64-byte chunks from Argon2id memory at each index
6. XOR all chunks and hash with SHA-256
7. Compare result to target difficulty

**Parameters:**
- Memory: 64MB (65536 KB)
- Iterations: 4
- Parallelism: 1
- Hash length: 32 bytes

**CPU Optimization:**
- Uses L2/L3 cache effectively
- Memory latency dependent
- Branch prediction friendly

**GPU Resistance:**
- High memory bandwidth requirement
- Random access pattern
- Memory latency bottleneck

### Address Format

**Version Byte:** `0x3F` (produces "SHD" prefix)

**Derivation Path:** BIP44 standard `m/44'/0'/0'/0/0`

**Format:**
```
[version:1][public_key_hash:32][checksum:4]
```

**Encoding:** Base58Check

**Example:** `SHD1qxyz7abc...def`

**Seed to Address Flow:**
1. 24-word BIP39 seed phrase → 512-bit seed
2. BIP32 HD derivation → master private key
3. BIP44 path derivation → account private key (Ed25519)
4. Private key → public key (Ed25519)
5. Public key → SHA-256 → RIPEMD-160 → address hash
6. Version + hash + checksum → Base58Check encode → address

### Transaction Format

**Binary Structure:**
```c
struct Transaction {
    uint32_t version;        // Always 1
    uint32_t input_count;    // Number of inputs
    TxInput inputs[];        // Transaction inputs
    uint32_t output_count;   // Number of outputs
    TxOutput outputs[];      // Transaction outputs
    uint32_t lock_time;      // Block height or timestamp
};

struct TxInput {
    uint8_t prev_tx_hash[32];    // Previous transaction hash
    uint32_t prev_output_index;  // Output index in prev tx
    uint8_t signature[64];       // Ed25519 signature
    uint8_t public_key[32];      // Public key
};

struct TxOutput {
    uint64_t amount;             // Amount in satoshis (1e-8 SHD)
    uint8_t recipient_hash[32];  // Hash of recipient public key
};
```

---

## Mining Guide

### Hardware Requirements

**Minimum:**
- CPU: 2 cores
- RAM: 2GB
- Storage: 10GB
- Network: 1 Mbps

**Recommended:**
- CPU: 8+ cores with high L3 cache
- RAM: 8GB
- Storage: 50GB SSD
- Network: 10 Mbps

### CPU Selection

**Best Performance:**
- AMD Ryzen 9 / Threadripper (large L3 cache)
- Intel Core i9 / Xeon (high single-thread performance)
- ARM-based chips with large cache (Apple M-series)

**Why These Work Well:**
- Large L2/L3 cache reduces memory latency
- High single-thread performance for hashing
- Many cores for parallel attempts

### Solo Mining

```bash
# Start node
shadowcoind -daemon

# Enable mining with 8 threads
shadowcoind setgenerate true 8

# Monitor mining
watch -n 5 shadowcoind getmininginfo
```

### Mining Pool

Pool mining protocol documentation coming soon. Shadowcoin uses a simplified version of the Stratum protocol.

### Expected Hashrate

Approximate hashrates on modern CPUs:

- AMD Ryzen 9 5950X: ~2.5 kH/s
- Intel i9-12900K: ~2.2 kH/s
- Apple M2 Max: ~1.8 kH/s
- AMD Ryzen 5 5600X: ~1.0 kH/s

### Profitability Calculation

```
Daily Coins = (Your Hashrate / Network Hashrate) × Blocks Per Day × Block Reward
Network Hashrate = Difficulty × 2^32 / Block Time
Blocks Per Day = 86400 / 120 = 720
```

---

## Wallet Management

### Creating a Wallet

```bash
shadowcoind createwallet
```

This generates:
- 24-word BIP39 seed phrase (mnemonic)
- Private key (Ed25519, derived from seed)
- Public key
- Public address

**CRITICAL**: The seed phrase is displayed only once. Write it down immediately and store it securely offline. Anyone with your seed phrase can access your coins.

### Seed Phrase Security

Your 24-word seed phrase is the master key to your wallet:

✅ **DO:**
- Write it down on paper immediately
- Store in a fireproof safe or safety deposit box
- Consider metal backup solutions for fire/water protection
- Keep multiple copies in different secure locations
- Use a BIP39 passphrase for additional security (advanced)

❌ **DON'T:**
- Store it digitally (computer, phone, cloud, email, photos)
- Share it with anyone
- Enter it on websites or apps you don't trust
- Store all copies in the same location

### Wallet Recovery

If you lose access to your wallet.dat file, you can restore your wallet using your seed phrase:

```bash
shadowcoind recoverwallet
```

Enter your 24-word seed phrase when prompted. This will regenerate your private key and restore access to your coins.

**Recovery scenarios:**
- Lost or corrupted wallet.dat file
- Moving to a new computer
- Forgot wallet encryption password
- Hardware failure

### Backup and Recovery

**Method 1: Seed Phrase (Recommended)**

Your 24-word seed phrase is the most important backup. With it, you can recover your wallet on any device running Shadowcoin.

No file backup needed - just your written seed phrase.

**Method 2: Wallet File Backup**

Backup wallet.dat file:
```bash
shadowcoind backupwallet ~/shadowcoin-backup.dat
cp ~/.shadowcoin/wallet.dat ~/shadowcoin-backup.dat
```

Restore from wallet.dat:
```bash
shadowcoind stop
cp ~/shadowcoin-backup.dat ~/.shadowcoin/wallet.dat
shadowcoind -daemon
```

**Note:** Wallet file backups require remembering your encryption password. Seed phrase recovery works even if you forget your password.

### Wallet Encryption

**Encrypt:**
```bash
shadowcoind encryptwallet "strong passphrase here"
```

**Unlock for transactions:**
```bash
shadowcoind walletpassphrase "strong passphrase here" 300
```

**Lock immediately:**
```bash
shadowcoind walletlock
```

### Security Best Practices

1. **Write down your seed phrase** immediately when creating a wallet
2. **Store seed phrase offline** in multiple secure physical locations
3. **Never share** your seed phrase with anyone
4. **Encrypt your wallet** with a strong passphrase (protects wallet.dat, not seed)
5. **Backup regularly** using either seed phrase or wallet.dat file
6. **Verify addresses** before sending transactions
7. **Keep software updated** to latest version
8. **Use offline storage** (paper/metal seed phrase) for large amounts
9. **Test recovery** with small amounts before storing significant value
10. **Beware of phishing** - never enter seed phrase on websites or unknown apps

---

## Network Protocol

### Message Types

#### Version Handshake
```c
struct VersionMessage {
    uint32_t version;          // Protocol version
    uint64_t services;         // Node services
    uint64_t timestamp;        // Current time
    uint8_t user_agent[256];   // Client identifier
    uint32_t start_height;     // Current blockchain height
};
```

#### Block Propagation
```c
struct InvMessage {
    uint32_t count;
    struct {
        uint32_t type;         // 1=tx, 2=block
        uint8_t hash[32];
    } inventory[];
};
```

### Connection Flow

1. **TCP Connection** established on port 8333
2. **Version Exchange** - Both nodes send version messages
3. **Verack** - Acknowledge version compatibility
4. **GetAddr** - Request peer addresses
5. **GetBlocks** - Synchronize blockchain
6. **Inv/GetData** - Ongoing transaction/block propagation

### Peer Discovery

**Initial Bootstrap:**
- DNS seeds: `seed.shadowcoin.org`
- Hardcoded seed nodes in source
- Manual `-connect` or `-addnode` parameters

**Peer Exchange:**
- Nodes share peer lists via `addr` messages
- Persist known peers to `peers.dat`
- Automatic reconnection to reliable peers

---

## Building from Source

### Dependencies

**Required:**
- GCC 7.0+ or Clang 6.0+
- Make
- libsodium (for Ed25519 and Argon2id)
- BIP39 library (for seed phrase generation)

**Optional:**
- libmicrohttpd (for RPC server)
- Check framework (for unit tests)

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install build-essential libsodium-dev libmicrohttpd-dev check
git clone https://github.com/shadowcoin/shadowcoin.git
cd shadowcoin
make
sudo make install
```

### macOS

```bash
brew install libsodium libmicrohttpd check
git clone https://github.com/shadowcoin/shadowcoin.git
cd shadowcoin
make
sudo make install
```

### Windows (MinGW)

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-libsodium
git clone https://github.com/shadowcoin/shadowcoin.git
cd shadowcoin
make
```

### Build Options

```bash
# Debug build
make DEBUG=1

# Release build (default)
make

# With tests
make test

# Clean build
make clean && make
```

---

## Developer Guide

### Code Structure

```
src/
├── main.c              # Entry point, command routing
├── daemon.c            # Main event loop
├── blockchain/         # Consensus and validation
├── crypto/             # Cryptographic primitives
├── mining/             # PoW implementation
├── network/            # P2P networking
├── wallet/             # Key and UTXO management
├── rpc/                # JSON-RPC server
└── cli/                # Command-line parsing
```

### Building a Block

```c
Block* create_block(const uint8_t prev_hash[32], 
                    Transaction* txs, 
                    uint32_t tx_count) {
    Block* block = malloc(sizeof(Block));
    block->version = 1;
    memcpy(block->prev_hash, prev_hash, 32);
    block->timestamp = time(NULL);
    block->tx_count = tx_count;
    block->transactions = txs;
    
    // Calculate merkle root
    calculate_merkle_root(block);
    
    return block;
}
```

### Deriving Keys from Seed Phrase

```c
#include "bip39.h"
#include "bip32.h"

// Generate wallet from seed phrase
Wallet* create_wallet_from_seed(const char* seed_phrase) {
    // Convert BIP39 mnemonic to seed
    uint8_t seed[64];
    mnemonic_to_seed(seed_phrase, "", seed, NULL);
    
    // Derive master key using BIP32
    HDNode master;
    hdnode_from_seed(seed, 64, SECP256K1_NAME, &master);
    
    // Derive account key using BIP44 path: m/44'/0'/0'/0/0
    hdnode_private_ckd(&master, 44 | 0x80000000);  // Purpose
    hdnode_private_ckd(&master, 0 | 0x80000000);   // Coin type
    hdnode_private_ckd(&master, 0 | 0x80000000);   // Account
    hdnode_private_ckd(&master, 0);                 // Change
    hdnode_private_ckd(&master, 0);                 // Index
    
    // Extract Ed25519 key pair
    Wallet* wallet = malloc(sizeof(Wallet));
    memcpy(wallet->private_key, master.private_key, 32);
    ed25519_publickey(wallet->private_key, wallet->public_key);
    
    // Generate address
    generate_address(wallet->public_key, wallet->address);
    
    return wallet;
}
```

### Validating a Transaction

```c
bool validate_transaction(Transaction* tx) {
    // Check version
    if (tx->version != 1) return false;
    
    // Verify signatures on all inputs
    for (int i = 0; i < tx->input_count; i++) {
        if (!verify_signature(&tx->inputs[i])) {
            return false;
        }
    }
    
    // Check input amounts exist and aren't spent
    uint64_t input_sum = 0;
    for (int i = 0; i < tx->input_count; i++) {
        UTXO* utxo = find_utxo(tx->inputs[i].prev_tx_hash,
                               tx->inputs[i].prev_output_index);
        if (!utxo || utxo->spent) return false;
        input_sum += utxo->amount;
    }
    
    // Check outputs
    uint64_t output_sum = 0;
    for (int i = 0; i < tx->output_count; i++) {
        output_sum += tx->outputs[i].amount;
    }
    
    // Fee must be non-negative
    return input_sum >= output_sum;
}
```

### Testing

```bash
# Run all tests
make test

# Run specific test suite
./tests/blockchain_test
./tests/crypto_test
./tests/mining_test
```

### Contributing

1. Fork the repository
2. Create feature branch: `git checkout -b feature/my-feature`
3. Follow coding style (see STYLE.md)
4. Write tests for new functionality
5. Submit pull request

### Coding Standards

- Use C99 standard
- 4-space indentation
- 80-character line limit
- Document all public functions
- Include unit tests
- No compiler warnings

---

## Appendix

### Configuration File Reference

Complete `shadowcoin.conf` options:

```conf
# Network
port=8333
maxconnections=125
timeout=30
bind=0.0.0.0

# RPC
rpcport=8332
rpcuser=user
rpcpassword=pass
rpcbind=127.0.0.1
rpcallowip=127.0.0.1

# Mining
gen=0
genproclimit=4

# Blockchain
datadir=~/.shadowcoin
txindex=1
prune=0

# Wallet
keypool=100

# Debug
debug=0
logtimestamps=1
```

### Glossary

- **UTXO**: Unspent Transaction Output
- **Mempool**: Memory pool of unconfirmed transactions
- **Satoshi**: Smallest unit, 0.00000001 SHD
- **Orphan Block**: Valid block not in main chain
- **Difficulty**: Measure of how hard it is to mine a block
- **Seed Phrase**: 24-word BIP39 mnemonic that generates wallet keys
- **BIP39**: Bitcoin Improvement Proposal for mnemonic phrases
- **BIP32**: Hierarchical Deterministic wallet standard
- **BIP44**: Multi-account hierarchy for HD wallets
- **HD Wallet**: Hierarchical Deterministic wallet derived from single seed

### Further Reading

- Bitcoin Developer Documentation: https://developer.bitcoin.org
- Cryptocurrency Engineering: https://nakamotoinstitute.org
- Ed25519 Paper: https://ed25519.cr.yp.to/ed25519-20110926.pdf
- Argon2 RFC: https://www.rfc-editor.org/rfc/rfc9106.html

---

**Last Updated:** October 2025  
**Version:** 0.1.0  
**Maintainer:** Shadowcoin Development Team