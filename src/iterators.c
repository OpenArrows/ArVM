#include "iterators.h"
#include <stdlib.h>
#include <string.h>

bool permutation(perm_iter_t *it) {
  do {
    if (!(it->cycles != ((void *)0))) {
      {
        if (it->permutation_length > it->length)
          goto iterator_end;
        else if (it->permutation_length == 0)
          it->permutation_length = it->length;
        it->cycles = malloc(sizeof(size_t) * it->permutation_length);
        for (size_t i = 0; i < it->permutation_length; i++)
          it->cycles[i] = it->length - i;
      };
    }
    {
      for (size_t i = it->permutation_length; i-- > 0;) {
        it->cycles[i]--;
        void *elem = it->array[i];
        if (it->cycles[i] == 0) {
          memcpy(&it->array[i], &it->array[i + 1],
                 sizeof(void *) * (it->length - i));
          it->array[it->length - 1] = elem;
          it->cycles[i] = it->length - i;
        } else {
          size_t j = it->length - it->cycles[i];
          it->array[i] = it->array[j];
          it->array[j] = elem;
          return 1;
        }
      }
      goto iterator_end;
    };
  iterator_end: {
    free(it->cycles);
    it->cycles = ((void *)0);
  };
    return 0;
  } while (0);
  /*ITERATOR(
      it->cycles != NULL,
      {
        if (it->permutation_length > it->length)
          BREAK();
        else if (it->permutation_length == 0)
          it->permutation_length = it->length;
        it->cycles = malloc(sizeof(size_t) * it->permutation_length);
        for (size_t i = 0; i < it->permutation_length; i++)
          it->cycles[i] = it->length - i;
      },
      {
        for (size_t i = it->permutation_length; i-- > 0;) {
          it->cycles[i]--;
          void *elem = it->array[i];
          if (it->cycles[i] == 0) {
            memcpy(&it->array[i], &it->array[i + 1],
                   sizeof(void *) * (it->length - i));
            it->array[it->length - 1] = elem;
            it->cycles[i] = it->length - i;
          } else {
            size_t j = it->length - it->cycles[i];
            it->array[i] = it->array[j];
            it->array[j] = elem;
            YIELD();
          }
        }
        BREAK();
      },
      {
        free(it->cycles);
        it->cycles = NULL;
      });*/
}