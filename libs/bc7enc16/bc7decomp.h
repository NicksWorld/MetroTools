#pragma once

#ifdef _MSC_VER
#define DETEX_INLINE_ONLY __forceinline
#define DETEX_RESTRICT __restrict
#else
#define DETEX_INLINE_ONLY
#define DETEX_RESTRICT
#endif

enum {
	/* Function returns false (invalid block) when the compressed block */
	/* is in a format not allowed to be generated by an encoder. */
	DETEX_DECOMPRESS_FLAG_ENCODE = 0x1,
	/* For compression formats that have opaque and non-opaque modes, */
	/* return false (invalid block) when the compressed block is encoded */
	/* using a non-opaque mode. */
	DETEX_DECOMPRESS_FLAG_OPAQUE_ONLY = 0x2,
	/* For compression formats that have opaque and non-opaque modes, */
	/* return false (invalid block) when the compressed block is encoded */
	/* using an opaque mode. */
	DETEX_DECOMPRESS_FLAG_NON_OPAQUE_ONLY = 0x4,
}; 

#ifdef __cplusplus
extern "C" {
#endif

bool detexDecompressBlockBPTC(const uint8_t * DETEX_RESTRICT bitstring, uint32_t mode_mask,
	uint32_t flags, uint8_t * DETEX_RESTRICT pixel_buffer);

#ifdef __cplusplus
}
#endif
