#include <stdlib.h>
#include <string.h>
#include <assert.h>

int
create_flat_sort_predicate (const void *_1, const void *_2)
{
  if (*(int *) (_1) > *(int *) (_2))
    {
      return 1;
    }
  else if (*(int *) (_1) < *(int *) (_2))
    {
      return -1;
    }
  else
    {
      return 0;
    }
}

struct Node *
create_flat_recursive (int *array, int L, int R)
{
  assert (L <= R);

  if (L == R)
    {
      return NULL;
    }

  int M = L + (R - L) / 2;
  assert (L <= M);
  assert (M <= R);

  struct Node *result = malloc (sizeof (struct Node));
  if (M < R)
    {
      result->x = array[M];
      result->left = create_flat_recursive (array, L, M);
      result->right = create_flat_recursive (array, M + 1, R);
    }
  else
    {
      result->x = array[L];
      result->left = NULL;
      result->right = NULL;
    }

  return result;
}

struct Node *
create_flat (int *a, int n)
{
  int *sorted = malloc (n * sizeof (*a));
  memcpy (sorted, a, n * sizeof (*a));
  qsort (sorted, n, sizeof (*sorted), create_flat_sort_predicate);

  struct Node *tree = create_flat_recursive (sorted, 0, n);

  free (sorted);
  return tree;
}
