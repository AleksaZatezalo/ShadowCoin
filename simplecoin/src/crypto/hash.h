#ifndef SIMPLECOIN_HASH_H
#define SIMPLECOIN_HASH_H

#include <stdint.h>
#include <stddef.h>

/* Hash output sizes */
#define HASH_SIZE 32
#define HASH256_SIZE 32

/* Memory-hard PoW configuration */
#define POW_MEMORY_SIZE (64 * 1024 * 1024)  /* 64MB default */
#define POW_MEMORY_PASSES 3                  /* Argon2 iterations */
#define POW_PARALLELISM 1                    /* Single-threaded */

/* Hash digest structure */
typedef struct {
    uint8_t data[HASH_SIZE];
} Hash256;

/* ============================================================================
 * STANDARD HASHING API
 * ============================================================================
 * These functions provide standard SHA-256 hashing for transaction and
 * block data that doesn't require memory-hard properties.
 */

/**
 * Compute SHA-256 hash of data
 * 
 * @param output  Output buffer (must be HASH256_SIZE bytes)
 * @param input   Input data to hash
 * @param len     Length of input data
 */
void hash_sha256(uint8_t output[HASH256_SIZE], 
                 const uint8_t *input, 
                 size_t len);

/**
 * Compute double SHA-256 hash (SHA-256(SHA-256(data)))
 * Used for transaction IDs and block hashes (non-PoW contexts)
 * 
 * @param output  Output buffer (must be HASH256_SIZE bytes)
 * @param input   Input data to hash
 * @param len     Length of input data
 */
void hash_double_sha256(uint8_t output[HASH256_SIZE],
                        const uint8_t *input,
                        size_t len);

/**
 * Compute hash of a Hash256 structure
 * Convenience wrapper for hash chaining
 */
void hash_hash256(Hash256 *output, const Hash256 *input);

/* ============================================================================
 * MEMORY-HARD PROOF-OF-WORK API
 * ============================================================================
 * These functions implement the CPU-optimized, GPU-resistant mining algorithm
 * that combines SHA-256 with Argon2id memory expansion and random access.
 */

/**
 * Context for memory-hard hashing
 * Maintains state and scratch memory for mining operations
 */
typedef struct {
    uint8_t *memory_pool;        /* Large memory buffer for random access */
    size_t memory_size;          /* Size of memory pool in bytes */
    uint32_t time_cost;          /* Argon2 time parameter */
    uint32_t parallelism;        /* Argon2 parallelism parameter */
} MemoryHardContext;

/**
 * Initialize memory-hard context for mining
 * Allocates and prepares the large memory buffer
 * 
 * @param ctx          Context to initialize
 * @param memory_size  Size of memory pool (use POW_MEMORY_SIZE default)
 * @return 0 on success, -1 on failure
 */
int hash_memhard_init(MemoryHardContext *ctx, size_t memory_size);

/**
 * Free memory-hard context resources
 * 
 * @param ctx  Context to clean up
 */
void hash_memhard_free(MemoryHardContext *ctx);

/**
 * Compute memory-hard proof-of-work hash
 * 
 * This is the core mining function that combines:
 * 1. SHA-256 of block header + nonce
 * 2. Argon2id memory expansion using SHA result as seed
 * 3. Random memory reads across the pool
 * 4. Final hash computation
 * 
 * @param ctx           Memory-hard context (must be initialized)
 * @param output        Output hash (HASH256_SIZE bytes)
 * @param input         Block header data
 * @param input_len     Length of block header
 * @param nonce         Mining nonce
 * @param memory_nonce  Additional nonce for memory access pattern
 * @return 0 on success, -1 on failure
 */
int hash_memhard_pow(MemoryHardContext *ctx,
                     uint8_t output[HASH256_SIZE],
                     const uint8_t *input,
                     size_t input_len,
                     uint32_t nonce,
                     const uint32_t memory_nonce[16]);

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================
 */

/**
 * Compare two hashes in constant time
 * 
 * @return 1 if equal, 0 if not equal
 */
int hash_equal(const Hash256 *a, const Hash256 *b);

/**
 * Check if hash is below target difficulty
 * 
 * @param hash    Hash to check
 * @param target  Target difficulty (256-bit number as byte array)
 * @return 1 if hash < target, 0 otherwise
 */
int hash_below_target(const uint8_t hash[HASH256_SIZE],
                      const uint8_t target[HASH256_SIZE]);

/**
 * Convert hash to hex string
 * 
 * @param output  Output buffer (must be at least HASH256_SIZE*2 + 1 bytes)
 * @param hash    Hash to convert
 */
void hash_to_hex(char *output, const uint8_t hash[HASH256_SIZE]);

/**
 * Convert hex string to hash
 * 
 * @param output  Output hash buffer
 * @param hex     Hex string (must be HASH256_SIZE*2 characters)
 * @return 0 on success, -1 on invalid input
 */
int hash_from_hex(uint8_t output[HASH256_SIZE], const char *hex);

#endif /* SIMPLECOIN_HASH_H */