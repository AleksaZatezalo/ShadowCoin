# ShadowCoin API Documentation

This document describes the ShadowCoin daemon RPC API, wallet API, and C library interfaces.

## Table of Contents

- [Daemon RPC API](#daemon-rpc-api)
- [Wallet RPC API](#wallet-rpc-api)
- [C Library API](#c-library-api)
- [Error Codes](#error-codes)

---

## Daemon RPC API

The ShadowCoin daemon exposes a JSON-RPC 2.0 interface on port 18081 (configurable).

### Connection

```bash
curl -X POST http://127.0.0.1:18081/json_rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":"0","method":"METHOD_NAME","params":{...}}'
```

### Authentication

Optional HTTP basic authentication or API key header:

```bash
curl -X POST http://127.0.0.1:18081/json_rpc \
  -H "Content-Type: application/json" \
  -H "X-API-Key: your_api_key_here" \
  -d '{...}'
```

---

## Daemon Methods

### get_info

Get general information about the daemon and blockchain state.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "get_info"
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "height": 145032,
    "top_block_hash": "a7f3c4b8d2e1f6a9...",
    "difficulty": 125673892,
    "target": 120,
    "tx_count": 1847392,
    "tx_pool_size": 23,
    "alt_blocks_count": 5,
    "outgoing_connections_count": 8,
    "incoming_connections_count": 12,
    "white_peerlist_size": 150,
    "grey_peerlist_size": 4827,
    "network": "mainnet",
    "version": "0.1.0",
    "status": "OK"
  }
}
```

### get_block_count

Get the current blockchain height.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "get_block_count"
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "count": 145032,
    "status": "OK"
  }
}
```

### get_block_header_by_hash

Get block header information by hash.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "get_block_header_by_hash",
  "params": {
    "hash": "a7f3c4b8d2e1f6a9..."
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "block_header": {
      "hash": "a7f3c4b8d2e1f6a9...",
      "height": 145032,
      "timestamp": 1727635200,
      "difficulty": 125673892,
      "nonce": 4829361,
      "prev_hash": "b8e4d5c9f3a2e7b1...",
      "reward": 50000000000,
      "tx_count": 15
    },
    "status": "OK"
  }
}
```

### get_block_header_by_height

Get block header information by height.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "get_block_header_by_height",
  "params": {
    "height": 145032
  }
}
```

**Response:** Same format as `get_block_header_by_hash`.

### get_block

Get complete block information including transactions.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "get_block",
  "params": {
    "hash": "a7f3c4b8d2e1f6a9..."
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "block": {
      "header": { /* same as get_block_header */ },
      "miner_tx": { /* coinbase transaction */ },
      "tx_hashes": [
        "c9d5e2f8a3b1c7e4...",
        "d2e8f3a9c5b7e1f6..."
      ]
    },
    "status": "OK"
  }
}
```

### submit_block

Submit a mined block to the network.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "submit_block",
  "params": {
    "block_blob": "0x0a0b0c0d..."
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "status": "OK"
  }
}
```

### get_transaction

Get transaction information by hash.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "get_transaction",
  "params": {
    "tx_hash": "c9d5e2f8a3b1c7e4...",
    "decode": true
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "tx": {
      "version": 2,
      "unlock_time": 0,
      "vin": [ /* inputs */ ],
      "vout": [ /* outputs */ ],
      "extra": "0x...",
      "rct_signatures": { /* ring CT data */ }
    },
    "tx_hash": "c9d5e2f8a3b1c7e4...",
    "in_pool": false,
    "block_height": 145030,
    "confirmations": 2,
    "status": "OK"
  }
}
```

### send_raw_transaction

Broadcast a signed transaction to the network.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "send_raw_transaction",
  "params": {
    "tx_blob": "0x0a0b0c0d...",
    "do_not_relay": false
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "tx_hash": "c9d5e2f8a3b1c7e4...",
    "status": "OK"
  }
}
```

### get_transaction_pool

Get transactions currently in the memory pool.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "get_transaction_pool"
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "transactions": [
      {
        "tx_hash": "d2e8f3a9c5b7e1f6...",
        "fee": 100000000,
        "receive_time": 1727635150,
        "size": 2847
      }
    ],
    "status": "OK"
  }
}
```

### get_connections

Get information about connected peers.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "get_connections"
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "connections": [
      {
        "peer_id": "3c7e9f2a1b5d8e4f",
        "ip": "192.168.1.100",
        "port": 18080,
        "incoming": false,
        "height": 145031,
        "live_time": 3600
      }
    ],
    "status": "OK"
  }
}
```

---

## Wallet RPC API

The ShadowCoin wallet RPC server runs on port 18082 (configurable).

### open_wallet

Open a wallet file.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "open_wallet",
  "params": {
    "filename": "mywallet",
    "password": "secretpassword"
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "status": "OK"
  }
}
```

### create_wallet

Create a new wallet.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "create_wallet",
  "params": {
    "filename": "newwallet",
    "password": "secretpassword",
    "language": "English"
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "address": "SHD1a2b3c4d5e6f7g8h9i0j1k2l3m4n5o6p7q8r9s0t1u2v3w4x5y6z7",
    "seed": "abbey ability above absent absorb abstract absurd...",
    "status": "OK"
  }
}
```

### get_balance

Get wallet balance.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "get_balance",
  "params": {
    "account_index": 0
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "balance": 15000000000000,
    "unlocked_balance": 14500000000000,
    "per_subaddress": [
      {
        "address": "SHD1a2b3c4d5...",
        "balance": 15000000000000,
        "unlocked_balance": 14500000000000
      }
    ],
    "status": "OK"
  }
}
```

### get_address

Get wallet addresses.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "get_address",
  "params": {
    "account_index": 0
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "address": "SHD1a2b3c4d5e6f7g8h9i0j1k2l3m4n5o6p7q8r9s0t1u2v3w4x5y6z7",
    "addresses": [
      {
        "address": "SHD1a2b3c4d5...",
        "label": "Primary",
        "address_index": 0,
        "used": true
      }
    ],
    "status": "OK"
  }
}
```

### create_address

Create a new subaddress.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "create_address",
  "params": {
    "account_index": 0,
    "label": "Savings"
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "address": "SHD2b3c4d5e6f7g8h9...",
    "address_index": 1,
    "status": "OK"
  }
}
```

### transfer

Send ShadowCoin to an address.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "transfer",
  "params": {
    "destinations": [
      {
        "amount": 1000000000000,
        "address": "SHD9z8y7x6w5v4u3..."
      }
    ],
    "account_index": 0,
    "priority": 2,
    "ring_size": 11,
    "get_tx_key": true
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "tx_hash": "e8f3a9c5b7e1f6d2...",
    "tx_key": "a3c7e9f2b5d8e4f1...",
    "amount": 1000000000000,
    "fee": 100000000,
    "status": "OK"
  }
}
```

### get_transfers

Get incoming and outgoing transfers.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "get_transfers",
  "params": {
    "in": true,
    "out": true,
    "pending": true,
    "failed": false,
    "pool": true
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "in": [
      {
        "txid": "c9d5e2f8a3b1c7e4...",
        "height": 145020,
        "timestamp": 1727634000,
        "amount": 5000000000000,
        "confirmations": 12
      }
    ],
    "out": [
      {
        "txid": "d2e8f3a9c5b7e1f6...",
        "height": 145025,
        "timestamp": 1727634600,
        "amount": 2000000000000,
        "fee": 100000000,
        "confirmations": 7
      }
    ],
    "status": "OK"
  }
}
```

### sweep_all

Send all unlocked balance to an address.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "method": "sweep_all",
  "params": {
    "address": "SHD9z8y7x6w5v4u3...",
    "account_index": 0,
    "priority": 2,
    "ring_size": 11
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": "0",
  "result": {
    "tx_hash_list": ["e8f3a9c5b7e1f6d2..."],
    "tx_key_list": ["a3c7e9f2b5d8e4f1..."],
    "amount_list": [14500000000000],
    "fee_list": [100000000],
    "status": "OK"
  }
}
```

---

## C Library API

The ShadowCoin C library provides direct access to core functionality.

### Initialization

```c
#include <shadowcoin/shadowcoin.h>

// Initialize the library
int shd_init(void);

// Cleanup resources
void shd_cleanup(void);
```

### Blockchain Operations

```c
// Create a new blockchain instance
shd_blockchain_t* shd_blockchain_create(const char* data_dir);

// Destroy blockchain instance
void shd_blockchain_destroy(shd_blockchain_t* bc);

// Get current height
uint64_t shd_blockchain_get_height(shd_blockchain_t* bc);

// Add a block to the chain
shd_result_t shd_blockchain_add_block(shd_blockchain_t* bc, 
                                      const shd_block_t* block);

// Get block by height
shd_block_t* shd_blockchain_get_block_by_height(shd_blockchain_t* bc, 
                                                 uint64_t height);

// Get block by hash
shd_block_t* shd_blockchain_get_block_by_hash(shd_blockchain_t* bc, 
                                               const uint8_t hash[32]);
```

### Transaction Operations

```c
// Create a new transaction
shd_transaction_t* shd_transaction_create(void);

// Destroy transaction
void shd_transaction_destroy(shd_transaction_t* tx);

// Add input to transaction
shd_result_t shd_transaction_add_input(shd_transaction_t* tx,
                                       const uint8_t key_image[32],
                                       const shd_ring_member_t* ring,
                                       size_t ring_size);

// Add output to transaction
shd_result_t shd_transaction_add_output(shd_transaction_t* tx,
                                        uint64_t amount,
                                        const uint8_t* stealth_address);

// Sign transaction with ring signature
shd_result_t shd_transaction_sign(shd_transaction_t* tx,
                                  const uint8_t* spend_key);

// Verify transaction
shd_result_t shd_transaction_verify(const shd_transaction_t* tx);

// Serialize transaction to bytes
size_t shd_transaction_serialize(const shd_transaction_t* tx,
                                uint8_t* buffer,
                                size_t buffer_size);
```

### Wallet Operations

```c
// Create a new wallet
shd_wallet_t* shd_wallet_create(const char* seed_phrase);

// Open existing wallet
shd_wallet_t* shd_wallet_open(const char* filepath, const char* password);

// Save wallet to file
shd_result_t shd_wallet_save(shd_wallet_t* wallet, 
                             const char* filepath,
                             const char* password);

// Destroy wallet
void shd_wallet_destroy(shd_wallet_t* wallet);

// Get primary address
const char* shd_wallet_get_address(shd_wallet_t* wallet);

// Get balance
uint64_t shd_wallet_get_balance(shd_wallet_t* wallet);

// Create and send transaction
shd_result_t shd_wallet_transfer(shd_wallet_t* wallet,
                                 const char* destination,
                                 uint64_t amount,
                                 uint32_t ring_size,
                                 uint8_t tx_hash_out[32]);
```

### Cryptographic Operations

```c
// Generate key pair
void shd_crypto_generate_keypair(uint8_t public_key[32], 
                                 uint8_t secret_key[32]);

// Generate stealth address
void shd_crypto_generate_stealth_address(const uint8_t* public_spend_key,
                                         const uint8_t* public_view_key,
                                         const uint8_t* random_data,
                                         uint8_t* stealth_address_out);

// Create ring signature
shd_result_t shd_crypto_ring_signature_create(const uint8_t message[32],
                                              const uint8_t* public_keys,
                                              size_t ring_size,
                                              const uint8_t secret_key[32],
                                              size_t secret_index,
                                              uint8_t* signature_out);

// Verify ring signature
shd_result_t shd_crypto_ring_signature_verify(const uint8_t message[32],
                                              const uint8_t* public_keys,
                                              size_t ring_size,
                                              const uint8_t* signature);

// Generate key image
void shd_crypto_generate_key_image(const uint8_t secret_key[32],
                                   const uint8_t public_key[32],
                                   uint8_t key_image_out[32]);
```

### Mining Operations

```c
// Create miner instance
shd_miner_t* shd_miner_create(shd_blockchain_t* bc, 
                              const char* wallet_address);

// Destroy miner
void shd_miner_destroy(shd_miner_t* miner);

// Start mining with specified threads
shd_result_t shd_miner_start(shd_miner_t* miner, uint32_t threads);

// Stop mining
void shd_miner_stop(shd_miner_t* miner);

// Get mining statistics
void shd_miner_get_stats(shd_miner_t* miner, shd_miner_stats_t* stats_out);
```

---

## Error Codes

### General Errors
- `SHD_OK = 0`: Success
- `SHD_ERROR_GENERIC = 1`: Generic error
- `SHD_ERROR_INVALID_ARGUMENT = 2`: Invalid argument provided
- `SHD_ERROR_OUT_OF_MEMORY = 3`: Memory allocation failed
- `SHD_ERROR_NOT_FOUND = 4`: Resource not found

### Blockchain Errors
- `SHD_ERROR_INVALID_BLOCK = 100`: Block validation failed
- `SHD_ERROR_INVALID_POW = 101`: Invalid proof of work
- `SHD_ERROR_DUPLICATE_BLOCK = 102`: Block already exists
- `SHD_ERROR_ORPHAN_BLOCK = 103`: Orphan block (parent not found)

### Transaction Errors
- `SHD_ERROR_INVALID_TRANSACTION = 200`: Transaction validation failed
- `SHD_ERROR_INVALID_SIGNATURE = 201`: Ring signature verification failed
- `SHD_ERROR_DOUBLE_SPEND = 202`: Key image already used
- `SHD_ERROR_INSUFFICIENT_FUNDS = 203`: Not enough balance
- `SHD_ERROR_INVALID_AMOUNT = 204`: Invalid transaction amount

### Network Errors
- `SHD_ERROR_NETWORK_TIMEOUT = 300`: Network operation timed out
- `SHD_ERROR_CONNECTION_FAILED = 301`: Failed to connect to peer
- `SHD_ERROR_PROTOCOL_ERROR = 302`: Protocol violation

### Wallet Errors
- `SHD_ERROR_WALLET_FILE = 400`: Wallet file error
- `SHD_ERROR_WRONG_PASSWORD = 401`: Incorrect wallet password
- `SHD_ERROR_WALLET_LOCKED = 402`: Wallet is locked

---

## Data Types

### shd_result_t
```c
typedef enum {
    SHD_OK = 0,
    SHD_ERROR = 1,
    // ... (see Error Codes)
} shd_result_t;
```

### shd_block_t
```c
typedef struct {
    uint32_t version;
    uint64_t height;
    uint64_t timestamp;
    uint8_t prev_hash[32];
    uint64_t nonce;
    shd_transaction_t* miner_tx;
    shd_transaction_t** transactions;
    size_t tx_count;
} shd_block_t;
```

### shd_transaction_t
```c
typedef struct {
    uint32_t version;
    uint64_t unlock_time;
    shd_tx_input_t* inputs;
    size_t input_count;
    shd_tx_output_t* outputs;
    size_t output_count;
    uint8_t* extra;
    size_t extra_size;
    shd_ringct_t* rct_signatures;
} shd_transaction_t;
```

### shd_miner_stats_t
```c
typedef struct {
    uint64_t blocks_found;
    uint64_t hashes_computed;
    double hashrate;
    uint64_t uptime_seconds;
} shd_miner_stats_t;
```

---

## Examples

### C Library Example

```c
#include <shadowcoin/shadowcoin.h>
#include <stdio.h>

int main(void) {
    // Initialize library
    shd_init();
    
    // Create wallet
    shd_wallet_t* wallet = shd_wallet_create(NULL);
    printf("Address: %s\n", shd_wallet_get_address(wallet));
    
    // Create blockchain instance
    shd_blockchain_t* bc = shd_blockchain_create("./data");
    printf("Height: %lu\n", shd_blockchain_get_height(bc));
    
    // Cleanup
    shd_wallet_destroy(wallet);
    shd_blockchain_destroy(bc);
    shd_cleanup();
    
    return 0;
}
```

### Python RPC Example

```python
import requests
import json

def rpc_call(method, params=None):
    url = "http://127.0.0.1:18081/json_rpc"
    headers = {"Content-Type": "application/json"}
    payload = {
        "jsonrpc": "2.0",
        "id": "0",
        "method": method,
        "params": params or {}
    }
    response = requests.post(url, headers=headers, data=json.dumps(payload))
    return response.json()

# Get blockchain info
info = rpc_call("get_info")
print(f"Height: {info['result']['height']}")
print(f"Difficulty: {info['result']['difficulty']}")
```

---

For more information, see the [Protocol Documentation](PROTOCOL.md) and [Whitepaper](WHITEPAPER.md).