#include "htbl.h"

#include <string.h>

#define HASH_SIZE 128


struct _entry{
    void* key  ;
    void* value;
    struct _entry *next;
};


/// @brief 
/// @param ht 
/// @param hash_of хэш функция для ключа 
/// @param key_equal функция сравнения ключей
/// @param val_equal функция сравнения значений          , может быть NULL, тогда сравнение идёт по указателям
/// @param delkey    функция освобождения памяти ключа   , может быть NULL
/// @param delval    функция освобождения памяти значения, может быть NULL
/// @return 
htbl_err htbl_init(htbl_s *ht, hashf hash_of, equalityf key_equal, equalityf val_equal, del_f delkey, del_f delval) {
    if (!ht || !hash_of || !key_equal) return HTBL_ERR_PARAM;

    ht->buckets = calloc(HASH_SIZE, sizeof(_entry_t*));
    if (!ht->buckets) return HTBL_ERR_NOMEM;

    ht->capacity = HASH_SIZE;
    ht->size = 0;
    
    ht->hash_of   = hash_of;
    ht->key_equal = key_equal;
    ht->val_equal = val_equal;
    ht->del_key   = delkey;
    ht->del_val   = delval;

    return HTBL_OK;
}

htbl_err htbl_free(htbl_s *ht) {
    if (!ht || !ht->buckets) return HTBL_ERR_PARAM;

    for (size_t i = 0; i < ht->capacity; i++) {
        _entry_t *curr = ht->buckets[i];
        while (curr) {
            _entry_t *tmp = curr;
            curr = curr->next;

            // Вызываем деструкторы, если они заданы
            if (ht->del_key && tmp->key)   ht->del_key(tmp->key);
            if (ht->del_val && tmp->value) ht->del_val(tmp->value);

            free(tmp);
        }
    }
    
    free(ht->buckets);
    ht->buckets = NULL;
    ht->capacity = 0;
    ht->size = 0;

    return HTBL_OK;
}

htbl_err htbl_put(htbl_s *ht, void* key, void* value) {
    if (!ht || !key) return HTBL_ERR_PARAM;

    uint32_t hash = ht->hash_of(key);
    size_t index = hash % ht->capacity;

    _entry_t *curr = ht->buckets[index];

    // Ищем ключ с помощью key_equal
    while (curr) {
        if (ht->key_equal(curr->key, key)) {
            if (ht->del_val && curr->value) ht->del_val(curr->value);
            curr->value = value;
            
            if (ht->del_key && curr->key != key) {
                ht->del_key(curr->key);
                curr->key = key; 
            }
            return HTBL_OK;
        }
        curr = curr->next;
    }

    _entry_t *new_node = malloc(sizeof(_entry_t));
    if (!new_node) return HTBL_ERR_NOMEM;

    new_node->key = key;
    new_node->value = value;
    new_node->next = ht->buckets[index];
    
    ht->buckets[index] = new_node;
    ht->size++;

    return HTBL_OK;
}

htbl_err htbl_find(const htbl_s *ht, const void* value, void** rkey) {
    if (!ht || !rkey) return HTBL_ERR_PARAM;

    for (size_t i = 0; i < ht->capacity; i++) {
        _entry_t *curr = ht->buckets[i];
        while (curr) {
            // Если val_equal задана - используем ее, иначе сравниваем адреса
            int match = ht->val_equal ? ht->val_equal(curr->value, value) : (curr->value == value);
            if (match) {
                *rkey = curr->key;
                return HTBL_OK;
            }
            curr = curr->next;
        }
    }
    return HTBL_ERR_NOT_FOUND;
}

htbl_err htbl_get(const htbl_s *ht, const void* key, void** rvalue) {
    if (!ht || !key || !rvalue) return HTBL_ERR_PARAM;

    uint32_t hash = ht->hash_of(key);
    size_t index = hash % ht->capacity;

    _entry_t *curr = ht->buckets[index];
    while (curr) {
        if (ht->key_equal(curr->key, key)) {
            *rvalue = curr->value;
            return HTBL_OK;
        }
        curr = curr->next;
    }

    return HTBL_ERR_NOT_FOUND;
}

htbl_err htbl_erase(htbl_s *ht, void* value) {
    if (!ht) return HTBL_ERR_PARAM;

    // Удаление по значению O(N)
    for (size_t i = 0; i < ht->capacity; i++) {
        _entry_t *curr = ht->buckets[i];
        _entry_t *prev = NULL;

        while (curr) {
            int match = ht->val_equal ? ht->val_equal(curr->value, value) 
                                      : (curr->value == value);
            if (match) {
                if (prev) {
                    prev->next = curr->next;
                } else {
                    ht->buckets[i] = curr->next;
                }

                if (ht->del_key && curr->key)   ht->del_key(curr->key);
                if (ht->del_val && curr->value) ht->del_val(curr->value);

                free(curr);
                ht->size--;
                return HTBL_OK;
            }
            prev = curr;
            curr = curr->next;
        }
    }
    return HTBL_ERR_NOT_FOUND;
}

htbl_err htbl_remove(htbl_s *ht, void* key) {
    if (!ht || !key) return HTBL_ERR_PARAM;

    uint32_t hash = ht->hash_of(key);
    size_t index = hash % ht->capacity;

    _entry_t *curr = ht->buckets[index];
    _entry_t *prev = NULL;

    while (curr) {
        if (ht->key_equal(curr->key, key)) {
            if (prev) {
                prev->next = curr->next;
            } else {
                ht->buckets[index] = curr->next;
            }

            if (ht->del_key && curr->key)   ht->del_key(curr->key);
            if (ht->del_val && curr->value) ht->del_val(curr->value);

            free(curr);
            ht->size--;
            return HTBL_OK;
        }
        prev = curr;
        curr = curr->next;
    }
    return HTBL_ERR_NOT_FOUND;
}

int htbl_contains(const htbl_s *ht, const void* key) {
    void *dummy = NULL;
    return (htbl_get(ht, key, &dummy) == HTBL_OK) ? 1 : 0;
}

size_t htbl_size(const htbl_s *ht) {
    return ht ? ht->size : 0;
}