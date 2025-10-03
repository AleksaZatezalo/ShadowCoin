# Agoracoin
A cryptocurrency for democratic participation and efficient governance.

*Named after the ancient Greek agora—the public assembly where citizens gathered to discuss and decide the future of their city-state.*

## Overview
Agoracoin is a blockchain implementation in pure C, designed for equitable participation through quadratic funding of community treasury allocation. Modeled after Bitcoin Core, Agoracoin delivers all functionality—full node, wallet, mining, and networking—in a single, lightweight binary (agoracoind). Its lean design minimizes resource use while maximizing performance.

## Core Principles
**Democratic Governance:** Quadratic funding ensures treasury grants reflect broad community support, not just the largest donors.
**Simple Mining:** SHA-256d proof-of-work, the same algorithm as Bitcoin, proven secure and ASIC-friendly.
**Lightweight Design:** Minimal dependencies and optimized code ensure efficiency on modest hardware.
**Community-Driven:** A treasury system, funded by 10% of each block reward, supports sustainable development through quadratic funding rounds.

## Economic Model

### Initial Distribution
75,000 AGORA created in genesis block, distributed as:
- **30,000 AGORA**: Development fund (vested linearly over 24 months)
- **30,000 AGORA**: Community bounties (governed by quadratic funding)
- **15,000 AGORA**: Security audit & infrastructure fund (6 month vest)

All genesis funds are locked in time-locked addresses with transparent vesting schedules published on-chain.

### Block Rewards
- **Block time**: 2 minutes (30 blocks/hour, 720 blocks/day)
- **Starting reward**: 10 AGORA per block
- **Miner receives**: 9 AGORA (90%)
- **Treasury receives**: 1 AGORA (10%)
- **Halving schedule**: Every 525,600 blocks (~2 years at 2-min blocks)
- **Tail emission**: After final halving, 1% annual inflation maintains miner incentives
- **Confirmations**: 6 blocks = 12 minutes (reasonable finality)

### Treasury Economics
```
Daily treasury accumulation: 1 AGORA/block × 720 blocks = 720 AGORA/day
Annual treasury budget: 720 × 365 = 262,800 AGORA/year
At $1/AGORA: $262,800/year for development
At $10/AGORA: $2.6M/year for development
```

The treasury automatically accumulates 10% of every block reward. These funds are allocated through quarterly quadratic funding rounds, where community donations are matched by the treasury pool.

## Quadratic Funding System

### How It Works

Quadratic funding amplifies the voice of the community by emphasizing the **number of contributors** over the **amount contributed**. A project supported by 100 people donating 1 AGORA each receives more matching funds than a project with 1 whale donating 100 AGORA.

**Formula**: Matching amount = (√contribution₁ + √contribution₂ + ... + √contributionₙ)²

**Example Round**:
```
Treasury matching pool: 60,000 AGORA

Project A: 100 contributors × 1 AGORA each = 100 AGORA raised
  Match = (√1 + √1 + ... [100 times])² = (100)² = 10,000 AGORA
  Total funding: 100 + 10,000 = 10,100 AGORA

Project B: 1 contributor × 100 AGORA = 100 AGORA raised  
  Match = (√100)² = (10)² = 100 AGORA
  Total funding: 100 + 100 = 200 AGORA

Result: Project A gets 50x more matching despite equal direct contributions!
```

### Funding Rounds

**Quarterly schedule**:
- Q1: January 1 - March 31 (proposals due Feb 1, voting Feb-Mar)
- Q2: April 1 - June 30 (proposals due May 1, voting May-Jun)
- Q3: July 1 - September 30 (proposals due Aug 1, voting Aug-Sep)  
- Q4: October 1 - December 31 (proposals due Nov 1, voting Nov-Dec)

**Per-round matching pool**: ~65,700 AGORA (262,800 / 4)

### Proposal Submission

Anyone can submit a proposal on-chain with a 100 AGORA deposit (refunded if proposal receives >1% of round donations).

```bash
# Submit proposal
./agoracoind submitproposal \
  --title "Core wallet GUI development" \
  --description "Build cross-platform GUI using Dear ImGui" \
  --recipient agora1a2b3c4d5e6f... \
  --amount 5000 \
  --milestone-1 "Basic wallet interface" \
  --milestone-2 "Transaction history view" \
  --milestone-3 "Advanced features"

# Proposal gets unique ID (hash of proposal data)
# Deposit: 100 AGORA locked until round ends
```

### Contributing to Proposals

During the contribution period (45 days per quarter), anyone can donate to proposals:

```bash
# Contribute to a proposal
./agoracoind contribute \
  --proposal-id 0x7f3a9b... \
  --amount 10

# Check proposal stats
./agoracoind getproposal 0x7f3a9b...
```

### Matching Calculation

At round end, matching funds are calculated and distributed:

```c
typedef struct {
    uint8_t proposal_id[32];
    uint64_t total_contributions;  // Sum of all donations
    uint32_t contributor_count;    // Number of unique donors
    Contribution *contributions;   // Array of donations
} Proposal;

typedef struct {
    uint8_t donor_address[32];
    uint64_t amount;               // In satoshis (COIN = 100000000)
    uint32_t timestamp;
} Contribution;

// Calculate quadratic matching
uint64_t calculate_qf_match(Proposal *proposal, uint64_t pool) {
    double sum_sqrt = 0.0;
    
    for (int i = 0; i < proposal->contributor_count; i++) {
        double amount = proposal->contributions[i].amount / (double)COIN;
        sum_sqrt += sqrt(amount);
    }
    
    double match_share = sum_sqrt * sum_sqrt;
    
    // Calculate proportion of total pool based on all proposals
    return (uint64_t)(match_share * pool_per_proposal_ratio * COIN);
}
```

### Sybil Resistance

Quadratic funding is vulnerable to one person creating many addresses. Agoracoin implements multiple defenses:

**1. Contribution Limits**
```c
// Per-round limits (resets every quarter)
#define MAX_CONTRIBUTION_PER_ADDRESS (100 * COIN)  // 100 AGORA max
#define MIN_CONTRIBUTION (0.1 * COIN)               // 0.1 AGORA min
```

**2. Staking Requirement**
```c
// Must stake coins 30 days before contributing
#define CONTRIBUTION_STAKE_PERIOD (30 * 24 * 60 * 60)  // 30 days in seconds

// Prevents address splitting because you can't split future stakes
bool can_contribute(Address addr, uint32_t current_time) {
    uint32_t first_stake = get_first_stake_time(addr);
    return (current_time - first_stake) >= CONTRIBUTION_STAKE_PERIOD;
}
```

**3. Historical Weighting**
```c
// Longer participation history = higher contribution weight
double get_contribution_weight(Address addr, uint32_t rounds_participated) {
    // New addresses: 50% weight
    // After 4 rounds (1 year): 100% weight
    return 0.5 + (0.5 * min(1.0, rounds_participated / 4.0));
}
```

## Architecture

### Core Components

```
agoracoin/
├── src/
│   ├── main.c                 # Entry point and command dispatcher
│   ├── daemon.c/h             # Node daemon and RPC server
│   ├── blockchain/
│   │   ├── block.c/h          # Block structure and validation
│   │   ├── chain.c/h          # Chain management and consensus
│   │   └── transaction.c/h    # Transaction handling
│   ├── crypto/
│   │   ├── hash.c/h           # SHA-256d hashing
│   │   └── signature.c/h      # Ed25519 signatures
│   ├── mining/
│   │   ├── miner.c/h          # SHA-256d mining logic
│   │   └── difficulty.c/h     # Difficulty adjustment
│   ├── network/
│   │   ├── peer.c/h           # P2P networking
│   │   └── protocol.c/h       # Message protocol
│   ├── wallet/
│   │   ├── wallet.c/h         # Key management
│   │   └── balance.c/h        # UTXO tracking
│   ├── governance/
│   │   ├── proposal.c/h       # Proposal creation and storage
│   │   ├── qf.c/h             # Quadratic funding calculations
│   │   └── treasury.c/h       # Treasury accounting
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

**SHA-256d Mining:** Uses the same double SHA-256 proof-of-work as Bitcoin. Battle-tested, secure, and ASIC-friendly. No gimmicks, no "ASIC-resistance" theater.

**Lightweight Design:** Minimal dependencies (standard C libraries + libsodium). Simple binary formats for blocks and transactions. Efficient UTXO set management.

**Fast Confirmations:** 2-minute blocks provide reasonable finality (6 confirmations = 12 minutes) while maintaining network security.

**Core Functionality:** Ed25519 signatures for secure transactions. UTXO-based transaction model. Difficulty adjustment every 1,008 blocks (~33.6 hours). Block reward halving every 525,600 blocks (~2 years), with 10% treasury allocation and tail emissions.

**Quadratic Funding:** On-chain proposal submission and contribution tracking. Quarterly funding rounds with automatic matching distribution. Transparent treasury accounting with multiple Sybil-resistance mechanisms.

## Building

```bash
# Install dependencies
sudo apt-get install libsodium-dev

# Build the single binary
make clean && make

# This produces: ./agoracoind

# Run tests
make test
```

## Usage

### Running as a Full Node

```bash
# Start node with default settings
./agoracoind

# Start with custom port
./agoracoind -port=8333

# Start in background
./agoracoind -daemon
```

### Wallet Operations

```bash
# Create new wallet (generates 24-word seed phrase)
./agoracoind createwallet

# Check balance
./agoracoind getbalance

# Send coins
./agoracoind sendtoaddress <recipient-address> <amount>
```

### Mining

```bash
# Start mining (requires running node)
./agoracoind setgenerate true

# Mine with specific thread count
./agoracoind setgenerate true 4

# Stop mining
./agoracoind setgenerate false
```

### Governance Commands

```bash
# Stake coins for governance participation (required 30 days before contributing)
./agoracoind stakeforgov 1000

# Submit a proposal (costs 100 AGORA deposit)
./agoracoind submitproposal \
  --title "Your project title" \
  --description "Detailed description" \
  --recipient agora1... \
  --amount 5000

# List active proposals
./agoracoind listproposals

# Get proposal details
./agoracoind getproposal <proposal-id>

# Contribute to a proposal
./agoracoind contribute \
  --proposal-id <id> \
  --amount 10

# Check current funding round status
./agoracoind getroundinfo

# Check treasury balance
./agoracoind gettreasurybalance
```

## Configuration

Configuration file: `~/.agoracoin/agoracoin.conf`

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
datadir=~/.agoracoin/data
txindex=1          # Maintain full transaction index

# Governance
enablegovernance=1 # Enable proposal submission/contribution
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

**SHA-256d** (double SHA-256), identical to Bitcoin:

```c
void hash_block_header(BlockHeader *header, uint8_t *output) {
    uint8_t first_hash[32];
    SHA256((uint8_t*)header, sizeof(BlockHeader), first_hash);
    SHA256(first_hash, 32, output);
}

bool check_proof_of_work(uint8_t *hash, uint32_t difficulty) {
    // Hash must be less than target (more leading zeros = harder)
    uint256 target = difficulty_to_target(difficulty);
    return uint256_compare(hash, target) < 0;
}
```

**Why SHA-256d?**
- Battle-tested for 15+ years (Bitcoin)
- Hardware-accelerated on modern CPUs
- Massive ASIC industry = more security options
- No "ASIC-resistance" snake oil
- Predictable, stable, boring = good for money

### Block Reward Calculation
```c
#define INITIAL_BLOCK_REWARD (10 * COIN)
#define TREASURY_PERCENT 10
#define MINER_PERCENT 90
#define HALVING_INTERVAL 525600  // ~2 years at 2-min blocks
#define TAIL_EMISSION_RATE 1     // 1% annual after final halving

uint64_t get_block_reward(uint32_t height) {
    uint64_t base_reward = INITIAL_BLOCK_REWARD;
    uint32_t halvings = height / HALVING_INTERVAL;
    
    // Calculate halved reward
    if (halvings < 64) {
        base_reward >>= halvings;  // Divide by 2^halvings
    } else {
        // After ~64 halvings, switch to tail emission
        // 1% annual = 0.01 / (365.25 * 720 blocks/day) per block
        base_reward = (uint64_t)(0.01 / (365.25 * 720) * TOTAL_SUPPLY * COIN);
    }
    
    return base_reward;
}

uint64_t get_miner_reward(uint32_t height) {
    return get_block_reward(height) * MINER_PERCENT / 100;
}

uint64_t get_treasury_reward(uint32_t height) {
    return get_block_reward(height) * TREASURY_PERCENT / 100;
}
```

### Governance Transaction Types

Special transaction types for governance:

```c
typedef enum {
    TX_TYPE_NORMAL = 0,
    TX_TYPE_PROPOSAL = 1,       // Submit proposal (100 AGORA deposit)
    TX_TYPE_CONTRIBUTION = 2,   // Contribute to proposal
    TX_TYPE_MATCH_PAYOUT = 3,   // Treasury match distribution
    TX_TYPE_STAKE_GOV = 4,      // Stake for governance participation
} TxType;

typedef struct {
    TxType type;
    uint8_t proposal_id[32];    // Hash of proposal content
    char title[256];
    char description[2048];
    uint8_t recipient[32];      // Proposal recipient address
    uint64_t requested_amount;
    uint32_t round_number;
} ProposalTx;

typedef struct {
    TxType type;
    uint8_t proposal_id[32];
    uint64_t amount;
    uint8_t contributor[32];
    double weight;              // Based on stake age and history
} ContributionTx;
```

### Network Bootstrap

**DNS Seeds:**
```c
const char *DNS_SEEDS[] = {
    "seed.agoracoin.org",
    "seed.agoracoin.io",
    "seed.agoracoin.net",
    NULL
};
```

**Hardcoded Seed Nodes:**
```c
const char *SEED_NODES[] = {
    "45.76.132.89:8333",
    "104.238.145.76:8333", 
    "149.28.177.234:8333",
    "207.148.85.91:8333",
    NULL
};
```

### Difficulty Adjustment

```c
#define DIFFICULTY_ADJUSTMENT_INTERVAL 1008  // ~33.6 hours at 2-min blocks
#define TARGET_TIMESPAN (1008 * 120)         // 2 minutes per block

uint32_t calculate_next_difficulty(Block *last_block, Block *first_block) {
    uint32_t actual_timespan = last_block->timestamp - first_block->timestamp;
    
    // Limit adjustment to 4x up or down
    if (actual_timespan < TARGET_TIMESPAN / 4) {
        actual_timespan = TARGET_TIMESPAN / 4;
    }
    if (actual_timespan > TARGET_TIMESPAN * 4) {
        actual_timespan = TARGET_TIMESPAN * 4;
    }
    
    uint256 new_target = current_target * actual_timespan / TARGET_TIMESPAN;
    return target_to_difficulty(new_target);
}
```

### RPC Interface

```bash
# Get current funding round info
curl --user user:pass --data-binary \
  '{"jsonrpc":"1.0","id":"1","method":"getroundinfo","params":[]}' \
  http://localhost:8332/

# Response:
{
  "round": 4,
  "start_block": 126001,
  "end_block": 157500,
  "matching_pool": "65700.00000000",
  "proposals": 12,
  "total_contributed": "8234.50000000"
}
```

## Development Roadmap

- [x] Core blockchain data structures
- [x] SHA-256d mining algorithm
- [x] Economic model with treasury allocation
- [x] Quadratic funding mechanism design
- [x] Sybil resistance mechanisms
- [ ] Single binary architecture with command routing
- [ ] P2P network protocol with bootstrap
- [ ] Wallet implementation (BIP39 seeds)
- [ ] Transaction pool (mempool)
- [ ] Governance transaction types
- [ ] Quadratic funding calculation engine
- [ ] JSON-RPC server
- [ ] CLI command interface
- [ ] Configuration file parsing
- [ ] Difficulty adjustment implementation
- [ ] Genesis block with time-locked vesting
- [ ] Tail emission implementation
- [ ] Transaction compression
- [ ] Compact blocks

## Directory Structure

```
~/.agoracoin/
├── agoracoin.conf       # Configuration file
├── wallet.dat            # Encrypted wallet (derived from seed)
├── peers.dat             # Known peers
├── governance/
│   ├── proposals.dat     # Active proposals
│   ├── contributions.dat # Contribution history
│   ├── stakes.dat        # Governance stakes
│   └── rounds.dat        # Funding round history
├── data/
│   ├── blocks/           # Block data
│   ├── chainstate/       # UTXO set
│   └── index/            # Transaction index
└── debug.log             # Log file
```

## Example Funding Round

**Q1 2026 Round**:
- **Matching pool**: 65,700 AGORA
- **Proposal period**: Feb 1-28
- **Contribution period**: Mar 1-31
- **Proposals submitted**: 8

**Results**:

| Proposal | Contributors | Raised | Match | Total | % of Pool |
|----------|--------------|--------|-------|-------|-----------|
| Core GUI | 234 | 1,247 | 18,234 | 19,481 | 27.8% |
| Mobile wallet | 189 | 892 | 14,503 | 15,395 | 22.1% |
| Block explorer | 156 | 634 | 11,826 | 12,460 | 18.0% |
| Documentation | 98 | 423 | 6,507 | 6,930 | 9.9% |
| Mining pool | 45 | 892 | 4,143 | 5,035 | 6.3% |
| Website redesign | 34 | 234 | 2,811 | 3,045 | 4.3% |
| Analytics | 23 | 178 | 2,376 | 2,554 | 3.6% |

Note how "Core GUI" with 234 contributors gets 27.8% of matching funds, while "Mining pool" with fewer contributors (45) but similar direct funding (892 AGORA) gets only 6.3% of matching.

## Comparison to Bitcoin

| Feature | Agoracoin | Bitcoin |
|---------|------------|---------|
| Algorithm | SHA-256d | SHA-256d |
| Block time | 2 minutes | 10 minutes |
| Initial reward | 10 AGORA | 50 BTC |
| Halving interval | ~2 years | ~4 years |
| Treasury | 10% to dev fund | 0% |
| Governance | Quadratic funding | None |
| Total supply | ~10.5M + 1% tail | 21M fixed |
| Confirmations | 6 (12 min) | 6 (60 min) |

## Why SHA-256d?

**We deliberately chose the same algorithm as Bitcoin because:**

1. **Security through maturity** - 15+ years of battle-testing, no critical vulnerabilities found
2. **ASIC availability** - Massive hardware market, competitive mining industry
3. **No false promises** - We don't claim "ASIC-resistance" (which always fails within 2 years)
4. **Energy efficiency** - Modern SHA-256 ASICs are far more efficient than GPU/CPU mining
5. **Simplicity** - Well-understood, extensively optimized, boring = good
6. **Merge mining potential** - Could enable merge mining with Bitcoin in the future

**ASIC mining is a feature, not a bug.** It means:
- Professional miners with infrastructure and accountability
- Higher hashrate = more security
- Predictable economics
- No algorithm update drama every 18 months

## License

MIT License - see LICENSE file for details

## Resources

- [Bitcoin Whitepaper](https://bitcoin.org/bitcoin.pdf)
- [Bitcoin Core](https://github.com/bitcoin/bitcoin)
- [Ed25519 Signatures](https://ed25519.cr.yp.to/)
- [Quadratic Funding by Vitalik Buterin](https://vitalik.ca/general/2019/12/07/quadratic.html)
- [Gitcoin Grants Mechanism](https://www.gitcoin.co/grants)
- [SHA-256 Specification (FIPS 180-4)](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf)

---