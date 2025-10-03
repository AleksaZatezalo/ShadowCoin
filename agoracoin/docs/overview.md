# Agoracoin Technical Documentation

*Complete technical documentation for the Agoracoin cryptocurrency—where every voice matters in the digital agora.*

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Architecture Overview](#architecture-overview)
3. [Command Reference](#command-reference)
4. [RPC API Reference](#rpc-api-reference)
5. [Protocol Specification](#protocol-specification)
6. [Mining Guide](#mining-guide)
7. [Wallet Management](#wallet-management)
8. [Governance Guide](#governance-guide)
9. [Network Protocol](#network-protocol)
10. [Building from Source](#building-from-source)
11. [Developer Guide](#developer-guide)

---

## Getting Started

### What is Agoracoin?

Agoracoin is a cryptocurrency built on democratic principles, named after the ancient Greek agora—the public assembly where citizens gathered to discuss and decide the future of their city-state. Unlike traditional cryptocurrencies, Agoracoin features built-in quadratic funding for community governance, ensuring that every voice matters, not just the largest wallets.

### Installation

Download the latest release or build from source:

```bash
git clone https://github.com/agoracoin/agoracoin.git
cd agoracoin
make
sudo make install
```

### Quick Start

```bash
# Start the node
agoracoind -daemon

# Create a wallet
agoracoind createwallet

# Get your address
agoracoind getaddress

# Check balance
agoracoind getbalance

# Start mining
agoracoind setgenerate true 4

# View governance proposals
agoracoind listproposals
```

### Data Directory

Default locations:
- **Linux**: `~/.agoracoin/`
- **macOS**: `~/Library/Application Support/Agoracoin/`
- **Windows**: `%APPDATA%\Agoracoin\`

---

## Architecture Overview

### Component Hierarchy

```
agoracoind
├── Main Entry Point
│   └── Command Router
├── Daemon Mode
│   ├── Blockchain Manager
│   ├── Network Layer
│   ├── Mining Engine (SHA-256d)
│   ├── Wallet Manager
│   ├── Governance Engine
│   │   ├── Proposal Management
│   │   ├── Quadratic Funding Calculator
│   │   └── Treasury Management
│   └── RPC Server
└── CLI Mode
    └── RPC Client
```

### Data Flow

1. **Transaction Creation**: Wallet → Transaction Pool → Block Template
2. **Block Mining**: Mining Engine → SHA-256d PoW → Block Validation
3. **Block Propagation**: Network Layer → Peer Validation → Chain Integration
4. **Balance Updates**: Chain Integration → UTXO Set → Wallet Balance
5. **Governance**: Proposal Submission → Community Contributions → QF Matching → Treasury Payout

### Threading Model

- **Main Thread**: Event loop, RPC server
- **Network Thread**: P2P message handling, peer management
- **Mining Threads**: SHA-256d proof-of-work computation (configurable count)
- **Validation Thread**: Block and transaction verification
- **Governance Thread**: Proposal tracking and quadratic funding calculations

---

## Command Reference

### Node Control

#### `agoracoind`
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
agoracoind -daemon -port=8333 -maxconnections=50
```

#### `stop`
Shut down the daemon gracefully.

```bash
agoracoind stop
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
- Treasury balance

```bash
agoracoind getinfo
```

**Example output:**
```json
{
  "version": "0.1.0",
  "protocolversion": 1,
  "blocks": 126543,
  "connections": 12,
  "difficulty": 4.52e+08,
  "mining": true,
  "hashrate": 2250000,
  "treasury": "82345.67000000"
}
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
agoracoind createwallet
```

**Output:**
```
Seed Phrase (WRITE THIS DOWN AND STORE SECURELY):
abandon ability able about above absent absorb abstract absurd abuse access accident
account accuse achieve acid acoustic acquire across act action actor actress actual

Your Address: agora1qxyz...abc
Wallet created successfully.

WARNING: Write down your seed phrase and store it in a secure location.
This is the ONLY way to recover your wallet if wallet.dat is lost.
```

#### `recoverwallet`
Recover wallet from 24-word BIP39 seed phrase.

**Interactive prompt** will ask for seed phrase (space-separated words).

**Example:**
```bash
agoracoind recoverwallet
```

**Prompt:**
```
Enter your 24-word seed phrase:
> abandon ability able about above absent absorb abstract absurd abuse access accident account accuse achieve acid acoustic acquire across act action actor actress actual

Wallet recovered successfully.
Your Address: agora1qxyz...abc
```

#### `getaddress`
Get the current wallet's public address.

```bash
agoracoind getaddress
```

#### `getbalance`
Get confirmed wallet balance.

**Options:**
- `minconf=<n>` - Minimum confirmations (default: 1)

```bash
agoracoind getbalance
agoracoind getbalance minconf=6
```

**Example output:**
```
Balance: 1234.56000000 AGORA
```

#### `sendtoaddress <address> <amount>`
Send coins to an address.

**Parameters:**
- `address` - Recipient's public address
- `amount` - Amount in coins (decimal)

**Returns:**
- Transaction ID

```bash
agoracoind sendtoaddress agora1qxyz...abc 10.5
```

**Example output:**
```
Transaction ID: a1b2c3d4e5f6...
Transaction sent successfully.
```

#### `listtransactions`
List recent transactions.

**Options:**
- `count=<n>` - Number of transactions (default: 10)
- `skip=<n>` - Skip first n transactions (default: 0)

```bash
agoracoind listtransactions count=20
```

#### `backupwallet <destination>`
Backup wallet.dat file to specified location.

**Note:** The seed phrase is a more portable backup method. This command backs up the encrypted wallet file.

```bash
agoracoind backupwallet ~/agoracoin-backup.dat
```

#### `encryptwallet <passphrase>`
Encrypt wallet with passphrase.

⚠️ **Warning**: This requires daemon restart.

```bash
agoracoind encryptwallet "my secure passphrase"
```

#### `walletpassphrase <passphrase> <timeout>`
Unlock encrypted wallet for specified seconds.

```bash
agoracoind walletpassphrase "my secure passphrase" 300
```

#### `walletlock`
Lock encrypted wallet immediately.

```bash
agoracoind walletlock
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
- Treasury balance

```bash
agoracoind getblockchaininfo
```

#### `getblock <hash>`
Get block details by hash.

**Returns:**
- Block header information
- Transaction list
- Confirmations
- Miner reward
- Treasury reward

```bash
agoracoind getblock 000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f
```

#### `getblockbyhash <height>`
Get block by height number.

```bash
agoracoind getblockbyhash 12345
```

#### `getblockhash <height>`
Get block hash at specific height.

```bash
agoracoind getblockhash 100
```

#### `gettransaction <txid>`
Get transaction details.

```bash
agoracoind gettransaction a1b2c3d4...
```

#### `getmempoolinfo`
Get mempool (transaction pool) information.

**Returns:**
- Transaction count
- Memory usage
- Minimum fee rate

```bash
agoracoind getmempoolinfo
```

### Mining Commands

#### `setgenerate <true|false> [threads]`
Enable or disable mining.

**Parameters:**
- `true|false` - Enable/disable mining
- `threads` - Number of mining threads (optional)

```bash
agoracoind setgenerate true 4
agoracoind setgenerate false
```

#### `getmininginfo`
Get mining statistics.

**Returns:**
- Mining status
- Hashrate
- Difficulty
- Block height
- Network hashrate estimate
- Next reward amount
- Treasury allocation

```bash
agoracoind getmininginfo
```

**Example output:**
```json
{
  "blocks": 126543,
  "currentblocksize": 1234,
  "difficulty": 452000000,
  "networkhashps": 15000000000,
  "pooledtx": 23,
  "chain": "main",
  "generate": true,
  "genproclimit": 4,
  "hashespersec": 2250000,
  "reward": "9.00000000",
  "treasury": "1.00000000"
}
```

#### `getdifficulty`
Get current mining difficulty.

```bash
agoracoind getdifficulty
```

#### `getblocktemplate`
Get block template for mining.

```bash
agoracoind getblocktemplate
```

#### `submitblock <hexdata>`
Submit mined block.

```bash
agoracoind submitblock 0000002000...
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
agoracoind getpeerinfo
```

#### `getnetworkinfo`
Get network status.

**Returns:**
- Protocol version
- Local services
- Connection count
- Network active status

```bash
agoracoind getnetworkinfo
```

#### `addnode <ip:port> <add|remove|onetry>`
Manually control peer connections.

```bash
agoracoind addnode 192.168.1.100:8333 add
agoracoind addnode 192.168.1.100:8333 remove
```

#### `getconnectioncount`
Get number of connections to other nodes.

```bash
agoracoind getconnectioncount
```

### Governance Commands

#### `stakeforgov <amount>`
Stake coins for governance participation (required 30 days before contributing to proposals).

**Parameters:**
- `amount` - Amount to stake in AGORA

**Note:** Staked coins remain yours and can be unstaked after use. The 30-day lock prevents Sybil attacks.

```bash
agoracoind stakeforgov 1000
```

**Example output:**
```
Staked 1000.00000000 AGORA for governance participation.
You can contribute to proposals starting 2026-02-15.
```

#### `unstakeforgov <amount>`
Unstake coins from governance.

```bash
agoracoind unstakeforgov 500
```

#### `getgovstake`
Get current governance stake information.

**Returns:**
- Total staked amount
- Stake date
- Eligible contribution date
- Participation history

```bash
agoracoind getgovstake
```

#### `submitproposal`
Submit a governance proposal.

**Interactive mode** prompts for all details:
- Title (max 256 characters)
- Description (max 2048 characters)
- Recipient address
- Requested amount
- Milestones (optional)

**Deposit:** 100 AGORA (refunded if proposal receives >1% of round donations)

```bash
agoracoind submitproposal
```

**Non-interactive mode:**
```bash
agoracoind submitproposal \
  --title "Core wallet GUI development" \
  --description "Build cross-platform GUI using Dear ImGui" \
  --recipient agora1a2b3c4d5e6f... \
  --amount 5000 \
  --milestone-1 "Basic wallet interface" \
  --milestone-2 "Transaction history view"
```

**Example output:**
```
Proposal submitted successfully.
Proposal ID: 0x7f3a9b2c...
Deposit: 100.00000000 AGORA
Round: Q1 2026
Contribution period: March 1-31, 2026
```

#### `listproposals [round]`
List active or past proposals.

**Options:**
- `round` - Specific round number (default: current round)
- `status` - Filter by status: active, closed, funded

```bash
agoracoind listproposals
agoracoind listproposals round=4
agoracoind listproposals status=active
```

**Example output:**
```
Q1 2026 Funding Round (Active)
Matching Pool: 65,700.00000000 AGORA
Proposals: 8

ID: 0x7f3a9b... | Core GUI | 234 contributors | 1,247 AGORA raised
ID: 0x8g4b0c... | Mobile wallet | 189 contributors | 892 AGORA raised
ID: 0x9h5c1d... | Block explorer | 156 contributors | 634 AGORA raised
...
```

#### `getproposal <proposal_id>`
Get detailed information about a proposal.

```bash
agoracoind getproposal 0x7f3a9b...
```

**Example output:**
```json
{
  "id": "0x7f3a9b2c...",
  "title": "Core wallet GUI development",
  "description": "Build cross-platform GUI using Dear ImGui...",
  "recipient": "agora1a2b3c4d5e6f...",
  "amount_requested": "5000.00000000",
  "round": 4,
  "status": "active",
  "contributors": 234,
  "amount_raised": "1247.50000000",
  "estimated_match": "18234.00000000",
  "milestones": [
    "Basic wallet interface",
    "Transaction history view"
  ]
}
```

#### `contribute <proposal_id> <amount>`
Contribute to a proposal.

**Requirements:**
- Must have staked coins 30+ days ago
- Maximum 100 AGORA per address per round
- Minimum 0.1 AGORA

```bash
agoracoind contribute 0x7f3a9b... 10
```

**Example output:**
```
Contributed 10.00000000 AGORA to proposal 0x7f3a9b...
Your contribution weight: 0.75 (75% - participating for 3 rounds)
Effective contribution: 7.50000000 AGORA
Thank you for participating in the agora!
```

#### `getroundinfo [round]`
Get information about a funding round.

```bash
agoracoind getroundinfo
agoracoind getroundinfo 4
```

**Example output:**
```json
{
  "round": 4,
  "quarter": "Q1 2026",
  "status": "active",
  "proposal_period": "2026-02-01 to 2026-02-28",
  "contribution_period": "2026-03-01 to 2026-03-31",
  "matching_pool": "65700.00000000",
  "proposals_submitted": 8,
  "total_contributed": "8234.50000000",
  "unique_contributors": 567,
  "estimated_total_funding": "94567.25000000"
}
```

#### `gettreasurybalance`
Get current treasury balance.

**Returns:**
- Total treasury balance
- Allocated funds (committed to proposals)
- Available funds (for next round)
- Historical disbursements

```bash
agoracoind gettreasurybalance
```

**Example output:**
```json
{
  "total": "145678.90000000",
  "allocated": "65700.00000000",
  "available": "79978.90000000",
  "lifetime_disbursed": "234567.80000000",
  "rounds_completed": 3
}
```

---

## RPC API Reference

### Connection

All RPC calls use JSON-RPC 1.0 over HTTP.

**Endpoint:** `http://localhost:8332/`

**Authentication:** Basic HTTP authentication using credentials from `agoracoin.conf`

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
    "difficulty": 452000000,
    "treasury": "145678.90000000"
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

# Submit proposal
proposal_data = {
    "title": "Mobile wallet development",
    "description": "Native iOS and Android wallets",
    "recipient": "agora1xyz...",
    "amount": 10000
}
result = rpc_call("submitproposal", [proposal_data])
print(f"Proposal ID: {result['result']['proposal_id']}")
```

#### Using JavaScript (Node.js)

```javascript
const axios = require('axios');

async function rpcCall(method, params = []) {
  const response = await axios.post('http://localhost:8332/', {
    jsonrpc: '1.0',
    id: 'nodejs',
    method: method,
    params: params
  }, {
    auth: {
      username: 'rpcuser',
      password: 'rpcpass'
    }
  });
  
  return response.data.result;
}

// Get current funding round
const roundInfo = await rpcCall('getroundinfo');
console.log(roundInfo);
```

### Error Codes

- `-1` - Generic error
- `-3` - Invalid amount
- `-4` - Wallet error
- `-5` - Invalid address
- `-6` - Insufficient funds
- `-7` - Governance error (not staked, wrong round, etc.)
- `-8` - Transaction rejected
- `-32600` - Invalid request
- `-32601` - Method not found

---

## Protocol Specification

### Consensus Rules

#### Block Validation
- **Block size**: Maximum 2MB
- **Block time**: Target 2 minutes (120 seconds)
- **Difficulty adjustment**: Every 1,008 blocks (~33.6 hours)
- **Maximum transactions per block**: 10,000
- **Block reward split**: 90% miner, 10% treasury

#### Transaction Validation
- **Minimum output**: 0.00000001 AGORA
- **Maximum output**: 21,000,000 AGORA
- **Transaction version**: 1 (normal), 2 (governance)
- **Signature algorithm**: Ed25519

#### Block Reward Schedule
- **Initial reward**: 10 AGORA (9 to miner, 1 to treasury)
- **Halving interval**: 525,600 blocks (~2 years)
- **Tail emission**: 1% annual after final halving
- **Maximum initial supply**: ~10.5M AGORA
- **Supply with tail emission**: Gradually increasing at 1%/year

### Proof-of-Work Algorithm

**Algorithm:** SHA-256d (double SHA-256)

**Identical to Bitcoin:**
```c
void hash_block_header(BlockHeader *header, uint8_t *output) {
    uint8_t first_hash[32];
    SHA256((uint8_t*)header, sizeof(BlockHeader), first_hash);
    SHA256(first_hash, 32, output);
}

bool check_proof_of_work(uint8_t *hash, uint32_t difficulty) {
    uint256 target = difficulty_to_target(difficulty);
    return uint256_compare(hash, target) < 0;
}
```

**Target Block Time:** 2 minutes

**Difficulty Adjustment:**
- Every 1,008 blocks
- Retarget to maintain 2-minute average
- Maximum adjustment: 4x in either direction

### Address Format

**Version Byte:** `0x1C` (produces "agora" prefix in Base58)

**Derivation Path:** BIP44 standard `m/44'/888'/0'/0/0` (coin type 888 for Agoracoin)

**Format:**
```
[version:1][public_key_hash:32][checksum:4]
```

**Encoding:** Base58Check

**Example:** `agora1qxyz7abc...def`

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
    uint32_t version;        // 1=normal, 2=governance
    uint32_t input_count;    // Number of inputs
    TxInput inputs[];        // Transaction inputs
    uint32_t output_count;   // Number of outputs
    TxOutput outputs[];      // Transaction outputs
    uint32_t lock_time;      // Block height or timestamp
    
    // For governance transactions (version=2)
    GovernanceData* gov_data; // Proposal or contribution data
};

struct TxInput {
    uint8_t prev_tx_hash[32];    // Previous transaction hash
    uint32_t prev_output_index;  // Output index in prev tx
    uint8_t signature[64];       // Ed25519 signature
    uint8_t public_key[32];      // Public key
};

struct TxOutput {
    uint64_t amount;             // Amount in satoshis (1e-8 AGORA)
    uint8_t recipient_hash[32];  // Hash of recipient public key
};

// Governance transaction types
struct GovernanceData {
    uint8_t type;                // 1=proposal, 2=contribution, 3=stake
    uint8_t data[2048];          // Type-specific data
};
```

### Quadratic Funding Protocol

**On-Chain Proposal Storage:**
```c
struct Proposal {
    uint8_t id[32];              // SHA-256 of proposal content
    uint32_t round;              // Funding round number
    char title[256];
    char description[2048];
    uint8_t recipient[32];       // Recipient address hash
    uint64_t amount_requested;
    uint64_t deposit;            // 100 AGORA deposit
    uint8_t submitter[32];       // Submitter address
    uint32_t submit_time;
    uint32_t contributor_count;
    uint64_t total_contributed;
};
```

**Contribution Tracking:**
```c
struct Contribution {
    uint8_t proposal_id[32];
    uint8_t contributor[32];
    uint64_t amount;
    double weight;               // Based on stake age and history
    uint32_t timestamp;
};
```

**Matching Calculation (End of Round):**
```c
// For each proposal, calculate quadratic funding match
double calculate_qf_match(Proposal *p, Contribution *contribs, uint64_t pool) {
    double sum_sqrt = 0.0;
    
    for (int i = 0; i < p->contributor_count; i++) {
        // Apply weight (50-100% based on participation history)
        double weighted_amount = contribs[i].amount * contribs[i].weight;
        double amount_in_coins = weighted_amount / COIN;
        sum_sqrt += sqrt(amount_in_coins);
    }
    
    // QF formula: (sum of square roots)^2
    double match_share = sum_sqrt * sum_sqrt;
    
    // Distribute pool proportionally across all proposals
    return match_share; // Normalized later by total of all matches
}
```

---

## Mining Guide

### Why Mine Agoracoin?

Unlike many modern cryptocurrencies, Agoracoin uses SHA-256d—the same algorithm as Bitcoin. This means:

- **Established hardware market**: Use existing Bitcoin ASICs
- **Predictable economics**: No algorithm changes or forks
- **Higher security**: More hashrate = more secure network
- **Merge mining potential**: Can potentially merge mine with Bitcoin in the future

### Hardware Requirements

**ASIC Mining (Recommended for serious miners):**
- Any SHA-256 ASIC (e.g., Antminer S19, Whatsminer M30S)
- Expected hashrate: 50-110 TH/s per unit
- Power consumption: 2,000-3,500W per unit
- Pool mining strongly recommended

**CPU/GPU Mining (Hobbyist, low profitability):**
- CPU: Any modern processor
- Expected hashrate: ~2-5 MH/s per CPU core
- Not economically viable compared to ASICs
- Good for learning and supporting the network

### Solo Mining (Small Scale)

For hobbyists and network supporters:

```bash
# Start node
agoracoind -daemon

# Enable mining with 8 threads
agoracoind setgenerate true 8

# Monitor mining
watch -n 5 agoracoind getmininginfo
```

**Expected Block Time (Solo Mining):**
```
Network hashrate: 1 PH/s (example)
Your hashrate: 10 MH/s (8-core CPU)
Your percentage: 0.000001%
Expected time to block: ~14,000 days

Conclusion: Join a pool instead!
```

### Pool Mining

**Pool Protocol:** Stratum (compatible with Bitcoin mining pools)

**Configuration:**
```conf
# agoracoin.conf
pool.url=stratum+tcp://pool.agoracoin.org:3333
pool.user=agora1youraddress
pool.password=x
```

**Popular Pools** (hypothetical, for documentation):
- `pool.agoracoin.org` - Official pool, 1% fee
- `agora.miningpool.io` - Community pool, 0.5% fee
- `solo.agoracoin.org` - Solo mining pool (no fee, irregular payouts)

### Profitability Calculation

```
Block Reward: 10 AGORA (9 to miners, 1 to treasury)
Block Time: 2 minutes
Blocks per Day: 720
Daily Network Reward: 6,480 AGORA

Your Daily Earnings = (Your Hashrate / Network Hashrate) × 6,480 AGORA

Example:
- Your hashrate: 50 TH/s (one Antminer S19)
- Network hashrate: 1 PH/s
- Your percentage: 0.005%
- Daily earnings: 6,480 × 0.00005 = 0.324 AGORA/day
- At $1/AGORA: $0.32/day
- At $10/AGORA: $3.24/day
- Power cost (3.5 kW × 24h × $0.10/kWh): $8.40/day
- Net: Negative at current prices (early stage)
```

**Note:** Mining profitability depends heavily on AGORA price and network hashrate. Early miners benefit from lower difficulty.

### ASIC Configuration

Most Bitcoin SHA-256 ASICs work out of the box with Agoracoin:

1. **Set Pool URL:** `stratum+tcp://pool.agoracoin.org:3333`
2. **Set Worker Name:** Your Agoracoin address
3. **Set Password:** Typically just "x" or "password"
4. **Optional:** Set difficulty target (consult pool documentation)

---

## Wallet Management

### Creating a Wallet

```bash
agoracoind createwallet
```

This generates:
- 24-word BIP39 seed phrase (mnemonic)
- Private key (Ed25519, derived from seed)
- Public key
- Public address (starts with "agora1")

**CRITICAL**: The seed phrase is displayed only once. Write it down immediately and store it securely offline. Anyone with your seed phrase can access your coins.

### Seed Phrase Security

Your 24-word seed phrase is the master key to your wallet and your voice in the agora:

✅ **DO:**
- Write it down on paper immediately
- Store in a fireproof safe or safety deposit box
- Consider metal backup solutions for fire/water protection
- Keep multiple copies in different secure locations
- Use a BIP39 passphrase for additional security (advanced)
- Test recovery with small amounts before storing large value

❌ **DON'T:**
- Store it digitally (computer, phone, cloud, email, photos)
- Share it with anyone (not even support—we'll never ask)
- Enter it on websites or apps you don't trust
- Store all copies in the same location
- Take photos of it
- Store it in password managers (they can be hacked)

### Wallet Recovery

If you lose access to your wallet.dat file, you can restore your wallet using your seed phrase:

```bash
agoracoind recoverwallet
```

Enter your 24-word seed phrase when prompted. This will regenerate your private key and restore access to your coins and governance participation.

**Recovery scenarios:**
- Lost or corrupted wallet.dat file
- Moving to a new computer
- Forgot wallet encryption password
- Hardware failure
- Want to access wallet on multiple devices

### Backup Strategies

**Method 1: Seed Phrase (Recommended)**

Your 24-word seed phrase is the most important backup. With it, you can recover your wallet on any device running Agoracoin.

**Metal plate storage example:**
- Product: Cryptosteel, Billfodl, or similar
- Engrave/stamp your 24 words
- Store in fireproof location
- Survives: fire, flood, electromagnetic interference

**Method 2: Wallet File Backup**

Backup wallet.dat file:
```bash
agoracoind backupwallet ~/agoracoin-backup.dat
# Or manually:
cp ~/.agoracoin/wallet.dat ~/agoracoin-backup.dat
```

Restore from wallet.dat:
```bash
agoracoind stop
cp ~/agoracoin-backup.dat ~/.agoracoin/wallet.dat
agoracoind -daemon
```

**Note:** Wallet file backups require remembering your encryption password. Seed phrase recovery works even if you forget your password.

### Wallet Encryption

**Encrypt (do this immediately after creating wallet):**
```bash
agoracoind encryptwallet "strong passphrase here"
```

The daemon will shut down. Restart it:
```bash
agoracoind -daemon
```

**Unlock for transactions:**
```bash
# Unlock for 5 minutes
agoracoind walletpassphrase "strong passphrase here" 300
```

**Lock immediately:**
```bash
agoracoind walletlock
```

**Security note:** Even with encryption, anyone with your seed phrase can recreate your wallet. Encryption only protects the wallet.dat file.

### Multi-Device Wallets

You can use the same wallet on multiple devices:

1. Create wallet on Device A
2. Write down seed phrase
3. On Device B: `agoracoind recoverwallet`
4. Enter seed phrase

**Both devices now have the same wallet.** Transactions from either device will affect the same balance.

**Warning:** This doesn't sync transaction history or labels. Each device maintains its own copy of the blockchain.

### Security Best Practices

1. **Write down your seed phrase** immediately when creating a wallet
2. **Store seed phrase offline** in multiple secure physical locations
3. **Never share** your seed phrase with anyone
4. **Encrypt your wallet** with a strong passphrase
5. **Backup regularly** using either seed phrase or wallet.dat file
6. **Verify addresses** before sending transactions (double-check character by character)
7. **Keep software updated** to latest version
8. **Use offline storage** (paper/metal seed phrase) for large amounts
9. **Test recovery** with small amounts before storing significant value
10. **Beware of phishing** - never enter seed phrase on websites or unknown apps
11. **Secure your computer** - antivirus, firewall, regular updates
12. **Consider hardware wallets** (if/when Agoracoin support is added)

---

## Governance Guide

### Understanding Quadratic Funding

Agoracoin's treasury is managed through **quadratic funding**, a mechanism that amplifies the voices of the many over the few. This aligns with the ancient Greek agora, where every citizen had a voice in governance.

**Why Quadratic?**

Linear funding (1 person × $100 = same as 100 people × $1) gives whales complete control.

Quadratic funding (√contributions) means:
- 100 people × 1 AGORA = (√1)×100 = **100² = 10,000 match**
- 1 person × 100 AGORA = (√100) = **10² = 100 match**

The many matter more than the whale!

### Funding Round Schedule

**Quarterly Cycle:**

| Quarter | Proposal Period | Contribution Period | Payout |
|---------|----------------|---------------------|--------|
| Q1 | Feb 1-28 | Mar 1-31 | Apr 1 |
| Q2 | May 1-31 | Jun 1-30 | Jul 1 |
| Q3 | Aug 1-31 | Sep 1-30 | Oct 1 |
| Q4 | Nov 1-30 | Dec 1-31 | Jan 1 |

**Annual Treasury:** ~262,800 AGORA/year
**Per-Round Pool:** ~65,700 AGORA/quarter

### How to Participate

#### Step 1: Stake for Governance

You must stake coins 30 days before contributing to prevent Sybil attacks:

```bash
# Stake 1000 AGORA
agoracoind stakeforgov 1000
```

**Important:**
- Staked coins remain yours and remain in your wallet
- You can unstake them anytime after use
- Staking just proves you're a long-term community member
- You need to stake at least 30 days before the contribution period

**Example Timeline:**
- Feb 1: Stake coins
- Mar 1: Earliest you can contribute (30 days later)
- Mar 31: Last day to contribute to Q1 round

#### Step 2: Review Proposals

```bash
# List current proposals
agoracoind listproposals

# Get detailed info on a proposal
agoracoind getproposal 0x7f3a9b...
```

**Evaluate proposals based on:**
- Clear milestones and deliverables
- Team competence and track record
- Benefit to the Agoracoin ecosystem
- Realistic budget and timeline
- Community support (current contributors)

#### Step 3: Contribute

```bash
# Contribute 10 AGORA to a proposal
agoracoind contribute 0x7f3a9b... 10
```

**Contribution Limits:**
- Minimum: 0.1 AGORA
- Maximum: 100 AGORA per address per round
- Must have staked 30+ days ago

**Your contribution weight:**
- New participant (first round): 50% weight
- After 1 year (4 rounds): 100% weight
- Example: 10 AGORA contribution with 75% weight = 7.5 effective AGORA

#### Step 4: Track Results

After the contribution period ends:

```bash
# See round results
agoracoind getroundinfo 4
```

Matching funds are automatically calculated and distributed on payout day (1st of next quarter).

### Creating a Proposal

**Requirements:**
- 100 AGORA deposit (refunded if >1% of round donations received)
- Clear project description
- Specific milestones
- Reasonable budget
- Recipient address

**Interactive submission:**
```bash
agoracoind submitproposal
```

**Non-interactive:**
```bash
agoracoind submitproposal \
  --title "Mobile Wallet Development" \
  --description "Native iOS and Android wallets with full feature parity to desktop client. Includes biometric authentication, QR code scanning, push notifications for transactions, and governance participation." \
  --recipient agora1abc... \
  --amount 15000 \
  --milestone-1 "iOS wallet beta (month 1-2)" \
  --milestone-2 "Android wallet beta (month 2-3)" \
  --milestone-3 "Production release and app store listings (month 4)"
```

**Tips for successful proposals:**
1. **Be specific:** Clear deliverables, timeline, budget breakdown
2. **Show competence:** Links to previous work, GitHub profile, portfolio
3. **Engage early:** Discuss in community forums before submitting
4. **Set realistic goals:** Under-promise, over-deliver
5. **Milestone-based:** Break large projects into verifiable milestones
6. **Community benefit:** Explain how this helps all Agoracoin users
7. **Open source:** Public goods get more support than private projects

### Proposal Guidelines

**Encouraged:**
- Core protocol improvements
- Wallet and user interface development
- Documentation and educational content
- Block explorers and analytics tools
- Exchange integrations
- Marketing and community building
- Security audits
- Mining pool software
- Libraries and SDKs

**Discouraged (unlikely to get funding):**
- Personal expenses unrelated to Agoracoin
- Vague or unclear deliverables
- Projects without public benefit
- Duplicate work that already exists
- Unrealistic timelines or budgets

### Example Funding Round

**Q1 2026 Round Results:**

| Proposal | Type | Contributors | Direct | Match | Total | % |
|----------|------|--------------|--------|-------|-------|---|
| Core GUI | Infrastructure | 234 | 1,247 | 18,234 | 19,481 | 27.8% |
| Mobile Wallet | UX | 189 | 892 | 14,503 | 15,395 | 22.1% |
| Block Explorer | Tools | 156 | 634 | 11,826 | 12,460 | 18.0% |
| Documentation | Education | 98 | 423 | 6,507 | 6,930 | 9.9% |
| Mining Pool | Infrastructure | 45 | 892 | 4,143 | 5,035 | 6.3% |
| Website Redesign | Marketing | 34 | 234 | 2,811 | 3,045 | 4.3% |
| Analytics Dashboard | Tools | 23 | 178 | 2,376 | 2,554 | 3.6% |
| Podcast Series | Education | 18 | 145 | 1,897 | 2,042 | 2.9% |

**Total Pool:** 65,700 AGORA
**Total Distributed:** 66,942 AGORA (includes direct contributions)

**Key Insights:**
- "Core GUI" with most contributors (234) got 27.8% despite having only moderate direct funding
- "Mining Pool" with high direct funding (892) but few contributors (45) got only 6.3%
- This demonstrates quadratic funding's power: broad support matters more than big donors

---

## Network Protocol

### Message Types

#### Version Handshake
```c
struct VersionMessage {
    uint32_t version;          // Protocol version (current: 1)
    uint64_t services;         // Node services (1=network, 2=mining, 4=governance)
    uint64_t timestamp;        // Current time
    uint8_t user_agent[256];   // Client identifier (e.g., "agoracoind/0.1.0")
    uint32_t start_height;     // Current blockchain height
};
```

#### Block Propagation
```c
struct InvMessage {
    uint32_t count;
    struct {
        uint32_t type;         // 1=tx, 2=block, 3=governance
        uint8_t hash[32];
    } inventory[];
};

struct GetDataMessage {
    uint32_t count;
    struct {
        uint32_t type;
        uint8_t hash[32];
    } requests[];
};
```

#### Governance Messages
```c
struct ProposalAnnounce {
    uint8_t proposal_id[32];
    uint32_t round;
    // Full proposal data follows
};

struct ContributionAnnounce {
    uint8_t proposal_id[32];
    uint8_t contributor[32];
    uint64_t amount;
    // Signature follows
};
```

### Connection Flow

1. **TCP Connection** established on port 8333
2. **Version Exchange** - Both nodes send version messages
3. **Verack** - Acknowledge version compatibility
4. **GetAddr** - Request peer addresses
5. **GetBlocks** - Synchronize blockchain
6. **GetProposals** - Synchronize active governance proposals
7. **Inv/GetData** - Ongoing transaction/block/proposal propagation

### Peer Discovery

**Initial Bootstrap:**
- DNS seeds: `seed.agoracoin.org`, `seed.agoracoin.io`, `seed.agoracoin.net`
- Hardcoded seed nodes in source
- Manual `-connect` or `-addnode` parameters

**DNS Seed Protocol:**
```bash
# Query DNS seed
dig seed.agoracoin.org A

# Returns list of IP addresses
45.76.132.89
104.238.145.76
149.28.177.234
207.148.85.91
```

**Peer Exchange:**
- Nodes share peer lists via `addr` messages
- Persist known peers to `peers.dat`
- Automatic reconnection to reliable peers
- Prefer peers with governance capabilities

### Port Configuration

| Service | Default Port | Purpose |
|---------|--------------|---------|
| P2P Network | 8333 | Blockchain sync, transactions |
| RPC Server | 8332 | JSON-RPC API |
| Mining Stratum | 3333 | Pool mining protocol |

---

## Building from Source

### Dependencies

**Required:**
- GCC 7.0+ or Clang 6.0+
- Make or CMake
- libsodium 1.0.18+ (for Ed25519 and Argon2id)
- OpenSSL 1.1+ (for SHA-256 acceleration)

**Optional:**
- libmicrohttpd (for RPC server)
- Check framework (for unit tests)
- Doxygen (for documentation generation)

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install build-essential libsodium-dev libssl-dev libmicrohttpd-dev check
git clone https://github.com/agoracoin/agoracoin.git
cd agoracoin
make
sudo make install
```

### macOS

```bash
brew install libsodium openssl libmicrohttpd check
git clone https://github.com/agoracoin/agoracoin.git
cd agoracoin
make
sudo make install
```

### Windows (MinGW)

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-libsodium mingw-w64-x86_64-openssl
git clone https://github.com/agoracoin/agoracoin.git
cd agoracoin
make
```

### Build Options

```bash
# Debug build with symbols
make DEBUG=1

# Release build with optimizations (default)
make

# Build with tests
make test

# Clean build
make clean && make

# Install to system
sudo make install

# Uninstall
sudo make uninstall
```

### Cross-Compilation

```bash
# For ARM (Raspberry Pi)
make CROSS_COMPILE=arm-linux-gnueabihf-

# For Windows from Linux
make CROSS_COMPILE=x86_64-w64-mingw32-
```

---

## Developer Guide

### Code Structure

```
src/
├── main.c              # Entry point, command routing
├── daemon.c            # Main event loop, service coordination
├── blockchain/
│   ├── block.c/h       # Block structure and validation
│   ├── chain.c/h       # Chain management, reorganization
│   ├── transaction.c/h # Transaction validation and UTXO
│   └── validation.c/h  # Consensus rules
├── crypto/
│   ├── hash.c/h        # SHA-256d implementation
│   ├── signature.c/h   # Ed25519 signatures (via libsodium)
│   └── merkle.c/h      # Merkle tree calculations
├── mining/
│   ├── miner.c/h       # SHA-256d mining loops
│   ├── difficulty.c/h  # Difficulty adjustment algorithm
│   └── stratum.c/h     # Pool mining protocol
├── network/
│   ├── peer.c/h        # Peer connection management
│   ├── protocol.c/h    # Message serialization/deserialization
│   ├── sync.c/h        # Blockchain synchronization
│   └── dns.c/h         # DNS seed client
├── wallet/
│   ├── wallet.c/h      # BIP39/32/44 HD wallet
│   ├── keys.c/h        # Key derivation and management
│   ├── balance.c/h     # UTXO tracking and balance calculation
│   └── bip39.c/h       # Mnemonic generation and recovery
├── governance/
│   ├── proposal.c/h    # Proposal creation and storage
│   ├── qf.c/h          # Quadratic funding math
│   ├── contribution.c/h# Contribution tracking
│   ├── treasury.c/h    # Treasury accounting
│   └── round.c/h       # Funding round management
├── rpc/
│   ├── server.c/h      # JSON-RPC HTTP server
│   ├── commands.c/h    # RPC command handlers
│   └── handlers/       # Individual command implementations
├── cli/
│   ├── parser.c/h      # Command-line argument parsing
│   └── commands.c/h    # CLI command dispatch
└── util/
    ├── log.c/h         # Logging system
    ├── config.c/h      # Configuration file parsing
    └── db.c/h          # Database abstraction (LevelDB/LMDB)
```

### Building a Block

```c
#include "blockchain/block.h"
#include "crypto/merkle.h"

Block* create_block(const uint8_t prev_hash[32], 
                    Transaction* txs, 
                    uint32_t tx_count,
                    uint8_t miner_address[32]) {
    Block* block = malloc(sizeof(Block));
    block->version = 1;
    memcpy(block->prev_hash, prev_hash, 32);
    block->timestamp = time(NULL);
    block->tx_count = tx_count;
    block->transactions = txs;
    
    // Calculate merkle root
    calculate_merkle_root(block);
    
    // Add coinbase transaction (miner reward + treasury)
    add_coinbase_transaction(block, miner_address);
    
    // Initialize nonce for mining
    block->nonce = 0;
    
    return block;
}

// Add coinbase with treasury split
void add_coinbase_transaction(Block* block, uint8_t miner_address[32]) {
    uint64_t reward = get_block_reward(block->height);
    uint64_t miner_reward = reward * 90 / 100;  // 90%
    uint64_t treasury_reward = reward * 10 / 100;  // 10%
    
    Transaction* coinbase = create_transaction();
    coinbase->version = 1;
    coinbase->input_count = 0;  // No inputs for coinbase
    coinbase->output_count = 2;
    
    // Output 1: Miner reward
    coinbase->outputs[0].amount = miner_reward;
    memcpy(coinbase->outputs[0].recipient_hash, miner_address, 32);
    
    // Output 2: Treasury
    coinbase->outputs[1].amount = treasury_reward;
    memcpy(coinbase->outputs[1].recipient_hash, TREASURY_ADDRESS, 32);
    
    // Add to block
    prepend_transaction(block, coinbase);
}
```

### Deriving Keys from Seed Phrase

```c
#include "wallet/bip39.h"
#include "wallet/bip32.h"
#include "crypto/signature.h"

// Generate wallet from seed phrase
Wallet* create_wallet_from_seed(const char* seed_phrase) {
    // Convert BIP39 mnemonic to 512-bit seed
    uint8_t seed[64];
    mnemonic_to_seed(seed_phrase, "", seed, NULL);
    
    // Derive master key using BIP32
    HDNode master;
    hdnode_from_seed(seed, 64, ED25519_NAME, &master);
    
    // Derive account key using BIP44 path: m/44'/888'/0'/0/0
    // 888 is Agoracoin's coin type
    hdnode_private_ckd(&master, 44 | 0x80000000);  // Purpose
    hdnode_private_ckd(&master, 888 | 0x80000000); // Coin type (Agoracoin)
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

// Generate address from public key
void generate_address(const uint8_t public_key[32], char* address_out) {
    // Hash public key: SHA-256 then RIPEMD-160
    uint8_t sha_hash[32];
    SHA256(public_key, 32, sha_hash);
    
    uint8_t pubkey_hash[20];
    RIPEMD160(sha_hash, 32, pubkey_hash);
    
    // Add version byte
    uint8_t versioned[21];
    versioned[0] = 0x1C;  // Agoracoin version byte
    memcpy(versioned + 1, pubkey_hash, 20);
    
    // Calculate checksum
    uint8_t checksum[32];
    SHA256(versioned, 21, checksum);
    SHA256(checksum, 32, checksum);
    
    // Append first 4 bytes of checksum
    uint8_t address_bytes[25];
    memcpy(address_bytes, versioned, 21);
    memcpy(address_bytes + 21, checksum, 4);
    
    // Base58 encode
    base58_encode(address_bytes, 25, address_out);
    // Result starts with "agora1..."
}
```

### Validating a Transaction

```c
#include "blockchain/transaction.h"
#include "blockchain/validation.h"
#include "crypto/signature.h"

bool validate_transaction(Transaction* tx, UTXOSet* utxo_set) {
    // Check version
    if (tx->version != 1 && tx->version != 2) return false;
    
    // Coinbase transactions have special rules
    if (tx->input_count == 0) {
        return validate_coinbase(tx);
    }
    
    // Verify signatures on all inputs
    for (int i = 0; i < tx->input_count; i++) {
        if (!verify_signature(&tx->inputs[i], tx)) {
            return false;
        }
    }
    
    // Check input amounts exist and aren't spent
    uint64_t input_sum = 0;
    for (int i = 0; i < tx->input_count; i++) {
        UTXO* utxo = utxo_set_find(utxo_set,
                                    tx->inputs[i].prev_tx_hash,
                                    tx->inputs[i].prev_output_index);
        if (!utxo || utxo->spent) {
            return false;
        }
        
        // Verify ownership
        if (!verify_utxo_ownership(utxo, &tx->inputs[i])) {
            return false;
        }
        
        input_sum += utxo->amount;
    }
    
    // Check outputs
    uint64_t output_sum = 0;
    for (int i = 0; i < tx->output_count; i++) {
        if (tx->outputs[i].amount == 0) return false;
        output_sum += tx->outputs[i].amount;
    }
    
    // Fee must be non-negative
    if (input_sum < output_sum) return false;
    
    // For governance transactions, validate governance data
    if (tx->version == 2) {
        return validate_governance_tx(tx);
    }
    
    return true;
}

bool verify_signature(TxInput* input, Transaction* tx) {
    // Create signature message (tx hash without signatures)
    uint8_t msg[32];
    hash_transaction_for_signing(tx, msg);
    
    // Verify Ed25519 signature
    return ed25519_verify(input->signature, 
                          msg, 32,
                          input->public_key);
}
```

### Implementing Quadratic Funding

```c
#include "governance/qf.h"
#include <math.h>

// Calculate QF match for all proposals in a round
void calculate_round_matches(Round* round) {
    double total_match_share = 0.0;
    
    // First pass: calculate match shares
    for (int i = 0; i < round->proposal_count; i++) {
        Proposal* p = &round->proposals[i];
        double match_share = calculate_proposal_match_share(p);
        p->match_share = match_share;
        total_match_share += match_share;
    }
    
    // Second pass: distribute pool proportionally
    for (int i = 0; i < round->proposal_count; i++) {
        Proposal* p = &round->proposals[i];
        
        if (total_match_share > 0) {
            double proportion = p->match_share / total_match_share;
            p->match_amount = (uint64_t)(proportion * round->matching_pool);
        } else {
            p->match_amount = 0;
        }
        
        p->total_funding = p->total_contributed + p->match_amount;
    }
}

double calculate_proposal_match_share(Proposal* p) {
    double sum_sqrt = 0.0;
    
    // Sum square roots of weighted contributions
    for (int i = 0; i < p->contributor_count; i++) {
        Contribution* c = &p->contributions[i];
        
        // Get contributor's participation weight (50-100%)
        double weight = get_contributor_weight(c->contributor_address,
                                               c->rounds_participated);
        
        // Apply weight to contribution
        double weighted_amount = (c->amount / (double)COIN) * weight;
        
        // Add square root to sum
        sum_sqrt += sqrt(weighted_amount);
    }
    
    // QF formula: (sum of square roots)^2
    return sum_sqrt * sum_sqrt;
}

double get_contributor_weight(uint8_t address[32], uint32_t rounds_participated) {
    // New participant: 50% weight
    // After 4 rounds (1 year): 100% weight
    // Linear interpolation
    return 0.5 + (0.5 * fmin(1.0, rounds_participated / 4.0));
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
./tests/governance_test

# Run with valgrind (memory leak detection)
valgrind --leak-check=full ./tests/all_tests

# Run with coverage
make coverage
```

**Example Test:**
```c
#include <check.h>
#include "governance/qf.h"

START_TEST(test_qf_calculation) {
    // Create round
    Round round;
    round.matching_pool = 10000 * COIN;
    round.proposal_count = 2;
    
    // Proposal A: 100 contributors × 1 AGORA
    Proposal* pA = &round.proposals[0];
    pA->contributor_count = 100;
    pA->total_contributed = 100 * COIN;
    for (int i = 0; i < 100; i++) {
        pA->contributions[i].amount = 1 * COIN;
        pA->contributions[i].rounds_participated = 4;  // 100% weight
    }
    
    // Proposal B: 1 contributor × 100 AGORA
    Proposal* pB = &round.proposals[1];
    pB->contributor_count = 1;
    pB->total_contributed = 100 * COIN;
    pB->contributions[0].amount = 100 * COIN;
    pB->contributions[0].rounds_participated = 4;  // 100% weight
    
    // Calculate matches
    calculate_round_matches(&round);
    
    // Verify: Proposal A should get much more matching
    ck_assert(pA->match_amount > pB->match_amount);
    
    // Specific check: A gets (100)^2 = 10000, B gets (10)^2 = 100
    // Ratio should be 100:1
    double ratio = (double)pA->match_amount / pB->match_amount;
    ck_assert(ratio > 50.0);  // Allow some precision loss
}
END_TEST
```

### Contributing

1. Fork the repository on GitHub
2. Create feature branch: `git checkout -b feature/my-feature`
3. Follow coding style (see STYLE.md)
4. Write tests for new functionality
5. Ensure all tests pass: `make test`
6. Update documentation if needed
7. Submit pull request with clear description

### Coding Standards

- **Language**: C99 standard
- **Indentation**: 4 spaces (no tabs)
- **Line length**: 100 characters maximum
- **Naming**: 
  - Functions: `snake_case`
  - Types: `PascalCase`
  - Constants: `UPPER_SNAKE_CASE`
- **Comments**: Doxygen format for public APIs
- **Error handling**: Always check return values
- **Memory**: No leaks (verified with valgrind)
- **Security**: No compiler warnings, no unsafe functions

**Example:**
```c
/**
 * Calculate block reward for given height
 * 
 * @param height Block height
 * @return Reward in satoshis (1e-8 AGORA)
 */
uint64_t get_block_reward(uint32_t height) {
    uint64_t base_reward = INITIAL_BLOCK_REWARD;
    uint32_t halvings = height / HALVING_INTERVAL;
    
    if (halvings < 64) {
        return base_reward >> halvings;
    }
    
    // Tail emission
    return (uint64_t)(0.01 / (365.25 * 720) * TOTAL_SUPPLY * COIN);
}
```

---

## Appendix

### Configuration File Reference

Complete `agoracoin.conf` options:

```conf
# Network
port=8333
maxconnections=125
timeout=30
bind=0.0.0.0
onlynet=ipv4                    # or ipv6
upnp=1                          # Enable UPnP port mapping

# RPC
rpcport=8332
rpcuser=user
rpcpassword=pass
rpcbind=127.0.0.1
rpcallowip=127.0.0.1

# Mining
gen=0                           # Auto-start mining
genproclimit=4                  # Mining threads

# Blockchain
datadir=~/.agoracoin
txindex=1                       # Maintain full tx index
prune=0                         # Prune mode (0=disable)
dbcache=300                     # Database cache size (MB)

# Wallet
keypool=100
walletnotify=/path/to/script.sh # Run on tx

# Governance
enablegovernance=1              # Enable governance features
govnotify=/path/to/script.sh    # Run on proposal events

# Debug
debug=0                         # Debug logging
debugnet=0                      # Network debug
debuggov=0                      # Governance debug
logtimestamps=1
printtoconsole=0

# Advanced
maxorphantx=100
maxmempool=300
mempoolexpiry=336               # Hours
```

### Glossary

- **UTXO**: Unspent Transaction Output - coins available to spend
- **Mempool**: Memory pool of unconfirmed transactions waiting for inclusion in blocks
- **Satoshi**: Smallest unit, 0.00000001 AGORA
- **Orphan Block**: Valid block not in the main chain (due to reorg)
- **Difficulty**: Measure of how hard it is to mine a block
- **Hashrate**: Number of hash calculations per second
- **Seed Phrase**: 24-word BIP39 mnemonic that generates wallet keys
- **BIP39**: Bitcoin Improvement Proposal for mnemonic phrases
- **BIP32**: Hierarchical Deterministic wallet standard
- **BIP44**: Multi-account hierarchy for HD wallets
- **HD Wallet**: Hierarchical Deterministic wallet derived from single seed
- **QF**: Quadratic Funding - democratic treasury allocation mechanism
- **Agora**: Ancient Greek public assembly; metaphor for democratic governance
- **Sybil Attack**: One entity creating many fake identities
- **Treasury**: Community-controlled development fund (10% of block rewards)
- **Proposal**: Request for treasury funding with specific deliverables
- **Contribution**: Donation to a proposal during funding round
- **Matching Pool**: Treasury funds distributed via quadratic formula
- **Stake**: Coins locked for 30 days to prove long-term participation

### Network Statistics (Hypothetical)

Current network stats (example for documentation):

```
Block Height: 526,789
Network Hashrate: 1.2 PH/s
Difficulty: 452,000,000
Average Block Time: 118 seconds (target: 120s)
Mempool Size: 234 transactions
Treasury Balance: 145,678 AGORA
Active Proposals: 8
Current Round: Q2 2026
```

### Important Addresses

**Genesis Block:**
- Block Hash: `0000000000000000000000000000000000000000000000000000000000000000`
- Timestamp: 2026-01-01 00:00:00 UTC
- Merkle Root: `...`

**Treasury Address:**
- Address: `agora1treasury...` (hardcoded in protocol)
- Balance: Query with `agoracoind gettreasurybalance`
- Controlled by: Quadratic funding mechanism (no private key exists)

**Vesting Addresses (Genesis Allocation):**
- Development Fund: `agora1dev...` (30,000 AGORA, 24-month linear vest)
- Community Bounties: `agora1community...` (30,000 AGORA, governed by QF)
- Security Audit: `agora1audit...` (15,000 AGORA, 6-month linear vest)

### Further Reading

**Agoracoin:**
- Website: https://agoracoin.org
- GitHub: https://github.com/agoracoin/agoracoin
- Forum: https://forum.agoracoin.org
- Documentation: https://docs.agoracoin.org

**Technical References:**
- Bitcoin Developer Documentation: https://developer.bitcoin.org
- Cryptocurrency Engineering: https://nakamotoinstitute.org
- Ed25519 Paper: https://ed25519.cr.yp.to/ed25519-20110926.pdf
- SHA-256 Specification: https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf
- BIP39: https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki
- BIP32: https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki
- BIP44: https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki

**Quadratic Funding:**
- Vitalik Buterin's QF Explanation: https://vitalik.ca/general/2019/12/07/quadratic.html
- Gitcoin Grants: https://gitcoin.co/grants
- WTF is QF?: https://wtfisqf.com

**Community:**
- Discord: https://discord.gg/agoracoin
- Twitter: @agoracoin
- Reddit: r/agoracoin
- Telegram: t.me/agoracoin

---

**Last Updated:** October 2025  
**Version:** 0.1.0  
**Maintainer:** Agoracoin Development Team

*"In the agora, every voice matters. Together, we build the future."*

---