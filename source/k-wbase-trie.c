/*
 * k-wbase-trie.c - get words that matches a pattern
 */

#include "k-wbase.h"
#include "k-wbase-intern.h"

#include "file.h"

extern int MAX_WORD_LENGTH;

/*
 * Create blank trie node
 */
static node_t* node_create(void)
{
  node_t* node = malloc(sizeof(node_t));

  node->is_end_of_word = false;
  node->is_used = false;

  memset(node->children, 0, sizeof(node_t*) * ALPHABET_SIZE);

  return node;
}

trie_t* trie_create(void) { return node_create(); }

/*
 * Free memory of trie node
 */
static void node_free(node_t** node)
{
  if(!node || !(*node)) return;

  for(int index = 0; index < ALPHABET_SIZE; index++)
  {
    node_free((*node)->children + index);
  }

  free(*node);

  *node = NULL;
}

void trie_free(trie_t** trie) { node_free((node_t**) trie); }

/*
 * Insert word in trie
 */
static void trie_word_insert(trie_t* trie, const char* word)
{
  node_t* node = (node_t*) trie;

  for(int index = 0; word[index] != '\0'; index++)
  {
    int child_index = letter_index_get(word[index]);

    if(child_index == -1) return;


    if(!node->children[child_index])
    {
      node->children[child_index] = node_create();
    }

    node = node->children[child_index];
  }

  node->is_end_of_word = true;
}

/*
 * Convert string to lowercase
 */
static char* string_lower(char* string)
{
  if(!string) return NULL;

  for(char* letter = string; *letter; letter++)
  {
    *letter = tolower(*letter);
  }

  return string;
}

/*
 * Load words from file and create trie struct
 *
 * PARAMS
 * - const char* wfile | Word file
 *
 * RETURN (trie_t* trie)
 * - NULL | Failed to read file
 */
trie_t* trie_load(const char* wfile)
{
  if(!wfile) return NULL;

  char words_dir[1024];

  sprintf(words_dir, "%s/.korsord/words", getenv("HOME"));


  size_t file_size = dir_file_size_get(words_dir, wfile);

  printf("file_size: %ld\n", file_size);

  char* buffer = malloc(sizeof(char) * (file_size + 1));

  if(dir_file_read(buffer, file_size, words_dir, wfile) == 0)
  {
    return NULL;
  }

  buffer[file_size] = '\0';


  trie_t* trie = trie_create();

  char* token = strtok(buffer, "\n");

  while(token)
  {
    char* word = string_lower(strdup(token));

    if(strlen(word) <= MAX_WORD_LENGTH)
    {
      trie_word_insert(trie, word);
    }

    free(word);

    token = strtok(NULL, "\n");
  }

  free(buffer);

  return trie;
}

/*
 * Mark word in trie as used
 */
void trie_word_use(trie_t* trie, const char* word)
{
  node_t* node = (node_t*) trie;

  for(int index = 0; word[index] != '\0'; index++)
  {
    int child_index = letter_index_get(word[index]);

    if(child_index == -1) return;


    if(!node->children[child_index])
    {
      node = NULL;
      break;
    }

    node = node->children[child_index];
  }

  if(node) node->is_used = true;
}

/*
 * Remove used mark of word in trie
 */
void trie_word_unuse(trie_t* trie, const char* word)
{
  node_t* node = (node_t*) trie;

  for(int index = 0; word[index] != '\0'; index++)
  {
    int child_index = letter_index_get(word[index]);

    if(child_index == -1) return;


    if(!node->children[child_index])
    {
      node = NULL;
      break;
    }

    node = node->children[child_index];
  }

  if(node) node->is_used = false;
}

/*
 * Reset trie node, by removing the used mark
 */
static void node_reset(node_t* node)
{
  if(!node) return;

  for(int index = 0; index < ALPHABET_SIZE; index++)
  {
    node_reset(node->children[index]);
  }

  node->is_used = false;
}

void trie_reset(trie_t* trie) { node_reset((node_t*) trie); }

/*
 * Duplicate trie node
 *
 * RETURN (node_t* dup)
 * - allocated node_t struct
 */
static node_t* node_dup(node_t* node)
{
  if(!node) return NULL;

  node_t* dup = malloc(sizeof(node_t));

  dup->is_end_of_word = node->is_end_of_word;
  dup->is_used        = node->is_used;

  for(int index = 0; index < ALPHABET_SIZE; index++)
  {
    dup->children[index] = node_dup(node->children[index]);
  }

  return dup;
}

trie_t* trie_dup(trie_t* trie) { return node_dup((node_t*) trie); }

/*
 * Copy trie node
 *
 * EXPECTS:
 * - copy and node have the same structure
 */
static node_t* node_copy(node_t* copy, node_t* node)
{
  if(!node || !copy) return NULL;

  copy->is_end_of_word = node->is_end_of_word;
  copy->is_used        = node->is_used;

  for(int index = 0; index < ALPHABET_SIZE; index++)
  {
    node_copy(copy->children[index], node->children[index]);
  }

  return copy;
}

trie_t* trie_copy(trie_t* copy, trie_t* trie) { return node_copy((node_t*) copy, (node_t*) trie); }
