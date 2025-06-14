#include "iterators.h"
#include <stdlib.h>
#include <string.h>

bool permutation(perm_iter_t *it) {
  ITERATOR(
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
          void *iptr = (char *)it->array + i * it->size;
          char elem[it->size];
          memcpy(elem, iptr, sizeof(elem));
          if (it->cycles[i] == 0) {
            memcpy(iptr, iptr + it->size, (it->length - i) * it->size);
            memcpy((char *)it->array + (it->length - 1) * it->size, elem,
                   it->size);
            it->cycles[i] = it->length - i;
          } else {
            size_t j = it->length - it->cycles[i];
            void *jptr = (char *)it->array + j * it->size;
            memcpy(iptr, jptr, it->size);
            memcpy(jptr, elem, it->size);
            YIELD();
          }
        }
        BREAK();
      },
      {
        free(it->cycles);
        it->cycles = NULL;
      });
}