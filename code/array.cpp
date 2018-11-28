


/*

my version of cpp vector, but without exceptions

push_back

pop_back

insert

delete

_expand

_capacity

_data

length






*/




#include <string.h>
#include <assert.h>


template<typename T>
struct array {
    unsigned int length;
    unsigned int _capacity;
    T* _data;
    
    T &operator[](unsigned int i) {
        return _data[i];
    }
    
    T const &operator[](unsigned int i) const {
        return _data[i];
    }
    
    
    void init() {
        length = 0;
		_capacity = 8;
		_data = (T*) malloc(sizeof(T) * _capacity);
    }
    
    void _expand() {
        _capacity = _capacity * 2 + 8;
        T* new_data = (T*)malloc(sizeof(T) * _capacity);
        memcpy(new_data, _data, length * sizeof(T));
        free(_data);
        _data = new_data;
    }
    
    void insert(T item) {
        if (length == _capacity) {
            _expand();
        }
        _data[length++] = item;
    }
    
    
    void remove(unsigned int i) {
        assert(i < length);
        assert(i >= 0);
        assert(length > 0);
        
        for(int j = i + 1; j < length; j++) {
            _data[j - 1] = _data[j];
        }
        
        length--;
    }
    
    void destroy() {
        if(_data) {
            free(_data);
        }
    }
};
