// Copyright 2017 Matteo Alessio Carrara <sw.matteoac@gmail.com>

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#define die(...) {fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE);}

struct node
{
	uint32_t freq;
	uint8_t data;
	struct node *lc, *rc, *parent;
};

void encode(char *fn)
{
	FILE *fp;
	uint8_t nc = 0, buf = 0, s = 0;
	uint16_t sym = 0, qi=0;
	uint32_t freq[256];
	struct node *leaf, **wq, *nd = NULL, *bp[256], *tp;

	if(!(fp = fopen(fn, "r"))) die("Cannot open file '%s'\n", fn);

	// calc frequencies
	for(short i = 0; i < 256; i++) freq[i] = 0;
	for(int tmp; (tmp = fgetc(fp)) != EOF; freq[tmp]++);
	
	// calc used symbols
	for(short i = 0; i < 256; i++) if(freq[i]) sym++;
	if(!sym) die("There is nothing to do...\n");
	
	// initializes the tree structure
	leaf = calloc(sym, sizeof(struct node));
	wq = malloc(sizeof(struct node*) * sym);
	for(short i = 0; i < 256; i++) 
	{
		bp[i] = NULL;
		if(freq[i])
		{
			leaf[qi] = (struct node){freq[i], i, NULL, NULL, NULL};
			bp[i] = leaf + qi;
			wq[qi] = leaf + qi;
			qi++;
		}
	}
	
	while(qi > 2)
	{
		// find the two smallest item
		uint32_t f1 = wq[0]->freq, f2 = wq[1]->freq;
		uint16_t idx1 = 0, idx2 = 1;
		for(int i = 0; i < qi;  i++) if((wq[i]->freq < f1) && (i != idx2)) f1 = wq[idx1 = i]->freq;
		for(int i = 0; i < qi;  i++) if((wq[i]->freq < f2) && (i != idx1)) f2 = wq[idx2 = i]->freq;

		// create a parent node with them as child
		nd = realloc(nd, sizeof(struct node) * ++nc);
		nd[nc - 1] = (struct node){f1 + f2, 0, wq[idx1], wq[idx2], NULL};
		wq[idx1]->parent = wq[idx2]->parent = nd + (nc - 1);
		
		// remove child from working queue and insert the parent
		for(int i = (idx1 > idx2? idx1 : idx2); i < (qi - 1); i++) wq[i] = wq[i+1];
		for(int i = (idx1 > idx2? idx2 : idx1); i < (qi - 1); i++) wq[i] = wq[i+1];
		wq[--qi-1] = nd + (nc - 1);
	}

	// create root node
	nd = realloc(nd, sizeof(struct node) * ++nc);
	nd[nc - 1] = (struct node){wq[0]->freq + wq[1]->freq, 0, wq[0], wq[1], NULL};
	wq[0]->parent = wq[1]->parent = nd + (nc - 1);


	// start encoding of the file
	rewind(fp);
	for(int tmp; (tmp = fgetc(fp)) != EOF;)
	{
		// if we can encode this
		if((tp = bp[tmp]))
		{
			// get the path until we arrive to the root node
			do
			{
				if(s > 7)
				{
					printf("%c", buf);
					buf = s = 0;
				}
				if(tp->parent->lc == tp)
				{
					buf &= ~(1 << s++);
				}
				else if(tp->parent->rc == tp)
				{
					buf |= (1 << s++);
				}
				else die("Invalid node references\n");
				tp = tp->parent;
			}while(tp->parent);
		}
		else die("Cannot encode symbol %x! File may have been changed...\n", tmp);
	}
	//flush the buffer
	printf("%c", buf);

	fclose(fp);
}

int main(int argc, char **argv)
{
	if(argc < 2) die("Usage: hf filename\n");
	encode(argv[1]);
}
