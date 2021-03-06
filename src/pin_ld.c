/*
 * =====================================================================================
 *
 *       Filename:  scaff_gm.c
 *
 *    Description:  scaffolding with genetic mapping
 *
 *        Version:  1.0
 *        Created:  27/11/2018 21:37:03
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dengfeng Guan (D. Guan), dfguan@hit.edu.cn
 *   Organization:  Center for Bioinformatics, Harbin Institute of Technology
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <string.h>

#include "col_ld_lnks.h"
#include "build_graph.h"
#include "get_seq.h"

int main(int argc, char *argv[])
{
	if (argc < 2) {
help:
		fprintf(stderr, "\n  pin_ld [-v] [-h] <command> [<args>]\n");
		fprintf(stderr, "  commands:\n");
		fprintf(stderr, "           link        generate links for contigs\n");
		fprintf(stderr, "           build       generate graph with links\n");
		fprintf(stderr, "           getc        get contigs from graph\n");
		return 1;
	} else {
		if (!strcmp(argv[1], "link")) main_ld_lnks(argc , argv);
	   	else if (!strcmp(argv[1], "build")) main_bldg(argc , argv, 0);	
		else if (!strcmp(argv[1], "getc")) main_get_seq(argc, argv);
	   	else if (!strcmp(argv[1], "-h")) goto help;	
	   	else if (!strcmp(argv[1], "-v")) fprintf(stderr, "version: 0.0.0\n");	
		else {
			fprintf(stderr, "  [E::%s] unrecognized command %s\n", __func__, argv[1]);
			goto help;	
		}	
	}
	return 0;
}
