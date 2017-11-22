// Copyright 2017 Matteo Alessio Carrara <sw.matteoac@gmail.com>

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

#define die(...) {fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE);}
#define dbg(...) fprintf(stderr, __VA_ARGS__)

struct node
{
	unsigned int freq;
	unsigned char data;
	struct node *lc, *rc, *parent;
};

int main(int argc, char **argv)
{
	FILE *fp;
	unsigned char buf = 0, s = 0;
	unsigned short nc = 0, syms = 0, qi=0;
	static unsigned int freq[256];
	static struct node leaf[256], *wq[256], nd[255], *lp[256], *tp;

	if(argc < 2) die("Please give me a filename!\n");
	if(!(fp = fopen(argv[1], "r"))) die("Cannot open input file\n");

	// calc frequencies and used symbols
	for(int c; (c = fgetc(fp)) != EOF; freq[c]++) if(!freq[c]) syms++;
	if(!syms) die("There is nothing to do...\n");

	// create leaf nodes and initialize working queue
	for(short i = 0; i < 256; i++)
	{
		if(freq[i])
		{
			leaf[qi] = (struct node){freq[i], i, NULL, NULL, NULL};
			wq[qi] = lp[i] = leaf + qi;
			qi++;
		}
	}

	while(qi > 2)
	{
		// find the two smallest item
		unsigned short idx1 = 0, idx2 = 1;
		for(int i = 0; i < qi;  i++) if((wq[i]->freq < wq[idx1]->freq) && (i != idx2)) idx1 = i;
		for(int i = 0; i < qi;  i++) if((wq[i]->freq < wq[idx2]->freq) && (i != idx1)) idx2 = i;

		// create a parent node with them as child
		nd[nc] = (struct node){wq[idx1]->freq + wq[idx2]->freq, 0, wq[idx1], wq[idx2], NULL};
		wq[idx1]->parent = wq[idx2]->parent = nd + nc++;

		// remove child from working queue and insert the parent
		for(int i = (idx1 > idx2? idx1 : idx2); i < (qi - 1); i++) wq[i] = wq[i+1];
		for(int i = (idx1 > idx2? idx2 : idx1); i < (qi - 1); i++) wq[i] = wq[i+1];
		wq[--qi-1] = nd + (nc - 1);
	}

	// create root node
	nd[nc] = (struct node){wq[0]->freq + wq[1]->freq, 0, wq[0], wq[1], NULL};
	wq[0]->parent = wq[1]->parent = nd + nc++;

	// start encoding of the file

	rewind(fp); // this will not work if the input is taken from stdin!
	// for complexity reasons, the file will be wrote from right to left
	for(int tmp; (tmp = fgetc(fp)) != EOF;)
	{
		// if we can encode this
		if((tp = lp[tmp]))
		{
			// get the path until we arrive to the root node
			// and write it into the output buffer
			do
			{
				if(s > 7) // buffer is full
				{
					printf("%uc", buf);
					buf = s = 0;
				}
				if(tp == tp->parent->lc)
				{
					buf &= ~(1 << s++);
				}
				else if(tp == tp->parent->rc)
				{
					buf |= (1 << s++);
				}
				else die("Invalid node reference\n");
				tp = tp->parent;
			}while(tp->parent);
		}
		else die("Cannot encode symbol %x! Input may have been changed...\n", tmp);
	}
	printf("%uc%uc", buf, 8 - s); // flush the buffer and writes the number of padding bits in the last byte
	return 0;
}
