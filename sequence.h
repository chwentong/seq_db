/* 
 * sequence.h 
 *
 * @COPYLEFT
 * ALL WRONGS RESERVED
 *
 * Chen Wentong <chenwentong@genomics.cn>
 */
#ifndef T_SEQUENCE_H
#define T_SEQUENCE_H

#include <stdint.h>

typedef uint8_t ubyte_t;
typedef uint32_t ulen_t;

// about sequence_t
typedef struct _gap_list_t {
	ulen_t *start, *stop;
	ulen_t num;
} gap_list_t;

typedef struct _id_list_t {
	char **name;
	ulen_t *start;
	ulen_t num;
} id_list_t;

typedef struct _sequence_t {
	ubyte_t *seq;
	gap_list_t gap_list;
	id_list_t id_list;
	ulen_t len;
} sequence_t;

sequence_t *load_fasta(const char *fn);
sequence_t *load_sequence(const char *fn);
void free_sequence(sequence_t * st);
void dump_sequence(sequence_t * st, const char *fn);

#endif				// T_SEQUENCE_H
