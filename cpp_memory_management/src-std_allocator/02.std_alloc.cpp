#include <cstdlib>
#include <cstddef>
#include <new>

#define __THROW_BAD_ALLOC \
  cerr << "out of memory"; exit(1);


// 一级分配器
template <int inst>
class __malloc_alloc_template {
  static void* oom_malloc(size_t);
  static void* oom_realloc(void*, size_t);
  static void (*__malloc_alloc_oom_handler)();
  // 上面这个指针用来指向 new-handler(if any)
public:
  static void* allocate(size_t n) {
    void *result = malloc(n);
    if (0 == result) {
      result = oom_malloc(n);
    }
    return result;
  }
  static void deallocate(void *p, size_t /* n */) {
    free(p);
  }
  static void* reallocate(void *p, size_t /* old_sz */, size_t new_sz) {
    void *result = realloc(p, new_sz);
    if (0 == result) {
      result = oom_realloc(p, new_sz);
    }
    return result;
  }
  static void (*set_malloc_handler(void (*f)())) () {
    // 类似 c++ 的 set_new_handler()
    void (*old) () = __malloc_alloc_oom_handler; // 记录 new-handler
    __malloc_alloc_oom_handler = f;
    return(old);
  }
};


//-----------------------------------------------------------------------------
// 第二级分配器
//-----------------------------------------------------------------------------
enum {__ALIGN = 8};
enum {__MAX_BYTES = 128};
enum {__NFREELIST = __MAX_BYTES / __ALIGN};

// 本例中两个 template 参数完全没有派上用场
template <bool thread, int inst>
class __default_alloc_template {
private:
  // 实际上应使用 static const int x = N
  // 取代之前的 enum { x = N }; 但目前支持该性质的编译器不多

  static size_t ROUND_UP(size_t bytes) {
    return (((bytes) + __ALIGN - 1) & ~(__ALIGN - 1));
  }

private:
  union obj {  // type defimition
    union obj* free_list_link;
  };  // 改用 struct 亦可

private:
  static obj* volatile free_list [__NFREELIST];
  static size_t FREELIST_INDEX(size_t bytes) {
    return (((bytes) + __ALIGN - 1) / __ALIGN - 1);
  }
  // Returns an object of size n, and optionally adds to size n free list
  static void *refill(size_t n);

  // Allocates a chunk for nobjs of size "size" , nobjs may be reduced
  // if it is inconvenient ot allocate the requested number;
  static char* chunk_size(size_t size, int &nobjs);

  // Chunk allocation state
  static char* start_free;  // 指向 pool 的头
  static char* end_free;    // 指向 pool 的尾
  static size_t heap_size;  // 分配累加器
};
