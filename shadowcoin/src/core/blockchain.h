/**
 * blockchain.h - ShadowCoin Blockchain Core
 * 
 * Provides blockchain data structure management and validation.
 * Implements the main chain state, block validation, and reorganization logic.
 */

#ifndef SHADOWCOIN_BLOCKCHAIN_H
#define SHADOWCOIN_BLOCKCHAIN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Forward declarations */
typedef struct shd_blockchain shd_blockchain_t;
typedef struct shd_block shd_block_t;
typedef struct shd_block_header shd_block_header_t;
typedef struct shd_transaction shd_transaction_t;

/* Result codes */
typedef enum {
    SHD_OK = 0,
    SHD_ERROR_GENERIC = 1,
    SHD_ERROR_INVALID_ARGUMENT = 2,
    SHD_ERROR_OUT_OF_MEMORY = 3,
    SHD_ERROR_NOT_FOUND = 4,
    SHD_ERROR_INVALID_BLOCK = 100,
    SHD_ERROR_INVALID_POW = 101,
    SHD_ERROR_DUPLICATE_BLOCK = 102,
    SHD_ERROR_ORPHAN_BLOCK = 103,
    SHD_ERROR_INVALID_TRANSACTION = 200,
    SHD_ERROR_DOUBLE_SPEND = 202,
    SHD_ERROR_DATABASE = 500
} shd_result_t;

/* Constants */
#define SHD_HASH_SIZE 32
#define SHD_BLOCK_TIME_TARGET 120  /* 2 minutes */
#define SHD_DIFFICULTY_WINDOW 720  /* ~1 day */
#define SHD_MAX_BLOCK_SIZE (2 * 1024 * 1024)  /* 2 MB */
#define SHD_COINBASE_MATURITY 60
#define SHD_HALVING_INTERVAL 1050000

/* Block header structure */
struct shd_block_header {
    uint32_t version;
    uint64_t height;
    uint64_t timestamp;
    uint8_t prev_hash[SHD_HASH_SIZE];
    uint64_t nonce;
    uint64_t difficulty;
};

/* Block structure */
struct shd_block {
    shd_block_header_t header;
    shd_transaction_t *miner_tx;
    shd_transaction_t **transactions;
    size_t tx_count;
    uint8_t hash[SHD_HASH_SIZE];
};

/* Blockchain statistics */
typedef struct {
    uint64_t height;
    uint64_t tx_count;
    uint64_t total_supply;
    uint64_t difficulty;
    uint64_t hashrate;  /* Estimated network hashrate */
    uint8_t top_hash[SHD_HASH_SIZE];
} shd_blockchain_stats_t;

/* Validation context - passed through validation functions */
typedef struct {
    const shd_blockchain_t *blockchain;
    uint64_t current_height;
    uint64_t current_difficulty;
    uint64_t median_timestamp;
} shd_validation_context_t;

/**
 * Blockchain Creation and Destruction
 */

/**
 * Create a new blockchain instance
 * 
 * @param data_dir Path to blockchain data directory
 * @return Blockchain instance or NULL on failure
 */
shd_blockchain_t* shd_blockchain_create(const char *data_dir);

/**
 * Destroy blockchain instance and free resources
 * 
 * @param bc Blockchain instance
 */
void shd_blockchain_destroy(shd_blockchain_t *bc);

/**
 * Initialize blockchain with genesis block
 * 
 * @param bc Blockchain instance
 * @return SHD_OK on success, error code otherwise
 */
shd_result_t shd_blockchain_init_genesis(shd_blockchain_t *bc);

/**
 * Blockchain Query Functions
 */

/**
 * Get current blockchain height
 * 
 * @param bc Blockchain instance
 * @return Current height (0 if empty)
 */
uint64_t shd_blockchain_get_height(const shd_blockchain_t *bc);

/**
 * Get blockchain statistics
 * 
 * @param bc Blockchain instance
 * @param stats Output statistics structure
 * @return SHD_OK on success
 */
shd_result_t shd_blockchain_get_stats(const shd_blockchain_t *bc,
                                      shd_blockchain_stats_t *stats);

/**
 * Get block by height
 * 
 * @param bc Blockchain instance
 * @param height Block height
 * @param block_out Output block (caller must free with shd_block_free)
 * @return SHD_OK on success, SHD_ERROR_NOT_FOUND if not found
 */
shd_result_t shd_blockchain_get_block_by_height(const shd_blockchain_t *bc,
                                                uint64_t height,
                                                shd_block_t **block_out);

/**
 * Get block by hash
 * 
 * @param bc Blockchain instance
 * @param hash Block hash
 * @param block_out Output block (caller must free with shd_block_free)
 * @return SHD_OK on success, SHD_ERROR_NOT_FOUND if not found
 */
shd_result_t shd_blockchain_get_block_by_hash(const shd_blockchain_t *bc,
                                              const uint8_t hash[SHD_HASH_SIZE],
                                              shd_block_t **block_out);

/**
 * Check if block exists
 * 
 * @param bc Blockchain instance
 * @param hash Block hash
 * @return true if block exists
 */
bool shd_blockchain_has_block(const shd_blockchain_t *bc,
                              const uint8_t hash[SHD_HASH_SIZE]);

/**
 * Get transaction by hash
 * 
 * @param bc Blockchain instance
 * @param tx_hash Transaction hash
 * @param tx_out Output transaction (caller must free)
 * @param height_out Output block height containing tx (can be NULL)
 * @return SHD_OK on success
 */
shd_result_t shd_blockchain_get_transaction(const shd_blockchain_t *bc,
                                            const uint8_t tx_hash[SHD_HASH_SIZE],
                                            shd_transaction_t **tx_out,
                                            uint64_t *height_out);

/**
 * Check if key image has been spent
 * 
 * @param bc Blockchain instance
 * @param key_image Key image to check
 * @return true if key image is spent
 */
bool shd_blockchain_is_key_image_spent(const shd_blockchain_t *bc,
                                       const uint8_t key_image[SHD_HASH_SIZE]);

/**
 * Block Addition and Validation
 */

/**
 * Add a block to the blockchain
 * Validates block and all transactions before adding.
 * May trigger chain reorganization if block is on alternate chain.
 * 
 * @param bc Blockchain instance
 * @param block Block to add
 * @return SHD_OK on success, error code on failure
 */
shd_result_t shd_blockchain_add_block(shd_blockchain_t *bc,
                                      const shd_block_t *block);

/**
 * Validate block structure and proof-of-work
 * Does not validate transactions or check if block connects to chain.
 * 
 * @param block Block to validate
 * @param difficulty Required difficulty
 * @return SHD_OK if valid
 */
shd_result_t shd_blockchain_validate_block_header(const shd_block_t *block,
                                                  uint64_t difficulty);

/**
 * Validate block completely (header + transactions)
 * 
 * @param bc Blockchain instance
 * @param block Block to validate
 * @return SHD_OK if valid
 */
shd_result_t shd_blockchain_validate_block(const shd_blockchain_t *bc,
                                           const shd_block_t *block);

/**
 * Validate single transaction
 * 
 * @param bc Blockchain instance
 * @param tx Transaction to validate
 * @param height Block height for context
 * @return SHD_OK if valid
 */
shd_result_t shd_blockchain_validate_transaction(const shd_blockchain_t *bc,
                                                 const shd_transaction_t *tx,
                                                 uint64_t height);

/**
 * Difficulty and Consensus
 */

/**
 * Calculate next block difficulty
 * 
 * @param bc Blockchain instance
 * @return Next difficulty value
 */
uint64_t shd_blockchain_get_next_difficulty(const shd_blockchain_t *bc);

/**
 * Calculate block reward for given height
 * 
 * @param height Block height
 * @return Block reward in atomic units
 */
uint64_t shd_blockchain_get_block_reward(uint64_t height);

/**
 * Get median timestamp of recent blocks
 * 
 * @param bc Blockchain instance
 * @param count Number of blocks to consider
 * @return Median timestamp
 */
uint64_t shd_blockchain_get_median_timestamp(const shd_blockchain_t *bc,
                                             size_t count);

/**
 * Chain Reorganization
 */

/**
 * Handle chain reorganization
 * Called when an alternate chain becomes longer than main chain.
 * 
 * @param bc Blockchain instance
 * @param fork_height Height where fork occurred
 * @param new_blocks Array of blocks forming new chain
 * @param block_count Number of blocks in new chain
 * @return SHD_OK on success
 */
shd_result_t shd_blockchain_reorganize(shd_blockchain_t *bc,
                                       uint64_t fork_height,
                                       const shd_block_t **new_blocks,
                                       size_t block_count);

/**
 * Find common ancestor of two chains
 * 
 * @param bc Blockchain instance
 * @param hash1 First chain tip hash
 * @param hash2 Second chain tip hash
 * @param ancestor_out Output ancestor block hash
 * @param height_out Output ancestor height
 * @return SHD_OK if found
 */
shd_result_t shd_blockchain_find_common_ancestor(const shd_blockchain_t *bc,
                                                 const uint8_t hash1[SHD_HASH_SIZE],
                                                 const uint8_t hash2[SHD_HASH_SIZE],
                                                 uint8_t ancestor_out[SHD_HASH_SIZE],
                                                 uint64_t *height_out);

/**
 * Utility Functions
 */

/**
 * Free block structure
 * 
 * @param block Block to free
 */
void shd_block_free(shd_block_t *block);

/**
 * Calculate block hash
 * 
 * @param header Block header
 * @param hash_out Output hash
 */
void shd_block_calculate_hash(const shd_block_header_t *header,
                              uint8_t hash_out[SHD_HASH_SIZE]);

/**
 * Verify proof-of-work
 * 
 * @param header Block header
 * @param difficulty Required difficulty
 * @return true if PoW is valid
 */
bool shd_block_verify_pow(const shd_block_header_t *header,
                          uint64_t difficulty);

/**
 * Get total size of block in bytes
 * 
 * @param block Block to measure
 * @return Size in bytes
 */
size_t shd_block_get_size(const shd_block_t *block);

/**
 * Serialize block to bytes
 * 
 * @param block Block to serialize
 * @param buffer Output buffer
 * @param buffer_size Size of buffer
 * @return Number of bytes written, or 0 on error
 */
size_t shd_block_serialize(const shd_block_t *block,
                           uint8_t *buffer,
                           size_t buffer_size);

/**
 * Deserialize block from bytes
 * 
 * @param buffer Input buffer
 * @param buffer_size Size of buffer
 * @param block_out Output block (caller must free)
 * @return SHD_OK on success
 */
shd_result_t shd_block_deserialize(const uint8_t *buffer,
                                   size_t buffer_size,
                                   shd_block_t **block_out);

#endif /* SHADOWCOIN_BLOCKCHAIN_H */