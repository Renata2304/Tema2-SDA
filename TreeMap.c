#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TreeMap.h"

#define MAX(a, b) (((a) >= (b))?(a):(b))


/* Creeaza un arbore cu o serie de metode asociate
 *
 * return: arborele creat
 */
TTree* createTree(void* (*createElement)(void*),
				  void (*destroyElement)(void*),
				  void* (*createInfo)(void*),
				  void (*destroyInfo)(void*),
				  int compare(void*, void*)) {
	TTree* tree = (TTree*) calloc(1, sizeof(TTree));

	tree->size = 0;
	tree->root = NULL;
	
	tree->createElement = createElement;
	tree->destroyElement = destroyElement;
	tree->createInfo = createInfo;
	tree->destroyInfo = destroyInfo;
	tree->compare = compare;

	return tree;
}


/* Verifica daca un arbore este gol (vid)
 * 		1 - daca arborele este gol
 * 		0 - in caz contrar
 */
int isEmpty(TTree* tree) {
	return (tree->root == NULL);
}


/* Cauta un anumit element in interiorul unui arbore
 *
 * tree: structura cu metodele asociate arborelui
 *   !folosit pentru a putea compara elementul curent cu cel cautat
 *
 * x: radacina arborelui curent (in care se face cautarea)
 * elem: elementul ce trebuie cautat
 */
TreeNode* search(TTree* tree, TreeNode* x, void* elem) {
	if(x == NULL)
		return NULL;
	while(x) {
		if(tree->compare(elem, x->elem) == 0)
			return x;
		if(tree->compare(elem, x->elem) == 1) 
			x = x->right;
		else if(tree->compare(elem, x->elem) == -1)
			x = x->left;
	}
	return NULL;
}


/* Gaseste nodul cu elementul minim dintr-un arbore
 * avand radacina in x
 */
TreeNode* minimum(TreeNode* x) {
	if(x == NULL)
		return x;
	while(x->left)
		x = x->left;
	return x;
}

/* Gaseste nodul cu elementul maxim dintr-un arbore
 * avand radacina in x
 */
TreeNode* maximum(TreeNode* x) {
	if(x == NULL)
		return x;
	while(x->right)
		x = x->right;
	return x;
}


/* Functie pentru gasirea succesorului unui nod
 * (succesorul in inordine)
 */
TreeNode* successor(TreeNode* x) {
	if(x == NULL)
		return x;

	if(x->right != NULL) {
		return minimum(x->right);
	}

	TreeNode* p = x->parent;
	while(p != NULL && x == p->right) {
		x = p;
		p = p->parent;
	}
	return p;
}

/* Functie pentru gasirea predecesorului unui nod
 * (predecesorul in inordine)
 */
TreeNode* predecessor(TreeNode* x) {
	if(x == NULL) 
		return x;
	
	if (x->left != NULL) {
		return maximum(x->left);
	} 
	TreeNode* tn = x->parent;
	while(tn != NULL && x == tn->left) {
		x = tn;
		tn = tn->parent;
	}
	return tn;
}


/* Actualizeaza inaltimea unui nod din arbore
 */
void updateHeight(TreeNode* x) {
	if(x == NULL)
		return;

	int leftHeight = 0;
	int rightHeight = 0;

	if (x->left != NULL)  leftHeight  = x->left->height;
	if (x->right != NULL) rightHeight = x->right->height;

	x->height = MAX(leftHeight, rightHeight) + 1;
}


/* Functie ce primeste adresa unui arbore si
 * a unui nod x si realizeaza o rotatie la stanga
 * a subarborelui care are varful in x
 *
 *   (x)	 Rotatie    (y)
 *	 / \     stanga     / \
 *  a  (y)    ---->   (x)  c
 *	   / \			  / \
 *    b   c          a   b
 */
void avlRotateLeft(TTree* tree, TreeNode* x) {
	if (tree == NULL || x == NULL || x->right == NULL) return;

	TreeNode* y = x->right;

	x->right = y->left;

	if(y->left != NULL)
		y->left->parent = x;
	
	if(x->parent != NULL) {
		y->parent = x->parent;

		if(x->parent->left == x)
			x->parent->left = y;
		else 
			x->parent->right = y;
	} else {
		y->parent = NULL;
		tree->root = y;
	}
	x->parent = y;
	y->left = x;
	
	updateHeight(x); updateHeight(y);
}


/* Functie ce primeste adresa unui arbore si
 * a unui nod x si realizeaza o rotatie la dreapta
 * a subarborelui care are varful in y
 *
 *     (y)	 Rotatie    (x)
 *	   / \   dreapta    / \
 *   (x)  c   ---->    a  (y)
 *	 / \			      / \
 *  a   b                b   c
 */
void avlRotateRight(TTree* tree, TreeNode* y) {
	if (tree == NULL || y == NULL || y->left == NULL) return;

	TreeNode* x = y->left;

	y->left = x->right;
	if(x->right != NULL)
		x->right->parent = y;

	if(y->parent != NULL) {
		x->parent = y->parent;

		if(y->parent->left == y)
			y->parent->left = x;
		else 
			y->parent->right = x;
	} else {
		x->parent = NULL;
		tree->root = x;
	}
	y->parent = x;
	x->right = y;

	updateHeight(x); updateHeight(y);
}


/* Calculeaza factorul de echilibrare pentru un nod x
 * (AVL balance factor)
*/
int avlGetBalance(TreeNode *x) {
	if(x == NULL)
		return 0;
	int rh = 0, lh = 0;
	if(x->left != NULL) lh = x->left->height;
	if(x->right != NULL) rh = x->right->height;
	return lh - rh;
}


/* Functie pentru reechilibrarea unui arbore AVL
 * in urma unei inserari prin rotatii simple sau duble
 * si recalcularea inaltimii fiecarui nod intalnit parcurgand
 * arborele de jos in sus, spre radacina
 *
 */
void avlFixUp(TTree* tree, TreeNode* y) {
	if(y == NULL)
		return;

	while(y){
		updateHeight(y);

		if(avlGetBalance(y) == 2) {
			if(avlGetBalance(y->left) == -1) {
				avlRotateLeft(tree, y->left); // left right case
				avlRotateRight(tree, y); // right case
			} else {
				avlRotateRight(tree, y); // right case
			}
		}

		if(avlGetBalance(y) == -2) {
			if(avlGetBalance(y->right) == 1) {
				avlRotateRight(tree, y->right); // right left case
				avlRotateLeft(tree, y); // left case
			} else {
				avlRotateLeft(tree, y); // left case
			}
		}
		y = y->parent;
	}
}

/* Functie pentru crearea unui nod
 *
 * value: valoarea/cheia din cadrul arborelui
 * info: informatia/valoarea din dictionar
 */
TreeNode* createTreeNode(TTree *tree, void* value, void* info) {

	if (tree == NULL)
		return NULL;

	// Alocarea memoriei
	TreeNode* node = (TreeNode*) malloc(sizeof(TreeNode));

	// Setarea elementului si a informatiei
	node->elem = tree->createElement(value);
	node->info = tree->createInfo(info);

	// Initializarea legaturilor din cadrul arborelui
	node->parent = node->right = node->left = NULL;

	// Initializarea legaturilor din cadrul listei dublu inlantuite
	node->next = node->prev = node->end = NULL;


	// Inaltimea unui nod nou este 1
	// Inaltimea lui NULL este 0
	node->height = 1;

	return node;
}

/* Inserarea unul nou nod in cadrul multi-dictionarului
 * ! In urma adaugarii arborele trebuie sa fie echilibrat
 *
 */
void insert(TTree* tree, void* elem, void* info) {
	if(tree == NULL || elem == NULL || info == NULL)
		return;

	tree->size++;

	TreeNode *n = createTreeNode(tree, elem, info), *n_insert = search(tree, tree->root, elem);
	n->end = n;
	// daca exista duplicate
	if(search(tree, tree->root, elem)) {
		TreeNode* aux = n_insert->end;

		n->prev = aux;
		n->next = aux->next;
		if(aux->next) {
			aux->next->prev = n;
		}
		aux->next = n;
		n_insert->end = n;
		tree->size++;
		return;
	}


	if(tree->root == NULL) { 
		tree->root = n;
		return;
	}

	TreeNode* x = tree->root;
	TreeNode* y = NULL;

	while(x) {
		y = x;
		if(tree->compare(x->elem, elem) == -1) {
			x = x->right;
		} else if(tree->compare(x->elem, elem) == 1) {
			x = x->left; 
		} 
	}
	
	n->parent = y;
	if(tree->compare(n->elem, y->elem) == -1)
		y->left = n;
	else
		y->right = n;
	
	TreeNode* p = predecessor(n), *s = successor(n);
	if(p) {
		n->prev = p;
		p->next = n;
	} else 
		n->prev = NULL; 
	if(s) {
		n->next = s;
		s->prev = n;
	} else 
		n->next = NULL;
	n->end = n;
	tree->size++;

	//2. Update list links for the new node z
	avlFixUp(tree, y);

}

/* Eliminarea unui nod dintr-un arbore
 *
 * ! tree trebuie folosit pentru eliberarea
 *   campurilor `elem` si `info`
 * */
void destroyTreeNode(TTree *tree, TreeNode* node){

	// Verificarea argumentelor functiei
	if(tree == NULL || node == NULL) 
		return;

	// Folosirea metodelor arborelui
	// pentru de-alocarea campurilor nodului
	tree->destroyElement(node->elem);
	tree->destroyInfo(node->info);

	// Eliberarea memoriei nodului
	free(node);
}

/* Eliminarea unui nod din arbore
 *
 * elem: cheia nodului ce trebuie sters
 * 	! In cazul in care exista chei duplicate
 *	  se va sterge ultimul nod din lista de duplicate
 */
void delete(TTree* tree, void* elem) {
	if(tree == NULL || tree->root == NULL || search(tree, tree->root, elem) == NULL)
		return;

	TreeNode *z = search(tree, tree->root, elem), *y, *x = NULL; 
	if(z == tree->root)
		y = NULL;
	else
		y = z->parent;
	
	if(z->next && tree->compare(z->elem, z->next->elem) == 0) {
		TreeNode* aux = z->end;				
		
		(z->end)->prev->next = z->end->next;
		if(z->end->next)
			(z->end)->next->prev = z->end->prev;

		z->end = aux->prev;
		tree->size--;
		destroyTreeNode(tree, aux);
		return;
	} else
	if(z->next && tree->compare(z->elem, z->next->elem) != 0) {
		if(z->prev)
			z->prev->next = z->next;			
		if(z->next)
			z->next->prev = z->prev;
	}

	// cazul in care se sterge o frunza
	if(z->left == NULL && z->right == NULL) {
		if(y) {
			if(y->left == z)
				y->left = NULL;
			else
				y->right = NULL;
			
			destroyTreeNode(tree, z);
			tree->size--;
			avlFixUp(tree, y);
			return;
		} else {
			tree->root = NULL;
			destroyTreeNode(tree, z);
			tree->size = 0;
			return;
		}
		
	} else  
	// cazul in care se sterge un nod cu 2 copii
	if(z->left && z->right) {
		x = minimum(z->right);
		
		if(x->parent != z){
			if(x->parent->left == x) {
				x->parent->left = NULL;
			} else {
				x->parent->right = NULL;
			}
		} else
			z->right = NULL;
			
		x->left = z->left;

		if(z->right != x)
			x->right = z->right;
		else 
			x->right = NULL;

		if(x->right)
			x->right->parent = x;
		
		if(x->left)
			 x->left->parent = x;

		if(y == NULL) {
			tree->root = x;
		} else {
			if(z->parent->right == z)
				y->right = x;
			else 
				y->left = x;
		}
		x->parent = y;

		destroyTreeNode(tree, z);
		
		tree->size--;
		avlFixUp(tree, y);
		return;
	} else {
	// cazul in care se sterge un nod cu un singur copil
		if(y != NULL) {
		if(z->left != NULL) { // daca copilul lui z e in partea stanga
			if(y->left == z) {
				y->left = z->left;
				y->left->parent = y;
			} else {
				y->right = z->left;
				y->right->parent = y;
			}
			destroyTreeNode(tree, z);
			tree->size--;
			avlFixUp(tree, y);
			return;
		} else { // daca copilul lui z in partea dreapta
			if(y->left == z) {
				y->left = z->right;
				y->left->parent = y;
			} else {
				y->right = z->right;
				y->right->parent = y;
			}
			destroyTreeNode(tree, z);
			tree->size--;
			avlFixUp(tree, y);
			return;
		}
		} else {
			if(z->left != NULL) { // daca copilul lui z e in partea stanga
				tree->root = z->left;
				tree->root->parent = y;
				destroyTreeNode(tree, z);
				tree->size--;
				avlFixUp(tree, y);
				return;
		  } else { // daca copilul lui z in partea dreapta
				tree->root = z->right;
				tree->root->parent = y;
				destroyTreeNode(tree, z);
				tree->size--;
				avlFixUp(tree, y);
				return;
		  }
		}
	}
}

/* Eliberarea memoriei unui arbore
 */
void destroyTree(TTree* tree){

	/* Se poate folosi lista dublu intalntuita pentru eliberarea memoriei
	 */
	if(tree == NULL)	
		return;
	
	if(tree->root == NULL) {
		free(tree);
		return;
	}

	TreeNode* x = minimum(tree->root), *y = x->next;

	// daca se sterge doar root ul
	if(y == NULL) {
		destroyTreeNode(tree, x);
		free(tree);
		return;
	}

	while(x) {
		destroyTreeNode(tree, x);
		x = y;
		if(x)
			y = x->next;
	}

	free(tree);
}
