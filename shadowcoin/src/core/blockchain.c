/**
 * blockchain.c - ShadowCoin Blockchain Implementation
 */

#include "blockchain.h"
#include "../crypto/hash.h"
#include "../storage/database.h"
#include "../util/logger.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

/* Internal blockchain structure */
struct shd_blockchain {
    char *data_dir;
    shd_database_t *db;
    
    /* Chain state */
    uint64_t height;
    uint8_t top_hash[SHD_HASH_SIZE];
    uint64_t difficulty;
    uint64_t total_supply;
    uint64_t tx_count;
    
    /* Alternative chains tracking */
    struct {
        shd_block_t **blocks;
        size_t count;
        size_t capacity;
    } alt_chains;
    
    /* Cache for recent blocks */
    struct {
        shd_block_t **blocks;
        size_t count;
        size_t capacity;
    } cache;
};

/* Genesis block constants */
static const uint8_t GENESIS_PREV_HASH[SHD_HASH_SIZE] = {0};
static const uint64_t GENESIS_TIMESTAMP = 1704067200; /* 2024-01-01 00:00:00 UTC */
static const uint64_t INITIAL_DIFFICULTY = 1000000;

/* Coin constant (9 decimals) */
#define COIN 1000000000ULL

/*
 * Internal helper functions
 */

static shd_result_t load_chain_state(shd_blockchain_t *bc);
static shd_result_t save_chain_state(shd_blockchain_t *bc);
static shd_result_t apply_block(shd_blockchain_t *bc, const shd_block_t *block);
static shd_result_t rollback_block(shd_blockchain_t *bc, const shd_block_t *block);
static shd_result_t validate_block_context(const shd_blockchain_t *bc, 
                                           const shd_block_t *block);
static bool is_in_main_chain(const shd_blockchain_t *bc, 
                             const uint8_t hash[SHD_HASH_SIZE]);

/*
 * Blockchain creation and initialization
 */

shd_blockchain_t* shd_blockchain_create(const char *data_dir) {
    if (!data_dir) {
        shd_log_error("blockchain: data_dir is NULL");
        return NULL;
    }
    
    shd_blockchain_t *bc = calloc(1, sizeof(shd_blockchain_t));
    if (!bc) {
        shd_log_error("blockchain: out of memory");
        return NULL;
    }
    
    bc->data_dir = strdup(data_dir);
    if (!bc->data_dir) {
        free(bc);
        return NULL;
    }
    
    /* Open database */
    bc->db = shd_database_open(data_dir);
    if (!bc->db) {
        shd_log_error("blockchain: failed to open database");
        free(bc->data_dir);
        free(bc);
        return NULL;
    }
    
    /* Initialize caches */
    bc->cache.capacity = 100;
    bc->cache.blocks = calloc(bc->cache.capacity, sizeof(shd_block_t*));
    if (!bc->cache.blocks) {
        shd_database_close(bc->db);
        free(bc->data_dir);
        free(bc);
        return NULL;
    }
    
    /* Load existing chain state or initialize */
    if (load_chain_state(bc) != SHD_OK) {
        /* No existing chain, needs genesis */
        bc->height = 0;
        bc->difficulty = INITIAL_DIFFICULTY;
        bc->total_supply = 0;
        bc->tx_count = 0;
        memset(bc->top_hash, 0, SHD_HASH_SIZE);
    }
    
    shd_log_info("blockchain: created at height %lu", bc->height);
    return bc;
}

void shd_blockchain_destroy(shd_blockchain_t *bc) {
    if (!bc) return;
    
    /* Free cached blocks */
    for (size_t i = 0; i < bc->cache.count; i++) {
        shd_block_free(bc->cache.blocks[i]);
    }
    free(bc->cache.blocks);
    
    /* Free alternative chains */
    for (size_t i = 0; i < bc->alt_chains.count; i++) {
        shd_block_free(bc->alt_chains.blocks[i]);
    }
    free(bc->alt_chains.blocks);
    
    /* Close database */
    shd_database_close(bc->db);
    
    free(bc->data_dir);
    free(bc);
}

shd_result_t shd_blockchain_init_genesis(shd_blockchain_t *bc) {
    if (!bc) return SHD_ERROR_INVALID_ARGUMENT;
    
    if (bc->height > 0) {
        shd_log_error("blockchain: genesis already exists");
        return SHD_ERROR_GENERIC;
    }
    
    /* Create genesis block */
    shd_block_t *genesis = calloc(1, sizeof(shd_block_t));
    if (!genesis) return SHD_ERROR_OUT_OF_MEMORY;
    
    /* Fill genesis header */
    genesis->header.version = 1;
    genesis->header.height = 0;
    genesis->header.timestamp = GENESIS_TIMESTAMP;
    memcpy(genesis->header.prev_hash, GENESIS_PREV_HASH, SHD_HASH_SIZE);
    genesis->header.nonce = 0;
    genesis->header.difficulty = INITIAL_DIFFICULTY;
    
    /* Calculate genesis hash */
    shd_block_calculate_hash(&genesis->header, genesis->hash);
    
    /* Create genesis coinbase transaction */
    genesis->miner_tx = calloc(1, sizeof(shd_transaction_t));
    if (!genesis->miner_tx) {
        free(genesis);
        return SHD_ERROR_OUT_OF_MEMORY;
    }
    
    /* Genesis has no other transactions */
    genesis->transactions = NULL;
    genesis->tx_count = 0;
    
    /* Add genesis block */
    shd_result_t result = apply_block(bc, genesis);
    if (result != SHD_OK) {
        shd_block_free(genesis);
        return result;
    }
    
    shd_log_info("blockchain: genesis block created");
    shd_block_free(genesis);
    
    return SHD_OK;
}

/*
 * Query functions
 */

uint64_t shd_blockchain_get_height(const shd_blockchain_t *bc) {
    return bc ? bc->height : 0;
}

shd_result_t shd_blockchain_get_stats(const shd_blockchain_t *bc,
                                      shd_blockchain_stats_t *stats) {
    if (!bc || !stats) return SHD_ERROR_INVALID_ARGUMENT;
    
    stats->height = bc->height;
    stats->tx_count = bc->tx_count;
    stats->total_supply = bc->total_supply;
    stats->difficulty = bc->difficulty;
    memcpy(stats->top_hash, bc->top_hash, SHD_HASH_SIZE);
    
    /* Estimate hashrate: difficulty * target_time */
    stats->hashrate = bc->difficulty / SHD_BLOCK_TIME_TARGET;
    
    return SHD_OK;
}

shd_result_t shd_blockchain_get_block_by_height(const shd_blockchain_t *bc,
                                                uint64_t height,
                                                shd_block_t **block_out) {
    if (!bc || !block_out) return SHD_ERROR_INVALID_ARGUMENT;
    
    if (height > bc->height) return SHD_ERROR_NOT_FOUND;
    
    /* Check cache first */
    for (size_t i = 0; i < bc->cache.count; i++) {
        if (bc->cache.blocks[i]->header.height == height) {
            *block_out = bc->cache.blocks[i];
            return SHD_OK;
        }
    }
    
    /* Load from database */
    return shd_database_get_block_by_height(bc->db, height, block_out);
}

shd_result_t shd_blockchain_get_block_by_hash(const shd_blockchain_t *bc,
                                              const uint8_t hash[SHD_HASH_SIZE],
                                              shd_block_t **block_out) {
    if (!bc || !hash || !block_out) return SHD_ERROR_INVALID_ARGUMENT;
    
    /* Check cache first */
    for (size_t i = 0; i < bc->cache.count; i++) {
        if (memcmp(bc->cache.blocks[i]->hash, hash, SHD_HASH_SIZE) == 0) {
            *block_out = bc->cache.blocks[i];
            return SHD_OK;
        }
    }
    
    /* Load from database */
    return shd_database_get_block_by_hash(bc->db, hash, block_out);
}

bool shd_blockchain_has_block(const shd_blockchain_t *bc,
                              const uint8_t hash[SHD_HASH_SIZE]) {
    if (!bc || !hash) return false;
    
    shd_block_t *block = NULL;
    shd_result_t result = shd_blockchain_get_block_by_hash(bc, hash, &block);
    
    return (result == SHD_OK);
}

bool shd_blockchain_is_key_image_spent(const shd_blockchain_t *bc,
                                       const uint8_t key_image[SHD_HASH_SIZE]) {
    if (!bc || !key_image) return false;
    
    return shd_database_has_key_image(bc->db, key_image);
}

/*
 * Block validation
 */

shd_result_t shd_blockchain_validate_block_header(const shd_block_t *block,
                                                  uint64_t difficulty) {
    if (!block) return SHD_ERROR_INVALID_ARGUMENT;
    
    const shd_block_header_t *header = &block->header;
    
    /* Version check */
    if (header->version != 1) {
        shd_log_error("blockchain: invalid block version %u", header->version);
        return SHD_ERROR_INVALID_BLOCK;
    }
    
    /* Timestamp sanity check (not more than 2 hours in future) */
    uint64_t current_time = (uint64_t)time(NULL);
    if (header->timestamp > current_time + 7200) {
        shd_log_error("blockchain: block timestamp too far in future");
        return SHD_ERROR_INVALID_BLOCK;
    }
    
    /* Verify proof-of-work */
    if (!shd_block_verify_pow(header, difficulty)) {
        shd_log_error("blockchain: invalid proof-of-work");
        return SHD_ERROR_INVALID_POW;
    }
    
    return SHD_OK;
}

shd_result_t shd_blockchain_validate_block(const shd_blockchain_t *bc,
                                           const shd_block_t *block) {
    if (!bc || !block) return SHD_ERROR_INVALID_ARGUMENT;
    
    /* Validate header */
    shd_result_t result = shd_blockchain_validate_block_header(block, bc->difficulty);
    if (result != SHD_OK) return result;
    
    /* Validate context (connections to chain) */
    result = validate_block_context(bc, block);
    if (result != SHD_OK) return result;
    
    /* Validate block size */
    size_t block_size = shd_block_get_size(block);
    if (block_size > SHD_MAX_BLOCK_SIZE) {
        shd_log_error("blockchain: block too large (%zu > %d)", 
                     block_size, SHD_MAX_BLOCK_SIZE);
        return SHD_ERROR_INVALID_BLOCK;
    }
    
    /* Validate coinbase transaction */
    if (!block->miner_tx) {
        shd_log_error("blockchain: missing coinbase transaction");
        return SHD_ERROR_INVALID_BLOCK;
    }
    
    uint64_t expected_reward = shd_blockchain_get_block_reward(block->header.height);
    /* TODO: Validate coinbase reward matches expected */
    
    /* Validate all transactions */
    for (size_t i = 0; i < block->tx_count; i++) {
        result = shd_blockchain_validate_transaction(bc, block->transactions[i],
                                                     block->header.height);
        if (result != SHD_OK) {
            shd_log_error("blockchain: invalid transaction %zu", i);
            return result;
        }
    }
    
    return SHD_OK;
}

shd_result_t shd_blockchain_validate_transaction(const shd_blockchain_t *bc,
                                                 const shd_transaction_t *tx,
                                                 uint64_t height) {
    if (!bc || !tx) return SHD_ERROR_INVALID_ARGUMENT;
    
    /* TODO: Implement full transaction validation:
     * - Verify ring signatures
     * - Check key images not spent
     * - Verify range proofs
     * - Check commitment sums
     * - Validate fee
     */
    
    return SHD_OK;
}

/*
 * Block addition
 */

shd_result_t shd_blockchain_add_block(shd_blockchain_t *bc,
                                      const shd_block_t *block) {
    if (!bc || !block) return SHD_ERROR_INVALID_ARGUMENT;
    
    /* Check if we already have this block */
    if (shd_blockchain_has_block(bc, block->hash)) {
        return SHD_ERROR_DUPLICATE_BLOCK;
    }
    
    /* Validate block completely */
    shd_result_t result = shd_blockchain_validate_block(bc, block);
    if (result != SHD_OK) return result;
    
    /* Check if block extends main chain */
    if (memcmp(block->header.prev_hash, bc->top_hash, SHD_HASH_SIZE) == 0) {
        /* Block extends main chain */
        result = apply_block(bc, block);
        if (result == SHD_OK) {
            shd_log_info("blockchain: added block %lu", block->header.height);
        }
        return result;
    }
    
    /* Block is on alternative chain - check if it's better */
    /* TODO: Implement full chain reorganization logic */
    shd_log_info("blockchain: received alternative chain block");
    
    return SHD_ERROR_ORPHAN_BLOCK;
}

/*
 * Difficulty calculation
 */

uint64_t shd_blockchain_get_next_difficulty(const shd_blockchain_t *bc) {
    if (!bc) return INITIAL_DIFFICULTY;
    
    /* Check if adjustment is needed */
    if (bc->height < SHD_DIFFICULTY_WINDOW) {
        return bc->difficulty;
    }
    
    if (bc->height % SHD_DIFFICULTY_WINDOW != 0) {
        return bc->difficulty;
    }
    
    /* Get timestamps for difficulty window */
    shd_block_t *old_block = NULL;
    shd_block_t *new_block = NULL;
    
    shd_blockchain_get_block_by_height(bc, bc->height - SHD_DIFFICULTY_WINDOW, 
                                       &old_block);
    shd_blockchain_get_block_by_height(bc, bc->height, &new_block);
    
    if (!old_block || !new_block) {
        return bc->difficulty;
    }
    
    uint64_t time_actual = new_block->header.timestamp - old_block->header.timestamp;
    uint64_t time_expected = SHD_DIFFICULTY_WINDOW * SHD_BLOCK_TIME_TARGET;
    
    /* Limit adjustment to ±25% */
    uint64_t min_time = (time_expected * 3) / 4;
    uint64_t max_time = (time_expected * 5) / 4;
    
    if (time_actual < min_time) time_actual = min_time;
    if (time_actual > max_time) time_actual = max_time;
    
    /* Calculate new difficulty */
    uint64_t new_difficulty = (bc->difficulty * time_expected) / time_actual;
    
    /* Prevent difficulty from going to zero */
    if (new_difficulty < 1000) new_difficulty = 1000;
    
    return new_difficulty;
}

uint64_t shd_blockchain_get_block_reward(uint64_t height) {
    uint64_t base_reward = 50 * COIN;
    uint64_t halvings = height / SHD_HALVING_INTERVAL;
    
    if (halvings >= 64) return 0;
    
    return base_reward >> halvings;
}

uint64_t shd_blockchain_get_median_timestamp(const shd_blockchain_t *bc,
                                             size_t count) {
    if (!bc || count == 0) return 0;
    
    if (count > bc->height + 1) {
        count = bc->height + 1;
    }
    
    uint64_t *timestamps = calloc(count, sizeof(uint64_t));
    if (!timestamps) return 0;
    
    /* Collect timestamps */
    for (size_t i = 0; i < count; i++) {
        shd_block_t *block = NULL;
        uint64_t height = bc->height - i;
        
        if (shd_blockchain_get_block_by_height(bc, height, &block) == SHD_OK) {
            timestamps[i] = block->header.timestamp;
        }
    }
    
    /* Sort timestamps (simple bubble sort for small arrays) */
    for (size_t i = 0; i < count - 1; i++) {
        for (size_t j = 0; j < count - i - 1; j++) {
            if (timestamps[j] > timestamps[j + 1]) {
                uint64_t temp = timestamps[j];
                timestamps[j] = timestamps[j + 1];
                timestamps[j + 1] = temp;
            }
        }
    }
    
    uint64_t median = timestamps[count / 2];
    free(timestamps);
    
    return median;
}

/*
 * Utility functions
 */

void shd_block_free(shd_block_t *block) {
    if (!block) return;
    
    if (block->miner_tx) {
        /* TODO: Free transaction properly */
        free(block->miner_tx);
    }
    
    for (size_t i = 0; i < block->tx_count; i++) {
        /* TODO: Free transaction properly */
        free(block->transactions[i]);
    }
    free(block->transactions);
    
    free(block);
}

void shd_block_calculate_hash(const shd_block_header_t *header,
                              uint8_t hash_out[SHD_HASH_SIZE]) {
    if (!header || !hash_out) return;
    
    /* Serialize header and hash */
    uint8_t buffer[256];
    size_t offset = 0;
    
    memcpy(buffer + offset, &header->version, sizeof(header->version));
    offset += sizeof(header->version);
    
    memcpy(buffer + offset, &header->timestamp, sizeof(header->timestamp));
    offset += sizeof(header->timestamp);
    
    memcpy(buffer + offset, header->prev_hash, SHD_HASH_SIZE);
    offset += SHD_HASH_SIZE;
    
    memcpy(buffer + offset, &header->nonce, sizeof(header->nonce));
    offset += sizeof(header->nonce);
    
    /* Hash the header */
    shd_hash_sha3(buffer, offset, hash_out);
}

bool shd_block_verify_pow(const shd_block_header_t *header,
                          uint64_t difficulty) {
    if (!header) return false;
    
    uint8_t hash[SHD_HASH_SIZE];
    shd_block_calculate_hash(header, hash);
    
    /* Convert hash to uint64 for comparison */
    uint64_t hash_value = 0;
    for (int i = 0; i < 8; i++) {
        hash_value |= ((uint64_t)hash[i]) << (i * 8);
    }
    
    uint64_t target = UINT64_MAX / difficulty;
    
    return hash_value < target;
}

size_t shd_block_get_size(const shd_block_t *block) {
    if (!block) return 0;
    
    /* TODO: Calculate actual serialized size */
    size_t size = sizeof(shd_block_header_t);
    
    /* Add transaction sizes */
    /* This is placeholder - need actual transaction size calculation */
    size += block->tx_count * 1024;
    
    return size;
}

/*
 * Internal helper functions
 */

static shd_result_t load_chain_state(shd_blockchain_t *bc) {
    /* TODO: Load chain state from database */
    return SHD_ERROR_NOT_FOUND;
}

static shd_result_t save_chain_state(shd_blockchain_t *bc) {
    /* TODO: Save chain state to database */
    return SHD_OK;
}

static shd_result_t apply_block(shd_blockchain_t *bc, const shd_block_t *block) {
    if (!bc || !block) return SHD_ERROR_INVALID_ARGUMENT;
    
    /* Store block in database */
    shd_result_t result = shd_database_put_block(bc->db, block);
    if (result != SHD_OK) return result;
    
    /* Update chain state */
    bc->height = block->header.height;
    memcpy(bc->top_hash, block->hash, SHD_HASH_SIZE);
    bc->difficulty = shd_blockchain_get_next_difficulty(bc);
    bc->tx_count += block->tx_count + 1; /* +1 for coinbase */
    bc->total_supply += shd_blockchain_get_block_reward(block->header.height);
    
    /* Store key images */
    for (size_t i = 0; i < block->tx_count; i++) {
        /* TODO: Extract and store key images from transaction */
    }
    
    /* Save chain state */
    save_chain_state(bc);
    
    return SHD_OK;
}

static shd_result_t rollback_block(shd_blockchain_t *bc, const shd_block_t *block) {
    /* TODO: Implement block rollback for reorganization */
    return SHD_OK;
}

static shd_result_t validate_block_context(const shd_blockchain_t *bc,
                                           const shd_block_t *block) {
    if (!bc || !block) return SHD_ERROR_INVALID_ARGUMENT;
    
    /* Check previous block exists */
    if (!shd_blockchain_has_block(bc, block->header.prev_hash)) {
        shd_log_error("blockchain: previous block not found");
        return SHD_ERROR_ORPHAN_BLOCK;
    }
    
    /* Verify height is correct */
    shd_block_t *prev_block = NULL;
    shd_blockchain_get_block_by_hash(bc, block->header.prev_hash, &prev_block);
    
    if (prev_block) {
        if (block->header.height != prev_block->header.height + 1) {
            shd_log_error("blockchain: incorrect block height");
            return SHD_ERROR_INVALID_BLOCK;
        }
        
        /* Check timestamp is after previous block */
        if (block->header.timestamp <= prev_block->header.timestamp) {
            shd_log_error("blockchain: timestamp not increasing");
            return SHD_ERROR_INVALID_BLOCK;
        }
    }
    
    return SHD_OK;
}

static bool is_in_main_chain(const shd_blockchain_t *bc,
                             const uint8_t hash[SHD_HASH_SIZE]) {
    if (!bc || !hash) return false;
    
    shd_block_t *block = NULL;
    if (shd_blockchain_get_block_by_hash(bc, hash, &block) != SHD_OK) {
        return false;
    }
    
    /* Check if this block's height matches main chain */
    shd_block_t *main_block = NULL;
    shd_blockchain_get_block_by_height(bc, block->header.height, &main_block);
    
    if (!main_block) return false;
    
    return memcmp(main_block->hash, hash, SHD_HASH_SIZE) == 0;
}