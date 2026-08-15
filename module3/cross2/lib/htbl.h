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

typedef enum ERR_CODE{
    HTBL_OK = 0,
    HTBL_ERR_PARAM,
    HTBL_ERR_NOMEM,
    HTBL_ERR_NOT_FOUND
} htbl_err;

htbl_err htbl_init   (htbl_s *ht, hashf hash_of, equalityf key_equal, equalityf val_equal, del_f delkey, del_f delval);
htbl_err htbl_free   (htbl_s *ht);

htbl_err htbl_put    (htbl_s *ht, void* key, void* value);
htbl_err htbl_find   (const htbl_s *ht, const void* value, void** rkey  );
htbl_err htbl_get    (const htbl_s *ht, const void* key  , void** rvalue);
htbl_err htbl_erase  (htbl_s *ht, void* value);
htbl_err htbl_remove (htbl_s *ht, void* key);

htbl_err htbl_foreach(htbl_s *ht, void (*callback)(void *key, void *value));

int htbl_contains (const htbl_s *ht, const void* key);
size_t htbl_size  (const htbl_s *ht);