#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<ctype.h>

#include "Cipher.h"



/* Construieste un multi-dictionar pe baza unui fisier text
 * Cheia (elem) unui nod va fi reprezentata de un cuvant din text
 * iar valoarea (info) va fi indexul de inceput al acelui cuvant
 * ignorand caracterele separatoare (",.? \n\r") - i.e. numarul
 * de caractere A-Z precedente
 *
 * E.g: THIS IS AN EXAMPLE
 *
 * 			IS (4)
 *		    / \
 * 	  (6) AN  THIS (0)
 *         \
 *	 	 EXAMPLE (8)
 * */
void buildTreeFromFile(char* fileName, TTree* tree) {
	// Verificarea argumentelor
	if(fileName == NULL || tree == NULL)
		return;

	FILE* f = fopen(fileName, "r");

	int idx = 0;
	char sep[10] = ",.? \n\r", s[256];

	while(fgets(s, 256, f)) {
		char* p = strtok(s, sep);
		while(p) { 
			insert(tree, p, &idx);
			idx = idx + strlen(p);
			p = strtok(NULL, sep);
		}
	}
	fclose(f);	
}


/* Functie pentru afisarea unei chei de criptare
 * O cheie este reprezentata print-o serie de offset-uri
 *
 * E.g: key = [1, 2, 3, 4]
 * input text  = A    A    A    A
 *			   +1|  +2|  +3|  +4|
 * 				 V    V    V    V
 * output text = B    C    D    E
 *
 */
void printKey(char *fileName, Range *key) {

	FILE *f = fopen(fileName, "w");

	if (key == NULL) {
		fprintf(f, "No key provided!\n");
		fclose(f);
		return;
	}

	fprintf(f, "Decryption key of length %d is:\n", key->size);

	// Afiseaza fiecare offset din cheie % 26 (lungimea
	// alfabetului A-Z)
	for (int i = 0; i < key->size; i++) {
		fprintf(f, "%d ", key->index[i] % 26);

		// Rand nou pentru lizibilitate
		if ((i + 1) % 10 == 0)
			fprintf(f, "\n");
	}

	fclose(f);
}


/* Returneaza cheia obtinuta de parcurgerea in ordine
 * crescatoare a nodurilor arborelui (parcurgand si listele
 * de duplicate)
 */
Range* inorderKeyQuery(TTree* tree) {
	if(tree == NULL)
		return NULL;

	Range* r = (Range*) malloc(sizeof(Range));
	r->size = 0; r->capacity = 100;
	r->index = (int*) malloc(r->capacity * sizeof(int));
	TreeNode* n = minimum(tree->root);
	
	while(n) {
		r->index[r->size++] = *((int*)n->info);
		n = n->next;
	}

	return r;
}

int lvl(TreeNode* root) {
	if(root == NULL) 
		return 0;
	int l = 1;
	while(root->parent) {
		root = root->parent;
		l++;
	}
	return l;
}

/* Functie pentru extragerea cheii formate din valorile
 * nodurilor de la nivelul ce contine cel mai frecvent cuvant
 * (in cazul in care exista mai multe cuvinte cu numar maxim
 * de aparitii atunci se va considera primul nod dintre acestea conform
 * parcurgerii in inordine a arborelui)
 */
Range* levelKeyQuery(TTree* tree) {
	if(tree == NULL) 
		return NULL;
	
	Range* r = (Range*) malloc(sizeof(Range));
	r->capacity = 1000;
	r->size = 0;
	r->index = (int*) malloc((sizeof(int))*(r->capacity));
	int lmax = 0, l = 1, nivel = -1;
	TreeNode* n = minimum(tree->root);

	// gasire element cu cea mai mare frecventa 
	while(n){
		if(n->next && tree->compare(n->elem, n->next->elem) == 0) {
			l++;
		} else if(n->next && tree->compare(n->elem, n->next->elem) != 0) {
			if(l>lmax) {
				lmax = l;
				nivel =  lvl(n);
				l = 1;
			}
		} 
		n = n->next;
	}
	n = minimum(tree->root);
	while(n) {
		if(lvl(n) == nivel) {
			r->index[r->size] = *((int*)n->info);
			r->size++;
		}
		n = n->next;
	}

	return r;
}

/* Extragerea cheii din nodurile aflate intr-un anumit
 * domeniu de valori specificat
 */
Range* rangeKeyQuery(TTree* tree, char* q, char* p) {
	if(tree == NULL)
		return NULL;

	Range* r = (Range*) malloc(sizeof(Range));
	r->capacity = 1000;
	r->size = 0;
	r->index = (int*) malloc((sizeof(int))*(r->capacity));
	int i = 0;
	TreeNode* n = minimum(tree->root);

	while(n) {
		if(tree->compare(q, n->elem) == -1 && tree->compare(p, n->elem) == 1){
			r->index[i] = *((int*)n->info);
			i++;
			r->size++;
		}
		n = n->next;
	}
	return r;
}

void encrypt(char *inputFile, char *outputFile, Range *key) {

	FILE * f_in  = fopen(inputFile,  "r");
	FILE * f_out = fopen(outputFile, "w");

	if (f_in == NULL)
		return;

	char *buff = (char*) malloc(BUFLEN+1);
	char c;

	int idx = 0;

	while (fgets(buff, BUFLEN, f_in) != NULL) {

		for (int i = 0; i < strlen(buff); i++) {
			if (buff[i] != ' ' && buff[i] != '\n' && buff[i] != '\r') {
				c = ((toupper(buff[i]) - 'A') + key->index[idx] % 26) % 26 + 'A';
				idx += 1;
			} else
			 	c = buff[i];

			fprintf(f_out, "%c", c);

			if (idx == key->size)
				idx = 0;
		}
	}

	free(buff);
	fclose(f_in);
	fclose(f_out);
}


void decrypt(char *inputFile, char *outputFile, Range *key) {
 	
	FILE * f_in  = fopen(inputFile,  "r");
	FILE * f_out = fopen(outputFile, "w");

	if (f_in == NULL)
		return;

	char *buff = (char*) malloc(BUFLEN+1);
	char c;

	int idx = 0;

	while (fgets(buff, BUFLEN, f_in) != NULL) {

		for (int i = 0; i < strlen(buff); i++) {
			if (buff[i] != ' ' && buff[i] != '\n' && buff[i] != '\r') {
				c = ((toupper(buff[i]) - 'A') - (key->index[idx] % 26) + 26) % 26 + 'A';
				idx += 1;
			} else
			 	c = buff[i];

			fprintf(f_out, "%c", c);

			if (idx == key->size)
				idx = 0;
		}
	}

	free(buff);
	fclose(f_in);
	fclose(f_out);
}
