/* 
 * sequence.c 
 *
 * @COPYLEFT 
 * ALL WRONGS RESERVED
 *
 * Chen Wentong <chenwentong@genomics.cn>
 */
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "sequence.h"

const ubyte_t atcg2num[128] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 0, 4, 1, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 0, 4, 1, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};				// A=a=0, C=c=1, G=g=2, T=t=3, else=4

void pack_sequence(sequence_t * st)
{
	ulen_t i, k, l, size;
	ubyte_t gapping;

	size = 32;
	st->gap_list.num = 0;
	st->gap_list.start = (ulen_t *) malloc(sizeof(ulen_t) * size);
	st->gap_list.stop = (ulen_t *) malloc(sizeof(ulen_t) * size);

	if (!st->gap_list.start || !st->gap_list.stop) {
		fprintf(stderr, "[pack_sequence] malloc error.\n");
		exit(2);
	}

	gapping = 0;
	for (i = k = l = 0; i < st->len; ++i) {
		if (atcg2num[st->seq[i]] < 4) {
			// st->seq[i] = atcg2num[st->seq[i]];
			if (gapping || (st->len == i + 1)) {	// new gap here
				st->gap_list.start[st->gap_list.num] = k;
				st->gap_list.stop[st->gap_list.num] = l;

				if (++(st->gap_list.num) > size) {
					size = size << 1;
					st->gap_list.start =
					    (ulen_t *) realloc(st->
							       gap_list.start,
							       sizeof(ulen_t) *
							       size);
					st->gap_list.stop =
					    (ulen_t *) realloc(st->
							       gap_list.stop,
							       sizeof(ulen_t) *
							       size);
				}

				gapping = 0;
			}
		} else {
			// st->seq[i] = 0;
			if (gapping)
				++l;
			else
				gapping = 1, k = l = i;
		}
	}

	if (gapping) {		// tailing gap 
		st->gap_list.start[st->gap_list.num] = k;
		st->gap_list.stop[st->gap_list.num] = l;
		++st->gap_list.num;
	}

	st->seq[st->len] = '$';	// magic flag
};

sequence_t *load_fasta(const char *fn)
{
	ulen_t idn, seq_len;
	char *buf, *ptr, *end;
	size_t n;
	ssize_t ret;
	sequence_t *st;
	FILE *fp;

	if ((fp = fopen(fn, "r")) == NULL) {
		fprintf(stderr, "[load_fasta] open %s error, %s.\n", fn,
			strerror(errno));
		exit(1);
	}

	buf = NULL;
	n = 0;
	if ((ret = getdelim(&buf, &n, '\0', fp)) == -1 || buf[0] != '>') {
		fprintf(stderr,
			"[load_fasta] error occurred while reading %s.\n", fn);
		fclose(fp);
		exit(1);
	}
	// count scaffold number
	n = strlen(buf);
	idn = 1;
	ptr = buf;
	end = buf + n;
	while ((ptr = strchr(++ptr, '>')) != NULL)
		++idn;

	// fill sequence_t *st;
	st = (sequence_t *) malloc(sizeof(sequence_t));
	st->seq = (ubyte_t *) malloc(n);
	st->id_list.num = idn;
	st->id_list.start = (ulen_t *) malloc(sizeof(ulen_t) * idn);
	st->id_list.name = (char **)malloc(sizeof(char *) * idn);

	if (!st->seq || !st->id_list.start || !st->id_list.name) {
		fprintf(stderr, "[load_fasta] malloc error.\n");
		exit(2);
	}

	idn = 0;
	seq_len = 0;
	ptr = end = buf;
	while ((end = strchr(ptr, '\n')) != NULL) {
		if (ptr[0] == '>') {
			st->id_list.name[idn] = strndup(ptr + 1, end - ptr - 1);
			st->id_list.start[idn] = seq_len;
			ptr = end + 1;
			++idn;
			continue;
		}

		strncpy((char *)st->seq + seq_len, ptr, end - ptr);
		seq_len += end - ptr;

		ptr = end + 1;
	}

	if (ptr < (buf + n) && ptr[0] != '>') {
		strncpy((char *)st->seq + seq_len, ptr, buf - ptr);
		seq_len += buf - ptr;
	}

	st->seq[seq_len] = '\0';
	st->len = seq_len;
	pack_sequence(st);

	free(buf);
	fclose(fp);
	return st;
}

void free_sequence(sequence_t * st)
{
	ulen_t i;

	// free id_list
	for (i = 0; i < st->id_list.num; ++i)
		free(st->id_list.name[i]);
	free(st->id_list.name);
	free(st->id_list.start);

	// free gap_list
	free(st->gap_list.start);
	free(st->gap_list.stop);

	// free sequence
	free(st->seq);

	// free sequence_t itself
	free(st);
}

void dump_sequence(sequence_t * st, const char *fn)
{
	FILE *fp;
	ulen_t i;

	if ((fp = fopen(fn, "w")) == NULL) {
		fprintf(stderr, "[dump_sequence] open %s error, %s.\n", fn,
			strerror(errno));
		exit(1);
	}
	// dump id list
	// fprintf(fp, "%u\tSEQ_MAGIC\n", st->id_list.num);
	fwrite(&st->id_list.num, sizeof(ulen_t), 1, fp);
	fputc('$', fp);		// magic flag
	for (i = 0; i < st->id_list.num; ++i) {
		// fprintf(fp, "%u\t%s\n", st->id_list.start[i], st->id_list.name[i]);
		fwrite(&st->id_list.start[i], sizeof(ulen_t), 1, fp);
		fputs(st->id_list.name[i], fp);
		fputc('\0', fp);	// magic flag
	}

	// dump gap list
	// fprintf(fp, "%u\tSEQ_MAGIC\n", st->gap_list.num);
	fwrite(&st->gap_list.num, sizeof(ulen_t), 1, fp);
	fputc('$', fp);		// magic flag
	for (i = 0; i < st->gap_list.num; ++i) {
		// fprintf(fp, "%u\t%u\n", st->gap_list.start[i], st->gap_list.stop[i]);
		fwrite(&st->gap_list.start[i], sizeof(ulen_t), 1, fp);
		fwrite(&st->gap_list.stop[i], sizeof(ulen_t), 1, fp);
	}

	// check sequence
	if (st->seq[st->len] != '$') {
		fprintf(stderr,
			"[dump_sequence] check seq error, st->[st->len] != '$'.\n");
		exit(3);
	}
	// dump sequence
	// fprintf(fp, "%u\tSEQ_MAGIC\n", st->len);
	fwrite(&st->len, sizeof(ulen_t), 1, fp);
	fputc('$', fp);		// magic flag
	fwrite(st->seq, 1, st->len + 1, fp);
	fputc('$', fp);		// magic flag

	fclose(fp);
}

sequence_t *load_sequence(const char *fn)
{
	sequence_t *st;
	char *buf;
	size_t n;
	ulen_t i;
	FILE *fp;

	buf = NULL;
	n = 0;

	if ((fp = fopen(fn, "r")) == NULL) {
		fprintf(stderr, "[load_sequence] open %s error, %s.\n", fn,
			strerror(errno));
		exit(1);
	}

	if ((st = (sequence_t *) malloc(sizeof(sequence_t))) == NULL) {
		fprintf(stderr, "[load_sequence] malloc error.\n");
		exit(2);
	}
	// see comments in dump_sequence
	// get id_list
	fread(&st->id_list.num, sizeof(ulen_t), 1, fp);
	if (fgetc(fp) != '$')
		goto LOAD_SEQ_ERROR;

	st->id_list.name = (char **)malloc(sizeof(char *) * st->id_list.num);
	st->id_list.start = (ulen_t *) malloc(sizeof(ulen_t) * st->id_list.num);

	if (!st->id_list.name || !st->id_list.start) {
		fprintf(stderr, "[load_sequence] malloc error.\n");
		exit(2);
	}

	for (i = 0; i < st->id_list.num; ++i) {
		fread(&st->id_list.start[i], sizeof(ulen_t), 1, fp);
		getdelim(&buf, &n, '\0', fp);
		st->id_list.name[i] = strdup(buf);
	}

	// get gap_list
	fread(&st->gap_list.num, sizeof(ulen_t), 1, fp);
	if (fgetc(fp) != '$')
		goto LOAD_SEQ_ERROR;

	st->gap_list.start =
	    (ulen_t *) malloc(sizeof(ulen_t) * st->gap_list.num);
	st->gap_list.stop =
	    (ulen_t *) malloc(sizeof(ulen_t) * st->gap_list.num);
	if (!st->gap_list.start || !st->gap_list.stop) {
		fprintf(stderr, "[load_sequence] malloc error.\n");
		exit(2);
	}

	for (i = 0; i < st->gap_list.num; ++i) {
		fread(&st->gap_list.start[i], sizeof(ulen_t), 1, fp);
		fread(&st->gap_list.stop[i], sizeof(ulen_t), 1, fp);
	}

	// get sequence
	fread(&st->len, sizeof(ulen_t), 1, fp);
	if (fgetc(fp) != '$')
		goto LOAD_SEQ_ERROR;

	st->seq = (ubyte_t *) malloc(st->len + 1);
	if (!st->seq) {
		fprintf(stderr, "[load_sequence] malloc error.\n");
		exit(2);
	}

	fread(st->seq, 1, st->len + 1, fp);
	if (st->seq[st->len] != '$') {
		fprintf(stderr,
			"[load_sequence] check seq error, st->seq[st->len] != '$'.\n");
		exit(3);
	}

	if (fgetc(fp) != '$')
		goto LOAD_SEQ_ERROR;

	free(buf);
	fclose(fp);
	return st;

 LOAD_SEQ_ERROR:
	fprintf(stderr, "[load_sequence] error occurred while reading file %s.",
		fn);
	exit(3);
	return NULL;
}
