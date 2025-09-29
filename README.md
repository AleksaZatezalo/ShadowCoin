# ShadowCoin

A privacy-focused cryptocurrency implemented in C, featuring ring signatures for transaction anonymity and an accessible mining algorithm.

## Overview

ShadowCoin is a decentralized digital currency that prioritizes user privacy through cryptographic ring signatures while maintaining an easy-to-mine consensus mechanism. The total supply is capped at 100 million coins.

## Key Features

- **Ring Signature Privacy**: Transactions use ring signatures to obscure the true sender among a group of possible signers
- **Easy Mining**: CPU-friendly proof-of-work algorithm accessible to everyday users
- **Fixed Supply**: Hard cap of 100,000,000 coins
- **Decentralized**: Peer-to-peer network with no central authority

## Technical Specifications

### Blockchain Parameters

- **Total Supply**: 100,000,000 coins
- **Block Time**: 120 seconds (2 minutes)
- **Block Reward**: Starts at 50 coins, halves every 1,050,000 blocks (~4 years)
- **Ring Size**: Minimum 11 decoys per transaction
- **Difficulty Adjustment**: Every 720 blocks (~24 hours)

### Privacy Features

- **Ring Signatures**: Each transaction input references multiple possible sources
- **Stealth Addresses**: One-time addresses for each transaction output
- **Ring Confidential Transactions (RingCT)**: Hides transaction amounts

### Mining Algorithm

- **Algorithm**: RandomX-lite variant (CPU-optimized)
- **Memory Requirement**: 256 MB (lower than Monero's RandomX)
- **Target**: Optimized for commodity hardware, resistant to ASIC development

## Architecture

```
shadowcoin/
├── src/
│   ├── core/
│   │   ├── blockchain.c/h      # Blockchain data structure and validation
│   │   ├── block.c/h           # Block structure and operations
│   │   ├── transaction.c/h     # Transaction structure and validation
│   │   └── mempool.c/h         # Transaction pool management
│   ├── crypto/
│   │   ├── ring_signature.c/h  # Ring signature implementation (Borromean/MLSAG)
│   │   ├── stealth.c/h         # Stealth address generation
│   │   ├── ringct.c/h          # Ring Confidential Transactions
│   │   ├── hash.c/h            # Cryptographic hash functions
│   │   └── keys.c/h            # Key generation and management
│   ├── mining/
│   │   ├── miner.c/h           # Mining logic and proof-of-work
│   │   ├── difficulty.c/h      # Difficulty adjustment algorithm
│   │   └── randomx_lite.c/h    # Custom PoW algorithm
│   ├── network/
│   │   ├── p2p.c/h             # Peer-to-peer networking
│   │   ├── protocol.c/h        # Network protocol definitions
│   │   └── sync.c/h            # Blockchain synchronization
│   ├── wallet/
│   │   ├── wallet.c/h          # Wallet operations
│   │   ├── address.c/h         # Address generation and parsing
│   │   └── balance.c/h         # Balance calculation with ring structure
│   ├── storage/
│   │   ├── database.c/h        # Blockchain storage (LMDB)
│   │   └── serialization.c/h   # Data serialization/deserialization
│   └── util/
│       ├── base58.c/h          # Base58 encoding for addresses
│       ├── varint.c/h          # Variable-length integer encoding
│       └── logger.c/h          # Logging system
├── include/
│   └── shadowcoin/             # Public header files
├── tests/
│   ├── test_ring_sig.c
│   ├── test_blockchain.c
│   ├── test_mining.c
│   └── test_network.c
├── deps/                       # External dependencies
│   ├── libsodium/              # Cryptographic primitives
│   └── lmdb/                   # Database backend
├── docs/
│   ├── WHITEPAPER.md
│   ├── PROTOCOL.md
│   └── API.md
├── Makefile
├── CMakeLists.txt
└── README.md
```

## Core Components

### 1. Ring Signature System

Implements **Multilayered Linkable Spontaneous Anonymous Group (MLSAG)** signatures:

- Allows signing on behalf of a group without revealing which member signed
- Prevents double-spending through key image linkability
- Configurable ring size (default: 11)

### 2. Blockchain Core

- **Chain validation**: Verifies proof-of-work, timestamps, and transaction validity
- **UTXO model**: Tracks unspent transaction outputs with ring structure
- **Reorganization handling**: Manages chain splits and resolves forks

### 3. Mining System

- **RandomX-lite**: Memory-hard, CPU-optimized algorithm
- **Dynamic difficulty**: Adjusts to maintain consistent block times
- **Solo and pool mining**: Supports both mining modes

### 4. Wallet

- **Hierarchical Deterministic (HD)**: Generates addresses from a seed
- **View and spend keys**: Separate keys for viewing and spending
- **Subaddresses**: Multiple addresses from a single seed for privacy

### 5. P2P Network

- **Gossip protocol**: Efficient block and transaction propagation
- **NAT traversal**: UPnP and hole-punching support
- **Peer discovery**: DNS seeds and peer exchange

## Building from Source

### Dependencies

- GCC or Clang (C11 support required)
- CMake 3.10+
- libsodium (cryptographic library)
- LMDB (Lightning Memory-Mapped Database)
- OpenSSL (optional, for TLS)

### Compilation

```bash
# Clone the repository
git clone https://github.com/yourusername/shadowcoin.git
cd shadowcoin

# Initialize submodules
git submodule update --init --recursive

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run tests
make test

# Install
sudo make install
```

## Usage

### Running a Node

```bash
# Start the daemon
shadowcoind --data-dir ~/.shadowcoin

# With custom settings
shadowcoind --data-dir ~/.shadowcoin --p2p-port 18080 --rpc-port 18081
```

### Mining

```bash
# Solo mining
shadowcoin-miner --daemon-host 127.0.0.1 --daemon-port 18081 --threads 4

# Pool mining
shadowcoin-miner --pool-address pool.example.com:3333 --wallet-address SHDxxx...
```

### Wallet Operations

```bash
# Create new wallet
shadowcoin-wallet create --file ~/mywallet.dat

# Check balance
shadowcoin-wallet balance --wallet-file ~/mywallet.dat

# Send transaction
shadowcoin-wallet send --wallet-file ~/mywallet.dat \
  --to SHDxxx... --amount 10.5 --ring-size 11
```

## Emission Schedule

```
Block Range          | Reward  | Total Emitted
---------------------|---------|---------------
0 - 1,049,999        | 50.0    | 52,500,000
1,050,000 - 2,099,999| 25.0    | 78,750,000
2,100,000 - 3,149,999| 12.5    | 91,875,000
3,150,000 - 4,199,999| 6.25    | 98,437,500
4,200,000+           | 3.125   | → 100,000,000
```

Final coins mined at approximately block 4,500,000 (~17 years).

## Security Considerations

### Privacy Guarantees

- **Sender anonymity**: Ring signatures hide the true sender
- **Receiver anonymity**: Stealth addresses prevent address reuse tracking
- **Amount confidentiality**: RingCT hides transaction amounts

### Known Limitations

- Ring signatures provide probabilistic anonymity (not absolute)
- Timing analysis may correlate transactions
- Network-level privacy requires Tor/I2P integration

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Development Guidelines

- Follow Linux kernel coding style
- Write unit tests for new features
- Document all public APIs
- Use valgrind to check for memory leaks

## License

This project is licensed under the BSD 3-Clause License - see [LICENSE](LICENSE) for details.

## Disclaimer

This is experimental software. Use at your own risk. Never invest more than you can afford to lose.

## Resources

- **Whitepaper**: [docs/WHITEPAPER.md](docs/WHITEPAPER.md)
- **Protocol Specification**: [docs/PROTOCOL.md](docs/PROTOCOL.md)
- **API Documentation**: [docs/API.md](docs/API.md)
- **Community**: [Discord/Telegram/Forum links]

## Acknowledgments

- Monero Project for ring signature inspiration
- RandomX authors for PoW algorithm concepts
- CryptoNote protocol designers

## Contact

- **Website**: https://shadowcoin.org
- **Email**: dev@shadowcoin.org
- **GitHub**: https://github.com/yourusername/shadowcoin