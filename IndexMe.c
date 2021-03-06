#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define MAX_LINE 256
#define MAX_TERMS 20
#define MAX_LEN 50
#define MAX_DOCS 100

bool is_letter(char c) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

void clean_string(char s[]) {
  char *p = strchr(s, '\n');
  if (p != NULL) *p = 0;
}

bool solve_brackets(char buffer[], int *no_brackets, unsigned int *i, bool *result,
                    bool *NOT, int i_doc, int *j_term, char terms[MAX_TERMS][MAX_LEN], bool **index) {
  // Backstop
  if (*no_brackets == 0) {
    if (*NOT == 1) {
      *NOT = 0;
    }
    return *result;
  }

    while ((buffer[*i] > 'a' && buffer[*i] < 'z') ||
               (buffer[*i] > 'A' && buffer[*i] < 'Z')) {
        *i += 1;
    }

    if (buffer[*i] == ' ') {
        *j_term += 1;
        *i += 1;
    }

  if (buffer[*i] == '!') {
    *NOT = 1;
    *i += 1;
  }

  // Finding another parantheses
  if (buffer[*i] == '(') {
    *i += 1;
    *no_brackets += 1;
    *result = solve_brackets(buffer, no_brackets, i, result, NOT, i_doc, j_term,
                             terms, index);
  }

    // Logical interpretation
    if (buffer[*i] == '&' && buffer[*i + 1] == '&') {
        *result &= index[*i][*j_term];
        if(NOT == 1) *result = !(*result);
    }

    if (buffer[*i] == '|' && buffer[*i + 1] == '|') {
        *result |= index[*i][*j_term];
        if(NOT == 1) *result = !(*result);
    }
        
    *i += 2;

  // Going down the stiva
  if (buffer[*i] == ')') {
    *i += 1;
    *no_brackets -= 1;
    // solve_brackets(buffer, no_brackets, i, result, NOT, i_doc, j_term, terms, index);
  }

}

int main() {
  printf("Welcome to IndexMe, your very first search engine!\n");
  int no_terms;
  char terms[MAX_TERMS][MAX_LEN], buffer[MAX_LINE], buffer_aux[MAX_LINE], *p,
      delim[] = "&()!| ", blanks[] = " ";

  // Identificarea documentelor in care se va realiza query-ul
  printf("Enter the docs to search through in the following format - \"doc1 "
         "doc2 doc3 ...\"\n");
  int no_docs = 0;
  char docs[MAX_DOCS][MAX_LEN];

  fgets(buffer, sizeof(buffer), stdin);
  p = strtok(buffer, blanks);

  while (p != NULL) {
    strcpy(docs[no_docs++], p);
    p = strtok(NULL, blanks);
  }
  clean_string(docs[no_docs - 1]);

  // Alocam memorie pentru matricea de tipul inverted index
  bool **index = calloc(no_docs, sizeof(bool *));
  for (int i = 0; i < no_docs; i++) {
    index[i] = calloc(MAX_TERMS, sizeof(bool *));
  }

  printf("Enter your query below or type \"q\" to quit\n");
  while (fgets(buffer, sizeof(buffer), stdin)) {
    /// This while = each query
    if (strcasecmp(buffer, "q\n") == 0) {
      exit(0);
    }

    clean_string(buffer);
    no_terms = 0;
    // Citirea datelor de intrare = expresia logica

    // Identificam termenii query-ului
    strcpy(buffer_aux, buffer);

    p = strtok(buffer_aux, delim);
    while (p != NULL) {
      
      strcpy(terms[no_terms], p);
      no_terms++;
      p = strtok(NULL, delim);
    }
    clean_string(terms[no_terms - 1]);

    // Realocam memoria pt matricea index
    for (int i = 0; i < no_docs; i++) {
      index[i] = realloc(index[i], no_terms * sizeof(bool *));
      int array_size = sizeof(index[i])/sizeof(index[i][0]);
      memset(index[i], 0, array_size);
    }

    // Completam matricea de tipul inverted index:
    for (int i = 0; i < no_docs; i++) {
      FILE *f = fopen(docs[i], "rt");

      // Verificam deschiderea corecta a fisierului
      if (f == NULL) {
        fprintf(stderr, "Error opening file %s", docs[i]);
        exit(-1);
      }

      while (fgets(buffer_aux, sizeof(buffer_aux), f)) {
        clean_string(buffer_aux);
        for (int j = 0; j < no_terms; j++) {
          if ((p = strstr(buffer_aux, terms[j])) != NULL &&
              ((*(p + strlen(terms[j])) == ' ') ||
               *(p + strlen(terms[j])) == 0)) {
            index[i][j] = 1;
          }
        }
      }

      fclose(f);
    }

    // Rezolvam expresia logica (the real deal, ladies and gentlemen)
    // In buffer: expresia logica data; despartite de spatii
    char sol[MAX_DOCS][MAX_LEN];
    bool NOT, result;
    int no_brackets, k, j;
    unsigned int i_buff; // indicii documentelor continute docs[sol[k]] = numele
                // documentului
    k = 0;
    NOT = 0;
  
    for (int i = 0; i < no_docs; i++) {
        j = i_buff = no_brackets = 0;
        result = index[i][0];
      // i = doc; j = term
      while (i_buff < strlen(buffer)) {
        
        if (buffer[i_buff] == '!') {
          NOT = 1;
          i_buff++;
        }

        if (buffer[(i_buff)] == '(') {
          i_buff++;
          // intra pe pozitia de litera
          result = solve_brackets(buffer, &no_brackets, &i_buff, &result, &NOT, i, &j, terms, index);
        } // cand iese de aici, este pe pozitia primei litere dintr un termen


        if (buffer[i_buff] == ' ') {
          i_buff++;
        }

        if (is_letter(buffer[i_buff])) {
          j++;
        }

        while (is_letter(buffer[i_buff])) {
          i_buff++;
        }

        if (buffer[i_buff] == ' ') {
          i_buff++;
        }

        // Logical interpretation
        if (buffer[i_buff] == '&' && buffer[i_buff + 1] == '&') {
          result &= index[i][j];
          i_buff += 2;
        }

        if (buffer[i_buff] == '|' && buffer[i_buff + 1] == '|') {
          result |= index[i][j];
          i_buff += 2;
        }
      }
      if (result == 1) {
          strcpy(sol[k++], docs[i]);
      }
    }

    // Afisarea rezultatelor
    if(k) {
      printf("Query match in:");
      for (int i = 0; i < k; i++) {
          printf(" %s", sol[i]);
      }
      printf("\n");
    } else {
      system()
      printf("No match found\n");
    }

    printf("Enter your query below or type \"q\" to quit\n");
  }

  // Eliberarea memoriei
  for (int i = 0; i < no_docs; i++) {
    free(index[i]);
  }
  free(index);

  return 0;
}
