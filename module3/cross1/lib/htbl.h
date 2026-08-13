#include "err.h"

#include <stdlib.h>
#include <stdint.h>

typedef uint32_t (*hashf )   (const void*);
typedef int      (*equalityf)(const void*, const void*);
typedef void     (*del_f)    (void *ptr);

typedef struct _entry _entry_t;

typedef struct hash_table {
    _entry_t **buckets;   // Массив указателей на узлы
    size_t capacity;      // Размер массива бакетов
    size_t size;          // Количество элементов
    hashf hash_of;        // Хеш-функция для ключа
    equalityf val_equal;  
    equalityf key_equal;
    del_f del_key;
    del_f del_val;
}htbl_s;



err htbl_init   (htbl_s *ht, hashf hash_of, equalityf key_equal, equalityf val_equal, del_f delkey, del_f delval);
err htbl_free   (htbl_s *ht);

err htbl_put    (htbl_s *ht, void* key, void* value);
err htbl_find   (const htbl_s *ht, const void* value, void** rkey  );
err htbl_get    (const htbl_s *ht, const void* key  , void** rvalue);
err htbl_erase  (htbl_s *ht, void* value);
err htbl_remove (htbl_s *ht, void* key);

int htbl_contains (const htbl_s *ht, const void* key);
size_t htbl_size  (const htbl_s *ht);