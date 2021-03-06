/*
 * =====================================================================================
 *
 *       Filename:  break_pins.c
 *
 *    Description:	generate breaks for a scaffold
 *
 *        Version:  1.0
 *        Created:  14/11/2019 09:39:40
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dengfeng Guan (D.G.), dfguan9@gmail.com
 *   Organization:  Harbin Institute of Technology
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>	

#include "graph.h" 
#include "sdict.h"
#include "cdict.h"
#include "bed.h"
#include "kvec.h"
#include "utls.h"
#include "asset.h"
#include "bamlite.h"
#include "ksort.h"

#define BC_LEN 16
#define MIN_MAQ 20
typedef struct {
	/*int mq:15, rev:1, as:16;*/
	uint32_t s, ns;
   	uint32_t tid:31, rev:1;
	int qual;
}aln_inf_t;

typedef struct {
	uint32_t s, e; //no more than 
	uint64_t bctn;
}bc_t;

typedef struct {
	uint32_t n, m;
	bc_t *ary;
}bc_ary_t;

#define bc_key(a) ((a).bctn)
KRADIX_SORT_INIT(mbbct, bc_t, bc_key, 8)

#define cord_key(a) ((a).s)
KRADIX_SORT_INIT(cord, cors, cord_key, 4);
typedef struct {
	float maxim, avg;
} cov_stat;

typedef struct {
	uint32_t cid;
	uint32_t c1s:31, c1rev:1;
	uint32_t c2s:31, c2rev:1;
}hit2_t;

typedef struct {
	uint32_t n, m;
	hit2_t *ary;
}hit2_ary_t;

void mb_bc_ary_push(bc_ary_t *l, bc_t *z)
{
	uint32_t max = -1;
	if (l->n >= l->m) {
		if (l->m > (max >> 1)) {
			fprintf(stderr, "Too many values here\n");
			exit(1);
		} else 
			l->m = l->m ? l->m << 1 : 16;
		l->ary = realloc(l->ary, sizeof(bc_t) * l->m);//beware of overflow
	}
	l->ary[l->n++] = *z;
}
void hit2_ary_push(hit2_ary_t *l, hit2_t *z)
{
	uint32_t max = -1;
	if (l->n >= l->m) {
		if (l->m > (max >> 1)) {
			fprintf(stderr, "Too many values here\n");
			exit(1);
		} else 
			l->m = l->m ? l->m << 1 : 16;
		l->ary = realloc(l->ary, sizeof(hit2_t) * l->m);//beware of overflow
	}
	l->ary[l->n++] = *z;
}

int cmp_hits2(const void *a, const void *b)
{
	hit2_t *m = (hit2_t *)a;
	hit2_t *n = (hit2_t *)b; //too many branches
	if (m->c1s > n->c1s) return 1;	
	else if (m->c1s < n->c1s) return -1;	
	else if (m->c1rev > n->c1rev) return 1;
	else if (m->c1rev < n->c1rev) return -1;
	else if (m->c2s > n->c2s) return 1;	
	else if (m->c2s < n->c2s) return -1;	
	else if (m->c2rev > n->c2rev) return 1;	
	else if (m->c2rev < n->c2rev) return -1;	
	else return 0;
}

int mb_col_contacts(hit2_ary_t *hit_ary, sdict_t *sd, ctg_pos_t *d)
{
	size_t i, j;
	/*sdict_t *use_sd = sd;*/
	hit2_t *hs = hit_ary->ary;
	size_t n = hit_ary->n;
	for (i = 0, j = 1; j <= n; ++j) {
		if (j == n || hs[i].cid != hs[j].cid || hs[i].c1s != hs[j].c1s || hs[i].c1rev != hs[j].c1rev || hs[i].c2s != hs[j].c2s || hs[i].c2rev != hs[j].c2rev) {
			/*if (hs[i].c2s > sd->seq[hs[i].cid].len) fprintf(stderr, "much larger\n");*/
			/*fprintf(stderr, "%s\t%u\t%u\n", sd->seq[hs[i].cid].name, hs[i].c1s, hs[i].c2s);*/
			pos_push(&d->ctg_pos[hs[i].cid], hs[i].c1s << 1);	
			pos_push(&d->ctg_pos[hs[i].cid], hs[i].c2s << 1 | 1);	
			i = j;	
		}
	}
	return 0;
}


int mb_col_hits2(aln_inf_t *f, int f_cnt, sdict_t *ctgs, sdict_t *scfs, hit2_ary_t *hit2_ary, int min_mq, char *qn)
{
	if (f[0].qual < min_mq || f[1].qual < min_mq) return 1;
	if (scfs->n_seq) {
		/*if (a_cnt == 2) {*/
			/*fprintf(stderr, "%u\t%u\n", a[0].tid, a[1].tid);*/
			/*sd_seq_t *sq1 = &ctgs->seq[a[0].tid];*/
			/*sd_seq_t *sq2 = &ctgs->seq[a[1].tid];*/

			/*uint32_t ind1 = sq1->le; //maybe not well paired up*/
			/*uint32_t ind2 = sq2->le;*/
			/*if (ind1 == ind2) return 1;*/
			/*fprintf(stderr, "%s\t%s\t%u\t%u\n", sq1->name, sq2->name, ind1, ind2);*/
			/*fprintf(stderr, "%s\t%s\n", r->ctgn1, r->ctgn2)	;*/
			/*uint32_t a0s = sq1->l_snp_n == a[0].rev ? sq1->rs + a[0].s : sq1->rs + sq1->len - a[0].s; */
			/*uint32_t a1s = sq2->l_snp_n == a[1].rev ? sq2->rs + a[1].s : sq2->rs + sq2->len - a[1].s; */
			
			/*if (ind1 < ind2) {*/
				/*uint64_t c1ns = (uint64_t)ind1 << 32 | a0s; //don't think there will be 2G contig, if happends might be a bug */
				/*uint64_t c2ns = (uint64_t)ind2 << 32 | a1s; //don't think there will be 2G contig, if happends might be a bug */
				/*hit_t h = (hit_t) {c1ns, a[0].rev, c2ns, a[1].rev}; */
				/*hit_ary_push(hit_ary, &h);	*/
			/*} else {*/
				/*uint64_t c1ns = (uint64_t)ind2 << 32 | a1s; //don't think there will be 2G contig, if happends might be a bug */
				/*uint64_t c2ns = (uint64_t)ind1 << 32 | a0s; //don't think there will be 2G contig, if happends might be a bug */
				/*hit_t h = (hit_t) {c1ns, a[1].rev, c2ns, a[0].rev}; */
				/*hit_ary_push(hit_ary, &h);	*/
			/*}*/
			/*return 0;*/
		/*} else if (f_cnt == 2){*/
			sd_seq_t *sq1 = &ctgs->seq[f[0].tid];
			sd_seq_t *sq2 = &ctgs->seq[f[1].tid];
			uint32_t ind1 = sq1->le; //maybe not well paired up
			uint32_t ind2 = sq2->le;
			/*uint32_t f0s = sq1->r_snp_n + f[0].s; */
			/*uint32_t f1s = sq2->r_snp_n + f[1].s; */
			uint32_t f0s = sq1->r_snp_n + (sq1->l_snp_n & 0x1 ? f[0].s : sq1->len - f[0].s);
			uint32_t f1s = sq2->r_snp_n + (sq2->l_snp_n & 0x1? f[1].s : sq2->len - f[1].s);
#ifdef DEBUG
			fprintf(stdout, "%s\t%u\t%u\t%s/1\t%d\t%c\n", scfs->seq[ind1].name, f0s, f0s + 100, qn, f[0].qual, (f[0].rev ^ (sq1->l_snp_n & 0x1)) ? '-':'+');
			fprintf(stdout, "%s\t%u\t%u\t%s/2\t%d\t%c\n", scfs->seq[ind2].name, f1s, f1s + 100, qn, f[1].qual, (f[1].rev ^ (sq2->l_snp_n & 0x1)) ? '-':'+');
#endif
			if (ind1 != ind2) return 1;
			if (f[0].tid > f[1].tid) {
				if (f[0].tid - f[1].tid > 1) return 1;
			} else {
				if (f[1].tid - f[0].tid > 1) return 1;
			}	
			/*fprintf(stderr, "%s\t%u\t%s\t%u\n", sq1->name, f[0].s, sq2->name, f[1].s);*/
			/*fprintf(stderr, "%s\t%u\t%u\t%u\t%u\t%u\t%u\n", scfs->seq[ind1].name, sq1->r_snp_n, scfs->seq[ind1].len, f0s, f1s, f[0].s, f[1].s);*/
			/*if (scfs->seq[ind1].len < f0s || scfs->seq[ind1].len < f1s) fprintf(stderr, "larger\n");*/
			/*fprintf(stderr, "%u\t%u\n", ind1, ind2);*/
			/*fprintf(stderr, "%s\t%s\n", r->ctgn1, r->ctgn2)	;*/
			/*uint32_t f0s = sq1->l_snp_n == f[0].rev ? sq1->rs + f[0].s : sq1->rs + sq1->len - f[0].s; */
			/*uint32_t f1s = sq1->l_snp_n == f[1].rev ? sq2->rs + f[1].s : sq2->rs + sq2->len - f[1].s; */
			/*fprintf(stderr, "%s\t%u\t%d\t%d\t%d\t%s\t%u\t%d\t%d\t%d\n", scfs->seq[ind1].name, f0s, f[0].qual, sq1->l_snp_n, f[0].rev, scfs->seq[ind2].name, f1s, f[1].qual, sq2->l_snp_n, f[1].rev);*/
			if (f0s < f1s) {
				hit2_t h = (hit2_t) {ind1, f0s, !!(f[0].rev ^ (sq1->l_snp_n & 0x1)), f1s, !!(f[1].rev ^ (sq2->l_snp_n & 0x1))}; 
				hit2_ary_push(hit2_ary, &h);	
			} else {
				hit2_t h = (hit2_t) {ind2, f1s, !!(f[1].rev ^ (sq2->l_snp_n & 0x1)), f0s, !!(f[0].rev ^ (sq1->l_snp_n&0x1))}; 
				hit2_ary_push(hit2_ary, &h);	
			}
			return 0;	
		/*}*/
	} else {
		/*if (a_cnt == 2) {*/
			/*uint32_t ind1 = a[0].tid; //maybe not well paired up*/
			/*uint32_t ind2 = a[1].tid;*/
			/*if (ind1 == ind2) return 1;*/
			/*fprintf(stderr, "%u\t%u\n", ind1, ind2);*/
			/*fprintf(stderr, "%s\t%s\n", r->ctgn1, r->ctgn2)	;*/
			/*uint32_t is_l1 = check_left_half(ctgs->seq[ind1].le, ctgs->seq[ind1].rs, a[0].s);*/
			/*if (is_l1 > 1) return 1; //middle won't be added*/
			/*uint32_t is_l2 = check_left_half(ctgs->seq[ind2].le, ctgs->seq[ind2].rs, a[1].s);*/
			/*if (is_l2 > 1) return 1; //middle won't be added*/
			
			/*if (ind1 < ind2) {*/
				/*uint64_t c1ns = (uint64_t)ind1 << 32 | a[0].s; //don't think there will be 2G contig, if happends might be a bug */
				/*uint64_t c2ns = (uint64_t)ind2 << 32 | a[1].s; //don't think there will be 2G contig, if happends might be a bug */
				/*hit_t h = (hit_t) {c1ns, a[0].rev, c2ns, a[1].rev}; */
				/*hit_ary_push(hit_ary, &h);	*/
			/*} else {*/
				/*uint64_t c1ns = (uint64_t)ind2 << 32 | a[1].s; //don't think there will be 2G contig, if happends might be a bug */
				/*uint64_t c2ns = (uint64_t)ind1 << 32 | a[0].s; //don't think there will be 2G contig, if happends might be a bug */
				/*hit_t h = (hit_t) {c1ns, a[1].rev, c2ns, a[0].rev}; */
				/*hit_ary_push(hit_ary, &h);	*/
			/*}*/
			/*return 0;*/
		/*} else if (f_cnt == 2){*/
			uint32_t ind1 = f[0].tid;
			uint32_t ind2 = f[1].tid;
			if (ind1 != ind2) return 1;
			uint32_t f0s = f[0].s;
			uint32_t f1s = f[1].s;
			/*fprintf(stderr, "%s\t%s\n", r->ctgn1, r->ctgn2)	;*/
			if (f0s < f1s) {
				hit2_t h = (hit2_t) {ind1, f0s, f[0].rev, f1s, f[1].rev}; 
				hit2_ary_push(hit2_ary, &h);	
			} else {
				/*uint64_t c1ns = (uint64_t)ind2 << 32 | f[1].s; //don't think there will be 2G contig, if happends might be a bug */
				/*uint64_t c2ns = (uint64_t)ind1 << 32 | f[0].s; //don't think there will be 2G contig, if happends might be a bug */
				hit2_t h = (hit2_t) {ind2, f1s, f[1].rev, f0s, f[0].rev}; 
				hit2_ary_push(hit2_ary, &h);	
			}
			return 0;	
		/*}*/
	}
	return 1;
}

int mb_get_links_hic(char *links_fn, cdict2_t *cds, sdict_t *ctgs)
{	
	bed_file_t *bf = bed_open(links_fn);
	if (!bf) return 0;
	lnk_rec_t r;
	uint32_t line_n = 0;
	while (lnk_read(bf, &r) >= 0) {
		uint32_t ind1;
		ind1 = sd_get(ctgs, r.ctgn);		
		sd_put(ctgs, r.ctgn2);		
		/*uint32_t ind2 = sd_put2(ctgs, r.ctgn, 0, 0, 0, r.llen, r.rlen);		*/
		line_n += 1;
		cd2_add(&cds[ind1], r.is_l, r.ctgn2, r.is_l2, r.wt);	//this has been normalized	
	} 
	bed_close(bf);
	return 0;	
}

int mb_norm_links(cdict2_t *cds, sdict_t *ctgs)
{
	uint32_t n_cds = ctgs->n_seq;
	uint32_t i;
	cdict2_t *c ;
	for ( i = 0; i < n_cds; ++i) {
		/*char *name1 = ctgs->seq[i>>1].name;*/
		/*uint32_t snpn = i&1 ? ctgs->seq[i>>1].l_snp_n:ctgs->seq[i>>1].r_snp_n;*/
		uint32_t j;
		c = cds + i;
		float icnt;
        
		for (j = 0; j < c->n_cnt; ++j) {
            /*fprintf(stderr, "%s\n", c->cnts[j].name);*/
			char *name2 = c->cnts[j].name; 
            uint32_t ctg2_idx = sd_get(ctgs, name2);
			icnt = c->cnts[j].cnt[0] + c->cnts[j].cnt[1] + c->cnts[j].cnt[2] + c->cnts[j].cnt[3]; 
            c->cnts[j].ncnt = icnt / ctgs->seq[ctg2_idx].len;
            /*c->cnts[j].ncnt = (float) icnt / (ctgs->seq[ctg2_idx].l_snp_n + ctgs->seq[ctg2_idx].r_snp_n);*/
            /*uint32_t z;*/
            /*for ( z = 0; z < 4; ++z) c->cnts[j].fcnt[z] = (float) c->cnts[j].cnt[z]/(z >> 1 ? ctgs->seq[i].l_snp_n : ctgs->seq[i].r_snp_n) / ( z & 0x1 ? ctgs->seq[ctg2_idx].l_snp_n : ctgs->seq[ctg2_idx].r_snp_n);  */
			/*uint32_t snp2 = c->cnts[j].is_l ? ctgs->seq[sd_get(ctgs, name2)].l_snp_n:ctgs->seq[sd_get(ctgs,name2)].r_snp_n;*/
			/*fprintf(stderr, "%s\t%c\t%s\t%c\t%u\t%u\t%u\t%lf\n", name1, i&1?'+':'-', name2, c->cnts[j].is_l?'+':'-', icnt, snpn, snp2, 100000.0*(double)icnt/(snp2*snpn));*/
		}
	}
	return 0;
}


int deflimn(cdict2_t *cds, sdict_t *ctgs)
{
    uint32_t n_cds = ctgs->n_seq;
	uint32_t i;
	cdict2_t *c;
    kvec_t(uint32_t) v;
    kv_init(v);
	uint32_t dstr[8] = {0};
	int limn = 7;
	for ( i = 0; i < n_cds; ++i) {
        char *name = ctgs->seq[i].name;
        uint32_t scf_idx = ctgs->seq[i].le;
        uint32_t ctg_idx = i; 
		uint32_t j;
		c = cds + i;
        uint32_t fgt, flt, s_ok, p_ok;
        fgt = flt = s_ok = p_ok = 0;
        int ctgn = 0;
		for (j = 0; j < c->n_cnt; ++j) {
		   uint32_t ctg_idx2 =  sd_get(ctgs, c->cnts[j].name);	
            uint32_t scf_idx2 = ctgs->seq[ctg_idx2].le;
			/*if (ctgn <= 50) fprintf(stderr, "%u\t%s\t%u\t%s\t%f\t%f\t%f\t%f\n", ctg_idx, name, ctg_idx2, ctgs->seq[ctg_idx2].name, c->cnts[j].cnt[0], c->cnts[j].cnt[1], c->cnts[j].cnt[2], c->cnts[j].cnt[3]);*/
            if (scf_idx2 == scf_idx) {
				++ctgn;
				if (ctg_idx2 > ctg_idx) {
					++fgt;
					if (ctg_idx2 == ctg_idx + 1 && fgt < limn) s_ok = 1, ++dstr[fgt];
				} else {
					++flt;	
					if (ctg_idx2 + 1 == ctg_idx && flt < limn) p_ok = 1, ++dstr[flt];
				}
				/*if (ctg_idx2 > ctg_idx + 1) */
					/*fgt = 1;*/
				/*else if (ctg_idx2 + 1 == ctg_idx) */
					/*flt = 1;	*/
			}
			if ((p_ok && s_ok) || (fgt + 1 > limn  && flt + 1 > limn)) 
				break;
        }
		/*fprintf(stderr, "%s\t%d\n", name, ctgs->seq[ctg_idx].rs >> 1 & 1);*/
		/*if (!s_ok) fprintf(stderr, "miss successor add a break\n");	*/
		/*if (!p_ok) fprintf(stderr, "miss predecessor add a break\n");*/
		//1 tail 2 head 3 head + tail 0 middle
        if (!s_ok && (ctgs->seq[ctg_idx].rs & 1) == 0)  
			++dstr[limn];	
        if (!p_ok && (ctgs->seq[ctg_idx].rs >> 1 & 1) == 0)  
			++dstr[limn];	
        /*if (ctgs->seq[ctg_idx].rs & 1) */
            /*kv_push(uint32_t, v, ctg_idx << 1 | 1);  */
	}
	fprintf(stderr, "%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n", dstr[1], dstr[2], dstr[2], dstr[3], dstr[4], dstr[5], dstr[6], dstr[7]);
}

uint32_t *find_breaks2(cdict2_t *cds, sdict_t *ctgs, uint32_t *n_brks, int lim)
{
    uint32_t n_cds = ctgs->n_seq;
	uint32_t i;
	cdict2_t *c;
    kvec_t(uint32_t) v;
    kv_init(v);
	for ( i = 0; i < n_cds; ++i) {
        char *name = ctgs->seq[i].name;
        uint32_t scf_idx = ctgs->seq[i].le;
        uint32_t ctg_idx = i; 
		uint32_t j;
		c = cds + i;
        uint32_t fgt, flt, s_ok, p_ok;
        fgt = flt = s_ok = p_ok = 0;
        int ctgn = 0;
		for (j = 0; j < c->n_cnt; ++j) {
		   uint32_t ctg_idx2 =  sd_get(ctgs, c->cnts[j].name);	
            uint32_t scf_idx2 = ctgs->seq[ctg_idx2].le;
			/*if (ctgn <= 50) fprintf(stderr, "%u\t%s\t%u\t%s\t%f\t%f\t%f\t%f\n", ctg_idx, name, ctg_idx2, ctgs->seq[ctg_idx2].name, c->cnts[j].cnt[0], c->cnts[j].cnt[1], c->cnts[j].cnt[2], c->cnts[j].cnt[3]);*/
            if (scf_idx2 == scf_idx) {
				++ctgn;
				if (ctg_idx2 > ctg_idx) {
					++fgt;
					if (ctg_idx2 == ctg_idx + 1 && fgt < lim) s_ok = 1;
				} else {
					++flt;	
					if (ctg_idx2 + 1 == ctg_idx && flt < lim) p_ok = 1;
				}
				/*if (ctg_idx2 > ctg_idx + 1) */
					/*fgt = 1;*/
				/*else if (ctg_idx2 + 1 == ctg_idx) */
					/*flt = 1;	*/
			}
			if ((p_ok && s_ok) || (fgt + 1 > lim  && flt + 1 > lim)) 
				break;
        }
		/*fprintf(stderr, "%s\t%d\n", name, ctgs->seq[ctg_idx].rs >> 1 & 1);*/
		/*if (!s_ok) fprintf(stderr, "miss successor add a break\n");	*/
		/*if (!p_ok) fprintf(stderr, "miss predecessor add a break\n");*/
		//1 tail 2 head 3 head + tail 0 middle
        if (!s_ok && (ctgs->seq[ctg_idx].rs & 1) == 0)  
            kv_push(uint32_t, v, ctg_idx << 1 | 1);  
        if (!p_ok && (ctgs->seq[ctg_idx].rs >> 1 & 1) == 0)  
            kv_push(uint32_t, v, (ctg_idx - 1) << 1 | 1);  
        /*if (ctgs->seq[ctg_idx].rs & 1) */
            /*kv_push(uint32_t, v, ctg_idx << 1 | 1);  */
	}
    *n_brks = v.n;
	return v.a;
}
uint32_t *find_breaks(cdict2_t *cds, sdict_t *ctgs, uint32_t *n_brks)
{
    uint32_t n_cds = ctgs->n_seq;
	uint32_t i;
	cdict2_t *c;
    kvec_t(uint32_t) v;
    kv_init(v);
	for ( i = 0; i < n_cds; ++i) {
        char *name = ctgs->seq[i].name;
		/*uint32_t seq_idx = i;*/
        uint32_t scf_idx = ctgs->seq[i].le;
        uint32_t ctg_idx = i; 
		/*uint32_t snpn = i&1 ? ctgs->seq[i>>1].l_snp_n:ctgs->seq[i>>1].r_snp_n;*/
		uint32_t j, z;
		c = cds + i;
        uint32_t fgt, flt;
        fgt = flt = 0;
        int ctgn = 0;
        /*float density[4];*/
        uint32_t susp_hd = -1, susp_tl = -1;
		for (j = 0; j < c->n_cnt; ++j) {
            uint32_t sum = 0;
            for ( z = 0; z < 4; ++z) sum += c->cnts[j].cnt[z];
		   uint32_t ctg_idx2 =  sd_get(ctgs, c->cnts[j].name);	
            uint32_t scf_idx2 = ctgs->seq[ctg_idx2].le;
            /*fprintf(stderr, "%s\t%s\t%u\t%u\t%u\t%u\n", name, ctgs->seq[ctg_idx2].name, c->cnts[j].cnt[0], c->cnts[j].cnt[1], c->cnts[j].cnt[2], c->cnts[j].cnt[3]);*/
            if (scf_idx2 == scf_idx) {
                //is_suc 
                uint32_t hh, ht, th, tt, tl, hd, tl2, hd2;
                hh = c->cnts[j].cnt[0], ht = c->cnts[j].cnt[1], th = c->cnts[j].cnt[2], tt = c->cnts[j].cnt[3]; 
                ctgn += 1;
                /*for (z = 0; z < 4; ++z) density[z] = (float)c->cnts[j].cnt[z]/(z >> 1 ? ctgs->seq[ctg_idx].r_snp_n : ctgs->seq[ctg_idx].l_snp_n) / (z&0x1 ? ctgs->seq[ctg_idx2].r_snp_n : ctgs->seq[ctg_idx2].l_snp_n);*/
                /*for (z = 0; z < 4; ++z) density[z] = (float)c->cnts[j].cnt[z]/((z >> 1 ? ctgs->seq[ctg_idx].r_snp_n : ctgs->seq[ctg_idx].l_snp_n) + (z&0x1 ? ctgs->seq[ctg_idx2].r_snp_n : ctgs->seq[ctg_idx2].l_snp_n));*/
                /*for (z = 0; z < 4; ++z) density[z] = c->cnts[j].cnt[z]/(ctgs->seq[ctg_idx].r_snp_n + ctgs->seq[ctg_idx2].r_snp_n);*/
				if (ctgn <= 50) fprintf(stderr, "%u\t%s\t%u\t%s\t%f\t%f\t%f\t%f\n", ctg_idx, name, ctg_idx2, ctgs->seq[ctg_idx2].name, c->cnts[j].cnt[0], c->cnts[j].cnt[1], c->cnts[j].cnt[2], c->cnts[j].cnt[3]);

                    /*if (ctgn >= 10) break;*/
                tl = th + tt;
                hd = hh + ht;
                hd2 = hh + th;
                tl2 = ht + tt;
                {
                        if (ctg_idx2 > ctg_idx) {
                            /*if (get_max(c->cnts[j].cnt, 4) > 1) {*/
                            /*if (get_fmax(density, 4) > 1) {*/
                            /*if (tl > hd || get_max(c->cnts[j].cnt, 4) > 1) {*/
                            if (tl > hd || norm_cdf(hd, 0.5, tl + hd) <= 0.95) { //a very loose condition for successor otherwise cause many false positives
                                if (fgt) continue;
                                ++fgt;
                                if (ctg_idx2 != ctg_idx + 1) {
                                    fprintf(stderr, "n1: %s\t%s\n", name, ctgs->seq[ctg_idx2].name);
                                    if (norm_cdf(tl, 0.5, tl + hd) > 0.95) kv_push(uint32_t, v, ctg_idx << 1 | 1);     
                                    else susp_tl = j;
                                } 
                            } // a successor
                            else {
                                if (flt) continue;
                                ++flt;
                                fprintf(stderr, "n0: %s\t%s\n", name, ctgs->seq[ctg_idx2].name);
                                if (!(ctgs->seq[ctg_idx].rs & 0x2)) kv_push(uint32_t, v, (ctg_idx - 1) << 1 | 1);         
                            }  //precessor
                        } else { //potential to be a precessor 
                            /*if (get_max(c->cnts[j].cnt, 4) < 2) {*/
                            /*if (get_fmax(density, 4) < 2) {*/
                            /*if (tl < hd || get_max(c->cnts[j].cnt, 4) < 2) {*/
                            if (tl < hd || norm_cdf(tl, 0.5, tl + hd) <= 0.95) {
                                if (flt) continue;
                                ++flt;
                                if (ctg_idx2 + 1 != ctg_idx) {
                                    fprintf(stderr, "m1: %s\t%s\n", name, ctgs->seq[ctg_idx2].name);
                                    if (!(ctgs->seq[ctg_idx].rs & 0x2)) {
                                        if (norm_cdf(hd, 0.5, tl + hd) > .95) kv_push(uint32_t, v, (ctg_idx - 1) << 1 | 1);         
                                        else susp_hd = j;
                                    }
                                } 
                            } // a predecessor
                            else {
                                if (fgt) continue;
                                ++fgt;
                                fprintf(stderr, "m0: %s\t%s\n", name, ctgs->seq[ctg_idx2].name);
                                kv_push(uint32_t, v, ctg_idx << 1 | 1);         
                            } // a successor 
                        }
                        //validate 
                }
            }
            /*if (fgt && flt) break;*/
        }
		
        if (susp_hd != 0xFFFFFFFF) {
            int insert = 1;
            for (j = susp_hd + 1; j < susp_hd + 2 && j < c->n_cnt; ++j) {
                uint32_t sum = 0;
                for ( z = 0; z < 4; ++z) sum += c->cnts[j].cnt[z]; 
				uint32_t ctg_idx2 = sd_get(ctgs, c->cnts[j].name);
                uint32_t scf_idx2 = ctgs->seq[ctg_idx2].le;
                uint32_t hh, ht, th, tt, tl, hd, tl2, hd2;
                hh = c->cnts[j].cnt[0], ht = c->cnts[j].cnt[1], th = c->cnts[j].cnt[2], tt = c->cnts[j].cnt[3]; 
                tl = th + tt;
                hd = hh + ht;
                hd2 = hh + th;
                tl2 = ht + tt;
                if (scf_idx2 == scf_idx && ctg_idx2 + 1 == ctg_idx) {
                fprintf(stderr, "val:%s\t%s\t%f\t%f\t%f\t%f\n", name, ctgs->seq[ctg_idx2].name, c->cnts[j].cnt[0], c->cnts[j].cnt[1], c->cnts[j].cnt[2], c->cnts[j].cnt[3]);
                    if (hd > tl && norm_cdf(hd, 0.5, tl + hd) > 0.95) 
                        insert = 0;
                    break;   
                } 
            }
            if (insert) kv_push(uint32_t, v, (ctg_idx - 1) << 1 | 1);      
        }
        if (susp_tl != 0xFFFFFFFF) {
            int insert = 1;
            for (j = susp_tl + 1; j < susp_tl + 2 && j < c->n_cnt; ++j) {
                uint32_t sum = 0;
                for ( z = 0; z < 4; ++z) sum += c->cnts[j].cnt[z]; 
				uint32_t seq_idx2 = sd_get(ctgs, c->cnts[j].name);
                uint32_t ctg_idx2 = ctgs->seq[seq_idx2].r_snp_n;
                uint32_t scf_idx2 = ctgs->seq[ctg_idx2].le;
                uint32_t hh, ht, th, tt, tl, hd, tl2, hd2;
                hh = c->cnts[j].cnt[0], ht = c->cnts[j].cnt[1], th = c->cnts[j].cnt[2], tt = c->cnts[j].cnt[3]; 
                tl = th + tt;
                hd = hh + ht;
                hd2 = hh + th;
                tl2 = ht + tt;
                if (scf_idx2 == scf_idx && ctg_idx2 == ctg_idx + 1) {
                fprintf(stderr, "val:%s\t%s\t%f\t%f\t%f\t%f\n", name, ctgs->seq[ctg_idx2].name, c->cnts[j].cnt[0], c->cnts[j].cnt[1], c->cnts[j].cnt[2], c->cnts[j].cnt[3]);
                    if (hd < tl && norm_cdf(tl, 0.5, tl + hd) >= 0.95) 
                        insert = 0;
                    break;   
                } 
            }
            if (insert) kv_push(uint32_t, v, (ctg_idx) << 1 | 1);      
        }
        if (fgt == 0 && (ctgs->seq[ctg_idx].rs & 1) == 0) 
            kv_push(uint32_t, v, ctg_idx << 1 | 1);  
        if (flt == 0 && (ctgs->seq[ctg_idx].rs >> 1 & 1) == 0)  
            kv_push(uint32_t, v, (ctg_idx - 1) << 1 | 1);  
        /*if (ctgs->seq[ctg_idx].rs & 1) */
            /*kv_push(uint32_t, v, ctg_idx << 1 | 1);  */
	}
    *n_brks = v.n;
	return v.a;
}

int detect_bks(cov_ary_t *ca, graph_t *g, sdict_t *ctgs, sdict_t *scfs, float min_rat)
{
	asm_t *as = &g->as.asms[g->as.casm];
	vertex_t *vt = g->vtx.vertices;
	path_t *pt = g->pt.paths;

	uint32_t n = as->n;
	uint32_t i;	
	uint32_t idx = 0;
	fprintf(stderr, "BREAK LOG STARTS\n");
	for ( i = 0; i < n; ++i) {
		uint32_t m = pt[as->pn[i] >> 1].n;
		cov_stat_t *ct = malloc(sizeof(cov_stat_t) * (2 * m - 1));
		uint32_t j;
		uint32_t scf_id = sd_get(scfs, pt[as->pn[i]>>1].name);
		for ( j = 0; j < 2 * m - 1; ++j ) {
			if ( j % 2) {
				uint32_t st = ctgs->seq[idx].r_snp_n + ctgs->seq[idx].len + 1; ;  
				uint32_t ed = st + GAP_SZ - 1;	
				cal_cov_stat4reg(&ca[scf_id], st, ed, &ct[j]);		
				fprintf(stderr, "%s\t%u\t%u\t%s\t%u\t%f\t%f\t%f\n", pt[as->pn[i]>>1].name, st, ed, "gap", GAP_SZ, ct[j].maxim, ct[j].avg, ct[j].rsd);
				++idx;
			} else {
				uint32_t st = ctgs->seq[idx].r_snp_n + 1;
				uint32_t ed = st + ctgs->seq[idx].len - 1;				
				cal_cov_stat4reg(&ca[scf_id], st, ed, &ct[j]);		
				fprintf(stderr, "%s\t%u\t%u\t%s\t%u\t%f\t%f\t%f\n", pt[as->pn[i]>>1].name, st, ed, ctgs->seq[idx].name, ctgs->seq[idx].len, ct[j].maxim, ct[j].avg, ct[j].rsd);
			}	
		}
		++idx;
		//find breaks
		kvec_t(uint32_t) pos;
		kv_init(pos);
		for (j = 0; j < 2 * m - 1; ++j) 
			if (j%2 && ct[j].maxim / ct[j-1].maxim <= min_rat && ct[j].maxim / ct[j+1].maxim <= min_rat)
				kv_push(uint32_t, pos, j >> 1);	
		break_path(g, as->pn[i]>>1, pos.a, pos.n);
		kv_destroy(pos);
		free(ct);
	}
	fprintf(stderr, "BREAK LOG ENDS\n");
	return 0;
}

int mb_init_scaffs(graph_t *g, sdict_t *ctgs, sdict_t *scfs)
{
	/*dump_sat(g);*/
	asm_t *as = &g->as.asms[g->as.casm];
	vertex_t *vt = g->vtx.vertices;
	path_t *pt = g->pt.paths;
	uint32_t n = as->n;
	uint32_t i;	
	/*for (i = 0; i < g->vtx.n; ++i) sd_put(ctgs, vt[i].name);	*/
	for ( i = 0; i < n; ++i) {
		uint32_t m = pt[as->pn[i] >> 1].n;
		/*fprintf(stderr, "%d\n", as->pn[i]); */
		uint32_t *p = pt[as->pn[i] >> 1].ns;
		uint32_t j, len = 0, len_ctg;
		//push scaffold name to scfs 
		/*int32_t scf_id = as->pn[i] >> 1;*/
		int32_t scf_id =sd_put2(scfs, pt[as->pn[i]>>1].name, 0, 0, 0, 0, 0);
		uint32_t sid;
		for ( j = 0; j < m; ++j ) { // pid, length,   
			len_ctg = vt[p[j]>>2].len;	
			sid = sd_put(ctgs, vt[p[j]>>2].name);
			//contig points to scaffold and set its start and direction
			/*fprintf(stderr, "sid: %u\t%s\t%s\n", sid, pt[as->pn[i]>>1].name, vt[p[j]>>2].name, p[j]);*/
			/*fprintf(stdout, "%s\t%s\t%u\n", pt[as->pn[i]>>1].name, vt[p[j]>>2].name, len);*/
			ctgs->seq[sid].len = len_ctg;
			/*ctgs->seq[sid].le = as->pn[i]>>1;*/
			ctgs->seq[sid].le = scf_id;
			ctgs->seq[sid].l_snp_n = (j << 1) | (p[j] & 0x1);
			ctgs->seq[sid].r_snp_n = len;
			ctgs->seq[sid].rs = 0;	
			if (!j) 
				ctgs->seq[sid].rs = 2;
			if (j == m - 1) 
				ctgs->seq[sid].rs |= 1, len += len_ctg;
			else 
				len += len_ctg + 200;
		}
		//reset scaffold length le rs l_snp_n, r_snp_n
		sd_put4(scfs, pt[as->pn[i]>>1].name, len, len >> 1, (len >>1) + 1, len >> 1, len >> 1, pt[as->pn[i]>>1].is_circ);
		/*free(p);*/
	}
	return 0;
}
int cmp_brks(const void *a, const void *b)
{
    uint32_t p = *(uint32_t *)a;
    uint32_t q = *(uint32_t *)b;
    if (p < q) return -1;
    else if (p == q) return 0;
    else return 1;
}

int cut_paths(graph_t *g, uint32_t *brks, uint32_t n_brks, sdict_t *ctgs, sdict_t *scfs) 
{
    qsort(brks, n_brks, sizeof(uint32_t), cmp_brks);  
    
    uint32_t i, j;
    /*for (i = 0; i < n_brks; ++i) {*/
        /*fprintf(stdout, "%s\t%d\t%d\n", ctgs->seq[brks[i]>>1].name, brks[i]>>1, brks[i]&1);*/
    /*}*/
    /*name_t nt, nt1;*/
	char *name, *name2;
    /*for ( i = 1, j = 0; i <= n_brks; ++i) {*/
        /*if (i == n_brks || brks[i] != brks[j]) {*/
            /*uint32_t ctg_idx = brks[j];*/
			/*fprintf(stdout, "%s\t%s\n", ctgs->seq[ctg_idx >> 1].name, ctgs->seq[(ctg_idx >> 1)+1].name);*/
			/*j = i;        */
        /*} */
    /*}*/
	uint32_t scf_id = -1;	
	kvec_t(uint32_t) pos;
	kv_init(pos);
	int brkn = 0;
	int adpn = 0;
	for ( i = 1, j = 0; i <= n_brks; ++i) {
		if (i == n_brks || brks[i] != brks[j]) {
			uint32_t ctg_idx = brks[j] >> 1;
			if (ctgs->seq[ctg_idx].le != scf_id) {
				if (~scf_id) brkn += pos.n, adpn += break_path(g, scf_id, pos.a, pos.n);
				kv_reset(pos);
				scf_id = ctgs->seq[ctg_idx].le;	
			}	
			kv_push(uint32_t, pos, ctgs->seq[ctg_idx].l_snp_n >> 1);
			j = i;	
		} 			
		/*uint32_t ctg_idx = brks[i];*/
			/*fprintf(stdout, "%u\t%s\t%s\n", ctgs->seq[ctg_idx>>1].le, ctgs->seq[ctg_idx >> 1].name, ctgs->seq[(ctg_idx >> 1)+1].name);*/
	}
	fprintf(stderr, "[M::%s] find %d breaks, add %d paths\n", __func__, brkn, adpn);
	kv_destroy(pos);
    return 0;
}

void mb_col_bcnt(aln_inf_t  *fal, uint32_t n, uint32_t bc, int min_mq, sdict_t *ctgs, sdict_t *scfs, uint32_t max_is, bc_ary_t *l)
{
	/*if (fal->mq >= min_mq) {*/ //should we allow user to set minimum mapping quality for each alignment? maybe not, those alignments with zero mapping quality are useful to bridge the gaps between two separate alignments
	int i;
	for ( i = 0; i < n; ++i ) {
		if (scfs->n_seq) {
			sd_seq_t *sq1 = &ctgs->seq[fal[i].tid];
			uint32_t ind1 = sq1->le; //maybe not well paired up
			uint32_t f0s = sq1->r_snp_n + (sq1->l_snp_n & 0x1 ? fal[i].s : sq1->len - fal[i].s);
			bc_t t = (bc_t) {f0s, fal[i].qual, (uint64_t)bc << 32 | ind1};
			mb_bc_ary_push(l, &t);	
		} else {
			bc_t t = (bc_t) {fal[i].s, fal[i].qual, (uint64_t)bc << 32 | fal[i].tid};
			mb_bc_ary_push(l, &t);	
		}
	}	
		/*uint32_t e = fal->e;*/
		/*s = s << 1;*/
		/*e = e << 1 | 1; */
		/*if (e - s < max_ins_len) {*/
			/*if (opt) {*/
				/*uint32_t tmp = s;*/
				/*s = e;*/
				/*e = tmp;*/
			/*}*/
		/*if (e - s < max_is) {*/
			/*bc_t t = (bc_t) {s, fal->mq, (uint64_t)bc << 32 | fal->tid};*/
			
			/*bc_ary_push(l, &t);	*/
		/*}*/
		/*}*/
		/*pos_push(&d->ctg_pos[fal[0].tid], s);*/
		/*pos_push(&d->ctg_pos[fal[0].tid], e); // we init */
		/*ctg_pos_push(&d[fal[0].tid], s);k*/
		/*ctg_pos_push(&d[fal[0].tid], e);*/
	/*}*/
}
int mb_proc_10x_bam(char *bam_fn, int min_mq, uint32_t max_is, sdict_t *ctgs,  sdict_t *scfs, sdict_t *bc_n, int opt, bc_ary_t *bc_l)
{
	bamFile fp;
	bam_header_t *h;
	bam1_t *b;
	fp = bam_open(bam_fn, "r"); //should check if bam is sorted
	if (fp == 0) {
		fprintf(stderr, "[E::%s] fail to open %s\n", __func__, bam_fn);
		return -1;
	}
	
	h = bam_header_read(fp);
	b = bam_init1();
	
	char **names = h->target_name;

	char *cur_qn = NULL, *cur_bc = NULL;
	/*int32_t cur_l = 0;*/
	long bam_cnt = 0;
	long rd_cnt = 0;
	aln_inf_t aln;
	kvec_t(aln_inf_t) alns;
	kv_init(alns);
	while (1) {
		//segment were mapped 
		if (bam_read1(fp, b) >= 0 ) {
			if (!cur_qn || strcmp(cur_qn, bam1_qname(b)) != 0) {
				/*fprintf(stderr, "%d\t%d\t%d\n", aln_cnt, rev, aln.mq);*/
				if (alns.n) mb_col_bcnt(alns.a, alns.n, sd_put(bc_n, cur_bc), min_mq, ctgs, scfs, max_is, bc_l);
				kv_reset(alns);
				if (cur_qn) free(cur_qn); 
				cur_qn = strdup(bam1_qname(b));
				cur_bc = cur_qn + b->core.l_qname - BC_LEN;
				++rd_cnt;
			}
			if ((b->core.flag & 0x4) || (b->core.flag & 0x80)) continue; //not aligned
			aln.s = b->core.pos + 1;
		   	aln.qual = b->core.qual;
			aln.tid = sd_get(ctgs, names[b->core.tid]);
			kv_push(aln_inf_t, alns, aln);	
			
			if ((++bam_cnt % 1000000) == 0) fprintf(stderr, "[M::%s] processing %ld bams\n", __func__, bam_cnt); 
		} else {
			/*fprintf(stderr, "%d\t%d\t%d\n", aln_cnt, rev, aln.mq);*/
			if (alns.n) mb_col_bcnt(alns.a, alns.n, sd_put(bc_n, cur_bc), min_mq, ctgs, scfs, max_is, bc_l);
			break;	
		}
	}
	fprintf(stderr, "[M::%s] finish processing %ld bams\n", __func__, bam_cnt); 
	fprintf(stderr, "[M::%s] read pair counter: %ld\n", __func__, rd_cnt);
	kv_destroy(alns);
	bam_destroy1(b);
	bam_header_destroy(h);
	bam_close(fp);
	return 0;
}

int mb_proc_hic_bam(char *bam_fn, int min_mq, sdict_t *ctgs, sdict_t *scfs, hit2_ary_t *ha)
{
	bamFile fp;
	bam_header_t *h;
	bam1_t *b;
	fp = bam_open(bam_fn, "r"); //should check if bam is sorted
	if (fp == 0) {
		fprintf(stderr, "[E::%s] fail to open %s\n", __func__, bam_fn);
		return -1;
	}
	
	h = bam_header_read(fp);
	b = bam_init1();
	char **names = h->target_name;
	/*int i;*/
	/*for ( i = 0; i < h->n_targets; ++i) {*/
		/*char *name = h->target_name[i];*/
		/*uint32_t len = h->target_len[i];*/
		/*if (ws > len) ws = len;*/
		/*uint32_t le = (len - ws) >> 1;*/
		/*uint32_t rs = (len + ws) >> 1;*/
		/*uint32_t lenl, lenr;*/
		/*lenl = lenr = (len - ws) >> 1;*/
		/*sd_put2(ctgs, name, len, le, rs, lenl, lenr);*/
	/*}*/
	/*uint32_t cur_ws;*/
	/*for ( i = 0; i < h->n_targets; ++i) {*/
		/*char *name = h->target_name[i];*/
		/*uint32_t len = h->target_len[i];*/
		/*cur_ws = ws;*/
		/*if (len < (cur_ws << 1)) cur_ws = len >> 1;*/
		/*uint32_t le = cur_ws;*/
		/*uint32_t rs = len - cur_ws + 1;*/
		/*uint32_t lenl, lenr;*/
		/*lenl = lenr = cur_ws;*/
		/*sd_put2(ctgs, name, len, le, rs, lenl, lenr);*/
	/*}*/
	/*if (!ns->ct) { //not initiate yet*/
		/*init_gaps(gap_fn, ns, ctgs, max_ins_len);*/
	/*}*/

	char *cur_qn = 0;
	long bam_cnt = 0;
	int is_set = 0;
	/*aln_inf_t aln[2];*/
	/*int aln_cnt;*/
	
	kvec_t(aln_inf_t) all;
	kv_init(all);
	kvec_t(aln_inf_t) five;
	kv_init(five);

	uint8_t rev;
	uint64_t rdp_counter  = 0;
	uint32_t rd1_cnt = 0, rd2_cnt = 0;
	uint32_t rd1_5cnt = 0, rd2_5cnt = 0;
	uint64_t used_rdp_counter = 0;
	/*fprintf(stderr, "Proc Bam %d\n", __LINE__);*/
	while (1) {
		//segment were mapped 
		if (bam_read1(fp, b) >= 0 ) {
			if (!cur_qn || strcmp(cur_qn, bam1_qname(b)) != 0) {
				if (rd1_cnt < 3 && rd2_cnt < 3 && rd1_5cnt == 1 && rd2_5cnt == 1 && !mb_col_hits2(five.a, five.n, ctgs, scfs, ha, min_mq, cur_qn)) ++used_rdp_counter;
				/*aln_cnt = 0;	*/
				/*rev = 0;*/
				/*is_set = 0;*/
				/*kv_reset(all);*/
				kv_reset(five);
				rd1_cnt = rd2_cnt = rd1_5cnt = rd2_5cnt = 0;
				if (cur_qn) ++rdp_counter, free(cur_qn); 
				cur_qn = strdup(bam1_qname(b));
			}
			b->core.flag & 0x40 ? ++rd1_cnt : ++rd2_cnt;	
			if (b->core.flag & 0x4) continue; //not aligned
			aln_inf_t tmp;
			rev = tmp.rev = !!(b->core.flag & 0x10);
			/*tmp.nrev = !!(b->core.flag & 0x20);*/
			//only collects five prime
			tmp.tid = sd_get(ctgs, names[b->core.tid]);
			/*tmp.ntid = b->core.mtid;*/
			tmp.s = b->core.pos + 1;
			tmp.qual = b->core.qual;
			/*tmp.ns = b->core.mpos + 1;*/
			/*kv_push(aln_inf_t, all, tmp);*/
				
			uint32_t *cigar = bam1_cigar(b);
			if ((!rev && bam_cigar_op(cigar[0]) == BAM_CMATCH) || (rev && bam_cigar_op(cigar[b->core.n_cigar-1]) == BAM_CMATCH)) {
				b->core.flag & 0x40 ? ++rd1_5cnt: ++rd2_5cnt; kv_push(aln_inf_t, five, tmp);
			}
			
			/*aln_cnt = (aln_cnt + 1 ) & 1;*/
			/*if ((++bam_cnt % 1000000) == 0) fprintf(stderr, "[M::%s] processing %ld bams\n", __func__, bam_cnt); */
		} else {
			if (rd1_cnt < 3 && rd2_cnt < 3 && rd1_5cnt == 1 && rd2_5cnt == 1 && !mb_col_hits2(five.a, five.n, ctgs, scfs, ha, min_mq, cur_qn)) ++used_rdp_counter;
			if (cur_qn) ++rdp_counter, free(cur_qn); 
			break;	
		}
	}
	fprintf(stderr, "[M::%s] finish processing %lld read pairs %lld (%.2f) passed\n", __func__, rdp_counter, used_rdp_counter, (double)used_rdp_counter/rdp_counter); 
	bam_destroy1(b);
	bam_header_destroy(h);
	bam_close(fp);
	kv_destroy(all);
	kv_destroy(five);
	return 0;
}

void mb_insert_sort(bc_t *b, bc_t *e)
{
	bc_t *i, *j, swap_tmp;
		for (i = b + 1; i < e; ++i)										
			for (j = i; j > b && ((j-1)->s > j->s || ((j-1)->s == j->s && (j-1)->e > j->e)); --j) {			
				swap_tmp = *j; *j = *(j-1); *(j-1) = swap_tmp;			
			}															
}
void mb_srt_by_nm_loc(bc_t *s, bc_t *e)
{
	bc_t *i = s;
	while (i < e) {
		bc_t *j;
		for (j = i + 1; j < e && j->bctn == i->bctn; ++j);
		mb_insert_sort(i, j);
		i = j;
	}	
}
cord_t *mb_col_cords(bc_ary_t *bc_l, uint32_t min_maq, uint32_t min_bc, uint32_t max_bc, uint32_t min_inner_bcn, uint32_t max_span, uint32_t min_mol_len, int n_targets, sdict_t *ctgs, char *gap_fn, uint32_t *n50)
{
	int k;
	/*init_gaps(gap_fn, ns, ctgs, 0);	*/
	cord_t *cc = calloc(n_targets, sizeof(cord_t));
	radix_sort_mbbct(bc_l->ary, bc_l->ary + bc_l->n);	
	uint32_t n = bc_l->n;
	bc_t *p = bc_l->ary;
	/*kvec_t(uint32_t) lenset;	*/
	/*kv_init(lenset);*/
	uint32_t i = 0;
	uint32_t maqs;
	while (i < n) {
		uint32_t z;
		for ( z = i; z < n && (p[z].bctn >> 32) == (p[i].bctn >> 32); ++z);
		uint32_t n_bc = z - i;
		if (n_bc > min_bc && n_bc < max_bc) {
			mb_srt_by_nm_loc(p+i, p+z); //sort by ctg name and locus
			uint32_t s = p[i].s,e = p[i].e;
			uint32_t j = i + 1;
			
			/*fprintf(stderr, "%u\t%u\n",j,z);*/
			while (j <= z) {
				if (j == z || p[j].bctn != p[i].bctn || (p[j].s - p[j-1].s > max_span)) {
					/*if (s > e) fprintf(stderr, "woofy%u\t%u\n", s, e);*/
					/*fprintf(stderr, "%s\t%u\t%u\t%u\n",ctgs->seq[p[i].bctn&0xFFFFFFFF].name, s, e, j - i);*/
					if (j - i > min_inner_bcn && e - s > min_mol_len && maqs / (j-i) > min_maq) {
						cors tmp = (cors) {s, e}; 
						/*kv_push(uint32_t, lenset, e-s+1);*/
						cord_push(&cc[p[i].bctn & 0xFFFFFFFF], &tmp);
						/*fprintf(stderr, "%s\t%u\t%u\n",ctgs->seq[p[i].bctn&0xFFFFFFFF].name, s, e);*/
					} 					
					/*pos_push(&d->ctg_pos[p[i].bctn & 0xFFFFFFFF], s << 1);*/
					/*pos_push(&d->ctg_pos[p[i].bctn & 0xFFFFFFFF], e << 1 | 1);*/
					if (j == z) break; //otherwise infinate loop
					i = j;
					s = p[i].s;
					e = p[i].s;
					maqs = p[i].e;
					j = i + 1;
				} else {
					e = p[j].s;
					maqs += p[j].e;
					++j;
				}
			}
		}	
		i = z;	
	}

	/*for (k=0; k < bc_l->n; ++k) {*/
		/*if (bc_l->ary[k].s > bc_l->ary[k].e)*/
			/*fprintf(stderr, "hhh%llx\t%llx\t%u\t%u\n", bc_l->ary[k].bctn >> 32, bc_l->ary[k].bctn & 0xFFFFFFFF, bc_l->ary[k].s, bc_l->ary[k].e);*/
	/*}*/
	/**n50 = cal_n50(lenset.a, lenset.n);*/
	/*kv_destroy(lenset);*/
	return cc;
}
ctg_pos_t *col_pos(cord_t *cc, int n, sdict_t *ctgs)
{
	ctg_pos_t *d = ctg_pos_init();
	int k;
	for ( k = 0;k < n; ++k) {
		ctg_pos_push(d, k);
		cors *e = cc[k].coords;
		size_t n_e = cc[k].n;
		radix_sort_cord(e, e + n_e);
		size_t i = 0, j;
		uint32_t st,ed;
		
		while (i < n_e) {
			st = e[i].s, ed= e[i].e;
					/*fprintf(stderr, "h%s\t%u\t%u\n",ctgs->seq[k].name, st, ed);*/
			for ( j = i + 1; j <= n_e; ++j) {
					/*fprintf(stderr, "t%s\t%u\t%u\n",ctgs->seq[k].name, st, ed);*/
				if (j == n_e || e[j].s != e[i].s) {
					/*fprintf(stderr, "d%s\t%u\t%u\n",ctgs->seq[k].name, st, ed);*/
					pos_push(&d->ctg_pos[k], st << 1);
					pos_push(&d->ctg_pos[k], ed << 1 | 1);
					i = j;
					break;
				} else {
					ed = max(ed, e[j].e);
				}
			} 
		}	
	} 
	return d;	
}
int mk_brks_10x(char *sat_fn, char *bam_fn[], int n_bam, char *gap_fn, int min_mq, int min_cov, float min_cov_rat, uint32_t max_span, int max_cov, uint32_t max_is, int min_bc, int max_bc, float min_rat, uint32_t min_inner_bcn, uint32_t min_mol_len, int opt, char *out_dir, char *out_fn)
{
	sdict_t *ctgs = sd_init();
	sdict_t *scfs = sd_init();
	
	sdict_t* bc_n = sd_init();
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] initiate contigs\n", __func__);
#endif
	graph_t *og = load_sat(sat_fn);
	/*if (!g) {*/
		/*fprintf(stderr, "[E::%s] fail to load the scaffolding graph!\n", __func__);*/
		/*return 1;*/
	/*}*/
	simp_graph(og);
	mb_init_scaffs(og, ctgs, scfs);	
	if (!ctgs) {
		fprintf(stderr, "[E::%s] fail to collect contigs\n", __func__);	
		return 1;
	} 
	if (!scfs->n_seq) {
		fprintf(stderr, "[E::%s] fail to collect scaffolds\n", __func__);	
		return 1;
	} 
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] processing bam file\n", __func__);
#endif
	int i;
	bc_ary_t *bc_l = calloc(1, sizeof(bc_ary_t));
	for ( i = 0; i < n_bam; ++i) {
		if (mb_proc_10x_bam(bam_fn[i], min_mq, max_is, ctgs, scfs, bc_n, opt, bc_l)) {
			return -1;	
		}	
	}	

	sd_destroy(bc_n);	
	if (!(bc_l&&bc_l->n)) {
		fprintf(stderr, "[W::%s] none useful information, quit\n", __func__);
		return -1;
	}
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] collecting coordinates\n", __func__);
#endif
	uint32_t n50;
	sdict_t *_sd = scfs->n_seq ? scfs : ctgs;
	cord_t *cc = mb_col_cords(bc_l, min_mq, min_bc, max_bc, min_inner_bcn, max_span, min_mol_len, _sd->n_seq, _sd, gap_fn, &n50);	
	free(bc_l->ary); free(bc_l);	
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] collecting positions\n", __func__);
#endif
	ctg_pos_t *d = col_pos(cc, _sd->n_seq, _sd);	
	cord_destroy(cc, _sd->n_seq);	
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] calculating coverage\n", __func__);
#endif
	/*float avgcov4wg;*/
	cov_ary_t *ca = cal_cov(d, _sd);
		
	ctg_pos_destroy(d);
	if (!ca) {
		fprintf(stderr, "[W::%s] low quality alignments\n", __func__);
		return 0;	
	}
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] detecting break points\n", __func__);
#endif
	detect_bks(ca, og, ctgs, scfs, min_rat);

#ifdef VERBOSE
	fprintf(stderr, "[M::%s] dump sat graph\n", __func__);
#endif
	dump_sat(og, out_fn);	
	
	char *type = "10x";
	char *desc = "10x data";
	
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] print average coverage for each 1024 base of the contigs\n", __func__);
#endif
	print_coverage_wig(ca, _sd, type, 1024, out_dir);
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] release coverage array\n", __func__);
#endif
	cov_ary_destroy(ca, _sd->n_seq); //a little bit messy
	graph_destroy(og);
	sd_destroy(ctgs);
	sd_destroy(scfs);
	return 0;


}

int mk_brks(char *sat_fn, char *bam_fn[], int n_bams, int min_mq, float min_rat, char *out_dir, char *prefix)
{
	
	sdict_t *ctgs = sd_init();	
	sdict_t *scfs = sd_init();
	int win_s = 0;
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] initiate contigs\n", __func__);
#endif
	graph_t *og = load_sat(sat_fn);
	simp_graph(og);
	mb_init_scaffs(og, ctgs, scfs);	
	if (!ctgs) {
		fprintf(stderr, "[E::%s] fail to collect contigs\n", __func__);	
		return 1;
	} 
	if (!scfs->n_seq) {
		fprintf(stderr, "[E::%s] fail to collect scaffolds\n", __func__);	
		return 1;
	} 

#ifdef VERBOSE
	fprintf(stderr, "[M::%s] processing bam file\n", __func__);
#endif
	
	hit2_ary_t *hit2_ary = calloc(1, sizeof(hit2_ary_t));
	int i;	
	for ( i = 0; i < n_bams; ++i) {
		if (mb_proc_hic_bam(bam_fn[i], min_mq, ctgs, scfs, hit2_ary)) {
			return 1;	
		}	
	}
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] sort array\n", __func__);
#endif
	//sort hit_ary
	if (!hit2_ary->ary) {
		fprintf(stderr, "[W::%s] no qualified hits found in the alignments\n", __func__);
		return 1;
	} 
	qsort(hit2_ary->ary, hit2_ary->n, sizeof(hit2_t), cmp_hits2);	
	//col joints
	sdict_t *_sd = scfs->n_seq ? scfs : ctgs;

#ifdef VERBOSE
	fprintf(stderr, "[M::%s] collecting positions\n", __func__);
#endif
	ctg_pos_t *d = ctg_pos_init_wctgn(_sd->n_seq);
	if (!d) {
		fprintf(stderr, "[E::%s] fail to initiate space for position arrays\n", __func__);
		return -1;
	}
	mb_col_contacts(hit2_ary, _sd, d);
	free(hit2_ary->ary); free(hit2_ary);
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] calculating coverage for each base on genome\n", __func__);
#endif
	cov_ary_t *ca = cal_cov(d, _sd);
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] release vector space for contig positions\n", __func__);
#endif
	ctg_pos_destroy(d);
	if (!ca) {
		fprintf(stderr, "[W::%s] low quality alignments\n", __func__);
		return 0;	
	}
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] detecting break points\n", __func__);
#endif
	detect_bks(ca, og, ctgs, scfs, min_rat);

#ifdef VERBOSE
	fprintf(stderr, "[M::%s] dump sat graph\n", __func__);
#endif
	char *out_fn = malloc(strlen(out_dir) + strlen(prefix) + 6);
	sprintf(out_fn, "%s/%s.sat", out_dir, prefix);
	dump_sat(og, out_fn);	
	free(out_fn);

	char *type = "HIC";
	char *desc = "hic data";
	
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] print average coverage for each 1024 base of the contigs\n", __func__);
#endif
	print_coverage_wig(ca, _sd, type, 1024, out_dir);
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] release coverage array\n", __func__);
#endif
	cov_ary_destroy(ca, _sd->n_seq); //a little bit messy
	graph_destroy(og);
	sd_destroy(ctgs);
	sd_destroy(scfs);
	return 0;
}

int mk_brks2(char *sat_fn, char *links_fn, int limn, char *out_fn)
{
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] loading SAT graph\n", __func__);
#endif
	graph_t *og = load_sat(sat_fn);
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] simplify SAT graph\n", __func__);
#endif
	simp_graph(og);
	sdict_t *ctgs = sd_init(); 
	sdict_t *scfs = sd_init();
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] initiate scaffolds\n", __func__);
#endif
	mb_init_scaffs(og, ctgs, scfs);
	if (!ctgs) return 1;
	uint32_t n_ctg = ctgs->n_seq;	
	uint32_t i = 0;
	/*fprintf(stderr, "%u\n", ctgs->n_seq);*/
	cdict2_t* cds = calloc(n_ctg, sizeof(cdict2_t)); 
	for ( i = 0; i < n_ctg; ++i) cd2_init(cds+i); 
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] collecting links\n", __func__);
#endif
	mb_get_links_hic(links_fn, cds, ctgs);

	/*for ( i = 0 ; i < n_ctg; ++i) {*/
		/*fprintf(stderr, "%s\t%u\t%u\n", ctgs->seq[i].name, ctgs->seq[i].le, ctgs->seq[i].r_snp_n);*/
	/*}*/
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] normalize contact matrix\n", __func__);
#endif
    mb_norm_links(cds, ctgs);
	/*return 0;*/
     //sort cd2
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] sort contact matrix\n", __func__);
#endif
	for ( i = 0; i < n_ctg; ++i) cd2_sort(cds+i); 

	deflimn(cds, ctgs);
	uint32_t n_brks;
	uint32_t *brks = find_breaks2(cds, ctgs, &n_brks, limn);
		
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] cut paths\n", __func__);
#endif
	cut_paths(og, brks, n_brks, ctgs, scfs);
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] dump sat graph\n", __func__);
#endif
	dump_sat(og, out_fn);	
#ifdef VERBOSE
	fprintf(stderr, "[M::%s] release memory resource\n", __func__);
#endif
	graph_destroy(og);
	if (brks) free(brks);
	return 0;
}

int main_brks_10x(int argc, char *argv[])
{
	int c;
	int max_cov = 1000000, min_cov = 10, min_mq = 20;
	int min_as = 0;
	uint32_t max_span = 20000, min_inner_bcn = 5, min_mol_len = 1000;
	uint32_t min_bc = 20, max_bc = 1000000, max_is=1000;
	float min_cov_rat = .15, min_rat = .2;
	char *r;
	char *out_dir = ".", *out_fn = 0;
		
	int option = 0; //the way to calculate molecule length //internal parameters not allowed to adjust by users
	char *program;
   	(program = strrchr(argv[0], '/')) ? ++program : (program = argv[0]);
	--argc, ++argv;
	while (~(c=getopt(argc, argv, "b:B:c:C:r:q:S:a:L:l:m:O:o:h"))) {
		switch (c) {
			case 'b': 
				min_bc = strtol(optarg, &r, 10);
				break;
			case 'B':
				max_bc = strtol(optarg, &r, 10);
				break;
			case 'c':
				min_cov = atoi(optarg); 
				break;
			case 'r':
				min_cov_rat = atof(optarg); 
				break;
			case 'C':
				max_cov = atoi(optarg); 
				break;
			case 'q':
				min_mq = atoi(optarg);
				break;
			case 'L':
				max_is = strtol(optarg, &r, 10);
				break;
			case 'm':
				min_rat = atof(optarg);
				break;
			case 'S':
				max_span = strtol(optarg, &r, 10);
				break;
			case 'a':
				min_inner_bcn = strtol(optarg, &r, 10);
				break;
			case 'l':
				min_mol_len = strtol(optarg, &r, 10);
				break;
			case 'O':
				out_dir = optarg;
				break;
			case 'o':
				out_fn = optarg;
				break;
			default:
				if (c != 'h') fprintf(stderr, "[E::%s] undefined option %c\n", __func__, c);
help:	
				fprintf(stderr, "\nUsage: %s %s [options] <GAP_BED> <BAM_FILEs> ...\n", program, argv[0]);
				fprintf(stderr, "Options:\n");
				fprintf(stderr, "         -b    INT      minimum barcode number for each molecule [5]\n");	
				fprintf(stderr, "         -B    INT      maximum barcode number for each molecule [inf]\n");
				fprintf(stderr, "         -O    STR      output directory [.]\n");
				fprintf(stderr, "         -o    STR      output SAT file [stdout]\n");
				fprintf(stderr, "         -c    INT      minimum coverage [10]\n");
				fprintf(stderr, "         -r    FLOAT    minimum coverage ratio [.5]\n");
				fprintf(stderr, "         -C    INT      maximum coverage [inf]\n");
				fprintf(stderr, "         -q    INT      minimum average mapping quality for each molecule [%d]\n", MIN_MAQ);
				/*fprintf(stderr, "         -S    INT      minimum aislignment score [0]\n");*/
				fprintf(stderr, "         -l    INT      minimum molecule length [1000]\n");
				fprintf(stderr, "         -S    INT      maximum spanning length [20000]\n");
				fprintf(stderr, "         -L    INT      maximum insertion length [1000]\n");
				fprintf(stderr, "         -m    FLOAT    minimum ratio between maixmum coverage and the gap coverage [.2]\n");
				fprintf(stderr, "         -a    INT      minimum barcode for contig [5]\n");
				fprintf(stderr, "         -h             help\n");
				return 1;	
		}		
	}
	if (optind + 2 > argc) {
		fprintf(stderr,"[E::%s] require at least one bed and bam file!\n", __func__); goto help;
	}
	char *sat_fn = argv[optind++];
	char **bam_fn = argv+optind;
	int n_bam = argc - optind;
	fprintf(stderr, "Program starts\n");	
	mk_brks_10x(sat_fn, bam_fn, n_bam, 0, min_mq, min_cov, min_cov_rat,  max_span, max_cov, max_is, min_bc, max_bc, min_rat, min_inner_bcn,  min_mol_len, option, out_dir, out_fn);
	fprintf(stderr, "Program ends\n");	
	return 0;	
}


int main_brks(int argc, char *argv[])
{
	int c;
	char *sat_fn = 0, *links_fn = 0, *out_fn = 0;
	int min_mq = 10;
	int limn = 2;
	float min_rat = 0.1;
	char *outdir = ".";
	char *prefix = "scaffs.bk";	
	char *program;
   	(program = strrchr(argv[0], '/')) ? ++program : (program = argv[0]);
	--argc, ++argv;
	while (~(c=getopt(argc, argv, "m:q:O:p:h"))) {
		switch (c) {
			case 'q':
				min_mq = atoi(optarg);
				break;	
			case 'm':
				min_rat = atof(optarg);
				break;	
			case 'p':
				prefix = optarg;
				break;	
			case 'O':
				outdir = optarg;
				break;	
			default:
				if (c != 'h') fprintf(stderr, "[E::%s] undefined option %c\n", __func__, c);
help:	
				fprintf(stderr, "\nUsage: %s [<options>] <SAT> <BAMs> ...\n", program);
				fprintf(stderr, "Options:\n");
				/*fprintf(stderr, "         -n    INT      allow successor or predecessor be top N candidates [2]\n");*/
				fprintf(stderr, "         -m    FLOAT    minimum coverage ratio between maximu coverage and the gap coverage [.1]\n");
				fprintf(stderr, "         -q    INT      minimum mapping quality [10]\n");
				fprintf(stderr, "         -O    STR      output directory [.]\n");
				fprintf(stderr, "         -p    STR      output file prefix [scaffs.bk]\n");
				fprintf(stderr, "         -h             help\n");
				return 1;	
		}		
	}
	if (optind + 2 > argc) {
		fprintf(stderr,"[E::%s] require a SAT and bam files!\n", __func__); goto help;
	}
	/*links_fn = argv[optind++];*/
	sat_fn = argv[optind++];
	char **bam_fn = argv + optind;
	int n_bams = argc - optind;	
	/*fprintf(stderr, "%d\t%s\t%s\n", n_bams, bam_fn[0], bam_fn[1]);*/
	fprintf(stderr, "Program starts\n");	
	mk_brks(sat_fn, bam_fn, n_bams, min_mq, min_rat, outdir, prefix);
/*int mk_brks(char *sat_fn, char *links_fn, int limn, char *out_fn)*/
	/*mk_brks(sat_fn, links_fn, limn, out_fn);*/
	fprintf(stderr, "Program ends\n");	
	return 0;	
}

