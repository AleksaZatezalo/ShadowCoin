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

    /* Chain state*/
    uint64_t height;
    uint8_t top_hash[SHD_HASH_SIZE];
    uint64_t difficulty;
    uint64_t total_supply;
    uint64_t tx_count;

    /* Alternatyive chains tracking */
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

/** 
 * Internal helper functions
*/
static shd_result_t load_chain_state(shd_blockchain_t *bc);
static shd_result_t save_chain_state(shd_blockchain_t *bc);
static shd_result_t apply_block(shd_blockchain_t *bc, const shd_block_t *block);
static shd_result_t rollback_block(shd_blockchain_t *bc, const shd_block_t *block);
static shd_result_t validate_block_context(const shd_blockchain_t *bc, const shd_block_t *block);
static bool is_in_main_chain(const shd_blockchain_t *bc,  const uint8_t hash[SHD_HASH_SIZE]);

/**
 * Blockchain creation and initialization
 */

shd_blockchain_t* shd_blockchain_create(const char *data_dir){
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
    if(!bc->data_dir){
        free(bc);
        return NULL;
    }

    /* Open database */
    bc->db = shd_database_open(data_dir);
    if (!bc->db){
        shd_log_error("blockchain: failed");
        free(bc->data_dir);
        free(bc);
        return NULL;
    }

    /* Initialize caches */
    bc->cache.capacity = 100;
    bc->cache.blocks = calloc(bc->cache.capacity, sizeof(shd_block_t*));
    if (!bc->cache.blocks){
        shd_database_close(bc->db);
        free(bc->data_dir);
        free(bc);
        return NULL;
    }

    /* Load existing chain state or initialize */
    if (load_chain_state(bc) != SHD_OK){
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

void shd_blockchain_destroy(shd_blockchain_t *bc){
    if (!bc) return;

    /* Free cached blocks */
    for (size_t i = 0; i < bc->cache.count; i++){
        shd_block_free(bc->cache.blocks[i]);
    }
    free(bc->cache.blocks);

    /* Free alternative chains */
    for (size_t i = 0; i < bc->alt_chains.count; i++){
        shd_block_free(bc->alt_chains.blocks[i]);
    }
    free(bc->alt_chains.blocks);

    /* Close database */
    shd_database_close(bc-db);

    free(bc->data_dir);
    free(bc);
}

shd_result_t shd_blockchain_init_genesis(shd_blockchain_t *bc){
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