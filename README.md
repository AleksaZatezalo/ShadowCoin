# Agoracoin

A lightweight cryptocurrency implementation designed for CPU-only mining.

## Overview

Agoracoin is a minimalist blockchain implementation written in pure C. It demonstrates the core concepts of cryptocurrency without the complexity of production systems. The design intentionally resists GPU optimization to keep mining accessible on standard CPUs.

Like Bitcoin Core, Agoracoin is delivered as a single binary (`Agoracoind`) that provides all functionality: full node operation, wallet management, mining, and network participation.

## Architecture

### Core Components

```
Agoracoin/
├── src/
│   ├── main.c                 # Entry point and command dispatcher
│   ├── daemon.c/h             # Node daemon and RPC server
│   ├── blockchain/
│   │   ├── block.c/h          # Block structure and validation
│   │   ├── chain.c/h          # Chain management and consensus
│   │   └── transaction.c/h    # Transaction handling
│   ├── crypto/
│   │   ├── hash.c/h           # SHA-256 + memory-hard extension
│   │   └── signature.c/h      # Ed25519 signatures
│   ├── mining/
│   │   ├── miner.c/h          # CPU mining logic
│   │   └── difficulty.c/h     # Difficulty adjustment
│   ├── network/
│   │   ├── peer.c/h           # P2P networking
│   │   └── protocol.c/h       # Message protocol
│   ├── wallet/
│   │   ├── wallet.c/h         # Key management
│   │   └── balance.c/h        # UTXO tracking
│   ├── rpc/
│   │   ├── server.c/h         # JSON-RPC server
│   │   └── commands.c/h       # RPC command handlers
│   └── cli/
│       └── parser.c/h         # Command-line interface
├── include/                    # Public headers
├── tests/                      # Unit tests
└── docs/                       # Additional documentation
```

## Key Features

### CPU-Only Mining
- Memory-hard proof-of-work algorithm (Argon2id-based)
- Random memory access patterns defeat GPU parallelization
- Configurable memory requirements (default: 64MB per hash)

### Lightweight Design
- Minimal dependencies (only standard C libraries + libsodium)
- Simple binary format for blocks and transactions
- Efficient UTXO set management
- Target block time: 2 minutes

### Core Functionality
- Ed25519 digital signatures for transactions
- UTXO transaction model
- Difficulty adjustment every 144 blocks (~4.8 hours)
- Maximum supply: 21 million coins
- Block reward halving every 210,000 blocks

## Building

```bash
# Install dependencies
sudo apt-get install libsodium-dev

# Build the single binary
make clean && make

# This produces: ./Agoracoind

# Run tests
make test
```

## Usage

The `Agoracoind` binary operates in different modes depending on the command provided.

### Running as a Full Node

Start the daemon to participate in the network:

```bash
# Start node with default settings
./Agoracoind

# Start with custom port
./Agoracoind -port=8333

# Start in background
./Agoracoind -daemon

# Connect to specific peers
./Agoracoind -connect=192.168.1.100:8333
```

### Wallet Operations

```bash
# Create new wallet (generates 24-word seed phrase)
./Agoracoind createwallet

# Recover wallet from seed phrase
./Agoracoind recoverwallet

# Get wallet address
./Agoracoind getaddress

# Check balance
./Agoracoind getbalance

# Send coins
./Agoracoind sendtoaddress <recipient-address> <amount>

# List transactions
./Agoracoind listtransactions
```

### Mining

```bash
# Start mining (requires running node)
./Agoracoind setgenerate true

# Mine with specific thread count
./Agoracoind setgenerate true 4

# Stop mining
./Agoracoind setgenerate false

# Get mining info
./Agoracoind getmininginfo
```

### Blockchain Queries

```bash
# Get blockchain info
./Agoracoind getblockchaininfo

# Get specific block
./Agoracoind getblock <block-hash>

# Get block by height
./Agoracoind getblockbyhash <height>

# Get transaction
./Agoracoind gettransaction <tx-hash>
```

### Node Control

```bash
# Stop the daemon
./Agoracoind stop

# Get peer info
./Agoracoind getpeerinfo

# Get network info
./Agoracoind getnetworkinfo
```

## Configuration

Configuration file: `~/.Agoracoin/Agoracoin.conf`

```conf
# Network
port=8333
maxconnections=125

# RPC Server
rpcport=8332
rpcuser=user
rpcpassword=pass

# Mining
gen=0              # Start mining on launch (0=no, 1=yes)
genproclimit=4     # Mining threads

# Blockchain
datadir=~/.Agoracoin/data
txindex=1          # Maintain full transaction index
```

## Technical Details

### Block Structure
```c
typedef struct {
    uint32_t version;
    uint8_t prev_hash[32];
    uint8_t merkle_root[32];
    uint32_t timestamp;
    uint32_t difficulty;
    uint32_t nonce;
    uint32_t memory_nonce[16];  // For memory-hard PoW
    uint32_t tx_count;
    Transaction *transactions;
} Block;
```

### Proof-of-Work Algorithm

The mining algorithm uses a hybrid approach:
1. Standard SHA-256(block_header || nonce)
2. Memory expansion using Argon2id with result as seed
3. Random memory reads (CPU cache resistant)
4. Final hash must be below target difficulty

This design makes GPU mining inefficient due to:
- High memory bandwidth requirements
- Random access patterns
- Memory latency sensitivity

### Transaction Format
- UTXO-based model
- Inputs reference previous transaction outputs
- Outputs specify amount and recipient public key
- Transaction fee = sum(inputs) - sum(outputs)

### RPC Interface

The daemon runs a JSON-RPC server for programmatic access:

```bash
# Example: Get blockchain info via RPC
curl --user user:pass --data-binary '{"jsonrpc":"1.0","id":"1","method":"getblockchaininfo","params":[]}' \
  http://localhost:8332/
```

## Development Roadmap

- [x] Core blockchain data structures
- [x] CPU-optimized mining algorithm
- [ ] Single binary architecture with command routing
- [ ] P2P network protocol
- [ ] Wallet implementation
- [ ] Transaction pool (mempool)
- [ ] JSON-RPC server
- [ ] CLI command interface
- [ ] Configuration file parsing
- [ ] Add Tail Emmisions of 1%
- [ ] Add Quadratic voting
- [ ] Add Transaction Compression
- [ ] Add Compact Blocks
- [ ] Add Treasury Based Funding

## Directory Structure

```
~/.Agoracoin/
├── Agoracoin.conf       # Configuration file
├── wallet.dat            # Encrypted wallet (derived from seed)
├── peers.dat             # Known peers
├── data/
│   ├── blocks/           # Block data
│   ├── chainstate/       # UTXO set
│   └── index/            # Transaction index
└── debug.log             # Log file
```

**Important**: Your 24-word seed phrase is shown only once during wallet creation. Write it down and store it securely offline. The seed phrase can recover your wallet even if `wallet.dat` is lost or corrupted.

## Contributing

This is an educational project. Contributions that maintain simplicity while improving correctness are welcome.

## License

MIT License - see LICENSE file for details

## Resources

- [Bitcoin Whitepaper](https://bitcoin.org/bitcoin.pdf)
- [Bitcoin Core](https://github.com/bitcoin/bitcoin)
- [Ed25519 Signatures](https://ed25519.cr.yp.to/)
- [Argon2 Specification](https://github.com/P-H-C/phc-winner-argon2)

---