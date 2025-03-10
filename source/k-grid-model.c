/*
 * k-grid-model.c - import grid from model
 */

#include "k-grid.h"
#include "k-grid-intern.h"

#include "k-wbase.h"

#include "file.h"

/*
 * Load grid from model
 *
 * RETURN (grid_t* grid)
 */
grid_t* grid_model_load(const char* model)
{
  if(!model) return NULL;

  char model_dir[1024];

  sprintf(model_dir, "%s/.korsord/models", getenv("HOME"));


  // 1. Read model file
  size_t file_size = dir_file_size_get(model_dir, model);

  if (file_size == 0)
  {
    return NULL;
  }

  char* buffer = malloc(sizeof(char) * (file_size + 1));

  if(dir_file_read(buffer, file_size, model_dir, model) == 0)
  {
    free(buffer);

    return NULL;
  }

  buffer[file_size] = '\0';


  // 2. Get width and height of model grid
  char* buffer_copy = strdup(buffer);

  int width  = 0;
  int height = 0;

  char* token = strtok(buffer_copy, "\n");

  for(height = 0; token; height++)
  {
    width = MAX(width, (strlen(token) + 1) / 2);

    token = strtok(NULL, "\n");
  }

  if(width < 3 || height < 3)
  {
    free(buffer_copy);
    free(buffer);

    return NULL;
  }

  // 3. Populate empty grid with model squares
  grid_t* grid = grid_create(width, height);

  strcpy(buffer_copy, buffer);

  token = strtok(buffer_copy, "\n");

  for(int y = 0; (y < height) && token; y++)
  {
    // The actual physical width of token
    int curr_width = strlen(token) / 2;

    for(int x = 0; x < width; x++)
    {
      square_t* square = xy_square_get(grid, x, y);

      char symbol = token[x * 2];

      // Choose square type depending on symbol
      switch(symbol)
      {
        case 'X':
          square->type = SQUARE_BORDER;
          break;

        case '.':
          square->type = SQUARE_EMPTY;
          break;

        case '#':
          square->type = SQUARE_BLOCK;
          break;

        default:
          int letter_index = letter_index_get(symbol);

          if(letter_index != -1)
          {
            square->type = SQUARE_LETTER;

            square->letter = symbol;
          }
          break;
      }
    }
    token = strtok(NULL, "\n");
  }

  free(buffer_copy);
  free(buffer);

  return grid;
}
