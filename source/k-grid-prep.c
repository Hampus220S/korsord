/*
 * k-grid-prep.c - prepare grid before generation
 */

#include "k-grid.h"
#include "k-grid-intern.h"

#include "k-wbase.h"

/*
 * Assign the out most squares as SQUARE_BORDER
 *
 * This outer border is a buffer that prevents index out of range
 */
static void grid_prep_border(grid_t* grid)
{
  square_t* square;

  // 1. Fill in border on top and bottom
  for(int x = 0; x < (grid->width + 2); x++)
  {
    square = xy_real_square_get(grid, x, 0);
    
    if(square) square->type = SQUARE_BORDER;

    square = xy_real_square_get(grid, x, grid->height + 1);
    
    if(square) square->type = SQUARE_BORDER;
  }

  // 2. Fill in border to the left and right
  for(int y = 0; y < (grid->height + 2); y++)
  {
    square = xy_real_square_get(grid, 0, y);
    
    if(square) square->type = SQUARE_BORDER;

    square = xy_real_square_get(grid, grid->width + 1, y);
    
    if(square) square->type = SQUARE_BORDER;
  }
}

// __builtin_clz counts the leading zeros, so the bit length is:
#define CAPACITY(n) (1 << (sizeof(n) * 8 - __builtin_clz(n)))

/*
 * Append index to index array
 */
static int index_append(int** indexes, int* count, int index)
{
  if(*count == 0 || ((*count) + 1) >= CAPACITY(*count))
  {
    int* new_indexes = realloc(*indexes, sizeof(int) * CAPACITY((*count) + 1));

    if(!new_indexes) return 1;

    *indexes = new_indexes;
  }

  (*indexes)[(*count)++] = index;

  return 0;
}

/*
 * Get indexes of corners in grid
 */
static void grid_corner_indexes_get(int** indexes, int* count, grid_t* grid)
{
  /*   
   * . X . . 
   * X A . .
   * . .\. .
   * . . \ .
   *
   * X . . . 
   * . A . .
   * . .\. .
   * . . \ .
   */
  for(int x = (grid->width + 1); x-- > 1;)
  {
    for(int y = (grid->height + 1); y-- > 1;)
    {
      if(xy_real_square_is_border(grid, x, y))
      {
        continue;
      }

      if((xy_real_square_is_border(grid, x, y - 1) &&
          xy_real_square_is_border(grid, x - 1, y)) ||

         (xy_real_square_is_border(grid, x - 1, y - 1) &&
         !xy_real_square_is_border(grid, x, y - 1) &&
         !xy_real_square_is_border(grid, x - 1, y)))
      {
        int index = (y * (grid->width + 2) + x);

        index_append(indexes, count, index);
      }
    }
  }

  /*
   * . . / .
   * . ./. .
   * X V . .
   * . X . . 
   *
   * The pattern below is at the next to left edge
   *
   * . . / .
   * - ./. .
   * X V . .
   * . . . . 
   */
  for(int x = (grid->width + 1); x-- > 1;)
  {
    for(int y = 1; y < (grid->height + 1); y++)
    {
      if(xy_real_square_is_border(grid, x, y))
      {
        continue;
      }

      if((xy_real_square_is_border(grid, x - 1, y) &&
          xy_real_square_is_border(grid, x, y + 1)) ||

         (xy_real_square_is_border(grid, x - 1, y) &&
         !xy_real_square_is_border(grid, x - 1, y - 1)))
      {
        int index = (y * (grid->width + 2) + x);

        index_append(indexes, count, index);
      }
    }
  }

  /*
   * . . X .
   * . . A X
   * . ./. .
   * . / . .
   *
   * The pattern below is at the under edge of right block
   *
   * . - X .
   * . . A .
   * . ./. .
   * . / . .
   */
  for(int x = 1; x < (grid->width + 1); x++)
  {
    for(int y = (grid->height + 1); y-- > 1;)
    {
      if(xy_real_square_is_border(grid, x, y))
      {
        continue;
      }

      if((xy_real_square_is_border(grid, x, y - 1) &&
          xy_real_square_is_border(grid, x + 1, y)) ||

         (xy_real_square_is_border(grid, x, y - 1) &&
         !xy_real_square_is_border(grid, x - 1, y - 1)))
      {
        int index = (y * (grid->width + 2) + x);

        index_append(indexes, count, index);
      }
    }
  }
}

/*
 * Add blocks at left and top side of grid
 */
static void grid_prep_blocks(grid_t* grid)
{
  // 1. Get the squares in the corners
  int* indexes = NULL;
  int  count = 0;

  grid_corner_indexes_get(&indexes, &count, grid);


  // 2. Assign SQUARE_BLOCK to the squares in the corners
  for(int index = 0; index < count; index++)
  {
    int square_index = indexes[index];

    int x = square_index % (grid->width + 2);
    int y = square_index / (grid->width + 2);

    square_t* square = xy_real_square_get(grid, x, y);

    // Don't overwrite model letters
    if(square->type == SQUARE_LETTER) continue;

    square->type = SQUARE_BLOCK;
  }


  // 3. Randomly assign SQUARE_BLOCK to squares at edges
  for(int index = 0; index < count; index++)
  {
    int square_index = indexes[index];

    int start_x = square_index % (grid->width + 2);
    int start_y = square_index / (grid->width + 2);

    // Important: At least one open square next to top left block
    // Implement this as soon as possible

    /*
     * X X X X
     * X # + .
     */
    bool last_is_block = true;

    for(int x = (start_x + 1); x < (grid->width + 0); x++)
    {
      // This ensures that the egde is being followed
      if(!xy_real_square_is_border(grid, x, start_y - 1) ||
          xy_real_square_is_border(grid, x, start_y))
      {
        break;
      }

      square_t* square = xy_real_square_get(grid, x, start_y);

      // Don't overwrite model letters
      if(square->type == SQUARE_LETTER) continue;

      if(square->type == SQUARE_BLOCK)
      {
        last_is_block = true;

        continue;
      }

      // If the last square is not a block, this must be a block
      if(!last_is_block)
      {
        square->type = SQUARE_BLOCK;

        last_is_block = true;

        continue;
      }

      // If the last is a block, this has a 60% chance of also being a block
      if((rand() % 100) > 60)
      {
        square->type = SQUARE_BLOCK;

        last_is_block = true;
      }
      else last_is_block = false;
    }

    /*
     * X X
     * X #
     * X +
     * X .
     */
    last_is_block = true;

    for(int y = start_y; y < (grid->height + 2); y++)
    {
      // This ensures that the egde is being followed
      if(!xy_real_square_is_border(grid, start_x - 1, y) ||
          xy_real_square_is_border(grid, start_x, y))
      {
        break;
      }

      square_t* square = xy_real_square_get(grid, start_x, y);

      // Don't overwrite model letters
      if(square->type == SQUARE_LETTER) continue;

      if(square->type == SQUARE_BLOCK)
      {
        last_is_block = true;

        continue;
      }

      // If the last square is not a block, this must be a block
      if(!last_is_block)
      {
        square->type = SQUARE_BLOCK;

        last_is_block = true;

        continue;
      }

      // If the last is a block, this has a 60% chance of also being a block
      if((rand() % 100) > 60)
      {
        square->type = SQUARE_BLOCK;

        last_is_block = true;
      }
      else last_is_block = false;
    }
  }

  free(indexes);
}

/*
 * Identify words in grid and _use them,
 * so they can't be used elsewhere in the grid
 */
static void grid_words_use(wbase_t* wbase, grid_t* grid)
{
  // Identify vertical words and _use them
  for(int x = 1; x < (grid->width + 2); x++)
  {
    char word[grid->height + 1];
    int  length = 0;

    for(int y = 1; y < (grid->height + 2); y++)
    {
      square_t* square = xy_real_square_get(grid, x, y);

      if(square->type == SQUARE_EMPTY)
      {
        length = 0;
      }
      else if(square->type == SQUARE_BLOCK || square->type == SQUARE_BORDER)
      {
        if(length > 1)
        {
          word[length++] = '\0';

          wbase_word_use(wbase, word);
        }

        length = 0;
      }
      else if(square->type == SQUARE_LETTER)
      {
        word[length++] = square->letter;
      }
    }
  }

  // Identify horizontal words and _use them
  for(int y = 1; y < (grid->height + 2); y++)
  {
    char word[grid->width + 1];
    int  length = 0;

    for(int x = 1; x < (grid->width + 2); x++)
    {
      square_t* square = xy_real_square_get(grid, x, y);

      if(square->type == SQUARE_EMPTY)
      {
        length = 0;
      }
      else if(square->type == SQUARE_BLOCK || square->type == SQUARE_BORDER)
      {
        if(length > 1)
        {
          word[length++] = '\0';

          wbase_word_use(wbase, word);
        }

        length = 0;
      }
      else if(square->type == SQUARE_LETTER)
      {
        word[length++] = square->letter;
      }
    }
  }
}

/*
 * Prepare the grid before generation
 */
void grid_prep(wbase_t* wbase, grid_t* grid)
{
  grid_prep_border(grid);

  grid_prep_blocks(grid);

  grid_words_use(wbase, grid);
}
