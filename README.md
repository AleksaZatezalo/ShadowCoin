# Simplecoin

A lightweight cryptocurrency implementation demonstrating blockchain fundamentals.

## Overview

Simplecoin is a minimalist blockchain implementation written in pure C. It demonstrates the core concepts of cryptocurrency without the complexity of production systems.

Like Bitcoin Core, Simplecoin is delivered as a single binary (`Simplecoind`) that provides all functionality: full node operation, wallet management, mining, and network participation.

## Architecture

### Core Components

```
Simplecoin/
├── src/
│   ├── main.c                 # Entry point and command dispatcher
│   ├── daemon.c/h             # Node daemon and RPC server
│   ├── blockchain/
│   │   ├── block.c/h          # Block structure and validation
│   │   ├── chain.c/h          # Chain management and consensus
│   │   └── transaction.c/h    # Transaction handling
│   ├── crypto/
│   │   ├── hash.c/h           # SHA-256 implementation
│   │   └── signature.c/h      # Ed25519 signatures
│   ├── mining/
│   │   ├── miner.c/h          # Mining logic
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

### Lightweight Design
- Minimal dependencies (only standard C libraries + libsodium)
- Simple binary format for blocks and transactions
- Efficient UTXO set management
- Target block time: 2 minutes

### Core Functionality
- Ed25519 digital signatures for transactions
- UTXO transaction model
- Difficulty adjustment every 144 blocks (~4.8 hours)
- Block reward halving every 210,000 blocks

## Building

```bash
# Install dependencies
sudo apt-get install libsodium-dev

# Build the single binary
make clean && make

# This produces: ./Simplecoind

# Run tests
make test
```

## Usage

The `Simplecoind` binary operates in different modes depending on the command provided.

### Running as a Full Node

Start the daemon to participate in the network:

```bash
# Start node with default settings
./Simplecoind

# Start with custom port
./Simplecoind -port=8333

# Start in background
./Simplecoind -daemon

# Connect to specific peers
./Simplecoind -connect=192.168.1.100:8333
```

### Wallet Operations

```bash
# Create new wallet (generates 24-word seed phrase)
./Simplecoind createwallet

# Recover wallet from seed phrase
./Simplecoind recoverwallet

# Get wallet address
./Simplecoind getaddress

# Check balance
./Simplecoind getbalance

# Send coins
./Simplecoind sendtoaddress <recipient-address> <amount>

# List transactions
./Simplecoind listtransactions
```

### Mining

```bash
# Start mining (requires running node)
./Simplecoind setgenerate true

# Mine with specific thread count
./Simplecoind setgenerate true 4

# Stop mining
./Simplecoind setgenerate false

# Get mining info
./Simplecoind getmininginfo
```

### Blockchain Queries

```bash
# Get blockchain info
./Simplecoind getblockchaininfo

# Get specific block
./Simplecoind getblock <block-hash>

# Get block by height
./Simplecoind getblockbyhash <height>

# Get transaction
./Simplecoind gettransaction <tx-hash>
```

### Node Control

```bash
# Stop the daemon
./Simplecoind stop

# Get peer info
./Simplecoind getpeerinfo

# Get network info
./Simplecoind getnetworkinfo
```

## Configuration

Configuration file: `~/.Simplecoin/Simplecoin.conf`

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
datadir=~/.Simplecoin/data
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
    uint32_t tx_count;
    Transaction *transactions;
} Block;
```

### Proof-of-Work Algorithm

The mining algorithm uses double SHA-256:

1. Construct block header with current nonce
2. Calculate `hash = SHA-256(SHA-256(block_header))`
3. Check if hash is below target difficulty
4. If not, increment nonce and repeat

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
- [x] SHA-256 based proof-of-work
- [ ] Single binary architecture with command routing
- [ ] P2P network protocol
- [ ] Wallet implementation
- [ ] Transaction pool (mempool)
- [ ] JSON-RPC server
- [ ] CLI command interface
- [ ] Configuration file parsing

## Directory Structure

```
~/.Simplecoin/
├── Simplecoin.conf       # Configuration file
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

---