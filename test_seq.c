/* 
 * seq_test.c
 * 
 * @COPYLEFT
 * ALL WRONGS RESERVED
 *
 * Chen Wentong <chenwentong@genomics.cn>
 */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "sequence.h"

void display_sequence(sequence_t * st, const char *log);

int main(int argc, char **argv)
{
	char *fin, *fo1, *fo2, *log;
	sequence_t *st1, *st2;

	if (argc != 5) {
		printf("Usage: %s tt.fsa tt.seq.1 tt.seq.2 tt.log\n", argv[0]);
		exit(1);
	}

	fin = argv[1];
	fo1 = argv[2];
	fo2 = argv[3];
	log = argv[4];

	// load fasta testing
	st1 = load_fasta(fin);
	dump_sequence(st1, fo1);

	// load and dump sequence testing
	st2 = load_sequence(fo1);
	dump_sequence(st2, fo2);
	display_sequence(st2, log);

	// finishing test
	free_sequence(st1);
	free_sequence(st2);

	return 0;
}

void display_sequence(sequence_t * st, const char *log)
{
	ubyte_t *buf;
	ulen_t i, j;
	ulen_t len, gap_num, id_num;
	int fd, s_fd;
    s_fd = 0;

	if (log[0] != '-') {
		if ((fd = open(log, O_CREAT | O_WRONLY, 0644)) == -1) {
			fprintf(stderr,
				"[display_sequence] open %s error, %s.\n", log,
				strerror(errno));
			exit(1);
		}
		fflush(stdout);
		s_fd = dup(1);
		dup2(fd, 1);
		close(fd);
	}

	printf("displaying sequence: \n");

	len = st->len;
	buf = (ubyte_t *) malloc(len + 1);
	memcpy(buf, st->seq, len + 1);
	if (buf[len] != '$') {
		fprintf(stderr,
			"[display_sequence] check seq error, buf[len] != '$'.\n");
		exit(3);
	}
	// replacing gaps with 'N'
	gap_num = st->gap_list.num;
	for (i = 0; i < gap_num; ++i)
		memset(buf + st->gap_list.start[i], 'N',
		       st->gap_list.stop[i] - st->gap_list.start[i] + 1);

	// display each scaffolds
	id_num = st->id_list.num;
	for (i = 1; i < id_num; ++i) {
		printf(">%s\n", st->id_list.name[i - 1]);
		for (j = st->id_list.start[i - 1]; j < st->id_list.start[i];
		     ++j)
			putchar(buf[j]);
		putchar('\n');
	}

	printf(">%s\n", st->id_list.name[i - 1]);
	for (j = st->id_list.start[i - 1]; j < st->len; ++j)
		putchar(buf[j]);
	putchar('\n');

	if (log[0] != '-')
		fflush(stdout), dup2(s_fd, 1), close(s_fd);

	free(buf);
}
